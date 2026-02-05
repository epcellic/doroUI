#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "math.h"
#include "AD.h"
#include "Serial.h"

/**---------------------------------doroUI V1.6 bata--------------------------------
---------------------------------个人作者：千分硬件坊-------------------------------
---------------------------------Github开源链接：后续更新---------------------------
-------------------------------特别鸣谢：江协科技库函数源码--------------------------------
----------------------------未经允许 禁止转载 禁止商业牟利等行为--------------------**/

// 全局变量
uint8_t KeyNum;
uint8_t chooseNum = 0;
uint8_t Serialmode = 0;
int gamemode = 0;
float targetTriangleX = 10.0f;// 三角形目标X（float提高精度）
float currentTriangleX = 10.0f;// 三角形当前X
const uint8_t triangleY = 14; // 三角形固定Y坐标
const float easeFactor = 0.2f;// 缓动系数（0.1-0.3）
const uint16_t frameDelay = 5;// 动画帧间隔（ms）

char recvFrame[FRAME_MAX_LEN];// 用于存储从串口获取的帧数据

// 6个图标的三角形目标X坐标（与图标中心对齐）
const float iconTriangleX[7] = {0.0f, 10.0f, 31.0f, 52.0f, 73.0f, 94.0f, 115.0f};


uint16_t AD1, AD2, AD3;	//定义AD值变量


// 页面状态枚举（明确当前处于哪个页面）
typedef enum {
    PAGE_MAIN_MENU,  // 主菜单页面
    PAGE_AD,         // 二级页面：AD模拟量
    PAGE_DWAVE,      // 二级页面：数字波形
    PAGE_PICTURE,    // 二级页面：图片
    PAGE_LED,        // 二级页面：LED控制
    PAGE_GAME,       // 二级页面：游戏
    PAGE_USART       // 二级页面：串口打印
} PageType;
PageType currentPage = PAGE_MAIN_MENU; // 当前页面（默认主菜单）

// 函数声明
void OLED_DrawMainMenu(void);                  // 绘制主菜单
void OLED_SetTargetSelected(uint8_t num);      // 设置三角形目标位置
void OLED_PlayEaseAnim(void);                  // 三角形缓动动画
void OLED_DrawSelectedEffect(uint8_t num);     // 主菜单选中效果
// 二级页面相关函数
void OLED_DrawSubPage(PageType page);          // 绘制指定二级页面
void OLED_HandleSubPageInput(PageType page);   // 处理二级页面按键输入
void OLED_ReturnToMainMenu(void);              // 返回主菜单




int main(void)
{
    // 初始化与开机动画（原有逻辑不变）
    OLED_Init();
    Key_Init();
    AD_Init();
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
    OLED_ShowString(30, 50, "Ver.1.6 Bata", OLED_6X8);
    OLED_Update();
    Delay_ms(500);
    OLED_Clear();OLED_Update();

    OLED_ShowString(0, 54, "Welcome!", OLED_6X8);
    OLED_DrawMainMenu();
    OLED_Update();
    Delay_ms(300);



    while (1)
    {
        KeyNum = Key_GetNum();  
				//调试：显示当前按键值（仅测试时使用）
			  OLED_Printf(122,1, OLED_6X8,"%d", KeyNum);
				OLED_Update();
			
        if (currentPage == PAGE_MAIN_MENU)//按键处理：用于选择页面，确定及返回键
        {
            if (KeyNum == 3)	
            {
                chooseNum++;
                if (chooseNum >= 7) chooseNum = 1;
                OLED_SetTargetSelected(chooseNum);
                OLED_PlayEaseAnim();
            }
            if (KeyNum == 4)
            {
                if (chooseNum <= 0) chooseNum = 7;
                chooseNum--;
                OLED_SetTargetSelected(chooseNum);
                OLED_PlayEaseAnim();
            }
            if (KeyNum == 1)	
            {
                switch (chooseNum)
                {
                    case 1: currentPage = PAGE_AD; break;
                    case 2: currentPage = PAGE_DWAVE; break;
                    case 3: currentPage = PAGE_PICTURE; break;
                    case 4: currentPage = PAGE_LED; break;
                    case 5: currentPage = PAGE_GAME; break;
                    case 6: currentPage = PAGE_USART; break;
                    default: break; 
                }
                OLED_Clear();       // 清空屏幕准备绘制二级页面
                OLED_DrawSubPage(currentPage); // 绘制当前二级页面
                OLED_Update();
            }
        }
        else
        {
            if (KeyNum == 2)	//按键2作为“返回键”，回到主菜单
            {
                OLED_ReturnToMainMenu();
            }
            else	// 处理当前二级页面的逻辑（进入了三级界面）（如LED控制、参数调节等）
            {
                OLED_HandleSubPageInput(currentPage);
            }
        }
    }
}






