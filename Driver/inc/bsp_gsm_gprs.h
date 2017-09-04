#ifndef __BSP_GSM_GPRS_H
#define __BSP_GSM_GPRS_H

#include "stm32f10x.h"
#include "bsp_SysTick.h"
#include "bsp_gsm_usart.h"


typedef enum{
    GSM_TRUE,
    GSM_FALSE,
    
}gsm_res_e;


typedef enum
{
    GSM_NULL                = 0,
    GSM_CMD_SEND            = '\r',         
    GSM_DATA_SEND           = 0x1A,         //发送数据(ctrl + z)
    GSM_DATA_CANCLE         = 0x1B,         //发送数据(Esc)    
}gsm_cmd_end_e;



extern  uint8_t     gsm_cmd         	(char *cmd, char *reply,uint32_t waittime );
extern  uint8_t     gsm_cmd_check   	(char *reply);

/***********************************GPRS 开关机  ***********************************************/
#define GSM_POWER_GPIO_ClockCmd                	  RCC_APB2PeriphClockCmd
#define GSM_POWER_GPIO_PORT                      GPIOA
#define GSM_POWER_GPIO_PIN                       GPIO_Pin_0
#define GSM_POWER_GPIO_CLK                       RCC_APB2Periph_GPIOA

//开关机
#define GSM_PWRKEY_GPIO_ClockCmd                 RCC_APB2PeriphClockCmd
#define GSM_PWRKEY_GPIO_PORT                     GPIOA
#define GSM_PWRKEY_GPIO_PIN                      GPIO_Pin_1
#define GSM_PWRKEY_GPIO_CLK                      RCC_APB2Periph_GPIOA
extern void GSM_POWER_Init(void);
extern void GSM_POWER_Open(void);
extern void GSM_POWER_Close(void);

extern void GSM_PWRKEY_Init(void);
extern void GSM_PWRKEY_Open(void);
extern void GSM_PWRKEY_Close(void);















/***********************************************************************************************/
#define     GSM_CLEAN_RX()              clean_rebuff()
#define     gsm_ate0()                  gsm_cmd("ATE0\r","OK",100)              //关闭回显
#define     GSM_TX(cmd)                	GSM_USART_printf("%s",cmd)
#define     GSM_IS_RX()                 (USART_GetFlagStatus(GSM_USARTx, USART_FLAG_RXNE) != RESET)
#define     GSM_RX(len)                 ((char *)get_rebuff(&(len)))
#define     GSM_DELAY(time)             Delay_ms(time)                 //延时
#define     GSM_SWAP16(data)    				 __REVSH(data)



/*************************** GPRS TCP***************************/
uint8_t 	gsm_gprs_init		(void);																//GPRS初始化环境
uint8_t gsm_gprs_tcp_link	(char *localport,char * serverip,char * serverport);				//TCP连接
uint8_t gsm_gprs_udp_link	(char *localport,char * serverip,char * serverport);				//UDP连接
uint8_t gsm_gprs_send		(const char * str);														//发送数据
uint8_t gsm_gprs_link_close	(void);              												//IP链接断开
uint8_t gsm_gprs_shut_close	(void);               												//关闭场景
uint8_t	PostGPRS(void);

/*************************************************************/




/************************************************************/






extern u8  gsm_gprs_Get_SignalStrength();


extern void Restart_Device(void);


/*寮�鍏虫満*/

#define GSM_DEBUG_ON         	0
#define GSM_DEBUG_ARRAY_ON    0
#define GSM_DEBUG_FUNC_ON   	0
// Log define
#define GSM_INFO(fmt,arg...)           printf("<<-GSM-INFO->> "fmt"\n",##arg)
#define GSM_ERROR(fmt,arg...)          printf("<<-GSM-ERROR->> "fmt"\n",##arg)
#define GSM_DEBUG(fmt,arg...)          do{\
                                         if(GSM_DEBUG_ON)\
                                         printf("<<-GSM-DEBUG->> [%d]"fmt"\n",__LINE__, ##arg);\
																					}while(0)

#define GSM_DEBUG_ARRAY(array, num)    do{\
                                         int32_t i;\
                                         uint8_t* a = array;\
                                         if(GSM_DEBUG_ARRAY_ON)\
                                         {\
                                            printf("<<-GSM-DEBUG-ARRAY->> [%d]\n",__LINE__);\
                                            for (i = 0; i < (num); i++)\
                                            {\
                                                printf("%02x   ", (a)[i]);\
                                                if ((i + 1 ) %10 == 0)\
                                                {\
                                                    printf("\n");\
                                                }\
                                            }\
                                            printf("\n");\
                                        }\
                                       }while(0)

#define GSM_DEBUG_FUNC()               do{\
                                         if(GSM_DEBUG_FUNC_ON)\
                                         printf("<<-GSM-FUNC->> Func:%s@Line:%d\n",__func__,__LINE__);\
                                       }while(0)



#endif