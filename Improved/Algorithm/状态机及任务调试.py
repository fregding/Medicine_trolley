import random

# 定义状态及其中文描述
class CarState:
    IDLE = "待机"
    SCAN = "扫描"
    IDENTIFY_BED = "识别病床"
    LOADING = "装载药物"
    NAVIGATE_NEAR = "导航到近端"
    NAVIGATE_MID = "导航到中端"
    NAVIGATE_FAR = "导航到远端"
    DELIVER = "递送药物"
    RETURN = "返回药房"
    ERROR = "错误"

# 定义事件及其中文描述
class Event:
    START = "启动"
    ROOM_SCANNED = "房间扫描完成"
    BED_NEAR = "识别为近端病床"
    BED_MID = "识别为中端病床"
    BED_FAR = "识别为远端病床"
    LOADED = "药物装载完成"
    ARRIVED = "到达目的地"
    DELIVERED = "药物递送完成"
    HOME = "返回药房"
    OBSTACLE = "遇到障碍物"
    RESOLVED = "问题解决"

# 初始化状态机和相关变量
Car1_state = CarState.IDLE
bedIdentified = False
Total_Cross_Numbers = 0
Tracking_Flag = -1
Quarter_turn_Flag = 0
cam_Num = None


def simulate_camera():
    # 随机生成一个病床号，假设在1到8之间
    return random.randint(1, 8)


def handleEvent(event):
    global Car1_state, bedIdentified, Total_Cross_Numbers, Tracking_Flag, Quarter_turn_Flag, cam_Num

    if Car1_state == CarState.IDLE:
        if event == Event.START:
            Car1_state = CarState.SCAN
            print("时间: 启动，状态: 待机 -> 扫描")
    elif Car1_state == CarState.SCAN:
        if event == Event.ROOM_SCANNED:
            Car1_state = CarState.IDENTIFY_BED
            print("时间: 房间扫描完成，状态: 扫描 -> 识别病床")
    elif Car1_state == CarState.IDENTIFY_BED:
        if event in [Event.BED_NEAR, Event.BED_MID, Event.BED_FAR]:
            bedIdentified = True
            cam_Num = event
            Car1_state = CarState.LOADING
            print(f"时间: {event}，状态: 识别病床 -> 装载药物")
    elif Car1_state == CarState.LOADING:
        if event == Event.LOADED:
            if bedIdentified:
                if cam_Num == Event.BED_NEAR:
                    Car1_state = CarState.NAVIGATE_NEAR
                elif cam_Num == Event.BED_MID:
                    Car1_state = CarState.NAVIGATE_MID
                elif cam_Num == Event.BED_FAR:
                    Car1_state = CarState.NAVIGATE_FAR
                bedIdentified = False
                print(f"时间: 药物装载完成，状态: 装载药物 -> {Car1_state}")
    elif Car1_state in [CarState.NAVIGATE_NEAR, CarState.NAVIGATE_MID, CarState.NAVIGATE_FAR]:
        if event == Event.ARRIVED:
            Car1_state = CarState.DELIVER
            print(f"时间: 到达目的地，状态: {Car1_state} -> 递送药物")
        elif event == Event.OBSTACLE:
            Car1_state = CarState.ERROR
            print("时间: 遇到障碍物，状态: 导航 -> 错误")
    elif Car1_state == CarState.DELIVER:
        if event == Event.DELIVERED:
            Car1_state = CarState.RETURN
            print("时间: 药物递送完成，状态: 递送药物 -> 返回药房")
    elif Car1_state == CarState.RETURN:
        if event == Event.HOME:
            Car1_state = CarState.IDLE
            Total_Cross_Numbers = 1
            print("时间: 返回药房，状态: 返回药房 -> 待机")


# 跟踪状态函数

