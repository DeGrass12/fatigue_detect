#include <stdlib.h>
#include <stdio.h>
#include "led.h"
#include "delay.h"
#include "sys.h"	 
#include "OLED.h"
#include "string.h" 	
#include "uart1.h"
#include "gps.h"
#include "uart3.h"
#include "beep.h"
#include "key.h"
#include "timer.h"





extern float g_longitude;
extern float g_latitude;

char dis_buf[20];
char Longitude[20];
char Latitude[20];
uint8_t fatigue_threshold = 3;
uint8_t alarm_enable=1;
char json_buf[256] = {0};  // JSON专用大缓冲区
// 报警控制
bool buzzer_long_alarm = false;  // 超限报警（需按键解除）
bool led_state         = false;
uint16_t led_tick      = 0;      // 定时器闪烁计数

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	TIM3_Init();
	Uart1_Init(); //用于mqtt
	key_init();
	Beep_Init();
	Beep_OFF();
	LED_Init();
	LED_OFF();
	OLED_Init();					//OLED初始化
	delay_ms(50);
	OLED_Clear();						//清屏
	Uart3_Init();
	GPS_Init();
	OLED_ShowChinese(1,1,0);
	OLED_ShowChinese(1,2,2);
	OLED_ShowString(1,5,":");
	
	OLED_ShowChinese(2,1,1);
	OLED_ShowChinese(2,2,2);
	OLED_ShowString(2,5,":");
	
	
	OLED_ShowChinese(3,1,13);
	OLED_ShowChinese(3,2,14);
	OLED_ShowString(3,5,":");
	
	OLED_ShowChinese(4,1,5);
	OLED_ShowChinese(4,2,6);
	OLED_ShowString(4,5,":");
	
	OLED_ShowChinese(4,1,15);
	OLED_ShowChinese(4,2,16);
	OLED_ShowChinese(4,3,17);
	OLED_ShowChinese(4,4,18);
	OLED_ShowString(4,9,":");
	
	
	
	while(1)
	{
		sprintf(dis_buf,"%d/%d ",fatigue_num,fatigue_threshold);
		OLED_ShowString(3,12,dis_buf);

		if(key_arr[1].flag==1)
		{
			key_arr[1].flag=0;
			fatigue_threshold++;
		}
		
		if(key_arr[2].flag==1)
		{
			key_arr[2].flag=0;
			if(fatigue_threshold>2)fatigue_threshold--;
		}
		
		if(key_arr[3].flag==1)
		{
			key_arr[3].flag=0;
			buzzer_long_alarm = false;
      fatigue_num = 0;

			TIM_Cmd(TIM3, DISABLE);
			led_tick = 0;
			led_state = false;
			LED_OFF();
			Beep_OFF();
		}
		
		if(key_arr[4].flag==1)
		{
			key_arr[4].flag=0;
			alarm_enable = !alarm_enable;

			// 关闭功能时，同时退出当前长报警
			if(alarm_enable == false)
			{
				buzzer_long_alarm = false;
				TIM_Cmd(TIM3, DISABLE);
				led_tick = 0;
				led_state = false;
				LED_OFF();
				Beep_OFF();
			}
		}
		
		// ====================== 【使能控制】只有开启才允许进入长报警 ======================
    if(alarm_enable == true)
    {
			// 进入长报警
			if(fatigue_num >= fatigue_threshold && !buzzer_long_alarm)
			{
				buzzer_long_alarm = true;
				Beep_ON();

				TIM_SetCounter(TIM3, 0);
				TIM_Cmd(TIM3, ENABLE);
			}

			// 自动退出长报警
			if(fatigue_num < fatigue_threshold && buzzer_long_alarm)
			{
				buzzer_long_alarm = false;

				TIM_Cmd(TIM3, DISABLE);
				led_tick = 0;
				led_state = false;
				LED_OFF();
				Beep_OFF();
			}
    }
		
		if(!buzzer_long_alarm)
		{
			if(fatigue_flag)
				Beep_ON();
			else
				Beep_OFF();
    }
		
		
		if(fatigue_flag==true)
		{
			OLED_ShowChinese(3,4,3);
			OLED_ShowChinese(3,5,4);
		}
		else
		{
			OLED_ShowChinese(3,4,11);
			OLED_ShowChinese(3,5,12);
		}
		
		if(alarm_enable==1)
		{
			OLED_ShowChinese(4,6,7);
			OLED_ShowChinese(4,7,8);
		}
		else
		{
			OLED_ShowChinese(4,6,9);
			OLED_ShowChinese(4,7,10);
		}


		Uart_Rece_Pares();   // 串口接收内容解析
		GpsInfor_OutPut();   // 输出GPS信息
//		//Uart1_Printf("{\"method\" : \"thing.event.property.post\", \"params\" : {\"Longitude\" : %f, \"Latitude\":%f}, \"version\" : \"1.0.0\"}\r\n",g_longitude,g_latitude);
//		if(fatigue_flag==true)
//		{
//			Uart1_Printf("{\"method\" : \"thing.event.property.post\", \"params\" : {\"fatigue_status\":%d}, \"version\" : \"1.0.0\"}\r\n",fatigue_flag);
//		}
		
//		//Uart1_Printf("{\"method\":\"thing.event.property.post\",\"params\":{\"Longitude\":%.5f,\"Latitude\":%.5f}}",g_longitude,g_latitude);

		if(upload_flag == true)
		{
				upload_flag = false;  // 清除标志
				
				// 上传阿里云
				Uart1_Printf("{\"method\":\"thing.event.property.post\",\"params\":{\"fatigue_status\":%d}}", fatigue_flag);
				
				//LED_Turn(); // 闪灯提示
		}

		memset(Longitude, 0, sizeof(Longitude));
		memset(Latitude, 0, sizeof(Latitude));

		sprintf(Longitude, "%.5f%c", g_longitude, receDataFrame.E_W[0]);
		OLED_ShowString(1, 6, Longitude);

		sprintf(Latitude, "%.5f%c", g_latitude, receDataFrame.N_S[0]);
		OLED_ShowString(2, 6, Latitude);

		if(TIM2_2000ms_Flag == 1)
		{
				TIM2_2000ms_Flag = 0;
				//LED_Turn();
				Uart1_Printf("{\"method\":\"thing.event.property.post\",\"params\":{\"Longitude\":\"%s\",\"Latitude\":\"%s\"}}",
						Longitude, Latitude);
		}
//		LED_Turn();
//		delay_ms(1000);
	}
}

void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

		led_tick++;
		if(led_tick >= 20)    // 500ms 翻转
		{
			led_tick = 0;
			LED_Turn();
		}
	}
}

