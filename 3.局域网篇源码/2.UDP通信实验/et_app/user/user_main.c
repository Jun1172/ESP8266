/******************************************************************************
 * ���ߣ���ɳ-Ƕ��ʽ-С��
 * �޸�ʱ�䣺2017-1-14
 * �ļ���: user_main.c
 * ��;�����������
*******************************************************************************/
#include "esp_common.h"  //������STM32��conf�ļ�������һЩ������ͷ�ļ�

#include "driver/delay.h"//��ʱ�����ļ�
#include "driver/gpio.h" //GPIO�����ļ�
#include "driver/uart.h" //GPIO�����ļ�

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

//���尴��
#define KEY  GPIO_INPUT_GET(GPIO_ID_PIN(0))

xTaskHandle xWIFITaskHandle;
xQueueHandle xQueueWIFI;

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

//����һ��ʱ�����
LOCAL os_timer_t timer;

char key_num=0;//������ر���

/********************************************
 * ��������timer_callback
 * ������;����ʱ���ص�����
 * �޸�ʱ�䣺2017-1-14
 * ��д��С��
 ********************************************/
void timer_callback()
{
	if(KEY==0)//�жϰ�������
	{
		while(KEY==0);//���ּ��
		key_num++;//�Լ�һ��

	}
}

/********************************************
 * ��������Check_StationState
 * ������;���������ӷ�������
 * �޸�ʱ�䣺2017-1-14
 * ��д��С��
 ********************************************/
void Check_StationState(void *pvParameters)
{
	uint8 getState;
	//��ȡWIFI����״̬
	getState = wifi_station_get_connect_status();
	uart0_tx_SendStr("\r\n station state:");
	uart0_tx_SendNum(getState);
	uart0_tx_SendStr("\r\n");
	//��ȡspi_flash��ID��
	os_printf("\r\n��ȡspi_flash ID��%d\r\n",spi_flash_get_id());

	vTaskDelete(NULL);
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
  //ʹ��UDP��ʽ��ӡ���յ�������
  InterNet_UDP_SendData("192.168.155.1", 8266,(uint8 *)pdata,(uint16)len);
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
   uart0_tx_SendStr("\r\n Send Success \r\n");
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
    os_free(pespconn->proto.tcp);
  }
  os_free(pespconn);
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

  espconn_sent(pespconn, (uint8_t *) data, os_strlen(data));
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
 * ��������udpTask
 * ������;��UDP��ʼ������
 * �޸�ʱ�䣺2017-1-14
 * ��д��С��
 ********************************************/
void udpTask()
{
	/*
	 * �������ƣ�InterNet_InitUDP
	 * Ŀ������IP��ַ��"192.168.155.1"
	 * Ŀ�������˿ڣ�8266
	 * ���������˿ڣ�8033
	 * ��д��С��
	 */
	InterNet_InitUDP("192.168.155.1",8266,8033);
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
 * �޸�ʱ�䣺2017-1-14
 * ��д��С��
 ********************************************/
void wifi_connet(void)
{
	uart0_tx_SendStr("��ʼWIFI����\r\n");
	struct station_config stationConf;                      //���ýṹ��
	bzero(&stationConf, sizeof(struct station_config));  //�������
	wifi_set_opmode(STATION_MODE);                          //��������ģʽ
	user_strcpy(stationConf.ssid,"JUN");                    //�ҵ�WIFI SSID
	user_strcpy(stationConf.password,"123456789");           //�ҵ�WIFI password
	wifi_station_disconnect();                              //�Ͽ�WIFI����
	ETS_UART_INTR_DISABLE();
	wifi_station_set_config(&stationConf);                  //����WIFI����
	ETS_UART_INTR_ENABLE();
	wifi_station_connect();                                 //����WIFI
	uart0_tx_SendStr("\r\n��������WIFI\r\n");

	udpTask();
	//ʹ��freertos����һ��wifi��Ϣ�ķ�������
	xTaskCreate(Check_StationState, (uint8 const *)"wifiTask", 1024, NULL, 1, &xWIFITaskHandle);
}

/********************************************
 * ��������user_init
 * ������;��������
 * �޸�ʱ�䣺2017-1-14
 * ��д��С��
 ********************************************/
void user_init(void)
{
	uart0_init(74880);//��ʼ�����ڣ����ò�����
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U,FUNC_GPIO0);//��ʼ������1
	uart0_tx_SendStr("\r\n��ʼ�����\r\n");

	wifi_connet();//����WIFI

	// ȡ����ֹͣ����ʱ��timer
	os_timer_disarm(&timer);
	// ��ʱ�䶨ʱ������timer������һ������timer_callback����������������ڵ��ﶨʱʱ��ʱִ��һ��
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// ����ʱ�䶨ʱ������timer��ʱ������Ϊ10���룬����������Ϊ1ʱ��timer_callback����ֻ��ִ��һ��
	os_timer_arm(&timer,10,1);//��ʱ���ص�����
}
