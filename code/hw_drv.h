/**
 * @file hw_drv.h
 * @brief 电机和舵机控制的硬件驱动头文件。
 * 
 * 本文件声明用于初始化和控制电机及舵机的函数。
 * 包括初始化HIP4082电机驱动器、设置电机占空比，
 * 以及初始化和设置舵机位置的函数声明。
 * 
 * @note 请确保在使用本头文件前包含必要的公共头文件。
 */

#ifndef HW_DRV_H
#define HW_DRV_H

#include "zf_common_headfile.h"

/** @brief 初始化HIP4082电机驱动器。 */
void hip4082_init(void);

/**
 * @brief 设置HIP4082电机驱动器的占空比。
 * 
 * 该函数设置左电机和右电机的占空比。占空比限制在MAX_DUTY定义的范围内。
 * 正值对应前进运动，负值对应后退运动。
 * 
 * @param left_duty 左电机的占空比 (-MAX_DUTY 到 MAX_DUTY)。
 * @param right_duty 右电机的占空比 (-MAX_DUTY 到 MAX_DUTY)。
 */
void hip4082_set_duty(int8 left_duty, int8 right_duty);

/** @brief 初始化舵机。 */
void servo_init(void);

/**
 * @brief 设置舵机的位置。
 * 
 * 该函数根据指定的角度设置舵机的位置。角度限制在SERVO_MAX定义的范围内。
 * 
 * @param angle 舵机的期望角度 (-SERVO_MAX 到 SERVO_MAX)。
 */
void servo_set_position(int16 angle);

extern int servo_position; /**< 舵机当前位置。 */

#endif /* HW_DRV_H */