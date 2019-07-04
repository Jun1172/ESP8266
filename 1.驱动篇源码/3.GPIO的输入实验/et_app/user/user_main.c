#include "esp_common.h"  //������STM32��conf�ļ�������һЩ������ͷ�ļ�

#include "driver/delay.h"//��ʱ�����ļ�
#include "driver/gpio.h" //GPIO�����ļ�

//�����Ƹߵ͵�ƽ״̬
#define Red_ON    GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1)
#define Red_OFF   GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0)

//�������Ƹߵ͵�ƽ״̬
#define Blue_ON    GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1)
#define Blue_OFF   GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 0)

//�����̵Ƹߵ͵�ƽ״̬
#define Green_ON    GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1)
#define Green_OFF   GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 0)

//���尴��
#define KEY  GPIO_INPUT_GET(GPIO_ID_PIN(0))

//����һ����ʱ������timer
LOCAL os_timer_t timer;

char key_num=0;//������ر���

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
		key_num++;//�Լ�һ��
		switch(key_num)
		{
			//RGB״̬�ı�
		    case 1:Red_ON;Blue_OFF;Green_OFF;break;
		    case 2:Red_OFF;Blue_ON;Green_OFF;break;
		    case 3:Red_OFF;Blue_OFF;Green_ON;break;
		    default :key_num=0 ;break;
		}
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
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);//��ʼ�����
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U,FUNC_GPIO14);//��ʼ������
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U,FUNC_GPIO15);//��ʼ���̵�

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U,FUNC_GPIO0);//��ʼ������1

	// ȡ����ֹͣ����ʱ��timer
	os_timer_disarm(&timer);
	// ��ʱ�䶨ʱ������timer������һ������timer_callback����������������ڵ��ﶨʱʱ��ʱִ��һ��
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// ����ʱ�䶨ʱ������timer��ʱ������Ϊ10���룬����������Ϊ1ʱ��timer_callback����ֻ��ִ��һ��
	os_timer_arm(&timer,10,1);//��ʱ���ص�����
}

