//  TIM3_CH4对应PB1， PWM
#include "stm32f10x.h"
void TIM3_Initialization(uint16_t arr,uint16_t psc,uint16_t vplus);
void GPIOB_Initialization(void);	
// void TIM3_IRQHandler(void);

int main(void)
{
GPIOB_Initialization();
TIM3_Initialization(9,71,5);

while(1);
}

void GPIOB_Initialization(void)
{
GPIO_InitTypeDef GPIO_InitStructure;
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);	
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;	  
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
GPIO_Init(GPIOB, &GPIO_InitStructure);
}

//PWM定时比较输出初始化函数实现
void TIM3_Initialization(uint16_t arr,uint16_t psc,uint16_t vplus)
{
RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	

/*
// 设置中断
NVIC_InitTypeDef NVIC_InitStructure;  
NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
//放在主程序main.c中比较好
NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 
NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
NVIC_Init(&NVIC_InitStructure);
*/
	
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStruct;
//复位TIM3 到初始状态
TIM_DeInit(TIM3);
//定时器TIM3初始化
//设置在下一个更新事件发生时装入活动的自动重装载寄存器的周期值
TIM_TimeBaseStruct.TIM_Period = arr;
//设置CNT计数器的计数时钟预分频值
TIM_TimeBaseStruct.TIM_Prescaler = psc;
//设置TIM计数模式：向上计数模式
TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
//设置输入滤波单元的采样时钟频率，时钟分割:TDTS = Tck_tim
//主要作用于滤波通道上，对应定时作用的Timer，无意义
TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
//重复计数次数
TIM_TimeBaseStruct.TIM_RepetitionCounter = 0;
//根据指定的参数初始化TIMx的时间基数单位
TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStruct);

TIM_OCInitTypeDef   TIM_OCInitStruct;
//初始化TIM3比较输出模式：PWM1
TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
//输出极性:TIM输出比较极性高
TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
//比较输出使能
 TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
//设置待装入捕获比较寄存器的脉冲值，初始化通道4
TIM_OCInitStruct.TIM_Pulse = vplus; 
TIM_OC4Init(TIM3,&TIM_OCInitStruct);
	
//使能TIM3
TIM_Cmd(TIM3,ENABLE);
}

/*
void TIM3_IRQHandler(void)
{    
; 
}
*/




