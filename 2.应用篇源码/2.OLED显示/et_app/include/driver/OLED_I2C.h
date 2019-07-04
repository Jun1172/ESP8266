/******************************************************************************
 * 作者：长沙-嵌入式-小白
 * 修改时间：2017-1-14
 * 文件名: OLED_I2C.h
 * 用途：OLED驱动函数
*******************************************************************************/
#ifndef __OLED_I2C_H
#define	__OLED_I2C_H

#include "esp_common.h"  //类似于STM32的conf文件，调用一些其他的头文件

#define OLED_SDA_L   GPIO_OUTPUT_SET(GPIO_ID_PIN(2), 0)
#define OLED_SCL_L   GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 0)

#define OLED_SDA_H  GPIO_OUTPUT_SET(GPIO_ID_PIN(2), 1)
#define OLED_SCL_H  GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1)

void IIC_Init();//IIC初始化
void OLED_Init(void);//OLED初始化

void IIC_Start(void);//发送开始信号
void IIC_Stop(void);//停止IIC
void OLED_CLS(void);//清屏;
void OLED_ON(void);//唤醒OLED
unsigned char IIC_Wait_Ask(void);//等待应答信号
unsigned char IIC_ReadByte(void);//读取一个字节数据
void IIC_WriteByte(unsigned char data);//写入一个字街数据
void WriteDat(unsigned char data);//写数据
void WriteCmd(unsigned char command);//写指令
void OLED_Fill(unsigned char fill_Data);//全屏填充
void OLED_SetPos(unsigned char x, unsigned char y) ;//设置起始点坐标
void OLED_show_chn(uint8 x, uint8 y, uint8 N);
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize);//显示字符串
void OLED_Show_NUM(unsigned char x, unsigned char y,unsigned char num, unsigned char TextSize);//显示数值

#endif
