#include "stm32f4xx.h"    
#include "delay.h"
#include "Key.h"
#include "Menu_TFT.h"
#include "stm32f4xx_gpio.h"
#include "led.h"


static key_struct key_list[KEY_NUMBER];  // 按键列表
//static int key_pins[KEY_NUMBER] = {GPIO_Key_PIN_1_PIN, GPIO_Key_PIN_2_PIN, GPIO_Key_PIN_3_PIN, GPIO_Key_PIN_4_PIN};  // 对应引脚
static int key_pins[KEY_NUMBER] = {GPIO_Key_PIN_1_PIN, GPIO_Key_PIN_2_PIN};  
static uint8_t timer;  // 扫描间隔

void Key_Init(void)
{
    /* 开启 GPIOC 时钟（或者根据实际情况选择其他GPIO端口时钟） */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);  

    /* GPIO 初始化 */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  // 配置上拉电阻

    for (uint8_t i = 0; i < KEY_NUMBER; i++)
    {
        GPIO_InitStructure.GPIO_Pin = key_pins[i]; 
        GPIO_Init(GPIO_Key_PORT, &GPIO_InitStructure);  
    }
}


void key_handler(void)
{
    for (uint8_t i = 0; i < KEY_NUMBER; i++)
    {
        // 更新按键状态（低电平按下）
        key_list[i].value = !GPIO_ReadInputDataBit(GPIOC, key_pins[i]);

        // 记录上次状态
        key_list[i].last_value = key_list[i].value;

        // 松开时长按计时归零
        if (!key_list[i].value && key_list[i].state != DITHER && key_list[i].state != HOLD)
        {
            key_list[i].hold_time = 0;
        }

        // 持续按下时累加时间
        if (key_list[i].value && key_list[i].last_value)
        {
            key_list[i].hold_time += timer;
        }

        // 处于 PRE_CLICK 或 IN_CLICK 状态时，间隔累加
        if (key_list[i].state == PRE_CLICK || key_list[i].state == IN_CLICK)
        {
            key_list[i].interval_time += timer;
        }
        else
        {
            key_list[i].interval_time = 0;
        }

        // 按键状态机
        switch (key_list[i].state)
        {
        case RELEASE:
            key_list[i].is_press = false;
            key_list[i].is_hold = false;
            key_list[i].is_multi = false;
            key_list[i].click_count = 0;

            if (key_list[i].value)
            {
                key_list[i].state = DITHER;
            }
            break;

        case DITHER:
            if (key_list[i].hold_time > HOLD_THRESHOLD)
            {
                key_list[i].state = HOLD;
            }
            else if (!key_list[i].value)
            {
                if (key_list[i].hold_time > PRESS_THRESHOLD)
                {
                    key_list[i].state = PRE_CLICK;
                    key_list[i].click_count++;
                }
                else
                {
                    key_list[i].state = RELEASE;
                }
            }
            break;

        case PRE_CLICK:
            if (key_list[i].interval_time < INTERVAL_THRESHOLD)
            {
                if (key_list[i].hold_time > PRESS_THRESHOLD)
                {
                    key_list[i].state = MULTI;
                    key_list[i].click_count++;
                }
                break;
            }
            key_list[i].state = PRESS;
            break;

        case IN_CLICK:
            if (key_list[i].interval_time < INTERVAL_THRESHOLD && key_list[i].hold_time > PRESS_THRESHOLD)
            {
                key_list[i].state = MULTI;
                key_list[i].click_count++;
            }
            else
            {
                key_list[i].is_multi = true;
                key_list[i].state = RELEASE;
            }
            break;

        case PRESS:
            key_list[i].is_press = true;
            if (!key_list[i].value)
            {
                key_list[i].state = RELEASE;
            }
            break;

        case HOLD:
            if (!key_list[i].value)
            {
                key_list[i].is_hold = true;
                key_list[i].state = RELEASE;
            }
            break;

        case MULTI:
            if (!key_list[i].value)
            {
                key_list[i].state = IN_CLICK;
            }
            break;
        }
    }
    key_action();
}

void key_action(void)
{
    for (uint8_t i = 0; i < KEY_NUMBER; i++)
    {
        if (BIN != 0)
        {
            BIN--;
        }

        if (key_list[i].is_press)
        {
            switch (i)
            {
            case 0:
                Turn_to_above_item();
						LED1_OFF;
                break;
            case 1:
							item_data_increase();
						LED1_OFF;
                break;
            }
        }

        if (key_list[i].is_hold)
        {
            switch (i)
            {
            case 0:
                Active_Flag = 1;
                refresh_Flag = 1;
						    LED1_OFF;
                break;
            }
        }

        if (key_list[i].is_multi)
        {
            switch (i)
            {
            case 0:
                Turn_to_below_item();
								LED1_OFF;
                break;
            case 1:
                item_data_decrease();
								LED1_OFF;
                break;
            }
        }
    }
}

void key_list_init(void)
{
    timer = 3;

    for (uint8_t i = 0; i < KEY_NUMBER; i++)
    {
        key_list[i].value = 0;
        key_list[i].last_value = 0;
        key_list[i].hold_time = 0;
        key_list[i].interval_time = 0;
        key_list[i].click_count = 0;
        key_list[i].state = RELEASE;
        key_list[i].is_press = false;
        key_list[i].is_hold = false;
        key_list[i].is_multi = false;
    }
    Key_Init();
}
