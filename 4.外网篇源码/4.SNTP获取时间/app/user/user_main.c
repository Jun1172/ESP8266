/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2015/1/23, v1.0 create this file.
*******************************************************************************/

#include "osapi.h"
#include "at_custom.h"
#include "user_interface.h"

#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "espconn.h"

#include "uart.h"

void init_done_cb(void)
{
	char buf[64] = {0};

	os_sprintf(buf, "compile time:%s %s", __DATE__, __TIME__);

	os_printf("uart init ok, %s\n", buf);
}
void user_rf_pre_init(void)
{
}
void user_init(void)
{
	uart_init(BIT_RATE_115200, BIT_RATE_115200);

	system_init_done_cb(init_done_cb);
}

