#ifndef __UART1_H_
#define __UART1_H_
void Uart1_Init();

void Uart1_SendByte(uint8_t Byte);

void Uart1_SendString(char *String);
void Uart1_Printf(char *format,...);

//void USART_SendByte(USART_TypeDef* USARTx, uint16_t Data);
//void Uart1_Printf(char* fmt,...) ;
#endif
