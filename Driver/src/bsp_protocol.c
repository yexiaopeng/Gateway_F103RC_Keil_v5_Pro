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

static void InverterDataPackResponseManager(u8 * data, u16 dataLength);





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
		//printf("\r\n length error %d %d ",dataLength,netDataLength - heartIndex);
		return 3;
	}
	
	dataBuffLength = netDataLength;
	return 1;
	
}



static u8 Gsm_SendData(u8 * dataBuff,u16 dataLength){
	 u8 i;
	 char end = 0x1A;
	uint8_t testSend=0;
	GSM_CLEAN_RX();
	 if( gsm_cmd("AT+CIPSEND\r",">",500) == 0)
	{
        for(i = 0; i < dataLength;i++){
		USART_SendData(GSM_USARTx, dataBuff[i]);
		while( USART_GetFlagStatus(GSM_USARTx, USART_FLAG_TXE) == RESET );
		}
	
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
			protocolHeart.function = HeartBeatFunctionType;
			protocolHeart.length[0]  = 0x00;
			protocolHeart.length[1]= 0x0e;			 
			memcpy(dataBuff,&protocolHeart,15);			
			dataBuff[15] =  gsm_gprs_Get_SignalStrength();//�ź�ǿ��
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
		    //InverterDataPackResponseManager(data,dataLength);
		    printf("\r\n InverterDataPackFunctionType");
		    InverterDataPackResponseManager(data,dataLength);
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
			printf("\r\n Gsm_SendNetDataPackToServer δ����ָ�� 0X%02X",function);
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
	memcpy(data, &dataBuff[heartIndex+15],dataLength-13);  // -13 �ɼ�����ַ8     ���ݳ���2     CRC2 ����1

	 
	
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
	char ip[16];//Ĭ��Ϊ xxx.xxx.xxx.xxx 
	char port[6];
	
	//TODO  ת��IP��PORT 
	
	//00: Ӧ��
	ConfigFunctionTypeResponseManager(0,0);
	
	if(length < 16){
		return ;
	}


	
	//01 ��ȡ�µ�ip��PORT      192.016.001.002  ��Ч
	ip[3]  = '.';
	ip[7]  = '.';
	ip[11] = '.';
	
	ip[0] = data[0];
	ip[1] = data[1];
	ip[2] = data[2];

	ip[4] = data[3];
	ip[5] = data[4];
	ip[6] = data[5];

	ip[8] = data[6];
	ip[9] = data[7];
	ip[10] = data[8];

	ip[12] = data[9];
	ip[13] = data[10];
	ip[14] = data[11];

	ip[15] = '\0';
	


 	//port  �ж�4 or 5 
 	if(length == 16){
 		//port length 4
 		sprintf(port,"%c%c%c%c",data[12],data[13],data[14],data[15]);
		port[4] = '\0';
 	}else{
 		sprintf(port,"%c%c%c%c%c",data[12],data[13],data[14],data[15],data[16]);
		port[5] = '\0';
 	}

    //02��д���ڴ�
	FLASH_WriteIpConfig(ip, port);
	
	//03���������� ���Ƿ���Ҫ�ٴ�������������
	
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



static void InverterDataPackResponseManager(u8 * data, u16 dataLength){
#if 1
	//��Ϊ��������ʹ��
	//����Ϊ�Զ�������

	//FA F1 1F AF 00 32 
	//31 32 33 34 35 36 37 38 50 xx
	//xx 04 21
	//00 00 00 00 00 00 01 00 00 00 01
	//00 00 00 01 00 00 00 01 00 00 00 00
	//00 00 00 01 00 00 00 01 FF FF
	//FF FF FA 1F F1 AF


	clean_rebuff();
	u8 dataBuff[58];
	protocolHeart.function = InverterDataPackFunctionType;
	protocolHeart.length[0]  = 0x00;
	protocolHeart.length[1]= 0x32;			 
	memcpy(dataBuff,&protocolHeart,15); 		
	dataBuff[15] = 0xff;
	dataBuff[16] = 0x01;
	dataBuff[17] = 0x04;
	dataBuff[18] = 0x21;

	//00 00 00 00 00 00 01 00
	dataBuff[19] = 0x00; 
	dataBuff[20] = 0x00;
	dataBuff[21] = 0x00;
	dataBuff[22] = 0x00;
	dataBuff[23] = 0x00;
	dataBuff[24] = 0x00;
	dataBuff[25] = 0x01;
	dataBuff[26] = 0x00;
	//00 00 01 00 00 00 01 00
	dataBuff[27] = 0x00; 
	dataBuff[28] = 0x00;
	dataBuff[29] = 0x01;
	dataBuff[30] = 0x00;
	dataBuff[31] = 0x00;
	dataBuff[32] = 0x00;
	dataBuff[33] = 0x01;
	dataBuff[34] = 0x00;

	//00 00 01 00 00 00 00 00
	dataBuff[35] = 0x00; 
	dataBuff[36] = 0x00;
	dataBuff[37] = 0x01;
	dataBuff[38] = 0x00;
	dataBuff[39] = 0x00;
	dataBuff[40] = 0x00;
	dataBuff[41] = 0x00;
	dataBuff[42] = 0x00;
	//00 00 01 00 00 00 01 FF
	dataBuff[43] = 0x00; 
	dataBuff[44] = 0x00;
	dataBuff[45] = 0x01;
	dataBuff[46] = 0x00;
	dataBuff[47] = 0x00;
	dataBuff[48] = 0x00;
	dataBuff[49] = 0x01;
	dataBuff[50] = 0xff;
	//ff
	dataBuff[51] = 0xff;

	//FF FF FA 1F F1 AF
	dataBuff[52] = 0xff; 
	dataBuff[53] = 0xff;
	dataBuff[54] = 0xfa;
	dataBuff[55] = 0x1f;
	dataBuff[56] = 0xf1;
	dataBuff[57] = 0xaf;

	Gsm_SendData(dataBuff,58);
	
#endif
}



