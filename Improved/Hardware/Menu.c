#include "Menu.h"
#include "OLED.h"
#include "string.h"

// 四个按钮 8个动作
// key1 short -> Turn to pre page        long -> Turn to next page
// key2 short -> Turn to above item      long -> Turn to below item
//               Cursor_position              Cursor_position
// key3 short -> item data increase      long -> item data decrease
// key4 short -> Set active item         long -> Release item

uint8_t Current_keyState = 0;  // 0 -> 浮空 1 -> 单击 2 -> 双击 3 -> 长按
uint8_t Active_Flag = 0;
uint8_t Cursor_position = 2;
uint8_t refresh_Flag = 0;
uint8_t Page_number = 0;  // 当前页面编号，0 表示第一页
uint8_t Item_offset = 10;

float num1 = 1.0, num2 = 0.0, num3 = 0.0;
float angle = 30;
float* adress = &num1;
float STEP = 10;
float BIN = 0;


MenuPage Pages[3];  // 定义三个页面

page_items Page1_Items = {
    {"a", "P", "I", "D", "Angle"},  // 项目名称
    {&BIN, &num1, &num1, &num2, &num3}  // 项目地址
};

page_items Page2_Items = {
    {"Speed", "Angle", "Position", "Time", "Error"},
    {&num1, &num2, &num3, &angle, &BIN}
};

page_items Page3_Items = {
    {"Current", "Voltage", "Power", "Temp", "Status"},
    {&num1, &num2, &num3, &angle, &BIN}
};


// 每页的初始化函数
void Pages_init(void)
{
    // 初始化页面 1 和 2
    Pages[0].Current_Step = &STEP;
    sprintf(Pages[0].TitleName, "Page 1");
    Pages[0].itemNames = &Page1_Items.ItemName;
    Pages[0].itemData = Page1_Items.itemData;

    Pages[1].Current_Step = &STEP;
    sprintf(Pages[1].TitleName, "Page 2");
    Pages[1].itemNames = &Page2_Items.ItemName;
    Pages[1].itemData = Page2_Items.itemData;
    
    Pages[2].Current_Step = &STEP;
    sprintf(Pages[2].TitleName, "Page 3");
    Pages[2].itemNames = &Page3_Items.ItemName;
    Pages[2].itemData = Page3_Items.itemData;
    
    // 页面的其他属性初始化
    Menu_show_current_page();
}

void Menu_show_current_page(void)
{
    OLED_Clear(); // 每次刷新页面，清空屏幕

    OLED_ShowString(Title_offset, 0, Pages[Page_number].TitleName, 1);
    OLED_ShowString(Item_offset, 2, "Step =", 1);
    OLED_ShowFloat(Item_offset + 40, 2, *Pages[Page_number].Current_Step, 1);

    for (int i = 0; i < 5; i++) {
        if (Pages[Page_number].itemNames[0][i][0] != '\0') {  // 判断项目是否有效
            OLED_ShowString(Item_offset, 3 + i, Pages[Page_number].itemNames[0][i], 1);
            OLED_ShowFloat(Data_offset, 3 + i, *Pages[Page_number].itemData[i], 1);
        }
    }
}

void judge_active(void)
{
    if (Cursor_position < 2 || Cursor_position > 7) return; // 保护Cursor_position不越界

    if (Active_Flag) {
        OLED_ShowChar(0, Cursor_position, '&', 1);
    } else {
        OLED_ShowChar(0, Cursor_position, '>', 1);
    }

    // 动态决定当前页面的项目地址
    switch (Cursor_position) {
        case 2: adress = Pages[Page_number].itemData[0]; break;
        case 3: adress = Pages[Page_number].itemData[1]; break;
        case 4: adress = Pages[Page_number].itemData[2]; break;
        case 5: adress = Pages[Page_number].itemData[3]; break;
        case 6: adress = Pages[Page_number].itemData[4]; break;
    }
}

void Menu_show_cursor(void)
{
    // 清除之前的光标
    for (int i = 0; i < 8; i++)
        OLED_ShowChar(0, i, ' ', 1);

    if (Cursor_position > 7) Cursor_position = 7;
    if (Cursor_position < 2) Cursor_position = 2;

    judge_active();
}

void Turn_to_next_page(void)
{
    // 循环翻页，从第一页到最后一页，再回到第一页
    Page_number = (Page_number + 1) % 3;
    refresh_Flag = 1;
}

void Turn_to_previous_page(void)
{
    // 循环翻页，从第一页回到最后一页
    Page_number = (Page_number + 2) % 3;
    refresh_Flag = 1;
}

void Turn_to_above_item(void)
{
    if (Cursor_position < 7) // 防止越界
    {
        Cursor_position++;
        Active_Flag = 0;
        refresh_Flag = 1;
    }
}

void Turn_to_below_item(void)
{
    if (Cursor_position > 2) // 防止越界
    {
        Cursor_position--;
        Active_Flag = 0;
        refresh_Flag = 1;
    }
}

void item_data_increase(void)
{
    if (Active_Flag == 1 && Cursor_position != 2)
    {
        (*adress) += STEP;
        refresh_Flag = 1;
    }
    if (Active_Flag == 1 && Cursor_position == 2)
    {
        if (*adress < 10) // 防止 Current_Step 超过限制
            (*adress) *= 10;
        refresh_Flag = 1;
    }

    if (*Pages[Page_number].Current_Step > 100)
    {
        *Pages[Page_number].Current_Step = 100;
        refresh_Flag = 1;
    }
}

void item_data_decrease(void)
{
    if (Active_Flag == 1 && Cursor_position != 2)
    {
        (*adress) -= STEP;
        refresh_Flag = 1;
    }
    if (Active_Flag == 1 && Cursor_position == 2)
    {
        if (*adress >(float) 0.1) // 防止过度缩小
            (*adress) /=(float) 10;
        refresh_Flag = 1;
    }

    if (*Pages[Page_number].Current_Step < (float)0.01)
    {
        *Pages[Page_number].Current_Step = (float)0.01;
        refresh_Flag = 1;
    }
}

void Menu_refresh(void)
{
    if (refresh_Flag > 0)
    {
        Menu_show_current_page();
        Menu_show_cursor();
        refresh_Flag = 0;
    }
}

void Menu_page_init(void)
{
    OLED_Clear();
    Pages_init();  // 初始化所有页面
}
