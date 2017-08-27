#include "stm32f10x.h"
#include "bsp_debug_usart1.h"
#include "bsp_gsm_usart.h"
#include "bsp_SysTick.h"
#include "bsp_gsm_gprs.h"
#include "bsp_protocol.h"
#include "bsp_Timer.h"
#include <string.h>


#define		LOCALPORT	"2000"

#define		SERVERIP	"120.24.49.3"
#define		SERVERPORT	"44486"


char * ip[15];
char * port[5];
char * deviceId[8];

//全局变量 
u8 usart_1_buffIndex;
u8 usart_1_Buff[254];

//读取FLASH
static u16 flashBuff[71];

const char *TESTBUFF1="\r\n1.秉火GSM模块TCP数据上传功能测试";

static uint8_t testCard = 0;

#define DEBUG 1
static u8 DeviceAddr[8] = {0X31,0X32,0X33,0X34,0X35,0X36,0X37,0X38}; 
 
/*
 * 系统软件复位
 */
void Soft_Reset(void)
{
  __set_FAULTMASK(1);   /* 关闭所有中断*/  
  NVIC_SystemReset();   /* 系统复位 */
}


static void App_Init(){
    //定时器初始化
	SysTick_Init();
	//串口1 调试初始化
	 DEBUG_USART1_Init();
	
	GSM_POWER_Init();
	GSM_PWRKEY_Init();
	
	Start_Led_Init();

	
	isNetWork = 1;
}

static unsigned char ch = 0x25;
u8 buff[37] = { 0xFA,0xF1 ,0x1F ,0xAF ,0x00 ,0x1D ,0x31 ,0x32 ,0x33 ,0x34 ,0x35 ,0x36,
			   0x37 ,0x38 ,0x01 ,0x31 ,0x39 ,0x32 ,0x31 ,0x36 ,0x38 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x31,
               0x38 ,0x38 ,0x38 ,0x38 ,0xFF ,0xFF ,0xFA ,0x1F ,0xF1 ,0xAF};
