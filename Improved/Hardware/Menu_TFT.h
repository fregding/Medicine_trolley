#ifndef _MENU_TFT_H
#define _MENU_TFT_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "lcd_init.h"
#include "lcd.h"
#include "string.h"
#include "encoder.h"

#define Title_offset 1
#define Data_offset 10
#define Item_offset 1

extern uint8_t Current_keyState;
extern float* adress;
extern uint8_t Active_Flag ;
extern uint8_t refresh_Flag;
extern uint8_t Cursor_position ;
extern uint8_t Page_number ;

////////////////  变量  /////////
extern float nu1,num2,num3;
extern float angle;
extern float BIN;

// 定义包含多个页面的结构体
typedef struct {
    char TitleName[20];       // 页面标题
    float* Current_Step;     // 当前步骤指针
    char (*itemNames)[5][20]; // 项目名称数组
    float** itemData;         // 项目数据指针数组
} MenuPage;

typedef struct {
    char ItemName[5][20];  // 每页最多 5 个项目名
    float* itemData[5];    // 每页最多 5 个数据项指针
} page_items;


void Turn_to_previous_page(void);
void Turn_to_next_page(void);

void Turn_to_above_item(void);
void Turn_to_below_item(void);

void item_data_increase(void);
void item_data_decrease(void);


void Menu_refresh(void);

void Menu_show_current_page(void);

void Menu_show_cursor(void);

void Menu_page_init(void);

#endif
