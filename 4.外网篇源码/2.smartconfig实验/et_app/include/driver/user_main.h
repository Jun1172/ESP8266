#ifndef _USER_MAIN_H_
#define _USER_MAIN_H_

//定义红灯高低电平状态
#define Red_ON    GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1)
#define Red_OFF   GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0)

//定义蓝灯高低电平状态
#define Blue_ON    GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1)
#define Blue_OFF   GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 0)

//定义绿灯高低电平状态
#define Green_ON    GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1)
#define Green_OFF   GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 0)

//定义按键
#define KEY  GPIO_INPUT_GET(GPIO_ID_PIN(0))

#include "esp_common.h"  //类似于STM32的conf文件，调用一些其他的头文件

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "espressif/espconn.h"
#include "espressif/airkiss.h"

#endif
