//////////////////////////////////////////////////////////////////////////////////	 

//  功能描述   : OLED I2C接口显示程序(STM32F103系列)
//              说明: 
//              ----------------------------------------------------------------
//              GND  电源地
//              VCC  3.3v电源
//              D0   PA0(SCL)
//              D1   PA1(SDA)
//              RES  PA2
//              DC   PA3
//              CS   PA4 
//              按钮  PA6 (4脚按钮，一端接PA6，一端接地，通过内部上拉)
//              ----------------------------------------------------------------
//              添加与ESP32串口通信功能
//              STM32 TX（PA9） → ESP32 GPIO16 (RX2) 
//              STM32 RX （PA10）→ ESP32 GPIO17 (TX2) 
//******************************************************************************/

#include "delay.h"
#include "sys.h"
#include "oled.h"
#include "bmp.h"
#include "dht11.h"
#include "light_sensor.h"
#include "sensor_jw01.h"
#include "sg90_servo.h"
#include "timer.h"
#include "usart.h"  // 添加串口通信支持
#include "string.h"  // 添加字符串处理函数支持
#include "stepper.h" // 步进电机驱动

// 小尺寸OLED屏幕布局优化（适配更小屏幕，保持双列结构）
#define TITLE_X     10    // 标题X坐标（左移适配小屏幕）
#define TITLE_Y     0     // 标题Y坐标
#define LABEL_COL1_X 2    // 第一列标签X坐标（左移节省空间）
#define LABEL_COL2_X 58   // 第二列标签X坐标（左移调整）
#define VALUE_COL1_X 35   // 第一列数据X坐标（左移调整）
#define VALUE_COL2_X 90   // 第二列数据X坐标（左移调整）
#define ROW1_Y      14    // 第一行Y坐标（缩小行间距）
#define ROW2_Y      30    // 第二行Y坐标（缩小行间距）
#define STATUS_X    15    // 状态信息X坐标（左移调整）
#define STATUS_Y    48    // 状态信息Y坐标（上移调整）
#define HUMI_LOW_THRESHOLD 40 //湿度阈值——加湿器

// 按钮和舵机相关定义
#define BUTTON_PORT  GPIOA
#define BUTTON_PIN   GPIO_Pin_6  // 按钮引脚
#define TEMP_THRESHOLD 25  // 温度阈值，超过此值舵机动作
#define BUTTON_PRESS_LEVEL 0  // 按钮按下电平（低电平，因为使用内部上拉）
SG90_ServoTypeDef servo;   // 舵机结构体实例
// 加湿器引脚定义
#define HUMIDIFIER_PORT  GPIOB  // 假设使用GPIOB
#define HUMIDIFIER_PIN   GPIO_Pin_0  // 假设使用PB0作为加湿控制引脚

// 风扇引脚定义
#define FAN_PORT  GPIOB  // 假设使用GPIOB
#define FAN_PIN   GPIO_Pin_1  // 假设使用PB1作为风扇控制引脚

// 串口通信相关定义
#define ESP32_USART USART1
#define ESP32_BAUD_RATE 115200  // 与ESP32通信的波特率

// 定义ESP32控制命令
#define ESP32_CMD_FAN_ON  "FAN:ON"
#define ESP32_CMD_FAN_OFF "FAN:OFF"
#define ESP32_CMD_MOTOR_FORWARD "MOTOR:FORWARD"
#define ESP32_CMD_MOTOR_BACKWARD "MOTOR:BACKWARD"
#define ESP32_CMD_MOTOR_STOP "MOTOR:STOP"



// 函数声明
void Button_Init(void);
uint8_t Button_GetState(void);
void Humidifier_Init(void);
void Fan_Init(void);
void Fan_Control(uint8_t state);
void ESP32_Send_Data(char *data);
void Process_ESP32_Command(void);
void Check_ESP32_Command(void);

