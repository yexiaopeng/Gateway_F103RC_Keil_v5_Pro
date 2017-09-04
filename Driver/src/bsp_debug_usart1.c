/*
 * bsp_debug_usart1.c
 *
 *  Created on: 2017年8月8日
 *      Author: Asuro
 */

#include "bsp_debug_usart1.h"
#include <stdio.h>
#include "stm32f10x_usart.h"



//全局变量 
u8 usart_1_buffIndex;
u8 usart_1_Buff[254];

char flashConcifgBuff[72];


static u8 FlashConfigFlag = 0;


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


/*
	1  -- 处理网络中断
	2  -- 先服务器发包
	3  -- 处理串口FLASH配置
*/
//extern void SetFlashConfigFlag(u8 flag){
//	FlashConfigFlag = flag;
//}
//extern u8   GetFlashConfigFlag(){
//	return FlashConfigFlag;
//}


extern void Clear_Usart1_Rec(){
	memset(usart_1_Buff,0,254);
	usart_1_buffIndex = 0 ;
}

//串口1中断处理函数
//配置设备参数
//共字节
//FF  AB 为头   数据长度 71byte     结尾 表示port长度(0x04        Or  0x05)和命令
//数据依次为 ip port(4byte) id serialnumber port长度 命令
//当port长度为4  倒数3byte为0  	长度为5 倒数3byte为port第5byte
//结尾为配置选项目
void USART1_IRQHandler(void){
	if (USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET){
		 usart_1_Buff[usart_1_buffIndex] = USART_ReceiveData(USART1);
		 usart_1_buffIndex++;

		 if(usart_1_buffIndex >= 253){
		  usart_1_buffIndex=0;
		 }
	
		
#if 1
		if( usart_1_buffIndex >=   71 ){
			 
			//匹配包头
			 u8 i;
			
			if( (usart_1_Buff[usart_1_buffIndex-71] == 0XFF)    && 
					(usart_1_Buff[usart_1_buffIndex-70] == 0XAB) ){
				//匹配结尾   0x04 或 0x05
				if(usart_1_Buff[usart_1_buffIndex-2] == 0x04 || usart_1_Buff[usart_1_buffIndex-2] == 0x05){
					//接受完毕
					
					memcpy(flashConcifgBuff,&usart_1_Buff[usart_1_buffIndex-71],71);
					flashConcifgBuff[71] = '\0';
					usart_1_buffIndex = 0;

					//for(i = 0; i < 71;i++){
					//	printf(" 0X%02X ",flashConcifgBuff[i]);
					//}
					
					SetIsReceiveGsmData(3);
				}
			}
		}
#endif
	#if 0	
		 //找包尾巴
		if( (usart_1_Buff[usart_1_buffIndex-1] == 0x04) && (usart_1_Buff[usart_1_buffIndex - 2] == 0x00) 
					&& (usart_1_Buff[usart_1_buffIndex - 3] == 0x30) && (usart_1_Buff[usart_1_buffIndex - 4] == 0x30) ){
			//配置FLASH参数
			FLASH_Config(usart_1_Buff,usart_1_buffIndex-1);
			SetFlashConfigFlag(1);
		}
	#endif				
	}
}


