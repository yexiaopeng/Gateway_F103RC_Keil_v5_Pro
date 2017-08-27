#include "bsp_protocol.h"
#include "bsp_start_led.h"
#include "bsp_gsm_gprs.h"
#include "stdio.h"
#include "string.h"
#include "bsp_Timer.h" 

#define DEBUG  1



//内部变量
static const u8 MaxdataLength = 255; //最大长度
static    u8 dataBuff[MaxdataLength]; //数据包存储
static u16 dataBuffLength;//数据包长度
static u16 heartIndex;//包头位置
static u16 dataLength;//数据长度 不含包头包尾



static u8 DeviceAddr[8] = {0X31,0X32,0X33,0X34,0X35,0X36,0X37,0X38}; //采集器ID
static ProtocolHeart protocolHeart;

#define NETDATAPACKLENGTH_H_INDEX 4 
#define NETDATAPACKLENGTH_L_INDEX 5 







//******************处理函数

//******************处理服务器指令
//更换IP和端口
static void ConfigFunctionTypeManager(u8 * addr,u8 * data, u16 length);
static void FirmwareUpdateFuncitonTypeManager(u8 * addr,u8 * data, u16 length);
static void RestartDeviceFuntionTypeManager(u8 * addr,u8 * data, u16 length);
static void SponsorRegisteredFunctionTypeManager(u8 * addr,u8 * data, u16 length);
static void RegisteredFunctionTypeManager(u8 * addr,u8 * data, u16 length);
static void OverRegisteredFunctionTypeManager(u8 * addr,u8 * data, u16 length);
//******************应答和上传服务器
extern  void ConfigFunctionTypeResponseManager();





//清理临时变量
static void Gsm_ClearData(){
	memset(dataBuff,0,MaxdataLength);
	dataBuffLength = 0;
}


//设置采集器ID
//存储采集器ID 8字节asc码
extern void SetDeviceAddr(u8 * tempDeviceAddr){
	memset(DeviceAddr,0,8);
	memcpy(DeviceAddr,tempDeviceAddr,8);
	
	//包头 0XFA 0XF1 0X1F 0XAF 
	protocolHeart.hander[0] = 0XFA;
	protocolHeart.hander[1] = 0XF1;
	protocolHeart.hander[2] = 0X1F;
	protocolHeart.hander[3] = 0XAF;
	
	//地址
	memcpy(&(protocolHeart.addr),tempDeviceAddr,8);
}

//获取采集器ID
extern void GetDeviceAddr(u8 ** tempDeviceAddr){
	memset(*tempDeviceAddr,0,8);
	memcpy(*tempDeviceAddr,DeviceAddr,8);
}



//将GSM获取到的网络数据进行拷贝
extern u8 Gsm_CopyNetDataToDataBuff(u8 * netData,u16 netDataLength){
	u16 i;
	
	//拷贝数据 
	memcpy(dataBuff,netData,netDataLength);
	
	//寻找包头 反向寻找   防止前一包的干扰
	for( i = netDataLength ; i > 0; i-- ){
		if( (dataBuff[i-3] == PACK_Heart_A) && ( dataBuff[i-2] == PACK_Heart_B) &&
				(dataBuff[i-1] == PACK_Heart_C) && (dataBuff[i] == PACK_Heart_D)){
			heartIndex = i-3;
			break;
		}
	}
	if( (heartIndex == 0) && (netDataLength == i) ){
		return 2;//丢包
	} 

	//判断数据包长度是否正确
	//防止粘包
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
		
		//检测是否发送完成
		while(gsm_cmd_check("SEND OK") != GSM_TRUE )
		{		
			if(++testSend >200)//最长等待20秒
			{
				goto gprs_send_failure;
			}
			GSM_DELAY(100);
			 
		}
		return GSM_TRUE;		
		 
	}else{
	gprs_send_failure:
		
		end = 0x1B;
		gsm_cmd(&end,0,0);	//ESC,取消发送 

		return GSM_FALSE;
	}
		

}




//向服务器发送数据
/* 函数名称 Gsm_SendNetDataPackToServer
 * 函数作用 向服务器发送数据
 * 输入参数 u8 function   指令类别
			u8 * data   数据
			u16 dataLength   数据长度
 */
