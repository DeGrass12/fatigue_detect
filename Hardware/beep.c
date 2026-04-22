#include "beep.h"
#include "stm32f10x.h"   

void Beep_Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_14);
}

void Beep_ON()
{
	GPIO_SetBits(GPIOB,GPIO_Pin_14);
}
void Beep_OFF()
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_14);
}
