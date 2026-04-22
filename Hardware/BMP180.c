#include "bmp180.h"
#include "Delay.h"
 
static STRUCT_BMP180_TYPEDEF bmp180;    /* 定义结构体用来保存bmp180传感器参数 */
 
/*
 * ----------------------------------------------------------------------------------------------
 * ------------------------------------- BMP180 IIC 驱动函数 -------------------------------------
 * ----------------------------------------------------------------------------------------------
 */
 
/*
 * 函数名称： bmp180_iic_init
 * 函数功能： 初始化IIC
 * 参    数： 无
 * 返 回 值： 无
 */
static void bmp180_iic_init(void)
{
    RCC_APB2PeriphClockCmd(BMP180_SDA_RCC_CLK | BMP180_SCL_RCC_CLK, ENABLE);
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
    /* SDA GPIO初始化设置 */
    GPIO_InitStructure.GPIO_Pin = BMP180_SDA_GPIO_PIN;
    GPIO_Init(BMP180_SDA_GPIO_PORT, &GPIO_InitStructure);
    
    /* SCL GPIO初始化设置 */
    GPIO_InitStructure.GPIO_Pin = BMP180_SCL_GPIO_PIN;
    GPIO_Init(BMP180_SCL_GPIO_PORT, &GPIO_InitStructure);
    
    BMP180_SCL(1); BMP180_SDA(1);
}
 
/*
 * 函数名称： bmp180_sda_inout
 * 函数功能： 配置SDA线为输入/输出模式
 * 参    数： 无
 * 返 回 值： 无
 */
static void bmp180_sda_inout(uint8_t dir)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(BMP180_SDA_RCC_CLK, ENABLE);
 
    if(dir) {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /* 推挽输出 */
    } else {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;/* 浮空输入 */
    }
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = BMP180_SDA_GPIO_PIN;
    GPIO_Init(BMP180_SDA_GPIO_PORT, &GPIO_InitStructure);
}
 
/*
 * 函数名称： bmp180_iic_start
 * 函数功能： 产生IIC起始信号
 * 参    数： 无
 * 返 回 值： 无
 */
static void bmp180_iic_start(void)
{
    bmp180_sda_inout(1); /* 配置sda线为输出模式 */
    BMP180_SDA(1);
    BMP180_SCL(1);
    Delay_us(4);
    BMP180_SDA(0);   /* 开始信号 CLK高电平时，SDA从高变低 */
    Delay_us(4);
    BMP180_SCL(0);   /* 钳住I2C总线，准备发送或接收数据 */
}
 
/*
 * 函数名称： bmp180_iic_stop
 * 函数功能： 产生IIC停止信号
 * 参    数： 无
 * 返 回 值： 无
 */
static void bmp180_iic_stop(void)
{
    bmp180_sda_inout(1);  /* 配置sda线为输出模式 */
    BMP180_SCL(0);
    BMP180_SDA(0);   /* 停止信号 CLK高电平时，SDA从低变高 */
    Delay_us(4);
    BMP180_SCL(1);
    BMP180_SDA(1);   /* 发送I2C总线结束信号 */
    Delay_us(4);
}
 
/*
 * 函数名称： bmp180_iic_waitAck
 * 函数功能： 等待应答信号到来
 * 参    数： 无
 * 返 回 值： 1-接收应答失败; 0-接收应答成功
 */
static uint8_t bmp180_iic_waitAck(void)
{
    uint8_t ucErrTime = 0;
    bmp180_sda_inout(0); /* SDA设置为输入模式 */
    
    BMP180_SDA(1);
    Delay_us(1);
    BMP180_SCL(1);
    Delay_us(1);
    while(BMP180_READ_SDA) {
        ucErrTime ++;
        if(ucErrTime>250) {
            bmp180_iic_stop();
            return 1;
        }
    }
    BMP180_SCL(0);
    return 0;
} 
 
/*
 * 函数名称： bmp180_iic_ack
 * 函数功能： 产生ACK应答
 * 参    数： 无
 * 返 回 值： 无
 */
static void bmp180_iic_ack(void)
{
    BMP180_SCL(0);
    bmp180_sda_inout(1);
    BMP180_SDA(0);
    Delay_us(2);
    BMP180_SCL(1);
    Delay_us(2);
    BMP180_SCL(0);
}
 
/*
 * 函数名称： bmp180_iic_nAck
 * 函数功能： 不产生ACK应答
 * 参    数： 无
 * 返 回 值： 无
 */
static void bmp180_iic_nAck(void)
{
    BMP180_SCL(0);
    bmp180_sda_inout(1);
    BMP180_SDA(1);
    Delay_us(2);
    BMP180_SCL(1);
    Delay_us(2);
    BMP180_SCL(0);
}
 
/*
 * 函数名称： bmp180_iic_sendByte
 * 函数功能： 发送一个字节
 * 参    数： txd
 * 返 回 值： 1，有应答;0，无应答
 */
