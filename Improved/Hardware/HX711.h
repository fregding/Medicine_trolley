#ifndef __HX711_H
#define __HX711_H
#include "sys.h"
#include "delay.h"

//	PA0 配置为输入模式，用于读取 HX711 的数据（DT）。
//	PA1 配置为输出模式，用于生成时钟信号（SCK）。
//	等待 DOUT 引脚为低电平，表示数据准备好。读取 24 位数据，并通过 SCK 时钟脉冲逐位读取数据。
//	最后生成一个额外的时钟脉冲来稳定数据。

// 定义 HX711 的引脚
#define HX711_DOUT_PIN GPIO_Pin_0  // 数据引脚
#define HX711_SCK_PIN GPIO_Pin_1   // 时钟引脚
#define HX711_GPIO_PORT GPIOA      // GPIOA

void HX711_Init(void);
uint32_t HX711_Read(void);
 				    
#endif
