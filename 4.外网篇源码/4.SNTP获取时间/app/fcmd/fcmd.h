#ifndef _FCMD_H_
#define _FCMD_H_

#include "c_types.h"
#include <string.h>
#include "osapi.h"

void fcmd_putc(uint8_t c);

#define PRINTF  os_printf
#define PUTC	fcmd_putc

#ifdef __cplusplus
extern "C" {
#endif

void fcmd_exec(uint8_t *cmd);

#ifdef __cplusplus
}
#endif

#endif


