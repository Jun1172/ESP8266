/******************************************************************************
 * Copyright (C) 2014 -2016  Espressif System
 *
 * FileName: user_upgrade.c
 *
 * Description: downlaod upgrade userbin file from upgrade server
 *
 * Modification history:
 * 2015/7/3, v1.0 create this file.
*******************************************************************************/
//#include "version.h"
#include "user_config.h"

#include "esp_common.h"
#include "lwip/mem.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "upgrade.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include <stdlib.h>

/*the size cannot be bigger than below*/
#define UPGRADE_DATA_SEG_LEN 1460
#define UPGRADE_RETRY_TIMES 30
#define BUFSIZE     512
#define CRC_BLOCK_SIZE 512

xTaskHandle *pxCreatedTask=NULL;
LOCAL os_timer_t upgrade_timer;
LOCAL os_timer_t upgrade_10s;

LOCAL et_uint32 totallength = 0;
LOCAL et_uint32 sumlength = 0;
LOCAL BOOL flash_erased=0;
et_char *precv_buf=NULL;
et_uint16 start_sec;
static et_uint32 *crc_table;

static int init_crc_table(void);
static et_uint32 crc32(et_uint32 crc, et_uchar * buffer, et_uint32 size);

struct upgrade_param 
{
    et_uint32 fw_bin_addr;
    et_uint16 fw_bin_sec;
    et_uint16 fw_bin_sec_num;
    et_uint16 fw_bin_sec_earse;
    et_uint8 extra;
    et_uint8 save[4];
    et_uint8 *buffer;
};

struct upgrade_param *upgrade;

static et_int32 ICACHE_FLASH_ATTR
init_crc_table(void)
{
	et_uint32 c;
	et_uint32 i, j;

	crc_table = (et_uint32*)zalloc(256 * 4);
	if(crc_table == NULL)
	{
		os_printf("malloc crc table failed\n");
		return -1;
	}
	
	for (i = 0; i < 256; i++)
	{
		c = (et_uint32)i;
		for (j = 0; j < 8; j++) 
		{
			if (c & 1)
				c = 0xedb88320L ^ (c >> 1);
			else
				c = c >> 1;
		}
		crc_table[i] = c;
	}
	return 0;
}

static et_uint32 ICACHE_FLASH_ATTR
crc32(et_uint32 crc, et_uchar *buffer, et_uint32 size)
{
	et_uint32 i;
	for (i = 0; i < size; i++) 
	{
		crc = crc_table[(crc ^ buffer[i]) & 0xff] ^ (crc >> 8);
	}
	return crc ;
}

static et_int32 ICACHE_FLASH_ATTR
calc_img_crc(et_uint32 sumlength,et_uint32 *img_crc)
{
	et_int32 fd;
	et_int32 ret;
	et_int32 i = 0;
	et_uint8 error = 0;
	et_uchar *buf = (et_char *)zalloc(BUFSIZE);
	
	if(buf == NULL)
	{
		os_printf("malloc crc buf failed\n");
		free(crc_table);
		return -1;
	}

	et_uint crc = 0xffffffff; 

	et_uint16 sec_block = sumlength / CRC_BLOCK_SIZE ;
	et_uint32 sec_last = sumlength % CRC_BLOCK_SIZE;
	for (i = 0; i < sec_block; i++) 
	{		
		if( 0 != (error = spi_flash_read(start_sec * SPI_FLASH_SEC_SIZE + i * CRC_BLOCK_SIZE ,(et_uint32 *)buf, BUFSIZE)))
		{
				free(crc_table);
				free(buf);
				os_printf("spi_flash_read error %d\n",error);
				return -1;
		}
		crc = crc32(crc, buf, BUFSIZE);		
	}
	
	if(sec_last > 0 ) 
	{
		if (0 != (error = spi_flash_read(start_sec * SPI_FLASH_SEC_SIZE + i * CRC_BLOCK_SIZE, (et_uint32 *)buf, sec_last)))
		{
			free(crc_table);
			free(buf);
			os_printf("spi_flash_read error %d\n",error);
			return -1;
		}
		crc = crc32(crc, buf, sec_last);
	}
	*img_crc = crc;
	free(crc_table);
	free(buf);
	return 0;
}


int ICACHE_FLASH_ATTR
upgrade_crc_check(et_uint16 fw_bin_sec ,et_uint32 sumlength)
{
	et_int32 ret;
	et_uint32 img_crc;
	et_uint32 flash_crc = 0xFF;
	
	start_sec = fw_bin_sec;
	if(0 != init_crc_table()) 
	{
		return false;
	}
	
	ret = calc_img_crc(sumlength - 4,&img_crc);
	if (ret < 0) 
	{
		return false;
	}
	
	img_crc = abs(img_crc);
	os_printf("img_crc = %u\n",img_crc);
	spi_flash_read(start_sec * SPI_FLASH_SEC_SIZE + sumlength - 4,&flash_crc, 4);
   	os_printf("flash_crc = %u\n",flash_crc);
	if(img_crc == flash_crc) 
	{
		return 0;
	} 
	else 
	{
		return -1;
	}
}

