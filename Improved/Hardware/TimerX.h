#ifndef __TIMERX_H
#define __TIMERX_H
#include "sys.h"
#include "Key.h"

//	高级定时器（TIM1/TIM8）
//	TIM1	
//		CH1: PA8		CH2: PA9		CH3: PA10		CH4: PA11		互补输出（CH1N）: PE8		ETR（外部触发）: PE7
//	TIM8	
//		CH1: PC6		CH2: PC7		CH3: PC8		CH4: PC9		互补输出（CH1N）: PC10

//	通用定时器（TIM2-TIM5）
//	TIM2（支持32位计数）		
//		CH1: PA0		CH2: PA1		CH3: PA2		CH4: PA3
//	TIM3
//		CH1: PA6		CH2: PA7		CH3: PB0		CH4: PB1
//	TIM4
//		CH1: PD12		CH2: PD13		CH3: PD14		CH4: PD15
//	TIM5（32位）
//		通道分配与TIM2相同（PA0-PA3）

//	简化通用定时器（TIM9-TIM14）
//	TIM9: CH1=PE5, CH2=PE6
//	TIM10: CH1=PF6
//	TIM11: CH1=PF7
//	TIM12: CH1=PH6, CH2=PH9
//	TIM13: CH1=PF8
//	TIM14: CH1=PF9

//	基本定时器（TIM6/TIM7） 无外部引脚，仅内部使用

void TIM2_Init(void);
// 获取当前时间戳（单位：ms）
uint32_t getCurrentTime(void);

void TIM7_Init(uint16_t ms);

void TIM6_Init(uint16_t time_ms);


				    
#endif
