#include "bsp_Flash.h"
#include "stm32f10x_flash.h"
#include "string.h"

#define FLASH_SIZE 512          //��ѡMCU��FLASH������С(��λΪK)

#if FLASH_SIZE<256
  #define SECTOR_SIZE           1024    //�ֽ�
#else 
  #define SECTOR_SIZE           2048    //�ֽ�
#endif


char ip[15] ;
char port[5];
char deviceId[8];
char serialNumber[20];


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

#if 0
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
#endif


//�˴�����IpConfig��FLASH��ַ
#define  DeviceConfigAddr 0x0800F000 //(0x0800000+2*1024*30)

/*
 * char IP[15]
 * char PORT[5]
 * char ID[8]
 * char  SerialNumber[20]
 *  add char DeviceConfig[48] ->  uint16_t DeviceConfig[24]
 * ��ȫ������  �޸�  ��д��  
 * д����̻����ԭ�ȵ�����
 */

#if 0 //�����ǳ���˼ά����ɵ���������  ��ȡһ������²��ؿ���
//����ϵͳmemcpy���ڴ�˳�����⣬ �Լ�дһ����
static void STM_Memcpy_u16Tou8(u8 * dest, const u16 *src, size_t srcLength){
	//�����Ƕδ�������
	u8 i;
	for (i = 0; i < srcLength; i++)
	{
		u16 temp = src[i];  
		dest[i*2] = (u8)((temp & 0xff00) >> 8);
		dest[i*2+1] = (u8)((temp & 0x00ff));
	}
} 

static void STM_Memcpy_u8To16(u16 * dest, const u8 *src, size_t destLength){
	//�����Ƕδ�������
	u8 i;
	for (i = 0; i < destLength; i++)
	{
		dest[i] = src[i*2] + src[i*2+1]*256;
	}
}

#endif

extern void FLASH_WriteDeviceAllConfig(char *ip,char * port,char * id,char * serialNumber){
	uint16_t DeviceConfig[24];
	char     DeviceConfig_Temp[48];	

	memcpy(DeviceConfig_Temp,ip, 15);
	memcpy(&DeviceConfig_Temp[15], port, 5);
	memcpy(&DeviceConfig_Temp[20],id,8);
	memcpy(&DeviceConfig_Temp[28],serialNumber,20);

	memcpy(DeviceConfig, DeviceConfig_Temp,48);
	FLASH_WriteMoreData(DeviceConfigAddr,DeviceConfig,24);
}

extern void FLASH_ReadDeviceAllConfig(char * ip,char * port,char * id,char * serialNumber){
	uint16_t DeviceConfig[24];
	char     DeviceConfig_Temp[48];	
	
	FLASH_ReadMoreData(DeviceConfigAddr, DeviceConfig, 24);
	memcpy(DeviceConfig_Temp, DeviceConfig, 48);

	memcpy(ip,DeviceConfig_Temp,15);
	memcpy(port,&DeviceConfig_Temp[15],5);
	memcpy(id,&DeviceConfig_Temp[20],8);
	memcpy(serialNumber,&DeviceConfig_Temp[28],20);

}





extern void FLASH_WriteIpConfig(char *ip,char * port){
	u8 i;
	uint16_t DeviceConfig[24];
	char     DeviceConfig_Temp[48];

#if 1
	FLASH_ReadMoreData(DeviceConfigAddr, DeviceConfig, 24);
	memcpy(DeviceConfig_Temp, DeviceConfig, 48);

	memcpy(DeviceConfig_Temp,ip, 15);
	memcpy(&DeviceConfig_Temp[15], port, 5);

	memcpy(DeviceConfig, DeviceConfig_Temp, 48);

	FLASH_WriteMoreData(DeviceConfigAddr,DeviceConfig,24);
#endif
	
#if 0
	//read all 
	FLASH_ReadMoreData(DeviceConfigAddr, DeviceConfig, 24);
	 
	STM_Memcpy_u16Tou8(DeviceConfig_Temp,DeviceConfig,24);
	 

	memcpy(DeviceConfig_Temp,ip, 15);
	memcpy(&DeviceConfig_Temp[15], port, 5);
	 

	for(i = 0; i < 48;i++){
		printf(" %c ",DeviceConfig_Temp[i]);
	}

	
	FLASH_WriteMoreData(DeviceConfigAddr,DeviceConfig,24);

	STM_Memcpy_u8To16(DeviceConfig,DeviceConfig_Temp,24);

	
	 
#endif
	
#if 0	
	FLASH_ReadMoreData(DeviceConfigAddr, DeviceConfig, 24);
	memset(DeviceConfig_Temp, 0, 48);
	STM_Memcpy_u16Tou8(DeviceConfig_Temp,DeviceConfig,24);
	for(i = 0; i < 48;i++){
		printf(" %d ",DeviceConfig_Temp[i]);
	}
	printf(" 6 "); 
#endif	
	
}