void  upgrade_deinit(void)
{
	if (system_upgrade_flag_check() != UPGRADE_FLAG_START) 
	{
		system_upgrade_deinit();
	}
}

void upgrade_check(struct upgrade_server_info *server)
{
    /*network not stable, upgrade data lost, this may be called*/
	vTaskDelete(pxCreatedTask);
	os_timer_disarm(&upgrade_timer);

	if(NULL != precv_buf)
	{
		free(precv_buf);
	}

	totallength = 0;
	sumlength = 0;
	flash_erased = FALSE;

	/*take too long to finish,fail*/
	server->upgrade_flag = false;
	system_upgrade_flag_set(UPGRADE_FLAG_IDLE);

	upgrade_deinit();

	os_printf("\n upgrade fail,exit.\n");
	if (server->check_cb != NULL) 
	{
		server->check_cb(server);
	}

}

bool OUT_OF_RANGE(et_uint16 erase_sec)
{
	et_uint8 spi_size_map = system_get_flash_size_map();
	et_uint16 sec_num = 0;
	et_uint16 start_sec = 0;
	
	if (spi_size_map == FLASH_SIZE_8M_MAP_512_512 || 
		spi_size_map ==FLASH_SIZE_16M_MAP_512_512 ||
		spi_size_map ==FLASH_SIZE_32M_MAP_512_512)
	{
		start_sec = (system_upgrade_userbin_check() == USER_BIN2)? 1:129;
		sec_num = 123;
	} 
	else if(spi_size_map == FLASH_SIZE_16M_MAP_1024_1024 || 
		spi_size_map == FLASH_SIZE_32M_MAP_1024_1024)
	{
		start_sec = (system_upgrade_userbin_check() == USER_BIN2)? 1:257;
		sec_num = 251;
	} 
	else 
	{
		start_sec = (system_upgrade_userbin_check() == USER_BIN2)? 1:65;
		sec_num = 59;
	}
	
	if((erase_sec >= start_sec) &&(erase_sec <= (start_sec + sec_num)))
	{
		return false;
	} 
	else 
	{
		return true;
	}
}

bool system_upgrade_internal(struct upgrade_param *upgrade, et_uint8 *data, et_uint32 len)
{
	bool ret = false;
	et_uint16 secnm=0;
	
	if(data == NULL || len == 0)
	{
	        return true;
	}

	/*got the sumlngth,erase all upgrade sector*/
	if(len > SPI_FLASH_SEC_SIZE )
	{
		upgrade->fw_bin_sec_earse = upgrade->fw_bin_sec;

		secnm = ((upgrade->fw_bin_addr + len) >> 12) + (len & 0xfff ? 1 : 0);
		while(upgrade->fw_bin_sec_earse != secnm) 
		{
			taskENTER_CRITICAL();
			if(OUT_OF_RANGE( upgrade->fw_bin_sec_earse))
			{
				os_printf("fw_bin_sec_earse:%d, Out of range\n",upgrade->fw_bin_sec_earse);
				break;
			}
			else
			{
				 spi_flash_erase_sector(upgrade->fw_bin_sec_earse);
				 upgrade->fw_bin_sec_earse++;
			}
			taskEXIT_CRITICAL();
			vTaskDelay(10 / portTICK_RATE_MS);
		}
		os_printf("flash erase over\n");
		return true;
	}

	upgrade->buffer = (uint8 *)zalloc(len + upgrade->extra);
	memcpy(upgrade->buffer, upgrade->save, upgrade->extra);
	memcpy(upgrade->buffer + upgrade->extra, data, len);

	len += upgrade->extra;
	upgrade->extra = len & 0x03;
	len -= upgrade->extra;

	if(upgrade->extra <= 4)
		memcpy(upgrade->save, upgrade->buffer + len, upgrade->extra);
	else
	      os_printf("ERR3:arr_overflow,%u,%d\n",__LINE__,upgrade->extra);

	do 
	{
		if(upgrade->fw_bin_addr + len >= (upgrade->fw_bin_sec + upgrade->fw_bin_sec_num) * SPI_FLASH_SEC_SIZE) 
		{
			break;
		}

		if(spi_flash_write(upgrade->fw_bin_addr, (uint32 *)upgrade->buffer, len) != SPI_FLASH_RESULT_OK) 
		{
		    	break;
		}

		ret = true;
		upgrade->fw_bin_addr += len;
	} while (0);

	free(upgrade->buffer);
	upgrade->buffer = NULL;
	return ret;
}

