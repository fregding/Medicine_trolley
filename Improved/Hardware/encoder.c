#include "encoder.h"
#include <math.h>
#include "TimerX.h"
// 使用 TIMx 触发 定时中断，例如 10ms 读取一次编码器值


float encoder_left = 0.f;
float encoder_right = 0.f;
float wheel_speed_left = 0, wheel_speed_right = 0; 

// TIM3编码器初始化（优化版）
void ENCODER_1_INIT(void)
{
    // 1. 使能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // 2. 配置GPIO（复用为定时器功能）
    GPIO_InitTypeDef GPIO_InitStruct = {
        .GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7,
        .GPIO_Mode = GPIO_Mode_AF,
        .GPIO_Speed = GPIO_Speed_100MHz,
        .GPIO_OType = GPIO_OType_OD,      // 改为开漏（或保持浮空输入）
        .GPIO_PuPd = GPIO_PuPd_UP         // 上拉确保信号稳定
    };
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_TIM3);

    // 3. 配置定时器编码器模式
    TIM_TimeBaseInitTypeDef TIM_BaseInitStruct = {
        .TIM_Prescaler = 0,
        .TIM_Period = 0xFFFF,            // 16位最大值
        .TIM_ClockDivision = TIM_CKD_DIV1,
        .TIM_CounterMode = TIM_CounterMode_Up // 编码器模式会自动控制方向
    };
    TIM_TimeBaseInit(TIM3, &TIM_BaseInitStruct);

    // 4. 配置编码器接口（关键修正）
    TIM_EncoderInterfaceConfig(
        TIM3, 
        TIM_EncoderMode_TI12,            // 双边沿触发（模式3）
        TIM_ICPolarity_Rising,           // CH1极性
        TIM_ICPolarity_Rising            // CH2极性
    );

    // 5. 启用输入滤波（抑制噪声）
    TIM_ICInitTypeDef TIM_ICInitStruct;
    TIM_ICStructInit(&TIM_ICInitStruct);
    TIM_ICInitStruct.TIM_Channel = TIM_Channel_1 | TIM_Channel_2;
    TIM_ICInitStruct.TIM_ICFilter = 0x8; // 设置滤波（推荐值：8-15）
    TIM_ICInit(TIM3, &TIM_ICInitStruct); // 仅配置滤波，不覆盖编码器模式参数

    // 6. 启动定时器
    TIM_Cmd(TIM3, ENABLE);
}

// TIM4编码器初始化（右轮，优化版）
void ENCODER_2_INIT(void)
{
    // 1. 使能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    // 2. 配置GPIO（复用为定时器功能）
    GPIO_InitTypeDef GPIO_InitStruct = {
        .GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7,
        .GPIO_Mode = GPIO_Mode_AF,
        .GPIO_Speed = GPIO_Speed_100MHz,
        .GPIO_OType = GPIO_OType_OD,      // 开漏模式，避免信号冲突
        .GPIO_PuPd = GPIO_PuPd_UP         // 上拉确保信号稳定
    };
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    // 确认GPIO复用映射到TIM4
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);

    // 3. 配置定时器编码器模式
    TIM_TimeBaseInitTypeDef TIM_BaseInitStruct = {
        .TIM_Prescaler = 0,              // 无分频
        .TIM_Period = 0xFFFF,            // 16位计数器最大值
        .TIM_ClockDivision = TIM_CKD_DIV1,
        .TIM_CounterMode = TIM_CounterMode_Up // 编码器模式会自动控制方向
    };
    TIM_TimeBaseInit(TIM4, &TIM_BaseInitStruct);

    // 4. 配置编码器接口（关键修正）
    TIM_EncoderInterfaceConfig(
        TIM4, 
        TIM_EncoderMode_TI12,            // 双边沿触发（模式3）
        TIM_ICPolarity_Rising,           // CH1极性
        TIM_ICPolarity_Rising            // CH2极性
    );

    // 5. 启用输入滤波（抑制噪声）
    TIM_ICInitTypeDef TIM_ICInitStruct;
    TIM_ICStructInit(&TIM_ICInitStruct);
    TIM_ICInitStruct.TIM_Channel = TIM_Channel_1 | TIM_Channel_2;
    TIM_ICInitStruct.TIM_ICFilter = 0x8; // 滤波参数（推荐值8-15）
    TIM_ICInit(TIM4, &TIM_ICInitStruct); // 仅配置滤波，不覆盖编码器模式参数

    // 6. 启动定时器
    TIM_Cmd(TIM4, ENABLE);
}


// 读取编码器计数值（带方向）
int16_t Read_Encoder(TIM_TypeDef *TIMx) 
{
    int16_t cnt;
    __disable_irq();          // 关闭全局中断
    cnt = (int16_t)(TIMx->CNT);
    __enable_irq();           // 开启全局中断
    return cnt;
}

