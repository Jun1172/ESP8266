/******************************************************************************
 * ���ߣ���ɳ-Ƕ��ʽ-С��
 * �޸�ʱ�䣺2017-1-14
 * �ļ���: user_main.c
 * ��;�����������
*******************************************************************************/
#include "esp_common.h"  //������STM32��conf�ļ�������һЩ������ͷ�ļ�

#include "driver/uart.h" //GPIO�����ļ�


//����һ��ʱ�䶨ʱ������timer
LOCAL os_timer_t timer;

char key_num=0;//������ر���

//����һ���ص�����
void timer_callback()
{

}

void user_init(void)
{
	uart0_init(74880);//��ʼ�����ڣ����ò�����

	uart0_tx_SendStr("��ʼ�����\r\n");
	// ȡ����ֹͣ����ʱ��timer
	os_timer_disarm(&timer);
	// ��ʱ�䶨ʱ������timer������һ������timer_callback����������������ڵ��ﶨʱʱ��ʱִ��һ��
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// ����ʱ�䶨ʱ������timer��ʱ������Ϊ10���룬����������Ϊ1ʱ��timer_callback����ֻ��ִ��һ��
	os_timer_arm(&timer,10,1);//��ʱ���ص�����
}

