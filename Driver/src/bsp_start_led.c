/*
 * bsp_start_led.c
 *
 *  Created on: 2017年8月9日
 *      Author: Asuro
 */


/*********************************************************
 * PC6 -- ZIG_LED
 * PC7 -- WORK_LED
 * PC8 -- WIFI_LED
 * PC9 -- GPRS_LED
 *********************************************************/
#include "bsp_start_led.h"

extern void Start_Led_Init(void){
	/* 定义IO硬件初始化结构体变量 */
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);

	/* 设定USART发送对应IO编号 */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	/* 设定USART发送对应IO模式：复用推挽输出 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	/* 设定USART发送对应IO最大操作速度 ：GPIO_Speed_50MHz */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	/* 初始化USART发送对应IO */
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	Work_Led_Close();
	Zig_Led_Close();
	Gprs_Led_Close();
	Wifi_Led_Close();
}


extern void Work_Led_Open(void){
	// PC7 -- WORK_LED
	GPIO_ResetBits(GPIOC,GPIO_Pin_7);
}

extern void Work_Led_Close(void){
	// PC7 -- WORK_LED
	GPIO_SetBits(GPIOC,GPIO_Pin_7);
}


extern void Zig_Led_Open(void){
	//PC6 -- ZIG_LED
	GPIO_ResetBits(GPIOC,GPIO_Pin_6);
}

extern void Zig_Led_Close(void){
	GPIO_SetBits(GPIOC,GPIO_Pin_6);
}


extern void Gprs_Led_Open(void){
	//PC9 -- GPRS_LED
	GPIO_ResetBits(GPIOC,GPIO_Pin_9);
}

extern void Gprs_Led_Close(void){
	GPIO_SetBits(GPIOC,GPIO_Pin_9);
}


extern void Wifi_Led_Open(void){
	//PC8 -- WIFI_LED
	GPIO_ResetBits(GPIOC,GPIO_Pin_8);
}

extern void Wifi_Led_Close(void){
	GPIO_SetBits(GPIOC,GPIO_Pin_8);
}
