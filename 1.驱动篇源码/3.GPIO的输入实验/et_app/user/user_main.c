#include "esp_common.h"  //类似于STM32的conf文件，调用一些其他的头文件

#include "driver/delay.h"//延时函数文件
#include "driver/gpio.h" //GPIO依赖文件

//定义红灯高低电平状态
#define Red_ON    GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1)
#define Red_OFF   GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0)

//定义蓝灯高低电平状态
#define Blue_ON    GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1)
#define Blue_OFF   GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 0)

//定义绿灯高低电平状态
#define Green_ON    GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1)
#define Green_OFF   GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 0)

//定义按键
#define KEY  GPIO_INPUT_GET(GPIO_ID_PIN(0))

//定义一个定时器变量timer
LOCAL os_timer_t timer;

char key_num=0;//按键相关变量

/**********************************************
 * 函数名      : timer_callback
 * 函数作用  : 定时器回调函数
 * 参数          : none
 * 返回值      : none
**********************************************/
void timer_callback()
{
	if(KEY==0)//判断按键按下
	{
		while(KEY==0);//松手检测
		key_num++;//自加一次
		switch(key_num)
		{
			//RGB状态改变
		    case 1:Red_ON;Blue_OFF;Green_OFF;break;
		    case 2:Red_OFF;Blue_ON;Green_OFF;break;
		    case 3:Red_OFF;Blue_OFF;Green_ON;break;
		    default :key_num=0 ;break;
		}
	}
}

/******************************************************************************
 * 函数名      : user_init
 * 函数作用  : entry of user application, init user function here
 * 参数          : none
 * 返回值      : none
*******************************************************************************/
void user_init(void)
{
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);//初始化红灯
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U,FUNC_GPIO14);//初始化蓝灯
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U,FUNC_GPIO15);//初始化绿灯

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U,FUNC_GPIO0);//初始化按键1

	// 取消（停止）定时器timer
	os_timer_disarm(&timer);
	// 将时间定时器变量timer关联到一个函数timer_callback（），这个函数会在到达定时时间时执行一次
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// 设置时间定时器变量timer的时间周期为10毫秒，第三个参数为1时，timer_callback（）只会执行一次
	os_timer_arm(&timer,10,1);//定时器回调函数
}

