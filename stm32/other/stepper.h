#ifndef __STEPPER_H
#define __STEPPER_H

#include "stm32f10x.h"

// 使用 PB12-PB15，避开 JTAG 冲突
#define STEPPER_PORT    GPIOB
#define STEPPER_RCC     RCC_APB2Periph_GPIOB
#define IN1_PIN         GPIO_Pin_12
#define IN2_PIN         GPIO_Pin_13
#define IN3_PIN         GPIO_Pin_14
#define IN4_PIN         GPIO_Pin_15

// 初始化步进电机引脚
void Stepper_Init(void);

// 电机控制函数
// direction: 0-顺时针, 1-逆时针
// steps: 步数
// delay_ms: 每步延时（控制速度，建议 > 2ms）
void Stepper_Move(uint8_t direction, uint32_t steps, uint32_t nms);

// 停止电机（断电以防发热）
void Stepper_Stop(void);

#endif
