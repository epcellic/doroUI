#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "math.h"

// 全局变量
uint8_t KeyNum;
uint8_t chooseNum = 0;  // 0-未选中，1-6对应6个图标

// 【修改】动画相关变量（新增缓动系数，移除固定步长）
float targetTriangleX = 10.0f;  // 目标X（用float提高精度，避免整数截断误差）
float currentTriangleX = 10.0f; // 当前X（float类型，支持小数步长计算）
const uint8_t triangleY = 14;   // 三角形固定Y坐标
const float easeFactor = 0.2f;  // 缓动系数（0.1-0.3为宜，值越大动画越快）
const uint16_t frameDelay = 5;  // 帧间隔（固定，确保流畅度）

// 存储6个图标的三角形目标X坐标（与每个图标的中心对齐）
const float iconTriangleX[7] = {0.0f, 10.0f, 31.0f, 52.0f, 73.0f, 94.0f, 115.0f};

// 函数声明
void OLED_DrawMainMenu(void);                  // 绘制主菜单所有图标
void OLED_SetTargetSelected(uint8_t num);      // 设置选中目标
void OLED_PlayEaseAnim(void);                  // 【修改】播放"慢-快-慢"缓动动画
void OLED_DrawSelectedEffect(uint8_t num);     // 绘制选中效果

int main(void)
{
    // 开机动画、初始化等代码完全不变，此处省略...
    OLED_Init();
    Key_Init();

    OLED_ShowString(48, 50, "doroUI", OLED_6X8);
    OLED_ShowImage(48, 10, 38, 38, doro1);	OLED_Update();Delay_ms(100);OLED_ClearArea(48, 12, 38, 38);	
    OLED_ShowImage(48, 10, 38, 38, doro2);	OLED_Update();Delay_ms(100);OLED_ClearArea(48, 12, 38, 38);	
    OLED_ShowImage(48, 10, 38, 38, doro3);	OLED_Update();Delay_ms(100);OLED_ClearArea(48, 12, 38, 38);	
    OLED_ShowImage(48, 10, 38, 38, doro2);	OLED_Update();Delay_ms(100);OLED_ClearArea(48, 12, 38, 38);	
    OLED_ShowImage(48, 10, 38, 38, doro1);	OLED_Update();Delay_ms(100);OLED_ClearArea(48, 12, 38, 38);	
    OLED_ShowImage(48, 10, 38, 38, doro2);	OLED_Update();Delay_ms(100);OLED_ClearArea(48, 12, 38, 38);	
    OLED_ShowImage(48, 10, 38, 38, doro3);	OLED_Update();Delay_ms(100);OLED_ClearArea(48, 12, 38, 38);	
    OLED_ShowImage(48, 10, 38, 38, doro2);	OLED_Update();Delay_ms(100);OLED_ClearArea(48, 12, 38, 38);	
    OLED_ShowImage(48, 10, 38, 38, doro1);  OLED_Update();Delay_ms(100);
    OLED_Clear();OLED_Update();
    OLED_ShowString(15, 10, "EPCELLIC 出品", OLED_8X16);
    OLED_ShowString(45, 50, "Ver.1.1", OLED_6X8);
    OLED_Update();
    Delay_ms(1500);
    OLED_Clear();OLED_Update();
	
    OLED_DrawMainMenu();
    OLED_Update();
    Delay_ms(500);

    while (1)
    {
        KeyNum = Key_GetNum();

        // 按键3：下一个图标
        if (KeyNum == 3)	
        {
            chooseNum++;
            if (chooseNum >= 7) chooseNum = 1;
            OLED_SetTargetSelected(chooseNum);
            OLED_PlayEaseAnim();  // 【修改】调用缓动动画函数
        }

        // 按键4：上一个图标
        if (KeyNum == 4)	
        {
            if (chooseNum <= 0) chooseNum = 7;
            chooseNum--;
            OLED_SetTargetSelected(chooseNum);
            OLED_PlayEaseAnim();  // 【修改】调用缓动动画函数
        }

        // 按键1/2功能不变
        if (KeyNum == 1) {}
        if (KeyNum == 2) {}
    }
}

// 绘制主菜单图标（不变）
void OLED_DrawMainMenu(void)
{
    OLED_ShowImage(0, 22, 20, 20, A);
    OLED_ShowImage(21, 22, 20, 20, Dwave);
    OLED_ShowImage(42, 22, 20, 20, picture);
    OLED_ShowImage(63, 22, 20, 20, LED);
    OLED_ShowImage(84, 22, 20, 20, game);
    OLED_ShowImage(105, 22, 20, 20, usart);
}

