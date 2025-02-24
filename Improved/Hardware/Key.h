#ifndef __KEY_H
#define __KEY_H

#include <stdint.h>
#include <stdbool.h>

void Key_Init(void);
uint8_t Key_GetNum(void);

#define KEY_NUMBER 2
#define PRESS_THRESHOLD 10     
#define HOLD_THRESHOLD 300     
#define INTERVAL_THRESHOLD 150 

#define GPIO_Key_PORT GPIOC
#define GPIO_Key_PIN_1_PIN GPIO_Pin_6
#define GPIO_Key_PIN_2_PIN GPIO_Pin_7
#define GPIO_Key_PIN_3_PIN GPIO_Pin_8
#define GPIO_Key_PIN_4_PIN GPIO_Pin_9

//================================================//



typedef struct
{
    uint8_t value, last_value;         
    uint32_t hold_time, interval_time; 
    uint32_t click_count;              

    enum
    {
        RELEASE,   
        DITHER,    
        PRE_CLICK, 
        IN_CLICK,  
        PRESS,     
        HOLD,      
        MULTI,     
    } state;     

    bool is_press; 
    bool is_hold;  
    bool is_multi; 
} key_struct;

uint8_t Key_GetNum(void);
void key_list_init(void);
void key_handler(void);
void key_action(void);

#endif
