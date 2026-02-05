#include "stm32f10x.h"                  // Device header

/**
  * 函    数：AD初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：支持PA0、PB0、PC0三个模拟输入通道
  */
void AD_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	//开启ADC1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//开启GPIOA的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//开启GPIOB的时钟（新增）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	//开启GPIOC的时钟（新增）
	
	/*设置ADC时钟*/
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);						//选择时钟6分频，ADCCLK = 72MHz / 6 = 12MHz
	
	/*GPIO初始化*/
	// 配置PA0为模拟输入
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;			//模拟输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				//PA0引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//初始化PA0
	
	// 配置PB0为模拟输入（新增）
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				//PB0引脚
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//初始化PB0
	
	// 配置PC0为模拟输入（新增）
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				//PC0引脚
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//初始化PC0
	
	/*ADC初始化*/
	ADC_InitTypeDef ADC_InitStructure;						//定义结构体变量
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;		//独立模式（单独使用ADC1）
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//数据右对齐
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//软件触发
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;		//单次转换模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;			//非扫描模式
	ADC_InitStructure.ADC_NbrOfChannel = 1;					//通道数为1（非扫描模式固定为1）
	ADC_Init(ADC1, &ADC_InitStructure);						//配置ADC1
	
	/*ADC使能*/
	ADC_Cmd(ADC1, ENABLE);									//使能ADC1
	
	/*ADC校准*/
	ADC_ResetCalibration(ADC1);								//复位校准
	while (ADC_GetResetCalibrationStatus(ADC1) == SET);
	ADC_StartCalibration(ADC1);								//开始校准
	while (ADC_GetCalibrationStatus(ADC1) == SET);
}

/**
  * 函    数：获取AD转换的值
  * 参    数：ADC_Channel 指定AD转换的通道
  *           可选值：ADC_Channel_0（PA0）、ADC_Channel_8（PB0）、ADC_Channel_10（PC0）
  * 返 回 值：AD转换的值，范围：0~4095
  */
uint16_t AD_GetValue(uint8_t ADC_Channel)
{
	// 配置规则组通道（根据输入参数选择通道）
	ADC_RegularChannelConfig(ADC1, ADC_Channel, 1, ADC_SampleTime_55Cycles5);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);					//软件触发转换
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);	//等待转换完成
	return ADC_GetConversionValue(ADC1);					//返回转换结果
}
