#include "InfraredTracking.h"
#include "delay.h"

//	SH (Shift Load)	LOAD 	GPIOX	低电平锁存数据，高电平准备移位
//	CL (Clock)			CLK 	GPIOY	提供时钟信号，使数据移位
//	DA (Data)				Q7 		GPIOZ	串行输出数据

// 由于static 关键字的作用，Infrared_Data(返回红外数据数组)的地址在程序运行期间是固定的

int8_t Tracking_Flag = -1;
int8_t Total_Cross_Numbers = 1;
void Infrared_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 开启时钟
    RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // SH (锁存信号) 输出
    GPIO_InitStructure.GPIO_Pin = Pin_SH;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;       // 推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // CL (时钟信号) 输出
    GPIO_InitStructure.GPIO_Pin = Pin_CL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // DA (数据输入) 输入模式
    GPIO_InitStructure.GPIO_Pin = Pin_DA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;  // 上拉输入
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}



uint8_t* Infrared_Get(void) 
{
    static uint8_t Infrared_Data[16];
    int i = 0;
    
    SH_L();  // 锁存数据
    delay_us(1);
    SH_H();  // 准备移位
    delay_us(1);

    for (i = 0; i < 16; i++) 
    {
        CL_L();  // 下降沿读取数据
        Infrared_Data[i] = DA();
        delay_us(1);
        CL_H();  // 上升沿推进下一位
        delay_us(1);
    }

    return Infrared_Data;  // 返回红外数据数组
}

// 时间滤波

// 对单个传感器进行简单时间平均滤波
uint8_t Infrared_Average(uint8_t* Infrared_Data, uint8_t index) 
{
    int i;
    uint8_t filtered_value = 0;
    uint8_t sample = 0;

    // 采样并计算平均
    for (i = 0; i < FILTER_SAMPLES; i++) 
	 {
        // 获取传感器的实时数据（使用索引定位）
        sample = Infrared_Data[index];  // 读取传感器数据
        filtered_value += sample;  // 累加采样值
        delay_us(1);  // 防止过度占用CPU
    }

    // 计算平均值
    filtered_value /= FILTER_SAMPLES;

    // 判断平均值是否大于阈值（假设大于128为黑线）
    return (filtered_value > 128) ? 1 : 0;  // 返回传感器状态，1为黑线，0为白线
}

// 获取16路传感器数据并进行时间平均滤波
uint8_t* Infrared_filter_Get(void) 
{
    static uint8_t Infrared_Data[16];  // 用数组保存16路传感器数据
    int i = 0;

    SH_L();  // 锁存数据
    delay_us(1);
    SH_H();  // 准备移位
    delay_us(1);

    // 读取每路传感器的数据，并存入数组
    for (i = 0; i < 16; i++) 
	 {
        CL_L();  // 下降沿读取数据
        Infrared_Data[i] = DA();  // 从串行中读取数据，并存储到对应位置
        delay_us(1);
        CL_H();  // 上升沿推进下一位
        delay_us(1);
    }

    // 对每一路传感器的数据进行时间平均滤波
    for (i = 0; i < 16; i++) 
		{
        // 将数组指针和索引传给滤波函数
        Infrared_Data[i] = Infrared_Average(Infrared_Data, i);  // 获取每路传感器的滤波值
    }

    return Infrared_Data;  // 返回滤波后的16路传感器数据
}

int calculate_error(uint8_t *sensor_data) 
{
	  int sensor_count = 16;
    int error = 0;
    int weighted_sum = 0;
    int sum = 0;

    // 假设传感器从0到15，0表示最左，15表示最右
    for (int i = 0; i < sensor_count; i++) 
	 {
        if (sensor_data[i] == 1) 
				{
            weighted_sum += (i - (sensor_count / 2));  // 中心为基准，越偏离，误差越大
            sum++;
        }
    }
    
    // 如果有传感器被触发，计算误差
    if (sum > 0) 
		{
        error = weighted_sum / sum;  // 计算加权平均误差
    }
    return error;
}

