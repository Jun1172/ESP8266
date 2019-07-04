/**
 * Copyright (c) 2016 ET iLink.
 * All rights reserved.
 */
/**
 * 
 * @file et_types.h
 * @brief ���ļ�����iLink U-SDK��ʹ�õ��������ͼ����ݽṹ
 * @date 3/10/2016
 * @author wangjia
 * 
 */
#ifndef ET_TYPE_H
#define ET_TYPE_H

#include "et_config.h"

#define ET_NET_IP_LEN_MAX			32
#define ET_NET_PORT_LEN_MAX			8
#define ET_FILE_NAME_LEN_MAX        128
#define ET_FILE_ID_LEN_MAX			128
#define ET_FILE_DESCN_LEN_MAX       32

/**
 * @name �ض���ı�׼��������
 * �û�����ݲ�ͬ��ƽ̨�����ʵ��޸�,�ò������ݵı仯��Ӱ������SDK
 */
///@{
typedef unsigned char       et_uchar;
typedef unsigned char		et_uint8;
typedef char                et_char;
typedef char                et_int8;

typedef unsigned short		et_ushort;
typedef unsigned short		et_uint16;
typedef short               et_short;
typedef short               et_int16;

typedef unsigned int		et_uint;
typedef unsigned int		et_uint32;
typedef int                 et_int;
typedef int                 et_int32;

typedef unsigned long		et_ulong32;
typedef long                et_long32;

typedef unsigned long long	et_uint64;
typedef long long           et_int64;

typedef float               et_float;
typedef double              et_double;

typedef et_uint32           et_size_t;
///@}

/**
 * ����bool������
 */
typedef enum{
	ET_FALSE	= 0, 
	ET_TRUE		= 1
}et_bool;

/**
 * SDK�������
 */
