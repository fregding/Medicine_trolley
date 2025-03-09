#ifndef __HEADFILE_H
#define __HEADFILE_H

//   基础系统 
#include "stm32f4xx.h" 
#include "sys.h"
#include "led.h"
#include "delay.h"
#include "exti.h"
#include "TimerX.h"
#include "bsp_usart.h"
#include <string.h> 
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

//   操作页面
#include "Key.h"
#include "lcd_init.h"
#include "lcd.h"
#include "Menu_TFT.h"

//   算法部分
#include "PID.h"
#include "mpu6050.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"

//   移动控制
#include "Motor.h"
#include "encoder.h"
#include "InfraredTracking.h"

//	 其他硬件
#include "HX711.h"

//   视觉部分
#include "cam_use.h"
#include "bsp_usart.h"

	 
#endif
