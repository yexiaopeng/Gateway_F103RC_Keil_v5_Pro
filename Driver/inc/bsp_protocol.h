#ifndef BSP_PROTOCOL_H_
#define BSP_PROTOCOL_H_

#include "stm32f10x.h"




#pragma pack(push)
#pragma pack(1)

//数据是变长的，我暂时只能这样了



//网络数据包报头
typedef struct{
	u8 hander[4]; //包头 0XFA 0XF1 0X1F 0XAF 
	u8 length[2];   //数据长度 2字节
	uint64_t addr;
	u8 function;
}ProtocolHeart;

//网络数据包的一些参数
typedef struct{
	u8  ServerIP[12]; // 192 168 010  020
	u16 ServerPort;
	u8  DeviceAddr[8];
}ProtocolParameter;


//服务器指令类别 解析服务器数据使用
enum ServerProtocolFunctionType{
    ConfigFunctionType         = 0x01,        //0x01：配置IP及端口
	FirmwareUpdateFuncitonType = 0x02,        //0x02：固件升级
	RestartDeviceFuntionType   = 0x04,        //0x04：重启采集器
	SponsorRegisteredFunctionType    = 0xA1,  //0xA1：发起注册
	RegisteredFunctionType        = 0XA2,     //0xA2：注册指令
	OverRegisteredFunctionType    = 0XA4      //0xA4：结束注册
};


#pragma pack(pop)

//采集器设备客户端指令类别  上传数据到服务器使用
//注：由于做服务器应答 所以将服务器指令也加入上传函数
enum DeviceProtocolFunctionType{
	HeartBeatFunctionType = 0x10,			//0x10：心跳包
	DeviceStateFunctionType = 0x20,			//0x20：采集器状态
	InverterDataPackFunctionType = 0x50,	//0x50：逆变器数据包
	DeviceErrorFunctionType = 0x80			//0x80：报错
};

enum NetProtocolServerPackHeart{
	PACK_Heart_A =   0XFA,
	PACK_Heart_B =   0XF1,
	PACK_Heart_C =   0X1F,
	PACK_Heart_D =   0XAF 
};
 
enum NetProtocolServerPackEnd{
	PACK_END_A =   0XFA,
	PACK_END_B =   0X1F,
	PACK_END_C =   0XF1,
	PACK_END_D =   0XAF 
};


//全局变量存储
extern  ProtocolParameter  DeiverProtocolParameter;

//
//extern unsigned char Gsm_SendNetProtocolData(u8 function); 

//处理接受的服务器数据
//extern void Gsm_DealwithServerNetProtocolData(u8 * addr, u8 function,u8 * data, u16 dataLength);

extern void Gsm_DealwithServerNetProtocolData();


#endif
