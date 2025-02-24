import sensor, image, time
from pyb import UART

# 初始化 UART
uart = UART(3, 115200)  # 使用 UART 3，波特率为 115200

# 初始化图像传感器
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)

# 计算校验和的函数
def calculate_checksum(data):
    checksum = 0
    for byte in data:
        checksum ^= byte  # 使用异或进行简单校验和计算
    return checksum

# 数据包发送函数
def send_packet(data):
    # 包头 0xAA，包尾 0x55
    header = 0xAA
    footer = 0x55
    data_length = len(data)
    
    # 计算校验和
    checksum = calculate_checksum(data)
    
    # 构建数据包
    packet = bytearray([header, data_length] + data + [checksum, footer])
    
    # 发送数据包
    uart.write(packet)

# 数字识别和发送数据包
while(True):
    img = sensor.snapshot()  # 获取图像
    digits = img.find_template("digit.bmp", 0.8)  # 假设有一个模板是预先存储的数字图像
    if digits:
        digit = digits[0][4]  # 提取识别出的数字
        data = [digit]  # 构建数据列表，数据是识别出的数字
        send_packet(data)  # 发送数据包
    time.sleep(100)
