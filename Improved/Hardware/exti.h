#ifndef __EXTI_H
#define __EXTI_H

#include "stm32f4xx.h"         // 包含基础的头文件
#include "stm32f4xx_gpio.h"     // 包含GPIO相关函数
#include "stm32f4xx_exti.h"     // 包含EXTI相关函数
#include "stm32f4xx_syscfg.h"   // 包含SYSCFG相关函数
#include "stm32f4xx_rcc.h"      // 包含RCC相关函数
#include "misc.h"               // 包含中断优先级等函数
#include "sys.h"  	
#include "delay.h"

void EXTIX_Init(void);

	 				    
#endif