typedef enum{
    ET_SUCCESS                      = 0,		///< �ɹ�
    ET_FAILURE                      = -1,		///< ִ��ʧ��
    ET_ERR_BUFFER_OVERFLOW          = -2,		///< ���ݹ������ڴ����
    ET_ERR_PARAM_INVALID            = -3,       ///< ��������
    ET_ERR_ALLOC_FAILED             = -4,       ///< �����ڴ�ʧ��
    ET_ERR_CONFIG_NONSUPPORT        = -5,       ///< ��֧�ֵ�����
    ET_ERR_MUTEX_FAILED             = -6,       ///< ����mutexʧ��
    ET_ERR_ACCOUNT_INVALID          = -7,       ///< �˺���Ϣ��Ч
    ET_ERR_LB_SOCKET_FAILED         = -8,       ///< ����LB Socketʧ��
    ET_ERR_LB_DNS_FAILED            = -9,       ///< LB DNS����ʧ��
    ET_ERR_LB_CONN_FAILED           = -10,      ///< LB ����ʧ��
    ET_ERR_LB_GET_SERVER_FAILED     = -11,      ///< ��LB��ȡ��������ַʧ��
    ET_ERR_SERVER_DNS_FAILED        = -12,      ///< ���������ܷ�������ַʧ��

    //
    ET_ERR_IM_UNKNOWN               = -0x1000,      ///< δ֪��IM����
    ET_ERR_IM_ILINK_REPEAT          = -0x1001,      ///< �ظ���¼iLink
    ET_ERR_IM_ILINK_SOCKET_FAILED   = -0x1002,      ///< ����iLink socketʧ��
    ET_ERR_IM_ILINK_CONN_FAILED     = -0x1003,      ///< ����iLink������ʧ��
    ET_ERR_IM_ILINK_PROTOCOL_VER    = -0x1010,      ///< �����Ѿܾ�����֧�ֵ�Э��汾
    ET_ERR_IM_ILINK_CLIENT_ID       = -0x1011,      ///< �����Ѿܾ����Ƿ��Ŀͻ��˱�ʶ��
    ET_ERR_IM_ILINK_UNAVAILABLE     = -0x1012,      ///< �����Ѿܾ����������˲�����
    ET_ERR_IM_ILINK_UID_APPKEY      = -0x1013,      ///< �����Ѿܾ�����Ч��UID��AppKey
    ET_ERR_IM_ILINK_UNAUTHORIZED    = -0x1014,      ///< �����Ѿܾ���δ��Ȩ
    ET_ERR_IM_ILINK_TASK_CREATE     = -0x1110,      ///< ����iLink����ʧ��
    ET_ERR_IM_ILINK_UNCONNECT       = -0x1111,      ///< iLink�����ѶϿ���APIִ��ʧ��
    ET_ERR_IM_ILINK_RECV_TIMEOUT    = -0x1305,      ///< ��Ϣ���ճ��r
    ET_ERR_IM_LOCAL_REPEAT          = -0x1200,      ///< �ظ�������������
    ET_ERR_IM_LOCAL_SOCKET_FAILED   = -0x1201,      ///< ��ʼ����������Socketʧ��
    ET_ERR_IM_LOCAL_GET_LOCALIP     = -0x1202,      ///< ��ȡ����IPʧ��
    ET_ERR_IM_LOCAL_DEV_NOTIFY      = -0x1203,      ///< �����豸����֪ͨʧ��
    ET_ERR_IM_LOCAL_TASK_CREATE     = -0x1204,      ///< ������������ʧ��
    ET_ERR_IM_LOCAL_USER_LIMIT      = -0x1205,      ///< ���������Ѵﵽ����
    ET_ERR_IM_QUEUE_FULL            = -0x1300,      ///< ��Ϣ��������
    ET_ERR_IM_DATA_PARSE            = -0x1301,      ///< ���ݽ���ʧ��
    ET_ERR_IM_WAITACK_TIMEOUT       = -0x1302,      ///< �ȴ�ACK��ʱ
    ET_ERR_IM_SEND_FAILED           = -0x1303,      ///< ��Ϣ����ʧ��
    ET_ERR_IM_SOCKET_FAILURE        = -0x1304,      ///< IM socket�ӿڳ���
    ET_ERR_IM_MSG_TOOLONG           = -0x1305,      ///< IM��Ϣ����
    //
    ET_ERR_FILE_UNKNOWN             = -0x2000,      ///< δ֪���ļ�����
    ET_ERR_FILE_INFO_PARSE          = -0x2001,      ///< �ļ���Ϣ����ʧ��
    ET_ERR_FILE_INFO_TYPE           = -0x2002,      ///< �ļ���Ϣȱ��type��Ϣ �� type��Ϣ���Ϸ�
    ET_ERR_FILE_INFO_ID             = -0x2003,      ///< �ļ���Ϣȱ��id��Ϣ �� id��Ϣ���Ϸ�
    ET_ERR_FILE_INFO_NAME           = -0x2004,      ///< �ļ���Ϣȱ��name��Ϣ �� name��Ϣ���Ϸ�
    ET_ERR_FILE_INFO_SIZE           = -0x2005,      ///< �ļ���Ϣȱ��size��Ϣ �� size��Ϣ���Ϸ�
    ET_ERR_FILE_INFO_IP             = -0x2006,      ///< �ļ���Ϣȱ��ip��Ϣ �� ip��Ϣ���Ϸ�
    ET_ERR_FILE_INFO_PORT           = -0x2007,      ///< �ļ���Ϣȱ��port��Ϣ �� port��Ϣ���Ϸ�
    ET_ERR_FILE_INFO_UNACCEPT       = -0x2100,      ///< ���Ϸ����ļ���Ϣ����
    ET_ERR_FILE_CLIENT_INIT         = -0x2200,      ///< ��ʼ���ļ��ͻ���ʧ��
    ET_ERR_FILE_CONN_FAILED         = -0x2201,      ///< �����ļ�������ʧ��
    ET_ERR_FILE_FILEID_INVALID      = -0x2202,      ///< �ļ�ID��Ч
    ET_ERR_FILE_GET_HEADER          = -0x2203,      ///< ��ȡ�ļ�ͷ��Ϣʧ��
    ET_ERR_FILE_STATE_EXCEPTION     = -0x2204,      ///< �ļ�״̬�쳣
    ET_ERR_FILE_RECV_FAILED         = -0x2205,      ///< �����ļ�����ʧ��
    ET_ERR_FILE_DOWN_CB_RETURN      = -0x2206,      ///< �ļ����ػص�ִ�з���ʧ��
    ET_ERR_FILE_GET_UPLOAD_PATH     = -0x2207,      ///< ��ȡ�ļ��ϴ�·��ʧ��
    ET_ERR_FILE_UPLOAD_FAILED       = -0x2208,      ///< �ļ��ϴ�ʧ��
    ET_ERR_FILE_MESSAGE_PACKAG      = -0x2209,      ///< ��װ�ļ���Ϣʧ��
    ET_ERR_FILE_MESSAGE_SEND        = -0x220A,      ///< �����ļ���Ϣʧ��
    //
    ET_ERR_HTTP_UNKNOWN             = -0x3000,      ///< δ֪��HTTP����
    ET_ERR_HTTP_DATA_PACKAG         = -0x3001,      ///< ��װ����ʧ��
    ET_ERR_HTTP_DATA_TOOLONG        = -0x3002,      ///< ���ݳ��ȳ���
    ET_ERR_HTTP_REQUEST_SEND        = -0x3003,      ///< ���ݷ���ʧ��
    ET_ERR_HTTP_RESPONSE_RECV       = -0x3004,      ///< ����Ӧ������ʧ��
    ET_ERR_HTTP_RESPONSE_APRSE      = -0x3005,      ///< ����Ӧ������ʧ��
    ET_ERR_HTTP_APPTOKEN_INVALID	= -0x3100,		///< ��Ч��apptoken
    ET_ERR_HTTP_KEY_INVALID         = -0x3101,		///< ��Ч��appkey��secretkey
    ET_ERR_HTTP_BASE64_INVALID		= -0x3102,		///< ��Ч��base64����ʧ��
    ET_ERR_HTTP_PARA_INVALID		= -0x3103,		///< ��Ч�Ĳ���
    ET_ERR_HTTP_API_NONE			= -0x3104,		///< ��Ч��API
    ET_ERR_HTTP_EXECUTE_FAILED		= -0x3105,		///< ִ�г���
    ET_ERR_HTTP_REDO                = -0x3106,		///< �ظ�������Ч
    ET_ERR_HTTP_PERMISSION_DENIED	= -0x3107,		///< Ȩ�޲���
    ET_ERR_HTTP_SEVER_FAILED		= -0x3108,		///< HTTP֪ͨIMʧ�ܣ�������Ч
    ET_ERR_HTTP_DATA_ABSENT         = -0x3109		///< ���ݲ�����
}et_code;

