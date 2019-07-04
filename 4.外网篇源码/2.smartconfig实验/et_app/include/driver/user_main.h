#ifndef _USER_MAIN_H_
#define _USER_MAIN_H_

//�����Ƹߵ͵�ƽ״̬
#define Red_ON    GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1)
#define Red_OFF   GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0)

//�������Ƹߵ͵�ƽ״̬
#define Blue_ON    GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1)
#define Blue_OFF   GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 0)

//�����̵Ƹߵ͵�ƽ״̬
#define Green_ON    GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1)
#define Green_OFF   GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 0)

//���尴��
#define KEY  GPIO_INPUT_GET(GPIO_ID_PIN(0))

#include "esp_common.h"  //������STM32��conf�ļ�������һЩ������ͷ�ļ�

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "espressif/espconn.h"
#include "espressif/airkiss.h"

#endif
