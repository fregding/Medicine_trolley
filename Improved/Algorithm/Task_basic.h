#ifndef __TASK_BASIC_H
#define __TASK_BASIC_H

#include "sys.h"
#include "Fsm.h"
#include <math.h>
#include "HX711.h"
#include "stdbool.h"
#include "InfraredTracking.h"
#include "encoder.h"
#include "Motor.h"
#include "MotionControl.h"

#define THRESHOLD 2      // 允许的误差阈值

//	逐步模拟：
//	1.从IDLE开始：Car1_state = IDLE，Tracking_Flag = -1。
//	2.接收到START事件：Car1_state变为SCAN。
//	3.扫描病房号：Car1_state变为IDENTIFY_BED。
//	4.识别床类型：根据床号，Car1_Event设置为BED_NEAR、BED_MID或BED_FAR。
//	5.加载药物：Car1_state变为LOADING。
//	6.药物加载完成：根据床类型，Car1_state变为NAVIGATE_NEAR、NAVIGATE_MID或NAVIGATE_FAR。
//	7.导航到病房：
//	对于NAVIGATE_NEAR，Tracking_Flag应从0增加到1。
//	对于NAVIGATE_MID，Tracking_Flag应从0增加到2。
//	对于NAVIGATE_FAR，Tracking_Flag应从0增加到3。
//	8.到达病房：Car1_state变为DELIVER。
//	9.药物递送完成：Car1_state变为RETURN。
//	10.返回药房：
//	Tracking_Flag应根据Total_Cross_Numbers减少。
//	11.一旦Tracking_Flag达到-2，Car1_state变为IDLE。

extern int8_t Quarter_turn_Flag;
extern int8_t Full_turn_Flag;
extern int8_t Car_stop_Flag;

bool isStraight(uint8_t *data);
bool isCrossOrT(uint8_t *data);

bool compare_arrays_range(uint8_t* arr1, uint8_t* arr2, uint8_t start, uint8_t len);

void Fsm_ActionS(void);

void PathTracking(uint8_t* data_address);

#endif
