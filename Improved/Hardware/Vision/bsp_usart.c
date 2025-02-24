#include "bsp_usart.h"
#include "stm32f4xx.h"
#include "stm32f4xx_dma.h"


//         DMA  +  Vofa       //
void USARTx_DMA_TX_Config(DMA_Stream_TypeDef* DMA_Stream, uint32_t channel, uint32_t peripheral_addr, uint32_t memory_addr, uint16_t data_length)
{
    DMA_InitTypeDef DMA_InitStructure;
    
    // 使能 DMA2 时钟（适用于 USART1）
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE); 

    // 复位 DMA 流
    DMA_DeInit(DMA_Stream);

    // DMA 配置
    DMA_InitStructure.DMA_Channel = channel; // 选择 DMA 通道
    DMA_InitStructure.DMA_PeripheralBaseAddr = peripheral_addr; // 设置外设地址
    DMA_InitStructure.DMA_Memory0BaseAddr = memory_addr; // 设置内存地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral; // 方向：内存到外设
    DMA_InitStructure.DMA_BufferSize = data_length; // 数据长度
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; // 外设地址不递增
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; // 内存地址递增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设数据单位：字节
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; // 内存数据单位：字节
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; // 非循环模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; // 优先级高
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; // 关闭 FIFO
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    
    // 初始化 DMA 流
    DMA_Init(DMA_Stream, &DMA_InitStructure); 
}

void USARTx_DMA_SEND_DATA(uint32_t SendBuff, uint16_t len) 
{
    // 配置 DMA
    USARTx_DMA_TX_Config(DMA2_Stream7, DMA_Channel_4, (uint32_t)&USART1->DR, (uint32_t)SendBuff, len);
    
    // 使能 USART1 的 DMA 发送请求
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE); 
    
    // 使能 DMA 传输
    DMA_Cmd(DMA2_Stream7, ENABLE); 
}

uint8_t data_to_send[12];

void vofa_send_vel(float v1, float v2)
{
    uint8_t _cnt = 0;
    
    // 填充数据
    data_to_send[_cnt++] = BYTE0(v1);
    data_to_send[_cnt++] = BYTE1(v1);
    data_to_send[_cnt++] = BYTE2(v1);
    data_to_send[_cnt++] = BYTE3(v1);
    data_to_send[_cnt++] = BYTE0(v2);
    data_to_send[_cnt++] = BYTE1(v2);
    data_to_send[_cnt++] = BYTE2(v2);
    data_to_send[_cnt++] = BYTE3(v2);
    
    data_to_send[_cnt++] = 0x00;
    data_to_send[_cnt++] = 0x00;
    data_to_send[_cnt++] = 0x80;
    data_to_send[_cnt++] = 0x7F;
    
    // 调用 DMA 发送数据
    USARTx_DMA_SEND_DATA((uint32_t)data_to_send, _cnt);
}
//         DMA  +  Vofa       //

//         串口通信配置        //

void USARTx_SendData(uint8_t data)
{
    // 等待发送数据寄存器空
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    
    // 发送数据
    USART_SendData(USART1, data);
}

void USARTx_SendString(const char* str)
{
    while (*str)  // 遍历字符串直到结束
    {
        USARTx_SendData(*str++);
    }
}

void USARTx_Init(void)
{
    // 时钟使能
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // 配置 GPIO 引脚（PA9 为 TX，PA10 为 RX）
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置 GPIO 复用功能
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    // 配置 USART1 参数
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600;  // 波特率 9600
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;  // 8 数据位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;  // 1 停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;  // 无校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  // 无硬件流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  // 发送和接收模式
    USART_Init(USART1, &USART_InitStructure);

    // 启用 USART1
    USART_Cmd(USART1, ENABLE);
}

//串口中断服务函数
void USART1_IRQHandler(void)
{
//	uint8_t Rx1_Temp;
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
//		Rx1_Temp = USART_ReceiveData(USART1);
//	  recv_k210msg(Rx1_Temp);

	}
}


/////////////////////////////////////////////////////


// 初始化 USART3
void USART3_Init(void) 
{
    // 启用 USART3 时钟
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    // 启用 GPIOB 时钟
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    
    // 配置 GPIOB 的 10 号和 11 号引脚（TX 和 RX）
    GPIOB->MODER |= (0x02 << (10 * 2)) | (0x02 << (11 * 2));  // 设置为复用模式
    GPIOB->AFR[1] |= (0x07 << (4 * (10 - 8))) | (0x07 << (4 * (11 - 8)));  // 设置为 USART3

    // 配置 USART3
    USART3->BRR = 0x116;  // 设置波特率 115200 (假设系统时钟是 50 MHz)
    USART3->CR1 |= USART_CR1_UE | USART_CR1_RE | USART_CR1_TE;  // 启用 USART 和接收发送功能
    USART3->CR1 |= USART_CR1_RXNEIE;  // 启用接收中断
    NVIC_EnableIRQ(USART3_IRQn);  // 启用 USART3 中断
}

// USART3 中断服务程序
void USART3_IRQHandler(void) 
{
    if (USART3->SR & USART_SR_RXNE) 
		{  // 如果接收到数据
        char data = USART3->DR;  // 读取接收到的数据
        if (data == HEADER && rx_index == 0) 
				{
            // 如果接收到包头，开始接收
            rx_buffer[rx_index++] = data;
        } 
				
				else if (rx_index > 0) 
				{
            rx_buffer[rx_index++] = data;
            if (rx_index > 2) 
						{  // 至少有数据长度和校验和信息
                uint8_t data_length = rx_buffer[1];  // 获取数据长度
                if (rx_index == data_length + 5) 
								{  // 完整的数据包长度 = 包头 + 数据长度 + 数据 + 校验和 + 包尾
                    // 检查校验和和包尾
                    uint8_t checksum = calculate_checksum(rx_buffer + 2, data_length);
                    if (checksum == rx_buffer[data_length + 2] && rx_buffer[rx_index - 1] == FOOTER) 
										{
                        // 校验成功，处理数据
                        process_data(rx_buffer + 2, data_length);
                    }
                    rx_index = 0;  // 重置索引
                }
            }
        }
    }
}
