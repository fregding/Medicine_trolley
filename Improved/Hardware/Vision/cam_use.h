#ifndef _CAM_USE_H_
#define _CAM_USE_H_
#include "sys.h"

#define RX_BUFFER_SIZE 20
#define HEADER (char)0xAA
#define FOOTER (char)0x55

typedef struct msg_buf
{
	u16 x; 
	u16 y; 
	u16 w; 
	u16 h; 
	u16 id; 
	u8 class_n;//例程号
	u8 msg_msg[20]; //数据包
}msg_k210;

extern char rx_buffer[RX_BUFFER_SIZE];  // 用于存储接收到的数据
extern uint8_t rx_index;  // 当前接收的数据索引

/********k210************/
void recv_k210msg(uint8_t recv_msg);
void deal_recvmsg(void);
void deal_data(u8 egnum);

extern msg_k210 k210_msg;//k210结构体

/********openmv************/
uint8_t calculate_checksum(char* data, uint8_t length);
void process_data(char* data, uint8_t length);

#endif
