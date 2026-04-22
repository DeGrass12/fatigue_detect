#ifndef __UART2_H_
#define __UART2_H_




void Uart2_Init();
void Uart2_SendByte(uint8_t Byte);
void Uart2_SendString(char *String);
void Uart2_Printf(char *format,...);

#endif