// 绘制主菜单图标
void OLED_DrawMainMenu(void)
{
    OLED_ShowImage(0, 22, 20, 20, A);
    OLED_ShowImage(21, 22, 20, 20, Dwave);
    OLED_ShowImage(42, 22, 20, 20, picture);
    OLED_ShowImage(63, 22, 20, 20, LED);
    OLED_ShowImage(84, 22, 20, 20, game);
    OLED_ShowImage(105, 22, 20, 20, usart);
}



// 设置三角形目标位置
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


// 三角形缓动动画
void OLED_PlayEaseAnim(void)
{
    while (fabs(currentTriangleX - targetTriangleX) > 0.5f)
    {
        OLED_ClearArea(0, 8, 128, 8);    // 清除三角形区域
        OLED_ClearArea(0, 44, 128, 16);  // 清除文字区域
        OLED_DrawMainMenu();             // 重绘图标

        float diff = targetTriangleX - currentTriangleX;
        float step = diff * easeFactor;
        if (fabs(step) < 0.5f)
        {
            step = (diff > 0) ? 0.5f : -0.5f;
        }
        currentTriangleX += step;

        if (chooseNum >= 1 && chooseNum <= 6)
        {
            OLED_DrawTriangle((uint8_t)currentTriangleX, triangleY, 
                             (uint8_t)(currentTriangleX - 7), triangleY - 6, 
                             (uint8_t)(currentTriangleX + 6), triangleY - 6, 
                             OLED_FILLED);
        }
        OLED_Update();
        Delay_ms(frameDelay);
    }

    currentTriangleX = targetTriangleX;
    OLED_Clear();
    OLED_DrawMainMenu();
    OLED_DrawSelectedEffect(chooseNum);
    OLED_Update();
}



// 主菜单选中效果
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
            OLED_ShowString(50, 44, "相册", OLED_8X16);
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
            OLED_ShowString(35, 44, "串口控制", OLED_8X16);
            OLED_ReverseArea(105, 22, 20, 20);
            break;
    }
}




//二级页面仅绘制，无逻辑
void OLED_DrawSubPage(PageType page)
{
    OLED_ShowString(1, 1, "<< back:key2", OLED_6X8);
    switch (page)
    {
        case PAGE_AD:
					  OLED_ShowImage(1, 10, 30, 40,ADhelp);
            OLED_ShowString(36, 14, "请照图连接", OLED_8X16);
				    OLED_ShowString(36, 33, "范围:", OLED_8X16); 
            OLED_ShowString(83, 33, "0-4096", OLED_6X8); 
            OLED_ShowString(75, 42, "(0-3.3v)", OLED_6X8);
            OLED_ShowString(1, 55, "enter:key1", OLED_6X8);
            break;
        
        case PAGE_DWAVE:
            OLED_ShowString(0, 14, "数字开关波形", OLED_8X16);
            OLED_DrawLine(0, 33, 118, 33); // 绘制波形基线
            OLED_ShowString(1, 55, "enter:key1", OLED_6X8);
				    OLED_ShowString(60, 44, "LOW_on:PC13", OLED_6X8);
						OLED_ShowString(66, 55, "UP_on:PC14", OLED_6X8);	
            break;
        
        case PAGE_PICTURE:
            OLED_ShowString(30, 15, "相册 v1.0", OLED_8X16);
            OLED_ShowImage(40, 30, 38, 38, doro3); // 需替换为你的图片数组
				    OLED_ShowString(1, 55, "enter:key1", OLED_6X8);
            break;
        
        case PAGE_LED:
            OLED_ShowString(30, 15, "LED控制", OLED_8X16);
            OLED_ShowString(10, 35, "待完善", OLED_8X16); // 占位，后续可接GPIO状态
            OLED_ShowString(1, 55, "swtich:key1", OLED_6X8);
            break;
        
        case PAGE_GAME:
            OLED_ShowString(30, 15, "小游戏", OLED_8X16);
            OLED_ShowString(1, 35, "更多正在开发中..", OLED_8X16);
            OLED_ShowString(1, 55, "enter:key1", OLED_6X8);
            break;
        
        case PAGE_USART:
            OLED_ShowString(1, 15, "串口控制", OLED_8X16);
			    	OLED_ShowString(1, 35, "PA9(RX)-TX", OLED_6X8);
				 	  OLED_ShowString(1, 44, "PA10(TX)-RX", OLED_6X8);
	          OLED_ReverseArea(1, 15, 16*4, 16);
				    OLED_ShowImage(77, 20, 50, 40,Usart);
            OLED_ShowString(1, 55, "enter:key1", OLED_6X8);
            break;
        
        default:
            OLED_Clear();  
			    	OLED_ReturnToMainMenu();
            break;
    }
}