/****************************************
@Function Name:       FLASH_ReadIpConfig
@Function Description:  Read ip and port config  
@Input Parameter:    None
@Output Parameter   ip  port
@Return Value: None
****************************************/ 
extern void FLASH_ReadIpConfig(char ** ip, char ** port){
	uint16_t DeviceConfig[24];
	char     DeviceConfig_Temp[48];

	FLASH_ReadMoreData(DeviceConfigAddr, DeviceConfig, 24);
	memcpy(DeviceConfig_Temp, DeviceConfig, 48);

	memcpy(*ip,DeviceConfig_Temp,15);
	memcpy(*port,&DeviceConfig_Temp[15],5);
}


extern void FLASH_WriteDeviceIdConfig(char * id){
	uint16_t DeviceConfig[24];
	char     DeviceConfig_Temp[48];
	FLASH_ReadMoreData(DeviceConfigAddr, DeviceConfig, 24);
	memcpy(DeviceConfig_Temp, DeviceConfig, 48);
	memcpy(&DeviceConfig_Temp[20],id,8);
	memcpy(DeviceConfig, DeviceConfig_Temp,48);
	FLASH_WriteMoreData(DeviceConfigAddr,DeviceConfig,24);
}

extern void FLASH_ReadDeviceIdConfig(char ** id){
	uint16_t DeviceConfig[24];
	char     DeviceConfig_Temp[48];
	FLASH_ReadMoreData(DeviceConfigAddr, DeviceConfig, 24);
	memcpy(DeviceConfig_Temp, DeviceConfig, 48);
	memcpy(*id,&DeviceConfig_Temp[20],8);
}

extern void FLASH_WriteDeviceSerialNumber(char * serialNumber){
	uint16_t DeviceConfig[24];
	char     DeviceConfig_Temp[48];
	FLASH_ReadMoreData(DeviceConfigAddr, DeviceConfig, 24);
	memcpy(DeviceConfig_Temp, DeviceConfig, 48);
	memcpy(&DeviceConfig_Temp[28],serialNumber,20);
	memcpy(DeviceConfig, DeviceConfig_Temp,48);
	FLASH_WriteMoreData(DeviceConfigAddr,DeviceConfig,24);

}
extern void FLASH_ReadDeviceSerialNumber(char ** serialNumber){
	uint16_t DeviceConfig[24];
	char	 DeviceConfig_Temp[48];
	FLASH_ReadMoreData(DeviceConfigAddr, DeviceConfig, 24);
	memcpy(DeviceConfig_Temp, DeviceConfig, 48);
	memcpy(*serialNumber,&DeviceConfig_Temp[28],8);
}

extern void FLASH_USART_ConfigDeviceParameter(char * usartBuff,u8 length){
	//����usartbuff�ж�Ӧ������
	char ip[15];
	char port[5];
	char id[8];
	char serialNumber[20];
	 

	//ip
	memset(ip,'.',15);
	ip[0] = usartBuff[2];
	ip[1] = usartBuff[3];
	ip[2] = usartBuff[4];

	ip[4] = usartBuff[5];
	ip[5] = usartBuff[6];
	ip[6] = usartBuff[7];

	ip[8] = usartBuff[8];
	ip[9] = usartBuff[9];
	ip[10] = usartBuff[10];

	ip[12] = usartBuff[11];
	ip[13] = usartBuff[12];
	ip[14] = usartBuff[13];

	 

	//port
	
	port[0] =  usartBuff[14];
	port[1] =  usartBuff[15];
	port[2] =  usartBuff[16];
	port[3] =  usartBuff[17];
	port[4] =  usartBuff[68]; 

	//id
	memcpy(id,&usartBuff[18],8);

	//serialNumber
	memcpy(serialNumber,&usartBuff[26],20);
	
	 
	//�������������޸Ķ�Ӧ����
	switch(usartBuff[70]){
		case 0x10:{
	
		}
		break;

		case 0x11:{

		}
		break;

		case 0x12:{
			//����ȫ����
			printf("\r\n 5555555555");
			FLASH_WriteDeviceAllConfig(ip,port,id,serialNumber);
		printf("\r\n 6666666666666");
		}
		break;
			//����ip
		case 0x13:{
			FLASH_WriteIpConfig(ip, port);
		}
		break;

		case 0x14:{
			//����id
			FLASH_WriteDeviceIdConfig(id);
		}
		break;

		case 0x15:{
		//�������к� serialnumber

		}
		break;

		case 0x17:{
			//��ӡ����
		}
		break;
		
		
	}
	
	
	
}





