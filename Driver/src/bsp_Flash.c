#include "bsp_Flash.h"
#include "stm32f10x_flash.h"

#define FLASH_SIZE 512          //��ѡMCU��FLASH������С(��λΪK)

#if FLASH_SIZE<256
  #define SECTOR_SIZE           1024    //�ֽ�
#else 
  #define SECTOR_SIZE           2048    //�ֽ�
#endif

//��ȡָ����ַ�İ���(16λ����)
extern uint16_t FLASH_ReadHalfWord(uint32_t address)
{
  return *(__IO uint16_t*)address; 
}

//��ȡָ����ַ��ȫ��(32λ����)
extern uint32_t FLASH_ReadWord(uint32_t address)
{
  uint32_t temp1,temp2;
  temp1=*(__IO uint16_t*)address; 
  temp2=*(__IO uint16_t*)(address+2); 
  return (temp2<<16)+temp1;
}

//��ָ����ַ��ʼ��ȡ�������
extern void FLASH_ReadMoreData(uint32_t startAddress,uint16_t *readData,uint16_t countToRead)
{
  uint16_t dataIndex;
  for(dataIndex=0;dataIndex<countToRead;dataIndex++)
  {
    readData[dataIndex]=FLASH_ReadHalfWord(startAddress+dataIndex*2);
  }
}

//��ָ����ַ��ʼд��������
extern void FLASH_WriteMoreData(uint32_t startAddress,uint16_t *writeData,uint16_t countToWrite)
{
  if(startAddress<FLASH_BASE||((startAddress+countToWrite*2)>=(FLASH_BASE+1024*FLASH_SIZE)))
  {
    return;//�Ƿ���ַ
  }
  FLASH_Unlock();         //����д����
  uint32_t offsetAddress=startAddress-FLASH_BASE;               //����ȥ��0X08000000���ʵ��ƫ�Ƶ�ַ
  uint32_t sectorPosition=offsetAddress/SECTOR_SIZE;            //����������ַ������STM32F103VET6Ϊ0~255
  
  uint32_t sectorStartAddress=sectorPosition*SECTOR_SIZE+FLASH_BASE;    //��Ӧ�������׵�ַ

  FLASH_ErasePage(sectorStartAddress);//�����������
  
  uint16_t dataIndex;
  for(dataIndex=0;dataIndex<countToWrite;dataIndex++)
  {
    FLASH_ProgramHalfWord(startAddress+dataIndex*2,writeData[dataIndex]);
  }
  
  FLASH_Lock();//����д����
}



extern void FLASH_Check(){
	//�Ƿ����ñ�־λ         1byte
	//�汾��                1byte �� 2.1  0X21
	//��ȡ�洢�ķ�������Ϣ      191 210 030 152 31688   //д��Ҫ18bit
	//��ȡ�洢���豸ID      8bit  SJ000001
	//��ȡ�洢���豸���к�  20bit HLSJ11100FCN00000001
	
	
	
	u8 i;
	uint16_t data[10];
	uint16_t dataBuf[10] = {0x003d,0x01,0x01,0x05,0x06,0x07,0x01,0x03,0x03,0x0c};
	FLASH_ReadMoreData(0x08000000+2*1024*20,data,1);
	
	printf("\r\n ��ȡ�ı�־λ data[0] = %d ", data[0]);
	
	if( (data[0] & 0x00ff) == 0x003d){
		printf("\r\n �Ѿ����ñ�־λ ");
		FLASH_WriteMoreData(0x08000000+2*1024*20,dataBuf,10);
		for(i = 0; i < 10; i++){
			printf("\r\n data[%d] = 0X%04X ",i,data[i]);
		}
	}else{
		printf("\r\n δ���ñ�־λ д������ ");
		
		FLASH_WriteMoreData(0x08000000+2*1024*20,dataBuf,10);
		Delay_ms(500);
		
		FLASH_ReadMoreData(0x08000000+2*1024*20,data,10);
		
		printf("\r\n �ٴζ�ȡ�ı�־λ data[0] = %d ", data[0]);
		
		if( (data[0] & 0x00ff) == 0x003d){
			printf("\r\n �Ѿ����ñ�־λ ");
			for(i = 0; i < 10; i++){
				printf("\r\n data[%d] = 0X%04X ",i,data[i]);
			}
		}else{
			printf("\r\n δ���ñ�־λ д������ ");	
		}
	}
	
	
	
}


extern void FLASH_Config(u8 *buff,u8 length){
	
	switch(buff[70]){
	    case 0x12:{
			printf("\r\n 0x12 ����");
		}break;
		
		default:{
			printf("\r\n default ����");
		}break;
	}
}







