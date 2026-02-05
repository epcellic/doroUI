#include "stm32f10x.h"                  // Device header
#include "Delay.h"

/**
  * 函    数：按键初始化
  * 参    数：无
  * 返 回 值：无
  * 说明：配置 PC6~PC9、PC13 为上拉输入，PC14为下拉输入，对应6个按键
  */
void Key_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);		//开启GPIOC的时钟
	
	/*GPIO初始化 - PC6~PC9、PC13 上拉输入*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;				//上拉输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_13;	//C6~C9、C13引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;				//GPIO速度
	GPIO_Init(GPIOC, &GPIO_InitStructure);						//初始化GPIOC
	
	/*GPIO初始化 - PC14 下拉输入*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;				//下拉输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;					//C14引脚
	GPIO_Init(GPIOC, &GPIO_InitStructure);						//初始化GPIOC
}

/**
  * 函    数：按键获取键码
  * 参    数：无
  * 返 回 值：按下按键的键码值，范围：0~6，返回0代表没有按键按下
  *           1-PC6按键，2-PC7按键，3-PC8按键，4-PC9按键
  *           5-PC13按键（上拉输入），6-PC14按键（下拉输入）
  * 注意事项：阻塞式操作，按键按住不放时函数会卡住，直到按键松手
  */
uint8_t Key_GetNum(void)
{
	uint8_t KeyNum = 0;		//定义变量，默认键码值为0
	
	//检测PC6按键（对应键码1）
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6) == 0)			
	{
		Delay_ms(20);											//延时消抖
		while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6) == 0);	//等待按键松手
		Delay_ms(20);											//延时消抖
		KeyNum = 1;												//置键码为1
	}
	
	//检测PC7按键（对应键码2）
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7) == 0)			
	{
		Delay_ms(20);											
		while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7) == 0);	
		Delay_ms(20);											
		KeyNum = 2;												
	}
	
	//检测PC8按键（对应键码3）
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8) == 0)			
	{
		Delay_ms(20);											
		while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8) == 0);	
		Delay_ms(20);											
		KeyNum = 3;												
	}
	
	//检测PC9按键（对应键码4）
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9) == 0)			
	{
		Delay_ms(20);											
		while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9) == 0);	
		Delay_ms(20);											
		KeyNum = 4;												
	}
	
	//检测PC13按键（对应键码5，上拉输入按下时为低电平）
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 0)			
	{
		Delay_ms(20);											//延时消抖
		while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 0);	//等待按键松手
		Delay_ms(20);											//延时消抖
		KeyNum = 5;												//置键码为5
	}
	
	//检测PC14按键（对应键码6，下拉输入按下时为高电平）
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_14) == 1)			
	{
		Delay_ms(20);											//延时消抖
		while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_14) == 1);	//等待按键松手
		Delay_ms(20);											//延时消抖
		KeyNum = 6;												//置键码为6
	}
	
	return KeyNum;			//返回键码值，无按键按下时返回0
}
