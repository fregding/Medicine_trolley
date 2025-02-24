#include "Motioncontrol.h" 


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

//  角度控制加上直线行驶的输出
// 优化后的运动控制函数
void updateMotionControl(float targetYaw, float *current_leftPWM, float *current_rightPWM)
{
    // 获取当前航向角（假设该函数已经获取到实际的 yaw 值）
    float yaw = getYawAngle();
    
    // 计算 yaw 误差，目标航向 - 当前航向
    float yaw_error = targetYaw - yaw;

    // 计算 PID 控制输出
    yaw_error_integral += yaw_error;  // 积分
    float yaw_derivative = yaw_error - last_yaw_error;  // 微分
    last_yaw_error = yaw_error;  // 更新上次误差

    // PID 控制器输出
    float correction = KP_mpu * yaw_error + KI_mpu * yaw_error_integral + KD_mpu * yaw_derivative;

    // 计算左右电机 PWM 修正（通过航向误差来调整转向）
    *current_leftPWM -= correction;
    *current_rightPWM += correction;

    // 限制 PWM 在合理范围内，防止过度调整导致不稳定
//    *current_leftPWM = constrain(*current_leftPWM, 0, MotorPWM_maxValue);
//    *current_rightPWM = constrain(*current_rightPWM, 0, MotorPWM_maxValue);
}
