/******************************************************************************
 * ���ߣ���ɳ-Ƕ��ʽ-С��
 * �޸�ʱ�䣺2017-1-14
 * �ļ���: user_main.c
 * ��;�����������
*******************************************************************************/
#include "esp_common.h"  //������STM32��conf�ļ�������һЩ������ͷ�ļ�

#include "driver/delay.h"//��ʱ�����ļ�
#include "driver/gpio.h" //GPIO�����ļ�
#include "driver/uart.h" //GPIO�����ļ�

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

//���尴��
#define KEY  GPIO_INPUT_GET(GPIO_ID_PIN(0))

xTaskHandle xWIFITaskHandle;

//����һ��ʱ�����
LOCAL os_timer_t timer;

char key_num=0;//������ر���

/********************************************
 * ��������timer_callback
 * ������;����ʱ���ص�����
 * �޸�ʱ�䣺2017-1-14
 * �޸��ˣ�С��
 ********************************************/
void timer_callback()
{
	uint8 getState;
	if(KEY==0)//�жϰ�������
	{
		while(KEY==0);//���ּ��
		key_num++;//�Լ�һ��
		switch(key_num)
		{
			//RGB״̬�ı�
		    case 1://��ȡWIFI����״̬
		    	getState = wifi_station_get_connect_status();
		    	uart0_tx_SendStr("\r\n station state:");
		    	uart0_tx_SendNum(getState);
		    	;break;
		    case 2:
		    	uart0_tx_SendStr("\r\n");
				//��ȡspi_flash��ID��
				os_printf("\r\n��ȡspi_flash ID��%d\r\n",spi_flash_get_id());break;
		    default :key_num=0 ;break;
		}
	}
}

/********************************************
 * ��������Check_StationState
 * ������;���������ӷ�������
 * �޸�ʱ�䣺2017-1-14
 * �޸��ˣ�С��
 ********************************************/
void Check_StationState(void *pvParameters)
{
	uart0_tx_SendStr("\r\n��������WIFI\r\n");
	wifi_station_connect();     //����WIFI

	vTaskDelete(NULL);
}

/********************************************
 * ��������user_strcpy
 * ������;��ת��ָ������
 * �޸�ʱ�䣺2017-1-14
 * �޸��ˣ�С��
 ********************************************/
void ICACHE_FLASH_ATTR
user_strcpy(uint8 *src,uint8 *des)
{
  while(*des)
  {
    *src++=*des++;
  }
}

/********************************************
 * ��������wifi_connet
 * ������;��WIFI��������
 * �޸�ʱ�䣺2017-1-14
 * �޸��ˣ�С��
 ********************************************/
void wifi_connet(void)
{
	uart0_tx_SendStr("��ʼWIFI����\r\n");
	struct station_config stationConf;                      //���ýṹ��
	bzero(&stationConf, sizeof(struct station_config));  //�������
	wifi_set_opmode(STATION_MODE);                          //��������ģʽ
	user_strcpy(stationConf.ssid,"JUN");                    //�ҵ�WIFI SSID
	user_strcpy(stationConf.password,"123456789");           //�ҵ�WIFI password
	ETS_UART_INTR_DISABLE();
	wifi_station_set_config(&stationConf);                  //����WIFI����
	ETS_UART_INTR_ENABLE();

	//ʹ��freertos����һ��wifi��Ϣ�ķ�������
	xTaskCreate(Check_StationState, (uint8 const *)"wifiTask", 1024, NULL, 1, &xWIFITaskHandle);
}

/********************************************
 * ��������user_init
 * ������;��������
 * �޸�ʱ�䣺2017-1-14
 * �޸��ˣ�С��
 ********************************************/
void user_init(void)
{
	uint32 *read;
	uart0_init(74880);//��ʼ�����ڣ����ò�����
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U,FUNC_GPIO0);//��ʼ������1
	uart0_tx_SendStr("\r\n��ʼ�����\r\n");

	wifi_connet();//����WIFI

	// ȡ����ֹͣ����ʱ��timer
	os_timer_disarm(&timer);
	// ��ʱ�䶨ʱ������timer������һ������timer_callback����������������ڵ��ﶨʱʱ��ʱִ��һ��
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// ����ʱ�䶨ʱ������timer��ʱ������Ϊ10���룬����������Ϊ1ʱ��timer_callback����ֻ��ִ��һ��
	os_timer_arm(&timer,10,1);//��ʱ���ص�����
}
