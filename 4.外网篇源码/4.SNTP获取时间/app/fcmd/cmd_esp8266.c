#include "osapi.h"
#include "at_custom.h"
#include "user_interface.h"

#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "espconn.h"

#include "cmd_esp8266.h"
#include "sntp.h"


/*****************************************************************************
 * ��Ϊ�⼸�������޷�ֱ�ӷ���CmdTbl�е��ã��������·�װһ��
 */
void ICACHE_FLASH_ATTR *_malloc(uint32_t size)
{
	return (void *)os_malloc(size);
}
void ICACHE_FLASH_ATTR _free(void *p)
{
	os_free(p);
}
void ICACHE_FLASH_ATTR *_memset(void *s, int c, uint32_t n)
{
	os_memset(s, c, n);
	return (void *)s;
}
void ICACHE_FLASH_ATTR delay_us(uint32_t us)
{
	os_delay_us(us);
}
void ICACHE_FLASH_ATTR get_ip_mac(void)
{
	char hwaddr[6];
	struct ip_info ipconfig;

	wifi_get_ip_info(SOFTAP_IF, &ipconfig);
	wifi_get_macaddr(SOFTAP_IF, hwaddr);
	os_printf("soft-AP:" MACSTR " " IPSTR, MAC2STR(hwaddr), IP2STR(&ipconfig.ip));

	wifi_get_ip_info(STATION_IF, &ipconfig);
	wifi_get_macaddr(STATION_IF, hwaddr);
	os_printf("\nstation:" MACSTR " " IPSTR "\n", MAC2STR(hwaddr), IP2STR(&ipconfig.ip));
}




/*
 * ����station_config,�����Զ��������õ�ap
 */
void ICACHE_FLASH_ATTR
set_station_config(char *ssid, char *password)
{
	// Wifi configuration
	struct station_config stationConf;

	//need not mac address
	stationConf.bssid_set = 0;
	os_memset(&stationConf, 0, sizeof(struct station_config));

	//Set ap settings, ����Ҫ���ӵ�ap
	os_strncpy(stationConf.ssid, ssid, 31);
	os_strncpy(stationConf.password, password, 63);
	
	wifi_station_set_config(&stationConf);
}
/*
 * softap���ò���
 * @ssid : softap����
 * @password: softap����
 * @channel: wifi�ŵ�:1-12
 * @authmode: ��֤��ʽ, �ο�AUTH_MODE
 * @ssid_hidden: softap�Ƿ�����
 */
void ICACHE_FLASH_ATTR
set_softap_config(char *ssid, char *password, uint8 channel,
                  uint8 authmode, uint8 ssid_hidden)
{
	struct softap_config softapConf;

	wifi_softap_get_config(&softapConf);

	os_strncpy(softapConf.ssid, ssid, 31);
	os_strncpy(softapConf.password, password, 63);
	softapConf.ssid_len = strlen(softapConf.ssid);
	softapConf.channel = channel;
	softapConf.authmode = authmode;
	softapConf.ssid_hidden = ssid_hidden;

	wifi_softap_set_config(&softapConf);
}




/*****************************************************************************************
 *  dns����
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
	//ʵ��Ӧ��ʱ����ÿ��1sִ��һ��, dns_found�ɹ�֮��ֹͣ��ʱ������
	//dns_foundʧ�ܵĻ�,�л�ap���ٴν���dns����
	espconn_gethostbyname(&dns_espconn, hostname, &iot_server_ip, dns_found);
}


/*****************************************************************************
 * sntp(������ʱ��Э��)����
 * now sdk support SNTP_MAX_SERVERS = 3
 * 202.120.2.101 (�Ϻ���ͨ��ѧ��������NTP��������ַ��
 * 210.72.145.44 (������ʱ���ķ�����)
 */
void ICACHE_FLASH_ATTR
get_real_time(void)
{
	//��ѯ��ǰ�����׼ʱ�䣨1970.01.01 00 ��00��00 GMT + 8����ʱ�������λ����
	uint32 t = sntp_get_current_timestamp();
	os_printf("sntp:%s\n", sntp_get_real_time(t));
}

void ICACHE_FLASH_ATTR
sntp_init_test(void)
{
	ip_addr_t ip;

	sntp_init();

	sntp_set_timezone(8);

	IP4_ADDR(&ip, 202, 120, 2, 101);
	sntp_setserver(0, &ip);

	IP4_ADDR(&ip, 210, 72, 145, 44);
	sntp_setserver(1, &ip);
}


