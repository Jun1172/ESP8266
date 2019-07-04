#ifndef _CMD_ESP8266_H_
#define _CMD_ESP8266_H_


void *_malloc(uint32_t size);
void _free(void *p);
void *_memset(void *s, int c, uint32_t n);
void delay_us(uint32_t us);
void get_ip_mac(void);


void set_station_config(char *ssid, char *password);
void set_softap_config(char *ssid, char *password, uint8 channel, uint8 authmode, uint8 ssid_hidden);
void dns_test(const char *hostname);
void get_real_time(void);
void sntp_init_test(void);

#endif

