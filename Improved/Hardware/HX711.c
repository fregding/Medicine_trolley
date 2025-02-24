#include "HX711.h" 

//		通过修改 HX711_SCK_Pulse 函数中的延时，可以调整 SCK 的脉冲频率
//		使用：weightData = HX711_Read();

// 初始化 GPIO 引脚
void HX711_Init(void) 
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // 开启 GPIOA 时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // 配置 SCK 引脚 (PA1) 为输出模式
    GPIO_InitStruct.GPIO_Pin = HX711_SCK_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 配置 DOUT 引脚 (PA0) 为输入模式
    GPIO_InitStruct.GPIO_Pin = HX711_DOUT_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
}

// 生成 SCK 时钟脉冲
void HX711_SCK_Pulse(void) 
{
    GPIO_SetBits(GPIOA, HX711_SCK_PIN);   // SCK = 1
    delay_ms(25);                           // 延时 
    GPIO_ResetBits(GPIOA, HX711_SCK_PIN); // SCK = 0
    delay_ms(25);                           // 延时 
}

// 读取 HX711 数据
uint32_t HX711_Read(void) 
{
    uint32_t data = 0;
    uint8_t i;

    // 等待 DOUT 引脚变为低电平
    while (GPIO_ReadInputDataBit(GPIOA, HX711_DOUT_PIN) == RESET);

    // 读取 24 位数据
    for (i = 0; i < 24; i++) 
		{
        HX711_SCK_Pulse();  // 生成时钟脉冲
        data = data << 1;   // 数据左移
        if (GPIO_ReadInputDataBit(GPIOA, HX711_DOUT_PIN) == SET) 
				{
            data |= 0x01;   // 如果 DOUT 为高电平，设置数据位
        }
    }

    // 生成一个额外的时钟脉冲来读取数据
    HX711_SCK_Pulse();

    return data;
}
