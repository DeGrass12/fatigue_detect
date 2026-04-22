#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <cstdarg>
#include <string.h>
#include "stdlib.h"
#include "uart1.h"
#include "uart2.h"
#include "uart3.h"
#include "gps.h"
#include "oled.h"


// 全局变量定义
uint8_t gUart2RcecBuf[UART2RX_MAX_LENGTH];   // 串口2接收缓冲区
uint16_t gReceCunt = 0;   // 接收计数变量
_DATAFrame receDataFrame;   // 定义接收数据帧结构体


void GPS_Init()
{
	Uart2_Init();
}

void USART2_IRQHandler (void)
{
	u8 recContent;   // 存储接收内容
	
	// 如果串口接收到内容
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) 
	{
		recContent = USART_ReceiveData(USART2);   // 存储接收内容
		
		// 如果接收到的是$（$是一帧信息的开始）
		// 保证每接收到新的一帧信息就从缓冲区起始位置开始存储
		if(recContent == '$')
		{
			gReceCunt = 0;   // 清零帧信息计数变量
		}
		
		// 存储接收到的帧信息
		gUart2RcecBuf[gReceCunt ++] = recContent;
		
		// 确定是否收到"GPRMC/GNRMC"这一帧数据
		if(gUart2RcecBuf[0] == '$' && gUart2RcecBuf[4] == 'M' && gUart2RcecBuf[5] == 'C')
		{
			// 接收到换行（接收完了一帧信息）
			if(recContent == '\n')									   
			{
				memset(receDataFrame.Frame_Buffer, 0, Frame_Buffer_Length);   // 初始化接收帧信息数组
				memcpy(receDataFrame.Frame_Buffer, gUart2RcecBuf, gReceCunt);   // 保存GPRMC/GNRMC这帧的数据
				receDataFrame.isGetData = TRUE;   // 接收成功
				gReceCunt = 0;   // 清零接收帧信息接收计数变量
				memset(gUart2RcecBuf, 0, UART2RX_MAX_LENGTH);   // 清空串口2接收Buf				
			}		
		}
		
		// 如果接收内容超出最大长度，不再继续接收
		if(gReceCunt >= UART2RX_MAX_LENGTH)
		{
			gReceCunt = UART2RX_MAX_LENGTH;
		}
	}
}

/*
 *==============================================================================
 *函数名称：Uart_Rece_Pares
 *函数功能：解析串口接收内容
 *输入参数：无
 *返回值：无
 *备  注：无
 *==============================================================================
*/
void Uart_Rece_Pares(void)   // 串口接收内容解析函数
{
	// 注意变量类型
	char *point = 0;   // 逗号的地址指针
	char *nextPoint = 0;   // 下一个逗号的地址指针
	u8 tempVar = 0;   // 临时循环变量
	
	// 如果数据接收成功
	if (receDataFrame.isGetData)
	{
		receDataFrame.isGetData = 0;   // 清除接收成功标志位
		
		// for循环解析接收帧
		// 总共需要找到7个逗号
		for (tempVar = 0;tempVar < 7;tempVar ++)
		{
			// 第一次循环
			if (tempVar == 0)
			{
				// 寻找第一个逗号
				if ((point = strstr(receDataFrame.Frame_Buffer,",")) == NULL)
				{
					//Uart1_Printf ("Prase Errpr!\r\n");   // 解析错误
				}
			}
			else
			{
				point ++;   // 防止重复找到同一个逗号
				
				// 寻找下一个逗号
				// 注意strstr函数的输入变量，是从上一个逗号之后开始找下一个逗号
				if ((nextPoint = strstr(point,",")) != NULL)
				{
					// 存储信息
					switch (tempVar)
					{
						case 1:   // UTC时间
							memcpy(receDataFrame.UTCTime,point,nextPoint - point);
							break;
						
						case 2:   // 数据有效标识
							memcpy(receDataFrame.UsefullFlag,point,nextPoint - point);
							break;
						
						case 3:   // 纬度
							memcpy(receDataFrame.latitude,point,nextPoint - point);
							break;
						
						case 4:   // 纬度方向
							memcpy(receDataFrame.N_S,point,nextPoint - point);
							break;
						
						case 5:   // 经度
							memcpy(receDataFrame.longitude,point,nextPoint - point);
							break;
						
						case 6:   // 经度方向
							memcpy(receDataFrame.E_W,point,nextPoint - point);
							break;
					}
					
					point = nextPoint;   // 更新上一个逗号地址指针
					
					receDataFrame.isParseEnd = TRUE;   // 数据解析完成
					
					// 数据有效
					if (receDataFrame.UsefullFlag[0] == 'A')
					{
						receDataFrame.isUsefull = TRUE;
						//Uart1_Printf ("Data is usefull!\r\n");
					}
					else if (receDataFrame.UsefullFlag[0] == 'V')
					{
						receDataFrame.isUsefull = FALSE;
						//Uart1_Printf ("Data is invalid!\r\n");
					}
				}
				else
				{
					//Uart1_Printf ("Prase Errpr!\r\n");   // 解析错误
				}
			}
		}
	}
}



