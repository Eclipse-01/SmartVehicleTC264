/**
 * @file hw_drv.c
 * @brief ����Ͷ�����Ƶ�Ӳ������ʵ�֡�
 * 
 * ���ļ�����ʹ��PWM�źų�ʼ���Ϳ��Ƶ���Ͷ���ĺ���ʵ�֡�
 * ������ʼ��HIP4082��������������õ�����Ƶ�ռ�ձȵĺ�����
 * �Լ���ʼ�������ö��λ�õĺ�����
 * 
 * @note PWMͨ����Ƶ���ǻ����ض�Ӳ�����ö���ġ�
 */

#define MAX_SPEED 100 /**< ������Ƶ����ռ�ձȰٷֱ� */
#define PWM_1 ATOM0_CH0_P21_2 /**< ����ǰ����PWMͨ�� */
#define PWM_2 ATOM0_CH3_P21_5 /**< �������˵�PWMͨ�� */
#define PWM_3 ATOM0_CH1_P21_3 /**< �ҵ��ǰ����PWMͨ�� */
#define PWM_4 ATOM0_CH2_P21_4 /**< �ҵ�����˵�PWMͨ�� */

#define SERVO_MOTOR_PWM ATOM1_CH1_P33_9 /**< �����PWMͨ�� */
#define SERVO_MOTOR_FREQ 50 /**< ���PWM�źŵ�Ƶ�� */
#define SERVO_MAX 50 /**< ��������Ƕ� */
#define SERVO_CENTER 770 /**< ���������λ�� */

#include "zf_common_headfile.h"
#include "hw_drv.h"


/*����������� �ͺ�HIP4082*/
void hip4082_init(void)
{
    pwm_init(PWM_1, 17000, 0); // PWM ͨ�� R1 ��ʼ��Ƶ�� 17KHz ռ�ձȳ�ʼΪ 0
    pwm_init(PWM_2, 17000, 0); // PWM ͨ�� R2 ��ʼ��Ƶ�� 17KHz ռ�ձȳ�ʼΪ 0
    pwm_init(PWM_3, 17000, 0); // PWM ͨ�� R1 ��ʼ��Ƶ�� 17KHz ռ�ձȳ�ʼΪ 0
    pwm_init(PWM_4, 17000, 0); // PWM ͨ�� R2 ��ʼ��Ƶ�� 17KHz ռ�ձȳ�ʼΪ 0
}

void hip4082_set_duty(int8 left_duty, int8 right_duty)
{
    // ��鲢�������ռ�ձ�
    if (left_duty > MAX_SPEED)
    {
        left_duty = MAX_SPEED;
    }
    else if (left_duty < -MAX_SPEED)
    {
        left_duty = -MAX_SPEED;
    }

    // ��鲢�����Ҳ�ռ�ձ�
    if (right_duty > MAX_SPEED)
    {
        right_duty = MAX_SPEED;
    }
    else if (right_duty < -MAX_SPEED)
    {
        right_duty = -MAX_SPEED;
    }

    // ���������
    if (left_duty >= 0)
    {
        pwm_set_duty(PWM_1, 0);
        pwm_set_duty(PWM_2, left_duty * 50);
    }
    else
    {
        pwm_set_duty(PWM_1, 0);
        pwm_set_duty(PWM_2, (-left_duty) * 50);
    }

    // �Ҳ�������
    if (right_duty >= 0)
    {
        pwm_set_duty(PWM_3, right_duty * 50);
        pwm_set_duty(PWM_4, 0);
    }
    else
    {
        pwm_set_duty(PWM_3, (-right_duty) * 50);
        pwm_set_duty(PWM_4, 0);
    }
}

/*�������*/
void servo_init(void)
{
    pwm_init(SERVO_MOTOR_PWM, SERVO_MOTOR_FREQ, SERVO_CENTER);
}

int servo_position;

void servo_set_position(int16 angle)
{
    uint32_t pulseWide = SERVO_CENTER;
    if (angle > SERVO_MAX)
    {
        angle = SERVO_MAX;
    }
    if (angle < -SERVO_MAX)
    {
        angle = -SERVO_MAX;
    }
    pulseWide = (SERVO_CENTER + angle);
    pwm_set_duty(SERVO_MOTOR_PWM, pulseWide);
    servo_position = angle;
}