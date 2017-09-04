#include "bsp_Timer.h"

u8 isNetWork;//�жϳ����Ƿ��ڴ�������Ӧ�� 
volatile u32  sec=0;//��ʱ������ ����������
volatile u32  sec_2=0;//��ʱ������ ���ڲɼ���״̬

/*******************************************************************************
* ������  : Timer2_Init_Config
* ����    : Timer2��ʼ������
* ����    : ��
* ���    : ��
* ����    : �� 
* ˵��    : 1s��ʱ
*******************************************************************************/
extern void Timer2_Init_Config(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);		//ʹ��Timer2ʱ��
	
	TIM_TimeBaseStructure.TIM_Period = 9999;					//��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ(������10000Ϊ1s)
	TIM_TimeBaseStructure.TIM_Prescaler = 7199;					//����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ(10KHz�ļ���Ƶ��)
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//����ʱ�ӷָ�:TDTS = TIM_CKD_DIV1
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);				//����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	/*�ж����ȼ�NVIC����*/
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;				//TIM2�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//��ռ���ȼ�1��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//ʹ��IRQͨ��
	NVIC_Init(&NVIC_InitStructure); 							//��ʼ��NVIC�Ĵ���
	 
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE ); 				//ʹ��TIM2ָ�����ж�
	
	TIM_Cmd(TIM2, ENABLE);  									//ʹ��TIMx����
}


/*TIM3_IRQHandler */
void TIM2_IRQHandler(){
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update); 

	sec++;
	sec_2++;


	//���Ͳɼ������ݰ�ʱ������������
	if(60 == sec_2 ){
		sec_2 = 0;
		sec = 0;
		if(0 == isNetWork){
			//����������
			SetIsReceiveGsmData(4);
		}
		//���Ͳɼ������ݰ�
		
			
	}



	//60s һ��������
	if(30 == sec){
		sec = 0;
         printf("\r\n �������� \r\n ");
		//TODO ���赱ǰδ��������Ӧ�� ��������
		if(0 == isNetWork){
			//����������
			SetIsReceiveGsmData(2);
		}//else  ������		
	}

	
}



























