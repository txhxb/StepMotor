#include "timer.h"

/**
	PA11——PUL+——脉冲信号输入
	PA12——DIR+——步进电机转向
**/

void GPIO_Config(void)//IO口初始化
{ 
	GPIO_InitTypeDef GPIO_InitStructure; 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //使能GPIO外设时钟 									 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;        
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       
	GPIO_Init(GPIOA, &GPIO_InitStructure);                  
}

/***********************TIM1初始化函数*************************/
/****参数：****************************************************/
/******u32 Cycle用于设定计数频率（计算公式：Cycle=1Mhz/目标频率）**/
/****返回值：**************************************************/
/******无*****************************************************/
void TIM1_config(u32 Cycle)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_TIM1 , ENABLE); //时钟使能
 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;                   //TIM1_CH4 PA11
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;             //复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
 
    TIM_TimeBaseStructure.TIM_Period = Cycle-1;                 //使用Cycle来控制频率（f=72/(71+1)/Cycle）  当Cycle为100时脉冲频率为10KHZ                           
    TIM_TimeBaseStructure.TIM_Prescaler =71;                    //设置用来作为TIMx时钟频率除数的预分频值                                                     
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //设置时钟分割：TDTS= Tck_tim            
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM向上计数模式
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;            //重复计数，一定要=0！！！（高级定时器特有）
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);                                       
 
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;          		//选择定时器模式：TIM脉冲宽度调制模式1       
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 	//比较输出使能
    TIM_OCInitStructure.TIM_Pulse = Cycle/2-1;                    	//设置待装入捕获寄存器的脉冲值（占空比：默认50%，这可也可以调节如果需要的话将它作为一个参数传入即可）                                   
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;      	//输出极性       
 
    TIM_OC4Init(TIM1, &TIM_OCInitStructure);        				//使能通道4                                                 
 
    TIM_SelectMasterSlaveMode(TIM1, TIM_MasterSlaveMode_Enable);	//设置为主从模式
    TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update);			//选择定时器1的触发方式（使用更新事件作为触发输出）
    
 
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);               //使能通道4预装载寄存器               
    TIM_ARRPreloadConfig(TIM1, ENABLE);                             //使能TIM1在ARR上的预装载寄存器       
}

/***********************TIM2初始化函数*************************/
/****参数：****************************************************/
/******u32 PulseNum用于设定脉冲数量****************************/
/****返回值：*************************************************/
/******无*****************************************************/
void TIM2_config(u32 PulseNum)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure; 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);		//使能定时器2的时钟
 
    TIM_TimeBaseStructure.TIM_Period = PulseNum-1;   			//脉冲数
    TIM_TimeBaseStructure.TIM_Prescaler =0;    
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);  
 
    TIM_SelectInputTrigger(TIM2, TIM_TS_ITR0);					//选择定时器2的输入触发源（内部触发（TIM1））
 
    TIM2->SMCR|=0x07;                                  			//设置从模式寄存器（SMS[2:0]:111 外部时钟模式1） 
 
    TIM_ITConfig(TIM2,TIM_IT_Update,DISABLE);					//更新中断失能
 
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;        
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;     
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);								//定时器2中断初始化
}

/************************脉冲输出函数**************************/
/****参数：****************************************************/
/******u32 Cycle用于设定计数频率(计算公式：Cycle=1Mhz/目标频率)**/
/******u32 PulseNum用于设定输出脉冲的数量（单位：个）************/
/****返回值：**************************************************/
/******无*****************************************************/
void Pulse_output(u32 Cycle,u32 PulseNum)
{
    TIM2_config(PulseNum);						//设置脉冲数量
    TIM_Cmd(TIM2, ENABLE);						//使能TIM2（从定时器）
    TIM_ClearITPendingBit(TIM2,TIM_IT_Update);	//清除中断标志位
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);	//使能更新中断
    TIM1_config(Cycle );							//使能定时器1（主定时器）
    
    TIM_Cmd(TIM1, ENABLE);						//使能定时器1
    TIM_CtrlPWMOutputs(TIM1, ENABLE);   		//高级定时器一定要加上，主输出使能
} 
 
/********************定时器2的中断服务函数**********************/
/****参数：****************************************************/
/******u32 PulseNum用于设定脉冲数量****************************/
/****返回值：*************************************************/
/******无*****************************************************/
/****函数说明:************************************************/
/*当TIM的CNT寄存器的值到达设定的Update值会触发更新中断，此时设定的脉冲数已输出完毕，关闭TIM1和TIM2*/
void TIM2_IRQHandler(void) 
{ 
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) 	//TIM_IT_Update
    { 
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update); 	// 清除中断标志位 
        TIM_CtrlPWMOutputs(TIM1, DISABLE);  			//主输出使能
        TIM_Cmd(TIM1, DISABLE); 						//关闭定时器 
        TIM_Cmd(TIM2, DISABLE); 						//关闭定时器 
        TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE); 	//关闭TIM2更新中断
    } 
} 

