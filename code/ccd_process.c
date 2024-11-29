/*
 *                        _oo0oo_
 *                       o8888888o
 *                       88" . "88
 *                       (| -_- |)
 *                       0\  =  /0
 *                     ___/`---'\___
 *                   .' \\|     |// '.
 *                  / \\|||  :  |||// \
 *                 / _||||| -:- |||||- \
 *                |   | \\\  - /// |   |
 *                | \_|  ''\---/''  |_/ |
 *                \  .-\__  '-'  ___/-. /
 *              ___'. .'  /--.--\  `. .'___
 *           ."" '<  `.___\_<|>_/___.' >' "".
 *          | | :  `- \`.;`\ _ /`;.`/ - ` : | |
 *          \  \ `_.   \_ __\ /__ _/   .-` /  /
 *      =====`-.____`.___ \_____/___.-`___.-'=====
 *                        `=---='
 *
 *
 *      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *            ���汣��       ����崻�     ����BUG
 */

#include "zf_common_headfile.h"

#define IMAGE_WIDTH MT9V03X_W
#define IMAGE_HEIGHT MT9V03X_H
#define CCD_START_LOCATION 45 // ģ��CCD��������������ʼ��
#define CCD_DISTANCE    5     // ģ��CCD�������������м��
#define CCD_NUMS 5            // ģ��CCD������������������
#define NOISE_THRESHOLD 5     // ������ֵ

#define TURNINGS_CCD_LOCATION 110 //����ʱCCD����������ʼ��
#define TURNINGS_THRESHOLD 130    //����ʱCCD����������ֵ

unsigned char image[IMAGE_HEIGHT][IMAGE_WIDTH];    // ԭʼͼ��
unsigned char ccd_line[CCD_NUMS + 1][IMAGE_WIDTH]; // CCD������������ͼ��
unsigned char ccd_turnings_line[CCD_NUMS + 1][IMAGE_WIDTH]; // ����ʱCCD������������ͼ��

void ccd_init(void)
{
    while (mt9v03x_init())
    {
        ips200_show_string(0, 80, "mt9v03x reinit.");
        system_delay_ms(100);
    }
}

// ��ȡͼ��
void ccd_get_image(void)
{
    if (mt9v03x_finish_flag)
    {
        mt9v03x_finish_flag = 0;
        memcpy(image, mt9v03x_image, IMAGE_WIDTH * IMAGE_HEIGHT);
    }
}

// ��򷨺���
int otsu_threshold(int *histogram, int total_pixels)
{
    int sum = 0;
    int sumB = 0;
    int wB = 0;
    int wF = 0;
    float max_var = 0.0;
    int threshold = 0;

    for (int i = 0; i < 256; i++)
        sum += i * histogram[i];

    for (int i = 0; i < 256; i++)
    {
        wB += histogram[i];
        if (wB == 0)
            continue;
        wF = total_pixels - wB;
        if (wF == 0)
            break;

        sumB += i * histogram[i];
        float mB = (float)sumB / wB;
        float mF = (float)(sum - sumB) / wF;

        float var_between = (float)wB * (float)wF * (mB - mF) * (mB - mF);
        if (var_between > max_var)
        {
            max_var = var_between;
            threshold = i;
        }
    }
    return threshold;
}

// Ԥ����ͼ�񣬴�򷨶�ֵ���ͷָ��CCD_NUMS��
int image_process()
{
    int histogram[256] = {0};
    int total_pixels = 0;
    int threshold = 0;

    for (int i = 0; i < IMAGE_HEIGHT; i++)
    {
        for (int j = 0; j < IMAGE_WIDTH; j++)
        {
            histogram[image[i][j]]++;
            total_pixels++;
        }
    }

    threshold = otsu_threshold(histogram, total_pixels);

    for (int i = 0; i < IMAGE_HEIGHT; i++)
    {
        for (int j = 0; j < IMAGE_WIDTH; j++)
        {
            if (image[i][j] > threshold)
                image[i][j] = 255;
            else
                image[i][j] = 0;
        }
    }
    // ��ͼ��ָ�ΪCCD_NUMS��
    for (int i = 1; i <= CCD_NUMS; i++)
    {
        memcpy(ccd_line[i], image[CCD_START_LOCATION + i * CCD_DISTANCE], IMAGE_WIDTH * sizeof(unsigned char));
    }

    //��������ʱ��CCD������������ͼ��
    for (int i = 1; i <= CCD_NUMS; i++)
    {
        memcpy(ccd_turnings_line[i], image[TURNINGS_CCD_LOCATION + i * CCD_DISTANCE], IMAGE_WIDTH * sizeof(unsigned char));
    }
}

