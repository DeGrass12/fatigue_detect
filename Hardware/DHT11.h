#ifndef __DHT11_H_
#define __DHT11_H_
void DHT11_IO_OUT();

void DHT11_IO_IN();

void DHT11_RST (void);
uint8_t DHT11_Check();

uint8_t DHT11_Init();

uint8_t DHT11_ReadBit();

uint8_t DHT11_ReadByte();

uint8_t DHT11_ReadData(uint8_t *temp, uint8_t *humi);

#endif