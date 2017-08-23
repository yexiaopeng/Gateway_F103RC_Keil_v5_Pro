/*
 * bsp_debug_usart1.c
 *
 *  Created on: 2017��8��8��
 *      Author: Asuro
 */

#include "bsp_debug_usart1.h"
#include <stdio.h>
#include "stm32f10x_usart.h"





static void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	 /* Ƕ�������жϿ�������ѡ�� */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
   /* ����USARTΪ�ж�Դ */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	 /* �������ȼ�Ϊ0 */
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	 /* �����ȼ�Ϊ1 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	 /* ʹ���ж� */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	 /* ��ʼ������NVIC */
	NVIC_Init(&NVIC_InitStructure);
}

extern void DEBUG_USART1_Init(void){
	/* ����IOӲ����ʼ���ṹ����� */
	GPIO_InitTypeDef GPIO_InitStructure;
	/* ����USART��ʼ���ṹ����� */
	USART_InitTypeDef USART_InitStructure;


	NVIC_Configuration();

	/* ʹ��USARTʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	/* ʹ��USART����GPIOʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO,ENABLE);

	/* ����USART����GPIO��ʼ�� */
	/* �趨USART���Ͷ�ӦIO��� */
	GPIO_InitStructure.GPIO_Pin =  DEBUG_USART1_TX_PIN;
	/* �趨USART���Ͷ�ӦIOģʽ������������� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	/* �趨USART���Ͷ�ӦIO�������ٶ� ��GPIO_Speed_50MHz */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	/* ��ʼ��USART���Ͷ�ӦIO */
	GPIO_Init(DEBUG_USART1_TX_PORT, &GPIO_InitStructure);

	/* �趨USART���ն�ӦIO��� */
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART1_RX_PIN;
	/* �趨USART���Ͷ�ӦIOģʽ���������� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	/* ����û�����¸�ֵ�ĳ�Աʹ���봮�ڷ�����ͬ���� */
	/* ��ʼ��USART���ն�ӦIO */
	GPIO_Init(DEBUG_USART1_RX_PORT, &GPIO_InitStructure);

	/* USART������������ */
	/* USART������:115200 */
	USART_InitStructure.USART_BaudRate = DEBUG_USART1_BAUDRATE;
	/* USART�ֳ�(��Чλ)��8λ */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	/* USARTֹͣλ��1λ */
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	/* USARTУ��λ���� */
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	/* USARTӲ������������(Ӳ���źſ��ƴ���ֹͣ)���� */
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	/* USART����ģʽʹ�ܣ�������պͷ��� */
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	/* ��ʼ��USART */
	USART_Init(USART1, &USART_InitStructure);

	/* ʹ�ܽ����ж� */
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	/* ʹ��USART */
	USART_Cmd(USART1, ENABLE);

	/* ���������ɱ�־ */
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
  * ��������: �ض���c�⺯��printf��USARTx
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
int fputc(int ch, FILE *f)
{
  /* ����һ���ֽ����ݵ����Դ��� */
  USART_SendData(DEBUG_USARTx, (uint8_t) ch);

  /* �ȴ��������ݷ������ */
  while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);

  return (ch);
}

/**
  * ��������: �ض���c�⺯��getchar,scanf��USARTx
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
int fgetc(FILE *f)
{
  /* �ȴ������������� */
  while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_RXNE) == RESET);

  return (int)USART_ReceiveData(DEBUG_USARTx);
}
