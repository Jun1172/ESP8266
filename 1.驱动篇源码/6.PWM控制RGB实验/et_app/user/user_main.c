/******************************************************************************
 * 作者：长沙-嵌入式-小白
 * 修改时间：2017-1-14
 * 文件名: user_main.c
 * 用途：主函数入口
*******************************************************************************/
#include "driver/user_main.h"


//定义一个时间变量
LOCAL os_timer_t timer;

unsigned char red_pwn_buf=100,blue_pwn_buf=100,green_pwn_buf=100;
/********************************************
 * 函数名：timer_callback
 * 函数用途：定时器回调函数
 * 修改时间：2017-1-14
 * 编写：小白
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
 * 函数名：user_init
 * 函数用途：主函数
 * 修改时间：2017-1-14
 * 编写：小白
 ********************************************/
void user_init(void)
{
	uart0_init(74880);  //初始化串口，设置波特率
	RGB_light_init();	//初始化RGB
	uart0_tx_SendStr("\r\n初始化完成\r\n");

	// 取消（停止）定时器timer
	os_timer_disarm(&timer);
	// 将时间定时器变量timer关联到一个函数timer_callback（），这个函数会在到达定时时间时执行一次
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// 设置时间定时器变量timer的时间周期为10毫秒，第三个参数为1时，timer_callback（）只会执行一次
	os_timer_arm(&timer,5,1);//定时器回调函数
}
