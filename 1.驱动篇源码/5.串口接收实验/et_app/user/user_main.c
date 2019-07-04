/******************************************************************************
 * 作者：长沙-嵌入式-小白
 * 修改时间：2017-1-14
 * 文件名: user_main.c
 * 用途：主函数入口
*******************************************************************************/
#include "esp_common.h"  //类似于STM32的conf文件，调用一些其他的头文件

#include "driver/uart.h" //GPIO依赖文件


//定义一个时间定时器变量timer
LOCAL os_timer_t timer;

char key_num=0;//按键相关变量

//创建一个回调函数
void timer_callback()
{

}

void user_init(void)
{
	uart0_init(74880);//初始化串口，设置波特率

	uart0_tx_SendStr("初始化完成\r\n");
	// 取消（停止）定时器timer
	os_timer_disarm(&timer);
	// 将时间定时器变量timer关联到一个函数timer_callback（），这个函数会在到达定时时间时执行一次
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// 设置时间定时器变量timer的时间周期为10毫秒，第三个参数为1时，timer_callback（）只会执行一次
	os_timer_arm(&timer,10,1);//定时器回调函数
}