/*********** Group A ***********/
//// 定时器1主模式
//void TIM1_GPIO_Config(uint16_t TIM1_Prescaler, uint16_t TIM1_Period, uint16_t CCR_A, uint16_t DIR_A)
//{
//	GPIO_InitTypeDef	GPIO_InitStructure;
//	TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;
//	TIM_OCInitTypeDef		TIM_OCInitStructure;
//	
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_TIM1, ENABLE);
//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_11;		// TIM1_CH1 - PA8， CH4 - PA11
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			// 复用推挽输出
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
//	
//	
//	TIM_TimeBaseStructure.TIM_Period = TIM1_Period - 1;
//	TIM_TimeBaseStructure.TIM_Prescaler = TIM1_Prescaler - 1;
//	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;			// 重复计数，一定要 = 0; 高级定时器TIM1,TIM8，这句必须有。
//	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
//	
//	
//	// 设置工作模式
//	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;							// PWM1
//	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	// 比较输出使能
//	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;			// 输出极性
//	// PWM通道，TIM1 - 通道1设置函数，50/100，脉冲信号
//	TIM_OCInitStructure.TIM_Pulse = CCR_A;								// 设置待装入捕获寄存器的脉冲值
//	TIM_OC1Init( TIM1, &TIM_OCInitStructure);
//	TIM_SelectMasterSlaveMode( TIM1, TIM_MasterSlaveMode_Enable);		// 主从模式下，作为主定时器使能
//	TIM_SelectOutputTrigger( TIM1, TIM_TRGOSource_Update);		// TIM - EGR寄存器， 定义UG位，产生更新事件
//	TIM_OC1PreloadConfig( TIM1, TIM_OCPreload_Enable);
//	// PWM通道，TIM1 - 通道4设置函数，100/100 or 0/100，方向信号
//	TIM_OCInitStructure.TIM_Pulse = DIR_A;							// 初始化 TIM1-OC4
//	TIM_OC4Init( TIM1, &TIM_OCInitStructure);						// CH4预装载使能，修改
//	TIM_OC4PreloadConfig( TIM1, TIM_OCPreload_Enable);
//	
//	TIM_ARRPreloadConfig(TIM1, ENABLE);				// 预装载使能	
//}

////定时器2从模式
//void TIM2_GPIO_Config(u32 PulseNum_A)
//{
//	TIM_TimeBaseInitTypeDef	TIM_TimeBaseStructure;
//	NVIC_InitTypeDef	NVIC_InitStructure;
//	
//	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2, ENABLE);
//	
//	TIM_TimeBaseStructure.TIM_Period = PulseNum_A;
//	TIM_TimeBaseStructure.TIM_Prescaler = 0;
//	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;			// 向上计数
//	TIM_TimeBaseInit( TIM2, &TIM_TimeBaseStructure);
//	
//	TIM_SelectInputTrigger( TIM2, TIM_TS_ITR0);			// TIM1-主，TIM2-从，表中对应 ITR0
//	TIM_SelectSlaveMode( TIM2, TIM_SlaveMode_External1);		//主从模式下作为从定时器使能
//	TIM_ITConfig( TIM2, TIM_IT_Update, DISABLE);
//	
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
//	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init( &NVIC_InitStructure);
//}

//// Cycle_A = Preiod_A; PulseNum_A 输出脉冲个数; DIR_A 高/低电平-方向信号
//void PWM_Output_A(u16 Cycle_A, u32 PulseNum_A, u16 DIR_A)			// TIM1-主，TIM2-从，输出PWM
//{
//	TIM2_GPIO_Config(PulseNum_A);
//	TIM_Cmd( TIM2, ENABLE);
//	TIM_ClearITPendingBit( TIM2, TIM_IT_Update);
//	TIM_ITConfig( TIM2, TIM_IT_Update, ENABLE);
//	TIM1_GPIO_Config( 72, Cycle_A, Cycle_A / 2, DIR_A);		// 72M / 72 = 1MHz;
//	TIM_Cmd( TIM1, ENABLE);
//	TIM_CtrlPWMOutputs( TIM1, ENABLE);		// 高级定时器必须有，使能其输出
//}

//void TIM2_PWM_Init(void)
//{
//	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; 
//	TIM_OCInitTypeDef TIM_OCInitStructure; 
//	//初始化TIM2
//	TIM_TimeBaseStructure.TIM_Period = 7199;                 //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
//	TIM_TimeBaseStructure.TIM_Prescaler =0;                   //设置用来作为TIMx时钟频率除数的预分频值 不分频
//	TIM_TimeBaseStructure.TIM_ClockDivision = 0;                //设置时钟分割:TDTS = Tck_tim
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM向上计数模式
//	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);             //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
//	
//	//初始化TIM2 Channel1、Channel2 PWM模式	 
//	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;           //选择定时器模式:TIM脉冲宽度调制模式2
// 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
//	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;   //输出极性:TIM输出比较极性高
//	TIM_OC1Init(TIM2, &TIM_OCInitStructure);                    //根据T指定的参数初始化外设TIM2 OC1
//	TIM_OC2Init(TIM2, &TIM_OCInitStructure);                    //根据T指定的参数初始化外设TIM2 OC2
// 
//	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);  //使能TIM2在CCR2上的预装载寄存器
//	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);  //使能TIM2在CCR2上的预装载寄存器
// 
//	TIM_Cmd(TIM2, ENABLE);  //使能TIM2
//}

