#include "PID.h" 
#include <stdint.h>
#include <math.h>

// **增量式 PID 计算函数**
// 公式：output = P*(e(k)-e(k-1)) + I*e(k) + D*(e(k)-2*e(k-1)+e(k-2))
float Incremental_Control(struct PID* pid, float measure, float target) 
{
    static float err_1 = 0.0f;
    static float err_2 = 0.0f;
    static float controlOutput = 0.0f;

    float err = target - measure;

    // 计算 PID 各项
    float proportion = pid->Kp * (err - err_1);
    float integral = pid->Ki * err; // 仅使用当前误差
    float derivative = pid->Kd * (err - 2.0f * err_1 + err_2);

    // 计算增量并累加
    controlOutput += proportion + integral + derivative;

    // 限幅处理，防止输出超限
    if (controlOutput > MAX_CONTROL_OUTPUT) {
        controlOutput = MAX_CONTROL_OUTPUT;
    } else if (controlOutput < MIN_CONTROL_OUTPUT) {
        controlOutput = MIN_CONTROL_OUTPUT;
    }

    // 误差更新
    err_2 = err_1;
    err_1 = err;

    return controlOutput;
}


// **位置式 PID 计算函数**
// 公式：output = P*e(k) + I*∑e(i) + D*(e(k)-e(k-1))
float Position_Control(struct PID* pid, float current, float target) 
{
    static float lastError = 0.0f;
    static float integral = 0.0f;

    float err = target - current;

    // 计算 PID 各项
    float proportion = pid->Kp * err;
    
    // **抗积分饱和**
    if (fabs(err) < 50) { // 误差较小时才累积积分
        integral += pid->Ki * err;
        // **积分限幅**（避免积分项过大）
        if (integral > 500.0f) integral = 500.0f;
        if (integral < -500.0f) integral = -500.0f;
    }

    float derivative = pid->Kd * (err - lastError);

    // 计算总控制输出
    float controlOutput = proportion + integral + derivative;

    // 限幅处理
    if (controlOutput > MAX_CONTROL_OUTPUT) {
        controlOutput = MAX_CONTROL_OUTPUT;
    } else if (controlOutput < MIN_CONTROL_OUTPUT) {
        controlOutput = MIN_CONTROL_OUTPUT;
    }

    // 更新误差
    lastError = err;

    return controlOutput;
}


int abs(int a)
{
	if(a<0)
	{
		a=-a;
	}
	return a;
}
