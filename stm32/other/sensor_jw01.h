#ifndef __SENSOR_JW01_H
#define __SENSOR_JW01_H

#include "sys.h"

//传感器帧定义
#define SENSOR_FRAME_HEADER1 0x2C  //帧头第1字节
#define SENSOR_FRAME_HEADER2 0xE4  //帧头第2字节
#define SENSOR_FRAME_LEN     9

//函数声明
void JW01_Init(void);       //初始化传感器
u16 Get_CO2(void);          //获取CO₂浓度值(ppm)
float Get_TVOC(void);       //获取TVOC浓度值(mg/m³)
float Get_CH2O(void);       //获取甲醛浓度值(mg/m³)

#endif
