#include "encoder.h"
#include <math.h>
#include <sys.h>	 
#include "TimerX.h"
// 使用 TIMx 触发 定时中断，例如 10ms 读取一次编码器值

float encoder_left = 0.f;
float encoder_right = 0.f;
float wheel_speed_left = 0, wheel_speed_right = 0; 

//// TIM3编码器初始化（优化版）
//void ENCODER_1_INIT(void)
//{
//    // 1. 使能时钟
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
//    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

//    // 2. 配置GPIO（复用为定时器功能）
//    GPIO_InitTypeDef GPIO_InitStruct = {
//        .GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7,
//        .GPIO_Mode = GPIO_Mode_AF,
//        .GPIO_Speed = GPIO_Speed_100MHz,
//        .GPIO_OType = GPIO_OType_OD,      // 改为开漏（或保持浮空输入）
//        .GPIO_PuPd = GPIO_PuPd_UP         // 上拉确保信号稳定
//    };
//    GPIO_Init(GPIOA, &GPIO_InitStruct);
//    
//    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);
//    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_TIM3);

//    // 3. 配置定时器编码器模式
//    TIM_TimeBaseInitTypeDef TIM_BaseInitStruct = {
//        .TIM_Prescaler = 0,
//        .TIM_Period = 0xFFFF,            // 16位最大值
//        .TIM_ClockDivision = TIM_CKD_DIV1,
//        .TIM_CounterMode = TIM_CounterMode_Up // 编码器模式会自动控制方向
//    };
//    TIM_TimeBaseInit(TIM3, &TIM_BaseInitStruct);

//    // 4. 配置编码器接口（关键修正）
//    TIM_EncoderInterfaceConfig(
//        TIM3, 
//        TIM_EncoderMode_TI12,            // 双边沿触发（模式3）
//        TIM_ICPolarity_BothEdge,           // CH1极性
//        TIM_ICPolarity_BothEdge            // CH2极性
//    );

//    // 5. 启用输入滤波（抑制噪声）
//    TIM_ICInitTypeDef TIM_ICInitStruct;
//    TIM_ICStructInit(&TIM_ICInitStruct);
//    TIM_ICInitStruct.TIM_Channel = TIM_Channel_1 | TIM_Channel_2;
//    TIM_ICInitStruct.TIM_ICFilter = 0x8; // 设置滤波（推荐值：8-15）
//    TIM_ICInit(TIM3, &TIM_ICInitStruct); // 仅配置滤波，不覆盖编码器模式参数

//    // 6. 启动定时器
//    TIM_Cmd(TIM3, ENABLE);
//}

//// TIM4编码器初始化（右轮，优化版）
//void ENCODER_2_INIT(void)
//{
//    // 1. 使能时钟
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
//    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

//    // 2. 配置GPIO（复用为定时器功能）
//    GPIO_InitTypeDef GPIO_InitStruct = {
//        .GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7,
//        .GPIO_Mode = GPIO_Mode_AF,
//        .GPIO_Speed = GPIO_Speed_100MHz,
//        .GPIO_OType = GPIO_OType_OD,      // 开漏模式，避免信号冲突
//        .GPIO_PuPd = GPIO_PuPd_UP         // 上拉确保信号稳定
//    };
//    GPIO_Init(GPIOB, &GPIO_InitStruct);
//    
//    // 确认GPIO复用映射到TIM4
//    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);
//    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);

//    // 3. 配置定时器编码器模式
//    TIM_TimeBaseInitTypeDef TIM_BaseInitStruct = {
//        .TIM_Prescaler = 0,              // 无分频
//        .TIM_Period = 0xFFFF,            // 16位计数器最大值
//        .TIM_ClockDivision = TIM_CKD_DIV1,
//        .TIM_CounterMode = TIM_CounterMode_Up // 编码器模式会自动控制方向
//    };
//    TIM_TimeBaseInit(TIM4, &TIM_BaseInitStruct);

//    // 4. 配置编码器接口（关键修正）
//    TIM_EncoderInterfaceConfig(
//        TIM4, 
//        TIM_EncoderMode_TI12,            // 双边沿触发（模式3）
//        TIM_ICPolarity_BothEdge,           // CH1极性
//        TIM_ICPolarity_BothEdge            // CH2极性
//    );

//    // 5. 启用输入滤波（抑制噪声）
//    TIM_ICInitTypeDef TIM_ICInitStruct;
//    TIM_ICStructInit(&TIM_ICInitStruct);
//    TIM_ICInitStruct.TIM_Channel = TIM_Channel_1 | TIM_Channel_2;
//    TIM_ICInitStruct.TIM_ICFilter = 0x8; // 滤波参数（推荐值8-15）
//    TIM_ICInit(TIM4, &TIM_ICInitStruct); // 仅配置滤波，不覆盖编码器模式参数

//    // 6. 启动定时器
//    TIM_Cmd(TIM4, ENABLE);
//}


//// 读取编码器计数值（带方向）
//int16_t Read_Encoder(TIM_TypeDef *TIMx) 
//{
//    int16_t cnt;
//    __disable_irq();          // 关闭全局中断
//    cnt = (int16_t)(TIMx->CNT);
//    __enable_irq();           // 开启全局中断
//    return cnt;
//}

 //全局变量记录上一次的CNT值
