//  串口通信     USART1    

#include "BSP_USART1.h"

/* USART1 引脚初始化 */
void USART1_IOInit(void)
{
GPIO_InitTypeDef GPIO_InitStructure;
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //使能 GPIOA 时钟
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //TXD 引脚->PA9
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //设置为复用推挽输出
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //输出速率 50MHz
GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化 GPIOA
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //RXD 引脚->PA10
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //设置为浮空输入
GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化 GPIOA
}

/* USART1 中断优先级初始化 */
void NVIC_USART1Enable(void)
{
NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
//一般放在主程序中
NVIC_InitTypeDef NVIC_InitStructure;
NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn; //选择 USART1 中断
NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //设置抢占优先级
NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //设置响应优先级
NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能通道
NVIC_Init(&NVIC_InitStructure);
}

/* USART1 模块初始化函数, BAUDRATE-波特率 */
void USART1Init(uint32_t BAUDRATE)
{
USART_InitTypeDef USART_InitStructure;
USART1_IOInit(); //USART1 引脚初始化
RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); //使能 USART1 时钟
USART_InitStructure.USART_BaudRate = BAUDRATE; //设置波特率
USART_InitStructure.USART_WordLength = USART_WordLength_8b;//设置为 8 位数据位
USART_InitStructure.USART_StopBits = USART_StopBits_1; //设置为 1 位停止位
USART_InitStructure.USART_Parity = USART_Parity_No; //不使用奇偶校验
//禁用硬件流控制
USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//选择 USART 发送和接收模式
USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
USART_Init(USART1, &USART_InitStructure); //初始化 USART1
NVIC_USART1Enable(); //USART1 中断优先级初始化
USART_ITConfig(USART1,USART_IT_RXNE, ENABLE); //开启串口接受中断
USART_Cmd(USART1,ENABLE); //使能 USART1
}

void USART1_IRQHandler(void)
{
uint8_t dat;
if(USART_GetITStatus(USART1,USART_IT_RXNE) != 0) //检测接收中断标志位
{
dat = USART_ReceiveData(USART1); //读取接收到的数据
USART_SendData(USART1, dat); //接收到的数据发送出去
}
}

//重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{	
// while(!((USART1->SR) & 0x80));  //等待发送完成 
while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);	
USART_SendData(USART1, (uint8_t)ch); 
return (ch);
}

//重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{
//等待串口输入数据
// while(!((USART1->SR) & 0x20));
while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
return (int)USART_ReceiveData(USART1);
}

void Show_Message(uint32_t BAUDRATE)
{
 printf("使用USART参数为：%d 8-N-1 \n",BAUDRATE);
}



