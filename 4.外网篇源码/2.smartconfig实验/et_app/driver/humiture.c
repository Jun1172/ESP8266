#include "esp_common.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "user_config.h"


et_int32 ICACHE_FLASH_ATTR 
DHT11_read_byte(et_uint8 *buf)
{
	et_uint8 i, data = 0;
	et_uint16 delay_cnt;
	
	for (i = 8; i > 0; i--) 
	{
		// start flag(50us->L), test value about 60us
		delay_cnt = 12;
		while ((LOGIC_LL == gpio_get_value(HUM_IO_NUM)) && (delay_cnt--))
			os_delay_us(5);
		
		if (MAX_U16 == delay_cnt)
		{
			printf("start flag L is too long\n");
			return RETURN_ERR;
		}
		
		// 0:30us->H, test value about 35us	
		delay_cnt = 7;
		while ((LOGIC_HL == gpio_get_value(HUM_IO_NUM)) && (delay_cnt--))			
			os_delay_us(5);
		
		if (MAX_U16 != delay_cnt)
			continue;
		
		data |= (1 << (i - 1));
		
		// 1:70us->H, test value about 75us
		delay_cnt = 10;
		while ((LOGIC_HL == gpio_get_value(HUM_IO_NUM)) && (delay_cnt--))
			os_delay_us(5);
		
		if (MAX_U16 == delay_cnt)
		{
			printf("data H is too long\n");
			return RETURN_ERR;
		}
	}
	
	*buf = data;
#ifdef USER_PRINT_DEBUG
//	printf("read byte is 0x%x\n", data);
#endif
	return RETURN_OK;
}

et_int32 ICACHE_FLASH_ATTR 
DHT11_get_temp_hum(et_uint8 *buf, et_uint32 len)
{
	et_uint8 i;
	et_uint8 check = 0;
	et_uint8 tmp[HUM_DATA_SIZE + 1] = {0};
	et_uint16 rsp_L_cnt = 16;
	et_uint16 rsp_H_cnt = 20;
	
	// start signal(L > 18ms)
	GPIO_OUTPUT_SET(HUM_IO_NUM, LOGIC_LL);
	os_delay_us(20000);
	
	// wait for response(20us < H < 40us)
	GPIO_OUTPUT_SET(HUM_IO_NUM, LOGIC_HL);
	os_delay_us(30);
	
	// wait for response from the slave
	GPIO_DIS_OUTPUT(HUM_IO_NUM);
	os_delay_us(3);
	
	if (LOGIC_LL != gpio_get_value(HUM_IO_NUM))
	{
		printf("DHT No response signal!\n");
		goto error;
	}
	
	// DHT response L->80us, test value about 40us
	while ((LOGIC_LL == gpio_get_value(HUM_IO_NUM)) && (rsp_L_cnt--))
		os_delay_us(5);

	if (MAX_U16 == rsp_L_cnt)
	{
		printf("DHT response L is timeout\n");
		goto error;
	}
		
	// DHT response H->80us, test value about 90us
	while ((LOGIC_HL == gpio_get_value(HUM_IO_NUM)) && (rsp_H_cnt--))
		os_delay_us(5);
	
	if (MAX_U16 == rsp_H_cnt)
	{
		printf("DHT response H is timeout\n");
		goto error;
	}
	
	for (i = 0; i < HUM_DATA_SIZE + 1; i++)
	{	
		if (RETURN_OK != DHT11_read_byte(&tmp[i]))
		{
			goto error;
		}
		if (i != HUM_DATA_SIZE)
		{
			check += tmp[i];
		}
	}
	
	if (check != tmp[HUM_DATA_SIZE])
	{
		printf("data check is fail!!!\n");
		goto error;
	}
	GPIO_OUTPUT_SET(HUM_IO_NUM, LOGIC_HL);
	memcpy(buf, tmp, len);
	
	return RETURN_OK;

error:
	GPIO_OUTPUT_SET(HUM_IO_NUM, LOGIC_HL);

	return RETURN_ERR;
}

et_int32 ICACHE_FLASH_ATTR 
DHT11_read_temp_hum(et_uint8 *buf, et_uint32 len)
{	
	et_int32 ret = RETURN_OK;
	et_ulong32 task_priority = 0;
		
	if (HUM_DATA_SIZE > len)
	{
		printf("parameter length(%d) is error\n", len);
		return RETURN_ERR;
	}
	if (HUM_DATA_SIZE < len)
		len = HUM_DATA_SIZE;

	task_priority = uxTaskPriorityGet(NULL);	
	vTaskPrioritySet(NULL, tskIDLE_PRIORITY + 14);
	
	if (RETURN_OK != DHT11_get_temp_hum(buf, len))
	{	
		ret = RETURN_ERR;
	}
	vTaskPrioritySet(NULL, tskIDLE_PRIORITY + task_priority);
	
	return ret;
}


void ICACHE_FLASH_ATTR
DHT11_init()
{	
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
	GPIO_OUTPUT_SET(HUM_IO_NUM, LOGIC_HL);
	PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO5_U);
}


