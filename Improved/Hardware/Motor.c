#include "Motor.h" 
#include "stm32f4xx_gpio.h"
#include <math.h>
#include "PID.h"
#include "encoder.h"

float Real_leftPWM = 0;
float Real_rightPWM = 0;
// 高级定时器（支持死区控制等）：TIM1、TIM8


void motor_init(void) 
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct = {0};
    TIM_OCInitTypeDef TIM_OCInitStruct = {0};
    TIM_BDTRInitTypeDef TIM_BDTRInitStruct = {0};

    // 1. 配置方向控制引脚（PE7~PE11）
    RCC_AHB1PeriphClockCmd(Tb6612_RCC, ENABLE);
    GPIO_InitStruct.GPIO_Pin = AIN1 | AIN2 | BIN1 | BIN2 | STBY;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(Tb6612_PORT, &GPIO_InitStruct);

    // 初始化电机控制引脚
    GPIO_ResetBits(Tb6612_PORT, AIN1 | AIN2 | BIN1 | BIN2); 
    GPIO_SetBits(Tb6612_PORT, STBY);

    // 2. 配置PWM引脚（TIM8_CH1/CH2对应PC6/PC7）
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &GPIO_InitStruct); 
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_TIM8);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_TIM8);

    // 3. 配置TIM8时基（核心修改：ARR调整为5000）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
    
    // 参数计算说明：
    // 目标PWM频率 = 10kHz
    // TIM_CLK = 84MHz (APB2时钟)
    // 计算公式：PWM频率 = TIM_CLK / [(PSC+1) * (ARR+1)]
    // 设ARR=5000-1，则 PSC = (TIM_CLK / (PWM频率 * ARR)) - 1 
    // PSC = 84,000,000 / (10,000 * 5000) - 1 = 1.68 → 取整为1
    TIM_TimeBaseStruct.TIM_Prescaler = 1;           // 新预分频值
    TIM_TimeBaseStruct.TIM_Period = 5000 - 1;       // ARR调整为5000-1
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStruct);

    // 4. 配置PWM通道（模式1，高电平有效）
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 0;                 // 初始占空比0%
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    
    TIM_OC1Init(TIM8, &TIM_OCInitStruct);
    TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);
    
    TIM_OC2Init(TIM8, &TIM_OCInitStruct);
    TIM_OC2PreloadConfig(TIM8, TIM_OCPreload_Enable);

    // 5. 调整死区时间（需重新计算）
    // 新时钟参数：TIM_CLK = 84MHz / (1+1) = 42MHz
    // Tdts = 1/(2*42MHz) ≈ 11.9ns
    // 死区时间 = 200ns / 11.9ns ≈ 16.8 → 取整17 (0x11)
    TIM_BDTRInitStruct.TIM_DeadTime = 0x11;         // 更新死区值
    TIM_BDTRInitStruct.TIM_Break = TIM_Break_Disable;
    TIM_BDTRInitStruct.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;
    TIM_BDTRConfig(TIM8, &TIM_BDTRInitStruct);

    // 6. 启动定时器
    TIM_CtrlPWMOutputs(TIM8, ENABLE);
    TIM_Cmd(TIM8, ENABLE);
}
// 控速环
struct PID motorPWM_pid = {0.5, 0.1, 0.05, 0, 0, 0, 0};  // 设定初始 PID 参数

float computeStraightCorrection(void) 
{
    float error = wheel_speed_left - wheel_speed_right; // 计算左右轮速度误差
    float Kc = 0.2;  // 直行修正增益，调整范围 0.1 ~ 1.0 适配不同小车
    float correction = Kc * error; // 计算修正量

    // 限制修正量，防止过度修正
    if (correction > 50) correction = 50;  
    if (correction < -50) correction = -50;

    return correction; // 返回修正值
}




/**
  * @brief  设置PWM输出值，支持正负方向控制
  * @param  TIMx: 定时器指针（如TIM8）
  * @param  channel: PWM通道（1-4）
  * @param  pwmValue: PWM值（范围：-ARR ~ +ARR，负值表示反向）
  */
void set_PWM_output(TIM_TypeDef *TIMx, uint8_t channel, int16_t pwmValue) 
{
    // 参数校验
    if (channel < 1 || channel > 4) return;
    
    uint32_t maxPWM = TIMx->ARR;  // ARR为周期值（实际计数值=ARR+1）
    int32_t clampedValue = pwmValue;
    
    // 限制PWM范围 [-maxPWM, +maxPWM]
    clampedValue = (clampedValue > (int32_t)maxPWM) ? maxPWM : clampedValue;
    clampedValue = (clampedValue < -(int32_t)maxPWM) ? -maxPWM : clampedValue;
    
    // 计算实际占空比（映射到0~maxPWM）
    uint32_t ccr_value = (clampedValue >= 0) ? (uint32_t)clampedValue : (uint32_t)(-clampedValue);
    
    
    // 更新CCR寄存器
    switch (channel) 
		{
        case 1: TIMx->CCR1 = ccr_value; break;
        case 2: TIMx->CCR2 = ccr_value; break;
        case 3: TIMx->CCR3 = ccr_value; break;
        case 4: TIMx->CCR4 = ccr_value; break;
        default: break;
    }
}

// TB6612驱动函数
void tb6612_out(int pwm_l, int pwm_r) 
{
    GPIO_SetBits(Tb6612_PORT, STBY);
    
    // 左电机方向控制
    if (pwm_l < 0) 
		{
        GPIO_SetBits(Tb6612_PORT, AIN1);
        GPIO_ResetBits(Tb6612_PORT, AIN2);
    } 
		else 
		{
        GPIO_SetBits(Tb6612_PORT, AIN2);
        GPIO_ResetBits(Tb6612_PORT, AIN1);
        pwm_l = -pwm_l; // 保持正值用于占空比计算
    }
    
    // 右电机方向控制
    if (pwm_r < 0) 
		{  
        GPIO_SetBits(Tb6612_PORT, BIN1);
        GPIO_ResetBits(Tb6612_PORT, BIN2);
    } 
		else 
		{
        GPIO_SetBits(Tb6612_PORT, BIN2);
        GPIO_ResetBits(Tb6612_PORT, BIN1);
        pwm_r = -pwm_r;
    }
    
    set_PWM_output(TIM8, 1, pwm_l);
    set_PWM_output(TIM8, 2, pwm_r);
}

void updateMotorControl(float targetSpeed,float currentSpeed) //wheel_speed_left
{
    // 计算 PID 速度控制量
    float incrementalOutput = Incremental_Control(&motorPWM_pid, currentSpeed, targetSpeed);
    
    // 计算直行修正
    float straightCorrection = computeStraightCorrection();

    // 计算左/右电机 PWM 输出
    Real_leftPWM = basePWM + incrementalOutput - straightCorrection;
    Real_rightPWM = basePWM + incrementalOutput + straightCorrection;

}


