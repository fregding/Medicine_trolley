#include "Motor.h" 
#include "stm32f4xx_gpio.h"
#include <math.h>
#include "PID.h"
#include "encoder.h"

float Real_leftPWM = 0;
float Real_rightPWM = 0;
// 高级定时器（支持死区控制等）：TIM1、TIM8
// 通道 1：GPIOB_PIN_13（TIM8_CH1）
// 通道 2：GPIOB_PIN_14（TIM8_CH2）

// 电机初始化  TIM8
#define AIN1 GPIO_Pin_7
#define AIN2 GPIO_Pin_8
#define BIN1 GPIO_Pin_9
#define BIN2 GPIO_Pin_10
#define STBY GPIO_Pin_11

// 电机初始化 TIM8（PWM频率10kHz，死区时间200ns）
void motor_init(void) 
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct = {0};
    TIM_OCInitTypeDef TIM_OCInitStruct = {0};
    TIM_BDTRInitTypeDef TIM_BDTRInitStruct = {0};

    // 1. 配置方向控制引脚（PE7~PE11）
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;  // 取消下拉
    GPIO_Init(GPIOE, &GPIO_InitStruct);

    // 初始化电机控制引脚
    GPIO_ResetBits(GPIOE, GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10); // 方向引脚置低
    GPIO_SetBits(GPIOE, GPIO_Pin_11); // STBY使能

    // 2. 配置PWM引脚（PB13: TIM8_CH1, PB14: TIM8_CH2）
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_TIM8);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_TIM8);

    // 3. 配置TIM8时基（PWM频率10kHz）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
    TIM_TimeBaseStruct.TIM_Prescaler = 84 - 1;      // 84MHz / 84 = 1MHz
    TIM_TimeBaseStruct.TIM_Period = 100 - 1;        // 1MHz / 100 = 10kHz
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStruct);

    // 4. 配置PWM通道（模式1，高电平有效）
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 0; // 初始占空比0%
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM8, &TIM_OCInitStruct);
    TIM_OC2Init(TIM8, &TIM_OCInitStruct);

    // 5. 配置死区时间（200ns）
    TIM_BDTRInitStruct.TIM_DeadTime = 0x18;         // 84MHz下，1周期≈11.9ns，0x18≈200ns
    TIM_BDTRInitStruct.TIM_Break = TIM_Break_Enable;
    TIM_BDTRInitStruct.TIM_BreakPolarity = TIM_BreakPolarity_High;
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




// PWM输出函数（解决占空比计算逻辑问题）
void set_PWM_output(TIM_TypeDef *TIMx, uint8_t channel, int16_t pwmValue) 
{
    uint32_t maxPWM = TIMx->ARR; 
    // 限制PWM范围并处理方向
    if (pwmValue > maxPWM) pwmValue = maxPWM;
    if (pwmValue < -maxPWM) pwmValue = -maxPWM;
    
    // 将pwmValue映射到0~ARR范围（占空比计算）
    uint32_t ccr_value = (abs(pwmValue) * maxPWM) / maxPWM;  // 修正计算逻辑

    switch (channel) 
		{
        case 1: TIMx->CCR1 = ccr_value; break;
        case 2: TIMx->CCR2 = ccr_value; break;
        case 3: TIMx->CCR3 = ccr_value; break;
        case 4: TIMx->CCR4 = ccr_value; break;
        // 其他通道同理
    }
}

// TB6612驱动函数
void tb6612_out(int pwm_l, int pwm_r) 
{
    GPIO_SetBits(GPIOE, STBY);
    
    // 左电机方向控制
    if (pwm_l > 0) 
		{
        GPIO_SetBits(GPIOE, AIN1);
        GPIO_ResetBits(GPIOE, AIN2);
    } 
		else 
		{
        GPIO_SetBits(GPIOE, AIN2);
        GPIO_ResetBits(GPIOE, AIN1);
        pwm_l = -pwm_l; // 保持正值用于占空比计算
    }
    
    // 右电机方向控制
    if (pwm_r > 0) 
		{  
        GPIO_SetBits(GPIOE, BIN1);
        GPIO_ResetBits(GPIOE, BIN2);
    } 
		else 
		{
        GPIO_SetBits(GPIOE, BIN2);
        GPIO_ResetBits(GPIOE, BIN1);
        pwm_r = -pwm_r;
    }
    
//    set_PWM_output(TIM8, 1, pwm_l);
//    set_PWM_output(TIM8, 2, pwm_r);
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


