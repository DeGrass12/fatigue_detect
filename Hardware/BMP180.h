#ifndef __BMP180_H
#define __BMP180_H
 
#include "stm32f10x.h"
 
/* 定义结构体用来保存校准数值参数 */
typedef struct {
    short ac1;
    short ac2;
    short ac3;
    unsigned short ac4;
    unsigned short ac5;
    unsigned short ac6;
    short   b1;
    short   b2;
    short   mb;
    short   mc;
    short   md;
} STRUCT_BMP180_ADJUST_TYPEDEF;
 
/* bmp180结构体 */
typedef struct {
    STRUCT_BMP180_ADJUST_TYPEDEF adjust;    /* 传感器寄存器校准值 */
    float temperature;  /* 温度值 */
    float pressure;     /* 气压值 */
    float altitude;     /* 海拔高度 */
    float temp_offset;  /* 温度补偿 */
    float pres_offset;  /* 气压补偿 */
    float alti_offset;  /* 海拔补偿 */
} STRUCT_BMP180_TYPEDEF;
 
/* -------------------------------- BMP180 IIC 引脚定义 ------------------------------- */
#define     BMP180_SCL_RCC_CLK      RCC_APB2Periph_GPIOB
#define     BMP180_SCL_GPIO_PORT    GPIOB
#define     BMP180_SCL_GPIO_PIN     GPIO_Pin_13
 
#define     BMP180_SDA_RCC_CLK      RCC_APB2Periph_GPIOB
#define     BMP180_SDA_GPIO_PORT    GPIOB
#define     BMP180_SDA_GPIO_PIN     GPIO_Pin_12
 
#define     BMP180_SCL(x)           GPIO_WriteBit(BMP180_SCL_GPIO_PORT, BMP180_SCL_GPIO_PIN, (BitAction)x)
#define     BMP180_SDA(x)           GPIO_WriteBit(BMP180_SDA_GPIO_PORT, BMP180_SDA_GPIO_PIN, (BitAction)x)
#define     BMP180_READ_SDA         GPIO_ReadInputDataBit(BMP180_SDA_GPIO_PORT, BMP180_SDA_GPIO_PIN)
 
/* ---------------------------- BMP180 寄存器地址 ----------------------------------- */
#define     BMP180_ADDR_WR          (0xEE)  /* 气压传感器器件地址+写指令 */
#define     BMP180_ADDR_RD          (0xEF)  /* 气压传感器器件地址+读指令 */
#define     BMP180_OSS              (0)     /* BMP180 OSS 等级 */
#define     BMP180_ZeroAltitude     (101325)/* 0海拔高度的气压值 单位Pa */
 
 
/* ----------------------------------- 函数清单 ----------------------------------- */
void        bmp180_init(void);                          /* bmp180初始化 */
void        bmp180_measure(void);                       /* 采集一次大气压传感器数据 */
float       bmp180_getTemperature(void);                /* 获取大气压传感器温度数据 */
float       bmp180_getPressure(void);                   /* 获取大气压传感器大气压数据 */
float       bmp180_getAltitude(void);                   /* 获取大气压传感器海拔高度数据 */
void        bmp180_set_temperature_offset(float offset);/* 设置温度数据补偿值 */
void        bmp180_set_pressure_offset(float offset);   /* 设置大气压数据补偿值 */
void        bmp180_set_altitude_offset(float offset);   /* 设置海拔高度数据补偿值 */
 
#endif