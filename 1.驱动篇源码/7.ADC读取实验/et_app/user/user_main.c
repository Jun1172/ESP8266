/******************************************************************************
 * 作者：长沙-嵌入式-小白
 * 修改时间：2017-1-14
 * 文件名: user_main.c
 * 用途：主函数入口
*******************************************************************************/
#include "esp_common.h"  //类似于STM32的conf文件，调用一些其他的头文件

//定义按键
#define KEY  GPIO_INPUT_GET(GPIO_ID_PIN(0))

//定义一个时间变量
LOCAL os_timer_t timer;

char key_num=0;//按键相关变量

/********************************************
 * 函数名：timer_callback
 * 函数用途：定时器回调函数
 * 修改时间：2017-1-14
 * 修改人：小白
 ********************************************/
void timer_callback()
{
	uint8 getState;
	if(KEY==0)//判断按键按下
	{
		while(KEY==0);//松手检测
		//返回ADC测量的数值
		os_printf("ADC检测数值=%u\r\n", system_adc_read());
		//计算出ADC数值对应的电压值
		os_printf("实际电压=0.");
		os_printf("%u\r\n", system_adc_read()*100/1024);
	}
}

/********************************************
 * 函数名：user_init
 * 函数用途：主函数
 * 修改时间：2017-1-14
 * 修改人：小白
 ********************************************/
void user_init(void)
{
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U,FUNC_GPIO0);//初始化按键1
	uart0_tx_SendStr("\r\n初始化完成\r\n");

	// 取消（停止）定时器timer
	os_timer_disarm(&timer);
	// 将时间定时器变量timer关联到一个函数timer_callback（），这个函数会在到达定时时间时执行一次
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// 设置时间定时器变量timer的时间周期为10毫秒，第三个参数为1时，timer_callback（）只会执行一次
	os_timer_arm(&timer,10,1);//定时器回调函数
}
