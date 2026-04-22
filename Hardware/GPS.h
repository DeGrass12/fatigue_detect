#ifndef __GPS_H_
#define __GPS_H_


#define TRUE   1
#define FALSE   0

#define UART2RX_MAX_LENGTH   200   // 串口2接收最大长度

// 定义数组长度
#define Frame_Buffer_Length   80   // 接收帧信息最大长度
#define UTCTime_Length   11   // UTC时间长度
#define UsefullFlag_Length   1   // 有效数据标识长度
#define latitude_Length   11   // 纬度数据长度
#define N_S_Length   2   // 纬度方向长度
#define longitude_Length   12   // 经度数据长度
#define E_W_Length   2   // 经度方向长度 





// 定义接收数据帧结构体
typedef struct FrameData 
{
	char isGetData;   // 是否获取到GPS数据
	char isParseEnd;   // 是否解析完成
	char isUsefull;   // 定位信息是否有效
	char Frame_Buffer[Frame_Buffer_Length];   // 接收到的帧信息
	char UTCTime[UTCTime_Length];   // UTC时间
	char UsefullFlag[UsefullFlag_Length];   // 有效数据标识
	char latitude[latitude_Length];   // 纬度
	char N_S[N_S_Length];   // N/S
	char longitude[longitude_Length];   // 经度
	char E_W[E_W_Length];   // E/W
}_DATAFrame;


extern _DATAFrame receDataFrame;   // 定义接收数据帧结构体

void GPS_Init();

void Data_Transfor(void);

void Uart_Rece_Pares(void);
void GpsInfor_OutPut(void);
#endif