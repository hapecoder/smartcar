#include "delay.h"
#include "led.h"
#include "us.h"


unsigned int overcount=0;


void CH_SR04_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructer;


	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	/*TRIG*/
	GPIO_InitStructer.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructer.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructer.GPIO_Pin=GPIO_Pin_3;
	GPIO_Init(GPIOC, &GPIO_InitStructer);
	GPIO_ResetBits(GPIOC,GPIO_Pin_3);
	/*ECHO*/
//	GPIO_InitStructer.GPIO_Mode=GPIO_Mode_IN_FLOATING;
//	GPIO_InitStructer.GPIO_Pin=GPIO_Pin_5;
//	GPIO_Init(GPIOA, & GPIO_InitStructer);
//	GPIO_ResetBits(GPIOA,GPIO_Pin_5);
	/*TIM2*/
//	TIM_DeInit(TIM2);
//	TIM_TimeBaseInitStructer.TIM_Period=999;  //1000us����һ��
//	TIM_TimeBaseInitStructer.TIM_Prescaler=71;  //72��Ƶ  1M
//	TIM_TimeBaseInitStructer.TIM_ClockDivision=TIM_CKD_DIV1;
//	TIM_TimeBaseInitStructer.TIM_CounterMode=TIM_CounterMode_Up;
//	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructer);

//	//TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE );
//	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE );
//	
//	NVIC_InitStructer.NVIC_IRQChannelPreemptionPriority=0;
//	NVIC_InitStructer.NVIC_IRQChannelSubPriority=0;
//	NVIC_InitStructer.NVIC_IRQChannel=TIM2_IRQn;
//	NVIC_InitStructer.NVIC_IRQChannelCmd=ENABLE;
//	NVIC_Init(&NVIC_InitStructer);

//	TIM_Cmd(TIM2, DISABLE);
}


TIM_ICInitTypeDef  TIM2_ICInitStructure;
/*ECHO*/
void TIM2_Cap_Init(u16 arr,u16 psc)
{	 
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	//ʹ��TIM2ʱ��
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //ʹ��GPIOAʱ��
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;  //PA0 ���֮ǰ����  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0 ����  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_0);						 //PA0 ����
	
	//��ʼ����ʱ��2 TIM2	 
	TIM_TimeBaseStructure.TIM_Period = arr; //�趨�������Զ���װֵ 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//Ԥ��Ƶ��   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
  
	//��ʼ��TIM2���벶�����
	TIM2_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  	TIM2_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
  	TIM2_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  	TIM2_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  	TIM2_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
  	TIM_ICInit(TIM2, &TIM2_ICInitStructure);
	
	//�жϷ����ʼ��
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //��ռ���ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //�����ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ��� 
	
	TIM_ITConfig(TIM2,TIM_IT_Update|TIM_IT_CC1,ENABLE);//��������ж� ,����CC1IE�����ж�	
	
  TIM_Cmd(TIM2,ENABLE ); 	//ʹ�ܶ�ʱ��2
 
}


u8  TIM2CH1_CAPTURE_STA=0;	//���벶��״̬		    				
u16	TIM2CH1_CAPTURE_VAL;	//���벶��ֵ


float Senor_Using(void)				//���ؾ���ֵ
{
	float length=0,sum=0;
	int q=0;
		PCout(3)=1; 
		delay_us(20); 
		PCout(3)=0;
		if(TIM2CH1_CAPTURE_STA&0X80)//�ɹ�������һ�θߵ�ƽ
		{
			length=TIM2CH1_CAPTURE_STA&0X3F;
			length*=65536;					//���ʱ���ܺ�
			length+=TIM2CH1_CAPTURE_VAL;		//�õ��ܵĸߵ�ƽʱ��  us
			length=length/1000000*340/2;   //m
			TIM2CH1_CAPTURE_STA=0;			//������һ�β���
			if(length<10 && length>0.01){       //���˴���ֵ
				sum=length;
			}
 		}
//	while(i!=measuretime)
//	{
//		PCout(3)=1; 
//		delay_us(20); 
//		PCout(3)=0;
//		if(TIM2CH1_CAPTURE_STA&0X80)//�ɹ�������һ�θߵ�ƽ
//		{
//			length=TIM2CH1_CAPTURE_STA&0X3F;
//			length*=65536;					//���ʱ���ܺ�
//			length+=TIM2CH1_CAPTURE_VAL;		//�õ��ܵĸߵ�ƽʱ��  us
//			length=length/1000000*340/2*100;   //cm
//			TIM2CH1_CAPTURE_STA=0;			//������һ�β���
//			if(length<10000){       //���˴���ֵ
//			sum+=length;
//			i+=1;
//			}
// 		}
//	}

	return sum;
}


void TIM2_IRQHandler(void) 
{
	if((TIM2CH1_CAPTURE_STA&0X80)==0)//��δ�ɹ�����	
	{	  
		if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
		 
		{	    
			if(TIM2CH1_CAPTURE_STA&0X40)//�Ѿ����񵽸ߵ�ƽ��
			{
				if((TIM2CH1_CAPTURE_STA&0X3F)==0X3F)//�ߵ�ƽ̫����
				{
					TIM2CH1_CAPTURE_STA|=0X80;//��ǳɹ�������һ��
					TIM2CH1_CAPTURE_VAL=0XFFFF;
				}else TIM2CH1_CAPTURE_STA++;  //����
			}	 
		}
	if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)//����1���������¼�
		{	
			if(TIM2CH1_CAPTURE_STA&0X40)		//����һ���½��� 		
			{	  			
				TIM2CH1_CAPTURE_STA|=0X80;		//��ǳɹ�����һ��������
				TIM2CH1_CAPTURE_VAL=TIM_GetCapture1(TIM2);
		   		TIM_OC1PolarityConfig(TIM2,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���
			}else  								//��δ��ʼ,��һ�β���������
			{
				TIM2CH1_CAPTURE_STA=0;			//���
				TIM2CH1_CAPTURE_VAL=0;
	 			TIM_SetCounter(TIM2,0);
				TIM2CH1_CAPTURE_STA|=0X40;		//��ǲ�����������
		   		TIM_OC1PolarityConfig(TIM2,TIM_ICPolarity_Falling);		//CC1P=1 ����Ϊ�½��ز���
			}		    
		}			     	    					   
 	}
 
    TIM_ClearITPendingBit(TIM2, TIM_IT_CC1|TIM_IT_Update); //����жϱ�־λ
}
