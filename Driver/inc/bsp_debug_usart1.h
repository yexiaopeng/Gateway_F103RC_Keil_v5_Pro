/*
 * bsp_debug_usart1.h
 *
 *  Created on: 2017年8月8日
 *      Author: Asuro
 */

#ifndef BSP_BSP_DEBUG_USART1_H_
#define BSP_BSP_DEBUG_USART1_H_

#include "stm32f10x.h"

extern  u8 usart_1_buffIndex;
extern  u8 usart_1_Buff[254];

extern char flashConcifgBuff[72];



#define DEBUG_USARTx                               USART1
#define DEBUG_USART1_BAUDRATE                      115200

#define DEBUG_USART1_ClockCmd                      RCC_APB2PeriphClockCmd
#define DEBUG_USART1_CLK                           RCC_APB2Periph_USART1

#define DEBUG_USART1_GPIO_ClockCmd                 RCC_APB2PeriphClockCmd
#define DEBUG_USART1_TX_PORT                       GPIOA
#define DEBUG_USART1_TX_PIN                        GPIO_Pin_9
#define DEBUG_USART1_TX_CLK                        RCC_APB2Periph_GPIOA

#define DEBUG_USART1_RX_PORT                       GPIOA
#define DEBUG_USART1_RX_PIN                        GPIO_Pin_10
#define DEBUG_USART1_RX_CLK                        RCC_APB2Periph_GPIOA

#define DEBUG_USART1_IRQHANDLER                    USART1_IRQHandler
#define DEBUG_USART1_IRQn                          USART1_IRQn
#define DEBUG_USART1_USARTx                        USART1

//************************** 函数声明
extern void DEBUG_USART1_Init(void);
extern void UART_PutChar(USART_TypeDef* USARTx, uint8_t Data);
extern void UART_PutStr (USART_TypeDef* USARTx, uint8_t *str);

//extern void SetFlashConfigFlag(u8 flag);
//extern u8   GetFlashConfigFlag();
#endif /* BSP_BSP_DEBUG_USART1_H_ */
