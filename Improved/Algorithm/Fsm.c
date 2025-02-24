#include "Fsm.h" 
//主循环负责长时间的任务，比如导航、送药和状态监控。它通过不断检查传感器和外部事件来触发状态机。
//中断处理需要即时响应的事件，比如碰撞检测、按键输入等。中断处理函数会快速地改变状态机的状态，并在短时间内返回。
//主循环 	 |检查事件|--->|执行任务|
//			   |检查事件|--->|传递事件|--->|状态机处理|--->触发状态变化，执行任务
//中断处理：|中断事件|--->|状态机处理|
//

CarState_t Car1_state = IDLE;
Event_t	Car1_Event = RESOLVED;

// 增加一个标志位来跟踪病床是否已识别
bool bedIdentified = false;

// 状态机处理函数
void handleEvent(Event_t event) 
{
    switch (Car1_state) 
    {
        case IDLE:
					
						// 在空闲状态下，准备开始新的任务
						Fsm_ActionS();
            if (event == START) 
						{
                Car1_state = SCAN;
            }
            break;

        case SCAN:
					
						// 在扫描状态下，调用 Recognised1 来识别卡片
            Fsm_ActionS();
						if (event == ROOM_SCANNED) 
						{
                Car1_state = IDENTIFY_BED;
            }
            break;

        case IDENTIFY_BED:
					
						// 在识别病床状态下，调用 Sort_and_Start 来准备送药
						Fsm_ActionS();
            // 识别病床时，根据病房编号来设置相应的事件
            if (event == BED_NEAR || event == BED_MID || event == BED_FAR) 
						{
                bedIdentified = true;  // 标记病床已识别
                Car1_state = LOADING;  // 跳转到装载药品
            }
            break;

        case LOADING:
					
						// 在装载药物状态下，调用 Departure1 来确认药物是否装载好
						Fsm_ActionS();
            if (event == LOADED) 
						{
                if (bedIdentified) 
								{
                    if (Car1_Event == BED_NEAR) 
										{
                        Car1_state = NAVIGATE_NEAR;
                    } 
									  else if (Car1_Event == BED_MID) 
										{
                        Car1_state = NAVIGATE_MID;
                    } 
										else if (Car1_Event == BED_FAR) 
										{
                        Car1_state = NAVIGATE_FAR;
                    }
                    bedIdentified = false;  // 清除标志
                }
            }
            break;

        case NAVIGATE_NEAR:
        case NAVIGATE_MID:
        case NAVIGATE_FAR:
					
					  // 在导航状态下，调用路径跟踪函数来进行路径跟踪
            PathTracking(Infrared_filter_Get());
						Fsm_ActionS();
            if (event == ARRIVED) 
						{
                Car1_state = DELIVER;
            } 
						else if (event == OBSTACLE) 
						{
                Car1_state = Error;
            }
            break;

        case DELIVER:
					
					  // 在送药状态下，调用 Departure2 来检查药物是否已取走
						Fsm_ActionS();
            if (event == DELIVERED) 
						{
                Car1_state = RETURN;
            }
            break;

        case RETURN:
						Fsm_ActionS();
            if (event == HOME) 
						{
                Car1_state = IDLE;
							  Total_Cross_Numbers = 1;
            }
            break;

        case Error:

            break;

        default:
            // 处理未知的事件
            break;
    }
}