int ccd_error(int row)
{
    ips200_displayimage03x((const uint8 *)image, MT9V03X_W, MT9V03X_H);
    int left_edge;
    int right_edge;
    // �ȿ����Ե�������������ҡ����ҵ�����NOISE_THRESHOLD���ڵ�ʱ����Ϊ�ҵ������Ե
    for (int i = IMAGE_WIDTH / 2; i >= 0; i--)
    {
        if (ccd_line[row][i] == 0)
        {
            int count = 0;
            for (int j = 0; j < NOISE_THRESHOLD; j++)
            {
                if (ccd_line[row][i - j] == 0)
                    count++;
            }
            if (count >= NOISE_THRESHOLD - 1)
            {
                left_edge = i;
                break;
            }
        }
    }
    // �ٿ��ұ�Ե�������������ҡ����ҵ�����NOISE_THRESHOLD���ڵ�ʱ����Ϊ�ҵ����ұ�Ե
    for (int i = IMAGE_WIDTH / 2; i < IMAGE_WIDTH; i++)
    {
        if (ccd_line[row][i] == 0)
        {
            int count = 0;
            for (int j = 0; j < NOISE_THRESHOLD; j++)
            {
                if (ccd_line[row][i + j] == 0)
                    count++;
            }
            if (count >= NOISE_THRESHOLD - 1)
            {
                right_edge = i;
                break;
            }
        }
    }
    return (left_edge + right_edge) / 2 - IMAGE_WIDTH / 2;
}

int ccd_error_turings(int row)
{
    int left_edge;
    int right_edge;
    // �ȿ����Ե�������������ҡ����ҵ�����NOISE_THRESHOLD���ڵ�ʱ����Ϊ�ҵ������Ե
    for (int i = IMAGE_WIDTH / 2; i >= 0; i--)
    {
        if (ccd_turnings_line[row][i] == 0)
        {
            int count = 0;
            for (int j = 0; j < NOISE_THRESHOLD; j++)
            {
                if (ccd_turnings_line[row][i - j] == 0)
                    count++;
            }
            if (count >= NOISE_THRESHOLD - 1)
            {
                left_edge = i;
                break;
            }
        }
    }
    // �ٿ��ұ�Ե�������������ҡ����ҵ�����NOISE_THRESHOLD���ڵ�ʱ����Ϊ�ҵ����ұ�Ե
    for (int i = IMAGE_WIDTH / 2; i < IMAGE_WIDTH; i++)
    {
        if (ccd_turnings_line[row][i] == 0)
        {
            int count = 0;
            for (int j = 0; j < NOISE_THRESHOLD; j++)
            {
                if (ccd_turnings_line[row][i + j] == 0)
                    count++;
            }
            if (count >= NOISE_THRESHOLD - 1)
            {
                right_edge = i;
                break;
            }
        }
    }
    return (left_edge + right_edge) / 2 - IMAGE_WIDTH / 2;
}

//����Ƿ���Ϊ���������
int ccd_turnings(void)
{
    for (int i = 1; i < CCD_NUMS; i++)
    {
        //�鿴��ǰ�����е�CCD���Ƿ��ɫ���ص������������ֵ
        int count = 0;
        for (int j = 0; j < IMAGE_WIDTH; j++)
        {
            if (ccd_line[i][j] == 0)
                count++;
        }
        if (count <= TURNINGS_THRESHOLD)
            return 0;
    }
    return 1;
}


int ccd_trace(void)
{
    ccd_get_image();
    image_process();
    if (ccd_turnings())
    {
        int errors = 0;
        for (int i = 1; i < CCD_NUMS; i++)
        {
            errors += ccd_error_turings(i);
        }
        int finals = errors / CCD_NUMS;
        char str[64];
        sprintf(str, "Current error: %d\n", finals);
        wireless_uart_send_buffer(str, strlen(str));
        return finals;
    }
    else
    {
        int errors = 0;
        for (int i = 1; i < CCD_NUMS; i++)
        {
            errors += ccd_error(i);
        }
        int finals = errors / CCD_NUMS;
        char str[64];
        sprintf(str, "Current error: %d\n", finals);
        wireless_uart_send_buffer(str, strlen(str));
        return finals;
    }
}

