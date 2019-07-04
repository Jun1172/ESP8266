/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/12/1, v1.0 create this file.
*******************************************************************************/
#include "esp_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "driver/uart.h"
#include "espressif/esp_system.h"
#include "et_types.h"

#include "driver/i2c_master.h"
#include "driver/OLED_I2C.h"

#include "driver/RGB_light.h"
#include "driver/delay.h"

#include "user_config.h"
#include "driver/gpio.h"
#include "espressif/smartconfig.h"
#include "driver/i2s.h"
#include "factory.h"
#include "et_api_compatible.h"
#ifdef IR_DEMO
#include "driver/ir.h"
#endif

extern void  et_user_main(void *pvParameters);

LOCAL os_timer_t test_timer;
LOCAL WORK_MODE_T work_mode = WORK_MODE_BUTT;

extern et_cloud_handle g_cloud_handle;
extern et_int32 to_stop_app;

et_int8 air_kiss_start_flag = 0;
et_int8 user_main_start_flag = 0;
et_int8 wifi_reconnect_start_flag = 0;

/******************************************************************************
 * FunctionName : user_get_mode_str
 * Description  : Get mode string
 * Parameters   : 
 * Returns      : NONE
*******************************************************************************/
et_uchar* ICACHE_FLASH_ATTR
user_get_mode_str(et_uint32 mode)
{
	et_uchar *mode_str[] = 
		{
			"DEFAULT",
			"AUDIO",
			"RGB",
			"BAROMETRIC",
			"OLED",
			"INVALID"
		};	

	return (WORK_MODE_BUTT <= mode ? mode_str[WORK_MODE_BUTT] : mode_str[mode]);
}

/******************************************************************************
 * FunctionName : user_esp_platform_check_ip
 * Description  : check whether get ip addr or not
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_check_ip(void)
{	
	static et_uint32 time = 0;
	et_uint32 connect_error_flag=0;
	struct ip_info ipconfig;

	os_timer_disarm(&test_timer);	
	//get ip info of ESP8266 station
	wifi_get_ip_info(STATION_IF, &ipconfig);
	if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) 
	{
		os_printf("got ip !!! \r\n");
		if (user_main_start_flag == 0)
		{
			user_main_start_flag = 1;
			xTaskCreate(et_user_main, "et_user_main", 1024, NULL, 2, NULL);
		}
		wifi_reconnect_start_flag = 1;
	} 
	else 
	{ 
	   if(wifi_station_get_connect_status() == STATION_WRONG_PASSWORD)
	   {
			if ((system_get_time() - time) >= 5000000)
			{				
				os_printf("connect fail, wrong password!!! \r\n");
				time = system_get_time();
			}
			connect_error_flag = 1;		
	    }
	    else if(wifi_station_get_connect_status() == STATION_NO_AP_FOUND)
	    {
			if ((system_get_time() - time) >= 5000000)
			{				
				os_printf("connect fail, no AP found!!! \r\n");
				time = system_get_time();
			}
			connect_error_flag = 1;	
	    }
	    else if(wifi_station_get_connect_status() == STATION_CONNECT_FAIL)
	    {
	    		if ((system_get_time() - time) >= 5000000)
			{				
				os_printf("connect fail, connect fail!!! \r\n");
				time = system_get_time();
			}
			connect_error_flag = 1;	
	    }

	    if(connect_error_flag == 1)
	    {
	           if (air_kiss_start_flag == 1)
	           {
			     wifi_station_set_reconnect_policy(false);
			     smartconfig_stop();
			     air_kiss_start_flag = 0;
			    if(work_mode == WORK_MODE_OLED) 
			    {
					OLED_clear();
					OLED_show_chn(0, 0, 15);   //show –°e:
					OLED_show_str(18, 0, "e:", 2);
					OLED_show_chn(0, 2, 8);    //show Õ¯¬Á≈‰÷√ ß∞‹
					OLED_show_chn(18, 2, 9);
					OLED_show_chn(36, 2, 10);
					OLED_show_chn(54, 2, 11);
					OLED_show_chn(72, 2, 24);
					OLED_show_chn(90, 2, 25);
					OLED_show_str(108, 2, "  ", 2);
			    }
	           }
	   }
		
		//re-arm timer to check ip
		os_timer_setfn(&test_timer, (os_timer_func_t *)user_esp_platform_check_ip, NULL);
		os_timer_arm(&test_timer, 1000, 0);
	}
}

/******************************************************************************
 * FunctionName : user_get_run_mode
 * Description  : Get the current working mode 
 * Parameters   : 
 * Returns      : NONE
*******************************************************************************/
et_uint32 ICACHE_FLASH_ATTR
user_get_run_mode()
{
	return work_mode;
}

