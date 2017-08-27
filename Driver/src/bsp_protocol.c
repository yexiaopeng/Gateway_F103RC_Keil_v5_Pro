#include "bsp_protocol.h"
#include "bsp_start_led.h"
#include "bsp_gsm_gprs.h"
#include "stdio.h"
#include "string.h"
#include "bsp_Timer.h" 

#define DEBUG  1



//�ڲ�����
static const u8 MaxdataLength = 255; //��󳤶�
static    u8 dataBuff[MaxdataLength]; //���ݰ��洢
static u16 dataBuffLength;//���ݰ�����
static u16 heartIndex;//��ͷλ��
static u16 dataLength;//���ݳ��� ������ͷ��β



static u8 DeviceAddr[8] = {0X31,0X32,0X33,0X34,0X35,0X36,0X37,0X38}; //�ɼ���ID
static ProtocolHeart protocolHeart;

#define NETDATAPACKLENGTH_H_INDEX 4 
#define NETDATAPACKLENGTH_L_INDEX 5 







//******************������

//******************���������ָ��
//����IP�Ͷ˿�
static void ConfigFunctionTypeManager(u8 * addr,u8 * data, u16 length);
static void FirmwareUpdateFuncitonTypeManager(u8 * addr,u8 * data, u16 length);
static void RestartDeviceFuntionTypeManager(u8 * addr,u8 * data, u16 length);
static void SponsorRegisteredFunctionTypeManager(u8 * addr,u8 * data, u16 length);
static void RegisteredFunctionTypeManager(u8 * addr,u8 * data, u16 length);
static void OverRegisteredFunctionTypeManager(u8 * addr,u8 * data, u16 length);
//******************Ӧ����ϴ�������
extern  void ConfigFunctionTypeResponseManager();





//������ʱ����
static void Gsm_ClearData(){
	memset(dataBuff,0,MaxdataLength);
	dataBuffLength = 0;
}


//���òɼ���ID
//�洢�ɼ���ID 8�ֽ�asc��
extern void SetDeviceAddr(u8 * tempDeviceAddr){
	memset(DeviceAddr,0,8);
	memcpy(DeviceAddr,tempDeviceAddr,8);
	
	//��ͷ 0XFA 0XF1 0X1F 0XAF 
	protocolHeart.hander[0] = 0XFA;
	protocolHeart.hander[1] = 0XF1;
	protocolHeart.hander[2] = 0X1F;
	protocolHeart.hander[3] = 0XAF;
	
	//��ַ
	memcpy(&(protocolHeart.addr),tempDeviceAddr,8);
}

//��ȡ�ɼ���ID
extern void GetDeviceAddr(u8 ** tempDeviceAddr){
	memset(*tempDeviceAddr,0,8);
	memcpy(*tempDeviceAddr,DeviceAddr,8);
}



//��GSM��ȡ�����������ݽ��п���
extern u8 Gsm_CopyNetDataToDataBuff(u8 * netData,u16 netDataLength){
	u16 i;
	
	//�������� 
	memcpy(dataBuff,netData,netDataLength);
	
	//Ѱ�Ұ�ͷ ����Ѱ��   ��ֹǰһ���ĸ���
	for( i = netDataLength ; i > 0; i-- ){
		if( (dataBuff[i-3] == PACK_Heart_A) && ( dataBuff[i-2] == PACK_Heart_B) &&
				(dataBuff[i-1] == PACK_Heart_C) && (dataBuff[i] == PACK_Heart_D)){
			heartIndex = i-3;
			break;
		}
	}
	if( (heartIndex == 0) && (netDataLength == i) ){
		return 2;//����
	} 

	//�ж����ݰ������Ƿ���ȷ
	//��ֹճ��
	dataLength = dataBuff[heartIndex+4];
	dataLength = dataLength << 8 ;
	dataLength +=  dataBuff[heartIndex+5];
	 
	if( (dataLength + 8) != (netDataLength - heartIndex)  ){
		printf("\r\n length error %d %d ",dataLength,netDataLength - heartIndex);
		return 3;
	}
	
	dataBuffLength = netDataLength;
	return 1;
	
}



