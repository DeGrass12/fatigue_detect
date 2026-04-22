#ifndef __UART3_H_
#define __UART3_H_

#include <stdint.h>

#include "stm32f10x.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>



// 数据包定义
#define FRAME_HEAD               0xAF
#define FRAME_TAIL               0xFA
#define PACKET_LEN               3
#define STATUS_NORMAL            0x00
#define STATUS_FATIGUE           0x01


// 状态机枚举
typedef enum {
    WAIT_HEAD,    // 等待帧头0xFA
    GET_DATA,     // 获取状态位
    WAIT_TAIL     // 等待帧尾0xFA
} RxStateTypeDef;

// 全局变量声明
extern RxStateTypeDef rx_state;
extern uint8_t uart_rx_buf[PACKET_LEN];
extern uint8_t rx_index;
extern bool fatigue_flag;
extern uint32_t fatigue_num;
extern bool upload_flag;


void Uart3_Init();
void Uart3_SendByte(uint8_t Byte);
void Uart3_SendString(char *String);
void Uart3_Printf(char *format,...);
void update_status_led(bool is_fatigue);
#endif