bool system_upgrade(et_uint8 *data, et_uint32 len)
{
    bool ret;

    ret = system_upgrade_internal(upgrade, data, len);

    return ret;
}

et_uint16 system_get_fw_start_sec()
{
	if(upgrade != NULL) 
	{
		return upgrade->fw_bin_sec;
	} 
	else 
	{
		return 0;
	}
}

BOOL upgrade_data_load(et_char *pusrdata, et_uint16 length)
{
	et_char *ptr = NULL;
	et_char *ptmp2 = NULL;
	et_char lengthbuffer[32]={0};

	//check http response protocol, \r\n\r\n is head end, and content length
	if (totallength == 0 && (ptr = (char *)strstr(pusrdata, "\r\n\r\n")) != NULL &&
		(ptr = (char *)strstr(pusrdata, "Content-Length")) != NULL) 
	{
		os_printf("\n pusrdata %s\n",pusrdata);
		ptr = (char *)strstr(pusrdata, "Content-Length: ");
		if (ptr != NULL) 
		{
			ptr += 16;
			ptmp2 = (char *)strstr(ptr, "\r\n");

			if (ptmp2 != NULL) 
			{
				if((ptmp2 - ptr) <= 32)
					memcpy(lengthbuffer, ptr, ptmp2 - ptr);
				else
				     os_printf("ERR1:arr_overflow,%u,%d\n",__LINE__,(ptmp2 - ptr));
				
				sumlength = atoi(lengthbuffer);
				os_printf("userbin sumlength:%d \n",sumlength);

				ptr = (char *)strstr(pusrdata, "\r\n\r\n"); 
				length -= (ptr - pusrdata); //http head length
				length -= 4;
				totallength += length;   //http head length

				/*at the begining of the upgrade,we get the sumlength 
				 *and erase all the target flash sectors,return false
				 *to close the connection, and start upgrade again.  
				 */
				if(FALSE == flash_erased)
				{
					flash_erased = system_upgrade(ptr + 4, sumlength);
					return flash_erased;
				}
				else
				{
					system_upgrade(ptr + 4, length);
				}
			}
			else
			{
				os_printf("ERROR:Get sumlength failed\n");
			    	return false;
			}
		}
		else
		{
			os_printf("ERROR:Get Content-Length failed\n");
			return false;
		}
	} 
	else 
	{
		if(totallength != 0)
		{
			totallength += length;
			if(totallength > sumlength)
			{
				os_printf("strip the 400 error mesg\n");
				length = length -(totallength- sumlength);
			}

			os_printf(">>>recv %dB, %dB left\n",totallength,sumlength - totallength);
			system_upgrade(pusrdata, length);
		} 
		else 
		{
			os_printf("server response with something else,check it!\n");
			return false;
		}
	}

	return true;
}

