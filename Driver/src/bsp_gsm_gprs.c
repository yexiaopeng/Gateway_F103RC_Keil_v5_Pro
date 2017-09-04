#include "bsp_gsm_gprs.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
 
#include "bsp_gsm_usart.h"

/*
 * ��������: GSM ��Դ ��ʼ������
 * �������: ��
 * �� �� ֵ: ��
 * ˵    ������
 */
extern void GSM_POWER_Init(void){
	/* ����IOӲ����ʼ���ṹ����� */
	GPIO_InitTypeDef GPIO_InitStructure;
	/* ʹ��USART����GPIOʱ�� */
	GSM_POWER_GPIO_ClockCmd(GSM_POWER_GPIO_CLK,ENABLE);

	/* �趨USART���Ͷ�ӦIO��� */
	GPIO_InitStructure.GPIO_Pin =  GSM_POWER_GPIO_PIN;
	/* �趨USART���Ͷ�ӦIOģʽ������������� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	/* �趨USART���Ͷ�ӦIO�������ٶ� ��GPIO_Speed_50MHz */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	/* ��ʼ��USART���Ͷ�ӦIO */
	GPIO_Init(GSM_POWER_GPIO_PORT, &GPIO_InitStructure);
}

/*
 * ��������: GSM ��Դ  ��
 * �������: ��
 * �� �� ֵ: ��
 * ˵    ������
 */
extern void GSM_POWER_Open(void){
	GPIO_SetBits(GSM_POWER_GPIO_PORT,GSM_POWER_GPIO_PIN);
}

/*
 * ��������: GSM ��Դ ��
 * �������: ��
 * �� �� ֵ: ��
 * ˵    ������
 */
extern void GSM_POWER_Close(void){
	GPIO_ResetBits(GSM_POWER_GPIO_PORT,GSM_POWER_GPIO_PIN);
}


