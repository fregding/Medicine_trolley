#ifndef __ENCODER_H
#define __ENCODER_H

#include "sys.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"

// **编码器 & 速度计算参数**
// 若需要反向计数方向，可交换通道的DirectTI/IndirectTI设置：
// 通道1设为间接输入
// TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_IndirectTI;
// 通道2设为直接输入
// TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;

// 编码器参数配置（根据实际硬件修改）
//#define ENCODER_RESOLUTION  1    // 编码器每转脉冲数（PPR）
//#define WHEEL_CIRCUMFERENCE  0.5    // 轮子周长（单位：米）
//#define SAMPLE_TIME          0.01    // 采样周期（单位：秒，需与中断周期一致）

#define MAX_PWM 5000            // PWM 最大值
#define MIN_PWM 0              // PWM 最小值

extern float encoder_left;
extern float encoder_right;
extern float wheel_speed_left, wheel_speed_right; // 轮速 (m/s)

void ENCODER_1_INIT(void);
void ENCODER_2_INIT(void);
	

int16_t Read_Encoder(TIM_TypeDef *TIMx) ;
void computeSpeed(int16_t LeftCNT, int16_t RightCNT);

#endif
