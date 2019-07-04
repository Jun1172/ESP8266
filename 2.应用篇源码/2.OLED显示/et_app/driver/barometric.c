#include "espressif/c_types.h"
#include "driver/i2c_master.h"
#include "esp_common.h"
#include "driver/delay.h"
#include "et_types.h"

#define OSS 0

et_int16  AC1=0x00;
et_int16   AC2=0x00;
et_int16   AC3=0x00;
et_uint16   AC4=0x00;
et_uint16  AC5=0x00;
et_uint16  AC6=0x00;
et_int16    B1=0x00;
et_int16    B2=0x00;
et_int16    MB=0x00;
et_int16    MC=0x00;
et_int16   MD=0x00;

static et_long32 temperature=0;

et_int16 read_register(et_uchar reg_addr)
{
	et_uchar rec_msb =0x0, rec_lsb = 0x0;
	et_int16 rec_data=0x0000;
	
	i2c_master_start();                                        //
	     
	i2c_master_writeByte(0xEE);     
	if(i2c_master_getAck() != 0)
	{
		i2c_master_stop();
		return -1;
	}
	
	i2c_master_writeByte(reg_addr);
	if(i2c_master_getAck() != 0)
	{
		i2c_master_stop();
		return -1;
	}
	
	i2c_master_start();
       i2c_master_wait(40000);
	   
       i2c_master_start();
	i2c_master_writeByte(0xEF);
	if(i2c_master_getAck() != 0)
	{
		i2c_master_stop();
		return -1;
	}


	rec_msb= i2c_master_readByte();
	i2c_master_send_ack();
	
	rec_lsb= i2c_master_readByte();
	i2c_master_send_nack();
	
	i2c_master_stop();
	rec_data = (rec_msb << 8) | rec_lsb;

	return rec_data;
	
}

et_int16 write_register(et_uchar reg_addr, et_uchar reg_data)
{
	i2c_master_start();                                        // 1. master send start
	i2c_master_writeByte(0xEE);                         // 2. master send 7'b device addr and 1'b w 
	if(i2c_master_getAck() != 0)
	{
		i2c_master_stop();
		return -1;
	}          									  // 3. slave ack
	

	i2c_master_writeByte(reg_addr);             	  // 4.master write reg addr
	if(i2c_master_getAck() != 0)
	{
		i2c_master_stop();
		return -1;
	}        // 5.slave send ack

	i2c_master_writeByte(reg_data);              	// 6.master send data
	if(i2c_master_getAck() != 0)
	{
		i2c_master_stop();
		return -1;
	}                   // ack

	i2c_master_stop();

	return 0;
}

void read_calibration(void)
{
	AC1 = read_register(0xAA);
	AC2 = read_register(0xAC);
	AC3 = read_register(0xAE);
	AC4 = (et_uint16)read_register(0xB0);
	AC5 = (et_uint16)read_register(0xB2);
	AC6 = (et_uint16)read_register(0xB4);
	B1 = read_register(0xB6);
	B2= read_register(0xB8);
	MB = read_register(0xBA);
	MC = read_register(0xBC);
	MD = read_register(0xBE);
}

et_long32 read_temp(void)
{
	et_long32 temp = -1;
	
	if(write_register(0xF4, 0x2E) == -1)
		return temp;
	
	os_delay_us(4500);
	temp = read_register(0xF6);
	
	return temp;
}

et_long32 read_pressure(void)
{
	et_long32 press = -1;
	et_int32 oss, delay;

	switch(OSS)
	{
		case 0:
			delay = 4500;
			break;
		case 1:
			delay = 7500;
			break;
		case 2:
			delay = 13500;
			break;
		case 3:
			delay = 25500;
			break;
		default:
			delay = 4500;
			break;
	}
	
	if(write_register(0xF4, (0x34 + (OSS << 6))) == -1)
		return -1;
	
	os_delay_us(delay);
	press = read_register(0xF6)  & 0x0000FFFF; 
	if(press == -1)
		return press;
	//press >>= (8 - OSS);
	
	return press;
}

et_long32 calculate(et_long32 UT, et_long32 UP)
{
	et_long32 X1, X2, B5, B6, X3, B3, P, T, B7;
	unsigned long B4; 
	
	X1 = ((et_long32)UT - AC6) * AC5  >> 15;    //Calculate temperature start 
	X2 = ((et_long32) MC << 11) / (X1 + MD);
	B5 = X1 + X2;
	T = (B5 + 8) >> 4;//calculate temperature finish
	T /= 10.0;
	temperature = (et_long32)T;
	
	B6 = B5 - 4000;       //Calculate pressure start 
	X1 = (B2 * (B6 * B6 >> 12)) >> 11;
	X2 = (AC2 * B6) >> 11;
	X3 = X1 + X2;
	B3 = ((((et_long32)AC1 * 4 + X3)<<OSS) + 2)/4;
	X1 = AC3 * B6 >> 13;
	X2 = (B1 * (B6 * B6 >> 12)) >> 16;
	X3 = ((X1 + X2) + 2) >> 2;
	B4 = (AC4 * (et_ulong32) (X3 + 32768)) >> 15;
	B7 = ((et_ulong32) UP - B3) * (50000 >> OSS);
	
	if( B7 < 0x80000000) 
	{
		P = (B7 * 2) / B4 ;
	} 
	else 
	{                 
		P = (B7 / B4) * 2;
	}
	
	X1 = (P >> 8) * (P >> 8);
	X1 = (X1 * 3038) >> 16;
	X2 = (-7357 * P) >> 16;

	P = P + ((X1 + X2 + 3791) >> 4);
	
	return P;
}

et_long32 barometric_collect(et_long32 *temp)
{	
	et_long32 UP, UT, baro;

	UT = read_temp();
	if(UT == -1)
		return -1;
	
	UP = read_pressure();
	if(UP == -1)
		return -1;
	
	read_calibration();
	
       baro = calculate(UT, UP);
	*temp = temperature;

	return baro;
}
