/******************************************************************************
 * 作者：长沙-嵌入式-小白
 * 修改时间：2017-1-14
 * 文件名: user_main.c
 * 用途：主函数入口
*******************************************************************************/
#include "esp_common.h"  //类似于STM32的conf文件，调用一些其他的头文件

#include "driver/delay.h"//延时函数文件
#include "driver/gpio.h" //GPIO依赖文件
#include "driver/uart.h" //GPIO依赖文件

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

//定义按键
#define KEY  GPIO_INPUT_GET(GPIO_ID_PIN(0))

xTaskHandle xWIFITaskHandle;

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
		    case 1://获取WIFI连接状态
		    	getState = wifi_station_get_connect_status();
		    	uart0_tx_SendStr("\r\n station state:");
		    	uart0_tx_SendNum(getState);
		    	;break;
		    case 2:
		    	uart0_tx_SendStr("\r\n");
				//读取spi_flash的ID号
				os_printf("\r\n读取spi_flash ID：%d\r\n",spi_flash_get_id());break;
		    default :key_num=0 ;break;
		}
	}
}

/********************************************
 * 函数名：Check_StationState
 * 函数用途：网络连接反馈任务
 * 修改时间：2017-1-14
 * 修改人：小白
 ********************************************/
void Check_StationState(void *pvParameters)
{
	uart0_tx_SendStr("\r\n尝试连接WIFI\r\n");
	wifi_station_connect();     //连接WIFI

	vTaskDelete(NULL);
}

/********************************************
 * 函数名：user_strcpy
 * 函数用途：转移指针数据
 * 修改时间：2017-1-14
 * 修改人：小白
 ********************************************/
void ICACHE_FLASH_ATTR
user_strcpy(uint8 *src,uint8 *des)
{
  while(*des)
  {
    *src++=*des++;
  }
}

/********************************************
 * 函数名：wifi_connet
 * 函数用途：WIFI连接设置
 * 修改时间：2017-1-14
 * 修改人：小白
 ********************************************/
void wifi_connet(void)
{
	uart0_tx_SendStr("开始WIFI设置\r\n");
	struct station_config stationConf;                      //调用结构体
	bzero(&stationConf, sizeof(struct station_config));  //清空数据
	wifi_set_opmode(STATION_MODE);                          //设置连接模式
	user_strcpy(stationConf.ssid,"JUN");                    //我的WIFI SSID
	user_strcpy(stationConf.password,"123456789");           //我的WIFI password
	ETS_UART_INTR_DISABLE();
	wifi_station_set_config(&stationConf);                  //设置WIFI连接
	ETS_UART_INTR_ENABLE();

	//使用freertos创建一个wifi信息的反馈任务
	xTaskCreate(Check_StationState, (uint8 const *)"wifiTask", 1024, NULL, 1, &xWIFITaskHandle);
}

/********************************************
 * 函数名：user_init
 * 函数用途：主函数
 * 修改时间：2017-1-14
 * 修改人：小白
 ********************************************/
void user_init(void)
{
	uint32 *read;
	uart0_init(74880);//初始化串口，设置波特率
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U,FUNC_GPIO0);//初始化按键1
	uart0_tx_SendStr("\r\n初始化完成\r\n");

	wifi_connet();//连接WIFI

	// 取消（停止）定时器timer
	os_timer_disarm(&timer);
	// 将时间定时器变量timer关联到一个函数timer_callback（），这个函数会在到达定时时间时执行一次
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// 设置时间定时器变量timer的时间周期为10毫秒，第三个参数为1时，timer_callback（）只会执行一次
	os_timer_arm(&timer,10,1);//定时器回调函数
}
