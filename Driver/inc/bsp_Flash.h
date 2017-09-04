/*********************************************
* bsp_Flash.h
* 内部Flash操作
* 三生未尽祈战死，化作忠魂常护国
**********************************************/

#ifndef BSP_FLASH_H_
#define BSP_FLASH_H_

#include "stm32f10x.h"


extern char ip[15] ;
extern char  port[5];
extern char  deviceId[8];
extern char  serialNumber[20];


extern void FLASH_Check();


extern uint16_t FLASH_ReadHalfWord(uint32_t address);
extern uint32_t FLASH_ReadWord(uint32_t address);
extern void FLASH_ReadMoreData(uint32_t startAddress,uint16_t *readData,uint16_t countToRead);
extern void FLASH_WriteMoreData(uint32_t startAddress,uint16_t *writeData,uint16_t countToWrite); 

//璇诲啓璁惧淇℃伅
extern void FLASH_WriteDeviceAllConfig(char *ip,char * port,char * id,char * serialNumber);
extern void FLASH_ReadDeviceAllConfig(char * ip,char* port,char * id,char * serialNumber);
extern void FLASH_WriteIpConfig(char *ip,char * port);
extern void FLASH_ReadIpConfig(char ** ip, char ** port);
extern void FLASH_WriteDeviceIdConfig(char * id);
extern void FLASH_ReadDeviceIdConfig(char ** id);
extern void FLASH_WriteDeviceSerialNumber(char * serialNumber);
extern void FLASH_ReadDeviceSerialNumber(char ** serialNumber);

extern void FLASH_USART_ConfigDeviceParameter(char * usartBuff,u8 length);


#endif
