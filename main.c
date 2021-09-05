#include "stm32f10x.h"
#include "sys.h"
#include "delay.h" 
#include "timer.h"
#include "driver.h"
#include "key.h"

static int state=0;

/**
	PA11——PUL+——脉冲信号输入
	PA12——DIR+——步进电机转向
**/

int main()
{
	u8 speed = 5;			//转速设置
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//设置NVIC中断分组2:2位抢占优先级，2位相应优先级
	GPIO_Config();		//IO口初始化
  delay_init();			//延时函数初始化	
	KEY_Init();				//按钮初始化
	Driver_init(true,32);		//步进电机初始化 true 正转 false 反转
//	Pulse_output(16,6400*10+1);
	
//	Driver_start(5);
//	delay_ms(100);
//	Driver_change(10);
//	Driver_keep();
	
//	delay_ms(100);
//	Driver_keep();
	while(1)
	{		
		if(KEY_Scan(1)==KEY0_PRES)
			state=~state;
		if(state==(-1))
		{
			Driver_start(speed);
			while(1)
			{
				if(KEY_Scan(1)==KEY0_PRES)
					state=~state;
				if(state==0){
					Motor_stop();
					break;
				}
				Driver_keep();
			}
		}
		if(state==0)
			Motor_stop();
	}
}
