#include "esp_common.h"
#include "driver/gpio.h"
#include "driver/delay.h"
#include "user_config.h"

os_timer_t wifi_led_spark_timer;
void wifi_led_init()
{
	gpio_set_output(WIFI_STATUS_IO_PIN, WIFI_STATUS_IO_NUM);
	GPIO_OUTPUT_SET(WIFI_STATUS_IO_NUM, LOGIC_HL);	
}

void wifi_led_on()
{
	GPIO_OUTPUT_SET(WIFI_STATUS_IO_NUM, LOGIC_LL);
}


void wifi_led_off()
{
	GPIO_OUTPUT_SET(WIFI_STATUS_IO_NUM, LOGIC_HL);	
}

void wifi_led_spark(void *param)
{
	static et_uchar out_value = LOGIC_LL;
	
	GPIO_OUTPUT_SET(WIFI_STATUS_IO_NUM, out_value);
	out_value = !out_value;
}

void set_wifi_spark_timer(et_uint32 time)
{
	os_timer_disarm(&wifi_led_spark_timer);
	os_timer_setfn(&wifi_led_spark_timer, (os_timer_func_t *)wifi_led_spark, NULL);
	os_timer_arm(&wifi_led_spark_timer, time, 1);
}

void disarm_wifi_spark_timer()
{
	os_timer_disarm(&wifi_led_spark_timer);
}

