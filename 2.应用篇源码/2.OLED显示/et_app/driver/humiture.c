/******************************************************************************
 * ���ߣ���ɳ-Ƕ��ʽ-С��
 * �޸�ʱ�䣺2017-1-14
 * �ļ���: humiture.c
 * ��;����ʪ�ȴ�������������
*******************************************************************************/
#include "driver/humiture.h"

int Warn,Hum;

/********************************************
 * ��������DHT11_read_byte
 * ������;����ȡ����������--Э��
 * �޸�ʱ�䣺2017-1-14
 * �޸��ˣ�С��
 ********************************************/
int32 ICACHE_FLASH_ATTR
DHT11_read_byte(uint8 *buf)
{
	uint8 i, data = 0;
	uint16 delay_cnt;
	
	for (i = 8; i > 0; i--) 
	{
		// start flag(50us->L), test value about 60us
		delay_cnt = 12;
		while ((LOGIC_LL == GPIO_INPUT_GET(GPIO_ID_PIN(5))) && (delay_cnt--))
			os_delay_us(5);

		if (MAX_U16 == delay_cnt)
		{
			uart0_tx_SendStr("start flag L is too long\n");
			return RETURN_ERR;
		}
		
		// 0:30us->H, test value about 35us	
		delay_cnt = 7;
		while ((LOGIC_HL == GPIO_INPUT_GET(GPIO_ID_PIN(5))) && (delay_cnt--))
			os_delay_us(5);
		
		if (MAX_U16 != delay_cnt)
			continue;
		
		data |= (1 << (i - 1));
		
		// 1:70us->H, test value about 75us
		delay_cnt = 10;
		while ((LOGIC_HL == GPIO_INPUT_GET(GPIO_ID_PIN(5))) && (delay_cnt--))
			os_delay_us(5);
		
		if (MAX_U16 == delay_cnt)
		{
			uart0_tx_SendStr("data H is too long\n");
			return RETURN_ERR;
		}
	}
	
	*buf = data;
	return RETURN_OK;
}

/********************************************
 * ��������DHT11_get_temp_hum
 * ������;����ȡ����������
 * �޸�ʱ�䣺2017-1-14
 * �޸��ˣ�С��
 ********************************************/
void DHT11_get_temp_hum(void)
{
	uint8 i;
	uint8 check = 0;
	uint8 tmp[HUM_DATA_SIZE + 1] = {0};
	uint16 rsp_L_cnt = 16;
	uint16 rsp_H_cnt = 20;
	
	// start signal(L > 18ms)
	GPIO_OUTPUT_SET(HUM_IO_NUM, LOGIC_LL);
	os_delay_us(20000);
	
	// wait for response(20us < H < 40us)
	GPIO_OUTPUT_SET(HUM_IO_NUM, LOGIC_HL);
	os_delay_us(30);
	
	// wait for response from the slave
	GPIO_DIS_OUTPUT(HUM_IO_NUM);
	os_delay_us(3);
	
	if (LOGIC_LL != GPIO_INPUT_GET(GPIO_ID_PIN(5)))
	{
		uart0_tx_SendStr("DHT No response signal!\n");
		goto error;
	}
	
	// DHT response L->80us, test value about 40us
	while ((LOGIC_LL == GPIO_INPUT_GET(GPIO_ID_PIN(5))) && (rsp_L_cnt--))
		os_delay_us(5);

	if (MAX_U16 == rsp_L_cnt)
	{
		uart0_tx_SendStr("DHT response L is timeout\n");
		goto error;
	}
		
	// DHT response H->80us, test value about 90us
	while ((LOGIC_HL == GPIO_INPUT_GET(GPIO_ID_PIN(5))) && (rsp_H_cnt--))
		os_delay_us(5);
	
	if (MAX_U16 == rsp_H_cnt)
	{
		uart0_tx_SendStr("DHT response H is timeout\n");
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
		uart0_tx_SendStr("data check is fail!!!\n");
		goto error;
	}
	Hum=tmp[0];
	Warn=tmp[2];

	GPIO_OUTPUT_SET(HUM_IO_NUM, LOGIC_HL);

error:
	GPIO_OUTPUT_SET(HUM_IO_NUM, LOGIC_HL);
}

/********************************************
 * ��������DHT11_init
 * ������;����ʪ�ȴ�������ʼ������
 * �޸�ʱ�䣺2017-1-14
 * �޸��ˣ�С��
 ********************************************/
void DHT11_init(void)
{	
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
	GPIO_OUTPUT_SET(HUM_IO_NUM, LOGIC_HL);
	PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO5_U);
}


