#include "bsp_Flash.h"
#include "stm32f10x_flash.h"

#define FLASH_SIZE 512          //��ѡMCU��FLASH������С(��λΪK)

#if FLASH_SIZE<256
  #define SECTOR_SIZE           1024    //�ֽ�
#else 
  #define SECTOR_SIZE           2048    //�ֽ�
#endif


extern char * ip[15] ;
extern char * port[5];
extern char * deviceId[8];

static char * serialNumber[20];


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
	//��ȡ�洢�ķ�������Ϣ      191 210 030 152 31688   //д��Ҫ17bit
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
	//��ǰ�汾  ������Ϣ����Ϊ 71byte port Ϊ4byte ��������
	//����Ʋ�����ӦԤ��5byte��Ϊport����
	
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
	
	

	//ȡip  char * ip
	sprintf(ip,"%d%d%d%c%d%d%d%c%d%d%d%c%d%d%d%",buff[startIndex+2]-48,buff[startIndex+3]-48,buff[startIndex+4]-48,'.',
												buff[startIndex+5]-48,buff[startIndex+6]-48,buff[startIndex+7]-48,'.',
												buff[startIndex+8]-48,buff[startIndex+9]-48,buff[startIndex+10]-48,'.',
												buff[startIndex+11]-48,buff[startIndex+12]-48,buff[startIndex+13]-48);
	
	//ȡ port  char * port
	sprintf(port,"%d%d%d%d%d",buff[startIndex+14]-48,buff[startIndex+15]-48,buff[startIndex+16]-48,buff[startIndex+17]-48);
	
	//ȡ8byte id
	sprintf(deviceId,"%c%c%d%d%d%d%d%d",buff[startIndex+18],buff[startIndex+19],buff[startIndex+20],buff[startIndex+21],
										buff[startIndex+22],buff[startIndex+23],buff[startIndex+24],buff[startIndex+25]);

	//ȡ���к�  serial number
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
			printf("\r\n 0x12 ����");
		}break;
		
		default:{
			printf("\r\n default ����");
		}break;
	}
}







