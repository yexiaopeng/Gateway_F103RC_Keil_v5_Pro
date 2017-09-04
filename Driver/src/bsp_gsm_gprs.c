#include "bsp_gsm_gprs.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
 
#include "bsp_gsm_usart.h"

/*
 * 函数功能: GSM 电源 初始化配置
 * 输入参数: 无
 * 返 回 值: 无
 * 说    明：无
 */
extern void GSM_POWER_Init(void){
	/* 定义IO硬件初始化结构体变量 */
	GPIO_InitTypeDef GPIO_InitStructure;
	/* 使能USART功能GPIO时钟 */
	GSM_POWER_GPIO_ClockCmd(GSM_POWER_GPIO_CLK,ENABLE);

	/* 设定USART发送对应IO编号 */
	GPIO_InitStructure.GPIO_Pin =  GSM_POWER_GPIO_PIN;
	/* 设定USART发送对应IO模式：复用推挽输出 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	/* 设定USART发送对应IO最大操作速度 ：GPIO_Speed_50MHz */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	/* 初始化USART发送对应IO */
	GPIO_Init(GSM_POWER_GPIO_PORT, &GPIO_InitStructure);
}

/*
 * 函数功能: GSM 电源  开
 * 输入参数: 无
 * 返 回 值: 无
 * 说    明：无
 */
extern void GSM_POWER_Open(void){
	GPIO_SetBits(GSM_POWER_GPIO_PORT,GSM_POWER_GPIO_PIN);
}

/*
 * 函数功能: GSM 电源 关
 * 输入参数: 无
 * 返 回 值: 无
 * 说    明：无
 */
extern void GSM_POWER_Close(void){
	GPIO_ResetBits(GSM_POWER_GPIO_PORT,GSM_POWER_GPIO_PIN);
}


extern void GSM_PWRKEY_Init(void){
	/* 定义IO硬件初始化结构体变量 */
	GPIO_InitTypeDef GPIO_InitStructure;
	/* 使能USART功能GPIO时钟 */
	GSM_PWRKEY_GPIO_ClockCmd(GSM_PWRKEY_GPIO_CLK,ENABLE);

	/* 设定USART发送对应IO编号 */
	GPIO_InitStructure.GPIO_Pin =  GSM_PWRKEY_GPIO_PIN;
	/* 设定USART发送对应IO模式：复用推挽输出 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	/* 设定USART发送对应IO最大操作速度 ：GPIO_Speed_50MHz */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	/* 初始化USART发送对应IO */
	GPIO_Init(GSM_PWRKEY_GPIO_PORT, &GPIO_InitStructure);
}
extern void GSM_PWRKEY_Open(void){
//	GPIO_ResetBits(GSM_PWRKEY_GPIO_PORT,GSM_PWRKEY_GPIO_PIN);
//	Delay(2500);//2.5s
//	GPIO_SetBits(GSM_PWRKEY_GPIO_PORT,GSM_PWRKEY_GPIO_PIN);

	//GPIO_ResetBits(GSM_PWRKEY_GPIO_PORT,GSM_PWRKEY_GPIO_PIN);
   // GPIO_ResetBits(GSM_PWRKEY_GPIO_PORT,GSM_PWRKEY_GPIO_PIN);
   // Delay(2500);//2.5s
    GPIO_SetBits(GSM_PWRKEY_GPIO_PORT,GSM_PWRKEY_GPIO_PIN);
   // Delay(2500);//2.5s
   // GPIO_ResetBits(GSM_PWRKEY_GPIO_PORT,GSM_PWRKEY_GPIO_PIN);
}

extern void GSM_PWRKEY_Close(void){
   GPIO_ResetBits(GSM_PWRKEY_GPIO_PORT,GSM_PWRKEY_GPIO_PIN);
}



//0表示成功，1表示失败

uint8_t gsm_cmd(char *cmd, char *reply,uint32_t waittime )
{    
		GSM_DEBUG_FUNC();
	
    GSM_CLEAN_RX();                 //清空了接收缓冲区数据

    GSM_TX(cmd);                    //发送命令

	  GSM_DEBUG("Send cmd:%s",cmd);	
	
    if(reply == 0)                      //不需要接收数据
    {
        return GSM_TRUE;
    }
    
    GSM_DELAY(waittime);                 //延时
    
    
    return gsm_cmd_check(reply);    //对接收数据进行处理
}


//0表示成功，1表示失败
uint8_t gsm_cmd_check(char *reply)
{
    uint8_t len;
    uint8_t n;
    uint8_t off;
    char *redata;
    
		GSM_DEBUG_FUNC();

    redata = GSM_RX(len);   //接收数据
   	  
	  *(redata+len) = '\0';
	  GSM_DEBUG("Reply:%s",redata);	

//		GSM_DEBUG_ARRAY((uint8_t *)redata,len);
	
    n = 0;
    off = 0;
    while((n + off)<len)
    {
        if(reply[n] == 0)                 //数据为空或者比较完毕
        {
            return GSM_TRUE;
        }
        
        if(redata[ n + off]== reply[n])
        {
            n++;                //移动到下一个接收数据
        }
        else
        {
            off++;              //进行下一轮匹配
            n=0;                //重来
        }
        //n++;
    }

    if(reply[n]==0)   //刚好匹配完毕
    {
        return GSM_TRUE;
    }
    
    return GSM_FALSE;       //跳出循环表示比较完毕后都没有相同的数据，因此跳出
}

