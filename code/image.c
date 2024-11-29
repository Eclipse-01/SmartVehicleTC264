#include "zf_common_headfile.h"
#include "image.h"

#define IMAGE_WIDTH  MT9V03X_W
#define IMAGE_HEIGHT MT9V03X_H
#define CCD_HEIGHT 10 //模拟CCD传感器采样的起始行 
#define CCD_WIDTH 10 //模拟CCD传感器采样的行数量

char image_raw[IMAGE_HEIGHT][IMAGE_WIDTH];
char image_compressed[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4];
char image_binary[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4];
char image_processed[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4];
char track[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4];

void cam_init(void)
{
    while (mt9v03x_init())
    {
        ips200_show_string(0, 80, "mt9v03x reinit.");
        system_delay_ms(100);
    }
}

void cam_get_image(void)
{
    if (mt9v03x_finish_flag)
    {
        memcpy(image_raw, mt9v03x_image, IMAGE_WIDTH * IMAGE_HEIGHT);
        mt9v03x_finish_flag = 0;
    }
    
}

void cam_compress_image(char (image_raw)[IMAGE_HEIGHT][IMAGE_WIDTH], char (image_compressed)[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4])
{
    //压缩图像,方法是取样
    for (int i = 0; i < IMAGE_HEIGHT / 4; i++)
    {
        for (int j = 0; j < IMAGE_WIDTH / 4; j++)
        {
            (image_compressed)[i][j] = (image_raw)[i * 4][j * 4];
        }
    }
}

//注意：只能传入压缩过的图像
void cam_bin_image(char (image_compressed)[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4], char (image_binary)[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4])
{
    // 计算直方图
    int histogram[256] = {0};
    for (int i = 0; i < IMAGE_HEIGHT / 4; i++)
    {
        for (int j = 0; j < IMAGE_WIDTH / 4; j++)
        {
            int pixel = (unsigned char)(image_compressed)[i][j];
            histogram[pixel]++;
        }
    }

    // 总像素数
    int total = (IMAGE_HEIGHT / 4) * (IMAGE_WIDTH / 4);

    // 计算累计直方图和累计平均值
    int sum = 0;
    for (int t = 0; t < 256; t++)
        sum += t * histogram[t];

    int sumB = 0;
    int wB = 0;
    int wF = 0;
    int varMax = 0;
    int threshold = 0;

    for (int t = 0; t < 256; t++)
    {
        wB += histogram[t];               // 前景像素点数
        if (wB == 0) continue;

        wF = total - wB;                  // 背景像素点数
        if (wF == 0) break;

        sumB += t * histogram[t];

        int mB = sumB / wB;               // 前景平均灰度
        int mF = (sum - sumB) / wF;       // 背景平均灰度

        // 类间方差
        int varBetween = wB * wF * (mB - mF) * (mB - mF);

        // 找到最大类间方差对应的阈值
        if (varBetween > varMax)
        {
            varMax = varBetween;
            threshold = t;
        }
    }

    // 根据阈值进行二值化
    for (int i = 0; i < IMAGE_HEIGHT / 4; i++)
    {
        for (int j = 0; j < IMAGE_WIDTH / 4; j++)
        {
            if ((unsigned char)(image_compressed)[i][j] > threshold)
            {
                (image_binary)[i][j] = 255;
            }
            else
            {
                (image_binary)[i][j] = 0;
            }
        }
    }
    ips200_show_gray_image(0, 0, image_binary, IMAGE_WIDTH / 4, IMAGE_HEIGHT / 4,IMAGE_WIDTH / 4, IMAGE_HEIGHT / 4, 127);
}

//模拟线性CCD传感器，返回小车偏离中线的偏移量
int bin_map_trace_ccd(char (*image_processed)[IMAGE_WIDTH / 4])
{
    #define NUM_ROWS 5           // 定义扫描的行数
    #define MIN_EDGE_WIDTH 3     // 定义有效边缘的最小宽度

    int left_edges[NUM_ROWS] = {0};
    int right_edges[NUM_ROWS] = {0};
    int valid_rows = 0;
    int sum_left = 0;
    int sum_right = 0;
    int car_position = (IMAGE_WIDTH / 4) / 2; // 假设小车位于图像中心

    // 从底部开始，向上扫描 NUM_ROWS 行
    for (int row = IMAGE_HEIGHT / 4 - 1; row >= IMAGE_HEIGHT / 4 - NUM_ROWS; row--)
    {
        int left_edge = -1;
        int right_edge = -1;

        // 扫描左边缘
        for (int col = 0; col < IMAGE_WIDTH / 4; col++)
        {
            if (image_processed[row][col] == 255)
            {
                // 检测连续的白色像素，过滤噪声
                int count = 0;
                while (col + count < IMAGE_WIDTH / 4 && image_processed[row][col + count] == 1)
                {
                    count++;
                }
                if (count >= MIN_EDGE_WIDTH)
                {
                    left_edge = col + count / 2; // 取边缘中心
                    break;
                }
                else
                {
                    col += count;
                }
            }
        }

        // 扫描右边缘
        for (int col = IMAGE_WIDTH / 4 - 1; col >= 0; col--)
        {
            if (image_processed[row][col] == 255)
            {
                // 检测连续的白色像素，过滤噪声
                int count = 0;
                while (col - count >= 0 && image_processed[row][col - count] == 1)
                {
                    count++;
                }
                if (count >= MIN_EDGE_WIDTH)
                {
                    right_edge = col - count / 2; // 取边缘中心
                    break;
                }
                else
                {
                    col -= count;
                }
            }
        }

        // 如果找到有效的左右边缘，记录并计数
        if (left_edge != -1 && right_edge != -1)
        {
            left_edges[valid_rows] = left_edge;
            right_edges[valid_rows] = right_edge;
            valid_rows++;
        }
    }

    // 如果未找到有效边缘，返回 0
    if (valid_rows == 0)
    {
        return 0;
    }

    // 计算左、右边缘的平均值
    for (int i = 0; i < valid_rows; i++)
    {
        sum_left += left_edges[i];
        sum_right += right_edges[i];
    }

    int avg_left = sum_left / valid_rows;
    int avg_right = sum_right / valid_rows;

    // 计算赛道中线位置
    int center_line = (avg_left + avg_right) / 2;

    // 计算偏移量（正值表示小车在赛道左侧，负值表示在右侧）
    int offset = car_position - center_line;

    return offset;
}

int ccd_trace()
{
    cam_get_image();
    cam_compress_image(image_raw, image_compressed);
    cam_bin_image(image_compressed, image_binary);
    return bin_map_trace_ccd(image_binary);
}