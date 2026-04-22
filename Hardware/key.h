#ifndef __KEY_H_
#define __KEY_H_

#include "stm32f10x.h"
extern volatile uint8_t TIM2_2000ms_Flag;
typedef struct
{
	uint16_t age;
	uint8_t flag;
}Key_t;
extern Key_t key_arr[5];
void key_serv(void);
void key_init(void);
#endif