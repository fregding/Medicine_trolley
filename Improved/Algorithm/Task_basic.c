#include "Task_basic.h" 


uint8_t cam_Num = 0;

int8_t Quarter_turn_Flag = 0;
int8_t Full_turn_Flag = 0;
int8_t Car_stop_Flag = 0;
// 预定义标准模式
//                        |   0-3 |  4-7  |  8-11 | 12-15 |
uint8_t Straight_eg0[16] = {0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0};
uint8_t Straight_eg1[16] = {0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0};
uint8_t CrossOrT_eg0[16] = {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0};
uint8_t CrossOrT_eg1[16] = {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0};
uint8_t CrossOrT_eg2[16] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

////////////    状态切换，触发函数     ///////////////

// 按键,准备开始识别 Car1_Event = START


// 摄像头识别到卡片   Car1_Event = ROOM_SCANNED
void Recognised1(uint8_t num)
{
	if(Car1_state == SCAN)
	{
		if(num>0&&num<9)
		{
			delay_ms(200);//待数字稳定
			// 识别到同一个数字20次再触发
			Car1_Event = ROOM_SCANNED;
		}
	}
}

// 识别远近病房，并准备开始送药
void Sort_and_Start(uint8_t num)  
{
    if (Car1_state == IDENTIFY_BED && !bedIdentified) // 确保未重复识别
    {
        if (num == 1 || num == 2)
            Car1_Event = BED_NEAR;
        else if (num == 3 || num == 4)
            Car1_Event = BED_MID;
        else if (num > 4 && num < 9)
            Car1_Event = BED_FAR;
        
        bedIdentified = true; // 避免重复识别
    }
}


// 检测到放药，准备发车 Car1_Event = LOADED
void Departure1(void)
{
	if(Car1_state == LOADING)
	{
		if(HX711_Read() > 100)
		{
			Car1_Event = LOADED;
		}
	}
}

// 识别到病房数字,到点后加停车 	Car1_Event = ARRIVED
void Recognised2(uint8_t num)
{
	if(Car1_state==NAVIGATE_NEAR && Tracking_Flag == 1)
	{
			if(num==1||num==2)
			{
				Car1_Event = ARRIVED;
			}
	}
	else if(Car1_state==NAVIGATE_MID && Tracking_Flag == 2)
	{
			if(num==3||num==4)
			{
				Car1_Event = ARRIVED;
			}		
	}
	else if(Car1_state==NAVIGATE_FAR && Tracking_Flag == 4)
	{
			if(num>3&&num<9)
			{
				Car1_Event = ARRIVED;
			}		
	}
}

// 检测到取药，准备返航 Car1_Event = DELIVERED
void Departure2(void)
{
	if(Car1_state==DELIVER)
	{
		if(HX711_Read() < 100)
		{
			Car1_Event = DELIVERED;
		}
	}
}

// 停车			Car1_Event = HOME||ARRIVED
void Ready_to_Stop(void)
{
	if(Car1_state==RETURN || Car1_state == ARRIVED)
	{
		if(Tracking_Flag == -2 && isCrossOrT(Infrared_filter_Get())==1)
		{
			Car1_Event = HOME;
			Car_stop_Flag = 1;
		}
		else if(isCrossOrT(Infrared_filter_Get())==1)
		{
			Car_stop_Flag = 1;
		}
	}		
}

// 原地掉头，再识别			Car1_Event = START
void Ready_to_Start(void)
{
	if(Car1_state == IDLE && Tracking_Flag == -2)
	{
		// 原地掉头
		Car1_Event = START;
		Tracking_Flag = -1;
	}
}

void Fsm_ActionS(void)
{
	Recognised1(cam_Num);
	Sort_and_Start(cam_Num);
	Departure1();
	Recognised2(cam_Num);
	Departure2();
	Ready_to_Stop();
}


////////////    路线特征，寻迹处理      ///////////////

