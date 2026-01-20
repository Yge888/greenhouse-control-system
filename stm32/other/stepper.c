#include "stepper.h"
#include "delay.h"

// 8步序列（半步模式，转矩大且平稳）
// A-AB-B-BC-C-CD-D-DA
static uint16_t phase8[] = {
    IN1_PIN,
    IN1_PIN | IN2_PIN,
    IN2_PIN,
    IN2_PIN | IN3_PIN,
    IN3_PIN,
    IN3_PIN | IN4_PIN,
    IN4_PIN,
    IN4_PIN | IN1_PIN
};

void Stepper_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    // 开启 GPIOB 和 AFIO 复用时钟
    RCC_APB2PeriphClockCmd(STEPPER_RCC | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitStructure.GPIO_Pin = IN1_PIN | IN2_PIN | IN3_PIN | IN4_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(STEPPER_PORT, &GPIO_InitStructure);

    Stepper_Stop();
}

void Stepper_SetPhase(uint8_t step) {
    // 先清除所有引脚
    GPIO_ResetBits(STEPPER_PORT, IN1_PIN | IN2_PIN | IN3_PIN | IN4_PIN);
    // 设置对应相位的引脚
    GPIO_SetBits(STEPPER_PORT, phase8[step % 8]);
}

void Stepper_Move(uint8_t direction, uint32_t steps, uint32_t nms) {
    static uint8_t current_step = 0;
    uint32_t i;
    
    for (i = 0; i < steps; i++) {
        if (direction == 0) { // 顺时针
            current_step++;
            if (current_step >= 8) current_step = 0;
        } else { // 逆时针
            if (current_step == 0) current_step = 7;
            else current_step--;
        }
        
        Stepper_SetPhase(current_step);
        delay_ms(nms); // 使用目的地项目的 delay_ms (小写)
    }
}

void Stepper_Stop(void) {
    GPIO_ResetBits(STEPPER_PORT, IN1_PIN | IN2_PIN | IN3_PIN | IN4_PIN);
}
