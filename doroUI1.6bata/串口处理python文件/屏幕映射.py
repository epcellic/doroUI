import numpy as np
import cv2
import serial
import time
import mss
import mss.tools


# 将128*64的二值化图像转换为OLED数组格式
def img2array(frame):
    array = np.zeros((8, 128), dtype='uint8')
    for j in range(64):
        for i in range(128):
            if frame[j][i] > 0:
                array[j // 8][i] = (array[j // 8][i]) | (0x01 << (j % 8))
    return array


# 屏幕捕获和处理函数
def capture_screen(monitor):
    with mss.mss() as sct:
        # 捕获屏幕
        sct_img = sct.grab(monitor)
        # 转换为OpenCV格式
        img = np.array(sct_img)
        # 转换为BGR格式（OpenCV默认格式）
        img = cv2.cvtColor(img, cv2.COLOR_RGBA2BGR)
        return img


# 主函数
def main():
    # 打开串口，根据实际情况修改端口
    try:
        serial_port = serial.Serial('COM3', 921600, timeout=1)
        time.sleep(2)  # 等待串口初始化
    except Exception as e:
        print(f"串口打开失败: {e}")
        return

    # 设置预览窗口
    cv2.namedWindow('Screen Stream', cv2.WINDOW_NORMAL)
    cv2.resizeWindow('Screen Stream', 128 * 4, 64 * 4)

    # 定义要捕获的屏幕区域（全屏）
    with mss.mss() as sct:
        monitor = sct.monitors[1]  # 1表示主屏幕

    # 控制帧率为30fps
    frame_interval = 1.0 / 30  # 每帧间隔时间（秒）
    last_frame_time = 0

    try:
        while True:
            current_time = time.time()

            # 控制帧率
            if current_time - last_frame_time < frame_interval:
                continue
            last_frame_time = current_time

            # 捕获屏幕
            img = capture_screen(monitor)

            # 图像处理
            img = cv2.resize(img, (128, 64))  # 缩放到OLED屏幕尺寸
            img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)  # 灰度化
            img = cv2.threshold(img, 80, 255, cv2.THRESH_BINARY)[1]  # 二值化

            # 显示预览
            cv2.imshow('Screen Stream', img)

            # 转换为OLED格式并发送
            img_array = img2array(img)
            serial_port.write(img_array.tobytes())  # 确保以字节形式发送

            # 按q键退出
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    except KeyboardInterrupt:
        print("程序被用户中断")
    finally:
        # 释放资源
        cv2.destroyAllWindows()
        serial_port.close()
        print("资源已释放")


if __name__ == "__main__":
    main()
