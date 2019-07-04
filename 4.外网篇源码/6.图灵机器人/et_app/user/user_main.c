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
#include "json/cJSON.h"

uint8_t dis_flag = 1;

#define STA_SIZE		(16)

#define BUFFER_SIZE		(1024)
xTaskHandle xWIFITaskHandle;

typedef enum{
teClient,
teServer
}teType;

typedef struct
{
  BOOL linkEn;
  BOOL teToff;
  uint8_t linkId;
  teType teType;
  uint8_t repeaTime;
  struct espconn *pCon;
} espConnectionType;

#define  at_linkConType    espConnectionType

espConnectionType user_contype;
static ip_addr_t host_ip;
char data[1024] = { 0 };

//定义一个时间变量
LOCAL os_timer_t timer;

char key_num=0;//按键相关变量

/********************************************
 * 函数名：Check_StationState
 * 函数用途：网络连接反馈任务
 * 修改时间：2017-1-14
 * 编写：小白
 ********************************************/
void Check_StationState(void *pvParameters)
{
	uint8 getState;
	//获取WIFI连接状态
	getState = wifi_station_get_connect_status();
	uart0_tx_SendStr("\r\n station state:");
	uart0_tx_SendNum(getState);
	uart0_tx_SendStr("\r\n");
	//读取spi_flash的ID号
	os_printf("\r\n读取spi_flash ID：%d\r\n",spi_flash_get_id());

	vTaskDelete(NULL);
}

/********************************************
 * 函数名：InterNet_UDP_SendData
 * 函数用途：UDP发送函数
 * 修改时间：2017-1-14
 * 编写：小白
 ********************************************/
void ICACHE_FLASH_ATTR
InterNet_UDP_SendData(char * Remote_IP, int32_t Remote_port,uint8 *buf,uint16 len)
{
  uint32_t ip = 0;
  //绑定目标主机IP
  ip = ipaddr_addr(Remote_IP);
  //绑定目标主机端口
  user_contype.pCon->proto.udp->remote_port = Remote_port;
  memcpy(user_contype.pCon->proto.udp->remote_ip, &ip, 4);
  //发送数据
  espconn_sent(user_contype.pCon,buf,len);
}

/********************************************
 * 函数名：InterNet_TCP_SendData
 * 函数用途：TCP发送函数
 * 修改时间：2017-1-14
 * 编写：小白
 ********************************************/
void ICACHE_FLASH_ATTR
InterNet_TCP_SendData(uint8 *buf,uint16 len)
{
  espconn_sent(user_contype.pCon,buf,len);
}

/**********************************************
  * @brief  接收回调函数
  * @param  arg: contain the ip link information
  * @param  pdata: received data
  * @param  len: the lenght of received data
  * @retval None
  * 编写：小白
  *********************************************/
unsigned char udp_flag=0;
int8_t len_num,pdata_table[100];
static void ICACHE_FLASH_ATTR
InterNet_Receive(void *arg, char *pdata, unsigned short len)
{
	//串口返回UDP接收到的数据
	  uart0_tx_SendStr("\r\nReceive Data:\r\n");
	  uart0_tx_SendStr(pdata);
	  uart0_tx_SendStr("\r\n");

	len_num=len;
	for(;len_num>0;len_num--)
	{
		pdata_table[len-len_num]=*pdata++;
		udp_flag=1;
	}
}

/**********************************************
  * @brief  发送成功的回调函数
  * @param  arg: contain the ip link information
  * @retval None
  * 编写：小白
  *********************************************/
static void ICACHE_FLASH_ATTR
InterNet_Send_Cb(void *arg)
{
   uart0_tx_SendStr("\r\n Send Success \r\n");
}

/**********************************************
  * @brief  断开连接成功后的回调函数
  * @param  arg: contain the ip link information
  * @retval None
  * 编写：小白
  *********************************************/
static void ICACHE_FLASH_ATTR
InterNet_Disconnect_Cb(void *arg)
{
  struct espconn *pespconn = (struct espconn *)arg;
  at_linkConType *linkTemp = (at_linkConType *)pespconn->reverse;

  if(pespconn == NULL)  {
    return;
  }
  if(pespconn->proto.tcp != NULL)  {
    os_free(pespconn->proto.tcp);
  }
  os_free(pespconn);
  linkTemp->linkEn = FALSE;
  uart0_tx_SendStr("\r\n DisConnect Success\r\n");
}


/**********************************************
  * @brief  连接成功后的回调函数
  * @param  arg: contain the ip link information
  * @retval None
  * 编写：小白
  *********************************************/
