#include "zf_common_headfile.h"
#include "hw_drv.h"
#include "controller.h"

/**
 * @brief 调节PID参数并控制舵机和电机。
 */
#define KP 2.736
#define KI 0
#define KD 4
#define MAX_INTEGRAL 1000
#define MAX_DERIVATIVE 1000

/**
 * @brief 跟随策略的主循环。
 */
void straint_follow()
{
    while(1){
        PID_straint();
        hip4082_set_duty(33, 33);
       // system_delay_ms(10);
    }
}

int error = 0;
int last_error = 0;
int integral = 0;
int derivative = 0;
float output = 0;   

/**
 * @brief PID控制函数，用于计算输出并控制舵机和电机。
 */
void PID_straint()
{
    error = ccd_trace();
    integral += error;
    derivative = error - last_error;
    output = KP * error + KI * integral + KD * derivative;
    last_error = error;

    if (integral > MAX_INTEGRAL)
    {
        integral = MAX_INTEGRAL;
    }
    else if (integral < -MAX_INTEGRAL)
    {
        integral = -MAX_INTEGRAL;
    }
    if (derivative > MAX_DERIVATIVE)
    {
        derivative = MAX_DERIVATIVE;
    }
    else if (derivative < -MAX_DERIVATIVE)
    {
        derivative = -MAX_DERIVATIVE;
    }
    servo_set_position(output);
}