static u8 Gsm_SendData(u8 * dataBuff,u16 dataLength){
	 u8 i;
	 char end = 0x1A;
	uint8_t testSend=0;
	printf("\r\n 1");
	GSM_CLEAN_RX();
	printf("\r\n 2");
	 if( gsm_cmd("AT+CIPSEND\r",">",500) == 0)
	{
		
		 printf("\r\n 3");
        for(i = 0; i < dataLength;i++){
		USART_SendData(GSM_USARTx, dataBuff[i]);
		while( USART_GetFlagStatus(GSM_USARTx, USART_FLAG_TXE) == RESET );
		}
		printf("\r\n 4");
		GSM_CLEAN_RX();
		gsm_cmd(&end,0,100);		
		
		//����Ƿ������
		while(gsm_cmd_check("SEND OK") != GSM_TRUE )
		{		
			if(++testSend >200)//��ȴ�20��
			{
				goto gprs_send_failure;
			}
			GSM_DELAY(100);
			 
		}
		return GSM_TRUE;		
		 
	}else{
	gprs_send_failure:
		
		end = 0x1B;
		gsm_cmd(&end,0,0);	//ESC,ȡ������ 

		return GSM_FALSE;
	}
		

}




//���������������
/* �������� Gsm_SendNetDataPackToServer
 * �������� ���������������
 * ������� u8 function   ָ�����
			u8 * data   ����
			u16 dataLength   ���ݳ���
 */
extern void Gsm_SendNetDataPackToServer(u8 function, u8 * data, u16 dataLength){
	
	switch(function){
		case HeartBeatFunctionType:{
			//0x10��������
			clean_rebuff();
			u8 dataBuff[22];
			printf("\r\n 444444444444444444444444"); 
			protocolHeart.function = HeartBeatFunctionType;
			protocolHeart.length[0]  = 0x00;
			protocolHeart.length[1]= 0x0e;
			printf("\r\n 33333333333333333333333333"); 
			memcpy(dataBuff,&protocolHeart,15);
			
			dataBuff[15] = 0x1F;
			dataBuff[16] = 0xff;
			dataBuff[17] = 0xff;
			dataBuff[18] = 0xfa;
			dataBuff[19] = 0x1f; 
			dataBuff[20] = 0xf1;
			dataBuff[21] = 0xaf;
			 
		    Gsm_SendData(dataBuff,22);		 
		}break;
		
		case DeviceStateFunctionType:{
		    //0x20���ɼ���״̬
		}break;
		
		case InverterDataPackFunctionType:{
		    //0x50����������ݰ�
		}break;
		
		case DeviceErrorFunctionType:{
		   //0x80������
		}break;
	
		
		//����Ϊ������Ӧ��
		//TODO  �Ƿ����ȡ��
	    case ConfigFunctionType:{
			//��������IP PORT
			 ConfigFunctionTypeResponseManager(data,dataLength);
		}break;
		
		case FirmwareUpdateFuncitonType:{
		     //�̼�����
			 
		}break;
		
		case RestartDeviceFuntionType:{
		     //�����ɼ���
		}break;
		
		case SponsorRegisteredFunctionType:{
		      //����ע��
		}break;
		
		case RegisteredFunctionType:{
		    //ע��ָ��
		}break;
		
		case OverRegisteredFunctionType:{
		     //����ע��
		}break;
		
		default:{
			printf("\r\n Gsm_SendNetDataPackToServer δ����ָ��");
		}
	}
}


/*
 *  �������� Gsm_DealwithServerNetProtocolData
 *  �������� ������ܵķ���������
 *  ������� u8 * addr  --  ��ַ   �̶�8�ֽ�
 *			u8 function --  ����
 *			u8 * data   --  ��������
 *			u16 dataLength  --  ���ݳ���
 */
