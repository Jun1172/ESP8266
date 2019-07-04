/******************************************************************************
 * 作者：长沙-嵌入式-小白
 * 修改时间：2017-1-14
 * 文件名: user_main.c
 * 用途：主函数入口
*******************************************************************************/
#include "driver/user_main.h"

//创建一个任务
xTaskHandle wifi_information_TaskHandle;

//定义一个时间变量
LOCAL os_timer_t timer;

unsigned char red_pwn_buf=100,blue_pwn_buf=100,green_pwn_buf=100;
unsigned char red_udp_buf=100,blue_udp_buf=100,green_udp_buf=100;
unsigned char udp_red=100,udp_blue=100,udp_green=100;
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
    }else if(red_udp_buf!=udp_red||blue_udp_buf!=udp_blue||green_udp_buf!=udp_green)
    {
    	RGB_light_set_color(100-udp_red,100-udp_blue,100-udp_green);
    	red_udp_buf=udp_red;
    	blue_udp_buf=udp_blue;
    	green_udp_buf=udp_green;
    }

	if(udp_flag==1)
	{
		if(pdata_table[0]=='R')
		{
			if(pdata_table[3]=='%')
			{
				udp_red=pdata_table[2]-48;
			}else if(pdata_table[4]=='%')
			{
				udp_red=10*(pdata_table[2]-48)+pdata_table[3]-48;
			}else if(pdata_table[5]=='%')
			{
				udp_red=100;
			}
		}else if(pdata_table[0]=='B')
		{
			if(pdata_table[3]=='%')
			{
				udp_blue=pdata_table[2]-48;
			}else if(pdata_table[4]=='%')
			{
				udp_blue=10*(pdata_table[2]-48)+pdata_table[3]-48;
			}else if(pdata_table[5]=='%')
			{
				udp_blue=100;
			}
		}else if(pdata_table[0]=='G')
		{
			if(pdata_table[3]=='%')
			{
				udp_green=pdata_table[2]-48;
			}else if(pdata_table[4]=='%')
			{
				udp_green=10*(pdata_table[2]-48)+pdata_table[3]-48;
			}else if(pdata_table[5]=='%')
			{
				udp_green=100;
			}
		}
	}
}

/********************************************
 * 函数名：wifi_information_Task
 * 函数用途：网络连接反馈任务
 * 修改时间：2017-1-14
 * 编写：小白
 ********************************************/
void wifi_information_Task(void *pvParameters)
{
	uint8 getState;
	//获取WIFI连接状态
	getState = wifi_station_get_connect_status();
	uart0_tx_SendStr("\r\n station state:");
	uart0_tx_SendNum(getState);
	uart0_tx_SendStr("\r\n");
	//读取spi_flash的ID号
	os_printf("\r\n读取spi_flash ID：%d\r\n",spi_flash_get_id());

	//这个任务我只做一次，搞完就干掉它
	vTaskDelete(NULL);
}

/********************************************
 * 函数名：Internet_connet
 * 函数用途：网络连接函数
 * 修改时间：2017-1-14
 * 编写：小白
 ********************************************/
void Internet_connet()
{
	/*
	 * 函数名称：wifi_connet
	 * 我的WIFI名称：JUN
	 * 我的WIFI密码：123456789
	 * 编写：小白
	 */
	wifi_connet("JUN","123456789");
	/*
	 * 函数名称：InterNet_InitUDP
	 * 目标主机IP地址："192.168.155.1"
	 * 目标主机端口：8266
	 * 本机监听端口：8033
	 * 编写：小白
	 */
	InterNet_InitUDP("192.168.155.1",8888,8033);

	//使用freertos创建一个wifi信息的反馈任务
	xTaskCreate(wifi_information_Task, (uint8 const *)"wifiTask", 1024, NULL,2, &wifi_information_TaskHandle);
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
	Internet_connet();//连接WIFI
	uart0_tx_SendStr("\r\n初始化完成\r\n");

	// 取消（停止）定时器timer
	os_timer_disarm(&timer);
	// 将时间定时器变量timer关联到一个函数timer_callback（），这个函数会在到达定时时间时执行一次
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// 设置时间定时器变量timer的时间周期为10毫秒，第三个参数为1时，timer_callback（）只会执行一次
	os_timer_arm(&timer,5,1);//定时器回调函数
}
