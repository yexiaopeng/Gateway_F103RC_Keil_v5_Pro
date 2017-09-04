#include "stm32f10x.h"
#include "bsp_debug_usart1.h"
#include "bsp_gsm_usart.h"
#include "bsp_SysTick.h"
#include "bsp_gsm_gprs.h"
#include "bsp_protocol.h"
#include "bsp_Timer.h"
#include <string.h>
#include "bsp_Flash.h"


#define		LOCALPORT	"2000"




//��ȡFLASH
//static u16 flashBuff[71];



static uint8_t testCard = 0;

#define DEBUG 1

static char staticip[16];
static char staticport[6];


/*
 * ϵͳ�����λ
 */
void Soft_Reset(void)
{
  __set_FAULTMASK(1);   /* �ر������ж�*/  
  NVIC_SystemReset();   /* ϵͳ��λ */
}


static void App_Init(){
    //��ʱ����ʼ��
	SysTick_Init();
	//����1 ���Գ�ʼ��
	 DEBUG_USART1_Init();
	
	GSM_POWER_Init();
	GSM_PWRKEY_Init();
	
	Start_Led_Init();

	
	isNetWork = 1;
}



static void App_Manager(){

	//�޸��ж�������λ��
    SCB->VTOR = FLASH_BASE| 0x10000;
	Work_Led_Open(); 
	 
	
	printf("\r\n @@ ����С���� ");
	
	printf("\r\n @@ Read Flash Config");

	FLASH_ReadDeviceAllConfig(ip,port, deviceId, serialNumber);

	while(ip[0] == 0xff){
		//δ����
		printf("\r\n �����ò��� ");
		Delay_ms(1000);
		if( GetIsReceiveGsmData() == 3 ){
				//�����й��̴�����FLASH����
				isNetWork = 1;
				SetIsReceiveGsmData(0);
				FLASH_USART_ConfigDeviceParameter(usart_1_Buff,71);
				Clear_Usart1_Rec();
				printf("\r\n ���ò����ɹ�   �豸���� r\n");
				// �����豸
				Gprs_Led_Close();
				printf("\r\n ����FLASH   �豸���� r\n");
				GSM_DELAY(1000); 
				printf("\nIP���ӶϿ�\n");
				GSM_DELAY(100);
				gsm_gprs_link_close();	
				
				printf("\n�رճ���\n");
				GSM_DELAY(100);
				gsm_gprs_shut_close();
				
				GSM_PWRKEY_Open();
				Delay_ms(1000);
				Delay_ms(1000);
				GSM_PWRKEY_Close();
				 
				Delay_ms(5000);
				 
				Soft_Reset();
		
				isNetWork = 0;
			}
	}

	printf("\r\n ����������� \r\n ip = ");

	memcpy(staticip,ip, 15);
	staticip[15] = '\0';
	
	printf("\r\n %s ",staticip);
	Delay_ms(20);


	if(port[0] == 0){
		memcpy(staticport,&port[1], 4);
		staticport[4] = '\0';
	}else{
		memcpy(staticport,port, 5);
		staticport[5] = '\0';
	}
	
	printf("\r\n port = %s",staticport);

	SetDeviceAddr(deviceId);



	printf("\r\n @@ ����GPRS ");
  
	
 
	
	GSM_POWER_Close();
	Delay_ms(2000);
	Delay_ms(2000);
	printf("\r\n ��GPRS��Դ\r\n");
	GSM_POWER_Open();
	Delay_ms(2000);
	                 
	printf("\r\n GPRS����\r\n");
	GSM_PWRKEY_Close();
	GSM_PWRKEY_Open();
	Delay_ms(2500);
	printf("\r\n  �ȴ�GPRS������\r\n");
	GSM_PWRKEY_Close();
	Delay_ms(2500);
	printf("\r\nGPRS�������------\r\n");
	
	 while( gsm_init()!= GSM_TRUE){
		static u8 i = 0; 
		printf("\r\nģ����Ӧ���Բ���������\r\n");
		printf("\r\n��ģ����Ӧ����һֱ������������ģ������ӻ��Ƿ��ѿ�����Դ����\r\n");
		GSM_DELAY(1000);
		 
		i++;
		
		if(i >= 8){
		 
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
	 }
	 
	 printf("\r\n���ڵȴ�GSMģ���ʼ��...\r\n");
	 
	 
	while(IsInsertCard() != GSM_TRUE)
	{
		 static u8 i = 0; 
		if(++testCard >20)
		{
			i++;
			printf("\r\n��ⲻ���绰������ϵ粢���½���绰��\r\n");
			if(i >= 12){
		 
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
		}
		GSM_DELAY(1000); 		
	}
	
	
	
	//ȷ�Ϲر�֮ǰ������		
	gsm_gprs_link_close();	
	
	GSM_DELAY(100);
	//ȷ�Ϲر�֮ǰ�ĳ���
	gsm_gprs_shut_close();
	
	printf("\n��ʼ��GPRS��������...\n");
	
	Delay_ms(2000);
	
	//���³�ʼ��gprs
	if(gsm_gprs_init()!= GSM_TRUE)//GPRS��ʼ������
	{
		 
		printf("\r��ʼ��GPRSʧ�ܣ������¸�ģ���ϵ粢��λ������\n");
		
		 
	//�ź�������1.5s��3s�ػ�
    //����3s���¿���	
	GSM_PWRKEY_Open();
	Delay_ms(1000);
	Delay_ms(1000);
	GSM_PWRKEY_Close();
	 
	Delay_ms(5000);
	
    __set_FAULTMASK(1);   /* �ر������ж�*/  
	NVIC_SystemReset();   /* ϵͳ��λ */
		while(1);
	}

	printf("\n���Խ���TCP���ӣ������ĵȴ�...\n");
	
	if(gsm_gprs_tcp_link(LOCALPORT,staticip,staticport)!=GSM_TRUE)
	{
		printf("\r\n TCP����ʧ�ܣ�������ȷ���ø���ģ��\r\n");
		GSM_DELAY(1000); 
		printf("\n IP���ӶϿ�\n");
		GSM_DELAY(100);
		gsm_gprs_link_close();	
		
		printf("\n �رճ���\n");
		GSM_DELAY(100);
		gsm_gprs_shut_close();



		while(1){
			printf("\n �����������豸 �� ��� ip����\n");
			GSM_DELAY(1000);
			if (GetIsReceiveGsmData()==3)
			{
			 	SetIsReceiveGsmData(0);
				FLASH_USART_ConfigDeviceParameter(usart_1_Buff,71);
				Clear_Usart1_Rec();
				printf("\n �޸���� ����\n");		
				GSM_PWRKEY_Open();
				Delay_ms(1000);
				Delay_ms(1000);
				GSM_PWRKEY_Close();
				 
				Delay_ms(5000);
				 
				Soft_Reset();
			}
		}


		
		
		GSM_PWRKEY_Open();
		Delay_ms(1000);
		Delay_ms(1000);
		GSM_PWRKEY_Close();
		 
		Delay_ms(5000);
		 
		Soft_Reset();
	}
	
	Gprs_Led_Open();
	printf("\r\n ���ӳɹ�....\n");
	GSM_CLEAN_RX();
	
	printf("\r\n���ӳɹ�,���Է���������...\n");	

	Gsm_SendNetDataPackToServer(HeartBeatFunctionType,0,0);


	
	
    GSM_DELAY(1000);	

#if 0	
	if(gsm_gprs_send(TESTBUFF1)!=GSM_TRUE)
	{
		Gprs_Led_Close();
		printf("\r\nTCP��������ʧ�ܣ�������ȷ���ø���ģ��\r\n");
		GSM_DELAY(1000); 
		printf("\nIP���ӶϿ�\n");
		GSM_DELAY(100);
		gsm_gprs_link_close();	
		
		printf("\n�رճ���\n");
		GSM_DELAY(100);
		gsm_gprs_shut_close();
		
		GSM_PWRKEY_Open();
		Delay_ms(1000);
		Delay_ms(1000);
		GSM_PWRKEY_Close();
		 
		Delay_ms(5000);
		 
		Soft_Reset();
		
		while(1);
	}

#endif	

	
    GSM_CLEAN_RX();

	//������ʱ���� ׼������������
	Timer2_Init_Config();
	isNetWork = 0;
	 
	while(1){
    
         if(GetIsReceiveGsmData() != 0x00){
			if(GetIsReceiveGsmData() == 1){
				//���������ж�
				SetIsReceiveGsmData(0);
				 printf("\r\n ���ܷ�����ָ��  ��ʼ����");
				//����GSM����
				Gsm_DealwithServerNetProtocolData();
			}else if(GetIsReceiveGsmData() == 2){
				SetIsReceiveGsmData(0);
				Gsm_SendNetDataPackToServer(HeartBeatFunctionType,0,0);
			}else if( GetIsReceiveGsmData() == 3 ){
				//�����й��̴�����FLASH����
				isNetWork = 1;
				SetIsReceiveGsmData(0);
				FLASH_USART_ConfigDeviceParameter(usart_1_Buff,71);
				Clear_Usart1_Rec();
				// �����豸

				
				Gprs_Led_Close();
				printf("\r\n ����FLASH   �豸���� r\n");
				GSM_DELAY(1000); 
				
				
				
				GSM_PWRKEY_Open();
				Delay_ms(1000);
				Delay_ms(1000);
				GSM_PWRKEY_Close();
				 
				Delay_ms(5000);
				 
				Soft_Reset();
		
				isNetWork = 0;
			}else if( GetIsReceiveGsmData() == 4 ){
				//�˴�Ϊ����������ʹ�� 
				//ʵ������и�
				isNetWork = 1;
				SetIsReceiveGsmData(0);
				printf("\r\n ���Ͳɼ������ݰ�");
				Gsm_SendNetDataPackToServer(InverterDataPackFunctionType,0,0);
				isNetWork = 0;
			}
		 } 
  }
}



int main(){

	App_Init();
	
	App_Manager();
	
	
	return 0;
}