char * gsm_waitask(uint8_t waitask_hook(void))      //等待有数据应答
{
    uint8_t len=0;
    char *redata;
	
		GSM_DEBUG_FUNC();
	
    do{
        redata = GSM_RX(len);   //接收数据
			
			
        if(waitask_hook!=0)
        {
            if(waitask_hook()==GSM_TRUE)           //返回 GSM_TRUE 表示检测到事件，需要退出
            {
                redata = 0;
                return redata;               
            }
        }
    }while(len==0);                 //接收数据为0时一直等待
    				
		GSM_DEBUG_ARRAY((uint8_t *)redata,len);

    
    GSM_DELAY(20);              //延时，确保能接收到全部数据（115200波特率下，每ms能接收11.52个字节）
    return redata;
}



//初始化并检测模块
//0表示成功，1表示失败
uint8_t gsm_init(void)
{
	char *redata;
	uint8_t len;
	
	GSM_DEBUG_FUNC();  

	GSM_CLEAN_RX();                 //清空了接收缓冲区数据
	GSM_USART_Config();					//初始化串口
	
   
	if(gsm_cmd("AT+CGMM\r","OK", 100) != GSM_TRUE)
	{
			return GSM_FALSE;
	}
	
	redata = GSM_RX(len);   //接收数据

	if(len == 0)
	{
			return GSM_FALSE;
	}
	//本程序兼容GSM900A、GSM800A、GSM800C
	if (strstr(redata,"SIMCOM_GSM900A") != 0)
	{
		return GSM_TRUE;
	}
	else if(strstr(redata,"SIMCOM_SIM800") != 0)
	{
		return GSM_TRUE;
	}
	else
		return GSM_FALSE;

}

//检测是否有卡
//0表示成功，1表示失败
uint8_t IsInsertCard(void)
{
	GSM_DEBUG_FUNC();
	
	GSM_CLEAN_RX();
	return gsm_cmd("AT+CNUM\r","OK",200);
   
}
      /**
 * @brief  初始化GPRS网络
 * @param  无
 * @retval 失败GSM_FALSE  成功GSM_TRUE
 */
uint8_t gsm_gprs_init(void)
{
	GSM_DEBUG_FUNC();  
		
	GSM_CLEAN_RX();
  Delay_ms(2000);
	if(gsm_cmd("AT+CGCLASS=\"B\"\r","OK", 200) != GSM_TRUE) return GSM_FALSE;

	GSM_CLEAN_RX();
  Delay_ms(2000);
	if(gsm_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"\r","OK", 200) != GSM_TRUE) return GSM_FALSE;		

	GSM_CLEAN_RX();
  Delay_ms(2000);	
	if(gsm_cmd("AT+CGATT=1\r","OK", 200)!= GSM_TRUE) return GSM_FALSE;

	GSM_CLEAN_RX();
  Delay_ms(2000);
	if(gsm_cmd("AT+CIPCSGP=1,\"CMNET\"\r","OK", 200)!= GSM_TRUE) return GSM_FALSE;
	
	return GSM_TRUE;
}

/**
 * @brief  建立TCP连接，最长等待时间20秒，可自行根据需求修改
	* @param  localport: 本地端口
	* @param  serverip: 服务器IP
	* @param  serverport: 服务器端口
 * @retval 失败GSM_FALSE  成功GSM_TRUE
 */
uint8_t gsm_gprs_tcp_link(char *localport,char * serverip,char * serverport)
{
	char cmd_buf[100];
	uint8_t testConnect=0;
	
	GSM_DEBUG_FUNC();  
		
	sprintf(cmd_buf,"AT+CLPORT=\"TCP\",\"%s\"\r",localport);
	
	if(gsm_cmd(cmd_buf,"OK", 100)!= GSM_TRUE)
		return GSM_FALSE;
	
	GSM_CLEAN_RX();
		
	sprintf(cmd_buf,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"\r",serverip,serverport);	
	gsm_cmd(cmd_buf,0, 100);	
	
	//检测是否建立连接
	while(gsm_cmd_check("CONNECT OK") != GSM_TRUE)
	{		
		if(++testConnect >200)//最长等待20秒
		{
			return GSM_FALSE;
		}
		GSM_DELAY(100); 		
	}				
	return GSM_TRUE;
}

/**
 * @brief  建立UDP连接，最长等待时间20秒，可自行根据需求修改
	* @param  localport: 本地端口
	* @param  serverip: 服务器IP
	* @param  serverport: 服务器端口
 * @retval 失败GSM_FALSE  成功GSM_TRUE
 */
