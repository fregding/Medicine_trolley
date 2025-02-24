#ifndef __INFRAREDTRACKING_H
#define __INFRAREDTRACKING_H

#include "stm32f4xx_gpio.h"

#define Pin_SH  GPIO_Pin_0
#define Pin_CL  GPIO_Pin_1
#define Pin_DA  GPIO_Pin_2

#define FILTER_SAMPLES 5  // 采样5次
#define SENSOR_COUNT 16  // 传感器数量

#define SH_H()  GPIO_SetBits(GPIOA, Pin_SH)  // SH 拉高
#define SH_L()  GPIO_ResetBits(GPIOA, Pin_SH) // SH 拉低
#define CL_H()  GPIO_SetBits(GPIOA, Pin_CL)  // CL 拉高
#define CL_L()  GPIO_ResetBits(GPIOA, Pin_CL) // CL 拉低
#define DA()    GPIO_ReadInputDataBit(GPIOA, Pin_DA)  // 读取 DA 数据

extern int8_t Tracking_Flag;
extern int8_t Total_Cross_Numbers;
// Tracking_Flag = -3
// Tracking_Flag = -2	 终点到位
// Tracking_Flag = -1  待机状态
// Tracking_Flag = 	0  开始巡线
// Tracking_Flag =  1  第一路口
// Tracking_Flag =  2  第二路口
// Tracking_Flag =  3  第三路口
// Tracking_Flag =  4  第四路口
// Tracking_Flag =  5  
// Tracking_Flag =  6  

void Infrared_GPIO_Init(void);

uint8_t* Infrared_Get(void);

uint8_t* Infrared_filter_Get(void); 

int calculate_error(uint8_t *sensor_data);

#endif