int main(void)
{
    u8 temp, humi;
    u8 read_err;
    uint16_t refresh_cnt = 0;
    uint8_t status_cnt = 0;
    uint8_t light_percent; 
    u16 co2_value;
    uint8_t servo_rotated = 0;
    uint8_t button_state = 0;
    uint8_t last_button_state = 0;
    uint8_t manual_control = 0;  // 手动控制标记 0-自动 1-手动
    uint16_t esp32_send_cnt = 0; // ESP32发送计数器，用于控制2秒发送一次
    char sensor_data[50]; // 用于存储格式化后的传感器数据

    // 初始化外设
    delay_init();
    OLED_Init();
    DHT11_Init();
    Light_Sensor_Init();
    JW01_Init();
    TIM3_PWM_Init(19999, 71);
    SG90_Init(&servo, TIM3, 2);
    Button_Init();  // 初始化按钮
	Humidifier_Init();  // 初始化加湿控制
    Fan_Init();  // 初始化风扇控制
    Stepper_Init();  // 初始化步进电机 (PB12-PB15)
    uart_init(ESP32_BAUD_RATE);  // 使用系统默认初始化，支持接收和中断
    OLED_Init();
    OLED_Clear();

    // 1. 显示标题（适配小屏幕缩短标题）
    OLED_ShowString(TITLE_X, TITLE_Y, "GREEN HOUSE", 12, 1);  // 更短的标题

    // 2. 显示固定标签（精简文字节省空间）
    // 第一列
    OLED_ShowString(LABEL_COL1_X, ROW1_Y, "Humi:", 12, 1);      // 简化标签
    OLED_ShowString(LABEL_COL1_X, ROW2_Y, "Temp:", 12, 1);      // 简化标签
    // 第二列
    OLED_ShowString(LABEL_COL2_X, ROW1_Y, "Light:", 12, 1);
    OLED_ShowString(LABEL_COL2_X, ROW2_Y, "CO2:", 12, 1);

    // 3. 显示单位（调整位置适配小屏幕）
    OLED_ShowChar(VALUE_COL1_X + 12, ROW1_Y, '%', 12, 1);  // 湿度单位
    OLED_ShowChar(VALUE_COL1_X + 12, ROW2_Y, 'C', 12, 1);  // 温度单位
    OLED_ShowChar(VALUE_COL2_X + 18, ROW1_Y, '%', 12, 1);  // 光照单位
    OLED_ShowString(VALUE_COL2_X + 18, ROW2_Y, "ppm", 12, 1);  // CO₂单位

    // 4. 初始化状态显示（调整位置）
    OLED_ShowString(STATUS_X, STATUS_Y, "Auto Mode", 12, 1);  // 简化状态文字
    
    OLED_Refresh();

    while(1)
    {
        // 读取按钮状态（消抖处理）
        button_state = Button_GetState();
        if(button_state != last_button_state)
        {
            delay_ms(20);  // 简单消抖
            if(button_state == BUTTON_PRESS_LEVEL)
            {
                // 按钮按下，切换手动/自动模式
                manual_control = !manual_control;
                if(manual_control)
                {
                    // 进入手动模式，切换舵机状态
                    servo_rotated = !servo_rotated;
                    SG90_SetAngle(&servo, servo_rotated ? 90 : 0);
                    OLED_ShowString(STATUS_X, STATUS_Y, "Manual Mode", 12, 1);
                }
                else
                {
                    // 回到自动模式
                    OLED_ShowString(STATUS_X, STATUS_Y, "Auto Mode ", 12, 1);
                }
                OLED_Refresh();
            }
            last_button_state = button_state;
        }
        
        // 每2秒向ESP32发送一次传感器数据 (200个delay_ms(10)循环 = 2秒)
        if(esp32_send_cnt >= 500)
        {
            // 格式化传感器数据：TEMP:xx,HUMI:xx,LIGHT:xx,CO2:xxxx\n
            sprintf(sensor_data, "TEMP:%d,HUMI:%d,LIGHT:%d,CO2:%d\n", temp, humi, light_percent, co2_value);
            ESP32_Send_Data(sensor_data);
            esp32_send_cnt = 0;
        }
        esp32_send_cnt++;

        // 每隔1秒更新数据
        if(refresh_cnt >= 100)
        {
            // 读取传感器数据
            read_err = DHT11_Read_Data(&temp, &humi);
            light_percent = Light_Sensor_GetPercentage();
            co2_value = Get_CO2();

            // 清除上一次数据显示区域
            OLED_ShowString(VALUE_COL1_X, ROW1_Y, "  ", 12, 1);  // 湿度
            OLED_ShowString(VALUE_COL1_X, ROW2_Y, "  ", 12, 1);  // 温度
            OLED_ShowString(VALUE_COL2_X, ROW1_Y, "  ", 12, 1);  // 光照
            OLED_ShowString(VALUE_COL2_X, ROW2_Y, "    ", 12, 1);  // CO₂
            
            // 根据读取结果显示
            if(read_err == 0)
            {
                // 显示传感器数据
                OLED_ShowNum(VALUE_COL1_X, ROW1_Y, humi, 2, 12, 1);
                OLED_ShowNum(VALUE_COL1_X, ROW2_Y, temp, 2, 12, 1);
                OLED_ShowNum(VALUE_COL2_X, ROW1_Y, light_percent, 2, 12, 1);
                OLED_ShowNum(VALUE_COL2_X, ROW2_Y, co2_value, 4, 12, 1);
                
                // 自动模式下的舵机控制逻辑
                if(!manual_control)
                {
                    if (temp > TEMP_THRESHOLD) {
                        if (!servo_rotated) {
                            SG90_SetAngle(&servo, 90);
                            servo_rotated = 1;
                        }
                    } else {
                        if (servo_rotated) {
                            SG90_SetAngle(&servo, 0);
                            servo_rotated = 0;
                        }
                    }
										
										// 湿度控制逻辑
										if (humi < HUMI_LOW_THRESHOLD) {
												// 湿度低于阈值，开启加湿
												GPIO_SetBits(HUMIDIFIER_PORT, HUMIDIFIER_PIN);

										} else {
												// 湿度足够，关闭加湿
												GPIO_ResetBits(HUMIDIFIER_PORT, HUMIDIFIER_PIN);
										}
										
                }
                
                // 更新状态显示
                if(status_cnt > 0)
                {
                    if(manual_control)
                        OLED_ShowString(STATUS_X, STATUS_Y, "Manual Mode", 12, 1);
                    else
                        OLED_ShowString(STATUS_X, STATUS_Y, "Auto Mode ", 12, 1);
                    status_cnt = 0;
                }
                
                // 传感器数据已显示在OLED上，不需要发送到ESP32
            }
            else
            {
                // 显示错误信息
                OLED_ShowString(VALUE_COL1_X, ROW1_Y, "ER", 12, 1);
                OLED_ShowString(VALUE_COL1_X, ROW2_Y, "ER", 12, 1);
                OLED_ShowString(VALUE_COL2_X, ROW1_Y, "ER",12, 1);
                OLED_ShowString(VALUE_COL2_X, ROW2_Y, "ER", 12, 1);
                
                // 显示错误状态
                OLED_ShowString(STATUS_X, STATUS_Y, "sensor Err", 12, 1);  // 简化错误文字
                status_cnt++;
            }
            
            OLED_Refresh();
            refresh_cnt = 0;
        }
        refresh_cnt++;
        // 处理ESP32发送的命令
        Process_ESP32_Command();
        
        delay_ms(10);
    }
	
}

