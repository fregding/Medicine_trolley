#ifndef __TIMERX_H
#define __TIMERX_H
#include "sys.h"

void SysTick_Init(uint32_t ms);

void TIM7_Init(uint16_t ms);

uint32_t millis(void);
				    
#endif