void audio_init(void)
{
	 //audio_key_init();	// SPEAKER init
	 i2s_audio_init();
}

/******************************************************************************
 * FunctionName : user_init_work_mode
 * Description  : Initialization work mode
 * Parameters   : 
 * Returns      : NONE
*******************************************************************************/
et_int32 ICACHE_FLASH_ATTR
user_init_work_mode(et_uint32 mode, et_uchar fac_norm_mode)
{
	if (WORK_MODE_BUTT <= mode) 
	{
		os_printf("The work mode=%u is invalid !!!\n", mode);
		return RETURN_ERR;
	}

	os_printf("get work mode=%s is success !!!\n", user_get_mode_str(mode));

	switch (mode)
	{
		case WORK_MODE_DEFAULT:
			DHT11_init();
			break;
			
		case WORK_MODE_AUDIO:
			DHT11_init();
			audio_init();
			break;

		case WORK_MODE_RGB:
			RGB_light_init();		// RGB init
			DHT11_init();
			break;
		
		case WORK_MODE_BAROMETRIC:
			i2c_master_gpio_init();	// BAROMETRIC init
			DHT11_init();			// temperature init
			break;
				
		case WORK_MODE_OLED:
			i2c_master_gpio_init(); // I2C init
			OLED_init(); 			// OLED init
			OLED_clear();
			DHT11_init();
			OLED_show_chn(0, 3, 16);
			OLED_show_chn(16, 3, 17);
			OLED_show_chn(32, 3, 18);
			OLED_show_chn(48, 3, 19);
			OLED_show_chn(64, 3, 20);
			OLED_show_chn(80, 3, 21);
			OLED_show_chn(96, 3, 22);
			OLED_show_chn(112, 3, 23);
			break;
			
		default:		
			break;
	}

	return RETURN_OK;
}

/******************************************************************************
 * FunctionName : user_get_work_mode
 * Description  : Get work mode from ADC 
 * Parameters   : 
 * Returns      : NONE
*******************************************************************************/
et_int32 ICACHE_FLASH_ATTR
user_get_work_mode(et_uint32 *mode)
{
	et_uint32 adc = system_adc_read();
	if (adc > 1024) 
	{
		os_printf("The adc value=%u is invalid !!!\n", adc);
		return RETURN_ERR;
	}

#ifdef USER_PRINT_DEBUG
	os_printf("get adc value=%u is success !!!\n", adc);
#endif

	// ADC turn into work mode
	if(adc < 100) 
	{
		*mode = WORK_MODE_DEFAULT;
	} 
	else if (adc < 350) 
	{
		*mode = WORK_MODE_AUDIO;
	} 
	else if(adc < 550) 
	{
		*mode = WORK_MODE_RGB;
	} 
	else if(adc < 750) 
	{
		*mode = WORK_MODE_BAROMETRIC;
	} 
	else if(adc < 1000) 
	{
		*mode = WORK_MODE_OLED;
	} 
	else 
	{
		*mode = WORK_MODE_BUTT;
	}
	return RETURN_OK;
}

void ICACHE_FLASH_ATTR
smartconfig_done(sc_status status, void *pdata)
{
	switch(status) 
	{
		case SC_STATUS_WAIT:
	    		os_printf("SC_STATUS_WAIT\n");
	    		break;
				
		case SC_STATUS_FIND_CHANNEL:
			set_wifi_spark_timer(300);
	    		os_printf("SC_STATUS_FIND_CHANNEL\n");
	    		break;
				
		case SC_STATUS_GETTING_SSID_PSWD:
	    		os_printf("SC_STATUS_GETTING_SSID_PSWD\n");
				
	    		sc_type *type = pdata;
		    	if (*type == SC_TYPE_ESPTOUCH) 
			{
		      		os_printf("SC_TYPE:SC_TYPE_ESPTOUCH\n");
		    	} 
			else 
			{
		        	os_printf("SC_TYPE:SC_TYPE_AIRKISS\n");
			}
			break;
			
		case SC_STATUS_LINK: 
		{
			os_printf("SC_STATUS_LINK\n");
			struct station_config *sta_conf = pdata;

			wifi_station_set_config(sta_conf);
			wifi_station_disconnect();
			wifi_station_connect();
		}
			break;
			
		case SC_STATUS_LINK_OVER: {
			os_printf("SC_STATUS_LINK_OVER\n");
			smartconfig_stop();
			if(work_mode == WORK_MODE_OLED) 
			{
				OLED_clear();
				OLED_show_chn(0, 0, 15);   //show –°e:
				OLED_show_str(18, 0, "e:", 2);
				OLED_show_chn(0, 2, 8);    //show Õ¯¬Á≈‰÷√ÕÍ≥…
				OLED_show_chn(18, 2, 9);
				OLED_show_chn(36, 2, 10);
				OLED_show_chn(54, 2, 11);
				OLED_show_chn(72, 2, 13);
				OLED_show_chn(90, 2, 14);
				OLED_show_str(108, 2, "  ", 2);
			}
			delay_s(2);
			system_restart();
			break;
		}
	}
}

