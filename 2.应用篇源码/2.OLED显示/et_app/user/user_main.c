/******************************************************************************
 * 作者：长沙-嵌入式-小白
 * 修改时间：2017-1-14
 * 文件名: user_main.c
 * 用途：主函数入口
*******************************************************************************/
#include "esp_common.h"  //类似于STM32的conf文件，调用一些其他的头文件
#include "driver/OLED_I2C.h"

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
		key_num++;//自加一次
		switch(key_num)
		{
			//RGB状态改变
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
 * 函数名：user_init
 * 函数用途：主函数
 * 修改时间：2017-1-14
 * 修改人：小白
 ********************************************/
void user_init(void)
{
	uart0_init(74880);//初始化串口，设置波特率
	DHT11_init();
	OLED_Init();
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U,FUNC_GPIO0);//初始化按键1
	uart0_tx_SendStr("\r\n初始化完成\r\n");

	// 取消（停止）定时器timer
	os_timer_disarm(&timer);
	// 将时间定时器变量timer关联到一个函数timer_callback（），这个函数会在到达定时时间时执行一次
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// 设置时间定时器变量timer的时间周期为10毫秒，第三个参数为1时，timer_callback（）只会执行一次
	os_timer_arm(&timer,10,1);//定时器回调函数
}