void upgrade_task(void *pvParameters)
{
	et_int32 recbytes;
	et_int32 sta_socket;
	et_int32 retry_count = 0;
	struct ip_info ipconfig;
	et_int32 ret=0;
	struct upgrade_server_info *server = pvParameters;

	flash_erased = FALSE;
	precv_buf = (char*)malloc(UPGRADE_DATA_SEG_LEN);
	if(NULL == precv_buf)
	{
		os_printf("upgrade_task,memory exhausted, check it\n");
	}

	while (retry_count++ < UPGRADE_RETRY_TIMES) 
	{
		wifi_get_ip_info(STATION_IF, &ipconfig);
		/* check the ip address or net connection state*/
		while (ipconfig.ip.addr == 0) 
		{
			vTaskDelay(1000 / portTICK_RATE_MS);
			wifi_get_ip_info(STATION_IF, &ipconfig);
		}

		sta_socket = socket(PF_INET, SOCK_STREAM, 0);
		if (-1 == sta_socket) 
		{
			close(sta_socket);
			vTaskDelay(1000 / portTICK_RATE_MS);
			os_printf("socket fail !\r\n");
			continue;
		}

		/*for upgrade connection debug*/
		//server->sockaddrin.sin_addr.s_addr= inet_addr("192.168.1.170");

		if((ret = connect(sta_socket,(struct sockaddr *)(&server->sockaddrin), sizeof(struct sockaddr))) != 0) 
		{
			printf("server->sockaddrin = %d, ret = %d\n",server->sockaddrin.sin_addr.s_addr, ret);
			close(sta_socket);
			vTaskDelay(1000 / portTICK_RATE_MS);
			os_printf("connect fail!\r\n");
			continue;
		}
		
		os_printf("Connect ok!\r\n");

		system_upgrade_init();
		system_upgrade_flag_set(UPGRADE_FLAG_START);

		if(write(sta_socket, server->url, strlen(server->url) + 1) < 0) 
		{
			close(sta_socket);
			vTaskDelay(1000 / portTICK_RATE_MS);
			os_printf("send fail\n");
			continue;
		}
		os_printf("Request send success\n");

		while((recbytes = read(sta_socket, precv_buf, UPGRADE_DATA_SEG_LEN)) > 0) 
		{
			if(FALSE == flash_erased)
			{
				close(sta_socket);
				os_printf("pre erase flash!\n");
				upgrade_data_load(precv_buf,recbytes);
				break;                    
			}

			if(false == upgrade_data_load(precv_buf, recbytes)) 
			{
				os_printf("upgrade data error!\n");
				close(sta_socket);
				flash_erased = FALSE;
				vTaskDelay(1000 / portTICK_RATE_MS);
				break;
			}
			/*this two length data should be equal, if totallength is bigger, 
			 *maybe data wrong or server send extra info, drop it anyway*/
			if(totallength >= sumlength) 
			{
				os_printf("upgrade data load finish.\n");
				close(sta_socket);
				goto finish;
			}
			os_printf("upgrade_task %d word left\n",uxTaskGetStackHighWaterMark(NULL));
		    
		}

		if(recbytes <= 0) 
		{
			close(sta_socket);
			flash_erased=FALSE;
			vTaskDelay(1000 / portTICK_RATE_MS);
			os_printf("ERROR:read data fail!\r\n");
		}

		totallength =0;
		sumlength = 0;
	}

	finish:
	os_timer_disarm(&upgrade_timer);
	
	if(upgrade_crc_check(system_get_fw_start_sec(), sumlength) != 0)
	{
		printf("upgrade crc check failed !\n");
		server->upgrade_flag = false;
		system_upgrade_flag_set(UPGRADE_FLAG_IDLE);	
	}

	if(NULL != precv_buf) 
	{
		free(precv_buf);
	}

	totallength = 0;
	sumlength = 0;
	flash_erased = FALSE;

	if(retry_count == UPGRADE_RETRY_TIMES)
	{
		/*retry too many times, fail*/
		server->upgrade_flag = false;
		system_upgrade_flag_set(UPGRADE_FLAG_IDLE);

	}
	else
	{
		server->upgrade_flag = true;
		system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
	}

	upgrade_deinit();
	os_printf("\n Exit upgrade task.\n");
	if (server->check_cb != NULL) 
	{
		server->check_cb(server);
	}
	vTaskDelay(100 / portTICK_RATE_MS);
	vTaskDelete(NULL);
}


BOOL  system_upgrade_start(struct upgrade_server_info *server)
{
	portBASE_TYPE ret = 0;

	if(NULL == pxCreatedTask)
	{
		ret = xTaskCreate(upgrade_task, "upgrade_task", 224, server, 5, pxCreatedTask);//1024, 890 left
		if(pdPASS == ret)
		{
		    os_timer_disarm(&upgrade_timer);
		    os_timer_setfn(&upgrade_timer, (os_timer_func_t *)upgrade_check, server);
		    os_timer_arm(&upgrade_timer, 1200000, 0);
		}
	}

    return(pdPASS == ret);
}

void system_upgrade_init(void)
{
	et_uint32 user_bin2_start, user_bin1_start;
	et_uint8 spi_size_map = system_get_flash_size_map();

	if (upgrade == NULL) 
	{
	   	upgrade = (struct upgrade_param *)zalloc(sizeof(struct upgrade_param));
	}

	user_bin1_start = 1; 
	if (spi_size_map == FLASH_SIZE_8M_MAP_512_512 || 
		spi_size_map ==FLASH_SIZE_16M_MAP_512_512 ||
		spi_size_map ==FLASH_SIZE_32M_MAP_512_512){
		user_bin2_start = 129;
		upgrade->fw_bin_sec_num = 123;
	}
	else if(spi_size_map == FLASH_SIZE_16M_MAP_1024_1024 || 
		spi_size_map == FLASH_SIZE_32M_MAP_1024_1024){
		user_bin2_start = 257;
		upgrade->fw_bin_sec_num = 251;
	} 
	else 
	{
		user_bin2_start = 65;
		upgrade->fw_bin_sec_num = 59;
	}
   
	upgrade->fw_bin_sec = (system_upgrade_userbin_check() == USER_BIN1) ? user_bin2_start : user_bin1_start;
	upgrade->fw_bin_addr = upgrade->fw_bin_sec * SPI_FLASH_SEC_SIZE;
	upgrade->fw_bin_sec_earse = upgrade->fw_bin_sec;
}

void system_upgrade_deinit(void)
{
	if(upgrade != NULL)
		free(upgrade);
	else
		return;
}
