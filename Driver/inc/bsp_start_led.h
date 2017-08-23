/*
 * bsp_start_led.h
 *
 *  Created on: 2017Äê8ÔÂ9ÈÕ
 *      Author: Asuro
 */

#ifndef BSP_BSP_START_LED_H_
#define BSP_BSP_START_LED_H_

/*********************************************************
 * PC6 -- ZIG_LED
 * PC7 -- WORK_LED
 * PC8 -- WIFI_LED
 * PC9 -- GPRS_LED
 *********************************************************/


#include "stm32f10x.h"


extern void Start_Led_Init(void);


extern void Work_Led_Open(void);

extern void Work_Led_Close(void);


extern void Zig_Led_Open(void);

extern void Zig_Led_Close(void);


extern void Gprs_Led_Open(void);

extern void Gprs_Led_Close(void);


extern void Wifi_Led_Open(void);

extern void Wifi_Led_Close(void);







#endif /* BSP_BSP_START_LED_H_ */
