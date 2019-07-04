/******************************************************************************
 * ���ߣ���ɳ-Ƕ��ʽ-С��
 * �޸�ʱ�䣺2017-1-14
 * �ļ���: user_main.c
 * ��;�����������
*******************************************************************************/
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

//����һ��ʱ�䶨ʱ������timer
LOCAL os_timer_t timer;

//����һ���ص�����
void timer_callback()
{
	//RGB״̬�ı�
	Red_ON;Blue_OFF;Green_OFF;delay_ms(500);
	Red_OFF;Blue_ON;Green_OFF;delay_ms(500);
	Red_OFF;Blue_OFF;Green_ON;delay_ms(500);
}

void user_init(void)
{
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);//��ʼ�����
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U,FUNC_GPIO14);//��ʼ������
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U,FUNC_GPIO15);//��ʼ���̵�

	GPIO_OUTPUT_SET(GPIO_ID_PIN(4), 0);

	// ȡ����ֹͣ����ʱ��timer
	os_timer_disarm(&timer);
	// ��ʱ�䶨ʱ������timer������һ������timer_callback����������������ڵ��ﶨʱʱ��ʱִ��һ��
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// ����ʱ�䶨ʱ������timer��ʱ������Ϊ500���룬����������Ϊ1ʱ��timer_callback����ֻ��ִ��һ��
	os_timer_arm(&timer,500,1);//��ʱ���ص�����
}

