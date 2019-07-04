/******************************************************************************
 * 作者：长沙-嵌入式-小白
 * 修改时间：2017-1-14
 * 文件名: user_main.c
 * 用途：主函数入口
*******************************************************************************/
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

//定义一个时间定时器变量timer
LOCAL os_timer_t timer;

//创建一个回调函数
void timer_callback()
{
	//RGB状态改变
	Red_ON;Blue_OFF;Green_OFF;delay_ms(500);
	Red_OFF;Blue_ON;Green_OFF;delay_ms(500);
	Red_OFF;Blue_OFF;Green_ON;delay_ms(500);
}

void user_init(void)
{
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);//初始化红灯
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U,FUNC_GPIO14);//初始化蓝灯
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U,FUNC_GPIO15);//初始化绿灯

	GPIO_OUTPUT_SET(GPIO_ID_PIN(4), 0);

	// 取消（停止）定时器timer
	os_timer_disarm(&timer);
	// 将时间定时器变量timer关联到一个函数timer_callback（），这个函数会在到达定时时间时执行一次
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// 设置时间定时器变量timer的时间周期为500毫秒，第三个参数为1时，timer_callback（）只会执行一次
	os_timer_arm(&timer,500,1);//定时器回调函数
}

