#include "zf_common_headfile.h"
#include "image.h"

#define IMAGE_WIDTH  MT9V03X_W
#define IMAGE_HEIGHT MT9V03X_H
#define CCD_START_LOCATION 10 //模拟CCD传感器采样的起始行 
#define CCD_BAND 10 //模拟CCD传感器采样的行数量

char image_raw[IMAGE_HEIGHT][IMAGE_WIDTH];      //原始图像
char image_compressed[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4];       //压缩图像
char image_binary[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4];       //二值化图像
char image_processed[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4];        //处理后的图像
char track[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4];      //赛道图像

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
    // 压缩图像, 方法是取16个像素的平均值
    for (int i = 0; i < IMAGE_HEIGHT / 4; i++)
    {
        for (int j = 0; j < IMAGE_WIDTH / 4; j++)
        {
            int sum = 0;
            for (int k = 0; k < 4; k++)
            {
                for (int l = 0; l < 4; l++)
                {
                    sum += (unsigned char)image_raw[i * 4 + k][j * 4 + l];
                }
            }
            image_compressed[i][j] = sum / 16;
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
    long long sum = 0; // 使用long long避免溢出
    for (int t = 0; t < 256; t++)
        sum += t * histogram[t];

    long long sumB = 0; // 使用long long避免溢出
    int wB = 0;
    int wF = 0;
    long long varMax = 0; // 使用long long避免溢出
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
        long long varBetween = (long long)wB * wF * (mB - mF) * (mB - mF);

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

//注意：只能传入二值化图像
void noise_cancel(char image_binary[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4])
{
    sint16 nr; // 行
    sint16 nc; // 列

    // 遍历图像的每一行和每一列，跳过边界像素
    for (nr = 1; nr < (IMAGE_HEIGHT / 4) - 1; nr++)
    {
        for (nc = 1; nc < (IMAGE_WIDTH / 4) - 1; nc++)
        {
            // 如果当前像素为0，且上下左右四个像素中有超过两个像素为1，则将当前像素设为1
            if ((image_binary[nr][nc] == 0)
                    && (image_binary[nr - 1][nc] + image_binary[nr + 1][nc] + image_binary[nr][nc + 1] + image_binary[nr][nc - 1] > 2))
            {
                image_binary[nr][nc] = 1;
            }
            // 如果当前像素为1，且上下左右四个像素中有少于两个像素为1，则将当前像素设为0
            else if ((image_binary[nr][nc] == 1)
                    && (image_binary[nr - 1][nc] + image_binary[nr + 1][nc] + image_binary[nr][nc + 1] + image_binary[nr][nc - 1] < 2))
            {
                image_binary[nr][nc] = 0;
            }
        }
    }
}


//模拟线性CCD传感器，返回小车偏离中线的偏移量
int bin_map_trace_ccd(char (*image_processed)[IMAGE_WIDTH / 4])
{
    int ccd_line[IMAGE_WIDTH / 4] = {0}; // 模拟CCD传感器
    int mid_point = (IMAGE_WIDTH / 4) / 2; // 中线位置
    int offset = 0; // 偏移量

    // 模拟CCD传感器采样
    for (int width = 0; width < IMAGE_WIDTH / 4; width++)
    {
        for (int height = CCD_START_LOCATION; height < CCD_START_LOCATION + CCD_BAND; height++)
        {
            ccd_line[width] += (unsigned char)image_processed[height][width];
        }
    }

    // 二值化ccd_line
    for (int i = 0; i < IMAGE_WIDTH / 4; i++)
    {
        if (ccd_line[i] > (CCD_BAND * 255) / 2)
            ccd_line[i] = 1;
        else
            ccd_line[i] = 0;
    }

    // 寻找赛道左右边界
    int left_edge = -1;
    int right_edge = -1;
    for (int i = 0; i < IMAGE_WIDTH / 4; i++)
    {
        if (ccd_line[i] == 1)
        {
            if (left_edge == -1)
                left_edge = i;
            right_edge = i;
        }
    }
    
    if (left_edge == -1)
        left_edge = 0;
    if (right_edge == -1)
        right_edge = IMAGE_WIDTH / 4 - 1;

        // 计算赛道中心位置
        int line_center = (left_edge + right_edge) / 2;

        // 计算偏移量
        return offset = line_center - mid_point;
}

int ccd_trace()
{
    cam_get_image();
    cam_compress_image(image_raw, image_compressed);
    cam_bin_image(image_compressed, image_binary);
    noise_cancel(image_binary, image_processed);
    return bin_map_trace_ccd(image_processed);
}