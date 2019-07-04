#include "esp_common.h"  //类似于STM32的conf文件，调用一些其他的头文件

#include "driver/gpio.h" //GPIO依赖文件
#include "driver/uart.h" //串口收发头文件

//定义按键
#define KEY  GPIO_INPUT_GET(GPIO_ID_PIN(0))

//定义一个定时器变量timer
LOCAL os_timer_t timer;


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
		uart0_tx_SendStr("欢迎阅读esp");
		uart0_tx_SendNum(8266);
		uart0_tx_SendStr("学习笔记\r\n");
		uart0_tx_SendStr("我是小白\r\n");

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
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U,FUNC_GPIO0);//初始化按键1

	//初始化串口0，设置波特率为74880
	uart0_init(74880);

	// 取消（停止）定时器timer
	os_timer_disarm(&timer);
	// 将时间定时器变量timer关联到一个函数timer_callback（），这个函数会在到达定时时间时执行一次
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// 设置时间定时器变量timer的时间周期为10毫秒，第三个参数为1时，timer_callback（）只会执行一次
	os_timer_arm(&timer,10,1);//定时器回调函数
}

