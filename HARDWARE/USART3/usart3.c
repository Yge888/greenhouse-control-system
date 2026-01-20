#include "usart3.h"	
#include <stdbool.h>
#include "sensor_jw01.h"  // 引入传感器头文件

// 声明外部变量（这些变量在sensor_jw01.c中定义）
extern u8 Usart3RecBuf[];    // 共享的接收缓冲区
extern u8 rev_complete;      // 接收完成标志位（在sensor_jw01.c中定义）

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
 
// 串口3初始化（仅保留硬件配置）
void USART3_Config(void){
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	//使能USART3时钟
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//使能GPIOB时钟
	
	//USART3_TX   GPIOB.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB.10
   
    //USART3_RX	  GPIOB.11初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PB11
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB.11  
 
  //Usart3 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;		//子优先级4
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//初始化VIC寄存器
  
   //USART 初始化设置
 
	USART_InitStructure.USART_BaudRate = 9600;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//1个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件流控
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
 
    USART_Init(USART3, &USART_InitStructure); //初始化串口3
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启接收中断
    USART_Cmd(USART3, ENABLE);                    //使能串口3 
}

// 获取接收完成标志
uint8_t Usart3_GetRxFlag(void)
{

    return rev_complete;
}
