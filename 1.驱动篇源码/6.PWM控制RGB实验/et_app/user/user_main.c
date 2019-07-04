/******************************************************************************
 * ���ߣ���ɳ-Ƕ��ʽ-С��
 * �޸�ʱ�䣺2017-1-14
 * �ļ���: user_main.c
 * ��;�����������
*******************************************************************************/
#include "driver/user_main.h"


//����һ��ʱ�����
LOCAL os_timer_t timer;

unsigned char red_pwn_buf=100,blue_pwn_buf=100,green_pwn_buf=100;
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
    }
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
	uart0_tx_SendStr("\r\n��ʼ�����\r\n");

	// ȡ����ֹͣ����ʱ��timer
	os_timer_disarm(&timer);
	// ��ʱ�䶨ʱ������timer������һ������timer_callback����������������ڵ��ﶨʱʱ��ʱִ��һ��
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// ����ʱ�䶨ʱ������timer��ʱ������Ϊ10���룬����������Ϊ1ʱ��timer_callback����ֻ��ִ��һ��
	os_timer_arm(&timer,5,1);//��ʱ���ص�����
}
