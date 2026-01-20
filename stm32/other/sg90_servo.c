#include "sg90_servo.h"

/**
 * @brief  初始化舵机
 * @param  servo: 舵机结构体指针
 * @param  TIMx: 定时器（如TIM2、TIM3等）
 * @param  channel: PWM通道（1-4）
 * @retval 无
 */
void SG90_Init(SG90_ServoTypeDef *servo, TIM_TypeDef *TIMx, uint8_t channel) {
    servo->TIMx = TIMx;
    servo->channel = channel;
    servo->angle = 0;
    
    // 启动PWM输出
    TIM_Cmd(TIMx, ENABLE);
    if(channel == 1) TIM_CCxCmd(TIMx, TIM_Channel_1, TIM_CCx_Enable);
    else if(channel == 2) TIM_CCxCmd(TIMx, TIM_Channel_2, TIM_CCx_Enable);
    else if(channel == 3) TIM_CCxCmd(TIMx, TIM_Channel_3, TIM_CCx_Enable);
    else if(channel == 4) TIM_CCxCmd(TIMx, TIM_Channel_4, TIM_CCx_Enable);
    
    // 初始化为0度
    SG90_SetAngle(servo, 0);
}

/**
 * @brief  设置舵机角度
 * @param  servo: 舵机结构体指针
 * @param  angle: 目标角度(0-180)
 * @retval 成功返回1，失败返回0
 */
uint8_t SG90_SetAngle(SG90_ServoTypeDef *servo, uint16_t angle) {
    uint32_t pulse_width;
    
    // 检查角度是否在有效范围内
    if (angle < SERVO_MIN_ANGLE || angle > SERVO_MAX_ANGLE) {
        return 0;
    }
    
    // 计算脉冲宽度: 从角度映射到脉冲宽度
    pulse_width = SERVO_MIN_PULSE + 
                  (SERVO_MAX_PULSE - SERVO_MIN_PULSE) * angle / 
                  (SERVO_MAX_ANGLE - SERVO_MIN_ANGLE);
    
    // 设置CCR寄存器值
    switch(servo->channel) {
        case 1: TIM_SetCompare1(servo->TIMx, pulse_width); break;
        case 2: TIM_SetCompare2(servo->TIMx, pulse_width); break;
        case 3: TIM_SetCompare3(servo->TIMx, pulse_width); break;
        case 4: TIM_SetCompare4(servo->TIMx, pulse_width); break;
        default: return 0;
    }
    
    // 更新当前角度
    servo->angle = angle;
    return 1;
}

/**
 * @brief  获取当前舵机角度
 * @param  servo: 舵机结构体指针
 * @retval 当前角度
 */
uint16_t SG90_GetAngle(SG90_ServoTypeDef *servo) {
    return servo->angle;
}
// 确保文件末尾有一个空行
    