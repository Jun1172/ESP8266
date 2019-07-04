#ifndef _INTERNET_H
#define _INTERNET_H

#include "esp_common.h"  //类似于STM32的conf文件，调用一些其他的头文件

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

extern unsigned char udp_flag;
extern unsigned char pdata_table[10];

static void ICACHE_FLASH_ATTR
InterNet_Receive(void *arg, char *pdata, unsigned short len);
static void ICACHE_FLASH_ATTR
InterNet_Send_Cb(void *arg);

void ICACHE_FLASH_ATTR
InterNet_UDP_SendData(char * Remote_IP, int32_t Remote_port,uint8 *buf,uint16 len);
void ICACHE_FLASH_ATTR
InterNet_TCP_SendData(uint8 *buf,uint16 len);

static void ICACHE_FLASH_ATTR
InterNet_Disconnect_Cb(void *arg);
static void ICACHE_FLASH_ATTR
InterNet_Connect_Cb(void *arg) ;
static void ICACHE_FLASH_ATTR
InterNet_Reconnect_Cb(void *arg, sint8 errType);
LOCAL void ICACHE_FLASH_ATTR
InterNet_Dns_Cb(const char *name, ip_addr_t *ipaddr, void *arg);

void ICACHE_FLASH_ATTR
InterNet_InitTCP(char * ipAddress, int32_t port);
void ICACHE_FLASH_ATTR
InterNet_InitUDP(char * Remote_IP, int32_t Remote_port,uint32_t Local_port);

void wifi_connet(uint8 *ssid,uint8 *password);

#endif
