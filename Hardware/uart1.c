#include "stm32f10x.h"                  
#include <stdio.h>
#include <cstdarg>
#include <string.h>
#include "uart1.h"
void Uart1_Init()
{
	GPIO_InitTypeDef  GPIO_InitStrue;
	USART_InitTypeDef USART1_InitStrue;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	GPIO_InitStrue.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStrue.GPIO_Pin=GPIO_Pin_9;
	GPIO_InitStrue.GPIO_Speed=GPIO_Speed_10MHz;		
	GPIO_Init(GPIOA,&GPIO_InitStrue);
	
	GPIO_InitStrue.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_InitStrue.GPIO_Pin=GPIO_Pin_10;//PA10		
	GPIO_Init(GPIOA,&GPIO_InitStrue);
	
	USART1_InitStrue.USART_BaudRate=9600;
	USART1_InitStrue.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART1_InitStrue.USART_Mode=USART_Mode_Tx;
	USART1_InitStrue.USART_Parity=USART_Parity_No;
	USART1_InitStrue.USART_StopBits=USART_StopBits_1;
	USART1_InitStrue.USART_WordLength=USART_WordLength_8b;
	
	USART_Init(USART1,&USART1_InitStrue);	
	
	//USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);//接收中断
	USART_Cmd(USART1,ENABLE);	
}

void Uart1_SendByte(uint8_t Byte)
{
	USART_SendData(USART1,Byte);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
}

void Uart1_SendString(char *String)
{
	for(uint16_t i=0;String[i]!='\0';i++)
	{
		Uart1_SendByte(String[i]);
	}
}

void Uart1_Printf(char *format,...)
{
	char string[256];
	va_list arg;
	va_start(arg,format);
	vsprintf(string,format,arg);
	va_end(arg);
	Uart1_SendString(string);
}

