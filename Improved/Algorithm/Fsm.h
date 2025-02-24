#ifndef __FSM_H
#define __FSM_H

#include "sys.h"
#include <stdbool.h>
#include "InfraredTracking.h"

// 定义状态枚举
typedef enum {
    IDLE,            // 待机状态
    SCAN,            // 扫描病房号
    IDENTIFY_BED,    // 识别病房类型
    LOADING,         // 装载药品
    NAVIGATE_NEAR,   // 导航到近端病房
    NAVIGATE_MID,    // 导航到中部病房
    NAVIGATE_FAR,    // 导航到远端病房
    DELIVER,         // 送药
    RETURN,          // 返航
    Error            // 错误状态
} CarState_t;

// 定义事件枚举
typedef enum {
    START,            // 开始任务
    ROOM_SCANNED,     // 病房号识别完成
    BED_NEAR,         // 近端病房
    BED_MID,          // 中部病房
    BED_FAR,          // 远端病房
    LOADED,           // 药品装载完成
    ARRIVED,          // 到达病房
    DELIVERED,        // 送药完成
    HOME,             // 返回药房
    OBSTACLE,         // 遇到障碍
    RESOLVED          // 问题解决
} Event_t;

// 当前状态变量
extern  CarState_t Car1_state;
extern  Event_t	Car1_Event;
extern bool bedIdentified;

void handleEvent(Event_t event);

#endif
