/* 头文件顺序调整 */
#include "stm32f4xx.h"
#include "TimerX.h"
#include "encoder.h"
#include "Menu_TFT.h"

// 在main.c或独立模块中初始化TIM2为1ms计数器
void TIM2_Init(void) 
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseInitTypeDef TIM_BaseStruct = {
        .TIM_Prescaler = 84 - 1,          // 84MHz/84 = 1MHz
        .TIM_Period = 1000 - 1,           // 1MHz/1000 = 1kHz (1ms)
        .TIM_ClockDivision = TIM_CKD_DIV1,
        .TIM_CounterMode = TIM_CounterMode_Up
    };
    TIM_TimeBaseInit(TIM2, &TIM_BaseStruct);
    TIM_Cmd(TIM2, ENABLE);
}

// 获取当前时间戳（单位：ms）
uint32_t getCurrentTime(void) {
    return TIM2->CNT;  // 直接读取计数器值
}
//-----------------------------------------------------------------------------
// TIM7 定时器模块（10ms周期）
//-----------------------------------------------------------------------------
void TIM7_Init(uint16_t time_ms) 
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

    /* 动态获取定时器时钟 */
    RCC_ClocksTypeDef clocks;
    RCC_GetClocksFreq(&clocks);
    uint32_t timer_clock = clocks.PCLK1_Frequency;
    if(clocks.HCLK_Frequency / clocks.PCLK1_Frequency > 1) 
		{
        timer_clock *= 2;
    }

    const uint32_t desired_freq = 10000; // 10 kHz
    uint32_t psc = (timer_clock / desired_freq) - 1;
    uint32_t arr = (time_ms * desired_freq) / 1000 - 1;

    /* 参数边界检查 */
    if(psc > 0xFFFF || arr > 0xFFFF) 
		{
        while(1); // 错误处理
    }

    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseInitStructure);  // 修正为TIM7

    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM7, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03; // 最低抢占优先级
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void TIM7_IRQHandler(void) 
{
    if(TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET) 
		{
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
        encoder_left = (float)Read_Encoder(TIM3);
        encoder_right = (float)Read_Encoder(TIM4);
        
        computeSpeed(encoder_left, encoder_right);

    }
}

// TIM6初始化函数
void TIM6_Init(uint16_t time_ms)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // 使能TIM6时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

    // 动态计算定时器时钟（考虑APB1分频）
    RCC_ClocksTypeDef clocks;
    RCC_GetClocksFreq(&clocks);
    uint32_t timer_clock = clocks.PCLK1_Frequency;
    if (clocks.HCLK_Frequency / clocks.PCLK1_Frequency > 1) 
		{
        timer_clock *= 2;  // APB1分频时，TIM6时钟自动倍频
    }

    // 配置定时周期（示例：1kHz中断频率）
    const uint32_t desired_freq = 1000; // 1 kHz
    uint32_t psc = (timer_clock / desired_freq) - 1;
    uint32_t arr = (time_ms * desired_freq) / 1000 - 1;

    TIM_TimeBaseInitStruct.TIM_Period = arr;          // 自动重装载值
    TIM_TimeBaseInitStruct.TIM_Prescaler = psc;       // 预分频系数
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStruct);

    // 使能中断
    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM6, ENABLE);

    // 配置NVIC（中断优先级和通道）
    NVIC_InitStruct.NVIC_IRQChannel = TIM6_DAC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

// 中断处理函数
void TIM6_DAC_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET) 
		{
        key_handler();
			
				refresh_Flag = 1;
			
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);  // 必须清除中断标志
    }
}