static void App_Manager(){
	//修改中断向量表位置
    SCB->VTOR = FLASH_BASE| 0x10000;
	Work_Led_Open(); 
	 
#if DEBUG
      SetDeviceAddr(DeviceAddr);
#endif	
	
	printf("\r\n @@ 中莱小网关 ");
	
	printf("\r\n @@ Read Flash Config");

	
//	Timer2_Init_Config();
//	isNetWork = 1;	
//	while(1);
	
	
	flashBuff[1] = 0X5612;
	FLASH_WriteMoreData(0x08000000+2*1024*20,flashBuff,2);
	//FLASH_ReadMoreData(0x08000000+2*1024*20,flashBuff,1);
	
	// while(1);
	if(0x3c == flashBuff[1]){
		//已经配置
		//读取数据  配置参数	
	}else{
		//未配置
		//while( GetFlashConfigFlag() != 1);
		//配置参数
		
		//重启
	}
	
	
	
	
	
	
	
	
	
	printf("\r\n @@ 启动GPRS ");
  
  //  FLASH_Check();
	
 
	
	GSM_POWER_Close();
	Delay_ms(2000);
	Delay_ms(2000);
	printf("\r\n 打开GPRS电源\r\n");
	GSM_POWER_Open();
	Delay_ms(2000);
	                 
	printf("\r\n GPRS软起动\r\n");
	GSM_PWRKEY_Close();
	GSM_PWRKEY_Open();
	Delay_ms(2500);
	printf("\r\n  等待GPRS软启动\r\n");
	GSM_PWRKEY_Close();
	Delay_ms(2500);
	printf("\r\nGPRS启动完毕------\r\n");
	
	 while( gsm_init()!= GSM_TRUE){
		static u8 i = 0; 
		printf("\r\n模块响应测试不正常！！\r\n");
		printf("\r\n若模块响应测试一直不正常，请检查模块的连接或是否已开启电源开关\r\n");
		GSM_DELAY(1000);
		 
		i++;
		
		if(i >= 8){
		 
			//信号线拉低1.5s至3s关机
			//超过3s重新开机	
			GSM_PWRKEY_Open();
			Delay_ms(1000);
			Delay_ms(1000);
			GSM_PWRKEY_Close();
			 
			Delay_ms(5000);
			
			__set_FAULTMASK(1);   /* 关闭所有中断*/  
			NVIC_SystemReset();   /* 系统复位 */
		}
	 }
	 
	 printf("\r\n正在等待GSM模块初始化...\r\n");
	 
	 
	while(IsInsertCard() != GSM_TRUE)
	{
		 static u8 i = 0; 
		if(++testCard >20)
		{
			i++;
			printf("\r\n检测不到电话卡，请断电并重新接入电话卡\r\n");
			if(i >= 12){
		 
			//信号线拉低1.5s至3s关机
			//超过3s重新开机	
			GSM_PWRKEY_Open();
			Delay_ms(1000);
			Delay_ms(1000);
			GSM_PWRKEY_Close();
			 
			Delay_ms(5000);
			
			__set_FAULTMASK(1);   /* 关闭所有中断*/  
			NVIC_SystemReset();   /* 系统复位 */
		}
		}
		GSM_DELAY(1000); 		
	}
	
	
	
	//确认关闭之前的连接		
	gsm_gprs_link_close();	
	
	GSM_DELAY(100);
	//确认关闭之前的场境
	gsm_gprs_shut_close();
	
	printf("\n初始化GPRS工作环境...\n");
	
	Delay_ms(2000);
	
	//重新初始化gprs
	if(gsm_gprs_init()!= GSM_TRUE)//GPRS初始化环境
	{
		 
		printf("\r初始化GPRS失败，请重新给模块上电并复位开发板\n");
		
		 
	//信号线拉低1.5s至3s关机
    //超过3s重新开机	
	GSM_PWRKEY_Open();
	Delay_ms(1000);
	Delay_ms(1000);
	GSM_PWRKEY_Close();
	 
	Delay_ms(5000);
	
    __set_FAULTMASK(1);   /* 关闭所有中断*/  
	NVIC_SystemReset();   /* 系统复位 */
		while(1);
	}
	
	printf("\n尝试建立TCP链接，请耐心等待...\n");
	
	if(gsm_gprs_tcp_link(LOCALPORT,SERVERIP,SERVERPORT)!=GSM_TRUE)
	{
		printf("\r\n TCP链接失败，请检测正确设置各个模块\r\n");
		GSM_DELAY(1000); 
		printf("\n IP链接断开\n");
		GSM_DELAY(100);
		gsm_gprs_link_close();	
		
		printf("\n 关闭场景\n");
		GSM_DELAY(100);
		gsm_gprs_shut_close();
		printf("\r\n 5s后自动重启\r\n");
		
		GSM_PWRKEY_Open();
		Delay_ms(1000);
		Delay_ms(1000);
		GSM_PWRKEY_Close();
		 
		Delay_ms(5000);
		 
		Soft_Reset();
	}
	
	Gprs_Led_Open();
	printf("\r\n 连接成功....\n");
	GSM_CLEAN_RX();
	
	printf("\r\n连接成功,尝试发送数据...\n");	
	
    GSM_DELAY(1000);	
	if(gsm_gprs_send(TESTBUFF1)!=GSM_TRUE)
	{
		Gprs_Led_Close();
		printf("\r\nTCP发送数据失败，请检测正确设置各个模块\r\n");
		GSM_DELAY(1000); 
		printf("\nIP链接断开\n");
		GSM_DELAY(100);
		gsm_gprs_link_close();	
		
		printf("\n关闭场景\n");
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

	//开启定时器， 准备发送心跳包
	Timer2_Init_Config();
	isNetWork = 0;
	 
	while(1){
    
         if(GetIsReceiveGsmData() != 0x00){
			if(GetIsReceiveGsmData() == 1){
				//处理网络中断
				SetIsReceiveGsmData(0);
				 printf("\r\n Gsm_DealwithServerNetProtocolData");
				//处理GSM数据
				Gsm_DealwithServerNetProtocolData();
			}else if(GetIsReceiveGsmData() == 2){
				SetIsReceiveGsmData(0);
				Gsm_SendNetDataPackToServer(HeartBeatFunctionType,0,0);
			}

		 
			
		 } 
  }
}



int main(){

	App_Init();
	
	App_Manager();
	
	
	return 0;
}