/**
 * �������ѡ����NULL
 */
#ifndef NULL
    #define NULL (void *)0
#endif

/**
 * ��Ϣ���ͷ�ʽ
 */
typedef enum{
    SEND_TO_LOCAL = 1,      ///< ����Ϣ���͸����������û�
    SEND_TO_CLOUD,          ///< ����Ϣ���͸�iLink�û�
    SEND_TO_ALL,            ///< ����Ϣͬʱ���͸�iLink�������û�
#if ET_CONFIG_CLOUD_EN && ET_CONFIG_SERVER_EN
    SEND_TO_CLOUD_FIRST,    ///< ���Ƚ���Ϣ���͸�iLink�û�����iLinkδ�������͸������û�
    SEND_TO_LOCAL_FIRST     ///< ���Ƚ���Ϣ���͸������û������������û��������͸�iLink�û�
#endif
}et_send_method_type;

/**
 * ��Ϣ���񼶱�
 */
typedef enum{
    ET_QOS0 = 0,    ///< ���һ�ν���
    ET_QOS1,        ///< ����һ�ν���
    ET_QOS2         ///< ֻ��һ�ν���
}et_mes_qos_type;

/**
 * ��Ϣ����
 */
typedef enum{
    MES_CHAT_TO_CHAT = 1,       ///< ��Ե���Ϣ
    MES_FROM_GROUP_SUBTOPIC,    ///< Ⱥ��Ϣ��������Ϣ
    MES_FROM_LOCAL,             ///< ������Ϣ
    MES_FILE_TRANSFERS,         ///< �ļ���Ϣ
    MES_USER_ONLINE,            ///< �û�����֪ͨ��Ϣ
    MES_USER_OFFLINE,           ///< �û�����֪ͨ��Ϣ
    MES_USER_STATUS,            ///< �û�״̬��Ϣ
    MES_NOTICE_ADD_BUDDY,       ///< ����û�֪ͨ��Ϣ
    MES_NOTICE_REMOVE_BUDDY,    ///< ɾ���û�֪ͨ��Ϣ
    MES_ONLINE_BUDDIES_LIST,    ///< ���ߺ����б���Ϣ
    MES_CHAT_TO_CHAT_EX         ///< ��Ե���չ��Ϣ
}et_mes_type;

/**
 * �¼�����
 */
