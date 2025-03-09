#ifndef __MOTOR_H
#define __MOTOR_H

#include "sys.h"
#include "PID.h"

#define Tb6612_PORT GPIOE
#define Tb6612_RCC RCC_AHB1Periph_GPIOE
#define AIN1 GPIO_Pin_7
#define AIN2 GPIO_Pin_8
#define BIN1 GPIO_Pin_12
#define BIN2 GPIO_Pin_14
#define STBY GPIO_Pin_10
// PWMA -> PC6  TIM8_ch1
// PWMB -> PC7  TIM8_ch2

#define basePWM 2000

extern float Real_leftPWM;
extern float Real_rightPWM;

extern struct PID motorPWM_pid;

void motor_init(void);

void set_PWM_output(TIM_TypeDef *TIMx, uint8_t channel, int16_t pwmValue);

void tb6612_out(int pwm_l, int pwm_r);

void updateMotorControl(float targetSpeed,float currentSpeed);

#endif
