/*******************************************************************************
 * Copyright (c) 2012, 2013 Beidouapp Corp.
 *
 * All rights reserved. 
 *
 * Contributors:
 *    Peter Peng - initial contribution
 *******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "esp_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/uart.h"

#include "user_config.h"
#include "et_fifo.h"
#include "lwip/netdb.h"
#include "factory.h"
#include "espressif/upgrade.h"
#include "et_api_compatible.h"

#define MAX_USER_ID 32
#define MAX_ILINK_CONNECT  5

et_char g_user_id[ET_USER_ID_LEN_MAX] = {0};
et_char g_group_id[ET_USER_ID_LEN_MAX] = {0};
et_uint32 g_group_message = 0;
struct hostent *file_server_ip_addr = NULL;
msg_to_net_t msg={0};
et_uchar code[32]={0};
extern et_int32 read_uart_data_flag;
extern xQueueHandle xQueueUart;
et_cloud_handle g_cloud_handle = NULL;
et_cloud_connect_type g_cloud_con_para = {ET_FALSE, ET_TRUE, 90};
et_uchar kick_out=0; 
et_dfs_file_info_type i_file_info;
	
struct ilink_connect_client
{
	et_int32 state;
	et_char user_id[MAX_USER_ID];

};

static struct ilink_connect_client g_ilink_connect_clients[MAX_ILINK_CONNECT];

static et_uchar uart_recv_buff[UART_MAX_READ_BUFFER] = {0};
static et_uchar uart_send_buff[UART_MAX_SEND_BUFFER] = {0};
et_int32 to_stop_app = 0;

et_uint32 write_flash_end=0;
et_int64 file_total_size=0;
et_uint32 audio_voice_data=0;
et_uint16 sector=AUDIO_FLASH_START_SECTOR;

void print_hex(et_uchar *label, et_uchar *str, et_int32 len)
{
	et_int32 i;
	
	os_printf("%s : ", label);
	for(i = 0; i < len; i++)
		os_printf("%02x ", str[i]);
	os_printf("\n");
}

et_uchar check_sum(et_uchar *buf, et_int32 len)
{
	et_uchar bcc=0;
	et_int32 i;

	for(i = 0; i < len; i ++)
		bcc ^= buf[i];

	return bcc; 
}

// ascii to 8421bcd,say 0x32 = 50
et_uint32 ascii_2_dec(et_int32 ascii)
{
	et_int32 n =1, dec=0;
	
	while(ascii > 0) 
	{
   		dec += (ascii % 10)  * n;
   		ascii /=10;  
   		n *= 16;
	}
	
	return dec;
}

static et_int32 ack_to_mqtt(et_uchar *msg_buf, et_uchar *ack_buf, et_int32 rc)
{
	et_int32 len=START_LEN;
	
	ack_buf[0] = 0xFF;
	ack_buf[1] = 0xFF;
	ack_buf[2] = 0x00;
	ack_buf[3] = ACK_ERROR_LEN;
	ack_buf[4] = msg_buf[4] | 0x0F;
	ack_buf[5] = msg_buf[5];
	ack_buf[6] = msg_buf[6];
	ack_buf[7] = rc;
	ack_buf[8] = check_sum(&ack_buf[2], ACK_ERROR_LEN + 1);
	len += ACK_ERROR_LEN;
	
	return len;
	
}

static et_int32 ack_temp_hum_to_mqtt(et_uchar *msg_buf, et_uchar *temp_hum, et_uchar *ack_buf, et_uchar rc)
{
	et_int32 len=START_LEN;
	
	ack_buf[0] = 0xFF;
	ack_buf[1] = 0xFF;
	ack_buf[2] = 0x00;
	ack_buf[3] = ACK_TEMP_HUM_LEN;
	ack_buf[4] = msg_buf[4] | 0x0F;
	ack_buf[5] = msg_buf[5];
	ack_buf[6] = msg_buf[6];
	ack_buf[7] = rc;
	ack_buf[8] = ascii_2_dec(temp_hum[0]);   //we transform to 8421bcd 0x50
	ack_buf[9] = ascii_2_dec(temp_hum[1]);
	ack_buf[10] = ascii_2_dec(temp_hum[2]);
	ack_buf[11] = ascii_2_dec(temp_hum[3]);
	ack_buf[12] = check_sum(&ack_buf[2], ACK_TEMP_HUM_LEN + 1);
	len += ACK_TEMP_HUM_LEN; 

	return len;
}

static et_int32 ack_barometric_to_mqtt(et_uchar *msg_buf, et_int32 barometric, et_int32 temp, et_uchar *ack_buf, et_uchar rc)
{
	et_uchar baro[4]={0}, t;
	et_int32 i, tmp, len=START_LEN;

	tmp = ascii_2_dec(barometric);
	for(i = 0; i < 4; i++)
    		baro[i] = tmp >> (3 - i) * 8;

	tmp = ascii_2_dec(temp);
	t = tmp;
	
	ack_buf[0] = 0xFF;
	ack_buf[1] = 0xFF;
	ack_buf[2] = 0x00;
	ack_buf[3] = ACK_BAROMETRIC_LEN;
	ack_buf[4] = msg_buf[4] | 0x0F;
	ack_buf[5] = msg_buf[5];
	ack_buf[6] = msg_buf[6];
	ack_buf[7] = rc;
	ack_buf[8] = baro[0];
	ack_buf[9] = baro[1];
	ack_buf[10] = baro[2];
	ack_buf[11] = baro[3];
	ack_buf[12] = t;
	ack_buf[13] = check_sum(&ack_buf[2], ACK_BAROMETRIC_LEN + 1);
	len += ACK_BAROMETRIC_LEN; 

	return len;
}

void upgrade_callback(void *arg)
{
	struct upgrade_server_info *server = arg;

	if(server->upgrade_flag == true)
	{
		printf("upgrade success.\n");
		system_upgrade_reboot();
	}
	else
		printf("upgrade failed.\n");

	free(server->url);
	server->url = NULL;
}

et_int32 parse_msg_from_mqtt(et_uchar *msg_buf, et_int32 data_len)
{
	et_int32 i, pos=0, rc = -1;
	et_uchar cmd, type, bcc;
	et_int32 len, seq;
	et_uint16 gb_code=0;
	WORK_MODE_T mode;

	os_printf("\r\n数据内容：\r\n");
	uart0_tx_SendStr(msg_buf);

	bzero(msg_buf,sizeof(msg_buf));
	rc = et_chat_to(g_cloud_handle,"123", 3, g_user_id, SEND_TO_CLOUD_FIRST);

	return rc;
}

et_int32 add_userid_to_clients(et_char *userid)
{
	et_int32 i = 0,j = 0;
	et_int32 ret = -1;
	et_char  *str = "now device has max users";
	//et_int32 add_flag = 0;
	
	for(i = 0; i < MAX_ILINK_CONNECT;i++)
	{
		if(g_ilink_connect_clients[i].state == 1)
		{
			ret = strcmp(userid,g_ilink_connect_clients[i].user_id);
			if(ret == 0)
			{
				return -1;
			}
		}
	}
			
	for(j = 0; j < MAX_ILINK_CONNECT;j ++)
	{
		if(g_ilink_connect_clients[j].state == -1)
		{
			g_ilink_connect_clients[j].state = 1;
			strcpy(g_ilink_connect_clients[j].user_id,userid);
			os_printf("now add userid is %s\n",g_ilink_connect_clients[j].user_id);
			break;

		}

	}

	if(j >= MAX_ILINK_CONNECT)
	{
		et_chat_to(g_cloud_handle,str,strlen(str),userid,SEND_TO_CLOUD);
		return -1;
	}
	else
	{
		return 0;
	}
}

et_int32 remove_userid_clients(et_char *userid)
{
	et_int32 i = 0;
	et_int32 ret = -1;
	for(i = 0; i < MAX_ILINK_CONNECT;i++)
	{
		if(g_ilink_connect_clients[i].state == 1)
		{
			ret = strcmp(userid,g_ilink_connect_clients[i].user_id);
			if(ret == 0)
			{
				os_printf("now userid is %s\n",g_ilink_connect_clients[i].user_id);
				g_ilink_connect_clients[i].state = -1;
				memset(g_ilink_connect_clients[i].user_id,0,sizeof(g_ilink_connect_clients[i].user_id));
				os_printf("now status is %d\n",g_ilink_connect_clients[i].state);
				return 0;
			}
		}
		
	}

	return -1;

}

void read_uart_buf_task(void *pvParameters)
{
    os_event_t e;
	et_int32 rc = 0;
	et_int32 cmd,seq,len;
	et_uchar *data;
	
	for (;;) 
	{
		if (xQueueReceive(xQueueUart, (void *)&e, (portTickType)portMAX_DELAY)) 
		{
			switch (e.event)
			{
				case UART_EVENT_RX_CHAR:
				{
					read_uart_data_flag = 1;
					memset(uart_recv_buff,0,sizeof(uart_recv_buff));
					rc = recv_data_uart(uart_recv_buff, e.param, e.param * 50);
					read_uart_data_flag = 0;
					printf("receive uart %d\n", rc);
                		}
                			break;

                default:
                    break;
            }
        }
    }

    vTaskDelete(NULL);
}

void init_clients(void)
{
	et_int32 j = 0;
	
	for(j = 0; j < MAX_ILINK_CONNECT;j ++)
	{
		g_ilink_connect_clients[j].state = -1;
		memset(g_ilink_connect_clients[j].user_id,0,sizeof(g_ilink_connect_clients[j].user_id));
			
	}

}

et_int32 write_flash_callback (void *arg, const et_int64 file_size, const et_char *data, const et_int32 current_size)
{
	et_int32 result=-1;
	
	if (arg == NULL)
	{
		return EINVAL;
	}

	if(sector > AUDIO_FLASH_STOP_SECTOR)             //超过flash，flash重新覆盖初始地址
	{	
		file_total_size += current_size;
		if(file_total_size >= file_size)
		{
			sector = AUDIO_FLASH_START_SECTOR;
			file_total_size = AUDIO_FLASH_LEN - 1;
			audio_voice_data = 1;        //to inform interrupt of voice data coming                              //越界不播放，仍然下载完，只是覆盖
			write_flash_end = 1;          //to inform interrupt of being able to send voice data to i2s 
			printf("file size > flash size.just play length of flash size(the former 45 seconds).\n");
		}
		
		return 0;
	}
	
	result = spi_flash_erase_sector(sector);                                             
	if(result != SPI_FLASH_RESULT_OK) 
	{
		printf("spi_flash_erase_sector error\n");
		return -1;
	}
	
	result = spi_flash_write(sector * SPI_FLASH_SEC_SIZE, (et_uint32 *)data, current_size);           //write received voice data(4096 bytes) to flash
	if(result != SPI_FLASH_RESULT_OK) 
	{
		printf("spi_flash_write error\n");
		return -1;
	}
	
	file_total_size += current_size;
	sector++;

	if(file_total_size >= file_size)                //下载完成
	{
		if(file_size > AUDIO_FLASH_LEN)             //文件总长度打印flash长度，只播放音频的flash长度
			file_total_size = AUDIO_FLASH_LEN;

		printf("down load file end.\n");
		sector = AUDIO_FLASH_START_SECTOR;
		audio_voice_data = 1;           //to inform interrupt of voice data coming 
		write_flash_end = 1;          //to inform interrupt of being able to send voice data to i2s 
	}	

	return 0;
}

void et_message_process(et_int32 type, et_char *send_userid, et_char *topic_name, et_int32 topic_len, et_context_mes_type *message)
{
	int rc = -1;

	switch(type)
	{
		case MES_CHAT_TO_CHAT:
		{
			os_printf("数据来源： %s\n", send_userid);
			memset(g_user_id, 0, sizeof(g_user_id));
			snprintf(g_user_id, sizeof(g_user_id), "%s",send_userid);
			parse_msg_from_mqtt(message->payload, message->payload_len);
		}
			break;
			
		case MES_FILE_TRANSFERS:
		{
			if(user_get_run_mode() != WORK_MODE_AUDIO)
			{
				printf("run in mode %d ,error\n", user_get_run_mode());
				break;
			}
				
			os_printf("File trans mes from %s:%s\n", send_userid, topic_name);
			memset(&i_file_info, 0, sizeof(et_dfs_file_info_type));
			rc = et_file_info(g_cloud_handle,message->payload, &i_file_info);
			if(rc == -1)
			{
				os_printf("file info parse failed\n");
				break;
			}
			
			if(file_server_ip_addr == NULL)
			{
				file_server_ip_addr = gethostbyname(i_file_info.source_ip_addr);
				if(file_server_ip_addr == NULL)
				{
					os_printf("failed, get ip from im server\n");
					break;
				}
			}

			memset(i_file_info.source_ip_addr, 0, sizeof(i_file_info.source_ip_addr));
			strcpy(i_file_info.source_ip_addr, inet_ntoa(*(file_server_ip_addr->h_addr)));

			while(write_flash_end) 
			{
				os_delay_us(500);
			}
			
			rc = et_down_file(g_cloud_handle,&i_file_info, write_flash_callback);
		}
			break;
			
		case MES_FROM_GROUP_SUBTOPIC:
		{
			os_printf("Group mes from %s\n", topic_name);

			parse_msg_from_mqtt(message->payload, message->payload_len);
		}
			break;
			
		case MES_FROM_LOCAL:
		{
			os_printf("Local mes from %s:%s\n", send_userid, topic_name);
			memset(g_user_id, 0, sizeof(g_user_id));
			snprintf(g_user_id, sizeof(g_user_id), "%s",send_userid);
			parse_msg_from_mqtt(message->payload, message->payload_len);
		}
			break;
			
		case MES_NOTICE_ADD_BUDDY:
			os_printf("You are be add buddy by %s:%s\n", send_userid, topic_name);
			break;
			
		case MES_NOTICE_REMOVE_BUDDY:
			os_printf("You are be remove buddy by %s:%s\n", send_userid, topic_name);
			break;
			
		case MES_USER_OFFLINE:
			os_printf("%s Offline:%s\n", send_userid, topic_name);
			break;
			
		case MES_USER_ONLINE:
			os_printf("%s Online:%s\n", send_userid, topic_name);
			break;
			
		case MES_USER_STATUS:
			os_printf("%s Status:%s\n", send_userid, topic_name);
			break;
			
		case MES_ONLINE_BUDDIES_LIST:
			os_printf("Get online buddies list%s:%s\n", send_userid, topic_name);
			break;
	}
}

void et_event_process(et_event_type event)
{
	et_int32 rc = -1;
	
	switch(event.event_no)
	{
		case EVENT_CLOUD_CONNECT:
		{
			disarm_wifi_spark_timer();
			wifi_led_on();
			os_printf("You are connect:0x%x\n", event.event_no);
		}
			break;
		case EVENT_CLOUD_DISCONNECT:
		{
			set_wifi_spark_timer(1000);
			os_printf("You are disconnect:0x%x\n", event.event_no);
		}
			break;
		case EVENT_LOGIN_KICK:
			kick_out = 1;
			os_printf("Your account was login by others:0x%x\n", event.event_no);
			break;
		case EVENT_CLOUD_SEND_FAILED:
			os_printf("Cloud send failed\n");
			break;
		case EVENT_CLOUD_SEND_SUCCESS:
			os_printf("Cloud send success\n");
			break;
		case EVENT_LOCAL_SEND_FAILED:
			os_printf("Local send failed\n");
			break;
		case EVENT_LOCAL_SEND_SUCCESS:
			os_printf("Local send success\n");
			break;
	}
}

void et_ilink_task(void *pvParameters)
{
	while(1)
	{
		if(!kick_out)
			et_ilink_loop(g_cloud_handle);
		taskYIELD();
	}

	vTaskDelete(NULL);	
}

void et_local_task(void *pvParameters)
{
	while(1)
	{
		et_server_loop(g_cloud_handle);
		taskYIELD();
	}
	
	vTaskDelete(NULL);	
}

void  et_user_main(void *pvParameters)
{
	et_int32 rc = -1;
	id_info_t id;
	et_server_info_t server_info;
	et_net_addr_type g_cloud_addr_para = {NULL, 0};
	
	os_printf("ET U-SDK var%s\n",et_sdk_ver());
	to_stop_app = 0;
	
	memset(&id, 0, sizeof(id_info_t));
	if(get_uid(&id) != SPI_FLASH_RESULT_OK)
	{
		os_printf("et_user_main, get_uid error\n");
		return ;
	}
	
	id.uid[UID_LEN - 1] = '\0';
	id.appkey[APPKEY_LEN - 1] = '\0';
	id.secretkey[SECRETKEY_LEN - 1] = '\0';

	memset(&server_info, 0, sizeof(et_server_info_t));
	if(get_server(&server_info) != SPI_FLASH_RESULT_OK)
	{
		os_printf("et_user_main, get_server error\n");
		return ;
	}

	server_info.lb_addr[ET_LB_ADDR_MAX - 1] = '\0';
	server_info.lb_port[ET_LB_PORT_MAX - 1] = '\0';
	
	g_cloud_addr_para.name_ip = server_info.lb_addr;
	g_cloud_addr_para.port = atoi(server_info.lb_port);

	os_printf("uid : %s\n", id.uid);
	os_printf("appkey : %s\n", id.appkey);
	os_printf("secretkey : %s\n", id.secretkey);
	os_printf("server information %s : %s\n", server_info.lb_addr, server_info.lb_port);

	g_cloud_handle = et_create_context(id.uid, id.appkey, id.secretkey, g_cloud_addr_para);
	if(NULL == g_cloud_handle)
	{
		os_printf("Init et account failed\n");
		goto error;
	}
	
	if(et_set_callback(g_cloud_handle,et_message_process, et_event_process) < 0)
	{
		os_printf("%s et_set_callback failed.\n");
		et_destroy_context(g_cloud_handle);	
		goto error;
	}

	rc = et_start_svr(g_cloud_handle);
	if(rc != 0){
		os_printf("%s et_start_svr fail.\n", __FUNCTION__);
		et_destroy_context(g_cloud_handle);
		return;
	}
	
	if(pdPASS != xTaskCreate(et_local_task, "local_task", 512, NULL, 2, NULL))
	{
		os_printf("%s et_local_task failed.\n", __FUNCTION__);
		et_stop_srv(g_cloud_handle);
		et_destroy_context(g_cloud_handle);
		return;
	}

	do
	{
		rc = et_login_cloud(g_cloud_handle, g_cloud_con_para);
		if(rc != 0)
		{
			os_printf("login_cloud fail\n");
		}
		vTaskDelay(2000 / portTICK_RATE_MS);
	}while(rc != 0 && to_stop_app == 0);
	
	if(pdPASS != xTaskCreate(et_ilink_task, "ilink_task", 768, NULL, 2, NULL))
	{
		os_printf("%s et_ilink_task failed.\n", __FUNCTION__);
		et_logout_cloud(g_cloud_handle);
		et_destroy_context(g_cloud_handle);
		goto error;
	}

error:
	vTaskDelete(NULL);
	return ;
}