extern void GSM_PWRKEY_Init(void){
	/* ����IOӲ����ʼ���ṹ����� */
	GPIO_InitTypeDef GPIO_InitStructure;
	/* ʹ��USART����GPIOʱ�� */
	GSM_PWRKEY_GPIO_ClockCmd(GSM_PWRKEY_GPIO_CLK,ENABLE);

	/* �趨USART���Ͷ�ӦIO��� */
	GPIO_InitStructure.GPIO_Pin =  GSM_PWRKEY_GPIO_PIN;
	/* �趨USART���Ͷ�ӦIOģʽ������������� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	/* �趨USART���Ͷ�ӦIO�������ٶ� ��GPIO_Speed_50MHz */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	/* ��ʼ��USART���Ͷ�ӦIO */
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



//0��ʾ�ɹ���1��ʾʧ��

uint8_t gsm_cmd(char *cmd, char *reply,uint32_t waittime )
{    
		GSM_DEBUG_FUNC();
	
    GSM_CLEAN_RX();                 //����˽��ջ���������

    GSM_TX(cmd);                    //��������

	  GSM_DEBUG("Send cmd:%s",cmd);	
	
    if(reply == 0)                      //����Ҫ��������
    {
        return GSM_TRUE;
    }
    
    GSM_DELAY(waittime);                 //��ʱ
    
    
    return gsm_cmd_check(reply);    //�Խ������ݽ��д���
}


//0��ʾ�ɹ���1��ʾʧ��
uint8_t gsm_cmd_check(char *reply)
{
    uint8_t len;
    uint8_t n;
    uint8_t off;
    char *redata;
    
		GSM_DEBUG_FUNC();

    redata = GSM_RX(len);   //��������
   	  
	  *(redata+len) = '\0';
	  GSM_DEBUG("Reply:%s",redata);	

//		GSM_DEBUG_ARRAY((uint8_t *)redata,len);
	
    n = 0;
    off = 0;
    while((n + off)<len)
    {
        if(reply[n] == 0)                 //����Ϊ�ջ��߱Ƚ����
        {
            return GSM_TRUE;
        }
        
        if(redata[ n + off]== reply[n])
        {
            n++;                //�ƶ�����һ����������
        }
        else
        {
            off++;              //������һ��ƥ��
            n=0;                //����
        }
        //n++;
    }

    if(reply[n]==0)   //�պ�ƥ�����
    {
        return GSM_TRUE;
    }
    
    return GSM_FALSE;       //����ѭ����ʾ�Ƚ���Ϻ�û����ͬ�����ݣ��������
}

char * gsm_waitask(uint8_t waitask_hook(void))      //�ȴ�������Ӧ��
{
    uint8_t len=0;
    char *redata;
	
		GSM_DEBUG_FUNC();
	
    do{
        redata = GSM_RX(len);   //��������
			
			
        if(waitask_hook!=0)
        {
            if(waitask_hook()==GSM_TRUE)           //���� GSM_TRUE ��ʾ��⵽�¼�����Ҫ�˳�
            {
                redata = 0;
                return redata;               
            }
        }
    }while(len==0);                 //��������Ϊ0ʱһֱ�ȴ�
    				
		GSM_DEBUG_ARRAY((uint8_t *)redata,len);

    
    GSM_DELAY(20);              //��ʱ��ȷ���ܽ��յ�ȫ�����ݣ�115200�������£�ÿms�ܽ���11.52���ֽڣ�
    return redata;
}



//��ʼ�������ģ��
//0��ʾ�ɹ���1��ʾʧ��
uint8_t gsm_init(void)
{
	char *redata;
	uint8_t len;
	
	GSM_DEBUG_FUNC();  

	GSM_CLEAN_RX();                 //����˽��ջ���������
	GSM_USART_Config();					//��ʼ������
	
   
	if(gsm_cmd("AT+CGMM\r","OK", 100) != GSM_TRUE)
	{
			return GSM_FALSE;
	}
	
	redata = GSM_RX(len);   //��������

	if(len == 0)
	{
			return GSM_FALSE;
	}
	//���������GSM900A��GSM800A��GSM800C
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

//����Ƿ��п�
//0��ʾ�ɹ���1��ʾʧ��
uint8_t IsInsertCard(void)
{
	GSM_DEBUG_FUNC();
	
	GSM_CLEAN_RX();
	return gsm_cmd("AT+CNUM\r","OK",200);
   
}
      /**
 * @brief  ��ʼ��GPRS����
 * @param  ��
 * @retval ʧ��GSM_FALSE  �ɹ�GSM_TRUE
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
 * @brief  ����TCP���ӣ���ȴ�ʱ��20�룬�����и��������޸�
	* @param  localport: ���ض˿�
	* @param  serverip: ������IP
	* @param  serverport: �������˿�
 * @retval ʧ��GSM_FALSE  �ɹ�GSM_TRUE
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
	
	//����Ƿ�������
	while(gsm_cmd_check("CONNECT OK") != GSM_TRUE)
	{		
		if(++testConnect >200)//��ȴ�20��
		{
			return GSM_FALSE;
		}
		GSM_DELAY(100); 		
	}				
	return GSM_TRUE;
}

/**
 * @brief  ����UDP���ӣ���ȴ�ʱ��20�룬�����и��������޸�
	* @param  localport: ���ض˿�
	* @param  serverip: ������IP
	* @param  serverport: �������˿�
 * @retval ʧ��GSM_FALSE  �ɹ�GSM_TRUE
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
	
		//����Ƿ�������
	while(gsm_cmd_check("CONNECT OK") != GSM_TRUE )
	{
		
		if(++testConnect >200)//��ȴ�20��
		{
			return GSM_FALSE;
		}
		GSM_DELAY(100); 		
	}		
		
	return GSM_TRUE;
}

/**
 * @brief  ʹ��GPRS�������ݣ�����ǰ��Ҫ�Ƚ���UDP��TCP����
	* @param  str: Ҫ���͵�����
 * @retval ʧ��GSM_FALSE  �ɹ�GSM_TRUE
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
	}
	else
	{
		
gprs_send_failure:
		
		end = 0x1B;
		gsm_cmd(&end,0,0);	//ESC,ȡ������ 

		return GSM_FALSE;
	}
}

/**
 * @brief  �Ͽ���������
 * @retval ʧ��GSM_FALSE  �ɹ�GSM_TRUE
 */
uint8_t gsm_gprs_link_close(void)              //IP���ӶϿ�
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
 * @brief  �رճ���
 * @retval ʧ��GSM_FALSE  �ɹ�GSM_TRUE
 */
uint8_t gsm_gprs_shut_close(void)               //�رճ���
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
 * @brief  ���ش�GSMģ����յ����������ݣ���ӡ������
 * @retval ʧ��GSM_FALSE  �ɹ�GSM_TRUE
 */
uint8_t PostGPRS(void)
{

	char *redata;
	uint8_t len;

	GSM_DEBUG_FUNC();  

	redata = GSM_RX(len);   //�������� 

	if(len == 0)
	{
			return GSM_FALSE;
	}
	
	printf("PostTCP:%s\n",redata);
	GSM_CLEAN_RX();
	
	return GSM_TRUE;

}


//���� ����ATָ��ִ�еķ��ؽ�����ǲ���ֵ ������ֵ����
//������д��������ȡ��ֵ����
static void gsm_query(char * cmd,char * returnValue,uint32_t waittime){
	uint8_t len;
	char *redata;
	GSM_CLEAN_RX();                 //����˽��ջ���������
	GSM_TX(cmd);                    //��������
    GSM_DELAY(waittime);                 //��ʱ
    
	redata = GSM_RX(len);   //��������

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
	//data ��ʽ  AT+CSQ +CSQ: 31,0  ����31��ʾRSSI ��Ϊ�ź�ǿ��  0-99       0��ʾ����
   //�ʣ�ԽСԽ��
   //for ���� ,
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

