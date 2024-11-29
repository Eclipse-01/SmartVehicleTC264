/**
 * @file hw_drv.c
 * @brief 电机和舵机控制的硬件驱动实现。
 * 
 * 本文件包含使用PWM信号初始化和控制电机和舵机的函数实现。
 * 包括初始化HIP4082电机驱动器和设置电机控制的占空比的函数，
 * 以及初始化和设置舵机位置的函数。
 * 
 * @note PWM通道和频率是基于特定硬件设置定义的。
 */

#define MAX_SPEED 100 /**< 电机控制的最大占空比百分比 */
#define PWM_1 ATOM0_CH0_P21_2 /**< 左电机前进的PWM通道 */
#define PWM_2 ATOM0_CH3_P21_5 /**< 左电机后退的PWM通道 */
#define PWM_3 ATOM0_CH1_P21_3 /**< 右电机前进的PWM通道 */
#define PWM_4 ATOM0_CH2_P21_4 /**< 右电机后退的PWM通道 */

#define SERVO_MOTOR_PWM ATOM1_CH1_P33_9 /**< 舵机的PWM通道 */
#define SERVO_MOTOR_FREQ 50 /**< 舵机PWM信号的频率 */
#define SERVO_MAX 50 /**< 舵机的最大角度 */
#define SERVO_CENTER 770 /**< 舵机的中心位置 */

#include "zf_common_headfile.h"
#include "hw_drv.h"


/*电机驱动部分 型号HIP4082*/
void hip4082_init(void)
{
    pwm_init(PWM_1, 17000, 0); // PWM 通道 R1 初始化频率 17KHz 占空比初始为 0
    pwm_init(PWM_2, 17000, 0); // PWM 通道 R2 初始化频率 17KHz 占空比初始为 0
    pwm_init(PWM_3, 17000, 0); // PWM 通道 R1 初始化频率 17KHz 占空比初始为 0
    pwm_init(PWM_4, 17000, 0); // PWM 通道 R2 初始化频率 17KHz 占空比初始为 0
}

void hip4082_set_duty(int8 left_duty, int8 right_duty)
{
    // 检查并限制左侧占空比
    if (left_duty > MAX_SPEED)
    {
        left_duty = MAX_SPEED;
    }
    else if (left_duty < -MAX_SPEED)
    {
        left_duty = -MAX_SPEED;
    }

    // 检查并限制右侧占空比
    if (right_duty > MAX_SPEED)
    {
        right_duty = MAX_SPEED;
    }
    else if (right_duty < -MAX_SPEED)
    {
        right_duty = -MAX_SPEED;
    }

    // 左侧电机控制
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

    // 右侧电机控制
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

/*舵机部分*/
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