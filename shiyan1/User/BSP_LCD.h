#ifndef __BSP_LCD_H
#define __BSP_LCD_H

#include "stm32f10x.h"

extern uint8_t LCD_SCAN_MODE;
extern uint16_t LCD_X_LENGTH, LCD_Y_LENGTH; 

#define      FSMC_CMD_ADDR        ( ( uint32_t ) 0x60000000 )
#define      FSMC_DATA_ADDR        ( ( uint32_t ) 0x60020000 )

// 定义显示屏常用颜色
#define      BACKGROUND     	 BLACK   //默认背景颜色
#define      WHITE	   	 0xFFFF	   //白色
#define      BLACK                     0x0000	   //黑色 
#define      GREY                       0x8430	   //灰色 
#define      BLUE                        0x001F	   //蓝色 
#define      RED                         0xF800	   //红色 
#define      MAGENTA               0xF81F	   //品红 
#define      GREEN                     0x07E0	   //绿色 
#define      CYAN                       0x7FFF	   //青色 
#define      YELLOW                   0xFFE0	   //黄色 

void LCD_GPIO_Init(void);
void LCD_FSMC_Init_REGISTER ( void );
void LCD_FSMC_Init ( void );
void LCD_Write_Cmd ( uint16_t Cmd );
uint16_t LCD_Read_Data ( void );
static void LCD_Delay ( __IO uint32_t nCount );
void LCD_ILI9341_Init ( void );
void LCD_RESET ( void );
void LCD_ScanDir ( uint8_t ucOption );
void LCD_Init ( void );
void LcdSetWindow ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight );
void LCD_DrawPoint ( uint16_t usX, uint16_t usY, uint16_t usColor );
void LcdFill ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight,  uint16_t usColor);
void LcdDispEnchar1608(uint16_t X, uint16_t Y,uint8_t Enchar,uint16_t TextColor, uint16_t BackColor);
void LcdDispEnchar2416(uint16_t X, uint16_t Y,uint8_t Enchar,uint16_t TextColor, uint16_t BackColor);
void LcdDispEnchar3224(uint16_t X, uint16_t Y,uint8_t Enchar,uint16_t TextColor, uint16_t BackColor);
// MODE: 1608,2416,3224,前两位对应字高，后两位对应字宽
void LcdDispEncharString(uint16_t X, uint16_t Y,char* pstr,uint16_t MODE,uint16_t TextColor, uint16_t BackColor);
void LcdDispHanzi1616(uint16_t X, uint16_t Y, uint16_t RelativePosition, uint16_t TextColor, uint16_t BackColor);
void LcdDispHanziString(uint16_t X, uint16_t Y, uint8_t number, uint16_t RelativePosition, uint16_t TextColor, uint16_t BackColor);
void LcdDrawRectangle(uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight, uint16_t usColor);
#endif
