#ifndef __LIGHT_SENSOR_H
#define __LIGHT_SENSOR_H

#include "sys.h"

// 初始化光敏电阻引脚定义（PA5 - ADC1_CH5）
#define LIGHT_SENSOR_PIN GPIO_Pin_5
#define LIGHT_SENSOR_PORT GPIOA

// 初始化光敏电阻
void Light_Sensor_Init(void);

// 读取光照强度（返回0-4095的ADC值，值越大光越强）
uint16_t Light_Sensor_Read(void);

// 将ADC值转换为光照强度百分比（0-100%）
uint8_t Light_Sensor_GetPercentage(void);

#endif
