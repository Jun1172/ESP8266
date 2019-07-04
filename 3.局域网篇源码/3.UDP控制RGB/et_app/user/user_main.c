/******************************************************************************
 * ���ߣ���ɳ-Ƕ��ʽ-С��
 * �޸�ʱ�䣺2017-1-14
 * �ļ���: user_main.c
 * ��;�����������
*******************************************************************************/
#include "driver/user_main.h"

//����һ������
xTaskHandle wifi_information_TaskHandle;

//����һ��ʱ�����
LOCAL os_timer_t timer;

unsigned char red_pwn_buf=100,blue_pwn_buf=100,green_pwn_buf=100;
unsigned char red_udp_buf=100,blue_udp_buf=100,green_udp_buf=100;
unsigned char udp_red=100,udp_blue=100,udp_green=100;
/********************************************
 * ��������timer_callback
 * ������;����ʱ���ص�����
 * �޸�ʱ�䣺2017-1-14
 * ��д��С��
 ********************************************/
void timer_callback()
{
	if(red_pwn_buf!=red_pwn||blue_pwn_buf!=blue_pwn||green_pwn_buf!=green_pwn)
    {
		RGB_light_set_color(100-red_pwn,100-blue_pwn,100-green_pwn);
		red_pwn_buf=red_pwn;
		blue_pwn_buf=blue_pwn;
		green_pwn_buf=green_pwn;
    }else if(red_udp_buf!=udp_red||blue_udp_buf!=udp_blue||green_udp_buf!=udp_green)
    {
    	RGB_light_set_color(100-udp_red,100-udp_blue,100-udp_green);
    	red_udp_buf=udp_red;
    	blue_udp_buf=udp_blue;
    	green_udp_buf=udp_green;
    }

	if(udp_flag==1)
	{
		if(pdata_table[0]=='R')
		{
			if(pdata_table[3]=='%')
			{
				udp_red=pdata_table[2]-48;
			}else if(pdata_table[4]=='%')
			{
				udp_red=10*(pdata_table[2]-48)+pdata_table[3]-48;
			}else if(pdata_table[5]=='%')
			{
				udp_red=100;
			}
		}else if(pdata_table[0]=='B')
		{
			if(pdata_table[3]=='%')
			{
				udp_blue=pdata_table[2]-48;
			}else if(pdata_table[4]=='%')
			{
				udp_blue=10*(pdata_table[2]-48)+pdata_table[3]-48;
			}else if(pdata_table[5]=='%')
			{
				udp_blue=100;
			}
		}else if(pdata_table[0]=='G')
		{
			if(pdata_table[3]=='%')
			{
				udp_green=pdata_table[2]-48;
			}else if(pdata_table[4]=='%')
			{
				udp_green=10*(pdata_table[2]-48)+pdata_table[3]-48;
			}else if(pdata_table[5]=='%')
			{
				udp_green=100;
			}
		}
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
	/*
	 * �������ƣ�InterNet_InitUDP
	 * Ŀ������IP��ַ��"192.168.155.1"
	 * Ŀ�������˿ڣ�8266
	 * ���������˿ڣ�8033
	 * ��д��С��
	 */
	InterNet_InitUDP("192.168.155.1",8888,8033);

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
	uart0_tx_SendStr("\r\n��ʼ�����\r\n");

	// ȡ����ֹͣ����ʱ��timer
	os_timer_disarm(&timer);
	// ��ʱ�䶨ʱ������timer������һ������timer_callback����������������ڵ��ﶨʱʱ��ʱִ��һ��
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// ����ʱ�䶨ʱ������timer��ʱ������Ϊ10���룬����������Ϊ1ʱ��timer_callback����ֻ��ִ��һ��
	os_timer_arm(&timer,5,1);//��ʱ���ص�����
}