static void ICACHE_FLASH_ATTR
InterNet_Connect_Cb(void *arg) {
  struct espconn *pespconn = (struct espconn *)arg;
  at_linkConType *linkTemp = (at_linkConType *)pespconn->reverse;

  linkTemp->linkEn = TRUE;
  linkTemp->teType = teClient;
  linkTemp->repeaTime = 0;

  espconn_sent(pespconn, (uint8_t *) data, os_strlen(data));
}


/**********************************************
  * @brief  重联回调函数
  * @param  arg: contain the ip link information
  * @retval None
  * 编写：小白
  *********************************************/
static void ICACHE_FLASH_ATTR
InterNet_Reconnect_Cb(void *arg, sint8 errType) {
  struct espconn *pespconn = (struct espconn *)arg;
  at_linkConType *linkTemp = (at_linkConType *) pespconn->reverse;
  if(linkTemp->linkEn) {
    return;
  }

  if(linkTemp->teToff == TRUE) {
    linkTemp->teToff = FALSE;
    linkTemp->repeaTime = 0;
    if(pespconn->proto.tcp != NULL) {
      os_free(pespconn->proto.tcp);
    }
    os_free(pespconn);
    linkTemp->linkEn = false;
  }   else   {
    linkTemp->repeaTime++;
    if(linkTemp->repeaTime >= 1)  {
       linkTemp->repeaTime = 0;
      if(pespconn->proto.tcp != NULL)  {
        os_free(pespconn->proto.tcp);
      }
      os_free(pespconn);
      linkTemp->linkEn = false;
      return;
    }

    pespconn->proto.tcp->local_port = espconn_port();
    espconn_connect(pespconn);
  }
}

/**********************************************
  * @brief  dns查询回调函数
  * @param  name -- pointer to the name that was looked up.
  * @param  ipaddr -- pointer to an ip_addr_t containing the IP address of
  *         the hostname, or NULL if the name could not be found (or on any
  *         other error).
  * @param  callback_arg -- a user-specified callback argument passed to
  *         dns_gethostbyname
  * @retval None
  * 编写：小白
  *********************************************/
LOCAL void ICACHE_FLASH_ATTR
InterNet_Dns_Cb(const char *name, ip_addr_t *ipaddr, void *arg) {
  struct espconn *pespconn = (struct espconn *) arg;
  at_linkConType *linkTemp = (at_linkConType *) pespconn->reverse;
  if(ipaddr == NULL)  {
    linkTemp->linkEn = FALSE;
    return;
  }

  if(host_ip.addr == 0 && ipaddr->addr != 0) {
    if(pespconn->type == ESPCONN_TCP)  {
    	memcpy(pespconn->proto.tcp->remote_ip, &ipaddr->addr, 4);
      espconn_connect(pespconn);
    } else {
    	memcpy(pespconn->proto.udp->remote_ip, &ipaddr->addr, 4);
      espconn_connect(pespconn);
    }
  }
}

/***********************************************
  * @brief  配置TCP连接
  * @param  ipAddress -- TCP Server's IP Address
  * @param  port      -- TCP Server Listen prot
  * @retval None
  * 编写：小白
  **********************************************/
void ICACHE_FLASH_ATTR
InterNet_InitTCP(char * ipAddress, int32_t port)
{
	char ipTemp[128];
	uint32_t ip = 0;
	user_contype.pCon = (struct espconn *)zalloc(sizeof(struct espconn));
	user_contype.pCon->state = ESPCONN_NONE;
	user_contype.linkId = 0;
	ip = ipaddr_addr(ipAddress);
	user_contype.pCon->type = ESPCONN_TCP;

	user_contype.pCon->proto.tcp = (esp_tcp *)zalloc(sizeof(esp_tcp));
	user_contype.pCon->proto.tcp->local_port = espconn_port();
	user_contype.pCon->proto.tcp->remote_port = port;
	memcpy(user_contype.pCon->proto.tcp->remote_ip, &ip, 4);
	user_contype.pCon->reverse = &user_contype;

	espconn_regist_recvcb(user_contype.pCon, InterNet_Receive);////////
	espconn_regist_sentcb(user_contype.pCon, InterNet_Send_Cb);///////
	espconn_regist_connectcb(user_contype.pCon, InterNet_Connect_Cb);
	espconn_regist_disconcb(user_contype.pCon, InterNet_Disconnect_Cb);
	espconn_regist_reconcb(user_contype.pCon, InterNet_Reconnect_Cb);
	if((ip == 0xffffffff) && (memcmp(ipTemp,"255.255.255.255",16) != 0))
	{
      espconn_gethostbyname(user_contype.pCon, ipAddress, &host_ip, InterNet_Dns_Cb);
	}else{
	  espconn_connect(user_contype.pCon);
	}
}
/**********************************************
  * @brief  配置udp连接
  * @参数：  ipAddress -- TCP Server's IP Address
  * @param  port      -- TCP Server Listen prot
  * @retval None
  * 编写：小白
 *********************************************/
