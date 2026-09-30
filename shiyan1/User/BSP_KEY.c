#include "BSP_KEY.h"

void BSP_KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // ?? GPIOA ? GPIOC ??
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);
    
    // ?? KEY1 (PA0) ?????
    GPIO_InitStructure.GPIO_Pin = KEY1_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY1_PORT, &GPIO_InitStructure);
    
    // ?? KEY2 (PC13) ?????
    GPIO_InitStructure.GPIO_Pin = KEY2_PIN;
    GPIO_Init(KEY2_PORT, &GPIO_InitStructure);
}