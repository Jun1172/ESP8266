#include "esp_common.h"  //类似于STM32的conf文件，调用一些其他的头文件
#include "driver/OLED_I2C.h"
#include "driver/codetab.h"

void IIC_Init()
{
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U,FUNC_GPIO14);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U,FUNC_GPIO2);
}

//开始信号
void IIC_Start(void)
{
	OLED_SCL_H;
	OLED_SDA_H;
	ets_delay_us(2);
    OLED_SDA_L;
    ets_delay_us(2);
    OLED_SCL_L;
    ets_delay_us(2);
}

void IIC_Stop(void)
{
	OLED_SCL_H;
	OLED_SDA_L;
	ets_delay_us(2);
    OLED_SDA_H;
    ets_delay_us(2);
}


/*
*   返回1--应答出错
*   放回0--应答正确
*/
unsigned char IIC_Wait_Ask(void)
{
    int count=0;
    OLED_SCL_H;    ets_delay_us(2);
    while(GPIO_INPUT_GET(GPIO_ID_PIN(2)))
    {
        count++;
        if(count>250)
        {
            IIC_Stop();
            return 1;
        }
    }
    OLED_SCL_L;
    ets_delay_us(2);
    return 0;
}

//写一个字节
void IIC_WriteByte(unsigned char data)
{
    u8 i;
    for(i=0;i<8;i++)
    {
    	OLED_SCL_L;
    	ets_delay_us(2);
        if(data & 0x80)     //MSB,从高位开始一位一位传输
        	OLED_SDA_H;
        else
        	OLED_SDA_L;
        OLED_SCL_H;
        ets_delay_us(2);
        OLED_SCL_L;
        data<<=1;

    }
}


unsigned char IIC_ReadByte(void)
{
    u8 data,i;
    OLED_SDA_H;
    ets_delay_us(2);
    for(i=0;i<8;i++)
    {
        data<<=1;
        OLED_SCL_L;
        ets_delay_us(2);
        OLED_SCL_H;
        ets_delay_us(2);
        if(GPIO_INPUT_GET(GPIO_ID_PIN(2)))
            data=data | 0x01;
        else
            data=data & 0xFE;

    }
    OLED_SCL_L;
    ets_delay_us(2);
    return data;

}


void WriteCmd(unsigned char command)
{
    IIC_Start();
    IIC_WriteByte(0x78);//OLED地址
    IIC_Wait_Ask();
    IIC_WriteByte(0x00);//寄存器地址
    IIC_Wait_Ask();
    IIC_WriteByte(command);
    IIC_Wait_Ask();
    IIC_Stop();
}


void WriteDat(unsigned char data)
{
    IIC_Start();
    IIC_WriteByte(0x78);//OLED地址
    IIC_Wait_Ask();
    IIC_WriteByte(0x40);//寄存器地址
    IIC_Wait_Ask();
    IIC_WriteByte(data);
    IIC_Wait_Ask();
    IIC_Stop();
}

