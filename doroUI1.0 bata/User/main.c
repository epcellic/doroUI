#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "Key.h"

// 全局变量
uint8_t KeyNum;
uint8_t chooseNum = 0;  // 0-未选中，1-6对应6个图标

// 函数声明
void OLED_DrawMainMenu(void);                  // 绘制主菜单所有图标
void OLED_UpdateSelectedState(uint8_t num);    // 更新选中状态（三角形+文字+反色）

int main(void)
{
    OLED_Init();
    Key_Init();

    // 开机动画（保持不变）
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
    //精简信息
	  OLED_ShowString(15, 10, "EPCELLIC 出品", OLED_8X16);
	  OLED_ShowString(45, 50, "Ver.1.0", OLED_6X8);
	  OLED_Update();
    Delay_ms(1500);
	  OLED_Clear();OLED_Update();
	
    // 初始绘制主菜单
    OLED_DrawMainMenu();
    OLED_Update();
    Delay_ms(500);

    while (1)
    {
        KeyNum = Key_GetNum();  // 获取按键键码

        // 按键3：选择下一个图标（1→6循环）
        if (KeyNum == 3)	
        {
            chooseNum++;
            if (chooseNum >= 7) chooseNum = 1;  // 超过6则回到1
            OLED_UpdateSelectedState(chooseNum);
        }

        // 按键4：选择上一个图标（6→1循环）
        if (KeyNum == 4)	
        {
            if (chooseNum <= 0) chooseNum = 7;  // 小于1则跳到7，再减1到6
            chooseNum--;
            OLED_UpdateSelectedState(chooseNum);
        }

        // 此处可添加按键1/2的功能（如确认选择、返回等）
        if (KeyNum == 1)
        {
            // 示例：根据选中的图标执行对应功能
            // switch(chooseNum) { case 1: /*AD模拟量功能*/ break; ... }
        }
        if (KeyNum == 2)	
        {
            // 示例：返回未选中状态
            // chooseNum = 0;
            // OLED_UpdateSelectedState(chooseNum);
        }

        OLED_Update();
    }
}

/**
 * @brief  绘制主菜单所有图标（无选中状态）
 * @param  无
 * @retval 无
 */
void OLED_DrawMainMenu(void)
{
    OLED_ShowImage(0, 22, 20, 20, A);
    OLED_ShowImage(21, 22, 20, 20, Dwave);
    OLED_ShowImage(42, 22, 20, 20, picture);
    OLED_ShowImage(63, 22, 20, 20, LED);
    OLED_ShowImage(84, 22, 20, 20, game);
    OLED_ShowImage(105, 22, 20, 20, usart);
}

/**
 * @brief  更新选中状态（清除屏幕→重绘图标→添加选中效果）
 * @param  num: 选中的图标编号（0=未选中，1-6=对应图标）
 * @retval 无
 */
void OLED_UpdateSelectedState(uint8_t num)
{
    OLED_Clear();          // 清除当前屏幕
    OLED_DrawMainMenu();   // 重绘所有图标

    // 根据选中编号添加对应效果
    switch (num)
    {
        case 0:  // 未选中状态（仅图标，无其他效果）
            break;
        
        case 1:  // A图标（AD模拟量）
            OLED_DrawTriangle(10, 14, 3, 8, 17, 8, OLED_FILLED);
            OLED_ShowString(35, 44, "AD模拟量", OLED_8X16);
            OLED_ReverseArea(0, 22, 20, 20);
            break;
        
        case 2:  // Dwave图标（数字波形）
            OLED_DrawTriangle(31, 14, 24, 8, 38, 8, OLED_FILLED);
            OLED_ShowString(35, 44, "数字波形", OLED_8X16);
            OLED_ReverseArea(21, 22, 20, 20);
            break;
        
        case 3:  // picture图标（图片）
            OLED_DrawTriangle(52, 14, 45, 8, 59, 8, OLED_FILLED);
            OLED_ShowString(50, 44, "图片", OLED_8X16);
            OLED_ReverseArea(42, 22, 20, 20);
            break;
        
        case 4:  // LED图标（LED）
            OLED_DrawTriangle(73, 14, 66, 8, 80, 8, OLED_FILLED);
            OLED_ShowString(55, 44, "LED", OLED_8X16);
            OLED_ReverseArea(63, 22, 20, 20);
            break;
        
        case 5:  // game图标（游戏）
            OLED_DrawTriangle(94, 14, 87, 8, 101, 8, OLED_FILLED);
            OLED_ShowString(50, 44, "游戏", OLED_8X16);
            OLED_ReverseArea(84, 22, 20, 20);
            break;
        
        case 6:  // usart图标（串口打印）
            OLED_DrawTriangle(115, 14, 108, 8, 122, 8, OLED_FILLED);
            OLED_ShowString(40, 44, "串口打印", OLED_8X16);
            OLED_ReverseArea(105, 22, 20, 20);
            break;
    }
}







/**
  * 坐标轴定义：
  * 左上角为(0, 0)点
  * 横向向右为X轴，取值范围：0~127
  * 纵向向下为Y轴，取值范围：0~63
  * 
  *       0             X轴           127 
  *      .------------------------------->
  *    0 |
  *      |
  *      |
  *      |
  *  Y轴 |
  *      |
  *      |
  *      |
  *   63 |
  *      v
  * 
  */
