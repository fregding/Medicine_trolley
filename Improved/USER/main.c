#include "headfile.h"

//手动将小车摆放在药房处（车头投影在门口区域内，面向
//病房），手持数字标号纸张由小车识别病房号，将约 200g 药品一次性装载到送
//药小车上；小车检测到药品装载完成后自动开始运送；小车根据走廊上的标识信
//息自动识别、寻径将药品送到指定病房（车头投影在门口区域内），点亮红色指
//示灯，等待卸载药品；病房处人工卸载药品后，小车自动熄灭红色指示灯，开始
//返回；小车自动返回到药房（车头投影在门口区域内，面向药房）后，点亮绿色
//指示灯。

char mystr[20];
msg_k210 k210_msg;//收到k210信息结构体

int main(void)
{ 
    //系统级初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  // 中断优先级分组
    SystemCoreClockUpdate();                        // 关键点，更新时钟变量
    
    //硬件外设初始化
    delay_init(168);    // 注意：delay_init已占用SysTick，不可再调用SysTick_Init()
    LED_Init();
    LCD_Init();
    LCD_Fill(0,0,LCD_W,LCD_H,WHITE);

    //定时器初始化

    TIM3_Init(20);     // 20ms按键扫描（优先级1.3）
    TIM7_Init(10);     // 10ms编码器采样（优先级1.2）

    //编码器硬件初始化
//    ENCODER_1_INIT();  // 初始化TIM3编码器接口
//    ENCODER_2_INIT();  // 初始化TIM4编码器接口


    Menu_page_init();
    Menu_show_current_page();
    key_list_init();   // 初始化按键队列


    delay_ms(500);     // 防止外设初始化不稳定


    while(1)
    {
        Menu_refresh();
        delay_ms(100); // 主循环周期建议与定时器周期匹配
    }
}
