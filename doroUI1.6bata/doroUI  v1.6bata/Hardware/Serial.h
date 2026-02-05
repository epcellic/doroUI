#ifndef __SERIAL_H
#define __SERIAL_H

#include "stm32f10x.h"
#include <stdint.h>
#include <string.h>

/* 1. 新增：串口工作模式枚举（替代直接用1/2，增强代码可读性） */
typedef enum {
    SERIAL_MODE_NONE = 0,    // 预留：无操作模式
    SERIAL_MODE_OLED = 1,    // OLED显存模式：接收数据直接写入OLED显存
    SERIAL_MODE_FRAME = 2    // #!帧模式：只解析#开头、!结尾的帧数据
} SerialMode;

/* 帧数据接收相关宏定义（保持不变） */
#define FRAME_MAX_LEN 128    // 单帧最大长度
#define FRAME_BUF_SIZE 5     // 帧缓存数组大小（最多存5帧）

// 帧接收状态枚举（保持不变）
typedef enum {
    FRAME_WAIT_START,        // 等待帧起始符#
    FRAME_RECEIVING,         // 正在接收帧数据
    FRAME_WAIT_END           // 预留：可扩展帧尾校验（当前用!直接结束）
} FrameState;

// 帧接收结构体（保持不变）
typedef struct {
    FrameState state;        // 当前帧接收状态
    char frameBuf[FRAME_MAX_LEN];  // 单帧数据缓冲区
    uint16_t frameIndex;     // 单帧缓冲区索引
    char frameArray[FRAME_BUF_SIZE][FRAME_MAX_LEN];  // 多帧缓存数组
    uint8_t arrayIndex;      // 多帧数组索引
    uint8_t dataCount;       // 有效帧数量
} FrameReceiver;

/* 函数声明（新增Serial_SetMode函数） */
void Serial_Init(void);
void Serial_DeInit(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendString(const char* String);
uint8_t Serial_GetFrameData(char* buf);
void USART1_IRQHandler(void);
void Serial_SetMode(SerialMode mode);  // 新增：模式配置函数

#endif /* __SERIAL_H */
