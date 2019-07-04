/******************************************************************************
 * ���ߣ���ɳ-Ƕ��ʽ-С��
 * �޸�ʱ�䣺2017-1-14
 * �ļ���: OLED_I2C.h
 * ��;��OLED��������
*******************************************************************************/
#ifndef __OLED_I2C_H
#define	__OLED_I2C_H

#include "esp_common.h"  //������STM32��conf�ļ�������һЩ������ͷ�ļ�

#define OLED_SDA_L   GPIO_OUTPUT_SET(GPIO_ID_PIN(2), 0)
#define OLED_SCL_L   GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 0)

#define OLED_SDA_H  GPIO_OUTPUT_SET(GPIO_ID_PIN(2), 1)
#define OLED_SCL_H  GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1)

void IIC_Init();//IIC��ʼ��
void OLED_Init(void);//OLED��ʼ��

void IIC_Start(void);//���Ϳ�ʼ�ź�
void IIC_Stop(void);//ֹͣIIC
void OLED_CLS(void);//����;
void OLED_ON(void);//����OLED
unsigned char IIC_Wait_Ask(void);//�ȴ�Ӧ���ź�
unsigned char IIC_ReadByte(void);//��ȡһ���ֽ�����
void IIC_WriteByte(unsigned char data);//д��һ���ֽ�����
void WriteDat(unsigned char data);//д����
void WriteCmd(unsigned char command);//дָ��
void OLED_Fill(unsigned char fill_Data);//ȫ�����
void OLED_SetPos(unsigned char x, unsigned char y) ;//������ʼ������
void OLED_show_chn(uint8 x, uint8 y, uint8 N);
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize);//��ʾ�ַ���
void OLED_Show_NUM(unsigned char x, unsigned char y,unsigned char num, unsigned char TextSize);//��ʾ��ֵ

#endif