static int16_t prevLeftCNT = 0, prevRightCNT = 0;

void computeSpeed(int16_t LeftCNT, int16_t RightCNT) 
{
    // 计算脉冲差值（考虑CNT溢出）
    int16_t deltaLeft = (int16_t)(LeftCNT - prevLeftCNT);
    int16_t deltaRight = (int16_t)(RightCNT - prevRightCNT);
    
    // 更新历史值
    prevLeftCNT = LeftCNT;
    prevRightCNT = RightCNT;

    // 计算实际速度（需替换为实际参数）
    wheel_speed_left = (deltaLeft * 10);
    wheel_speed_right = (deltaRight * 10);

}


/**************************************************************************
Function: Initialize TIM3 as the encoder interface mode
Input   : none
Output  : none
函数功能：把TIM3初始化为编码器接口模式
入口参数：无
返回  值：无
**************************************************************************/
void Encoder_Init_TIM3(void) 
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;  
  TIM_ICInitTypeDef TIM_ICInitStructure;  
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);   // 使能 TIM3 时钟
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);  // 使能 GPIOA 时钟（原 GPIOB 改为 GPIOA）

  // 配置 PA6 和 PA7 为 TIM3 编码器输入
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;  // PA6 (TIM3_CH1), PA7 (TIM3_CH2)
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;            // 复用模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;          // 开漏输出（根据编码器信号类型可选）
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;           // 内部上拉（根据硬件连接调整）
  GPIO_Init(GPIOA, &GPIO_InitStructure);                  // 初始化 GPIOA

  // 复用 PA6 和 PA7 为 TIM3 功能
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);  // PA6 -> TIM3_CH1
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_TIM3);  // PA7 -> TIM3_CH2

  // TIM3 基础配置（保持不变）
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);  
  TIM_TimeBaseStructure.TIM_Prescaler = 0x0;              // 无分频
  TIM_TimeBaseStructure.TIM_Period = ENCODER_TIM_PERIOD;  // 自动重装载值
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // 无时钟分频
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);         // 初始化 TIM3

  // 编码器接口配置（保持不变）
  TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising); // 模式3（双边沿计数）
  TIM_ICStructInit(&TIM_ICInitStructure);
  TIM_ICInitStructure.TIM_ICFilter = 0;                   // 无输入滤波
  TIM_ICInit(TIM3, &TIM_ICInitStructure);                 // 初始化输入捕获
  
  // 清除标志位并使能定时器
  TIM_ClearFlag(TIM3, TIM_FLAG_Update);                   // 清除更新标志
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);              // 使能更新中断（可选）
  TIM_SetCounter(TIM3, 0);                                // 计数器归零
  TIM_Cmd(TIM3, ENABLE);                                  // 启动 TIM3
}
/**************************************************************************
Function: Initialize TIM4 as the encoder interface mode
Input   : none
Output  : none
函数功能：把TIM4初始化为编码器接口模式
入口参数：无
返 回 值：无
**************************************************************************/
void Encoder_Init_TIM4(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;  
  TIM_ICInitTypeDef TIM_ICInitStructure;  
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);//使能定时器4的时钟
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能PB端口时钟
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;//端口配置
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);   //根据设定参数初始化GPIOB

  GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_TIM4); //复用为TIM4 编码器接口
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource7,GPIO_AF_TIM4); //复用为TIM4 编码器接口
  
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  
  TIM_TimeBaseStructure.TIM_Prescaler = 0x0; // No prescaling 
  TIM_TimeBaseStructure.TIM_Period = ENCODER_TIM_PERIOD; //设定计数器自动重装值
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//选择时钟分频：不分频
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;   //TIM向上计数  
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
  
  TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);//使用编码器模式3
  TIM_ICStructInit(&TIM_ICInitStructure);
  TIM_ICInitStructure.TIM_ICFilter = 0;
  TIM_ICInit(TIM4, &TIM_ICInitStructure);
  
  TIM_ClearFlag(TIM4, TIM_FLAG_Update);
  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM4,0);
  TIM_Cmd(TIM4, ENABLE); 
}

/**************************************************************************
Function: Read the encoder count
Input   : The timer
Output  : Encoder value (representing speed)
函数功能：读取编码器计数
入口参数：定时器
返回  值：编码器数值(代表速度)
**************************************************************************/
int Read_Encoder(u8 TIMX)
{
 int Encoder_TIM;    
 switch(TIMX)
 {
	case 2:  Encoder_TIM= (short)TIM2 -> CNT;   TIM2 -> CNT=0;  break;
	case 3:  Encoder_TIM= (short)TIM3 -> CNT;   TIM3 -> CNT=0;  break;
	case 4:  Encoder_TIM= (short)TIM4 -> CNT;   TIM4 -> CNT=0;  break;	
	case 5:  Encoder_TIM= (short)TIM5 -> CNT;   TIM5 -> CNT=0;  break;	
	default: Encoder_TIM=0;
 }
	return Encoder_TIM;
}