def Tracking_reg(data_address):
    global Tracking_Flag, Quarter_turn_Flag, Total_Cross_Numbers, Car1_state
    error = 0

    # 判断是否需要直行
    if isStraight(data_address):
        Tracking_Flag = 0
        Quarter_turn_Flag = 0
        error = calculate_error(data_address)
    # 判断是否为交叉口或T型路口
    elif isCrossOrT(data_address):
        if Car1_state in [CarState.NAVIGATE_NEAR, CarState.NAVIGATE_MID, CarState.NAVIGATE_FAR]:
            Tracking_Flag += 1
            if Total_Cross_Numbers < Tracking_Flag:
                Total_Cross_Numbers = Tracking_Flag  # 记录最终路口数
        elif Car1_state == CarState.RETURN:
            if Tracking_Flag > 0:
                Tracking_Flag -= 1
            else:
                Tracking_Flag = -2

        # 四分之一圈转向逻辑判断
        if ((Car1_state == CarState.NAVIGATE_NEAR and Tracking_Flag == 1) or
            (Car1_state == CarState.NAVIGATE_MID and Tracking_Flag == 2) or
            (Car1_state == CarState.NAVIGATE_FAR and Tracking_Flag == 3) or
            (Car1_state == CarState.RETURN and
             ((Total_Cross_Numbers < 3 and Tracking_Flag == Total_Cross_Numbers) or
              (Total_Cross_Numbers > 2 and
               (Tracking_Flag == Total_Cross_Numbers or Tracking_Flag == Total_Cross_Numbers - 1))))):
            Quarter_turn_Flag = 1
        else:
            Quarter_turn_Flag = 0

    else:
        Quarter_turn_Flag = 0

    return error


# 模拟转向控制

def isStraight(data_address):
    return random.choice([True, False])


def isCrossOrT(data_address):
    return not isStraight(data_address)


def calculate_error(data_address):
    return random.randint(-5, 5)


def updateQuarterTurnFlag():
    global Car1_state, Tracking_Flag, Quarter_turn_Flag, Total_Cross_Numbers

    if (Car1_state == CarState.NAVIGATE_NEAR and Tracking_Flag == 1) or \
       (Car1_state == CarState.NAVIGATE_MID and Tracking_Flag == 2) or \
       (Car1_state == CarState.NAVIGATE_FAR and Tracking_Flag == 3):
        Quarter_turn_Flag = 1
        print("时间: 检测到交叉路口，需要进行四分之一圈转向。")
    elif Car1_state == CarState.RETURN:
        if Total_Cross_Numbers < 3 and Tracking_Flag == Total_Cross_Numbers:
            Quarter_turn_Flag = 1
            print("时间: 检测到交叉路口，需要进行四分之一圈转向。")
        elif Total_Cross_Numbers > 2 and (Tracking_Flag == Total_Cross_Numbers or Tracking_Flag == Total_Cross_Numbers - 1):
            Quarter_turn_Flag = 1
            print("时间: 检测到交叉路口，需要进行四分之一圈转向。")
        else:
            Quarter_turn_Flag = 0
            print("时间: 继续直行，不需要转向。")
    else:
        Quarter_turn_Flag = 0
        print("时间: 继续直行，不需要转向。")


def simulate_full_process():
    global Tracking_Flag

    cam_Num = simulate_camera()
    print(f"随机生成的病床号: {cam_Num}")

    if cam_Num <= 2:
        bed_event = Event.BED_NEAR
        cross_numbers = 1
    elif cam_Num <= 4:
        bed_event = Event.BED_MID
        cross_numbers = 2
    else:
        bed_event = Event.BED_FAR
        cross_numbers = 3

    events = [
        Event.START,
        Event.ROOM_SCANNED,
        bed_event,
        Event.LOADED,
        Event.ARRIVED,
        Event.DELIVERED,
        Event.HOME
    ]

    for event in events:
        handleEvent(event)

    # 导航过程
    for i in range(1, cross_numbers + 1):
        Tracking_Flag = i
        print(f"时间: 通过第{i}个交叉路口，Tracking_Flag: {Tracking_Flag}，Quarter_turn_Flag:{Quarter_turn_Flag}")
        updateQuarterTurnFlag()

    Car1_state = CarState.RETURN
    print("时间: 开始返回药房...")

    # 返回过程
    for i in range(cross_numbers, 0, -1):
        Tracking_Flag -= 1
        print(f"时间: 返回第{i}个交叉路口，Tracking_Flag: {Tracking_Flag}，Quarter_turn_Flag:{Quarter_turn_Flag}")
        updateQuarterTurnFlag()

    handleEvent(Event.HOME)


# 运行模拟
simulate_full_process()
