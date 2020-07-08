


#include "sys.h"

#define TRIG_Send  PAout(6)
#define ECHO_Reci  PAin(7)


void CH_SR04_Init(void);
float Senor_Using(void);
void TIM2_Cap_Init(u16 arr,u16 psc);
