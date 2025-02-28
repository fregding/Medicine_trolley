#include "encoder.h"

// 使用 TIMx 触发 定时中断，例如 10ms 读取一次编码器值

//	定时器	通道1（TI1）	通道2（TI2）
//	TIM1	PA8	PA9
//	TIM2	PA15/PB3	PB3/PA15
//	TIM3	PA6/PB4	PA7/PB5
//	TIM4	CH1	PB6	AF2
//	TIM4	CH2	PB7	AF2
//	TIM5	CH1	PA0	AF2
//	TIM5	CH2	PA1	AF2
//	定时器 CNT 寄存器（TIM3->CNT）会自动存储编码器的位置计数值，可以直接读取

int16_t encoder_left = 0;
int16_t encoder_right = 0;
float wheel_speed_left = 0, wheel_speed_right = 0; 

/**
  * @brief  TIM4 编码器模式初始化（PB6/CH1, PB7/CH2）
  * @param  无
  * @retval 无
  */
void ENCODER_1_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct = {0};
    TIM_ICInitTypeDef TIM_ICInitStruct = {0};

    /* 1. 使能时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); // GPIOB时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);  // TIM4时钟

    /* 2. 配置GPIO为复用模式 */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; // 上拉
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* 3. 映射引脚复用功能到TIM4 */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);

    /* 4. 配置TIM4时基（编码器模式需要） */
    TIM_TimeBaseStruct.TIM_Prescaler = 0;        // 无分频
    TIM_TimeBaseStruct.TIM_Period = 0xFFFF;      // ARR=65535（避免溢出）
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStruct);

    /* 5. 配置编码器接口（四倍频模式） */
    TIM_EncoderInterfaceConfig(TIM4, 
        TIM_EncoderMode_TI12,        // TI1和TI2均用于计数
        TIM_ICPolarity_BothEdge,    // 双边沿触发
        TIM_ICPolarity_BothEdge
    );

    /* 6. 配置输入捕获滤波器（可选） */
    TIM_ICInitStruct.TIM_ICFilter = 6; // 滤波值（0~15）
    TIM_ICInit(TIM4, &TIM_ICInitStruct);

    /* 7. 启动编码器 */
    TIM_Cmd(TIM4, ENABLE);
    TIM4->CNT = 30000; // 初始计数值（避免负数溢出）
}

/**
  * @brief  TIM5 编码器模式初始化（PA0/CH1, PA1/CH2）
  * @param  无
  * @retval 无
  */
void ENCODER_2_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct = {0};
    TIM_ICInitTypeDef TIM_ICInitStruct = {0};

    /* 1. 使能时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); // GPIOA时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);  // TIM5时钟

    /* 2. 配置GPIO为复用模式 */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; // 上拉
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* 3. 映射引脚复用功能到TIM5 */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM5);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_TIM5);

    /* 4. 配置TIM5时基 */
    TIM_TimeBaseStruct.TIM_Prescaler = 0;        // 无分频
    TIM_TimeBaseStruct.TIM_Period = 0xFFFFFFFF;  // TIM5是32位计数器
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStruct);

    /* 5. 配置编码器接口 */
    TIM_EncoderInterfaceConfig(TIM5, 
        TIM_EncoderMode_TI12, 
        TIM_ICPolarity_BothEdge,
        TIM_ICPolarity_BothEdge
    );

    /* 6. 输入捕获滤波器 */
    TIM_ICInitStruct.TIM_ICFilter = 6;
    TIM_ICInit(TIM5, &TIM_ICInitStruct);

    /* 7. 启动编码器 */
    TIM_Cmd(TIM5, ENABLE);
    TIM5->CNT = 30000; // 初始值
}

/* 读取编码器值 */
int16_t Read_Encoder(TIM_TypeDef *TIMx) 
{
    int16_t encoder_value = (int16_t)TIMx->CNT;
    TIMx->CNT = 0;  // 直接清零，避免溢出
    return encoder_value;
}

// RPM= （10ms内的编码器脉冲数/PULSE_PER_TURN ）×60

// 计算轮速
// ALPHA (float)0.3  // 低通滤波系数，取 0.1~0.5 之间，越小越平滑

void computeSpeed(int16_t LeftCNT,int16_t RightCNT) 
{
    float rpm1 = (LeftCNT / PULSE_PER_TURN) * 60.0;
    float rpm2 = (RightCNT / PULSE_PER_TURN) * 60.0;

    float new_speed_left = (rpm1 * (float)3.1416 * WHEEL_DIAMETER) / (float)60.0;
    float new_speed_right = (rpm2 *(float) 3.1416 * WHEEL_DIAMETER) / (float)60.0;

    // 低通滤波
    wheel_speed_left = ALPHA * new_speed_left + (1 - ALPHA) * wheel_speed_left;
    wheel_speed_right = ALPHA * new_speed_right + (1 - ALPHA) * wheel_speed_right;
}



