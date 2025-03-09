#include "Menu_TFT.h"


// 四个按钮 8个动作
// key1 short -> Turn to pre page        long -> Turn to next page
// key2 short -> Turn to above item      long -> Turn to below item
//               Cursor_position              Cursor_position
// key3 short -> item data increase      long -> item data decrease
// key4 short -> Set active item         long -> Release item

uint8_t Current_keyState = 0;  // 0 -> 浮空 1 -> 单击 2 -> 双击 3 -> 长按
uint8_t Active_Flag = 0;
uint8_t Cursor_position = 3;
uint8_t refresh_Flag = 1;
uint8_t Page_number = 0;  // 当前页面编号，0 表示第一页

static float num1 = 1.0, num2 = 0.0, num3 = 0.0, num4 = 1145.1, num5 = -110.0;
float* adress = &num1;
static float STEP = 10;
float BIN = 0;

// 变量集
float tims = 0;
float init_F = 1;
float Pitch,Roll,Yaw;

MenuPage Pages[3];  // 定义三个页面

page_items Page1_Items = {
    {"speed_L", "speed_R", "Tims", "I", "D"},  // 项目名称
    {&encoder_left, &encoder_right, &tims, &num4, &num5}  // 项目地址
};

page_items Page2_Items = {
    {"init_F", "Pitch", "Roll", "Yaw", "Error"},
    {&init_F, &Pitch, &Roll, &Yaw, &num1}
};

page_items Page3_Items = {
    {"Current", "Voltage", "Power", "Temp", "Status"},
    {&num1, &num2, &num3, &num1, &num1}
};


// 每页的初始化函数
void Pages_init(void)
{
    // 初始化页面 123
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
    LCD_Fill(0,0,LCD_W,LCD_H,WHITE);// 每次刷新页面，清空屏幕

    Menu_show_String(Title_offset, 0, Pages[Page_number].TitleName, 0);
    Menu_show_String(Item_offset, 2, "Step =", 0);

    for (int i = 0; i < 5; i++) {
        if (Pages[Page_number].itemNames[0][i][0] != '\0') {  // 判断项目是否有效
            Menu_show_String(Item_offset, 3 + i, Pages[Page_number].itemNames[0][i], 0);
            Menu_show_Float(Data_offset, 3 + i, *Pages[Page_number].itemData[i], 0);
				
        }
    }
}

void Menu_show_current_Value(void)
{
	Menu_show_Float(Item_offset + 8, 2, *Pages[Page_number].Current_Step, 0);
	for (int i = 0; i < 5; i++) 
	{
			if (Pages[Page_number].itemNames[0][i][0] != '\0') 
			{  // 判断项目是否有效
					Menu_show_String(Data_offset, 3 + i, "        ", 0);
					Menu_show_Float(Data_offset, 3 + i, *Pages[Page_number].itemData[i], 0);
			
			}
	}
}

void judge_active(void)
{
    if (Cursor_position < 2 || Cursor_position > 7) return; // 保护Cursor_position不越界

    if (Active_Flag) 
		{
        Menu_show_String(0, Cursor_position,"&", 0);
    } 
		else 
		{
        Menu_show_String(0, Cursor_position,">", 0);
    }

    // 动态决定当前页面的项目地址
    switch (Cursor_position) {
				case 2: adress = &STEP;break;
        case 3: adress = Pages[Page_number].itemData[0]; break;
        case 4: adress = Pages[Page_number].itemData[1]; break;
        case 5: adress = Pages[Page_number].itemData[2]; break;
        case 6: adress = Pages[Page_number].itemData[3]; break;
        case 7: adress = Pages[Page_number].itemData[4]; break;
    }
}

void Menu_show_cursor(void)
{
    // 清除之前的光标
    for (int i = 0; i < 8; i++)
        Menu_show_Char(0, i, ' ', 0);

    if (Cursor_position > 7) Cursor_position = 7;
    if (Cursor_position < 2) Cursor_position = 2;

    judge_active();
}

void Turn_to_next_page(void)
{
    // 循环翻页，从第一页到最后一页，再回到第一页
    Page_number = (Page_number + 1) % 3;
		Active_Flag = 0;
		refresh_Flag = 1;
}

void Turn_to_previous_page(void)
{
    // 循环翻页，从第一页回到最后一页
    Page_number = (Page_number + 2) % 3;
		Active_Flag = 0;
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
        if (STEP < 1000) // 防止 Current_Step 超过限制
            STEP*= 10;
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
        if (STEP >=(float) 0.01) // 防止过度缩小
            STEP /=(float) 10;
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
		 Menu_show_current_Value();
}

void Menu_page_init(void)
{
    LCD_Clear();
    Pages_init();  // 初始化所有页面
}
