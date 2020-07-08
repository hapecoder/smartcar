#include "motor.h"
#include "delay.h"

void Stop(void)
{
	TIM_SetCompare1(TIM3,1500);  //电机
	TIM_SetCompare2(TIM3,1500);  	//舵机

}

void Forward(void)
{
	TIM_SetCompare1(TIM3,1550);  //电机

}



void Turnleft(void)
{

	TIM_SetCompare2(TIM3,1700);

}


void Turnright(void)
{

	TIM_SetCompare2(TIM3,1300);

}


void Turnback(void)
{
	
	TIM_SetCompare1(TIM3,1300);
	

	
}


void Turnfront(void)   //
{
	TIM_SetCompare2(TIM3,1500);  	//舵机


}