// 设置目标位置（不变，仅将float类型适配）
void OLED_SetTargetSelected(uint8_t num)
{
    if (num == 0)
    {
        targetTriangleX = 0.0f;
    }
    else if (num >= 1 && num <= 6)
    {
        targetTriangleX = iconTriangleX[num];
    }
}

/**
 * @brief  【核心修改】播放"慢-快-慢"缓动动画
 * @param  无
 * @retval 无
 */
void OLED_PlayEaseAnim(void)
{
    // 循环直到当前位置与目标位置的误差≤0.5（确保精度）
    while (fabs(currentTriangleX - targetTriangleX) > 0.5f)
    {
        // 1. 清除上一帧的三角形和文字区域（优化效率，避免全清）
        OLED_ClearArea(0, 8, 128, 8);    // 清除三角形区域（Y=8-16）
        OLED_ClearArea(0, 44, 128, 16);  // 清除从X=0，Y=44开始，宽128像素，高16像素的区域（整行清除）
        OLED_DrawMainMenu();             // 重绘图标（避免被清除）

        // 2. 计算距离差和动态步长（核心：缓动算法）
        float diff = targetTriangleX - currentTriangleX;  // 距离差（可为正/负）
        float step = diff * easeFactor;                   // 动态步长 = 距离差 × 缓动系数

        // 3. 处理小步长（避免因系数过小导致停滞）
        if (fabs(step) < 0.5f)
        {
            step = (diff > 0) ? 0.5f : -0.5f;  // 最小步长0.5，确保向目标移动
        }

        // 4. 更新当前位置（累加步长）
        currentTriangleX += step;

        // 5. 绘制当前位置的三角形（将float转为uint8_t用于绘制）
        if (chooseNum >= 1 && chooseNum <= 6)
        {
            OLED_DrawTriangle((uint8_t)currentTriangleX, triangleY, 
                             (uint8_t)(currentTriangleX - 7), triangleY - 6, 
                             (uint8_t)(currentTriangleX + 7), triangleY - 6, 
                             OLED_FILLED);
        }

        // 6. 刷新屏幕并延时（固定帧间隔，确保流畅）
        OLED_Update();
        Delay_ms(frameDelay);
    }

    // 7. 动画结束：精确对齐目标位置，绘制最终选中效果
    currentTriangleX = targetTriangleX;
    OLED_Clear();
    OLED_DrawMainMenu();
    OLED_DrawSelectedEffect(chooseNum);
    OLED_Update();
}

// 绘制最终选中效果（不变）
void OLED_DrawSelectedEffect(uint8_t num)
{
    switch (num)
    {
        case 0:  break;
        case 1:  
            OLED_DrawTriangle(10, 14, 3, 8, 17, 8, OLED_FILLED);
            OLED_ShowString(35, 44, "AD模拟量", OLED_8X16);
            OLED_ReverseArea(0, 22, 20, 20);
            break;
        case 2:  
            OLED_DrawTriangle(31, 14, 24, 8, 38, 8, OLED_FILLED);
            OLED_ShowString(35, 44, "数字波形", OLED_8X16);
            OLED_ReverseArea(21, 22, 20, 20);
            break;
        case 3:  
            OLED_DrawTriangle(52, 14, 45, 8, 59, 8, OLED_FILLED);
            OLED_ShowString(50, 44, "图片", OLED_8X16);
            OLED_ReverseArea(42, 22, 20, 20);
            break;
        case 4:  
            OLED_DrawTriangle(73, 14, 66, 8, 80, 8, OLED_FILLED);
            OLED_ShowString(55, 44, "LED", OLED_8X16);
            OLED_ReverseArea(63, 22, 20, 20);
            break;
        case 5:  
            OLED_DrawTriangle(94, 14, 87, 8, 101, 8, OLED_FILLED);
            OLED_ShowString(50, 44, "游戏", OLED_8X16);
            OLED_ReverseArea(84, 22, 20, 20);
            break;
        case 6:  
            OLED_DrawTriangle(115, 14, 108, 8, 122, 8, OLED_FILLED);
            OLED_ShowString(40, 44, "串口打印", OLED_8X16);
            OLED_ReverseArea(105, 22, 20, 20);
            break;
    }
}
