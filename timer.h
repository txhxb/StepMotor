#ifndef __TIMER_H
#define	__TIMER_H
#include "stm32f10x.h"

void GPIO_Config(void);
void Pulse_output(u32 Cycle,u32 PulseNum);
//void PWM_Output_A(u16 Cycle_A, u32 PulseNum_A, u16 DIR_A);
//void TIM2_PWM_Init(void);

#endif
