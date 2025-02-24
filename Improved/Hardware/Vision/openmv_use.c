#include "headfile.h"

//	| Header | Data Length | Data | Checksum | Footer |

char rx_buffer[RX_BUFFER_SIZE];  // 用于存储接收到的数据
uint8_t rx_index = 0;  // 当前接收的数据索引

// 校验和计算函数
uint8_t calculate_checksum(char* data, uint8_t length) 
{
    uint8_t checksum = 0;
    for (uint8_t i = 0; i < length; i++) 
		{
        checksum ^= data[i];
    }
    return checksum;
}

// 数据处理函数
void process_data(char* data, uint8_t length) 
{
    // 假设数据是一个单独的数字
    int digit = data[0];  // 获取第一个字节的数据
    if (digit == 5) 
		{
					// 例如
    } 
		else 
		{
         
    }
}
