#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "zf_common_headfile.h"

// 函数声明
void straint_follow(void);
void PID_straint(void);

// 外部变量声明
extern int error;
extern int last_error;
extern int integral;
extern int derivative;
extern float output;

#endif // CONTROLLER_H