// 按钮初始化函数（使用内部上拉电阻）
void Button_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  // 使能PA端口时钟
    
    GPIO_InitStructure.GPIO_Pin = BUTTON_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BUTTON_PORT, &GPIO_InitStructure);
}

// 加湿控制引脚初始化
void Humidifier_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  // 使能PB端口时钟
    
    GPIO_InitStructure.GPIO_Pin = HUMIDIFIER_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(HUMIDIFIER_PORT, &GPIO_InitStructure);
    
    // 初始关闭加湿
    GPIO_ResetBits(HUMIDIFIER_PORT, HUMIDIFIER_PIN);
}

// 获取按钮状态
uint8_t Button_GetState(void)
{
    return GPIO_ReadInputDataBit(BUTTON_PORT, BUTTON_PIN);
}

// 发送数据到ESP32
// 发送数据到ESP32（修复发送阻塞问题）
void ESP32_Send_Data(char *data)
{
    while(*data)  // 遍历字符串中的每个字符
    {
        // 等待发送数据寄存器为空（USART_FLAG_TXE），而非发送完成（TC）
        while(USART_GetFlagStatus(ESP32_USART, USART_FLAG_TXE) == RESET); 
        USART_SendData(ESP32_USART, *data++);  // 发送当前字符
        // 等待当前字符发送完成（可选，确保连续发送不丢包）
        while(USART_GetFlagStatus(ESP32_USART, USART_FLAG_TC) == RESET);
        USART_ClearFlag(ESP32_USART, USART_FLAG_TC);  // 清除发送完成标志
    }
}