extern void Gsm_DealwithServerNetProtocolData(){
	printf("\r\n Gsm_DealwithServerNetProtocolData  ");
	u8 addr[8];//��ַ
	u8 function;//����
	u8 data[200];//���ݱ䳤
	//u16 dataLength ;//

	//���ñ�־λ  �������ڴ�������Ӧ��
	isNetWork = 1;



	
	//��ȡ���ݰ�
	memcpy(addr,&dataBuff[heartIndex+6],8);
	function = dataBuff[heartIndex+14];
	memcpy(data, &dataBuff[heartIndex+15],dataLength-13);  
	
	switch(function){
		case ConfigFunctionType:{
			//��������IP PORT
			ConfigFunctionTypeManager(addr,data,dataLength-13);
		}break;
		
		case FirmwareUpdateFuncitonType:{
		     //�̼�����
			FirmwareUpdateFuncitonTypeManager(addr,data,dataLength-13);
			
		}break;
		
		case RestartDeviceFuntionType:{
		     //�����ɼ���
			 RestartDeviceFuntionTypeManager(addr,data,dataLength-13);
		}break;
		
		case SponsorRegisteredFunctionType:{
		      //����ע��
			  SponsorRegisteredFunctionTypeManager(addr,data,dataLength-13);
		}break;
		
		case RegisteredFunctionType:{
		    //ע��ָ��
			 RegisteredFunctionTypeManager(addr,data,dataLength-13);
		}break;
		
		case OverRegisteredFunctionType:{
		     //����ע��
			 OverRegisteredFunctionTypeManager(addr,data,dataLength-13);
		}break;
		
		default:{
			printf("\r\n Gsm_DealwithServerNetProtocolData δ����ָ��");
		}
	}

	//���ñ�־λ  ��������Ӧ��
	isNetWork = 0;
}








//***********************************************������

//**************************  ���ܴ�����
//ConfigFunctionType
//��������IP PORT
static void ConfigFunctionTypeManager(u8 * addr,u8 * data, u16 length){
#if 0
	u16 i;
	printf("\r\n ConfigFunctionTypeManager ");
	for(i = 0; i < 8; i++){
		printf("\r\n %d ",addr[i]);
	}
	
	printf("\r\n **************************");
	
	for(i = 0; i < length; i++){
		printf("\r\n %d ",data[i]);
	}
#endif	
	
	
	
	//TODO  ת��IP��PORT 
	
	//00: Ӧ��
	ConfigFunctionTypeResponseManager(0,0);
	
	
	//01 �����µ�ip��PORT
	
    //02��д���ڴ�
	
	
	//03���������� ���Ƿ���Ҫ�ٴ�������������

	
	
	
	
}
 
extern  void ConfigFunctionTypeResponseManager(){
	//���� IP���˿�    ������Ӧ��
	u8 i; 
	u8 dataBuff[22];
	
	protocolHeart.function = ConfigFunctionType;
	protocolHeart.length[0]  = 0x00;
	protocolHeart.length[1]= 0x0e;
	
	memcpy(dataBuff,&protocolHeart,15);
	
	dataBuff[15] = 0x01;
	dataBuff[16] = 0xff;
	dataBuff[17] = 0xff;
	dataBuff[18] = 0xfa;
	dataBuff[19] = 0x1f; 
	dataBuff[20] = 0xf1;
	dataBuff[21] = 0xaf;
	 
     Gsm_SendData(dataBuff,22);	
}




/*
 * �̼�����������
 * FirmwareUpdateFuncitonTypeManager
 *
 */
static void FirmwareUpdateFuncitonTypeManager(u8 * addr,u8 * data, u16 length){
#if 0
	printf("\r\n FirmwareUpdateFuncitonType %d \r\n",length);
	
	u16 i;
 
	//�������ߵ�λ
	//data[0] - [5]
 
	//���ص�ַ
	for(i = 0; i < length -6; i++){
		printf("%c",data[i+6]);
	}
	
	 
#endif 
	u16 i;
	u8 dataBuff[22];
	u8 urlAddr[length-6]; //�洢��ַ
	//01 ��ȡ�̼����ص�ַ
	 memcpy(urlAddr,&data[6],length-6);
	
	for(i = 0; i < length -6; i++){
		printf("%c",urlAddr[i]);
	}
	
	
	//02 ��ȡ�̼���
	 //TODO  ����
	
	
	//03 У��̼�
	
	
	//04  ��Ӧ��    
	
	
	protocolHeart.function = FirmwareUpdateFuncitonType;
	protocolHeart.length[0]  = 0x00;
	protocolHeart.length[1]= 0x0e;
	
	memcpy(dataBuff,&protocolHeart,15);
	
	dataBuff[15] = 0x01;
	dataBuff[16] = 0xff;
	dataBuff[17] = 0xff;
	dataBuff[18] = 0xfa;
	dataBuff[19] = 0x1f; 
	dataBuff[20] = 0xf1;
	dataBuff[21] = 0xaf;
	 
     Gsm_SendData(dataBuff,22);	
	

	//05��
    //У��ɹ�  ����
    //У��ʧ��  ������

}

 

