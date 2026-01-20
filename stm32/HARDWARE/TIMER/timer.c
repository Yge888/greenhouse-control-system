#include "timer.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

/**
 * @brief  初始化TIM3为PWM模式
 * @param  arr: 自动重装载值
 * @param  psc: 预分频系数
 * @retval 无
 */
void TIM3_PWM_Init(u16 arr, u16 psc) {
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // 使能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

    // 配置TIM3_CH2 (PA7) 为复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  // 复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 初始化TIM3
    TIM_TimeBaseStructure.TIM_Period = arr;  // 自动重装载值
    TIM_TimeBaseStructure.TIM_Prescaler = psc;  // 预分频系数
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;  // 时钟分割
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  // 向上计数模式
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    // 配置TIM3_CH2为PWM模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;  // PWM模式1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;  // 输出使能
    TIM_OCInitStructure.TIM_Pulse = 0;  // 初始脉冲宽度
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;  // 输出极性高
    TIM_OC2Init(TIM3, &TIM_OCInitStructure);

    // 使能预装载寄存器
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);

    // 启动定时器
    TIM_Cmd(TIM3, ENABLE);
}
// 确保文件末尾有一个空行
    