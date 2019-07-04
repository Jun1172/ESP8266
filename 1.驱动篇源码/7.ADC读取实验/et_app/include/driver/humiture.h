/******************************************************************************
 * 作者：长沙-嵌入式-小白
 * 修改时间：2017-1-14
 * 文件名: humiture.h
 * 用途：温湿度传感器驱动函数
*******************************************************************************/
#ifndef _HUMITURE_H
#define _HUMITURE_H

#include "esp_common.h"  //类似于STM32的conf文件，调用一些其他的头文件

#define LOGIC_LL						(0)
#define LOGIC_HL						(1)

#define RETURN_OK						(0)
#define RETURN_ERR						(-1)

#define HUM_IO_NUM						(5)
#define HUM_IO_PIN						(GPIO_Pin_5)
#define HUM_DATA_SIZE					(4)

#define MAX_U16							(0xFFFF)

extern int Warn,Hum;

void DHT11_init(void);
void DHT11_get_temp_hum(void);

#endif
