#ifndef BSP_PROTOCOL_H_
#define BSP_PROTOCOL_H_

#include "stm32f10x.h"




#pragma pack(push)
#pragma pack(1)

//�����Ǳ䳤�ģ�����ʱֻ��������



//�������ݰ���ͷ
typedef struct{
	u8 hander[4]; //��ͷ 0XFA 0XF1 0X1F 0XAF 
	u8 length[2];   //���ݳ��� 2�ֽ�
	uint64_t addr;
	u8 function;
}ProtocolHeart;

//�������ݰ���һЩ����
typedef struct{
	u8  ServerIP[12]; // 192 168 010  020
	u16 ServerPort;
	u8  DeviceAddr[8];
}ProtocolParameter;


//������ָ����� ��������������ʹ��
enum ServerProtocolFunctionType{
    ConfigFunctionType         = 0x01,        //0x01������IP���˿�
	FirmwareUpdateFuncitonType = 0x02,        //0x02���̼�����
	RestartDeviceFuntionType   = 0x04,        //0x04�������ɼ���
	SponsorRegisteredFunctionType    = 0xA1,  //0xA1������ע��
	RegisteredFunctionType        = 0XA2,     //0xA2��ע��ָ��
	OverRegisteredFunctionType    = 0XA4      //0xA4������ע��
};


#pragma pack(pop)

//�ɼ����豸�ͻ���ָ�����  �ϴ����ݵ�������ʹ��
//ע��������������Ӧ�� ���Խ�������ָ��Ҳ�����ϴ�����
enum DeviceProtocolFunctionType{
	HeartBeatFunctionType = 0x10,			//0x10��������
	DeviceStateFunctionType = 0x20,			//0x20���ɼ���״̬
	InverterDataPackFunctionType = 0x50,	//0x50����������ݰ�
	DeviceErrorFunctionType = 0x80			//0x80������
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


//ȫ�ֱ����洢
extern  ProtocolParameter  DeiverProtocolParameter;

//
//extern unsigned char Gsm_SendNetProtocolData(u8 function); 

//������ܵķ���������
//extern void Gsm_DealwithServerNetProtocolData(u8 * addr, u8 function,u8 * data, u16 dataLength);

extern void Gsm_DealwithServerNetProtocolData();


#endif