uint8_t gsm_gprs_udp_link(char *localport,char * serverip,char * serverport)
{
	char cmd_buf[100];
	uint8_t testConnect=0;
	
	GSM_DEBUG_FUNC();  

	sprintf(cmd_buf,"AT+CLPORT=\"UDP\",\"%s\"\r",localport);
	
	if(gsm_cmd(cmd_buf,"OK", 100)!= GSM_TRUE)
		return GSM_FALSE;
	
	GSM_CLEAN_RX();
		
	sprintf(cmd_buf,"AT+CIPSTART=\"UDP\",\"%s\",\"%s\"\r",serverip,serverport);
	
	gsm_cmd(cmd_buf,0, 100);	
	
		//检测是否建立连接
	while(gsm_cmd_check("CONNECT OK") != GSM_TRUE )
	{
		
		if(++testConnect >200)//最长等待20秒
		{
			return GSM_FALSE;
		}
		GSM_DELAY(100); 		
	}		
		
	return GSM_TRUE;
}

/**
 * @brief  使用GPRS发送数据，发送前需要先建立UDP或TCP连接
	* @param  str: 要发送的数据
 * @retval 失败GSM_FALSE  成功GSM_TRUE
 */
uint8_t gsm_gprs_send(const char * str)
{
	char end = 0x1A;
	uint8_t testSend=0;
	
	GSM_DEBUG_FUNC();

	GSM_CLEAN_RX();
	
	if( gsm_cmd("AT+CIPSEND\r",">",500) == 0)
	{
		GSM_USART_printf("%s",str);
		//printf("Send String:%s",str);

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
	}
	else
	{
		
gprs_send_failure:
		
		end = 0x1B;
		gsm_cmd(&end,0,0);	//ESC,取消发送 

		return GSM_FALSE;
	}
}

/**
 * @brief  断开网络连接
 * @retval 失败GSM_FALSE  成功GSM_TRUE
 */
uint8_t gsm_gprs_link_close(void)              //IP链接断开
{
	GSM_DEBUG_FUNC();  

	GSM_CLEAN_RX();
	if(gsm_cmd("AT+CIPCLOSE=1\r","OK",200) != GSM_TRUE)
    {
        return GSM_FALSE;
    }
	return GSM_TRUE;
}

/**
 * @brief  关闭场景
 * @retval 失败GSM_FALSE  成功GSM_TRUE
 */
uint8_t gsm_gprs_shut_close(void)               //关闭场景
{
	uint8_t  check_time=0;
	GSM_DEBUG_FUNC();  

	GSM_CLEAN_RX();	
	gsm_cmd("AT+CIPSHUT\r",0,0) ;
	while(	gsm_cmd_check("OK") != GSM_TRUE)
	{
		if(++check_time >50)
			return GSM_FALSE;
		
		GSM_DELAY(200);
	}

	return GSM_TRUE;
}

/**
 * @brief  返回从GSM模块接收到的网络数据，打印到串口
 * @retval 失败GSM_FALSE  成功GSM_TRUE
 */
uint8_t PostGPRS(void)
{

	char *redata;
	uint8_t len;

	GSM_DEBUG_FUNC();  

	redata = GSM_RX(len);   //接收数据 

	if(len == 0)
	{
			return GSM_FALSE;
	}
	
	printf("PostTCP:%s\n",redata);
	GSM_CLEAN_RX();
	
	return GSM_TRUE;

}


//由于 部分AT指令执行的返回结果不是布尔值 而是数值变量
//所以另写函数，获取数值变量
static void gsm_query(char * cmd,char * returnValue,uint32_t waittime){
	uint8_t len;
	char *redata;
	GSM_CLEAN_RX();                 //清空了接收缓冲区数据
	GSM_TX(cmd);                    //发送命令
    GSM_DELAY(waittime);                 //延时
    
	redata = GSM_RX(len);   //接收数据

	*(redata+len) = '\0';
	
	if(len < 100){
		strcpy(returnValue,redata);
	}else{
		strncpy(returnValue,redata,100);
	}
}

extern u8  gsm_gprs_Get_SignalStrength(){
	u8 i;
	u8 SignalStrength = 0;
	char data[100];
	gsm_query("AT+CSQ\r",data, 100);
	//printf("\r\n %s",data);
	//data 格式  AT+CSQ +CSQ: 31,0  其中31表示RSSI 认为信号强度  0-99       0表示误码
   //率，越小越好
   //for 查找 ,
   for (i = 0; i <100; ++i)
   {
   		if(data[i] == ','){
			if(i >= 2 ){
				SignalStrength = (data[i-2] - 48)*10 + data[i-1]-48;  
				break;
			}
		}
   }
   
   //printf("\r\n SignalStrength = %d",SignalStrength);
	return SignalStrength;		

	
}


extern void Restart_Device(void){
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

