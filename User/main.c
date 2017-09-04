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




//读取FLASH
//static u16 flashBuff[71];



static uint8_t testCard = 0;

#define DEBUG 1

static char staticip[16];
static char staticport[6];


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



static void App_Manager(){

	//修改中断向量表位置
    SCB->VTOR = FLASH_BASE| 0x10000;
	Work_Led_Open(); 
	 
	
	printf("\r\n @@ 中莱小网关 ");
	
	printf("\r\n @@ Read Flash Config");

	FLASH_ReadDeviceAllConfig(ip,port, deviceId, serialNumber);

	while(ip[0] == 0xff){
		//未配置
		printf("\r\n 请配置参数 ");
		Delay_ms(1000);
		if( GetIsReceiveGsmData() == 3 ){
				//在运行过程处理串口FLASH配置
				isNetWork = 1;
				SetIsReceiveGsmData(0);
				FLASH_USART_ConfigDeviceParameter(usart_1_Buff,71);
				Clear_Usart1_Rec();
				printf("\r\n 配置参数成功   设备重启 r\n");
				// 重启设备
				Gprs_Led_Close();
				printf("\r\n 更新FLASH   设备重启 r\n");
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
		
				isNetWork = 0;
			}
	}

	printf("\r\n 参数配置完成 \r\n ip = ");

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



	printf("\r\n @@ 启动GPRS ");
  
	
 
	
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
	
	if(gsm_gprs_tcp_link(LOCALPORT,staticip,staticport)!=GSM_TRUE)
	{
		printf("\r\n TCP链接失败，请检测正确设置各个模块\r\n");
		GSM_DELAY(1000); 
		printf("\n IP链接断开\n");
		GSM_DELAY(100);
		gsm_gprs_link_close();	
		
		printf("\n 关闭场景\n");
		GSM_DELAY(100);
		gsm_gprs_shut_close();



		while(1){
			printf("\n 请重新启动设备 或 检查 ip设置\n");
			GSM_DELAY(1000);
			if (GetIsReceiveGsmData()==3)
			{
			 	SetIsReceiveGsmData(0);
				FLASH_USART_ConfigDeviceParameter(usart_1_Buff,71);
				Clear_Usart1_Rec();
				printf("\n 修改完成 重启\n");		
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
	printf("\r\n 连接成功....\n");
	GSM_CLEAN_RX();
	
	printf("\r\n连接成功,尝试发送心跳包...\n");	

	Gsm_SendNetDataPackToServer(HeartBeatFunctionType,0,0);


	
	
    GSM_DELAY(1000);	

#if 0	
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

#endif	

	
    GSM_CLEAN_RX();

	//开启定时器， 准备发送心跳包
	Timer2_Init_Config();
	isNetWork = 0;
	 
	while(1){
    
         if(GetIsReceiveGsmData() != 0x00){
			if(GetIsReceiveGsmData() == 1){
				//处理网络中断
				SetIsReceiveGsmData(0);
				 printf("\r\n 接受服务器指令  开始处理");
				//处理GSM数据
				Gsm_DealwithServerNetProtocolData();
			}else if(GetIsReceiveGsmData() == 2){
				SetIsReceiveGsmData(0);
				Gsm_SendNetDataPackToServer(HeartBeatFunctionType,0,0);
			}else if( GetIsReceiveGsmData() == 3 ){
				//在运行过程处理串口FLASH配置
				isNetWork = 1;
				SetIsReceiveGsmData(0);
				FLASH_USART_ConfigDeviceParameter(usart_1_Buff,71);
				Clear_Usart1_Rec();
				// 重启设备

				
				Gprs_Led_Close();
				printf("\r\n 更新FLASH   设备重启 r\n");
				GSM_DELAY(1000); 
				
				
				
				GSM_PWRKEY_Open();
				Delay_ms(1000);
				Delay_ms(1000);
				GSM_PWRKEY_Close();
				 
				Delay_ms(5000);
				 
				Soft_Reset();
		
				isNetWork = 0;
			}else if( GetIsReceiveGsmData() == 4 ){
				//此处为服务器测试使用 
				//实际情况有改
				isNetWork = 1;
				SetIsReceiveGsmData(0);
				printf("\r\n 发送采集器数据包");
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



