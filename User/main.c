#include "stm32f10x.h"
#include "bsp_debug_usart1.h"
#include "bsp_gsm_usart.h"
#include "bsp_SysTick.h"
#include "bsp_gsm_gprs.h"
#include "bsp_protocol.h"

#include <string.h>


#define		LOCALPORT	"2000"

#define		SERVERIP	"120.24.49.3"
#define		SERVERPORT	"36504"


const char *TESTBUFF1="\r\n1.����GSMģ��TCP�����ϴ����ܲ���";

static uint8_t testCard = 0;

#define DEBUG 1
static u8 DeviceAddr[8] = {0X31,0X32,0X33,0X34,0X35,0X36,0X37,0X38}; 
 
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
}

static unsigned char ch = 0x25;
u8 buff[37] = { 0xFA,0xF1 ,0x1F ,0xAF ,0x00 ,0x1D ,0x31 ,0x32 ,0x33 ,0x34 ,0x35 ,0x36,
			   0x37 ,0x38 ,0x01 ,0x31 ,0x39 ,0x32 ,0x31 ,0x36 ,0x38 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x31,
               0x38 ,0x38 ,0x38 ,0x38 ,0xFF ,0xFF ,0xFA ,0x1F ,0xF1 ,0xAF};
static void App_Manager(){
 
	Work_Led_Open(); 
	 
#if DEBUG
      SetDeviceAddr(DeviceAddr);
#endif	
	
	printf("\r\n ����С���� ");
	 
	printf("\r\n ����һ ����GPRS ");
  
 
	
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
	
	if(gsm_gprs_tcp_link(LOCALPORT,SERVERIP,SERVERPORT)!=GSM_TRUE)
	{
		printf("\r\n TCP����ʧ�ܣ�������ȷ���ø���ģ��\r\n");
		GSM_DELAY(1000); 
		printf("\n IP���ӶϿ�\n");
		GSM_DELAY(100);
		gsm_gprs_link_close();	
		
		printf("\n �رճ���\n");
		GSM_DELAY(100);
		gsm_gprs_shut_close();
		printf("\r\n 5s���Զ�����\r\n");
		
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
	
	printf("\r\n���ӳɹ�,���Է�������...\n");	
	
    GSM_DELAY(1000);	
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
 
    GSM_CLEAN_RX();
	
	while(1){
    
         if(GetIsReceiveGsmData() != 0x00){
			 SetIsReceiveGsmData(0);
			  printf("\r\n Gsm_DealwithServerNetProtocolData");
			 //����GSM����
			 Gsm_DealwithServerNetProtocolData();
		 } 
  }
}



int main(){

	App_Init();
	
	App_Manager();
	
	
	return 0;
}
