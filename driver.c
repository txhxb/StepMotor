#include "driver.h"
#include "timer.h"
#include "delay.h"
#include "key.h"
#include "rate.h"
	
static u16 num;
static u8  spd,rate;
static u8 s[15];

void Driver_init(bool flag,u8 sub)     //设置步进电机的转向和细分数
{
	int i,n;
	if(flag)
		GPIO_ResetBits(GPIOA, GPIO_Pin_12);//A12置0  初始化电机转动方向为正  A12——DIR+
	else
		GPIO_SetBits(GPIOA,GPIO_Pin_12);
	switch(sub){												 //装载步进电机每转所需的脉冲数
		case 32: num = 6400,n=4;	break;
		case 16: num = 3200,n=3;	break;
		case 8:  num = 1600,n=2;	break;
		case 4:  num = 800, n=1; 	break;
		case 2:  num = 400, n=0; 	break;
	}
	for(i=0;i<=14;i++)                  //转载频率数组
		s[i]=arry32[n][i];
}

void Driver_start(u8 speed)//speed设置电机转速(1r/s,需小于15r/s)
{
	int i = 1000;
	spd = speed;
	rate=s[spd-1];
	for(;i>=rate;i--)
	{
		Pulse_output(i,100);//脉冲数需要填满对应频率的1ms
		delay_ms(1);
	}
}

void Driver_keep(void)//步进电机转速保持 
{
	Pulse_output(rate,num*spd+1);
	delay_ms(1000);//等待脉冲发送完毕
}

void Driver_change(u8 _spd)//步进电机转速变更
{
	int i=rate,j=s[_spd-1];
	for(;i<j;i++)
	{
		Pulse_output(i,100);
		delay_ms(1);
	}
	for(;i>j;i--)
	{
		Pulse_output(i,100);
		delay_ms(1);
	}
	rate=i;
}

void Driver_angle(u8 speed,u8 angle)//控制步进电机转动角度
{
	rate = s[speed-1];
	Pulse_output(rate,num*(angle/360));
}

void Motor_stop(void)//步进电机停止
{
	TIM_Cmd(TIM1, DISABLE); 
}