typedef enum{
    EVENT_CLOUD_CONNECT = 1,    ///< iLink�����¼�
    EVENT_CLOUD_DISCONNECT,     ///< iLink����ʱ��
    EVENT_LOGIN_KICK,           ///< ���������¼�
    EVENT_CLOUD_SEND_FAILED,    ///< iLink��Ϣ����ʧ���¼�
    EVENT_CLOUD_SEND_SUCCESS,   ///< iLink��Ϣ���ͳɹ��¼�
    EVENT_LOCAL_SEND_FAILED,    ///< ������Ϣ����ʧ���¼�
    EVENT_LOCAL_SEND_SUCCESS,   ///< ������Ϣ���ͳɹ��¼�
    EVENT_LOCAL_CONNECT,        ///< ���������¼�
    EVENT_LOCAL_DISCONNECT,     ///< ���ض����¼�
    EVENT_HTTP_INIT_SUCCESS,    ///< HTTP��ʼ���ɹ��¼�
    EVENT_HTTP_INIT_FAILED      ///< HTTP��ʼ��ʧ���¼�
}et_event_no_type;

/**
 * �ļ����ط�ʽ
 */
typedef enum{
    ET_FILE_DOWN_TO_BUFF = 1,
    ET_FILE_DOWN_TO_FILE,
    ET_FILE_DOWN_TO_CALLBACK
}et_file_down_type;

/**
 * ��Ϣ�ṹ\n
 */
typedef struct{
    et_uint16 mes_id;
    et_mes_qos_type mes_qos;
    et_uchar *payload;
    et_uint32 payload_len;
}et_context_mes_type;

/**
 * �¼��ṹ
 */
typedef struct{
    et_event_no_type event_no;
    et_int32 data;
}et_event_type;

/**
 * �ļ���Ϣ����
 */
typedef enum{
    ET_FILE_MSG_PUSH = 0,   ///< ��Ϣ���ͷ����������ļ�
    ET_FILE_MSG_PULL,       ///< ��Ϣ���ͷ���ȡ�ļ�
    ET_FILE_MSG_ERROR       ///< ��������,���ʹ���ʱ,������Ϣ�����descn��
}et_file_msg_type;
/**
 * �ļ���Ϣ�ṹ
 */
typedef struct {
    et_file_msg_type type;                      ///< �ļ���Ϣ����,5 bytes
    et_uint32 crc32;                            ///< CRC32У����,10 bytes
    et_long32 file_size;                        ///< �ļ���С,20 bytes
    et_char source_ip_addr[ET_NET_IP_LEN_MAX];  ///< �ļ��洢IP
    et_char port[ET_NET_PORT_LEN_MAX];          ///< �˿�
    et_char file_id[ET_FILE_ID_LEN_MAX];        ///< �ļ�ID
    et_char file_name[ET_FILE_NAME_LEN_MAX];    ///< �ļ���
    et_char descn[ET_FILE_DESCN_LEN_MAX];       ///< �쳣��Ϣ
} et_dfs_file_info_type;

/**
 * �����ַ��Ϣ
 */
typedef struct{
    et_char *name_ip;   ///< �������ַ
    et_uint16 port;     ///< �˿�
}et_net_addr_type;

/**
 * iLink���
 */
typedef void * et_cloud_handle;

/**
 * ����ص�����
 */
///@{
/**
 * �¼�����ص�����
 */
typedef void (*et_event_process_type)(et_event_type event);
/**
 * ��Ϣ����ص�����
 */
typedef void (* et_msg_process_Type)(et_int32 type, et_char *send_userid, et_char *topic_name,
                                     et_int32 topic_len, et_context_mes_type *message);

/**
 * �ļ����ػص�����
 * @param arg �ļ���
 * @param file_size �ļ���С
 * @param data ָ���ѽ��յ�������
 * @param current_size ���ν��յ��Ĵ�С
 *
 * @return ���մ���ɹ�����0��-1��ʾʧ��
 */
typedef et_int32 (*et_file_down_process_type) (void *arg, const et_int64 file_size, const et_char *data,
                                               const et_int32 current_size);
/**
 * �ļ��ϴ��ص�����
 * @param arg �ļ���
 * @param file_size �跢�͵��ļ���С
 * @param sock ����socket,�û�ֱ��ʹ�ø�socket����TCP����
 *
 * @return ���ʹ���ɹ��򷵻�0��-1��ʾʧ��
 */
typedef et_int32 (*et_file_uplaod_process_type) (void *arg, const et_int64 file_size, et_int32 sock);
///@}

#endif
