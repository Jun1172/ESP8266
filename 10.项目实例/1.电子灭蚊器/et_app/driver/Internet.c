/******************************************************************************
 * 作者：长沙-嵌入式-小白
 * 修改时间：2017-1-14
 * 文件名: Internet.c
 * 用途：关于网络的相关操作函数
*******************************************************************************/
#include "driver/Internet.h"
#include "driver/uart.h"
#include "espconn.h"
#include "c_types.h"
#include "driver/gpio.h"

/*开关继电器的操作*/
#define my_gpio  GPIO_INPUT_GET(GPIO_ID_PIN(12))
#define MY_ON    GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1)
#define MY_OFF   GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0)

espConnectionType user_contype;
static ip_addr_t host_ip;
char data[1024] = { 0 };

/**********************************************
  * @brief  接收回调函数
  * @param  arg: contain the ip link information
  * @param  pdata: received data
  * @param  len: the lenght of received data
  * @retval None
  * 编写：小白
  *********************************************/
static void ICACHE_FLASH_ATTR
InterNet_Receive(void *arg, char *pdata, unsigned short len)
{
	char *my_OFF="OFF";
	char *my_ON="ON";
	char *my_get="GET";

	if(strcmp(pdata,my_OFF)==0)
	{
		MY_OFF;
		InterNet_UDP_SendData("192.168.31.199", 8266,"开关已打开",10);
	}
	else if(strcmp(pdata,my_ON)==0)
	{
		MY_ON;
		InterNet_UDP_SendData("192.168.31.199", 8266,"开关已关闭",10);
	}
	else if(strcmp(pdata,my_get)==0)
	{
		if(my_gpio==0)
			InterNet_UDP_SendData("192.168.31.199", 8266,"设备正在工作",10);
		else if(my_gpio==1)
			InterNet_UDP_SendData("192.168.31.199", 8266,"设备已经关闭",10);
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