void airkiss_key_init(key_gpio_t*key)
{
	et_uint32 io_reg;

	io_reg = GPIO_PIN_REG(key->key_num);

	PIN_PULLUP_EN(io_reg);
	PIN_FUNC_SELECT(io_reg, 0);
	GPIO_AS_INPUT(key->key_gpio_pin);
}

void ICACHE_FLASH_ATTR
airkiss_key_poll_task(void *pvParameters)
{
	et_uint32 value, i;
	
	while(1) 
	{
		value = gpio_get_value(AIRKISS_KEY_IO_NUM);
		if(!air_kiss_start_flag && !value) 
		{
			delay_s(1);
			value = gpio_get_value(AIRKISS_KEY_IO_NUM);
			if(!air_kiss_start_flag && !value) 
			{
				os_printf("begin to airkiss\n");
				air_kiss_start_flag = 1;
				os_timer_disarm(&test_timer);
				to_stop_app = 1;  	//in airkiss mode, stop et_user_main thread
				if(g_cloud_handle != NULL)
				{
					et_logout_cloud(g_cloud_handle);
					et_destroy_context(g_cloud_handle);
					g_cloud_handle = NULL;
				}
				delay_s(1);
				wifi_reconnect_start_flag = 0;
				smartconfig_start(smartconfig_done); 	//airkiss start
				if(work_mode == WORK_MODE_OLED) 
				{
					OLED_clear();
					OLED_show_chn(0, 0, 15);    //show –°e:
					OLED_show_str(18, 0, "e:", 2);
					OLED_show_chn(0, 2, 8);    //show Õ¯¬Á≈‰÷√÷–...
					OLED_show_chn(18, 2, 9);
					OLED_show_chn(36, 2, 10);
					OLED_show_chn(54, 2, 11);
					OLED_show_chn(72, 2, 12);
					OLED_show_str(90, 2, "...", 2);
				}
   				os_timer_setfn(&test_timer, (os_timer_func_t *)user_esp_platform_check_ip, NULL);
   				os_timer_arm(&test_timer, 1000, 0);
			}
		}
		delay_ms(500);
	}

	os_printf("end airkiss\n");
	vTaskDelete(NULL);
}

void ICACHE_FLASH_ATTR
user_show_logo()
{	
	extern et_uchar BMP1[];
	et_uint32 len = 1024;	// BMP1 member
	
	i2c_master_gpio_init(); // I2C init
	OLED_init(); 			// OLED init
	OLED_clear();

	// show logo
	OLED_show_bmp(0, 0, 128, 8, BMP1, len);
}

