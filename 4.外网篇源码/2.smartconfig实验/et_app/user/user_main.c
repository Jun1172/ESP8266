/******************************************************************************
 * 作者：长沙-嵌入式-小白
 * 修改时间：2017-1-14
 * 文件名: user_main.c
 * 用途：主函数入口
*******************************************************************************/
#include "driver/user_main.h"

#define DEVICE_TYPE 		"gh_9e2cff3dfa51" //wechat public number
#define DEVICE_ID 			"122475" //model ID

#define DEFAULT_LAN_PORT 	12476

LOCAL esp_udp ssdp_udp;
LOCAL struct espconn pssdpudpconn;
LOCAL os_timer_t ssdp_time_serv;

uint8  lan_buf[200];
uint16 lan_buf_len;
uint8  udp_sent_cnt = 0;
const airkiss_config_t akconf =
{
	(airkiss_memset_fn)&memset,
	(airkiss_memcpy_fn)&memcpy,
	(airkiss_memcmp_fn)&memcmp,
	0,
};

/********************************************
 * 函数名：airkiss_wifilan_time_callback
 * 函数用途：局域网查询
 * 修改时间：2017-1-14
 * 编写：小白
 ********************************************/
LOCAL void ICACHE_FLASH_ATTR
airkiss_wifilan_time_callback(void)
{
	uint16 i;
	airkiss_lan_ret_t ret;

	//连接了一段时间后，关闭这个smartcongfig查询，准备重启
	if ((udp_sent_cnt++) >30) {
		udp_sent_cnt = 0;
		os_timer_disarm(&ssdp_time_serv);
	}

	ssdp_udp.remote_port = DEFAULT_LAN_PORT;
	ssdp_udp.remote_ip[0] = 255;
	ssdp_udp.remote_ip[1] = 255;
	ssdp_udp.remote_ip[2] = 255;
	ssdp_udp.remote_ip[3] = 255;
	lan_buf_len = sizeof(lan_buf);
	ret = airkiss_lan_pack(AIRKISS_LAN_SSDP_NOTIFY_CMD,
		DEVICE_TYPE, DEVICE_ID, 0, 0, lan_buf, &lan_buf_len, &akconf);
	if (ret != AIRKISS_LAN_PAKE_READY) {
		os_printf("Pack lan packet error!");
		return;
	}

	//通过UDP向局域网发送数据，udp是不需要连接的
	ret = espconn_sendto(&pssdpudpconn, lan_buf, lan_buf_len);
	if (ret != 0) {
		os_printf("UDP send error!");
	}
	os_printf("Finish send notify!\n");
}
/********************************************
 * 函数名：airkiss_wifilan_recv_callbk
 * 函数用途：UDP接收回调函数
 * 修改时间：2017-1-14
 * 编写：小白
 ********************************************/
LOCAL void ICACHE_FLASH_ATTR
airkiss_wifilan_recv_callbk(void *arg, char *pdata, unsigned short len)
{
	uint16 i;
	remot_info* pcon_info = NULL;

	airkiss_lan_ret_t ret = airkiss_lan_recv(pdata, len, &akconf);
	airkiss_lan_ret_t packret;

	switch (ret){
	case AIRKISS_LAN_SSDP_REQ:
		espconn_get_connection_info(&pssdpudpconn, &pcon_info, 0);
		os_printf("remote ip: %d.%d.%d.%d \r\n",pcon_info->remote_ip[0],pcon_info->remote_ip[1],
			                                    pcon_info->remote_ip[2],pcon_info->remote_ip[3]);
		os_printf("remote port: %d \r\n",pcon_info->remote_port);

        pssdpudpconn.proto.udp->remote_port = pcon_info->remote_port;
		memcpy(pssdpudpconn.proto.udp->remote_ip,pcon_info->remote_ip,4);
		ssdp_udp.remote_port = DEFAULT_LAN_PORT;

		lan_buf_len = sizeof(lan_buf);
		packret = airkiss_lan_pack(AIRKISS_LAN_SSDP_RESP_CMD,
			DEVICE_TYPE, DEVICE_ID, 0, 0, lan_buf, &lan_buf_len, &akconf);

		if (packret != AIRKISS_LAN_PAKE_READY) {
			os_printf("Pack lan packet error!");
			return;
		}

		os_printf("\r\n\r\n");
		for (i=0; i<lan_buf_len; i++)
			os_printf("%c",lan_buf[i]);
		os_printf("\r\n\r\n");

		packret = espconn_sendto(&pssdpudpconn, lan_buf, lan_buf_len);
		if (packret != 0) {
			os_printf("LAN UDP Send err!");
		}

		break;
	default:
		os_printf("Pack is not ssdq req!%d\r\n",ret);
		break;
	}
}
/********************************************
 * 函数名：airkiss_start_discover
 * 函数用途：初始化一个UDP用于查找局域网
 * 修改时间：2017-1-14
 * 编写：小白
 ********************************************/
