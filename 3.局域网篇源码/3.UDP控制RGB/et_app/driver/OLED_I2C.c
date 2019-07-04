/************************************************************************************

* 
* Description:128*64�����OLED��ʾ�������ļ����������ڻ����Զ���(heltec.taobao.com)��SD1306����IICͨ�ŷ�ʽ��ʾ��
*
* Others: none;
*
* Funct ion List:
* 1. void I2C_Configuration(void) -- ����CPU��Ӳ��I2C
* 2. void I2C_WriteByte(et_uint8_t addr,et_uint8_t data) -- ��Ĵ�����ַдһ��byte������
* 3. void write_cmd(unsigned char I2C_Command) -- д����
* 4. void write_data(unsigned char I2C_Data) -- д����
* 5. void OLED_init(void) -- OLED����ʼ��
* 6. void OLED_set_pos(unsigned char x, unsigned char y) -- ������ʼ������
* 7. void OLED_fill(unsigned char fill_Data) -- ȫ�����
* 8. void OLED_clear(void) -- ����
* 9. void OLED_wakeup(void) -- ����
* 10. void OLED_sleep(void) -- ˯��
* 11. void OLED_show_str(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize) -- ��ʾ�ַ���(�����С��6*8��8*16����)
* 12. void OLED_show_chn(unsigned char x, unsigned char y, unsigned char N) -- ��ʾ����(������Ҫ��ȡģ��Ȼ��ŵ�codetab.h��)
* 13. void OLED_show_bmp(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]) -- BMPͼƬ
*
* History: none;
*
*************************************************************************************/
#include "et_types.h"
#include "espressif/c_types.h"
#include "driver/OLED_I2C.h"
#include "driver/delay.h"
#include "driver/codetab.h"

void write_cmd(et_uint8 I2C_Command)//д����
{	
	i2c_master_start();
	i2c_master_writeByte(0x78);	//Slave address,SA0=0
	i2c_master_waitAck();
	i2c_master_writeByte(0x00);	//write command
	i2c_master_waitAck();
	i2c_master_writeByte(I2C_Command);
	i2c_master_waitAck();
	i2c_master_stop();
}

void write_data(et_uint8 I2C_Data)//д����
{
	i2c_master_start();
	i2c_master_writeByte(0x78);	//D/C#=0; R/W#=0
	i2c_master_waitAck();
	i2c_master_writeByte(0x40);	//write data
	i2c_master_waitAck();
	i2c_master_writeByte(I2C_Data);
	i2c_master_waitAck();
	i2c_master_stop();
}

void OLED_init(void)
{
	delay_ms(100); //�������ʱ����Ҫ
	delay_ms(500); //�������ʱ����Ҫ
	write_cmd(0xAE); //display off
	write_cmd(0x20);	//Set Memory Addressing Mode	
	write_cmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	write_cmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	write_cmd(0xc8);	//Set COM Output Scan Direction
	write_cmd(0x00); //---set low column address
	write_cmd(0x10); //---set high column address
	write_cmd(0x40); //--set start line address
	write_cmd(0x81); //--set contrast control register
	write_cmd(0xff); //���ȵ��� 0x00~0xff
	write_cmd(0xa1); //--set segment re-map 0 to 127
	write_cmd(0xa6); //--set normal display
	write_cmd(0xa8); //--set multiplex ratio(1 to 64)
	write_cmd(0x3F); //
	write_cmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	write_cmd(0xd3); //-set display offset
	write_cmd(0x00); //-not offset
	write_cmd(0xd5); //--set display clock divide ratio/oscillator frequency
	write_cmd(0xf0); //--set divide ratio
	write_cmd(0xd9); //--set pre-charge period
	write_cmd(0x22); //
	write_cmd(0xda); //--set com pins hardware configuration
	write_cmd(0x12);
	write_cmd(0xdb); //--set vcomh
	write_cmd(0x20); //0x20,0.77xVcc
	write_cmd(0x8d); //--set DC-DC enable
	write_cmd(0x14); //
	write_cmd(0xaf); //--turn on oled panel
}

void OLED_set_pos(et_uint8 x, et_uint8 y) //������ʼ������
{ 
	write_cmd(0xb0+y);
	write_cmd(((x&0xf0)>>4)|0x10);
	write_cmd((x&0x0f)|0x01);
}