/*
 *==============================================================================
 *函数名称：Data_Transfor
 *函数功能：数据转换
 *输入参数：无
 *返回值：无
 *备  注：无
 *==============================================================================
*/
float g_longitude=0;
float g_latitude=0;
void Data_Transfor(void)
{
	float latitude = 0;   // 存储纬度信息
	u16 temp1 = 0;   // 临时变量1，存储整数
	float longitude = 0;   // 存储经度信息
	u16 temp2 = 0;   // 临时变量2，存储整数
	
	latitude = strtod(receDataFrame.latitude,NULL);   // 字符串转换成浮点数
	longitude = strtod(receDataFrame.longitude,NULL);   // 字符串转换成浮点数
	
	// 纬度信息处理
	// 五位纬度信息
	if ((latitude - 10000.0) >= 0)
	{
		// 前三位需要单独拿出来组成一个数
		temp1 = (((u16)latitude / 10000) % 10) * 100 + (((u16)latitude / 1000) % 10) * 10 + ((u16)latitude / 100) % 10;
		latitude = latitude - (float)temp1 * 100;
		latitude = (float)temp1 + latitude / 60;
		//Uart1_Printf ("latitude:%.3f\r\n",latitude);
	}
	else   // 四位纬度信息
	{
		// 前两位需要单独拿出来组成一个数
		temp1 = (((u16)latitude / 1000) % 10) * 10 + ((u16)latitude / 100) % 10;
		latitude = latitude - (float)temp1 * 100;
		latitude = (float)temp1 + latitude / 60;
		//Uart1_Printf ("latitude:%.3f\r\n",latitude);
	}
	
	// 经度信息处理
	// 五位经度信息
	if ((longitude - 10000.0) >= 0)
	{
		// 前三位需要单独拿出来组成一个数
		temp2 = (((u16)longitude / 10000) % 10) * 100 + (((u16)longitude / 1000) % 10) * 10 + ((u16)longitude / 100) % 10;
		longitude = longitude - (float)temp2 * 100;
		longitude = (float)temp2 + longitude / 60;
		//Uart1_Printf ("longitude:%.3f\r\n",longitude);
	}
	else   // 四位经度信息
	{
		// 前两位需要单独拿出来组成一个数
		temp2 = (((u16)longitude / 1000) % 10) * 10 + ((u16)longitude / 100) % 10;
		longitude = longitude - (float)temp2 * 100;
		longitude = (float)temp2 + longitude / 60;
		//Uart1_Printf ("longitude:%.3f\r\n",longitude);
	}
	g_longitude=longitude;
	g_latitude=latitude;
	//Uart1_Printf("{\"method\" : \"thing.event.property.post\", \"params\" : {\"Longitude\" : %f, \"Latitude\":%f}, \"version\" : \"1.0.0\"}\r\n",longitude,latitude);
	//Uart3_Printf("Longitude:%f,Latitude:%f\r\n",longitude,latitude);
}

/*
 *==============================================================================
 *函数名称：GpsInfor_OutPut
 *函数功能：GPS信息输出
 *输入参数：无
 *返回值：无
 *备  注：无
 *==============================================================================
*/
void GpsInfor_OutPut (void)
{
	// 数据解析完成时输出
	if (receDataFrame.isParseEnd)
	{
		// 清除数据解析完成标志位
		receDataFrame.isParseEnd = FALSE;
		
		// 即使定位失败也会有UTC时间信息
		//Uart1_Printf ("UTCTime: %s\r\n",receDataFrame.UTCTime );
	}
	
	// 数据有效时输出
	if (receDataFrame.isUsefull)
	{
		//Uart1_Printf ("UTCTime: %s\r\n",receDataFrame.UTCTime );
//		Uart1_Printf ("N/S: %s\r\n",receDataFrame.N_S);
//		Uart1_Printf ("E/W: %s\r\n",receDataFrame.E_W);
		
		
		Data_Transfor();   // 数据转换输出
	}
}