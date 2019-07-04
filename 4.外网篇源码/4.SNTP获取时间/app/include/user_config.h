#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define ICACHE_STORE_TYPEDEF_ATTR __attribute__((aligned(4),packed))
#define ICACHE_STORE_ATTR __attribute__((aligned(4)))
#define ICACHE_RAM_ATTR

#define USE_OPTIMIZE_PRINTF

#define SOFTAP_CHANGESSID 	1
#define SOFTAP_PREFIX 		"ROBIN"
#define WDT_CLEAR()			WRITE_PERI_REG( 0x60000914, 0x73 )

#endif
