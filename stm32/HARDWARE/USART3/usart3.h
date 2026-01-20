#ifndef __USART3_H
#define __USART3_H
 
#include "stm32f10x.h"                  // Device header
#include "oled.h"
#include "usart.h"
 
// 项目信息（保持不变）
/*****************辰哥单片机设计******************
											STM32
 * 项目			:	JW01三合一传感器实验                     
 * 版本			: V1.0
 * 日期			: 2025.2.4
 * MCU			:	STM32F103C8T6
 * 接口			:	串口3						
 * BILIBILI	:	辰哥单片机设计
 * CSDN			:	辰哥单片机设计
 * 作者			:	辰哥 
**********************BEGIN***********************/
 
#define USART3_RXBUFF_SIZE   54  //可保持不变，实际使用SENSOR_FRAME_LEN定义
 
void USART3_Config(void);
uint8_t Usart3_GetRxFlag(void);
float Get_CH2O(void);  //已在sensor_jw01.c中实现
float Get_TVOC(void);  //新增声明

#endif