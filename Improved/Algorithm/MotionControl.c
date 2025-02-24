#include "Motioncontrol.h" 
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include <stddef.h>
#include "encoder.h"
#include "TimerX.h"

//	高精度转向：陀螺仪 PID + 电子罗盘IMU 
//	使用 IMU 进行航向角补偿：
//	陀螺仪短时间转向快但有漂移，可结合电子罗盘修正。
//	转向完成后做静态角度校准：
//	通过 MPU6050 读取 Z 轴角速度，使用 梯形积分法 计算实时角度。

//  10ms定时中断里设置：
//	encoder_left = Read_Encoder(TIM3);
//	encoder_right = Read_Encoder(TIM4);
//	computeSpeed( encoder_left, encoder_right);
//	updateMotorControl( targetSpeed, wheel_speed_left); //直行修正
//  getYawAngle();
//	updateMotionControl( targetYaw, &Real_leftPWM, &Real_rightPWM); //加角度控制

float yaw, pitch, roll;

void MPU_Get_Gyro(float *gz)
{
    short gyro[3];
    mpu_get_gyro_reg(gyro, NULL);
    *gz = gyro[2] / 16.4f;
}

float getYawAngle(void)
{
    static float yaw_angle = 0.0;
    static uint32_t last_time = 0;
    
    float encoder_angle = ((wheel_speed_right - wheel_speed_left) / WHEEL_DIAMETER) * (float)180.0 / (float)3.1416;
    float gyro_z;
    MPU_Get_Gyro(&gyro_z);

    uint32_t now = millis();
    float dt = (now - last_time) / 1000.0;
    last_time = now;

    float gyro_angle = yaw_angle + gyro_z * dt;
    yaw_angle =(float) 0.98 * gyro_angle + (float)0.02 * encoder_angle;

    return yaw_angle;
}



float yaw_error_integral = 0;
float last_yaw_error = 0;

//  加上直线行驶的输出
void updateMotionControl(float targetYaw,float *current_leftPWM, float *current_rightPWM)
{
    float yaw = getYawAngle();
    float yaw_error = targetYaw - yaw;

    yaw_error_integral += yaw_error;
    float yaw_derivative = yaw_error - last_yaw_error;
    last_yaw_error = yaw_error;

    float correction = KP_mpu * yaw_error + KI_mpu * yaw_error_integral + KD_mpu * yaw_derivative;

    *current_leftPWM -= correction;
    *current_rightPWM += correction;

    if (*current_leftPWM > 255) *current_leftPWM = 255;
    if (*current_rightPWM > 255) *current_rightPWM = 255;
    if (*current_leftPWM < 0) *current_leftPWM = 0;
    if (*current_rightPWM < 0) *current_rightPWM = 0;
}
