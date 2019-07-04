/******************************************************************************
 * ���ߣ���ɳ-Ƕ��ʽ-С��
 * �޸�ʱ�䣺2017-1-14
 * �ļ���: Internet.c
 * ��;�������������ز�������
*******************************************************************************/
#include "driver/Internet.h"
#include "driver/uart.h"

espConnectionType user_contype;
static ip_addr_t host_ip;
char data[1024] = { 0 };

/**********************************************
  * @brief  ���ջص�����
  * @param  arg: contain the ip link information
  * @param  pdata: received data
  * @param  len: the lenght of received data
  * @retval None
  * ��д��С��
  *********************************************/
static void ICACHE_FLASH_ATTR
InterNet_Receive(void *arg, char *pdata, unsigned short len)
{
	//���ڷ���UDP���յ�������
	  uart0_tx_SendStr("\r\nReceive Data:\r\n");
	  uart0_tx_SendStr(pdata);
	  uart0_tx_SendStr("\r\n");
	uart0_tx_SendStr("\r\n ���� \r\n");

	InterNet_TCP_SendData((uint8 *)pdata,(uint16)len);
}

/**********************************************
  * @brief  ���ͳɹ��Ļص�����
  * @param  arg: contain the ip link information
  * @retval None
  * ��д��С��
  *********************************************/
static void ICACHE_FLASH_ATTR
InterNet_Send_Cb(void *arg)
{
//   uart0_tx_SendStr("\r\n Send Success \r\n");
}

/********************************************
 * ��������InterNet_UDP_SendData
 * ������;��UDP���ͺ���
 * �޸�ʱ�䣺2017-1-14
 * ��д��С��
 ********************************************/
void ICACHE_FLASH_ATTR
InterNet_UDP_SendData(char * Remote_IP, int32_t Remote_port,uint8 *buf,uint16 len)
{
  uint32_t ip = 0;
  //��Ŀ������IP
  ip = ipaddr_addr(Remote_IP);
  //��Ŀ�������˿�
  user_contype.pCon->proto.udp->remote_port = Remote_port;
  memcpy(user_contype.pCon->proto.udp->remote_ip, &ip, 4);
  //��������
  espconn_sent(user_contype.pCon,buf,len);
}

/********************************************
 * ��������InterNet_TCP_SendData
 * ������;��TCP���ͺ���
 * �޸�ʱ�䣺2017-1-14
 * ��д��С��
 ********************************************/
void ICACHE_FLASH_ATTR
InterNet_TCP_SendData(uint8 *buf,uint16 len)
{
  espconn_sent(user_contype.pCon,buf,len);

}

/**********************************************
  * @brief  �Ͽ����ӳɹ���Ļص�����
  * @param  arg: contain the ip link information
  * @retval None
  * ��д��С��
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
    free(pespconn->proto.tcp);
  }
  free(pespconn);
  linkTemp->linkEn = FALSE;
  uart0_tx_SendStr("\r\n DisConnect Success\r\n");
}
/**********************************************
  * @brief  ���ӳɹ���Ļص�����
  * @param  arg: contain the ip link information
  * @retval None
  * ��д��С��
  *********************************************/
static void ICACHE_FLASH_ATTR
InterNet_Connect_Cb(void *arg) {
  struct espconn *pespconn = (struct espconn *)arg;
  at_linkConType *linkTemp = (at_linkConType *)pespconn->reverse;

  linkTemp->linkEn = TRUE;
  linkTemp->teType = teClient;
  linkTemp->repeaTime = 0;

  espconn_sent(pespconn, (uint8_t *) data, strlen(data));
}