static void bmp180_iic_sendByte(uint8_t txd)
{
    uint8_t tx_flag;
    bmp180_sda_inout(1);
    BMP180_SCL(0);
    for(uint8_t i=0;i<8;i++) {
        tx_flag = (txd&0x80) >> 7;
        if(tx_flag == 0) {
            BMP180_SDA(0);
        } else {
            BMP180_SDA(1);
        }
        txd <<= 1;
        Delay_us(2);
        BMP180_SCL(1);
        Delay_us(2); 
        BMP180_SCL(0);
        Delay_us(2);
    }
}
 
/*
 * 函数名称： bmp180_iic_readByte
 * 函数功能： 读1个字节
 * 参    数： ack (ack=1 发送ACK; ack=0 发送nACK)
 * 返 回 值： receive 接收到的数据
 */
static uint8_t bmp180_iic_readByte(uint8_t ack)
{
    uint8_t receive = 0;
    bmp180_sda_inout(0);/* SDA设置为输入 */
    
    for(uint8_t i=0;i<8;i++ ) {
        BMP180_SCL(0);
        Delay_us(2);
        BMP180_SCL(1);
        
        receive <<= 1;
        if(BMP180_READ_SDA)
            receive ++;
        Delay_us(1); 
    }
    if (!ack) {
        bmp180_iic_nAck();  /* 发送nACK */
    } else {
        bmp180_iic_ack();   /* 发送ACK */
    }
    return receive;
}
 
/*
 * 函数名称： Multiple_read
 * 函数功能： 连续读出两个内部数据
 * 参    数： reg_addr 存储单元地址
 * 返 回 值： 读取出的16位数据 
 */
static uint16_t bmp180_multiRead(uint8_t reg_addr)
{
    uint8_t msb, lsb;
    
    bmp180_iic_start();                 /* 起始信号 */
    bmp180_iic_sendByte(BMP180_ADDR_WR);/* 发送设备地址+写信号 */
    while(bmp180_iic_waitAck());
    
    bmp180_iic_sendByte(reg_addr);      /* 发送存储单元地址 */
    while(bmp180_iic_waitAck());
    
    bmp180_iic_start();                 /* 起始信号 */
    bmp180_iic_sendByte(BMP180_ADDR_RD);/* 发送设备地址+读信号 */
    while(bmp180_iic_waitAck());
 
    msb = bmp180_iic_readByte(1);       /* BUF[0]存储 */
    lsb = bmp180_iic_readByte(0);       /* 最后一个数据需要回NOACK */
    bmp180_iic_stop();                  /* 停止信号 */
    Delay_ms(5);
    
    return (msb << 8 | lsb);
}
 
/*
 * 函数名称： bmp180_readTemperatureReg
 * 函数功能： BMP180 读取温度值
 * 参    数： 无
 * 返 回 值： 16位温度数据 
 */
static uint16_t bmp180_readTemperatureReg(void)
{
    bmp180_iic_start();                 /* 起始信号 */
    bmp180_iic_sendByte(BMP180_ADDR_WR);/* 发送设备地址+写信号 */
    while(bmp180_iic_waitAck());
    
    bmp180_iic_sendByte(0xF4);          /* 温度数据 寄存器地址 */
    while(bmp180_iic_waitAck());
    
    bmp180_iic_sendByte(0x2E);          /* 向寄存器地址写入数据 */
    while(bmp180_iic_waitAck());
    
    bmp180_iic_stop();                  /* 发送停止信号 */
    Delay_ms(10);
    
    return bmp180_multiRead(0xF6);
}
 
/*
 * 函数名称： bmp180_readPressureReg
 * 函数功能： 读取气压值
 * 参    数： 无
 * 返 回 值： 16位气压数据 
 */
static uint16_t bmp180_readPressureReg(void)
{
    bmp180_iic_start();                 /* 起始信号 */
    bmp180_iic_sendByte(BMP180_ADDR_WR);/* 发送设备地址+写信号 */
    while(bmp180_iic_waitAck());
    
    bmp180_iic_sendByte(0xF4);          /* 气压数据 寄存器地址 */
    while(bmp180_iic_waitAck());
    
    bmp180_iic_sendByte(0x34);          /* 向寄存器地址写入数据 */
    while(bmp180_iic_waitAck());
    
    bmp180_iic_stop();                  /* 发送停止信号 */
    Delay_ms(20);
    
    return bmp180_multiRead(0xF6);
}
 
/*
 * 函数名称： bmp180_struct_init
 * 函数功能： 初始化气压传感器结构体参数
 * 参    数： 无
 * 返 回 值： 无
 */
static void bmp180_struct_init(void)
{
    bmp180.adjust.ac1 = bmp180_multiRead(0xAA);
    bmp180.adjust.ac2 = bmp180_multiRead(0xAC);
    bmp180.adjust.ac3 = bmp180_multiRead(0xAE);
    bmp180.adjust.ac4 = bmp180_multiRead(0xB0);
    bmp180.adjust.ac5 = bmp180_multiRead(0xB2);
    bmp180.adjust.ac6 = bmp180_multiRead(0xB4);
    bmp180.adjust.b1 =  bmp180_multiRead(0xB6);
    bmp180.adjust.b2 =  bmp180_multiRead(0xB8);
    bmp180.adjust.mb =  bmp180_multiRead(0xBA);
    bmp180.adjust.mc =  bmp180_multiRead(0xBC);
    bmp180.adjust.md =  bmp180_multiRead(0xBE);
    
    bmp180.temperature = 0.f;
    bmp180.temp_offset = 0.f;
    bmp180.pressure = 0.f;
    bmp180.pres_offset = 0.f;
    bmp180.altitude = 0.f;
    bmp180.alti_offset = 0.f;
}
 
