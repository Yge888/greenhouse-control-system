#ifndef __SG90_SERVO_H
#define __SG90_SERVO_H

#include "sys.h"
#include "stm32f10x_tim.h"  // 添加定时器头文件

// 舵机角度范围定义
#define SERVO_MIN_ANGLE     0
#define SERVO_MAX_ANGLE     180

// 舵机PWM信号范围(微秒)
#define SERVO_MIN_PULSE     500     // 0度对应脉冲
#define SERVO_MAX_PULSE     2500    // 180度对应脉冲

// 舵机结构体定义
typedef struct {
    TIM_TypeDef *TIMx;          // 定时器指针
    uint8_t channel;            // PWM通道（1-4）
    uint16_t angle;             // 当前角度
} SG90_ServoTypeDef;

// 函数声明
void SG90_Init(SG90_ServoTypeDef *servo, TIM_TypeDef *TIMx, uint8_t channel);
uint8_t SG90_SetAngle(SG90_ServoTypeDef *servo, uint16_t angle);
uint16_t SG90_GetAngle(SG90_ServoTypeDef *servo);

#endif /* __SG90_SERVO_H */
// 确保文件末尾有一个空行
    