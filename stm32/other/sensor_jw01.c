#include "sensor_jw01.h"
#include "usart3.h"
#include "delay.h"
#include "sys.h"
#include <stdint.h>  // 确保包含标准整数类型定义

// 接收缓冲区（B1-B9）
uint8_t Usart3RecBuf[SENSOR_FRAME_LEN];
// 接收完成标志
uint8_t rev_complete = 0;

// 初始化JW01传感器
void JW01_Init(void)
{
    USART3_Config();  // 配置串口3用于传感器通信
}

// 校验和计算
static uint8_t CalculateChecksum(void)
{
    uint8_t sum = 0;
    uint8_t i;  // C89标准：在循环外声明变量
    // 累加前8字节计算校验和
    for(i = 0; i < SENSOR_FRAME_LEN - 1; i++)
    {
        sum += Usart3RecBuf[i];
    }
    return sum;
}

// 从接收缓冲区解析CO2值 (ppm)
uint16_t Get_CO2(void)
{
    uint16_t co2 = 11;
    if(rev_complete)  // 接收完成且校验通过
    {
        // CO2数据在B7(高8位)和B8(低8位)
        uint16_t raw_data = (Usart3RecBuf[6] << 8) | Usart3RecBuf[7];
        co2 = raw_data;  // 协议中CO2单位为ppm，无需乘以系数
        rev_complete = 0;  // 重置接收标志
    }
    return co2;
}

// 从接收缓冲区解析TVOC值 (mg/m³)
float Get_TVOC(void)
{
    float tvoc = 0.0f;
    if(rev_complete)
    {
        // TVOC数据在B3(高8位)和B4(低8位)
        uint16_t raw_data = (Usart3RecBuf[2] << 8) | Usart3RecBuf[3];
        tvoc = raw_data * 0.001f;  // 转换为mg/m³
    }
    return tvoc;
}

// 从接收缓冲区解析甲醛(CH2O)值 (mg/m³)
float Get_CH2O(void)
{
    float ch2o = 0.0f;
    if(rev_complete)
    {
        // 甲醛数据在B5(高8位)和B6(低8位)
        uint16_t raw_data = (Usart3RecBuf[4] << 8) | Usart3RecBuf[5];
        ch2o = raw_data * 0.001f;  // 转换为mg/m³
    }
    return ch2o;
}

// 串口3中断服务函数（接收传感器数据）
void USART3_IRQHandler(void)
{
    static uint8_t rec_index = 0;
    uint8_t data;
    
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        data = USART_ReceiveData(USART3);

        // 双帧头检测逻辑
        if(rec_index == 0)
        {
            // 第一字节必须为0x2C
            if(data == SENSOR_FRAME_HEADER1)
            {
                Usart3RecBuf[rec_index++] = data;
            }
            else
            {
                rec_index = 0;  // 帧头错误，重置
            }
        }
        else if(rec_index == 1)
        {
            // 第二字节必须为0xE4
            if(data == SENSOR_FRAME_HEADER2)
            {
                Usart3RecBuf[rec_index++] = data;
            }
            else
            {
                rec_index = 0;  // 帧头错误，重置
            }
        }
        // 接收后续数据
        else if(rec_index < SENSOR_FRAME_LEN)
        {
            Usart3RecBuf[rec_index++] = data;
            
            // 判断是否接收完一帧
            if(rec_index == SENSOR_FRAME_LEN)
            {
                // 校验和验证
                if(CalculateChecksum() == Usart3RecBuf[8])
                {
                    rev_complete = 1;  // 校验通过，标记完成
                }
                rec_index = 0;  // 重置接收索引
            }
        }
        else
        {
            rec_index = 0;  // 异常情况重置
        }
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}