/* ------------------------------------- 接口函数 ---------------------------------------- */
 
/*
 * 函数名称： bmp180_init
 * 函数功能： 初始化 气压传感器
 * 参    数： 无
 * 返 回 值： 无
 */
void bmp180_init(void)
{
    bmp180_iic_init();
    bmp180_struct_init();
}
 
/*
 * 函数名称： bmp180_measure
 * 函数功能： 测量一次大气压传感器
 * 参    数： 无
 * 返 回 值： 无
 */
void bmp180_measure(void)
{
    long UT, UP;
    long x1, x2, b5, b6, x3, b3, p;
    unsigned long b4, b7;
    
    UT = bmp180_readTemperatureReg();  /* 读取温度 */
    UP = bmp180_readPressureReg();     /* 读取压强 */
    
    /* ------------------------ 温度值计算 ------------------------ */
    x1 = ((UT - (long)bmp180.adjust.ac6)*(long)bmp180.adjust.ac5) >> 15;
    x2 = ((long) bmp180.adjust.mc << 11) / (x1 + bmp180.adjust.md);
    b5 = x1 + x2;
    bmp180.temperature = (((b5 + 8) >> 4)/10.f) + (((b5 + 8) >> 4)%10/10.f);/* 获取温度值 */
    
    /* ------------------------ 气压值计算 ------------------------ */
    b6 = b5 - 4000;
    /* 计算 b3 */
    x1 = (bmp180.adjust.b2 * (b6 * b6) >> 12) >> 11;
    x2 = (bmp180.adjust.ac2 * b6) >> 11;
    x3 = x1 + x2;
    b3 = (((((long)bmp180.adjust.ac1)*4 + x3) << BMP180_OSS) + 2) >> 2;
    /* 计算 b4 */
    x1 = (bmp180.adjust.ac3 * b6) >> 13;
    x2 = (bmp180.adjust.b1 * ((b6 * b6) >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    b4 = (bmp180.adjust.ac4 * (unsigned long)(x3 + 32768)) >> 15;
    b7 = ((UP - b3) * (50000 >> BMP180_OSS));
    if (b7 < 0x80000000) {
        p = (b7 << 1) / b4;
    } else {
        p = (b7 / b4) << 1;
    }
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    /* 得到气压值 */
    bmp180.pressure = (p + ((x1 + x2 + 3791) >> 4))/1000.0 + ((float)(( p +((x1 + x2 + 3791) >> 4))%1000)/1000.0);
    
    /* ------------------------ 海拔高度计算 ------------------------ */
    bmp180.altitude =  (BMP180_ZeroAltitude - p) / 133.0 * 12.0;
}
 
/*
 * 函数名称： bmp180_getTemperature
 * 函数功能： 获取大气压传感器温度数据
 * 参    数： 无
 * 返 回 值： 温度值 单位 ℃
 */
float bmp180_getTemperature(void)
{
    return (bmp180.temperature + bmp180.temp_offset);
}
 
/*
 * 函数名称： bmp180_getPressure
 * 函数功能： 获取大气压传感器大气压数据
 * 参    数： 无
 * 返 回 值： 气压值 单位 kPa
 */
float bmp180_getPressure(void)
{
    return (bmp180.pressure + bmp180.pres_offset);
}
 
/*
 * 函数名称： bmp180_getAltitude
 * 函数功能： 获取大气压传感器海拔高度
 * 参    数： 无
 * 返 回 值： 海拔高度 单位 m
 */
float bmp180_getAltitude(void)
{
    return (bmp180.altitude + bmp180.alti_offset);
}
 
/*
 * 函数名称： bmp180_set_temperature_offset
 * 函数功能： 设置温度数据补偿值
 * 参    数： 温度数据补偿值
 * 返 回 值： 无
 */
void bmp180_set_temperature_offset(float offset)
{
    bmp180.temp_offset = offset;
}
 
/*
 * 函数名称： bmp180_set_pressure_offset
 * 函数功能： 设置大气压数据补偿值
 * 参    数： 大气压数据补偿值
 * 返 回 值： 无
 */
void bmp180_set_pressure_offset(float offset)
{
    bmp180.pres_offset = offset;
}
 
/*
 * 函数名称： bmp180_set_altitude_offset
 * 函数功能： 设置海拔高度数据补偿值
 * 参    数： 海拔高度数据补偿值
 * 返 回 值： 无
 */
void bmp180_set_altitude_offset(float offset)
{
    bmp180.alti_offset = offset;
}