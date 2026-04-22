#include "key.h"
#include "stm32f10x.h" 
#include "stm32f10x_tim.h"



Key_t key_arr[5];

volatile uint8_t TIM2_2000ms_Flag = 0;


// TIM2 初始化：10ms 中断
void TIM2_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // 开时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseStruct.TIM_Prescaler = 7199;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStruct.TIM_Period = 99;
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStruct);

    // 中断配置
    NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2; // 低优先级
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}

uint16_t tim2_cnt=0;
// 中断服务函数
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		
		key_serv();
		
		tim2_cnt++;
		if(tim2_cnt==200)
		{
			tim2_cnt=0;
			TIM2_2000ms_Flag=1;
		}
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

// 配置 PB15、PA8、PA11、PA12 为 上拉输入
void key_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_8;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;  
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_12;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
		
		TIM2_Init();
}


uint8_t  key_read()
{
	if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15) == 0) return 1;
	else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == 0) return 2;
	else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11) == 0) return 3;
	else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12) == 0) return 4;
	else return 0;
}


void key_serv()
{
	uint8_t key_sta=key_read();
	if(key_sta!=0)
	{
		key_arr[key_sta].age++;
		if(key_arr[key_sta].age==2) key_arr[key_sta].flag=1;
	}
	else
	{
		for(int i=0;i<5;i++) key_arr[i].age=0;
	}
}
