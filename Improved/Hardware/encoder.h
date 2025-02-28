#ifndef __ENCODER_H
#define __ENCODER_H

#include "sys.h"

// **编码器 & 速度计算参数**
#define PULSE_PER_TURN 1024    // MG513X 编码器每圈脉冲数
#define WHEEL_DIAMETER (float)0.065   // 轮径 6.5cm
#define ALPHA (float)0.3              // 低通滤波系数（越小越平滑）
#define CONTROL_PERIOD 10      // 控制周期 10ms
#define MAX_PWM 255            // PWM 最大值
#define MIN_PWM 0              // PWM 最小值

extern int16_t encoder_left;
extern int16_t encoder_right;
extern float wheel_speed_left, wheel_speed_right; // 轮速 (m/s)

void ENCODER_1_INIT(void);
void ENCODER_2_INIT(void);
	

int16_t Read_Encoder(TIM_TypeDef *TIMx) ;
void computeSpeed(int16_t LeftCNT,int16_t RightCNT);

#endif
