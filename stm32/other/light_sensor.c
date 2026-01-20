#include "light_sensor.h"
#include "delay.h"
#include "stm32f10x_adc.h"   // 补充ADC相关定义

// 定义光敏传感器引脚和端口（根据实际硬件连接修改）
#define LIGHT_SENSOR_PORT    GPIOA
#define LIGHT_SENSOR_PIN     GPIO_Pin_5

// 初始化ADC用于读取光敏电阻
void Light_Sensor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    
    // 使能GPIOA和ADC1时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);
    
    // 配置PA5为模拟输入
    GPIO_InitStructure.GPIO_Pin = LIGHT_SENSOR_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  // 模拟输入模式
    GPIO_Init(LIGHT_SENSOR_PORT, &GPIO_InitStructure);
    
    // ADC配置
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  // 独立模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;       // 单通道模式
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;  // 单次转换模式
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;  // 软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;  // 右对齐
    ADC_InitStructure.ADC_NbrOfChannel = 1;  // 1个转换通道
    ADC_Init(ADC1, &ADC_InitStructure);
    
    // 使能ADC1
    ADC_Cmd(ADC1, ENABLE);
    
    // 校准ADC
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));  // 等待校准完成
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));      // 等待校准完成
}

// 读取ADC原始值
uint16_t Light_Sensor_Read(void)
{
    // 配置ADC通道5，采样时间为55.5周期
    ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 1, ADC_SampleTime_55Cycles5);
    
    // 启动ADC转换
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    // 等待转换完成
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    
    // 返回转换结果
    return ADC_GetConversionValue(ADC1);
}

// 转换为百分比（0-100%）
uint8_t Light_Sensor_GetPercentage(void)
{
    uint16_t adc_val = Light_Sensor_Read();
    // ADC最大值为4095，转换为0-100%
    return (uint8_t)((4095 - adc_val) * 100 / 4095);
}
