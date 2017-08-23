/*
 * bsp_debug_usart1.c
 *
 *  Created on: 2017年8月8日
 *      Author: Asuro
 */

#include "bsp_debug_usart1.h"
#include <stdio.h>
#include "stm32f10x_usart.h"





static void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	 /* 嵌套向量中断控制器组选择 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
   /* 配置USART为中断源 */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	 /* 抢断优先级为0 */
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	 /* 子优先级为1 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	 /* 使能中断 */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	 /* 初始化配置NVIC */
	NVIC_Init(&NVIC_InitStructure);
}

extern void DEBUG_USART1_Init(void){
	/* 定义IO硬件初始化结构体变量 */
	GPIO_InitTypeDef GPIO_InitStructure;
	/* 定义USART初始化结构体变量 */
	USART_InitTypeDef USART_InitStructure;


	NVIC_Configuration();

	/* 使能USART时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	/* 使能USART功能GPIO时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO,ENABLE);

	/* 调试USART功能GPIO初始化 */
	/* 设定USART发送对应IO编号 */
	GPIO_InitStructure.GPIO_Pin =  DEBUG_USART1_TX_PIN;
	/* 设定USART发送对应IO模式：复用推挽输出 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	/* 设定USART发送对应IO最大操作速度 ：GPIO_Speed_50MHz */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	/* 初始化USART发送对应IO */
	GPIO_Init(DEBUG_USART1_TX_PORT, &GPIO_InitStructure);

	/* 设定USART接收对应IO编号 */
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART1_RX_PIN;
	/* 设定USART发送对应IO模式：浮空输入 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	/* 其他没有重新赋值的成员使用与串口发送相同配置 */
	/* 初始化USART接收对应IO */
	GPIO_Init(DEBUG_USART1_RX_PORT, &GPIO_InitStructure);

	/* USART工作环境配置 */
	/* USART波特率:115200 */
	USART_InitStructure.USART_BaudRate = DEBUG_USART1_BAUDRATE;
	/* USART字长(有效位)：8位 */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	/* USART停止位：1位 */
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	/* USART校验位：无 */
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	/* USART硬件数据流控制(硬件信号控制传输停止)：无 */
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	/* USART工作模式使能：允许接收和发送 */
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	/* 初始化USART */
	USART_Init(USART1, &USART_InitStructure);

	/* 使能接收中断 */
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	/* 使能USART */
	USART_Cmd(USART1, ENABLE);

	/* 清除发送完成标志 */
	USART_ClearFlag(USART1, USART_FLAG_TC|USART_FLAG_TXE|USART_FLAG_RXNE);

}


extern void UART_PutChar(USART_TypeDef* USARTx, uint8_t Data)
{
    USART_SendData(USARTx, Data);
    while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET){}
}

extern void UART_PutStr (USART_TypeDef* USARTx, uint8_t *str)
{
    while (0 != *str)
    {
        UART_PutChar(USARTx, *str);
        str++;
    }
}

/**
  * 函数功能: 重定向c库函数printf到USARTx
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
int fputc(int ch, FILE *f)
{
  /* 发送一个字节数据到调试串口 */
  USART_SendData(DEBUG_USARTx, (uint8_t) ch);

  /* 等待串口数据发送完毕 */
  while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);

  return (ch);
}

/**
  * 函数功能: 重定向c库函数getchar,scanf到USARTx
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
int fgetc(FILE *f)
{
  /* 等待串口输入数据 */
  while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_RXNE) == RESET);

  return (int)USART_ReceiveData(DEBUG_USARTx);
}
