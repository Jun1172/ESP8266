/******************************************************************************
 * ���ߣ���ɳ-Ƕ��ʽ-С��
 * �޸�ʱ�䣺2017-1-14
 * �ļ���: user_main.c
 * ��;�����������
*******************************************************************************/
#include "driver/user_main.h"

//����һ������
xTaskHandle wifi_information_TaskHandle;

//���尴��
#define KEY  GPIO_INPUT_GET(GPIO_ID_PIN(0))

//����һ��ʱ�����
LOCAL os_timer_t timer;

char key_num=0;//������ر���
/*****************************************************************************************
 *  dns����
 */
ip_addr_t iot_server_ip;
LOCAL struct espconn dns_espconn;

LOCAL void ICACHE_FLASH_ATTR
dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
	struct espconn *pespconn = (struct espconn *)arg;
	if (ipaddr == NULL)
	{
		os_printf("dns_found NULL\n");
	}
	else
	{
		os_printf("dns_found  %d.%d.%d.%d\n",
		          *((uint8 *)&ipaddr->addr), *((uint8 *)&ipaddr->addr + 1),
		          *((uint8 *)&ipaddr->addr + 2), *((uint8 *)&ipaddr->addr + 3));
	}
}
void ICACHE_FLASH_ATTR
dns_test(const char *hostname)
{
	//ʵ��Ӧ��ʱ����ÿ��1sִ��һ��, dns_found�ɹ�֮��ֹͣ��ʱ������
	//dns_foundʧ�ܵĻ�,�л�ap���ٴν���dns����
	espconn_gethostbyname(&dns_espconn, hostname, &iot_server_ip, dns_found);
}

/********************************************
 * ��������timer_callback
 * ������;����ʱ���ص�����
 * �޸�ʱ�䣺2017-1-14
 * ��д��С��
 ********************************************/
void timer_callback()
{
	if(KEY==0)//�жϰ�������
	{
		while(KEY==0);//���ּ��
		key_num++;//�Լ�һ��
		dns_test("www.baidu.com");
		os_printf("\r\n����һ����ַ\r\n");
	}
}

/********************************************
 * ��������wifi_information_Task
 * ������;���������ӷ�������
 * �޸�ʱ�䣺2017-1-14
 * ��д��С��
 ********************************************/
void wifi_information_Task(void *pvParameters)
{
	uint8 getState;
	//��ȡWIFI����״̬
	getState = wifi_station_get_connect_status();
	uart0_tx_SendStr("\r\n station state:");
	uart0_tx_SendNum(getState);
	uart0_tx_SendStr("\r\n");
	//��ȡspi_flash��ID��
	os_printf("\r\n��ȡspi_flash ID��%d\r\n",spi_flash_get_id());

	//���������ֻ��һ�Σ�����͸ɵ���
	vTaskDelete(NULL);
}

/********************************************
 * ��������Internet_connet
 * ������;���������Ӻ���
 * �޸�ʱ�䣺2017-1-14
 * ��д��С��
 ********************************************/
void Internet_connet()
{
	/*
	 * �������ƣ�wifi_connet
	 * �ҵ�WIFI���ƣ�JUN
	 * �ҵ�WIFI���룺123456789
	 * ��д��С��
	 */
	wifi_connet("JUN","123456789");
	//ʹ��freertos����һ��wifi��Ϣ�ķ�������
	xTaskCreate(wifi_information_Task, (uint8 const *)"wifiTask", 1024, NULL,2, &wifi_information_TaskHandle);
}

/********************************************
 * ��������user_init
 * ������;��������
 * �޸�ʱ�䣺2017-1-14
 * ��д��С��
 ********************************************/
void user_init(void)
{
	uart0_init(74880);  //��ʼ�����ڣ����ò�����
	RGB_light_init();	//��ʼ��RGB
	Internet_connet();//����WIFI
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U,FUNC_GPIO0);//��ʼ������1
	uart0_tx_SendStr("\r\n��ʼ�����\r\n");

	// ȡ����ֹͣ����ʱ��timer
	os_timer_disarm(&timer);
	// ��ʱ�䶨ʱ������timer������һ������timer_callback����������������ڵ��ﶨʱʱ��ʱִ��һ��
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// ����ʱ�䶨ʱ������timer��ʱ������Ϊ10���룬����������Ϊ1ʱ��timer_callback����ֻ��ִ��һ��
	os_timer_arm(&timer,5,1);//��ʱ���ص�����
}
