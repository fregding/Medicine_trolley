#ifndef __TIMERX_H
#define __TIMERX_H
#include "sys.h"
#include "Key.h"

void SysTick_Init(uint32_t ms);

void TIM7_Init(uint16_t ms);

void TIM3_Init(uint16_t time_ms);

uint32_t millis(void);
				    
#endif