/**********************************************
  * @brief  �����ص�����
  * @param  arg: contain the ip link information
  * @retval None
  * ��д��С��
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
      free(pespconn->proto.tcp);
    }
    free(pespconn);
    linkTemp->linkEn = false;
  }   else   {
    linkTemp->repeaTime++;
    if(linkTemp->repeaTime >= 1)  {
       linkTemp->repeaTime = 0;
      if(pespconn->proto.tcp != NULL)  {
        free(pespconn->proto.tcp);
      }
      free(pespconn);
      linkTemp->linkEn = false;
      return;
    }

    pespconn->proto.tcp->local_port = espconn_port();
    espconn_connect(pespconn);
  }
}

/**********************************************
  * @brief  dns��ѯ�ص�����
  * @param  name -- pointer to the name that was looked up.
  * @param  ipaddr -- pointer to an ip_addr_t containing the IP address of
  *         the hostname, or NULL if the name could not be found (or on any
  *         other error).
  * @param  callback_arg -- a user-specified callback argument passed to
  *         dns_gethostbyname
  * @retval None
  * ��д��С��
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
  * @brief  ����TCP����
  * @param  ipAddress -- TCP Server's IP Address
  * @param  port      -- TCP Server Listen prot
  * @retval None
  * ��д��С��
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

	if((ip == 0xffffffff) && (memcmp(ipTemp,"255.255.255.255",16) != 0))
	{
      espconn_gethostbyname(user_contype.pCon, ipAddress, &host_ip, InterNet_Dns_Cb);
	}else{
	  espconn_connect(user_contype.pCon);
	}
}
/**********************************************
  * @brief  ����udp����
  * @������  ipAddress -- TCP Server's IP Address
  * @param  port      -- TCP Server Listen prot
  * @retval None
  * ��д��С��
 *********************************************/
void ICACHE_FLASH_ATTR
InterNet_InitUDP(char * Remote_IP, int32_t Remote_port,uint32_t Local_port)
{
	char ipTemp[128];
	uint32_t ip = 0;
	//Ϊ������UDP����һ���ռ�
	user_contype.pCon = (struct espconn *)zalloc(sizeof(struct espconn));
	user_contype.pCon->state = ESPCONN_NONE;
	user_contype.linkId = 0;
	ip = ipaddr_addr(Remote_IP);
	//����ΪUDPͨѶ��ʽ
	user_contype.pCon->type = ESPCONN_UDP;

	//�����ص�������������
	user_contype.pCon->proto.udp = (esp_udp *)zalloc(sizeof(esp_udp));
	//������������
	user_contype.pCon->proto.udp->local_port = Local_port;
	user_contype.pCon->proto.udp->remote_port = Remote_port;
	memcpy(user_contype.pCon->proto.udp->remote_ip, &ip, 4);
	user_contype.pCon->reverse = &user_contype;
	user_contype.linkEn = TRUE;
	user_contype.teType = teClient;
	//��ӡ�˿���Ϣ
	uart0_tx_SendStr("\r\nLocal_port:");
	uart0_tx_SendNum(user_contype.pCon->proto.udp->local_port);

	//����һ�����ܺ���
	espconn_regist_recvcb(user_contype.pCon, InterNet_Receive);
	//����һ�����ͺ���
	espconn_regist_sentcb(user_contype.pCon, InterNet_Send_Cb);

	//һ����������һ���µ�UDP����
	if((ip == 0xffffffff) && (memcpy(ipTemp,"255.255.255.255",16) != 0))  {
	espconn_gethostbyname(user_contype.pCon, ipTemp, &host_ip, InterNet_Dns_Cb);
	} else {
	  espconn_create(user_contype.pCon);
	}
}

/********************************************
 * ��������user_strcpy
 * ������;��ת��ָ������
 * �޸�ʱ�䣺2017-1-14
 * ��д��С��
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
 * ��������wifi_connet
 * ������;��WIFI��������
 * ������
 * ssid:WIFI����
 * password��WIFI����
 * �޸�ʱ�䣺2017-1-14
 * ��д��С��
 ********************************************/
void wifi_connet(uint8 *ssid,uint8 *password)
{
	uart0_tx_SendStr("��ʼWIFI����\r\n");
	struct station_config stationConf;                      //���ýṹ��
	bzero(&stationConf, sizeof(struct station_config));  //�������
	wifi_set_opmode(STATION_MODE);                          //��������ģʽ
	user_strcpy(stationConf.ssid,ssid);                    //�ҵ�WIFI SSID
	user_strcpy(stationConf.password,password);           //�ҵ�WIFI password
	wifi_station_disconnect();                              //�Ͽ�WIFI����
	ETS_UART_INTR_DISABLE();
	wifi_station_set_config(&stationConf);                  //����WIFI����
	ETS_UART_INTR_ENABLE();
	wifi_station_connect();                                 //����WIFI
	uart0_tx_SendStr("\r\n��������WIFI\r\n");
}

