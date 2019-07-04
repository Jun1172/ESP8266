#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "et_types.h"
#include "et_fifo.h"


#define CMD_CONTROL   					0x70
#define CMD_CONTROL_ACK  				0x7F
#define CMD_QUERY						0x80
#define CMD_QUERY_ACK					0x8f

#define CMD_VER_UPDATE_NOTE			0x60
#define TYPE_VERSION					0x50

#define TYPE_RGB_LIGHT_DEV  			0x10
#define TYPE_TEMP_HUM_SENSOR 			0x20
#define TYPE_BAROMETRIC_SENSOR		0x30
#define TYPE_OLED_DEV					0x40

#define TEMP_AND_HUM					0x00
#define TEMPERATUR						0x01
#define HUMMITY							0x02

#define START_LEN 						4   //sizeof(start_flag and length)
#define ACK_ERROR_LEN					5
#define ACK_TO_UART					START_LEN + ACK_ERROR_LEN
#define ACK_TEMP_HUM_LEN 				9
#define ACK_BAROMETRIC_LEN 			10
#define APPKEY_LEN 						48
#define UID_LEN  						APPKEY_LEN
#define SECRETKEY_LEN				UID_LEN
#define ET_LB_PORT_MAX  				9
#define ET_LB_ADDR_MAX 				128
#define MAX_MSG_LEN					256
#define MAX_UART_MSG_LEN				192
#define WAV_HEAD_LEN					78

#define ACK_SUCCESS         				0x00
#define ACK_BCC_ERROR     				0x01
#define ACK_DEV_FAILED					0x02
#define ACK_MODE_ERR					0x03
#define ACK_CMD_ILLIGAL  				0xFF
#define ACK_INVALID          				0xFE

#define MODE_INFO_FLASH_SECTOR		0xFC
#define SERVER_INFO_SECTOR				0xFF
#define APPKEY_UID_FASH_SECTOR		0xFE
#define AUDIO_FLASH_START_SECTOR		0x100
#define AUDIO_FLASH_STOP_SECTOR		0x1AE
#define AUDIO_FLASH_LEN				0xAE000
#define AUDIO_FLASH_READ_START_ADDR	AUDIO_FLASH_START_SECTOR * SPI_FLASH_SEC_SIZE + WAV_HEAD_LEN
#define GB_DZK_START_SECTOR			0x1B0					 //gb2312 word library start address, from b0a1 to f7


#define SOFTWARE_VERSION  				"1.4.0.8-u34"

//#define USER_PRINT_DEBUG

//#define IR_DEMO
//#define IR_RX

#ifdef IR_DEMO
#include "driver/hw_timer.h"
#define WDEV_NOW()          REG_READ(0x3ff20c00)
#endif


#define pheadbuffer "Connection: keep-alive\r\n\
Cache-Control: no-cache\r\n\
User-Agent: Mozilla/5.0 (Windows NT 5.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.101 Safari/537.36 \r\n\
Accept: */*\r\n\
Accept-Encoding: gzip,deflate,sdch\r\n\
Accept-Language: zh-CN,zh;q=0.8\r\n\r\n"

#define RETURN_OK						(0)
#define RETURN_ERR						(-1)

#define LOGIC_LL						(0)
#define LOGIC_HL						(1)

#define HUM_IO_NUM						(5)
#define HUM_IO_PIN						(GPIO_Pin_5)
#define HUM_DATA_SIZE					(4)

#define AIRKISS_KEY_IO_NUM				(0)
#define AIRKISS_KEY_IO_PIN				(GPIO_Pin_0)

#define MAX_U16							(0xFFFF)

// define wifi status led
#define WIFI_STATUS_IO_MUX 				(PERIPHS_IO_MUX_GPIO4_U)
#define WIFI_STATUS_IO_NUM 				(4)
#define WIFI_STATUS_IO_FUNC  			(FUNC_GPIO4)
#define WIFI_STATUS_IO_PIN				(GPIO_Pin_4)

#define ARRAY_LEN(x)					(sizeof(x) / sizeof(x[0]))

typedef enum {
	WORK_MODE_DEFAULT = 0,
	WORK_MODE_AUDIO,
	WORK_MODE_RGB,
	WORK_MODE_BAROMETRIC,
	WORK_MODE_OLED,
	WORK_MODE_BUTT,
}WORK_MODE_T;

typedef enum {
	CONNECTING_WIFI=1,
	DO_SMARTCONFIG,
	CONNECTING_CLOUD,
	CONNECTED_CLOUD,
}DEV_STATUS_T;

typedef struct id_info {
	et_uchar appkey[APPKEY_LEN];
	et_uchar uid[UID_LEN];
	et_uchar secretkey[SECRETKEY_LEN];
}id_info_t;

struct et_server_info {
	et_uchar lb_port[ET_LB_PORT_MAX];
	et_uchar lb_addr[ET_LB_ADDR_MAX];
};
typedef struct et_server_info et_server_info_t;

typedef struct key_gpio{
	et_uchar key_num;
	et_uint32 key_gpio_pin;
}key_gpio_t;

typedef struct msg_to_net {
	et_uchar buf[MAX_MSG_LEN];
	et_uint32 len;
}msg_to_net_t;

typedef struct msg_to_uart {
	et_uchar buf[MAX_UART_MSG_LEN];
	et_uint32 len;
}msg_to_uart_t;

typedef struct {
	et_int32 crc32;
	et_long32 file_size;
	et_char source_ip_addr[32];  //source storage ip address
	et_char port[8];
	et_char file_id[128];
} et_dfs_file_info;

void airkiss_key_init(key_gpio_t*key);
et_uint32 user_get_run_mode();
et_int32 parse_msg_from_mqtt(et_uchar *msg_buf, et_int32 data_len);
void DHT11_init();
et_int32 DHT11_read_temp_hum(et_uint8 *buf, et_uint32 len);
et_long32 barometric_collect();

void wifi_led_spark(void *param);
void wifi_led_on();
void wifi_led_off();
void wifi_led_init();
void set_wifi_spark_timer(et_uint32 time);
void disarm_wifi_spark_timer();

void print_hex(et_uchar *label, et_uchar *str, et_int32 len);
et_uchar check_sum(et_uchar *buf, et_int32 len);
et_uint32 ascii_2_dec(et_int32 ascii);
void user_show_logo();
et_int32 down_file(et_dfs_file_info *file_info);
void audio_init(void);
#endif

