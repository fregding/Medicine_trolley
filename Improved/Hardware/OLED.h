//////////////////////////////////////////////////////////////////////////////////
#ifndef __OLED_H
#define __OLED_H			  	 
#include "sys.h"
#include "stdlib.h"	    	
#define OLED_MODE 0
#define SIZE 8
#define XLevelL		0x00
#define XLevelH		0x10
#define Max_Column	128
#define Max_Row		64
#define	Brightness	0xFF 
#define X_WIDTH 	128
#define Y_WIDTH 	64	    						  
//-----------------OLED IIC端口定义----------------  					   

#define OLED_SCLK_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_13)//CLK
#define OLED_SCLK_Set() GPIO_SetBits(GPIOB,GPIO_Pin_13)

#define OLED_SDIN_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_15)//DIN
#define OLED_SDIN_Set() GPIO_SetBits(GPIOB,GPIO_Pin_15)

#define OLED_RST_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_11)//RES
#define OLED_RST_Set() GPIO_SetBits(GPIOB,GPIO_Pin_11)
 		     
#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据

#include <stdio.h>
#include <string.h>
//OLED控制用函数

void OLED_Init(void);

void OLED_Clear(void);

void OLED_Set_Pos(unsigned char x, unsigned char y);

void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 Char_Size);

void OLED_ShowString(u8 x,u8 y,char *p,u8 Char_Size);	 

void OLED_ShowFloat(unsigned char x,unsigned char y,float num,unsigned char TextSize);

void OLED_ShowInt(unsigned char x,unsigned char y,int num,unsigned char TextSize);

#endif  
