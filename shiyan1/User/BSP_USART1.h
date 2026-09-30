//  串口通信     USART1    

#ifndef __BSP_USART1_H
#define __BSP_USART1_H

#include <stdio.h>     
// 使用printf、scanf、getchar等函数
//  Keil 软件C/C++中勾选"Use MicroLIB"

#include "stm32f10x.h"

void USART1_IOInit(void);
void NVIC_USART1Enable(void);
void USART1Init(uint32_t BAUDRATE);
void USART1_IRQHandler(void);
int fputc(int ch, FILE *f);
int fgetc(FILE *f);
void Show_Message(uint32_t BAUDRATE);

#endif