void ICACHE_FLASH_ATTR
InterNet_InitUDP(char * Remote_IP, int32_t Remote_port,uint32_t Local_port)
{
	char ipTemp[128];
	uint32_t ip = 0;
	//为创建的UDP分配一个空间
	user_contype.pCon = (struct espconn *)zalloc(sizeof(struct espconn));
	user_contype.pCon->state = ESPCONN_NONE;
	user_contype.linkId = 0;
	ip = ipaddr_addr(Remote_IP);
	//设置为UDP通讯方式
	user_contype.pCon->type = ESPCONN_UDP;

	//清空相关的网络连接数据
	user_contype.pCon->proto.udp = (esp_udp *)zalloc(sizeof(esp_udp));
	//重置所有数据
	user_contype.pCon->proto.udp->local_port = Local_port;
	user_contype.pCon->proto.udp->remote_port = Remote_port;
	memcpy(user_contype.pCon->proto.udp->remote_ip, &ip, 4);
	user_contype.pCon->reverse = &user_contype;
	user_contype.linkEn = TRUE;
	user_contype.teType = teClient;
	//打印端口信息
	uart0_tx_SendStr("\r\nLocal_port:");
	uart0_tx_SendNum(user_contype.pCon->proto.udp->local_port);

	//创建一个接受函数
	espconn_regist_recvcb(user_contype.pCon, InterNet_Receive);
	//创建一个发送函数
	espconn_regist_sentcb(user_contype.pCon, InterNet_Send_Cb);

	//一切正常创建一个新的UDP传输
	if((ip == 0xffffffff) && (memcpy(ipTemp,"255.255.255.255",16) != 0))  {
	espconn_gethostbyname(user_contype.pCon, ipTemp, &host_ip, InterNet_Dns_Cb);
	} else {
	  espconn_create(user_contype.pCon);
	}
}

/********************************************
 * 函数名：udpTask
 * 函数用途：UDP初始化函数
 * 修改时间：2017-1-14
 * 编写：小白
 ********************************************/
void udpTask()
{
	/*
	 * 函数名称：InterNet_InitUDP
	 * 目标主机IP地址："192.168.155.1"
	 * 目标主机端口：8266
	 * 本机监听端口：8033
	 * 编写：小白
	 */
	InterNet_InitUDP("192.168.155.1",8266,8033);
}
/********************************************
 * 函数名：user_strcpy
 * 函数用途：转移指针数据
 * 修改时间：2017-1-14
 * 编写：小白
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
 * 编写：小白
 ********************************************/
void wifi_connet(void)
{
	uart0_tx_SendStr("开始WIFI设置\r\n");
	struct station_config stationConf;                      //调用结构体
	bzero(&stationConf, sizeof(struct station_config));  //清空数据
	wifi_set_opmode(STATION_MODE);                          //设置连接模式
	user_strcpy(stationConf.ssid,"chenjun");                    //我的WIFI SSID
	user_strcpy(stationConf.password,"cj15343093441");           //我的WIFI password
	wifi_station_disconnect();                              //断开WIFI连接
	ETS_UART_INTR_DISABLE();
	wifi_station_set_config(&stationConf);                  //设置WIFI连接
	ETS_UART_INTR_ENABLE();
	wifi_station_connect();                                 //连接WIFI
	uart0_tx_SendStr("\r\n尝试连接WIFI\r\n");

	udpTask();
	//使用freertos创建一个wifi信息的反馈任务
	xTaskCreate(Check_StationState, (uint8 const *)"wifiTask", 1024, NULL, 1, &xWIFITaskHandle);
}


