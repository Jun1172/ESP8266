#ifndef __OLED_I2C_H
#define	__OLED_I2C_H


void write_cmd(et_uint8 I2C_Command);
void write_data(et_uint8 I2C_Data);
void OLED_init(void);
void OLED_set_pos(et_uint8 x, et_uint8 y);
void OLED_fill(et_uint8 fill_Data);
void OLED_clear(void);
void OLED_wakeup(void);
void OLED_sleep(void);
void OLED_show_str(et_uint8 x, et_uint8 y, et_uint8 ch[], et_uint8 TextSize);
void OLED_show_chn(et_uint8 x, et_uint8 y, et_uint8 N);
void OLED_show_bmp(et_uint8 x0,et_uint8 y0,et_uint8 x1,et_uint8 y1,et_uint8 BMP[],et_uint32 len);

#endif
