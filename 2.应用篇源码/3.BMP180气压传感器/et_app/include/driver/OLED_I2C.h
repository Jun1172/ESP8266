#ifndef __OLED_I2C_H
#define	__OLED_I2C_H

#define OLED_SDA_L   GPIO_OUTPUT_SET(GPIO_ID_PIN(2), 0)
#define OLED_SCL_L   GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 0)

#define OLED_SDA_H  GPIO_OUTPUT_SET(GPIO_ID_PIN(2), 1)
#define OLED_SCL_H  GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1)

void IIC_Init();
void OLED_Init(void);

void IIC_Start(void);
void IIC_Stop(void);
void OLED_CLS(void);//清屏;
void OLED_ON(void);
unsigned char IIC_Wait_Ask(void);
unsigned char IIC_ReadByte(void);
void IIC_WriteByte(unsigned char data);
void WriteDat(unsigned char data);
void WriteCmd(unsigned char command);
void OLED_Fill(unsigned char fill_Data);//全屏填充
void OLED_SetPos(unsigned char x, unsigned char y) ;//设置起始点坐标
void OLED_show_chn(uint8 x, uint8 y, uint8 N);
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize);
void OLED_Show_NUM(unsigned char x, unsigned char y,unsigned char num, unsigned char TextSize);

#endif
