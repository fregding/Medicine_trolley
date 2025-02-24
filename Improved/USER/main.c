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
	/*SYS INIT		NVIC_INIT	SYS--168M	TIM--84M*/	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  //初始化延时函数
	LED_Init();
	LCD_Init();//LCD初始化
	LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
	
	RCC_ClocksTypeDef get_rcc_clock;    //获取系统时钟状态
	RCC_GetClocksFreq(&get_rcc_clock);	

//	delay_ms(200);
//	sprintf(mystr,"%d",get_rcc_clock.SYSCLK_Frequency);
//	LCD_ShowString(10,0,mystr,BLACK,WHITE,16,0);
	
	Menu_page_init();
	Menu_show_current_page();
	
	delay_ms(500);//防止接了匿名数传卡死程序
	
	key_list_init();					//初始化按键

//	ENCODER_1_INIT();
//  ENCODER_2_INIT();
//	
//	
//	SysTick_Init(10); //10ms 
   EXTIX_Init();

 	while(1)
	{
		Menu_refresh();
	  delay_ms(100);
	} 	
}


