#ifndef __BSP_USART_H
#define __BSP_USART_H

#include "headfile.h"

#define BYTE0(dwTemp)       ( *( (char *)(&dwTemp)    ) )
#define BYTE1(dwTemp)       ( *( (char *)(&dwTemp) + 1) )
#define BYTE2(dwTemp)       ( *( (char *)(&dwTemp) + 2) ) 
#define BYTE3(dwTemp)       ( *( (char *)(&dwTemp) + 3) )

void USARTx_Init(void);
void USARTx_SendData(uint8_t data);
void USARTx_SendString(const char* str);
void vofa_send_vel(float v1, float v2);

void USART3_Init(void);

#endif
