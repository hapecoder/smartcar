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
//	TIM_TimeBaseInitStructer.TIM_Period=999;  //1000us更新一次
//	TIM_TimeBaseInitStructer.TIM_Prescaler=71;  //72分频  1M
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

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	//使能TIM2时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //使能GPIOA时钟
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;  //PA0 清除之前设置  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0 输入  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_0);						 //PA0 下拉
	
	//初始化定时器2 TIM2	 
	TIM_TimeBaseStructure.TIM_Period = arr; //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//预分频器   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
  
	//初始化TIM2输入捕获参数
	TIM2_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	选择输入端 IC1映射到TI1上
  	TIM2_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  	TIM2_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  	TIM2_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  	TIM2_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  	TIM_ICInit(TIM2, &TIM2_ICInitStructure);
	
	//中断分组初始化
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 
	
	TIM_ITConfig(TIM2,TIM_IT_Update|TIM_IT_CC1,ENABLE);//允许更新中断 ,允许CC1IE捕获中断	
	
  TIM_Cmd(TIM2,ENABLE ); 	//使能定时器2
 
}


u8  TIM2CH1_CAPTURE_STA=0;	//输入捕获状态		    				
u16	TIM2CH1_CAPTURE_VAL;	//输入捕获值


float Senor_Using(void)				//返回距离值
{
	float length=0,sum=0;
	int q=0;
		PCout(3)=1; 
		delay_us(20); 
		PCout(3)=0;
		if(TIM2CH1_CAPTURE_STA&0X80)//成功捕获到了一次高电平
		{
			length=TIM2CH1_CAPTURE_STA&0X3F;
			length*=65536;					//溢出时间总和
			length+=TIM2CH1_CAPTURE_VAL;		//得到总的高电平时间  us
			length=length/1000000*340/2;   //m
			TIM2CH1_CAPTURE_STA=0;			//开启下一次捕获
			if(length<10 && length>0.01){       //过滤错误值
				sum=length;
			}
 		}
//	while(i!=measuretime)
//	{
//		PCout(3)=1; 
//		delay_us(20); 
//		PCout(3)=0;
//		if(TIM2CH1_CAPTURE_STA&0X80)//成功捕获到了一次高电平
//		{
//			length=TIM2CH1_CAPTURE_STA&0X3F;
//			length*=65536;					//溢出时间总和
//			length+=TIM2CH1_CAPTURE_VAL;		//得到总的高电平时间  us
//			length=length/1000000*340/2*100;   //cm
//			TIM2CH1_CAPTURE_STA=0;			//开启下一次捕获
//			if(length<10000){       //过滤错误值
//			sum+=length;
//			i+=1;
//			}
// 		}
//	}

	return sum;
}


void TIM2_IRQHandler(void) 
{
	if((TIM2CH1_CAPTURE_STA&0X80)==0)//还未成功捕获	
	{	  
		if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
		 
		{	    
			if(TIM2CH1_CAPTURE_STA&0X40)//已经捕获到高电平了
			{
				if((TIM2CH1_CAPTURE_STA&0X3F)==0X3F)//高电平太长了
				{
					TIM2CH1_CAPTURE_STA|=0X80;//标记成功捕获了一次
					TIM2CH1_CAPTURE_VAL=0XFFFF;
				}else TIM2CH1_CAPTURE_STA++;  //计数
			}	 
		}
	if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)//捕获1发生捕获事件
		{	
			if(TIM2CH1_CAPTURE_STA&0X40)		//捕获到一个下降沿 		
			{	  			
				TIM2CH1_CAPTURE_STA|=0X80;		//标记成功捕获到一次上升沿
				TIM2CH1_CAPTURE_VAL=TIM_GetCapture1(TIM2);
		   		TIM_OC1PolarityConfig(TIM2,TIM_ICPolarity_Rising); //CC1P=0 设置为上升沿捕获
			}else  								//还未开始,第一次捕获上升沿
			{
				TIM2CH1_CAPTURE_STA=0;			//清空
				TIM2CH1_CAPTURE_VAL=0;
	 			TIM_SetCounter(TIM2,0);
				TIM2CH1_CAPTURE_STA|=0X40;		//标记捕获到了上升沿
		   		TIM_OC1PolarityConfig(TIM2,TIM_ICPolarity_Falling);		//CC1P=1 设置为下降沿捕获
			}		    
		}			     	    					   
 	}
 
    TIM_ClearITPendingBit(TIM2, TIM_IT_CC1|TIM_IT_Update); //清除中断标志位
}
