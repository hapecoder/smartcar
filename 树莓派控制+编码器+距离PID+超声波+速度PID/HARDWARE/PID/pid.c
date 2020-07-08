
#include "stdio.h"
#include "stdlib.h"
#include "pid.h"
#include "hall.h"
PID pid;
PID pid_s;

void PID_init(void){
		pid_s.Speed=0;
		pid_s.Speed_err=0.0;
		pid_s.Speed_err_last=0.0;
		pid_s.Speed_err_lastest=0.0;
    pid_s.pwm=1500.0;
		pid_s.Kp=0.5;
		pid_s.Ki=50;
		pid_s.Kd=0;
	
		pid.Speed=0.0;
		pid.Safe_distance=0.6;
		pid.Actual_distance=0;
		pid.err=0.0;
		pid.err_last=0.0;
		pid.err_lastest=0.0;
    pid.pwm=1500.0;
		pid.Kp=10*1.5*0.5;
		pid.Ki=1*0.5;
		pid.Kd=0;
	;
}


void PID_s_realize(float Speed){
		if(Speed>0.2 && pid_s.pwm>1520){pid_s.pwm=1570+Speed*5;}
	
		if(Speed<0){hall_speed=-hall_speed;}
	  pid_s.Speed=Speed;
	  pid_s.Speed_err=(pid_s.Speed-hall_speed);
    pid_s.pwm=pid_s.pwm+(pid_s.Kp*(pid_s.Speed_err-pid_s.Speed_err_last)+pid_s.Ki*pid_s.Speed_err+pid_s.Kd*(pid_s.Speed_err-2*pid_s.Speed_err_last+pid_s.Speed_err_lastest));  //车速误差对应的PWM的增量
	  pid_s.Speed_err_lastest=pid_s.Speed_err_last;
	  pid_s.Speed_err_last=pid_s.Speed_err;
    if(pid_s.pwm>1640){pid_s.pwm=1640;}  //pwm限制
    if(Speed>0 && pid_s.pwm<1560  && pid_s.pwm>1500 && Speed<0.2){pid_s.pwm=1560;}
		//if(Speed>0.2 && pid_s.pwm<1570  && pid_s.pwm>1560){pid_s.pwm=1570;}
		//if(Speed>0.3 && pid_s.pwm<1580  && pid_s.pwm>1570){pid_s.pwm=1580;}
	  if(pid_s.pwm<1300){pid_s.pwm=1300;}  //pwm限制
}




void PID_realize(float Actual_distance){
	  // pid.Safe_distance=0.6+hall_speed*0;
	  pid.Actual_distance=Actual_distance;
		pid.err=pid.Safe_distance-pid.Actual_distance;
		pid.Speed=pid.Speed-(pid.Kp*(pid.err-pid.err_last)+pid.Ki*pid.err+pid.Kd*(pid.err-2*pid.err_last+pid.err_lastest));
		pid.err_lastest=pid.err_last;
		pid.err_last=pid.err;
       if(pid.Actual_distance>=1){
           pid.Speed=1;
       }
       else if(pid.Speed<=0){
           pid.Speed=0;
       } 
       if(pid.Actual_distance<0.3){pid.Speed=0;}
  		 if(pid.Speed>2){pid.Speed=2;}
			 pid.pwm=pid.Speed*56.813+1541.1;         //输出PWM与速度的关系 1538.046358
       if(pid.Speed<0.1){pid.pwm=1500;}
			 
}