//三级页面（每个功能暂时分配一个）
void OLED_HandleSubPageInput(PageType page)
{
    if (KeyNum != 0) // 有按键按下时才处理
    {
        switch (page)
        {
            case PAGE_AD://AD页面逻辑
                if (KeyNum == 1) // 按键1：启用刷新AD值
                {
									OLED_Clear();                 // 清空屏幕
									OLED_ShowString(1, 1, "<<OFF:k2||ON:k1", OLED_6X8);
									while(1)
									{
										KeyNum = Key_GetNum();  
										//调试：显示当前按键值（仅测试时使用）
										OLED_Printf(122,1, OLED_6X8,"%d", KeyNum);
										OLED_Update();
										AD1 = AD_GetValue(ADC_Channel_0);		//单次启动ADC，转换通道1
										AD2 = AD_GetValue(ADC_Channel_8);		//单次启动ADC，转换通道2
										AD3 = AD_GetValue(ADC_Channel_10);		//单次启动ADC，转换通道3
										// 前10行已显示固定内容，从第11行开始布局AD数据
										// 列1：起始X=0（左起第1列，占42像素）
										OLED_Printf(10, 12, OLED_6X8,  "AD1");      // 第1行：AD标识（6X8）
										OLED_Printf(4, 20, OLED_8X16, "%04d", AD1); // 第2行：AD数值（8X16，X=0对齐）
										OLED_Printf(1, 37, OLED_8X16, "%.2fv", (float)AD1 / 4095 * 3.3); // 第3行：电压值（8X16）
										OLED_Printf(10, 56, OLED_6X8,  "PA0");      // 第4行：引脚（6X8，底部对齐64像素）
										// 列2：起始X=43（中间列，与第1列间隔1像素）
										OLED_Printf(55, 12, OLED_6X8,  "AD2");      // 第1行：AD标识
										OLED_Printf(49, 20, OLED_8X16, "%04d", AD2); // 第2行：AD数值
										OLED_Printf(44, 37, OLED_8X16, "%.2fv", (float)AD2 / 4095 * 3.3); // 第3行：电压值
										OLED_Printf(55, 56, OLED_6X8,  "PB0");      // 第4行：引脚
										// 列3：起始X=86（右列，与第2列间隔1像素）
										OLED_Printf(97, 12, OLED_6X8,  "AD3");      // 第1行：AD标识
										OLED_Printf(92, 20, OLED_8X16, "%04d", AD3); // 第2行：AD数值
										OLED_Printf(87, 37, OLED_8X16, "%.2fv", (float)AD3 / 4095 * 3.3); // 第3行：电压值
										OLED_Printf(97, 56, OLED_6X8,  "PC0");      // 第4行：引脚
										// 绘制表格
										OLED_DrawLine(0, 10, 127, 10);
										OLED_DrawLine(0, 63, 127, 63);
										OLED_DrawLine(0, 20, 127, 19);
										OLED_DrawLine(0, 36, 127, 36);
										OLED_DrawLine(0, 55, 127, 55);
										OLED_DrawLine(0, 10, 0, 64);
										OLED_DrawLine(127, 10, 127, 63);
										OLED_DrawLine(42, 10, 42, 63);
										OLED_DrawLine(85, 10, 85, 63);
                    OLED_Update();
										if (KeyNum ==2)break;//由于下次未刷新，默认会卡在当前页面并暂停，再次点击会退出
								  }
								}
                break;
	
						case PAGE_DWAVE://数字波形页面逻辑
								if (KeyNum == 1) 
								{
										OLED_Clear();
										OLED_ShowString(1, 1, "<<OFF:k2||ON:k1", OLED_6X8);

										// 绘制波形坐标轴（整体下移10像素）
										OLED_DrawLine(10, 42, 118, 42);  // 基线(0电平)，原32+10=42
										OLED_DrawLine(10, 25, 10, 59);   // 纵轴，原15+10=25，原49+10=59
										OLED_ShowString(2, 25, "1", OLED_6X8);  // 高电平标识，原15+10=25
										OLED_ShowString(2, 42, "0", OLED_6X8);  // 低电平标识，原32+10=42
										// 波形数据缓冲区和位置指针
										uint8_t waveBuffer[108] = {0};  // 存储波形数据(108个点)
										uint8_t wavePos = 0;            // 当前波形绘制位置
										// 速度控制参数
										const uint8_t step = 1;         // 每次移动的点数
										const uint16_t refreshDelay = 1;// 刷新间隔(ms)
										while(1)
										{
												KeyNum = Key_GetNum();  
												// 显示当前按键值
												OLED_Printf(122, 1, OLED_6X8, "%d", KeyNum);
												
												// 一次更新多个点
												for(uint8_t s = 0; s < step; s++)
												{
														waveBuffer[wavePos] = (KeyNum == 5 || KeyNum == 6) ? 1 : 0;
														wavePos = (wavePos + 1) % 108;  // 循环更新位置
												}
												
												// 清除上一帧波形（区域也下移10像素）
												OLED_ClearArea(10, 25, 108, 34);  // 原15+10=25
												
												// 绘制波形
												for(uint8_t i = 0; i < 107; i++)
												{
														// 计算当前点在缓冲区中的实际索引
														uint8_t bufferIndex1 = (wavePos + i) % 108;
														uint8_t bufferIndex2 = (wavePos + i + 1) % 108;
														
														// 计算显示位置（Y坐标下移10像素）
														uint8_t x1 = 10 + i;
														uint8_t x2 = 10 + i + 1;
														uint8_t y1 = waveBuffer[bufferIndex1] ? 25 : 42;  // 原15+10=25，原32+10=42
														uint8_t y2 = waveBuffer[bufferIndex2] ? 25 : 42;  // 同上
														
														// 绘制线段
														OLED_DrawLine(x1, y1, x2, y2);
												}
										OLED_ShowString(25,54, "<-<-<-<-<-<-<", OLED_6X8);
												OLED_Update();
												if (KeyNum == 2) break; 
												Delay_ms(refreshDelay);
										}
								}
								break;
								
					  case PAGE_PICTURE://相册页面逻辑
								if (KeyNum == 1) 
								{
										OLED_Clear();
										OLED_ShowString(1, 1, "<<OFF:k2||ON:k1", OLED_6X8);

										// 绘制波形坐标轴（整体下移10像素）
										OLED_DrawLine(10, 42, 118, 42);  // 基线(0电平)，原32+10=42
										OLED_DrawLine(10, 25, 10, 59);   // 纵轴，原15+10=25，原49+10=59
										OLED_ShowString(2, 25, "1", OLED_6X8);  // 高电平标识，原15+10=25
										OLED_ShowString(2, 42, "0", OLED_6X8);  // 低电平标识，原32+10=42
										// 波形数据缓冲区和位置指针
										uint8_t waveBuffer[108] = {0};  // 存储波形数据(108个点)
										uint8_t wavePos = 0;            // 当前波形绘制位置
										// 速度控制参数
										const uint8_t step = 2;         // 每次移动的点数
										const uint16_t refreshDelay = 5;// 刷新间隔(ms)
										while(1)
										{
												KeyNum = Key_GetNum();  
												// 显示当前按键值
												OLED_Printf(122, 1, OLED_6X8, "%d", KeyNum);
												
												// 一次更新多个点
												for(uint8_t s = 0; s < step; s++)
												{
														waveBuffer[wavePos] = (KeyNum != 0) ? 1 : 0;
														wavePos = (wavePos + 1) % 108;  // 循环更新位置
												}
												
												// 清除上一帧波形（区域也下移10像素）
												OLED_ClearArea(10, 25, 108, 34);  // 原15+10=25
												
												// 绘制波形
												for(uint8_t i = 0; i < 107; i++)
												{
														// 计算当前点在缓冲区中的实际索引
														uint8_t bufferIndex1 = (wavePos + i) % 108;
														uint8_t bufferIndex2 = (wavePos + i + 1) % 108;
														
														// 计算显示位置（Y坐标下移10像素）
														uint8_t x1 = 10 + i;
														uint8_t x2 = 10 + i + 1;
														uint8_t y1 = waveBuffer[bufferIndex1] ? 25 : 42;  // 原15+10=25，原32+10=42
														uint8_t y2 = waveBuffer[bufferIndex2] ? 25 : 42;  // 同上
														
														// 绘制线段
														OLED_DrawLine(x1, y1, x2, y2);
												}
										OLED_ShowString(25,54, "<-<-<-<-<-<-<", OLED_6X8);
												OLED_Update();
												if (KeyNum == 2) break; 
												Delay_ms(refreshDelay);
										}
								}
								break;	
						
            case PAGE_LED:
                if (KeyNum == 1) // 按键1：切换LED状态（示例，需配合GPIO代码）
                {
                    static uint8_t ledState = 0;
                    ledState = !ledState;
                    // 实际项目中添加GPIO控制：GPIO_WriteBit(GPIOx, GPIO_Pin_x, ledState);
									  OLED_ShowString(10, 35, "LED1: ", OLED_8X16);
                    OLED_ShowString(50, 35, ledState ? "开启" : "关闭", OLED_8X16);
                    OLED_Update();
                }
                break;
            

            case PAGE_GAME://内含双while，便于绘制四级页面
							if (KeyNum == 1) 
								{
									  Serial_Init();
										OLED_Clear();
										OLED_ShowString(1, 1, "<<OFF:k2||ON:k1", OLED_6X8);			
                    OLED_ShowString(20, 15, "请选择游戏", OLED_8X16);
									  OLED_ShowString(75, 34, "Aliya", OLED_8X16);
									  OLED_ShowString(80, 54, "key4>>", OLED_6X8);//如果需要添加游戏，请在这里进行排版图标或动画及名称,此处为静态，如需动态页面，需要把动态元素写在while里面并及时更新清除
									  OLED_DrawLine(68, 34, 68, 63);
									  OLED_Update();
									
										while(1)//第一个while，选择对应的游戏并设置模式，方便下一个while明白进入了哪个游戏逻辑
										{
											KeyNum = Key_GetNum();  //目前因为游戏逻辑比较少，所以暂且用按键代表进入了什么游戏，后续需要改为选择->变量->确定->根据变量进入对应页面
											if(KeyNum==4)
										  {			//目前默认进入第一个游戏，定义gamemode为1，默认什么都不做为0					
											gamemode = 1;
											break;
										  }
										}

										while(1)
										{
											KeyNum = Key_GetNum(); 
										if(gamemode == 1)
											{
												//游戏1的逻辑在这里，会循环执行
												
												
												OLED_Update();
											}

										else if(gamemode == 0)break;
	
												if (KeyNum == 2) {
													gamemode = 0;
													break; 
												}
								    }
								}
								break;
						
						
						
            case PAGE_USART:
							if (KeyNum == 1) 
								{
									  Serial_Init();
										OLED_Clear();
										OLED_ShowString(1, 1, "<<OFF:k2||ON:k1", OLED_6X8);
									
                    OLED_ShowString(20, 15, "请选择模式", OLED_8X16);
									  OLED_ShowString(1, 34, "视频投屏", OLED_8X16);
									OLED_ShowString(1, 54, "<<key3", OLED_6X8);
									  OLED_ShowString(75, 34, "数据帧", OLED_8X16);
									OLED_ShowString(80, 54, "key4>>", OLED_6X8);
									  OLED_DrawLine(68, 34, 68, 63);
									
									  OLED_Update();
									
										while(1)
										{
											KeyNum = Key_GetNum();  
											
											
											if(KeyNum==3){										
											Serial_SetMode(SERIAL_MODE_OLED);
        							Serial_SendString("当前模式：OLED显存模式（接收数据直接显示）\r\n");
											Serialmode = 1;
											break;
											}
											if(KeyNum==4){										
											Serial_SetMode(SERIAL_MODE_FRAME);
											Serial_SendString("当前模式：#!帧模式（接收#...!格式数据）\r\n");
											Serialmode = 2;
											break;
											}	
										  if(KeyNum==2){										
											Serialmode = 3;
											break;
											}	
										}

										while(1)
										{
											KeyNum = Key_GetNum(); 

										if(Serialmode == 1){
												OLED_Update();
											}else if(Serialmode == 2){
																					if (Serial_GetFrameData(recvFrame) == 1){
																							Serial_SendString("收到帧数据：");
																							Serial_SendString(recvFrame);
																							Serial_SendString("\r\n");
																						}
											}else if(Serialmode == 3)break;
	
												if (KeyNum == 2) {
													Serial_DeInit();
													Serial_SetMode(SERIAL_MODE_NONE);
													Serialmode = 0;
													break; 
												}
								    }
								}
								break;
								
            default:
                break;
        }
        KeyNum = 0; // 处理完按键后清零，避免重复触发
    }
}



// 返回主菜单（统一清除+重绘主菜单）
void OLED_ReturnToMainMenu(void)
{
    currentPage = PAGE_MAIN_MENU; // 更新页面状态为“主菜单”
    OLED_Clear();                 // 清空屏幕
    OLED_DrawMainMenu();          // 重绘主菜单图标
    OLED_DrawSelectedEffect(chooseNum); // 保留之前选中的图标效果
    OLED_Update();                // 刷新显示
}