void OLED_Init(void)
{
	IIC_Init();
	ets_delay_us(100000); //这里的延时很重要

    WriteCmd(0xAE); //display off
    WriteCmd(0x20); //Set Memory Addressing Mode
    WriteCmd(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    WriteCmd(0xb0); //Set Page Start Address for Page Addressing Mode,0-7
    WriteCmd(0xc8); //Set COM Output Scan Direction
    WriteCmd(0x00); //---set low column address
    WriteCmd(0x10); //---set high column address
    WriteCmd(0x40); //--set start line address
    WriteCmd(0x81); //--set contrast control register
    WriteCmd(0xff); //亮度调节 0x00~0xff
    WriteCmd(0xa1); //--set segment re-map 0 to 127
    WriteCmd(0xa6); //--set normal display
    WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
    WriteCmd(0x3F); //
    WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
    WriteCmd(0xd3); //-set display offset
    WriteCmd(0x00); //-not offset
    WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
    WriteCmd(0xf0); //--set divide ratio
    WriteCmd(0xd9); //--set pre-charge period
    WriteCmd(0x22); //
    WriteCmd(0xda); //--set com pins hardware configuration
    WriteCmd(0x12);
    WriteCmd(0xdb); //--set vcomh
    WriteCmd(0x20); //0x20,0.77xVcc
    WriteCmd(0x8d); //--set DC-DC enable
    WriteCmd(0x14); //
    WriteCmd(0xaf); //--turn on oled panel

    OLED_CLS();
}


 /**
  * @brief  OLED_ON，将OLED从休眠中唤醒
  * @param  无
    * @retval 无
  */
void OLED_ON(void)
{
    WriteCmd(0X8D);  //设置电荷泵
    WriteCmd(0X14);  //开启电荷泵
    WriteCmd(0XAF);  //OLED唤醒
}


 /**
  * @brief  OLED_SetPos，设置光标
  * @param  x,光标x位置
    *                   y，光标y位置
  * @retval 无
  */
void OLED_SetPos(unsigned char x, unsigned char y) //设置起始点坐标
{
    WriteCmd(0xb0+y);
    WriteCmd(((x&0xf0)>>4)|0x10);
    WriteCmd((x&0x0f)|0x01);
}

 /**
  * @brief  OLED_Fill，填充整个屏幕
  * @param  fill_Data:要填充的数据
    * @retval 无
  */
void OLED_Fill(unsigned char fill_Data)//全屏填充
{
    unsigned char m,n;
    for(m=0;m<8;m++)
    {
        WriteCmd(0xb0+m);       //page0-page1
        WriteCmd(0x00);     //low column start address
        WriteCmd(0x10);     //high column start address
        for(n=0;n<128;n++)
            {
                WriteDat(fill_Data);
            }
    }
}

void OLED_CLS(void)//清屏
{
    OLED_Fill(0x00);
}

/**
  * @brief  OLED_ShowStr，显示codetab.h中的ASCII字符,有6*8和8*16可选择
  * @param  x,y : 起始点坐标(x:0~127, y:0~7);
    *                   ch[] :- 要显示的字符串;
    *                   TextSize : 字符大小(1:6*8 ; 2:8*16)
    * @retval 无
  */
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
{
    unsigned char c = 0,i = 0,j = 0;
    switch(TextSize)
    {
        case 1:
        {
            while(ch[j] != '\0')
            {
                c = ch[j] - 32;
                if(x > 126)
                {
                    x = 0;
                    y++;
                }
                OLED_SetPos(x,y);
                for(i=0;i<6;i++)
                    WriteDat(F6x8[c][i]);
                x += 6;
                j++;
            }
        }break;
        case 2:
        {
            while(ch[j] != '\0')
            {
                c = ch[j] - 32;
                if(x > 120)
                {
                    x = 0;
                    y++;
                }
                OLED_SetPos(x,y);
                for(i=0;i<8;i++)
                    WriteDat(F8X16[c*16+i]);
                OLED_SetPos(x,y+1);
                for(i=0;i<8;i++)
                    WriteDat(F8X16[c*16+i+8]);
                x += 8;
                j++;
            }
        }break;
    }
}

unsigned char Num_To_Str(unsigned char num)
{
	unsigned char Str;

	if(num==0) Str='0';
	else if(num==1) Str='1';
	else if(num==2) Str='2';
	else if(num==3) Str='3';
	else if(num==4) Str='4';
	else if(num==5) Str='5';
	else if(num==6) Str='6';
	else if(num==7) Str='7';
	else if(num==8) Str='8';

	return Str;
}

void OLED_Show_NUM(unsigned char x, unsigned char y,unsigned char num, unsigned char TextSize)
{
	unsigned char Str[2];
	Str[1]='\0';
	if(TextSize==1)
	{
		if(num>=100)
		{
			Str[0]=Num_To_Str(num/100);
			OLED_ShowStr(x,y,Str, TextSize);
			Str[0]=Num_To_Str(num%100/10);
			OLED_ShowStr(x+6,y,Str , TextSize);
			Str[0]=Num_To_Str(num%10);
			OLED_ShowStr(x+12,y,Str , TextSize);
		}else if(num<100&&num>10){
			Str[0]=Num_To_Str(num%100/10);
			OLED_ShowStr(x,y,Str , TextSize);
			Str[0]=Num_To_Str(num%10);
			OLED_ShowStr(x+6,y,Str , TextSize);
		}else if(num<10){
			Str[0]=Num_To_Str(num%10);
			OLED_ShowStr(x,y,Str , TextSize);
		}
	}
	else if(TextSize==2)
	{
		if(num>=100)
		{
			Str[0]=Num_To_Str(num/100);
			OLED_ShowStr(x,y,Str, TextSize);
			Str[0]=Num_To_Str(num%100/10);
			OLED_ShowStr(x+8,y,Str, TextSize);
			Str[0]=Num_To_Str(num%10);
			OLED_ShowStr(x+16,y,Str, TextSize);
		}else if(num<100&&num>10){
			Str[0]=Num_To_Str(num%100/10);
			OLED_ShowStr(x,y,Str, TextSize);
			Str[0]=Num_To_Str(num%10);
			OLED_ShowStr(x+8,y,Str, TextSize);
		}else if(num<10){
			Str[0]=Num_To_Str(num%10);
			OLED_ShowStr(x,y,Str , TextSize);
		}
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_show_chn(unsigned char x, unsigned char y, unsigned char N)
// Calls          :
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); N:汉字在codetab.h中的索引
// Description    : 显示codetab.h中的汉字,16*16点阵
//--------------------------------------------------------------
void OLED_show_chn(uint8 x, uint8 y, uint8 N)
{
	uint8 wm=0;
	uint32  addr=32*N;
	OLED_SetPos(x , y);
	for(wm = 0;wm < 16;wm++)
	{
		WriteDat(F16x16[addr]);
		addr += 1;
	}
	OLED_SetPos(x,y + 1);
	for(wm = 0;wm < 16;wm++)
	{
		WriteDat(F16x16[addr]);
		addr += 1;
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_show_bmp(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
// Calls          :
// Parameters     : x0,y0 -- 起始点坐标(x0:0~127, y0:0~7); x1,y1 -- 起点对角线(结束点)的坐标(x1:1~128,y1:1~8)
// Description    : 显示BMP位图
//--------------------------------------------------------------
void OLED_show_bmp(uint8 x0,uint8 y0,uint8 x1,uint8 y1,uint8 BMP[],uint32 len)
{
	uint32 j=0;
	uint8 x,y;

	for(y=y0;y<y1;y++)
	{
		OLED_SetPos(x0,y);
		for(x=x0;x<x1;x++)
		{
			if (j >= len)
			{
				return;
			}
			WriteDat(BMP[j++]);
		}
	}
}

void oled_show_gb_chn(uint8 x, uint8 y, uint8 *code)
{
	uint8 wm=0;
	uint32  addr=0;

	OLED_SetPos(x , y);
	for(wm = 0;wm < 16;wm++)
	{
		WriteDat(code[addr]);
		addr += 1;
	}
	OLED_SetPos(x,y + 1);
	for(wm = 0;wm < 16;wm++)
	{
		WriteDat(code[addr]);
		addr += 1;
	}
}

