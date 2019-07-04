/******************************************************************************
 * ���ߣ���ɳ-Ƕ��ʽ-С��
 * �޸�ʱ�䣺2017-1-14
 * �ļ���: humiture.h
 * ��;����ʪ�ȴ�������������
*******************************************************************************/
#ifndef _HUMITURE_H
#define _HUMITURE_H

#include "esp_common.h"  //������STM32��conf�ļ�������һЩ������ͷ�ļ�

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
