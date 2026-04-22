#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <cstdarg>
#include <string.h>
#include "uart3.h"
#include "beep.h"

RxStateTypeDef rx_state = WAIT_HEAD;
uint8_t uart_rx_buf[PACKET_LEN] = {0};
uint8_t rx_index = 0;
bool fatigue_flag = false;  // 疲劳状态标记
bool last_status = false;
bool upload_flag=false;
uint32_t fatigue_num=2;

void Uart3_Init()
{
	GPIO_InitTypeDef  GPIO_InitStrue;
	USART_InitTypeDef USART_InitStrue;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
	GPIO_InitStrue.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStrue.GPIO_Pin=GPIO_Pin_10;
	GPIO_InitStrue.GPIO_Speed=GPIO_Speed_10MHz;		
	GPIO_Init(GPIOB,&GPIO_InitStrue);
	
	GPIO_InitStrue.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_InitStrue.GPIO_Pin=GPIO_Pin_11;
	GPIO_Init(GPIOB,&GPIO_InitStrue);
	
	USART_InitStrue.USART_BaudRate=115200;
	USART_InitStrue.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStrue.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	USART_InitStrue.USART_Parity=USART_Parity_No;
	USART_InitStrue.USART_StopBits=USART_StopBits_1;
	USART_InitStrue.USART_WordLength=USART_WordLength_8b;
	
	USART_Init(USART3,&USART_InitStrue);	
	
	NVIC_InitTypeDef  NVIC_InitStruct;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 先配置优先级分组（关键！原代码缺失）
	NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0; // 最高抢占优先级
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;        // 最高子优先级
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
		
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);//接收中断
	USART_Cmd(USART3,ENABLE);	
}

void Uart3_SendByte(uint8_t Byte)
{
	USART_SendData(USART3,Byte);
	while(USART_GetFlagStatus(USART3,USART_FLAG_TXE)==RESET);
}

void Uart3_SendString(char *String)
{
	for(uint16_t i=0;String[i]!='\0';i++)
	{
		Uart3_SendByte(String[i]);
	}
}

void Uart3_Printf(char *format,...)
{
	char string[256];
	va_list arg;
	va_start(arg,format);
	vsprintf(string,format,arg);
	va_end(arg);
	Uart3_SendString(string);
}


//uint8_t rx_byte = 0;  // 存储接收到的单个字节
//void USART3_IRQHandler(void)
//{
//    // 仅处理接收中断
//    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
//    {
//        // 1. 读取接收到的字节
//        rx_byte = USART_ReceiveData(USART3);
//        
//        // 2. 回显打印（直观看到是否收到数据）
//        Uart3_Printf("收到字节：0x%02X | 字符：%c\r\n", rx_byte, rx_byte);
//        
//        // 3. 清除中断标志
//        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
//    }
//}


//void USART3_IRQHandler(void)
//{
//    uint8_t rx_data = 0;
//    
//    // 检查接收中断标志
//    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
//    {
//        // 读取接收数据
//        rx_data = USART_ReceiveData(USART3);
//        
//        // 状态机解析（永不错位的核心）
//        switch(rx_state)
//        {
//            case WAIT_HEAD:
//                // 等待帧头0xFA，收到则进入下一个状态
//                if(rx_data == FRAME_HEAD)
//                {
//                    uart_rx_buf[0] = rx_data;
//                    rx_index = 1;
//                    rx_state = GET_DATA;
//                }
//                break;
//                
//            case GET_DATA:
//                // 接收状态位，直接存入缓冲区
//                uart_rx_buf[1] = rx_data;
//                rx_index = 2;
//                rx_state = WAIT_TAIL;
//                break;
//                
//            case WAIT_TAIL:
//                // 接收帧尾，校验是否为0xFA
//                uart_rx_buf[2] = rx_data;
//                if(rx_data == FRAME_TAIL)
//                {
//                    // 数据包完整且校验通过，更新状态
//                    if(uart_rx_buf[1] == STATUS_FATIGUE)
//                    {
//                      fatigue_flag = true;  // 疲劳
//											if(last_status == false)
//                      {
//                            fatigue_num++; // 疲劳次数+1
//                      }
//                    }
//                    else if(uart_rx_buf[1] == STATUS_NORMAL)
//                    {
//                        fatigue_flag = false; // 正常
//                    }
//										last_status = fatigue_flag;
//                }
//                // 无论是否成功，立刻重置状态机（解决错位的关键）
//                rx_state = WAIT_HEAD;
//                rx_index = 0;
//                break;
//                
//            default:
//                // 异常状态，重置
//                rx_state = WAIT_HEAD;
//                rx_index = 0;
//                break;
//        }
//        
//        // 清除中断标志
//        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
//    }
//}

void USART3_IRQHandler(void)
{
    uint8_t rx_data = 0;
    
    // 检查接收中断标志
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        // 读取接收数据
        rx_data = USART_ReceiveData(USART3);
        
        // 状态机解析（永不错位的核心）
        switch(rx_state)
        {
            case WAIT_HEAD:
                // 等待帧头0xFA，收到则进入下一个状态
                if(rx_data == FRAME_HEAD)
                {
                    uart_rx_buf[0] = rx_data;
                    rx_index = 1;
                    rx_state = GET_DATA;
                }
                break;
                
            case GET_DATA:
                // 接收状态位，直接存入缓冲区
                uart_rx_buf[1] = rx_data;
                rx_index = 2;
                rx_state = WAIT_TAIL;
                break;
                
            case WAIT_TAIL:
                // 接收帧尾，校验是否为0xFA
                uart_rx_buf[2] = rx_data;
                if(rx_data == FRAME_TAIL)
                {
                    // 数据包完整且校验通过，更新状态
                    if(uart_rx_buf[1] == STATUS_FATIGUE)
                    {
                      fatigue_flag = true;  // 疲劳
											
                    }
                    else if(uart_rx_buf[1] == STATUS_NORMAL)
                    {
                        fatigue_flag = false; // 正常
                    }
										if(fatigue_flag != last_status)
                    {
                        upload_flag = true;  // 👈 变化了，需要上传
                    }
										
										if(fatigue_flag == true && last_status == false)
                    {
                        fatigue_num++;
                    }
										
										last_status = fatigue_flag;
                }
                // 无论是否成功，立刻重置状态机（解决错位的关键）
                rx_state = WAIT_HEAD;
                rx_index = 0;
                break;
                
            default:
                // 异常状态，重置
                rx_state = WAIT_HEAD;
                rx_index = 0;
                break;
        }
        
        // 清除中断标志
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}

void update_status_led(bool is_fatigue)
{
    if(is_fatigue)
    {
			
      Beep_ON();
    }
    else
    { 
       Beep_OFF();
    }
}