void et_wifi_event_cb(System_Event_t *event)
{
	switch(event->event_id)
	{
		case EVENT_STAMODE_SCAN_DONE://ESP8266 station finish scanning AP
			
			break;
		case EVENT_STAMODE_CONNECTED://ESP8266 station connected to AP
			
			os_printf("et connect to ssid %s, channel %d\n", event->event_info.connected.ssid, event->event_info.connected.channel);
			break;
		case EVENT_STAMODE_DISCONNECTED://ESP8266 station disconnected to AP
			disarm_wifi_spark_timer();
			wifi_led_off();
			if(true != wifi_station_get_reconnect_policy())
			{
				os_printf("et wifi set to reconnect\n");
				wifi_station_set_reconnect_policy(true);
			}
			if(wifi_reconnect_start_flag != 1)
			{
				os_printf("airkiss start or start first don't restart %d\n",wifi_reconnect_start_flag);
			}
			else
			{
				os_printf("et wifi station connect status %d, restart system\n",wifi_station_get_connect_status());
				system_restart();
			}
			break;
		case EVENT_STAMODE_AUTHMODE_CHANGE://the auth mode of AP connected by ESP8266 station changed
			os_printf("mode: %d -> %d\n", event->event_info.auth_change.old_mode, event->event_info.auth_change.new_mode);
			break;
		case EVENT_STAMODE_GOT_IP://ESP8266 station got IP from connected AP
			set_wifi_spark_timer(1000);
			break;
		case EVENT_SOFTAPMODE_STACONNECTED://a station connected to ESP8266 soft-AP
			os_printf("et station: " MACSTR "join, AID = %d\n", MAC2STR(event->event_info.sta_connected.mac), event->event_info.sta_connected.aid);
			break;
		case EVENT_SOFTAPMODE_STADISCONNECTED://a station disconnected to ESP8266 soft-AP
			os_printf("et station: " MACSTR "leave, AID = %d\n", MAC2STR(event->event_info.sta_disconnected.mac), event->event_info.sta_disconnected.aid);
			break;
		default:
			break;
	}
}

#ifdef IR_DEMO
void ir_tx_key(void *pvParameters)
{
	et_uchar value=0;

	ir_tx_msg_t tx_data;
	tx_data.ir_tx_addr = 0x55;
	tx_data.ir_tx_data = 0x28;
	tx_data.ir_tx_rep = 0;
	
	while(1)
	{
		value = gpio_get_value(AIRKISS_KEY_IO_NUM);
		if(!value) 
		{
			delay_ms(200);
			value = gpio_get_value(AIRKISS_KEY_IO_NUM);
			if(!value) 
			{                 //Always press down
				ir_tx_func(&tx_data);
			}
		}
		delay_ms(500);
	}
	
	vTaskDelete(NULL);
	
}
#endif

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{	
	os_printf("software version:%s\n", SOFTWARE_VERSION);
	
	et_uchar result=0;
	
	if(get_fac_norm_mode(&result) != SPI_FLASH_RESULT_OK)
	{
		os_printf("get_fac_norm_mode error, NORMAL mode\n");
	}
	
	if(result == FAC_MODE)
	{
		os_printf("run in factory mode\n");
		uart_init_new_uart1(BIT_RATE_115200);
		UART_SetPrintPort(UART1);
		uart_init_new(BIT_RATE_115200, result);
		return;
	}

	os_printf("run in normal mode\n");

//if define IR_DEMO, ir rx or tx test
#ifdef IR_DEMO
	struct station_config config;
	
	memset(&config, 0, sizeof(config));
	wifi_set_opmode(STATION_MODE);
	wifi_station_set_config_current(&config);
	
 //if define IR_RX, ir rx test
#ifdef IR_RX             
	ir_rx_init();

  //ir tx test
#else                      
	ir_tx_init(0);
	xTaskCreate(ir_tx_key, "ir_tx_key", 256, NULL, 2, NULL);
#endif

#else                     
	key_gpio_t key;
	struct station_config config;
	struct ip_info info;
	
	// show logo
	user_show_logo();

	if (RETURN_OK != user_get_work_mode(&work_mode)) 
	{
		os_printf("get work mode fail !!!\n");
		return;
	}

	if (RETURN_OK != user_init_work_mode(work_mode, result)) 
	{
		os_printf("init work mode fail !!!\n");
		return;
	}
	
	wifi_set_event_handler_cb(et_wifi_event_cb);
	memset(&key, 0, sizeof(key_gpio_t));
	key.key_gpio_pin = AIRKISS_KEY_IO_PIN;
	key.key_num = AIRKISS_KEY_IO_NUM;

	airkiss_key_init(&key);
	wifi_set_opmode(STATION_MODE);

	wifi_reconnect_start_flag = 0;

	xTaskCreate(airkiss_key_poll_task, "smartconfig_task", 256, NULL, 2, NULL);

	wifi_led_init();
	memset(&config, 0, sizeof(struct station_config));
	if(wifi_station_get_config_default(&config) == true) 
	{
		os_printf("ssid=%s\n", config.ssid);
		wifi_station_set_config_current(&config);
	}
	
	os_timer_disarm(&test_timer);
   	os_timer_setfn(&test_timer, (os_timer_func_t *)user_esp_platform_check_ip, NULL);
   	os_timer_arm(&test_timer, 1000, 0);
#endif
}
