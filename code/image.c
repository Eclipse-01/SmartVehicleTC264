#include "zf_common_headfile.h"
#include "image.h"

#define IMAGE_WIDTH  MT9V03X_W
#define IMAGE_HEIGHT MT9V03X_H
#define CCD_START_LOCATION 10 //ģ��CCD��������������ʼ�� 
#define CCD_BAND 10 //ģ��CCD������������������

char image_raw[IMAGE_HEIGHT][IMAGE_WIDTH];      //ԭʼͼ��
char image_compressed[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4];       //ѹ��ͼ��
char image_binary[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4];       //��ֵ��ͼ��
char image_processed[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4];        //������ͼ��
char track[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4];      //����ͼ��

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
    // ѹ��ͼ��, ������ȡ16�����ص�ƽ��ֵ
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

//ע�⣺ֻ�ܴ���ѹ������ͼ��
void cam_bin_image(char (image_compressed)[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4], char (image_binary)[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4])
{
    // ����ֱ��ͼ
    int histogram[256] = {0};
    for (int i = 0; i < IMAGE_HEIGHT / 4; i++)
    {
        for (int j = 0; j < IMAGE_WIDTH / 4; j++)
        {
            int pixel = (unsigned char)(image_compressed)[i][j];
            histogram[pixel]++;
        }
    }

    // ��������
    int total = (IMAGE_HEIGHT / 4) * (IMAGE_WIDTH / 4);

    // �����ۼ�ֱ��ͼ���ۼ�ƽ��ֵ
    long long sum = 0; // ʹ��long long�������
    for (int t = 0; t < 256; t++)
        sum += t * histogram[t];

    long long sumB = 0; // ʹ��long long�������
    int wB = 0;
    int wF = 0;
    long long varMax = 0; // ʹ��long long�������
    int threshold = 0;

    for (int t = 0; t < 256; t++)
    {
        wB += histogram[t];               // ǰ�����ص���
        if (wB == 0) continue;

        wF = total - wB;                  // �������ص���
        if (wF == 0) break;

        sumB += t * histogram[t];

        int mB = sumB / wB;               // ǰ��ƽ���Ҷ�
        int mF = (sum - sumB) / wF;       // ����ƽ���Ҷ�

        // ��䷽��
        long long varBetween = (long long)wB * wF * (mB - mF) * (mB - mF);

        // �ҵ������䷽���Ӧ����ֵ
        if (varBetween > varMax)
        {
            varMax = varBetween;
            threshold = t;
        }
    }

    // ������ֵ���ж�ֵ��
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

//ע�⣺ֻ�ܴ����ֵ��ͼ��
void noise_cancel(char image_binary[IMAGE_HEIGHT / 4][IMAGE_WIDTH / 4])
{
    sint16 nr; // ��
    sint16 nc; // ��

    // ����ͼ���ÿһ�к�ÿһ�У������߽�����
    for (nr = 1; nr < (IMAGE_HEIGHT / 4) - 1; nr++)
    {
        for (nc = 1; nc < (IMAGE_WIDTH / 4) - 1; nc++)
        {
            // �����ǰ����Ϊ0�������������ĸ��������г�����������Ϊ1���򽫵�ǰ������Ϊ1
            if ((image_binary[nr][nc] == 0)
                    && (image_binary[nr - 1][nc] + image_binary[nr + 1][nc] + image_binary[nr][nc + 1] + image_binary[nr][nc - 1] > 2))
            {
                image_binary[nr][nc] = 1;
            }
            // �����ǰ����Ϊ1�������������ĸ���������������������Ϊ1���򽫵�ǰ������Ϊ0
            else if ((image_binary[nr][nc] == 1)
                    && (image_binary[nr - 1][nc] + image_binary[nr + 1][nc] + image_binary[nr][nc + 1] + image_binary[nr][nc - 1] < 2))
            {
                image_binary[nr][nc] = 0;
            }
        }
    }
}


//ģ������CCD������������С��ƫ�����ߵ�ƫ����
int bin_map_trace_ccd(char (*image_processed)[IMAGE_WIDTH / 4])
{
    int ccd_line[IMAGE_WIDTH / 4] = {0}; // ģ��CCD������
    int mid_point = (IMAGE_WIDTH / 4) / 2; // ����λ��
    int offset = 0; // ƫ����

    // ģ��CCD����������
    for (int width = 0; width < IMAGE_WIDTH / 4; width++)
    {
        for (int height = CCD_START_LOCATION; height < CCD_START_LOCATION + CCD_BAND; height++)
        {
            ccd_line[width] += (unsigned char)image_processed[height][width];
        }
    }

    // ��ֵ��ccd_line
    for (int i = 0; i < IMAGE_WIDTH / 4; i++)
    {
        if (ccd_line[i] > (CCD_BAND * 255) / 2)
            ccd_line[i] = 1;
        else
            ccd_line[i] = 0;
    }

    // Ѱ���������ұ߽�
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

        // ������������λ��
        int line_center = (left_edge + right_edge) / 2;

        // ����ƫ����
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