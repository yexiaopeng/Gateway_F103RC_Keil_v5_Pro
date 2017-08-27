#include "bsp_Flash.h"
#include "stm32f10x_flash.h"

#define FLASH_SIZE 512          //所选MCU的FLASH容量大小(单位为K)

#if FLASH_SIZE<256
  #define SECTOR_SIZE           1024    //字节
#else 
  #define SECTOR_SIZE           2048    //字节
#endif


extern char * ip[15] ;
extern char * port[5];
extern char * deviceId[8];

static char * serialNumber[20];


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
	//获取存储的服务器信息      191 210 030 152 31688   //写入要17bit
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
	//当前版本  配置信息长度为 71byte port 为4byte 进行配置
	//此设计不合理，应预留5byte作为port参数
	
	u8 i;
	u8 startIndex;
	
	for(i = length; i >=0 ; i-- ){
	    if( (buff[i] == 0xab) && (buff[i-1] == 0xff)){
			startIndex = i-1;
			break;
		}
	}
	
	printf(" startIndex %d ",startIndex);
	for(i = startIndex; i < length; i++){
		printf(" %x ",buff[i]);
	}
	
	

	//取ip  char * ip
	sprintf(ip,"%d%d%d%c%d%d%d%c%d%d%d%c%d%d%d%",buff[startIndex+2]-48,buff[startIndex+3]-48,buff[startIndex+4]-48,'.',
												buff[startIndex+5]-48,buff[startIndex+6]-48,buff[startIndex+7]-48,'.',
												buff[startIndex+8]-48,buff[startIndex+9]-48,buff[startIndex+10]-48,'.',
												buff[startIndex+11]-48,buff[startIndex+12]-48,buff[startIndex+13]-48);
	
	//取 port  char * port
	sprintf(port,"%d%d%d%d%d",buff[startIndex+14]-48,buff[startIndex+15]-48,buff[startIndex+16]-48,buff[startIndex+17]-48);
	
	//取8byte id
	sprintf(deviceId,"%c%c%d%d%d%d%d%d",buff[startIndex+18],buff[startIndex+19],buff[startIndex+20],buff[startIndex+21],
										buff[startIndex+22],buff[startIndex+23],buff[startIndex+24],buff[startIndex+25]);

	//取序列号  serial number
	sprintf(serialNumber,"%c%c%c%c%d%d%d%d%d%c%c%c%d%d%d%d%d%d%d%d",buff[startIndex+26],buff[startIndex+27],buff[startIndex+28],buff[startIndex+29]
									,buff[startIndex+30],buff[startIndex+31],buff[startIndex+32],buff[startIndex+33]
									,buff[startIndex+34],buff[startIndex+35],buff[startIndex+36],buff[startIndex+37]
									,buff[startIndex+38],buff[startIndex+39],buff[startIndex+40],buff[startIndex+41]
									,buff[startIndex+42],buff[startIndex+43],buff[startIndex+44],buff[startIndex+45]);
	
	printf("\r\n ip = %s",ip);
	printf("\r\n port = %s",port);
	printf("\r\n id = %s",deviceId);
	printf("\r\n number = %s",serialNumber);
	
	Clear_Usart1_Rec();
	switch(buff[70]){
	    case 0x12:{
			printf("\r\n 0x12 接受");
		}break;
		
		default:{
			printf("\r\n default 接受");
		}break;
	}
}