extern void Gsm_SendNetDataPackToServer(u8 function, u8 * data, u16 dataLength){
	
	switch(function){
		case HeartBeatFunctionType:{
			//0x10：心跳包
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
		    //0x20：采集器状态
		}break;
		
		case InverterDataPackFunctionType:{
		    //0x50：逆变器数据包
		}break;
		
		case DeviceErrorFunctionType:{
		   //0x80：报错
		}break;
	
		
		//以下为服务器应答
		//TODO  是否可以取消
	    case ConfigFunctionType:{
			//重新设置IP PORT
			 ConfigFunctionTypeResponseManager(data,dataLength);
		}break;
		
		case FirmwareUpdateFuncitonType:{
		     //固件升级
			 
		}break;
		
		case RestartDeviceFuntionType:{
		     //重启采集器
		}break;
		
		case SponsorRegisteredFunctionType:{
		      //发起注册
		}break;
		
		case RegisteredFunctionType:{
		    //注册指令
		}break;
		
		case OverRegisteredFunctionType:{
		     //结束注册
		}break;
		
		default:{
			printf("\r\n Gsm_SendNetDataPackToServer 未定义指令");
		}
	}
}


/*
 *  函数名称 Gsm_DealwithServerNetProtocolData
 *  函数作用 处理接受的服务器数据
 *  输入参数 u8 * addr  --  地址   固定8字节
 *			u8 function --  功能
 *			u8 * data   --  数据数组
 *			u16 dataLength  --  数据长度
 */
extern void Gsm_DealwithServerNetProtocolData(){
	printf("\r\n Gsm_DealwithServerNetProtocolData  ");
	u8 addr[8];//地址
	u8 function;//功能
	u8 data[200];//数据变长
	//u16 dataLength ;//

	//设置标志位  表明正在处理网络应答
	isNetWork = 1;



	
	//提取数据包
	memcpy(addr,&dataBuff[heartIndex+6],8);
	function = dataBuff[heartIndex+14];
	memcpy(data, &dataBuff[heartIndex+15],dataLength-13);  
	
	switch(function){
		case ConfigFunctionType:{
			//重新设置IP PORT
			ConfigFunctionTypeManager(addr,data,dataLength-13);
		}break;
		
		case FirmwareUpdateFuncitonType:{
		     //固件升级
			FirmwareUpdateFuncitonTypeManager(addr,data,dataLength-13);
			
		}break;
		
		case RestartDeviceFuntionType:{
		     //重启采集器
			 RestartDeviceFuntionTypeManager(addr,data,dataLength-13);
		}break;
		
		case SponsorRegisteredFunctionType:{
		      //发起注册
			  SponsorRegisteredFunctionTypeManager(addr,data,dataLength-13);
		}break;
		
		case RegisteredFunctionType:{
		    //注册指令
			 RegisteredFunctionTypeManager(addr,data,dataLength-13);
		}break;
		
		case OverRegisteredFunctionType:{
		     //结束注册
			 OverRegisteredFunctionTypeManager(addr,data,dataLength-13);
		}break;
		
		default:{
			printf("\r\n Gsm_DealwithServerNetProtocolData 未定义指令");
		}
	}

	//设置标志位  结束网络应答
	isNetWork = 0;
}








//***********************************************处理函数

//**************************  接受处理函数
//ConfigFunctionType
//重新设置IP PORT
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
	
	
	
	//TODO  转换IP和PORT 
	
	//00: 应答
	ConfigFunctionTypeResponseManager(0,0);
	
	
	//01 积累新的ip和PORT
	
    //02：写入内存
	
	
	//03：重新连接 （是否需要再次重启？？？）

	
	
	
	
}
 
extern  void ConfigFunctionTypeResponseManager(){
	//配置 IP及端口    服务器应答
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
 * 固件升级处理函数
 * FirmwareUpdateFuncitonTypeManager
 *
 */
static void FirmwareUpdateFuncitonTypeManager(u8 * addr,u8 * data, u16 length){
#if 0
	printf("\r\n FirmwareUpdateFuncitonType %d \r\n",length);
	
	u16 i;
 
	//升级包高低位
	//data[0] - [5]
 
	//下载地址
	for(i = 0; i < length -6; i++){
		printf("%c",data[i+6]);
	}
	
	 
#endif 
	u16 i;
	u8 dataBuff[22];
	u8 urlAddr[length-6]; //存储地址
	//01 获取固件下载地址
	 memcpy(urlAddr,&data[6],length-6);
	
	for(i = 0; i < length -6; i++){
		printf("%c",urlAddr[i]);
	}
	
	
	//02 获取固件包
	 //TODO  不会
	
	
	//03 校验固件
	
	
	//04  做应答    
	
	
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
	

	//05：
    //校验成功  升级
    //校验失败  不升级

}

 

/*
 * 重启采集器处理函数
 * RestartDeviceFuntionTypeManager
 * 控制好对GPRS的断电时间  以使GPRS可以重新连接
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

/*
 * 发起注册处理函数
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
	
	//TODO  发起注册
}

/*
 * 注册指令处理函数
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
	
	//TODO 处理注册指令
	
	
	
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
	
	
	//TODO  结束注册
}


//**************************  发送应答处理函数




