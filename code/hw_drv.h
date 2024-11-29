/**
 * @file hw_drv.h
 * @brief ����Ͷ�����Ƶ�Ӳ������ͷ�ļ���
 * 
 * ���ļ��������ڳ�ʼ���Ϳ��Ƶ��������ĺ�����
 * ������ʼ��HIP4082��������������õ��ռ�ձȣ�
 * �Լ���ʼ�������ö��λ�õĺ���������
 * 
 * @note ��ȷ����ʹ�ñ�ͷ�ļ�ǰ������Ҫ�Ĺ���ͷ�ļ���
 */

#ifndef HW_DRV_H
#define HW_DRV_H

#include "zf_common_headfile.h"

/** @brief ��ʼ��HIP4082����������� */
void hip4082_init(void);

/**
 * @brief ����HIP4082�����������ռ�ձȡ�
 * 
 * �ú��������������ҵ����ռ�ձȡ�ռ�ձ�������MAX_DUTY����ķ�Χ�ڡ�
 * ��ֵ��Ӧǰ���˶�����ֵ��Ӧ�����˶���
 * 
 * @param left_duty ������ռ�ձ� (-MAX_DUTY �� MAX_DUTY)��
 * @param right_duty �ҵ����ռ�ձ� (-MAX_DUTY �� MAX_DUTY)��
 */
void hip4082_set_duty(int8 left_duty, int8 right_duty);

/** @brief ��ʼ������� */
void servo_init(void);

/**
 * @brief ���ö����λ�á�
 * 
 * �ú�������ָ���ĽǶ����ö����λ�á��Ƕ�������SERVO_MAX����ķ�Χ�ڡ�
 * 
 * @param angle ����������Ƕ� (-SERVO_MAX �� SERVO_MAX)��
 */
void servo_set_position(int16 angle);

extern int servo_position; /**< �����ǰλ�á� */

#endif /* HW_DRV_H */