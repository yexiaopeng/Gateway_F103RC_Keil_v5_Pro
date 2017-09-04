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

static void InverterDataPackResponseManager(u8 * data, u16 dataLength);





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
			protocolHeart.function = HeartBeatFunctionType;
			protocolHeart.length[0]  = 0x00;
			protocolHeart.length[1]= 0x0e;			 
			memcpy(dataBuff,&protocolHeart,15);			
			dataBuff[15] =  gsm_gprs_Get_SignalStrength();//信号强度
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
		    //InverterDataPackResponseManager(data,dataLength);
		    printf("\r\n InverterDataPackFunctionType");
		    InverterDataPackResponseManager(data,dataLength);
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
			printf("\r\n Gsm_SendNetDataPackToServer 未定义指令 0X%02X",function);
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
	memcpy(data, &dataBuff[heartIndex+15],dataLength-13);  // -13 采集器地址8     数据长度2     CRC2 命令1

	 
	
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
	char ip[16];//默认为 xxx.xxx.xxx.xxx 
	char port[6];
	
	//TODO  转换IP和PORT 
	
	//00: 应答
	ConfigFunctionTypeResponseManager(0,0);
	
	if(length < 16){
		return ;
	}


	
	//01 提取新的ip和PORT      192.016.001.002  有效
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
	


 	//port  判断4 or 5 
 	if(length == 16){
 		//port length 4
 		sprintf(port,"%c%c%c%c",data[12],data[13],data[14],data[15]);
		port[4] = '\0';
 	}else{
 		sprintf(port,"%c%c%c%c%c",data[12],data[13],data[14],data[15],data[16]);
		port[5] = '\0';
 	}

    //02：写入内存
	FLASH_WriteIpConfig(ip, port);
	
	//03：重新连接 （是否需要再次重启？？？）
	
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



static void InverterDataPackResponseManager(u8 * data, u16 dataLength){
#if 1
	//此为测试数据使用
	//数据为自定义数据

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



