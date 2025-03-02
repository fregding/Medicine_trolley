# STM32F407ZET6 电赛送药小车说明

- 不可删除代码（main.c部分）
  - //系统级初始化
  - NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  // 中断优先级分组
  - SystemCoreClockUpdate();                        // 关键点，更新时钟变量
  - delay_init(168);    // 注意：delay_init已占用SysTick，不可再调用SysTick_Init()
    
### 一、按键功能

启用TIM3定时中断，20ms按键扫描（优先级1.3），在TIM3_IRQHandler()里调用key_handler();进行列表查询，简化中断配置。
可自由增加按键数，struct配备短按，长按，双击功能。

- **key1**  
  - **短按**: 
  - **长按**:
  - **双击**：
  
- **key2**  
  - **短按**: 
  - **长按**: 
  

### 二、Menu部分
该系统可自由添加页面（仿照文档示例），每个页面包含多个项目。每个项目的数据可以被选择并修改。
每次刷新屏幕时，系统会显示当前页面的标题、步长（Step）以及各项数据。菜单项通过光标标示，用户可以选择并修改相关数据。

- **页面 1**  
  - 项目名称: `a`, `P`, `I`, `D`, `Angle`  
  - 数据: `num1`, `num2`, `num3`, `num2`, `num3`  
   
- **页面 2**  
  - 项目名称: `Speed`, `Angle`, `Position`, `Time`, `Error`  
  - 数据: `num1`, `num2`, `num3`, `angle`, `BIN`  

- **页面 3**  
  - 项目名称: `Current`, `Voltage`, `Power`, `Temp`, `Status`  
  - 数据: `num1`, `num2`, `num3`, `angle`, `BIN`

- **翻页功能**  
- **光标显示**  
  当前选择的菜单项会用光标 `>` 或 `&` 标示，提示用户当前所选项。

- **主要变量**
- `Current_keyState`: 当前按键状态。可以是浮空、单击、双击或长按。
- `Active_Flag`: 标记当前选中项是否为活动项。
- `Cursor_position`: 当前光标的位置。
- `refresh_Flag`: 页面是否需要刷新。
- `Page_number`: 当前页面编号，取值范围 [0, 2]。

### 三、电机部分
TIMx各端口在TimerX.h文件标出，自行查阅。
电机驱动建议使用10kHz-PWM，计算方式见Motor.c

