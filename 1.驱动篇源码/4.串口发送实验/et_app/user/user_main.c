#include "esp_common.h"  //������STM32��conf�ļ�������һЩ������ͷ�ļ�

#include "driver/gpio.h" //GPIO�����ļ�
#include "driver/uart.h" //�����շ�ͷ�ļ�

//���尴��
#define KEY  GPIO_INPUT_GET(GPIO_ID_PIN(0))

//����һ����ʱ������timer
LOCAL os_timer_t timer;


/**********************************************
 * ������      : timer_callback
 * ��������  : ��ʱ���ص�����
 * ����          : none
 * ����ֵ      : none
**********************************************/
void timer_callback()
{
	if(KEY==0)//�жϰ�������
	{
		while(KEY==0);//���ּ��
		uart0_tx_SendStr("��ӭ�Ķ�esp");
		uart0_tx_SendNum(8266);
		uart0_tx_SendStr("ѧϰ�ʼ�\r\n");
		uart0_tx_SendStr("����С��\r\n");

	}
}

/******************************************************************************
 * ������      : user_init
 * ��������  : entry of user application, init user function here
 * ����          : none
 * ����ֵ      : none
*******************************************************************************/
void user_init(void)
{
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U,FUNC_GPIO0);//��ʼ������1

	//��ʼ������0�����ò�����Ϊ74880
	uart0_init(74880);

	// ȡ����ֹͣ����ʱ��timer
	os_timer_disarm(&timer);
	// ��ʱ�䶨ʱ������timer������һ������timer_callback����������������ڵ��ﶨʱʱ��ʱִ��һ��
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// ����ʱ�䶨ʱ������timer��ʱ������Ϊ10���룬����������Ϊ1ʱ��timer_callback����ֻ��ִ��һ��
	os_timer_arm(&timer,10,1);//��ʱ���ص�����
}