void OLED_fill(et_uint8 fill_Data)//ȫ�����
{
	et_uint8 m,n;
	for(m=0;m<8;m++)
	{
		write_cmd(0xb0+m);		//page0-page1
		write_cmd(0x00);		//low column start address
		write_cmd(0x10);		//high column start address
		for(n=0;n<128;n++)
		{
			write_data(fill_Data);
		}
	}
}

void OLED_clear(void)//����
{
	OLED_fill(0x00);
}

//--------------------------------------------------------------
// Prototype      : void OLED_wakeup(void)
// Calls          : 
// Parameters     : none
// Description    : ��OLED�������л���
//--------------------------------------------------------------
void OLED_wakeup(void)
{
	write_cmd(0X8D);  //���õ�ɱ�
	write_cmd(0X14);  //������ɱ�
	write_cmd(0XAF);  //OLED����
}

//--------------------------------------------------------------
// Prototype      : void OLED_sleep(void)
// Calls          : 
// Parameters     : none
// Description    : ��OLED���� -- ����ģʽ��,OLED���Ĳ���10uA
//--------------------------------------------------------------
void OLED_sleep(void)
{
	write_cmd(0X8D);  //���õ�ɱ�
	write_cmd(0X10);  //�رյ�ɱ�
	write_cmd(0XAE);  //OLED����
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
// Calls          : 
// Parameters     : x,y -- ��ʼ������(x:0~127, y:0~7); ch[] -- Ҫ��ʾ���ַ���; TextSize -- �ַ���С(1:6*8 ; 2:8*16)
// Description    : ��ʾcodetab.h�е�ASCII�ַ�,��6*8��8*16��ѡ��
//--------------------------------------------------------------
void OLED_show_str(et_uint8 x, et_uint8 y, et_uint8 ch[], et_uint8 TextSize)
{
	et_uint8 c = 0,i = 0,j = 0;

	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				OLED_set_pos(x,y);
				for(i=0;i<6;i++)
					write_data(F6x8[c][i]);
				x += 6;
				j++;
			}
		}
		break;
		
		case 2:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y += 2;
				}
				OLED_set_pos(x,y);
				for(i=0;i<8;i++)
					write_data(F8X16[c*16+i]);
				OLED_set_pos(x,y+1);
				for(i=0;i<8;i++)
					write_data(F8X16[c*16+i+8]);
				x += 8;
				j++;
			}
		}
		break;
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_show_chn(unsigned char x, unsigned char y, unsigned char N)
// Calls          : 
// Parameters     : x,y -- ��ʼ������(x:0~127, y:0~7); N:������codetab.h�е�����
// Description    : ��ʾcodetab.h�еĺ���,16*16����
//--------------------------------------------------------------
void OLED_show_chn(et_uint8 x, et_uint8 y, et_uint8 N)
{
	et_uint8 wm=0;
	et_uint32  addr=32*N;
	OLED_set_pos(x , y);
	for(wm = 0;wm < 16;wm++)
	{
		write_data(F16x16[addr]);
		addr += 1;
	}
	OLED_set_pos(x,y + 1);
	for(wm = 0;wm < 16;wm++)
	{
		write_data(F16x16[addr]);
		addr += 1;
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_show_bmp(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
// Calls          : 
// Parameters     : x0,y0 -- ��ʼ������(x0:0~127, y0:0~7); x1,y1 -- ���Խ���(������)������(x1:1~128,y1:1~8)
// Description    : ��ʾBMPλͼ
//--------------------------------------------------------------
void OLED_show_bmp(et_uint8 x0,et_uint8 y0,et_uint8 x1,et_uint8 y1,et_uint8 BMP[],et_uint32 len)
{
	et_uint32 j=0;
	et_uint8 x,y;
	
	for(y=y0;y<y1;y++)
	{
		OLED_set_pos(x0,y);
		for(x=x0;x<x1;x++)
		{
			if (j >= len)
			{
				return;
			}
			write_data(BMP[j++]);
		}
	}
}

void oled_show_gb_chn(et_uint8 x, et_uint8 y, et_uint8 *code)
{
	et_uint8 wm=0;
	et_uint32  addr=0;
	
	OLED_set_pos(x , y);
	for(wm = 0;wm < 16;wm++)
	{
		write_data(code[addr]);
		addr += 1;
	}
	OLED_set_pos(x,y + 1);
	for(wm = 0;wm < 16;wm++)
	{
		write_data(code[addr]);
		addr += 1;
	}
}