// 风扇初始化函数
void Fan_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  // 使能PB端口时钟
    
    GPIO_InitStructure.GPIO_Pin = FAN_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(FAN_PORT, &GPIO_InitStructure);
    
    // 初始关闭风扇
    GPIO_ResetBits(FAN_PORT, FAN_PIN);
}

// 风扇控制函数
// state: 0-关闭风扇, 1-开启风扇
void Fan_Control(uint8_t state)
{
    if(state)
    {
        GPIO_SetBits(FAN_PORT, FAN_PIN);  // 开启风扇
    }
    else
    {
        GPIO_ResetBits(FAN_PORT, FAN_PIN);  // 关闭风扇
    }
}

// 处理ESP32发送的控制命令（兼容旧接口）
void Process_ESP32_Command(void)
{
    Check_ESP32_Command();
}

// 检查是否接收到来自ESP32的命令
// 此函数将在主循环中调用，使用usart.c中的USART_RX_BUF和USART_RX_STA来检查命令
void Check_ESP32_Command(void)
{
    // 检查是否接收到完整的数据帧（由usart.c中的中断处理函数设置）
    if(USART_RX_STA & 0x8000)  // 判断接收是否完成
    {
        // 处理接收到的数据
        if(strstr((char*)USART_RX_BUF, "FAN:ON") != NULL)
        {
            Fan_Control(1);  // 开启风扇
            ESP32_Send_Data("FAN:ON_ACK\n");  // 发送确认
        }
        else if(strstr((char*)USART_RX_BUF, "FAN:OFF") != NULL)
        {
            Fan_Control(0);  // 关闭风扇
            ESP32_Send_Data("FAN:OFF_ACK\n");  // 发送确认
        }
        else if(strstr((char*)USART_RX_BUF, "MOTOR:FORWARD") != NULL)
        {
            Stepper_Move(0, 2048, 3);  // 正转2048步 (半周)，延时3ms确保转矩
            ESP32_Send_Data("MOTOR:FORWARD_ACK\n");  // 发送确认
        }
        else if(strstr((char*)USART_RX_BUF, "MOTOR:BACKWARD") != NULL)
        {
            Stepper_Move(1, 2048, 3);  // 反转2048步 (半周)，延时3ms确保转矩
            ESP32_Send_Data("MOTOR:BACKWARD_ACK\n");  // 发送确认
        }
        else if(strstr((char*)USART_RX_BUF, "MOTOR:STOP") != NULL)
        {
            Stepper_Stop();  // 停止电机
            ESP32_Send_Data("MOTOR:STOP_ACK\n");  // 发送确认
        }
        
        // 清空接收缓冲区和状态
        USART_RX_STA = 0;
        memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));
    }
}