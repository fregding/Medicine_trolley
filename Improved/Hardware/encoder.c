#include "encoder.h"

// 使用 TIMx 触发 定时中断，例如 10ms 读取一次编码器值


float encoder_left = 0.f;
float encoder_right = 0.f;
float wheel_speed_left = 0, wheel_speed_right = 0; 

// TIM3编码器初始化（左轮）
void ENCODER_1_INIT(void)
{
    // 1. 使能时钟（标准库时钟使能方式）
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // 2. 配置GPIO（复用为定时器功能）
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_TIM3);

    // 3. 配置定时器编码器模式（标准库方法）
    TIM_TimeBaseInitTypeDef TIM_BaseInitStruct;
    TIM_ICInitTypeDef TIM_ICInitStruct;
    
    // 3.1 时基单元配置
    TIM_BaseInitStruct.TIM_Prescaler = 0;
    TIM_BaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_BaseInitStruct.TIM_Period = 0xFFFF;  // 16位计数器最大值
    TIM_BaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3, &TIM_BaseInitStruct);
    
    // 3.2 输入捕获配置（编码器模式）
    TIM_ICStructInit(&TIM_ICInitStruct);  // 初始化默认值
    TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStruct.TIM_ICFilter = 0x0;
    TIM_ICInit(TIM3, &TIM_ICInitStruct);

    TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;
    TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_IndirectTI;
    TIM_ICInit(TIM3, &TIM_ICInitStruct);

    // 3.3 启用编码器接口（模式3：TI1和TI2边沿均触发计数）
    TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, 
                              TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    
    TIM_Cmd(TIM3, ENABLE);  // 启动定时器
}

// TIM4编码器初始化（右轮）
void ENCODER_2_INIT(void)
{
    // 1. 使能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    // 2. 配置GPIO
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);

    // 3. 定时器配置
    TIM_TimeBaseInitTypeDef TIM_BaseInitStruct;
    TIM_ICInitTypeDef TIM_ICInitStruct;
    
    // 时基单元初始化
    TIM_BaseInitStruct.TIM_Prescaler = 0;
    TIM_BaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_BaseInitStruct.TIM_Period = 0xFFFF;  // 16位最大值
    TIM_BaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM4, &TIM_BaseInitStruct);

    // 输入捕获通道配置
    TIM_ICStructInit(&TIM_ICInitStruct);
    
    // 通道1配置
    TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStruct.TIM_ICFilter = 0x0;
    TIM_ICInit(TIM4, &TIM_ICInitStruct);

    // 通道2配置
    TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;
    TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_IndirectTI;
    TIM_ICInit(TIM4, &TIM_ICInitStruct);

    // 启用编码器接口（双边沿计数模式）
    TIM_EncoderInterfaceConfig(TIM4, 
                              TIM_EncoderMode_TI12,  // 模式3
                              TIM_ICPolarity_Rising, 
                              TIM_ICPolarity_Rising);
    
    TIM_Cmd(TIM4, ENABLE);  // 启动定时器
}

// 读取编码器计数值（带方向）
int16_t Read_Encoder(TIM_TypeDef *TIMx)
{
    int16_t cnt = (int16_t)(TIMx->CNT);
    TIMx->CNT = 0;  // 读取后清零计数器
    return cnt;
}

// 速度计算函数
void computeSpeed(int16_t LeftCNT, int16_t RightCNT)
{
    /* 完全消除隐式转换的写法 */
    wheel_speed_left = (float)(
        (double)LeftCNT * 
        (double)WHEEL_CIRCUMFERENCE / 
        ((double)PULSE_PER_TURN * (double)SAMPLE_TIME)
    );
    
    wheel_speed_right = (float)(
        (double)RightCNT * 
        (double)WHEEL_CIRCUMFERENCE / 
        ((double)PULSE_PER_TURN * (double)SAMPLE_TIME)
    );
}
