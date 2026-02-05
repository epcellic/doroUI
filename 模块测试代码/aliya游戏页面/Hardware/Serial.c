#include "stm32f10x.h"
#include <stdlib.h>
#include <string.h>                   
#include "Serial.h"  

/* 2. 新增：静态全局模式变量（仅本文件可见，通过函数控制修改） */
static SerialMode serialMode = SERIAL_MODE_NONE;  // 初始化为无操作模式

/* 3. 静态全局帧接收器（保持不变） */
static FrameReceiver frameRecv = {
    .state = FRAME_WAIT_START,
    .frameIndex = 0,
    .arrayIndex = 0,
    .dataCount = 0
};

extern uint8_t OLED_DisplayBuf[8][128]; 

/* 4. 新增：OLED显存写入位置变量（从中断中抽离，便于统一管理） */
static uint8_t oledP0 = 0;  // OLED显存行索引（0-7）
static uint8_t oledP1 = 0;  // OLED显存列索引（0-127）

// 串口1初始化函数（保持不变）
void Serial_Init(void)
{
    // 1. 使能USART1和GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    // 2. 配置TX引脚(PA9)为复用推挽输出
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 3. 配置RX引脚(PA10)为上拉输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 4. 配置USART1通信参数（921600波特率，8N1）
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 921600;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStructure);
    
    // 5. 开启接收中断（RXNE）
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    
    // 6. 配置NVIC（中断分组2，抢占优先级1，子优先级1）
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);
    
    // 7. 使能USART1
    USART_Cmd(USART1, ENABLE);
}

// 串口关闭函数（保持不变）
void Serial_DeInit(void)
{
    USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);
    USART_Cmd(USART1, DISABLE);
    USART_DeInit(USART1);
}

// 发送一个字节（保持不变）
void Serial_SendByte(uint8_t Byte)
{
    USART_SendData(USART1, Byte);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

// 发送字符串（保持不变）
void Serial_SendString(const char* String)
{
    if (String == NULL) return;
    while (*String != '\0') {
        Serial_SendByte((uint8_t)*String);
        String++;
    }
}

// 获取帧数据（保持不变）
uint8_t Serial_GetFrameData(char* buf)
{
    if (frameRecv.dataCount == 0 || buf == NULL) return 0;
    
    // 复制最早的一帧数据
    strcpy(buf, frameRecv.frameArray[0]);
    
    // 数据移位（队列逻辑）
    for (uint8_t i = 0; i < frameRecv.dataCount - 1; i++) {
        strcpy(frameRecv.frameArray[i], frameRecv.frameArray[i + 1]);
    }
    frameRecv.dataCount--;
    return 1;
}

/* 5. 新增：模式配置函数（外部调用，切换模式并重置状态） */
void Serial_SetMode(SerialMode mode)
{
    // 1. 关闭串口接收中断（防止切换时接收数据导致混乱）
    USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
    
    // 2. 更新模式
    serialMode = mode;
    
    // 3. 根据新模式重置对应状态（关键：避免残留数据导致错位）
    switch (mode) {
        case SERIAL_MODE_OLED:
            // 重置OLED显存写入位置（从头开始显示）
            oledP0 = 0;
            oledP1 = 0;
            // 重置帧接收状态（避免模式切换后帧状态异常）
            frameRecv.state = FRAME_WAIT_START;
            frameRecv.frameIndex = 0;
            memset(frameRecv.frameBuf, 0, FRAME_MAX_LEN);
            break;
            
        case SERIAL_MODE_FRAME:
            // 重置帧接收状态（确保从等待#开始）
            frameRecv.state = FRAME_WAIT_START;
            frameRecv.frameIndex = 0;
            memset(frameRecv.frameBuf, 0, FRAME_MAX_LEN);
            // 可选：清空OLED显存位置（根据需求决定是否保留OLED显示）
            // oledP0 = 0; oledP1 = 0;
            break;
            
        case SERIAL_MODE_NONE:
            // 无操作模式：重置所有状态
            oledP0 = 0;
            oledP1 = 0;
            frameRecv.state = FRAME_WAIT_START;
            frameRecv.frameIndex = 0;
            frameRecv.dataCount = 0;
            memset(frameRecv.frameBuf, 0, FRAME_MAX_LEN);
            memset(frameRecv.frameArray, 0, FRAME_BUF_SIZE * FRAME_MAX_LEN);
            break;
    }
    
    // 4. 重新开启串口接收中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

/* 6. 重构：USART1中断服务函数（核心：按模式分支处理数据） */
void USART1_IRQHandler(void)
{
    uint8_t RxData;
    
    // 仅处理接收非空中断（RXNE）
    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET) {
        RxData = USART_ReceiveData(USART1);  // 读取接收数据
        
        // 按当前模式分支处理（互斥：同一时间只处理一种数据）
        switch (serialMode) {
            // -------------------------- 模式1：OLED显存模式 --------------------------
            case SERIAL_MODE_OLED:
                // 直接将接收数据写入OLED显存，不处理#!帧
                OLED_DisplayBuf[oledP0][oledP1] = RxData;
                
                // 更新OLED显存位置（循环：0-7行，0-127列）
                oledP1++;
                if (oledP1 >= 128) {  // 列满，换行
                    oledP1 = 0;
                    oledP0++;
                    if (oledP0 >= 8) {  // 行满，从头开始
                        oledP0 = 0;
                    }
                }
                break;
                
            // -------------------------- 模式2：#!帧模式 --------------------------
            case SERIAL_MODE_FRAME:
                // 只解析#开头、!结尾的帧，不写入OLED
                switch (frameRecv.state) {
                    case FRAME_WAIT_START:
                        // 等待起始符#，收到后初始化帧缓冲区
                        if (RxData == '#') {
                            frameRecv.state = FRAME_RECEIVING;
                            frameRecv.frameIndex = 0;
                            memset(frameRecv.frameBuf, 0, FRAME_MAX_LEN);
                        }
                        // 非#字符：直接丢弃（不处理）
                        break;
                        
                    case FRAME_RECEIVING:
                        // 收到结束符!：保存帧数据
                        if (RxData == '!') {
                            frameRecv.frameBuf[frameRecv.frameIndex] = '\0';  // 字符串结尾
                            
                            // 帧缓存未满时，存入多帧数组
                            if (frameRecv.dataCount < FRAME_BUF_SIZE) {
                                strcpy(frameRecv.frameArray[frameRecv.arrayIndex], frameRecv.frameBuf);
                                frameRecv.arrayIndex = (frameRecv.arrayIndex + 1) % FRAME_BUF_SIZE;
                                frameRecv.dataCount++;
                            }
                            // 回到等待起始符状态
                            frameRecv.state = FRAME_WAIT_START;
                        } else {
                            // 非!字符：存入帧缓冲区（防止溢出）
                            if (frameRecv.frameIndex < FRAME_MAX_LEN - 1) {
                                frameRecv.frameBuf[frameRecv.frameIndex++] = RxData;
                            } else {
                                // 缓冲区满：丢弃当前帧，重置状态
                                frameRecv.state = FRAME_WAIT_START;
                            }
                        }
                        break;
                        
                    default:
                        // 异常状态：重置为等待起始符
                        frameRecv.state = FRAME_WAIT_START;
                        break;
                }
                break;
                
            // -------------------------- 无操作模式 --------------------------
            case SERIAL_MODE_NONE:
                // 丢弃所有接收数据（不处理）
                break;
        }
        
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);  // 清除中断标志
    }
}