// 全局变量记录上一次的CNT值
static int16_t prevLeftCNT = 0, prevRightCNT = 0;

void computeSpeed(int16_t LeftCNT, int16_t RightCNT) 
{
    // 计算脉冲差值（考虑CNT溢出）
    int16_t deltaLeft = (int16_t)(LeftCNT - prevLeftCNT);
    int16_t deltaRight = (int16_t)(RightCNT - prevRightCNT);
    
    // 更新历史值
    prevLeftCNT = LeftCNT;
    prevRightCNT = RightCNT;

    // 计算实际速度（需替换为实际参数）
    wheel_speed_left = (deltaLeft * 10);
    wheel_speed_right = (deltaRight * 10);

}

//// 编码器参数
//#define ENCODER_RESOLUTION   50.f  //定义：每个编码器脉冲对应的实际位移（单位：米/脉冲）。
//#define SAMPLE_TIME_MS       20
//#define MOVING_AVG_SIZE      3     //（滑动平均窗口大小）
//#define SPEED_DEADZONE       0.02f  // 定义：速度归零的阈值（单位：米/秒），绝对值小于此值视为静止。
//#define ZERO_TIMEOUT_MS      300

//typedef struct {
//    float q, r, x, p, k;
//} KalmanFilter;

//static uint16_t prevLeftCNT = 0, prevRightCNT = 0;
//static KalmanFilter kalman_left, kalman_right;
//static float speed_history_left[MOVING_AVG_SIZE] = {0};
//static float speed_history_right[MOVING_AVG_SIZE] = {0};
//static uint8_t history_index = 0;

//void kalmanInit(KalmanFilter *filter, float q, float r, float x, float p) {
//    filter->q = q;
//    filter->r = r;
//    filter->x = x;
//    filter->p = p;
//    filter->k = 0;
//}

//float kalmanUpdate(KalmanFilter *filter, float measurement) {
//    filter->p += filter->q;
//    filter->k = filter->p / (filter->p + filter->r);
//    filter->x += filter->k * (measurement - filter->x);
//    filter->p *= (1 - filter->k);
//    return filter->x;
//}

//void SpeedSensor_Init(void) {
//    kalmanInit(&kalman_left, 0.05f, 0.5f, 0.0f, 1.0f);
//    kalmanInit(&kalman_right, 0.05f, 0.5f, 0.0f, 1.0f);
//}

//void computeSpeed(uint16_t LeftCNT, uint16_t RightCNT) {
//    // 1. 计算脉冲差值（自动处理溢出）
//    uint16_t deltaLeft = LeftCNT - prevLeftCNT;
//    uint16_t deltaRight = RightCNT - prevRightCNT;
//    prevLeftCNT = LeftCNT;
//    prevRightCNT = RightCNT;

//    // 2. 计算原始速度
//    float raw_left = ((int16_t)deltaLeft * ENCODER_RESOLUTION) / SAMPLE_TIME_MS * 1000;
//    float raw_right = ((int16_t)deltaRight * ENCODER_RESOLUTION) / SAMPLE_TIME_MS * 1000;

//    // 3. 卡尔曼滤波
//    float filtered_left = kalmanUpdate(&kalman_left, raw_left);
//    float filtered_right = kalmanUpdate(&kalman_right, raw_right);

//    // 4. 滑动平均滤波
//    speed_history_left[history_index] = filtered_left;
//    speed_history_right[history_index] = filtered_right;
//    history_index = (history_index + 1) % MOVING_AVG_SIZE;

//    float avg_left = 0, avg_right = 0;
//    for (uint8_t i = 0; i < MOVING_AVG_SIZE; i++) {
//        avg_left += speed_history_left[i];
//        avg_right += speed_history_right[i];
//    }
//    avg_left /= MOVING_AVG_SIZE;
//    avg_right /= MOVING_AVG_SIZE;

//    // 5. 动态死区处理
//    static uint32_t last_left_time = 0, last_right_time = 0;
//    
//    if (fabs(avg_left) < SPEED_DEADZONE) {
//        if (getCurrentTime() - last_left_time > ZERO_TIMEOUT_MS) {
//            avg_left = 0;
//        }
//    } else {
//        last_left_time = getCurrentTime();
//    }

//    if (fabs(avg_right) < SPEED_DEADZONE) {
//        if (getCurrentTime() - last_right_time > ZERO_TIMEOUT_MS) {
//            avg_right = 0;
//        }
//    } else {
//        last_right_time = getCurrentTime();
//    }

//    // 6. 输出最终速度
//    wheel_speed_left = avg_left;
//    wheel_speed_right = avg_right;
//}
