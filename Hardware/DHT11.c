#include "stm32f10x.h"                  // Device header
#include "Delay.h"

#define DHT11_IO   GPIO_Pin_7
#define DHT11_PORT  GPIOA

void DHT11_IO_OUT()
{
	GPIO_InitTypeDef  GPIO_InitStructure; 	
  GPIO_InitStructure.GPIO_Pin = DHT11_IO;                   
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
}

void DHT11_IO_IN()
{
	GPIO_InitTypeDef  GPIO_InitStructure; 	
  GPIO_InitStructure.GPIO_Pin = DHT11_IO;                   
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;      
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
}

void DHT11_RST (void){ 						//DHT11端口复位，发出起始信号（IO发送）
	DHT11_IO_OUT();							//端口为输出
	GPIO_ResetBits(DHT11_PORT,DHT11_IO); 	//使总线为低电平
	Delay_ms(20); 							//拉低至少18ms						
	GPIO_SetBits(DHT11_PORT,DHT11_IO); 		//使总线为高电平							
	Delay_us(30); 							//主机拉高20~40us
}

uint8_t DHT11_Check()
{
	uint8_t retry=0;			//定义临时变量
    DHT11_IO_IN();		//IO到输入状态	 
//GPIO端口输入时，配置为上拉输入或者浮空输入，因为外接上拉电阻，所以默认为高电平
//如果DHT11的数据线输入为高电平，且 retry 小于100，则将 retry 加1，并延时1微秒，重复这个过程直到 retry 大于等于100 或者DHT11的数据线输入变成低电平。如果 retry 大于等于100，表示检测失败，返回1；否则，将 retry 重置为0。
	while ((GPIO_ReadInputDataBit(DHT11_PORT,DHT11_IO) == 1) && retry<100)	//DHT11会拉低40~80us
	{
		retry++;
        Delay_us(1);
    }
    if(retry>=100)return 1; 	
	else retry=0;
//如果DHT11的数据线输入为低电平，且 retry 小于100，则将 retry 加1，并延时1微秒，重复这个过程直到 retry 大于等于100 或者DHT11的数据线输入变成高电平。如果 retry 大于等于100，表示检测失败，返回1；否则，返回0，表示检测成功。
    while ((GPIO_ReadInputDataBit(DHT11_PORT,DHT11_IO) == 0) && retry<100)  //DHT11拉低后会再次拉高40~80us
	{  
        retry++;
        Delay_us(1);
    }
    if(retry>=100)return 1;	    
    return 0;
}

uint8_t DHT11_Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	DHT11_RST();
	return DHT11_Check();
}

uint8_t DHT11_ReadBit()
{
	uint8_t retry = 0;
    while((GPIO_ReadInputDataBit(DHT11_PORT,DHT11_IO) == 1) && retry < 100) //等待变为低电平
    {
        retry++;
        Delay_us(1);
    }
    retry = 0;
    while((GPIO_ReadInputDataBit(DHT11_PORT,DHT11_IO) == 0) && retry < 100) //等待变高电平
    {
        retry++;
        Delay_us(1);
    }
    Delay_us(40);//等待40us
    if(GPIO_ReadInputDataBit(DHT11_PORT,DHT11_IO) == 1)       //用于判断高低电平，即数据1或0
        return 1;
    else
        return 0;

}

uint8_t DHT11_ReadByte()
{
	uint8_t Data;
	for(int8_t i=0;i<8;i++)
	{
		Data<<=1;
		Data|=DHT11_ReadBit();
	}
	return Data;
}

uint8_t DHT11_ReadData(uint8_t *temp, uint8_t *humi)
{
    uint8_t buf[5];
    uint8_t i;
    DHT11_RST();						//DHT11端口复位，发出起始信号
    if(DHT11_Check() == 0)				//等待DHT11回应，0为成功回应
    {
        for(i = 0; i < 5; i++) 			//读取40位数据
        {
            buf[i] = DHT11_ReadByte();	//读出数据
        }
        if((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])	//数据校验
        {
            *humi = buf[0];				//将湿度值放入指针humi
            *temp = buf[2];				//将温度值放入指针temp
        }
    }
    else return 1;
    return 0;
}
