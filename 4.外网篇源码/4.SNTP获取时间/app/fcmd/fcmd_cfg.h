#ifndef _FCMD_CFG_H_
#define _FCMD_CFG_H_
/*******************************************************************************
* 用户函数命令头文件包含，函数声明
*/

#include "cmd_mem.h"
#include "osapi.h"

#include "user_interface.h"
#include "mem.h"
#include "cmd_esp8266.h"
#include "flash_api.h"


/*******************************************************************************
 * 自定义函数命令表
 */
CmdTbl_t CmdTbl[] =
{
	//系统命令, SYSTEM_CMD_NUM和系统命令个数保持一致
	"ls",  0,
	"addr", 0,
	"help", 0,


	//用户命令
	"void md(int addr, int elem_cnt, int elem_size)", (void(*)(void))md,
	"int cmp(void *addr1, void *addr2, int elem_cnt, int elem_size)", (void(*)(void))cmp,


	//system api
	"uint32 system_get_free_heap_size(void)", (void(*)(void))system_get_free_heap_size,
	"uint32 system_get_chip_id(void)", (void(*)(void))system_get_chip_id,
	"void system_restart(void)", (void(*)(void))system_restart,
	"void system_restore(void)",  (void(*)(void))system_restore,

	"void get_ip_mac(void)", (void(*)(void))get_ip_mac,

	"void *_malloc(uint32_t size)", (void(*)(void))_malloc,
	"void _free(void *p)", (void(*)(void))_free,
	"void *_memset(void *s, int c, uint32_t n)",  (void(*)(void))_memset,
	"void delay_us(uint32_t us)",	(void(*)(void))delay_us,

	// flash
	"uint32 spi_flash_get_id(void)",   (void(*)(void))spi_flash_get_id,
	"int sfmd(u32 addr, int elem_cnt, int elem_size)",	(void(*)(void))sfmd,
	"int sfmw(u32 writeaddr, u8 *pbuf, u32 num)",		(void(*)(void))sfmw,

	// wifi操作
	"void set_station_config(char *ssid, char *password)", (void(*)(void))set_station_config,
	"void set_softap_config(char *ssid, char *password, uint8 channel, uint8 authmode, uint8 ssid_hidden)",
	(void(*)(void))set_softap_config,
	"bool wifi_station_connect(void)",  (void(*)(void))wifi_station_connect,
	"bool wifi_station_disconnect(void)", (void(*)(void))wifi_station_disconnect,
	"bool wifi_set_opmode(uint8 opmode)", (void(*)(void))wifi_set_opmode,
	"uint8 wifi_station_get_connect_status(void)",  (void(*)(void))wifi_station_get_connect_status,
	"bool wifi_station_set_auto_connect(uint8 set)",  (void(*)(void))wifi_station_set_auto_connect,
	"void dns_test(const char *hostname)", (void(*)(void))dns_test,
	"void get_real_time(void)", (void(*)(void))get_real_time,
	"void sntp_init_test(void)", (void(*)(void))sntp_init_test,
};
uint8_t CmdTblSize = sizeof(CmdTbl) / sizeof(CmdTbl[0]);

#endif


