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

// 电机初始化  TIM8   arr = 8400 psc = 8399
void motor_init(u32 arr, u32 psc) 
{
    // 配置方向控制引脚（AIN1, AIN2, BIN1, BIN2）和使能引脚（STBY）
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_InitStructure.GPIO_Pin = AIN1 | AIN2 | BIN1 | BIN2 | STBY;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;        // 输出模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;   // 速度100MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;       // 推挽输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;        // 下拉
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // STBY 被设置为高电平，激活 TB6612 驱动器。
    // AIN1, AIN2, BIN1, BIN2 被初始化为低电平，意味着电机的方向尚未确定，待后续控制。
    GPIO_SetBits(GPIOE, STBY);
    GPIO_ResetBits(GPIOE, AIN1 | AIN2 | BIN1 | BIN2);

    // PWM 信号设置定时器
    // 使能 GPIOB 时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    // 配置 TIM8_CH1 引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;         // 复用功能
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;      // 无上下拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 配置 TIM8_CH2 引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 将引脚复用为 TIM8
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_TIM8);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_TIM8);

    // 使能 TIM8 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);

    // 初始化定时器
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = arr;              // ARR
    TIM_TimeBaseStructure.TIM_Prescaler = psc;           // PSC
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);

    // 配置通道 1
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;                   // 初始占空比为 0
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM8, &TIM_OCInitStructure);

    // 配置通道 2
    TIM_OCInitStructure.TIM_Pulse = 0;                   // 初始占空比为 0
    TIM_OC2Init(TIM8, &TIM_OCInitStructure);

    // 使能 TIM8 的主输出
    TIM_CtrlPWMOutputs(TIM8, ENABLE);

    // 使能定时器
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




void set_PWM_output(TIM_TypeDef *TIMx, uint8_t channel, int16_t pwmValue)
{
    uint32_t maxPWM = TIMx->ARR; // 获取 PWM 最大值
    
    if (pwmValue > maxPWM) pwmValue = maxPWM;
    if (pwmValue < -maxPWM) pwmValue = -maxPWM;

    uint32_t ccr_value = (pwmValue + maxPWM) * maxPWM / (2 * maxPWM);

    switch (channel)
    {
        case 1: TIMx->CCR1 = ccr_value; break;
        case 2: TIMx->CCR2 = ccr_value; break;
        case 3: TIMx->CCR3 = ccr_value; break;
        case 4: TIMx->CCR4 = ccr_value; break;
        default: break;
    }
}

int constrain(int x, int min, int max) 
{
    if (x < min) 
		{
        return min;
    }
    else if (x > max) 
		{
        return max;
    }
    else 
		{
        return x;
    }
}


// PWM 输出
void tb6612_out(int pwm_l, int pwm_r) 
{
	GPIO_SetBits(GPIOE, STBY);
	if(pwm_l>0)
	{
		GPIO_SetBits(GPIOE, AIN1);
		GPIO_ResetBits(GPIOE,AIN2);
	}
	else
	{
		pwm_l = -pwm_l;
		GPIO_SetBits(GPIOE, AIN2);
		GPIO_ResetBits(GPIOE,AIN1);		
	}
	
	if(pwm_r)
	{
		GPIO_SetBits(GPIOE, BIN1);
		GPIO_ResetBits(GPIOE,BIN2);		
	}
	else
	{
		pwm_l = -pwm_l;
		GPIO_SetBits(GPIOE, BIN2);
		GPIO_ResetBits(GPIOE,BIN1);				
	}
	
    // 限制 PWM 在合理范围内（假设 PWM 最大值为 MotorPWM_maxValue)
    pwm_l = constrain(pwm_l, 0, MotorPWM_maxValue);
    pwm_r = constrain(pwm_r, 0, MotorPWM_maxValue);	
     // **映射到 PWM 输出**
    set_PWM_output(TIM8, 1, pwm_l);   // 左轮 PWM 输出
    set_PWM_output(TIM8, 2, pwm_r);  // 右轮 PWM 输出  
}


void updateMotorControl(float targetSpeed, float currentSpeed,int Senser_error) 
{
    // 计算 PID 速度控制量
    float incrementalOutput = Incremental_Control(&motorPWM_pid, currentSpeed, targetSpeed);
    
    // 计算传感器误差
    //int Senser_error = calculate_error(sensor_data);
    
    // 计算直行修正（基于误差）
    float straightCorrection = Senser_error * 0.001;  // Kp 是控制误差的比例系数

    // 计算左/右电机 PWM 输出
    int pwm_l = basePWM + incrementalOutput - straightCorrection;
    int pwm_r = basePWM + incrementalOutput + straightCorrection;

	  // 传递修正后的 PWM 值
    Real_leftPWM = pwm_l;
    Real_rightPWM = pwm_r;

}

