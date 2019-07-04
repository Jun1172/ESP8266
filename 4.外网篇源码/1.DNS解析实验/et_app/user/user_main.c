/******************************************************************************
 * 作者：长沙-嵌入式-小白
 * 修改时间：2017-1-14
 * 文件名: user_main.c
 * 用途：主函数入口
*******************************************************************************/
#include "driver/user_main.h"

//创建一个任务
xTaskHandle wifi_information_TaskHandle;

//定义按键
#define KEY  GPIO_INPUT_GET(GPIO_ID_PIN(0))

//定义一个时间变量
LOCAL os_timer_t timer;

char key_num=0;//按键相关变量
/*****************************************************************************************
 *  dns测试
 */
ip_addr_t iot_server_ip;
LOCAL struct espconn dns_espconn;

LOCAL void ICACHE_FLASH_ATTR
dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
	struct espconn *pespconn = (struct espconn *)arg;
	if (ipaddr == NULL)
	{
		os_printf("dns_found NULL\n");
	}
	else
	{
		os_printf("dns_found  %d.%d.%d.%d\n",
		          *((uint8 *)&ipaddr->addr), *((uint8 *)&ipaddr->addr + 1),
		          *((uint8 *)&ipaddr->addr + 2), *((uint8 *)&ipaddr->addr + 3));
	}
}
void ICACHE_FLASH_ATTR
dns_test(const char *hostname)
{
	//实际应用时可以每隔1s执行一次, dns_found成功之后，停止定时器即可
	//dns_found失败的话,切换ap，再次进行dns解析
	espconn_gethostbyname(&dns_espconn, hostname, &iot_server_ip, dns_found);
}

/********************************************
 * 函数名：timer_callback
 * 函数用途：定时器回调函数
 * 修改时间：2017-1-14
 * 编写：小白
 ********************************************/
void timer_callback()
{
	if(KEY==0)//判断按键按下
	{
		while(KEY==0);//松手检测
		key_num++;//自加一次
		dns_test("www.baidu.com");
		os_printf("\r\n解析一次网址\r\n");
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
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U,FUNC_GPIO0);//初始化按键1
	uart0_tx_SendStr("\r\n初始化完成\r\n");

	// 取消（停止）定时器timer
	os_timer_disarm(&timer);
	// 将时间定时器变量timer关联到一个函数timer_callback（），这个函数会在到达定时时间时执行一次
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// 设置时间定时器变量timer的时间周期为10毫秒，第三个参数为1时，timer_callback（）只会执行一次
	os_timer_arm(&timer,5,1);//定时器回调函数
}