//读取网页数据，反正我也看不明白，自己理解
int32_t vGetRawData(int8_t *pbuf,const int32_t maxsiz)
{
	int32_t temp=0;
	int32_t recbytes=0;
	int32_t buf_index=0;
	int8_t *c=NULL;
	int8_t buffer[128];
	int8_t len,buff_len;
	struct sockaddr_in remote_ip;
	const int32_t server_port=80;

	//合成一个可用的URL
	int8_t request[256]="GET /openapi/api?key=c32ccaa805b6441be76bc18074f12e51&info=";
	strcat(request,pdata_table);
	memset(pdata_table,0,sizeof(buffer));
	int8_t HTTP_str[100] = " HTTP/1.1\nHOST: www.tuling123.com\nCache-Control: max-age=0\n\n\n";
	strcat(request,HTTP_str);
	//将合成的URL打印出来
	printf("%s\n", request);

	//使用的API网址
	const static int8_t *serverurl="www.tuling123.com";

	const struct hostent *pURL=gethostbyname(serverurl);
	const int socketfd=socket(AF_INET, SOCK_STREAM, 0);

	//开始连接网址
	if (socketfd<0)
	{
		printf("C > socket fail!\n");
		close(socketfd);
		return -1;
	}
	bzero(&remote_ip, sizeof(struct sockaddr_in));
	remote_ip.sin_family = AF_INET;
	remote_ip.sin_addr.s_addr = *((unsigned long*)pURL->h_addr_list[0]);
	remote_ip.sin_port = htons(server_port);
	if (0 != connect(socketfd, (struct sockaddr *)(&remote_ip), sizeof(struct sockaddr)))
	{
		close(socketfd);
		printf("C > connect fail!\n");
		vTaskDelay(4000 / portTICK_RATE_MS);
		return -1;
	}

	//向网址发送数据
	if (write(socketfd, request, strlen(request) + 1) < 0)
	{
		close(socketfd);
		printf("C > send fail\n");
		return -1;
	}
	memset(buffer,0,sizeof(buffer));
	vTaskDelay(500 / portTICK_RATE_MS);

	//把数据提取出来
	while ((recbytes = read(socketfd , buffer, sizeof(buffer)-1)) > 0)
	{
		c=strstr(buffer,"{");
		if (NULL==c)
		{
			memset(buffer,0,sizeof(buffer));
			vTaskDelay(100 / portTICK_RATE_MS);
			continue;
		}
		buf_index=recbytes-(c-buffer);
		memcpy(pbuf,c,recbytes-(c-buffer));
		break;
	}
	if (recbytes<1)
	{
		close(socketfd);
		return -1;
	}
	memset(buffer,0,sizeof(buffer));
	while ((recbytes = read(socketfd , buffer, sizeof(buffer)-1)) > 0)
	{
		if (recbytes+buf_index>maxsiz)
			temp=maxsiz-buf_index;
		else
			temp=recbytes;
		//temp
		memcpy(pbuf+buf_index,buffer,temp);
		buf_index=buf_index+temp;
		if (temp<recbytes)
		{
			break;
		}
		vTaskDelay(100 / portTICK_RATE_MS);
	}
	temp=maxsiz;
	while ('}'!=pbuf[temp])
	{
		pbuf[temp]='\0';
		--temp;
	}

	//原样输出网页数据
	printf("-------------------Read-------------------\n");
	printf("%s\n", pbuf);
	printf("-------------------ReadEnd----------------\n");
	close(socketfd);
	return 0;
}

void vTaskNetwork(void *pvParameters)
{
	//创建一个指针，用来存放我收到的数据
	int8_t *pbuf=NULL;
	//申请一块内存存数据
	pbuf=(int8_t *)zalloc(BUFFER_SIZE+1);

	//判断这个内存是不是成功申请到，否则就不继续执行了
	if (NULL==pbuf)
	{
		printf("ERROR\n");
		while (1);
	}
	//内存申请好
	printf("%s Running...\n",__func__);

	//等待WIFI连接上，用死循环没关系，反正上不了网，后面也没必要执行了
	while(wifi_station_get_connect_status()!=5)
	{
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
	//WIFI连接上，创建一个UDP连接
	InterNet_InitUDP("192.168.155.1",8266,8033);
	//不断循环获取网页信息
    while (1)
	{
    	//判断是不是接收到UDP传来的标志位
    	if(udp_flag)
    	{
			//清空之前的缓存
			memset(pbuf,'\0',BUFFER_SIZE+1);
			//读取一次天气数据
			if (0!=vGetRawData(pbuf,BUFFER_SIZE))
			{
				printf("GetRawData Error\n");
				continue;
			}
			//清除标志位
			udp_flag=0;
    	}
    	//延时一下，不要跑太快了
		vTaskDelay(1000 / portTICK_RATE_MS);
    }
	free(pbuf);
}


/********************************************
 * 函数名：user_init
 * 函数用途：主函数
 * 修改时间：2017-1-14
 * 编写：小白
 ********************************************/
void user_init(void)
{
	uart0_init(74880);//初始化串口，设置波特率
	uart0_tx_SendStr("\r\n初始化完成\r\n");

	wifi_connet();//连接WIFI
	xTaskCreate(vTaskNetwork, "tskNetwork", 2048, NULL, 2, NULL);
}