void ICACHE_FLASH_ATTR
airkiss_start_discover(void)
{
	//创建一个UDP的通信
	ssdp_udp.local_port = DEFAULT_LAN_PORT;
	pssdpudpconn.type = ESPCONN_UDP;
	pssdpudpconn.proto.udp = &(ssdp_udp);
	espconn_regist_recvcb(&pssdpudpconn, airkiss_wifilan_recv_callbk);
	espconn_create(&pssdpudpconn);

	os_timer_disarm(&ssdp_time_serv);
	os_timer_setfn(&ssdp_time_serv, (os_timer_func_t *)airkiss_wifilan_time_callback, NULL);
	os_timer_arm(&ssdp_time_serv, 1000, 1);//1s
}
/********************************************
 * 函数名：smartconfig_done
 * 函数用途：smartconfig连接函数
 * 修改时间：2017-1-14
 * 编写：小白
 ********************************************/
void ICACHE_FLASH_ATTR
smartconfig_done(sc_status status, void *pdata)
{
    switch(status) {
        case SC_STATUS_WAIT:
            printf("SC_STATUS_WAIT\n");
            break;
        case SC_STATUS_FIND_CHANNEL:
            printf("SC_STATUS_FIND_CHANNEL\n");
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
            printf("SC_STATUS_GETTING_SSID_PSWD\n");
            sc_type *type = pdata;
            if (*type == SC_TYPE_ESPTOUCH) {
                printf("SC_TYPE:SC_TYPE_ESPTOUCH\n");
            } else {
                printf("SC_TYPE:SC_TYPE_AIRKISS\n");
            }
            break;
        case SC_STATUS_LINK:
            printf("SC_STATUS_LINK\n");
            //获取到UDP传送来的ssid和password
            struct station_config *sta_conf = pdata;
            //根据收到的信息连接wifi
	        wifi_station_set_config(sta_conf);
	        wifi_station_disconnect();
	        wifi_station_connect();
            break;
        case SC_STATUS_LINK_OVER:
            printf("SC_STATUS_LINK_OVER\n");
            if (pdata != NULL) {
                uint8 phone_ip[4] = {0};

                //返回执行smartconfig的手机IP
                memcpy(phone_ip, (uint8*)pdata, 4);
                printf("Phone ip: %d.%d.%d.%d\n",phone_ip[0],phone_ip[1],phone_ip[2],phone_ip[3]);
            } else {
            	//用于内网发现
				airkiss_start_discover();
			}
            smartconfig_stop();
            break;
    }
}
/********************************************
 * 函数名：smartconfig_task
 * 函数用途：smartconfig_start开启函数
 * 修改时间：2017-1-14
 * 编写：小白
 ********************************************/
void ICACHE_FLASH_ATTR
smartconfig_task(void *pvParameters)
{
	//启动一个回调函数
    smartconfig_start(smartconfig_done);
    //任务执行一回，直接干掉
    vTaskDelete(NULL);
}

/********************************************
 * 函数名：user_init
 * 函数用途：主函数
 * 修改时间：2017-1-14
 * 编写：小白
 ********************************************/
void ICACHE_FLASH_ATTR
user_init(void)
{
	//根据API要求，wifi模式只能是STATION_MODE
    wifi_set_opmode(STATION_MODE);
    //根据smartconfig_start要求，只允许执行一次
    xTaskCreate(smartconfig_task, "smartconfig_task", 256, NULL, 2, NULL);
}
