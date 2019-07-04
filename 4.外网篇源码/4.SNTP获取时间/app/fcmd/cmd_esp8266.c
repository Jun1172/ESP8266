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
 * 因为这几个函数无法直接放在CmdTbl中调用，所以重新封装一次
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
 * 设置station_config,将会自动连接设置的ap
 */
void ICACHE_FLASH_ATTR
set_station_config(char *ssid, char *password)
{
	// Wifi configuration
	struct station_config stationConf;

	//need not mac address
	stationConf.bssid_set = 0;
	os_memset(&stationConf, 0, sizeof(struct station_config));

	//Set ap settings, 设置要连接的ap
	os_strncpy(stationConf.ssid, ssid, 31);
	os_strncpy(stationConf.password, password, 63);
	
	wifi_station_set_config(&stationConf);
}
/*
 * softap设置测试
 * @ssid : softap名字
 * @password: softap密码
 * @channel: wifi信道:1-12
 * @authmode: 验证方式, 参考AUTH_MODE
 * @ssid_hidden: softap是否隐藏
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


/*****************************************************************************
 * sntp(简单网络时间协议)测试
 * now sdk support SNTP_MAX_SERVERS = 3
 * 202.120.2.101 (上海交通大学网络中心NTP服务器地址）
 * 210.72.145.44 (国家授时中心服务器)
 */
void ICACHE_FLASH_ATTR
get_real_time(void)
{
	//查询当前距离基准时间（1970.01.01 00 ：00：00 GMT + 8）的时间戳，单位：秒
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