/*
 * �����ɼ���������
 * RestartDeviceFuntionTypeManager
 * ���ƺö�GPRS�Ķϵ�ʱ��  ��ʹGPRS������������
 */
static void RestartDeviceFuntionTypeManager(u8 * addr,u8 * data, u16 length){
#if 1
	printf("\r\n RestartDeviceFuntionTypeManager %d \r\n",length);
#endif	
	protocolHeart.function = RestartDeviceFuntionType;
	protocolHeart.length[0]  = 0x00;
	protocolHeart.length[1]= 0x0e;
	
	memcpy(dataBuff,&protocolHeart,15);
	
	dataBuff[15] = 0x01;
	dataBuff[16] = 0xff;
	dataBuff[17] = 0xff;
	dataBuff[18] = 0xfa;
	dataBuff[19] = 0x1f; 
	dataBuff[20] = 0xf1;
	dataBuff[21] = 0xaf;
	 
    Gsm_SendData(dataBuff,22);	
	
	Delay_ms(1000);
	gsm_gprs_link_close();	

	
	Delay_ms(100);
	gsm_gprs_shut_close();

	Delay_ms(5000);
	 
	//�ź�������1.5s��3s�ػ�
    //����3s���¿���	
	GSM_PWRKEY_Open();
	Delay_ms(1000);
	Delay_ms(1000);
	GSM_PWRKEY_Close();
	 
	Delay_ms(5000);
	
    __set_FAULTMASK(1);   /* �ر������ж�*/  
	NVIC_SystemReset();   /* ϵͳ��λ */

}

/*
 * ����ע�ᴦ����
 * SponsorRegisteredFunctionTypeManager
 */
static void SponsorRegisteredFunctionTypeManager(u8 * addr,u8 * data, u16 length){
 #if 1
	printf("\r\n SponsorRegisteredFunctionTypeManager \r\n");
#endif	
	
	protocolHeart.function = SponsorRegisteredFunctionType;
	protocolHeart.length[0]  = 0x00;
	protocolHeart.length[1]= 0x0e;
	
	memcpy(dataBuff,&protocolHeart,15);
	
	dataBuff[15] = 0x01;
	dataBuff[16] = 0xff;
	dataBuff[17] = 0xff;
	dataBuff[18] = 0xfa;
	dataBuff[19] = 0x1f; 
	dataBuff[20] = 0xf1;
	dataBuff[21] = 0xaf;
	 
    Gsm_SendData(dataBuff,22);	
	
	//TODO  ����ע��
}

/*
 * ע��ָ�����
 * RegisteredFunctionTypeManager
 */
static void RegisteredFunctionTypeManager(u8 * addr,u8 * data, u16 length){
 #if 1
	printf("\r\n RegisteredFunctionTypeManager \r\n");
#endif	
	
	protocolHeart.function = RegisteredFunctionType;
	protocolHeart.length[0]  = 0x00;
	protocolHeart.length[1]= 0x0e;
	
	memcpy(dataBuff,&protocolHeart,15);
	
	dataBuff[15] = 0x01;
	dataBuff[16] = 0xff;
	dataBuff[17] = 0xff;
	dataBuff[18] = 0xfa;
	dataBuff[19] = 0x1f; 
	dataBuff[20] = 0xf1;
	dataBuff[21] = 0xaf;
	 
    Gsm_SendData(dataBuff,22);
	
	//TODO ����ע��ָ��
	
	
	
}

static void OverRegisteredFunctionTypeManager(u8 * addr,u8 * data, u16 length){
#if 1
	printf("\r\n OverRegisteredFunctionTypeManager \r\n");
#endif	
	
	protocolHeart.function = OverRegisteredFunctionType;
	protocolHeart.length[0]  = 0x00;
	protocolHeart.length[1]= 0x0e;
	
	memcpy(dataBuff,&protocolHeart,15);
	
	dataBuff[15] = 0x01;
	dataBuff[16] = 0xff;
	dataBuff[17] = 0xff;
	dataBuff[18] = 0xfa;
	dataBuff[19] = 0x1f; 
	dataBuff[20] = 0xf1;
	dataBuff[21] = 0xaf;
	 
    Gsm_SendData(dataBuff,22);
	
	
	//TODO  ����ע��
}


//**************************  ����Ӧ������




