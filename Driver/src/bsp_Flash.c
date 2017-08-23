#include "bsp_Flash.h"
#include "stm32f10x_flash.h"

#define FLASH_SIZE 512          //所选MCU的FLASH容量大小(单位为K)

#if FLASH_SIZE<256
  #define SECTOR_SIZE           1024    //字节
#else 
  #define SECTOR_SIZE           2048    //字节
#endif

//读取指定地址的半字(16位数据)
extern uint16_t FLASH_ReadHalfWord(uint32_t address)
{
  return *(__IO uint16_t*)address; 
}

//读取指定地址的全字(32位数据)
extern uint32_t FLASH_ReadWord(uint32_t address)
{
  uint32_t temp1,temp2;
  temp1=*(__IO uint16_t*)address; 
  temp2=*(__IO uint16_t*)(address+2); 
  return (temp2<<16)+temp1;
}

//从指定地址开始读取多个数据
extern void FLASH_ReadMoreData(uint32_t startAddress,uint16_t *readData,uint16_t countToRead)
{
  uint16_t dataIndex;
  for(dataIndex=0;dataIndex<countToRead;dataIndex++)
  {
    readData[dataIndex]=FLASH_ReadHalfWord(startAddress+dataIndex*2);
  }
}

//从指定地址开始写入多个数据
extern void FLASH_WriteMoreData(uint32_t startAddress,uint16_t *writeData,uint16_t countToWrite)
{
  if(startAddress<FLASH_BASE||((startAddress+countToWrite*2)>=(FLASH_BASE+1024*FLASH_SIZE)))
  {
    return;//非法地址
  }
  FLASH_Unlock();         //解锁写保护
  uint32_t offsetAddress=startAddress-FLASH_BASE;               //计算去掉0X08000000后的实际偏移地址
  uint32_t sectorPosition=offsetAddress/SECTOR_SIZE;            //计算扇区地址，对于STM32F103VET6为0~255
  
  uint32_t sectorStartAddress=sectorPosition*SECTOR_SIZE+FLASH_BASE;    //对应扇区的首地址

  FLASH_ErasePage(sectorStartAddress);//擦除这个扇区
  
  uint16_t dataIndex;
  for(dataIndex=0;dataIndex<countToWrite;dataIndex++)
  {
    FLASH_ProgramHalfWord(startAddress+dataIndex*2,writeData[dataIndex]);
  }
  
  FLASH_Lock();//上锁写保护
}



extern void FLASH_Check(){
	//是否设置标志位         1byte
	//版本号                1byte 如 2.1  0X21
	//获取存储的服务器信息      191 210 030 152 31688   //写入要18bit
	//获取存储的设备ID      8bit  SJ000001
	//获取存储的设备序列号  20bit HLSJ11100FCN00000001
	
	
	
	u8 i;
	uint16_t data[10];
	uint16_t dataBuf[10] = {0x003d,0x01,0x01,0x05,0x06,0x07,0x01,0x03,0x03,0x0c};
	FLASH_ReadMoreData(0x08000000+2*1024*20,data,1);
	
	printf("\r\n 读取的标志位 data[0] = %d ", data[0]);
	
	if( (data[0] & 0x00ff) == 0x003d){
		printf("\r\n 已经配置标志位 ");
		FLASH_WriteMoreData(0x08000000+2*1024*20,dataBuf,10);
		for(i = 0; i < 10; i++){
			printf("\r\n data[%d] = 0X%04X ",i,data[i]);
		}
	}else{
		printf("\r\n 未配置标志位 写入数据 ");
		
		FLASH_WriteMoreData(0x08000000+2*1024*20,dataBuf,10);
		Delay_ms(500);
		
		FLASH_ReadMoreData(0x08000000+2*1024*20,data,10);
		
		printf("\r\n 再次读取的标志位 data[0] = %d ", data[0]);
		
		if( (data[0] & 0x00ff) == 0x003d){
			printf("\r\n 已经配置标志位 ");
			for(i = 0; i < 10; i++){
				printf("\r\n data[%d] = 0X%04X ",i,data[i]);
			}
		}else{
			printf("\r\n 未配置标志位 写入数据 ");	
		}
	}
	
	
	
}


extern void FLASH_Config(u8 *buff,u8 length){
	
	switch(buff[70]){
	    case 0x12:{
			printf("\r\n 0x12 接受");
		}break;
		
		default:{
			printf("\r\n default 接受");
		}break;
	}
}







