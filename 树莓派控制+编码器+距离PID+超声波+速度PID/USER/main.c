#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "oled.h"
#include "pwm.h"
#include "motor.h"
#include "stdlib.h"
#include "string.h"
#include "hall.h"
#include "us.h"
#include "pid.h"
//ALIENTEK Mini STM32开发板范例代码3
//串口实验   
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司

extern float distance;

 int main(void)
 {	
	float speed_set=0;
	int angle_set;
	float speed_last=0;
	float k=0;
	u8 acc_time=0;
  u8 r_len=0;
	u8 len=0;
	u8 mode=1; //1循迹   2ACC 3遥控 
	u16 time;
	u8 Pid_flag=0; 
	delay_init();	    	 //延时函数初始化	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
	uart_init(9600);	 //串口初始化为9600
	LED_Init();		  	 //初始化与LED连接的硬件接口 
	CH_SR04_Init();
	TIM2_Cap_Init(0XFFFF,72-1);
	TIM3_PWM_Init(20000-1,72-1); 
  TIM_SetCompare1(TIM3,1500);
	TIM_SetCompare2(TIM3,1500);
	TIM4_Cap_Init();		
	TIM5_Int_Init(49999,71); //以10khz的频率计数  1us*50000=50ms更新一次
	PID_init(); 
	 
	while(1)
	{	time++;
		distance= Senor_Using();
		if(distance==0 || (distance-k)>3){distance=k;}
		else k=distance;
		if(time>10000){        //10*50ms输出一次
			printf("r%0.2f %0.2f",hall_speed,distance);	 // m/s  
			printf("\r\n");
		//	while((USART1->SR&0X40)==0);//等待发送结束      
 			time=0;
			//TIM_Cmd(TIM5, DISABLE); 	
		}
		
	if(mode==1||mode==3){
		if(USART_RX_STA&0x8000)

		{ len=USART_RX_STA&0x3fff;
			Pid_flag=0;
			
			if(len<2){
				if((USART_RX_BUF[0]-48)>0 &&(USART_RX_BUF[0]-48)<4){
				mode=USART_RX_BUF[0]-48;
				printf("mode:%d",mode);
				printf("\r\n");//插入换行
				//TIM_SetCompare1(TIM3,1500);	

			}}
			else {
				if(USART_RX_BUF[0]=='t'){
					r_len = strlen(speed);memcpy(speed,speed+1,r_len);
				
					speed_set=(float)atof(speed);
					angle_set=(int)atof(angle);
				//	printf("s:%0.2f  ",speed_set);
				//	printf("a:%d  ",angle_set);
					
					if(speed_set<1700 && speed_set>1300){
					
					 TIM_SetCompare1(TIM3,(int)speed_set);}
					
					if(speed_set==0 && speed_last==0){TIM_SetCompare1(TIM3,1500);} 
					if(abs(speed_set)<5 && speed_set!=0){ Pid_flag=1;}
					if(speed_set<0){pid_s.pwm=1440;}
					else 	{pid_s.pwm=1550;}
					
					speed_last=speed_set;
					if(angle_set>500  && angle_set<2500){

					 TIM_SetCompare2(TIM3,angle_set);
					}
					
				//printf("\r\n");//插入换行
				//	while((USART1->SR&0X40)==0);//等待发送结束

					delay_ms(10);
					time=time+2000;
					
		  }}
			USART_RX_STA=0;
			memset(USART_RX_BUF,0,100);
		}
		
		
		if(Pid_flag==1){
	
				PID_s_realize(speed_set);
				if(pid_s.pwm<1520 && pid_s.pwm>1460){
							TIM_SetCompare1(TIM3,1500);
				}
				else if (pid_s.pwm>1600){TIM_SetCompare1(TIM3,1600);}
				else if (pid_s.pwm<1400){TIM_SetCompare1(TIM3,1400);}
				else TIM_SetCompare1(TIM3,pid_s.pwm);
				
			//	if(abs(pid_s.err)<0.01){Pid_flag=0;}
//				printf("speed_set:%.2f\r\n",speed_set);
			//	printf("pid_s.pwm:%.2f",pid_s.pwm);
			//	printf("\r\n");//插入换行
				delay_ms(20);
				time=time+500;
			}


		}


	if(mode==2){	
		if(acc_time==2){TIM_SetCompare2(TIM3,1450);acc_time=0;}
		
			if(USART_RX_STA&0x8000)
		{	len=USART_RX_STA&0x3fff;
			if(len<2){
				if((USART_RX_BUF[0]-48)>0 &&(USART_RX_BUF[0]-48)<4){
					mode=USART_RX_BUF[0]-48;
				//	printf("mode:%d ",mode);
				//	TIM_SetCompare1(TIM3,1500);
					
					
			}}
			else {
				angle_set=(int)atof(angle);
				if(angle_set>500  && angle_set<2500){
				 TIM_SetCompare2(TIM3,angle_set);
				}
			}
			
			USART_RX_STA=0;
			memset(USART_RX_BUF,0,100);
		}	
			delay_ms(50);
			//pid.Speed=hall_speed;
			PID_realize(distance);
			//PID_s_realize(pid.Speed);
			TIM_SetCompare1(TIM3,pid.pwm);
//			printf("\r\ndistance:%.2f\r\n",distance);	
//			printf("\r\npid.pwm:%.2f\r\n",pid.pwm);
//			printf("\r\npid.speed:%.2f\r\n",pid.Speed);
			time=time+500;
			acc_time++;
	}

	}	 
}


