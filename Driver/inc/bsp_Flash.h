/*********************************************
* bsp_Flash.h
* 内部Flash操作
* 三生未尽祈战死，化作忠魂常护国
**********************************************/

#ifndef BSP_FLASH_H_
#define BSP_FLASH_H_

#include "stm32f10x.h"

extern void FLASH_Check();


extern uint16_t FLASH_ReadHalfWord(uint32_t address);
extern uint32_t FLASH_ReadWord(uint32_t address);
extern void FLASH_ReadMoreData(uint32_t startAddress,uint16_t *readData,uint16_t countToRead);
extern void FLASH_WriteMoreData(uint32_t startAddress,uint16_t *writeData,uint16_t countToWrite); 

#endif