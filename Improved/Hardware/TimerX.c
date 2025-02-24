#include "TimerX.h" 
#include "stm32f4xx.h"
#include "encoder.h"

//使用举例：
//SysTick_Init(1);   // SysTick 每 1ms 触发
//TIM2_Init(10);     // TIM2 每 10ms 触发

volatile uint32_t sysTickCounter = 0;  // 计数变量
//使用 SysTick_Handler() 处理周期任务，如 定时任务调度。
//可用 millis() 记录系统时间，方便测量运行时间。

void SysTick_Init(uint32_t ms) 
{
    // 计算加载值，假设时钟频率为 168MHz
    uint32_t load = (SystemCoreClock / 1000) * ms - 1;
    
    SysTick->LOAD  = load;  // 设置重装载值
    SysTick->VAL   = 0;     // 清除当前计数
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | // 使用核心时钟
                     SysTick_CTRL_TICKINT_Msk   | // 开启中断
                     SysTick_CTRL_ENABLE_Msk;    // 使能 SysTick
}

// SysTick 中断处理函数（每 ms 进入一次）
void SysTick_Handler(void) {
    sysTickCounter++;  // 计数自增
}

// 读取时间戳
uint32_t millis(void) {
    return sysTickCounter;
}



#include "stm32f4xx.h"

void TIM7_Init(uint16_t ms) 
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);  // 使能 TIM7 时钟

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = (ms * 1000) - 1;  // 计算定时周期
    TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1;  // 1MHz 计数时钟 (168MHz / 2 / 84)
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);

    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);  // 使能更新中断
    TIM_Cmd(TIM7, ENABLE);  // 使能 TIM7

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

// TIM7 中断处理函数（每 10ms 触发）
void TIM7_IRQHandler(void) 
{
    if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
        // 在此执行 10ms 任务
			
				 encoder_left = Read_Encoder(TIM3);
				 encoder_right = Read_Encoder(TIM4);
			
				 computeSpeed(encoder_left,encoder_right);   // 计算轮速（带低通滤波）

			
    }
}