// 自定义函数，比较两个 uint8_t 数组的指定部分
bool compare_arrays_range(uint8_t* arr1, uint8_t* arr2, uint8_t start, uint8_t len) 
{
    // 确保起始位置和长度不会超过数组边界
    for (uint8_t i = 0; i < len; i++) 
   {
        if (arr1[start + i] != arr2[start + i]) 
				{
            return false;  // 如果有任何不同，返回 false
        }
    }
    return true;  // 如果指定范围内完全相同，返回 true
}

// 由于可能存在部分传感器误判（如单个 0 变 1），可以计算相似度（如汉明距离）来进行匹配。

// 计算两个数组的汉明距离（即不相同的位数）
int hammingDistance(uint8_t *a, uint8_t *b, int len) 
{
    int diff = 0;
    for (int i = 0; i < len; i++) 
	 {
        if (a[i] != b[i]) 
				{
            diff++;
        }
    }
    return diff;
}

// 判断是否为直线
bool isStraight(uint8_t *data) 
{
    if (hammingDistance(data, Straight_eg0, SENSOR_COUNT) <= THRESHOLD) return true;
    if (hammingDistance(data, Straight_eg1, SENSOR_COUNT) <= THRESHOLD) return true;
    return false;
}

// 判断是否为十字或 T 型路口
bool isCrossOrT(uint8_t *data) 
{
    if (hammingDistance(data, CrossOrT_eg0, SENSOR_COUNT) <= THRESHOLD) return true;
    if (hammingDistance(data, CrossOrT_eg1, SENSOR_COUNT) <= THRESHOLD) return true;
    if (hammingDistance(data, CrossOrT_eg2, SENSOR_COUNT) <= THRESHOLD) return true;
    return false;
}

// 最终输出控制量-error
int Tracking_reg(uint8_t* data_address)
{
	int error = 0;
	
	if (isStraight(data_address)) 
	{
		Quarter_turn_Flag = 0;
		error = calculate_error(data_address);
	} 
	else if (isCrossOrT(data_address)) 
	{
		if(Car1_state==NAVIGATE_NEAR||Car1_state==NAVIGATE_MID||Car1_state==NAVIGATE_FAR)
		{
			Tracking_Flag += 1;
			if(Total_Cross_Numbers<Tracking_Flag)
			{
				Total_Cross_Numbers = Tracking_Flag;//记录最终路口数
			}
		}
	else if (Car1_state == RETURN)
	{
    if(Tracking_Flag > 0) 
		{
        Tracking_Flag -= 1; // 逐步减少
		}
    else
		{
        Tracking_Flag = -2; // 终点到位
		}
	}

		if ((Car1_state == NAVIGATE_NEAR && Tracking_Flag == 1)||
				(Car1_state == NAVIGATE_MID && Tracking_Flag == 2) ||
				(Car1_state == NAVIGATE_FAR && (Tracking_Flag == 3||Tracking_Flag == 4)) ||
				(Car1_state == RETURN && 
					((Total_Cross_Numbers < 3 && Tracking_Flag == Total_Cross_Numbers) ||
					 (Total_Cross_Numbers > 2 && 
						(Tracking_Flag == Total_Cross_Numbers || Tracking_Flag == Total_Cross_Numbers - 1)))))
		{
				Quarter_turn_Flag = 1;
		}
		else
		{
				Quarter_turn_Flag = 0;
		}
			

	} 
	else 
	{
		Quarter_turn_Flag = 0;
	}
	return error;
}


// 2. 路径跟踪和误差计算函数
void PathTracking(uint8_t* data_address) 
{
    int error = Tracking_reg(data_address);  
    
    if (Quarter_turn_Flag == 1) 
		{
				updateMotorControl(CircleTurn_Speed,wheel_speed_left,0);
        // 执行转向动作，转向的操作需要依据机器人的硬件和驱动来实现
        updateMotionControl(90,&Real_leftPWM,&Real_rightPWM);
				tb6612_out(Real_leftPWM,Real_rightPWM);
    } 
		else 
		{
        updateMotorControl(GoStraight_Speed,wheel_speed_left,error);
				// 按照计算的误差执行纠正动作
				tb6612_out(Real_leftPWM,Real_rightPWM);
				
    }
}
