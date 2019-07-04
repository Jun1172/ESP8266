/******************************************************************************
 * ���ߣ���ɳ-Ƕ��ʽ-С��
 * �޸�ʱ�䣺2017-1-14
 * �ļ���: user_main.c
 * ��;�����������
*******************************************************************************/
#include "esp_common.h"  //������STM32��conf�ļ�������һЩ������ͷ�ļ�
#include "driver/OLED_I2C.h"

//���尴��
#define KEY  GPIO_INPUT_GET(GPIO_ID_PIN(0))

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
		    case 1:
		    	DHT11_get_temp_hum();
		    	OLED_CLS();
				OLED_show_chn(2,1,0);
				OLED_show_chn(18,1,1);
				OLED_ShowStr(34,1,":",2);
				OLED_Show_NUM(40,1,Warn,2);
				OLED_show_chn(56,1,1);
				OLED_show_chn(2,4,3);
				OLED_show_chn(18,4,4);
				OLED_ShowStr(34,4,":",2);
				OLED_Show_NUM(40,4,Hum,2);
		    	break;
		    case 2:
		    	OLED_CLS();
				OLED_ShowStr(34,1,"ID:JUN",2);
		    	break;
		    default :key_num=0 ;break;
		}
	}
}

/********************************************
 * ��������user_init
 * ������;��������
 * �޸�ʱ�䣺2017-1-14
 * �޸��ˣ�С��
 ********************************************/
void user_init(void)
{
	uart0_init(74880);//��ʼ�����ڣ����ò�����
	DHT11_init();
	OLED_Init();
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U,FUNC_GPIO0);//��ʼ������1
	uart0_tx_SendStr("\r\n��ʼ�����\r\n");

	// ȡ����ֹͣ����ʱ��timer
	os_timer_disarm(&timer);
	// ��ʱ�䶨ʱ������timer������һ������timer_callback����������������ڵ��ﶨʱʱ��ʱִ��һ��
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// ����ʱ�䶨ʱ������timer��ʱ������Ϊ10���룬����������Ϊ1ʱ��timer_callback����ֻ��ִ��һ��
	os_timer_arm(&timer,10,1);//��ʱ���ص�����
}
