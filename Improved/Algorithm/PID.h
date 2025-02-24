#ifndef __PID_H
#define __PID_H
#include "sys.h"


#define MAX_CONTROL_OUTPUT 10000.0f
#define MIN_CONTROL_OUTPUT -10000.0f

struct PID {
    float Kp;        // 比例增益
    float Ki;        // 积分增益
    float Kd;        // 微分增益
    float setPoint;  // 目标值
    float input;     // 实际值
    float lastError; // 上一次误差
    float integral;  // 积分项
};
int abs(int a);
	
float Incremental_Control(struct PID* pid, float measure, float target);
float Position_Control(struct PID* pid, float current, float target);

#endif
