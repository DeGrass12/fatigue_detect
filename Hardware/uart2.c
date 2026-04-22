#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <cstdarg>
#include <string.h>
#include "stdlib.h"
#include "uart2.h"
#include "uart1.H"

void Uart2_Init()
{
	GPIO_InitTypeDef  GPIO_InitStrue;
	USART_InitTypeDef USART2_InitStrue;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	
	GPIO_InitStrue.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStrue.GPIO_Pin=GPIO_Pin_2;
	GPIO_InitStrue.GPIO_Speed=GPIO_Speed_10MHz;		
	GPIO_Init(GPIOA,&GPIO_InitStrue);
	
	GPIO_InitStrue.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_InitStrue.GPIO_Pin=GPIO_Pin_3;
	GPIO_Init(GPIOA,&GPIO_InitStrue);
	
	USART2_InitStrue.USART_BaudRate=38400;
	USART2_InitStrue.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART2_InitStrue.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	USART2_InitStrue.USART_Parity=USART_Parity_No;
	USART2_InitStrue.USART_StopBits=USART_StopBits_1;
	USART2_InitStrue.USART_WordLength=USART_WordLength_8b;
	
	USART_Init(USART2,&USART2_InitStrue);	
	
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);//接收中断
	USART_Cmd(USART2,ENABLE);	
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&NVIC_InitStructure);
	
}

void Uart2_SendByte(uint8_t Byte)
{
	USART_SendData(USART2,Byte);
	while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);
}

void Uart2_SendString(char *String)
{
	for(uint16_t i=0;String[i]!='\0';i++)
	{
		Uart2_SendByte(String[i]);
	}
}

void Uart2_Printf(char *format,...)
{
	char string[256];
	va_list arg;
	va_start(arg,format);
	vsprintf(string,format,arg);
	va_end(arg);
	Uart2_SendString(string);
}

//void USART2_IRQHandler(void)
//{
//	if(USART_GetFlagStatus(USART2,USART_FLAG_RXNE)==SET)
//	{
//		if(USART2->DR)
//		{                                 	
//			USART2_RxBuff[USART2_RxCounter]=USART_ReceiveData(USART2); 	
//			USART2_RxCounter++;                        
//		}		
//		USART_ClearITPendingBit(USART2,USART_FLAG_RXNE);
//	}
//	
//}


