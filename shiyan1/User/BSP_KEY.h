#ifndef __BSP_KEY_H
#define __BSP_KEY_H
#include "stm32f10x.h"

#define KEY1_PIN    GPIO_Pin_0
#define KEY1_PORT   GPIOA
#define KEY2_PIN    GPIO_Pin_13
#define KEY2_PORT   GPIOC

#define KEY1        GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN)
#define KEY2        GPIO_ReadInputDataBit(KEY2_PORT, KEY2_PIN)

void BSP_KEY_Init(void);
#endif