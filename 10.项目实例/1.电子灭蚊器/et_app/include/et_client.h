/**@mainpage
 * Copyright (c) 2016 ET iLink.
 * All rights reserved.
 *
 * iLink U-SDK�������û����豸����������Android��IOS��Web��Windows��Linux��
 * ����IMͨ�š��ļ����䡣
 *
 * \b ʹ���������£�\n
 *
 * 	1������Context��et_create_context()
 * 	2�����ӣ�et_connect()
 * 	3��������Ϣ��et_chat_to()
 *
 * \b �������̣�\n
 *
 *  1���Զ���������et_connect�д������Ӳ���et_cloud_connect_type����auto_reloginΪET_TRUE��\n
 * ��SDK���״����ӳɹ�֮�������ֶ������Զ��������û�������EVENT_CLOUD_DISCONNECT�¼��������ٵ�����������et_relogin_cloud();
 *  2���ֶ��������û���auto_relogin����ΪET_FALSEʱ����Ҫ����EVENT_CLOUD_DISCONNECT�¼���\n
 * �ڸ��¼��е���et_reconnect()��ɽ����Զ�����������ʧ�ܽ��ٴη������¼�;
 *  3���������ߣ��û��������ߣ�SDK�������Զ����������ǲ���EVENT_LOGIN_KICK�¼�;
 *
 */

/**
 * @file et_client.h
 * @brief ���ļ��ṩ��SDK������API�ӿ�
 * @date 3/11/2016
 * @author wangjia
 */
#ifndef __ET_CLIENT_H__
#define __ET_CLIENT_H__

#include "et_types.h"
#include "et_config.h"
#include "et_std.h"
#include "et_base.h"
#if ET_CONFIG_RTOS_EN
#include "et_rtos.h"
#endif

/**
 * �û���־��ӡ
 * @see ET_LOG_USER_EN
 */
#if ET_LOG_USER_EN
#define ET_LOG_USER(format, ...)    {et_printf("User log: "format, ##__VA_ARGS__);}
#else
#define ET_LOG_GEN(format, ...)     {}
#endif

typedef struct{
	et_bool clr_offline_mes;
	et_bool auto_relogin;
	et_uint16 alive_sec;
}et_cloud_connect_type;

typedef enum{
	LIST_ONLINE = 1
}et_buddies_list_type;

/**
 * ��ȡSDK�汾��Ϣ
 * @return SDK�汾�ַ���
 */
et_char *et_get_sdk_version(void);

/**
 * ����������
 * @note �ýӿڲ����ڻص�������ʹ��
 * @brief ��et_destroy_context���ʹ��
 * @param userid ָ��UID buffer
 * @param app_key ָ��AppKey buffer
 * @param secret_key ָ��SecretKey buffer
 * @param lb_info ���ؾ����ַ��Ϣ
 *
 * @return �����ľ��\n
 *          0 ��ʾʧ��
 */
et_cloud_handle et_create_context(const et_char *userid, const et_char *app_key,    \
                                  const et_char *secret_key, et_net_addr_type lb_info);

/**
 * ����������
 * @note �ýӿڲ����ڻص�������ʹ��
 * @brief ��et_create_context����ʹ��
 * @param cloud_handle ��et_create_context����
 *
 * @return 0 ��ʾ�ɹ�\n
 *         <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32 et_destroy_context(et_cloud_handle *cloud_handle);

/**
  * ���ûص�
  * @note �ڷ�RTOSϵͳ�У��ص���ռ��SDK�����¼���
  *       ��RTOSϵͳ�У��ص������������ջ�������ڻص��н��й��ڸ��ӵĲ���
  * @param cloud_handle ��et_create_context����
  * @param mes_call ��Ϣ�ص�,���@see et_msg_process_Type
  * @param event_call �¼��ص�,���@see et_event_process_type
  *
  * @return 0 ��ʾ�ɹ�\n
  *         <0 ��ʾʧ�ܣ����@see et_code
  */
et_int32 et_set_callback(et_cloud_handle cloud_handle, et_msg_process_Type mes_call, et_event_process_type event_call);

#if ET_CONFIG_SERVER_EN
/**
 * ������ѯ
 * @note �û���Ҫ���û�������ѭ�����øýӿ�
 * @param cloud_handle ��et_create_context����
 *
 * @return 0 ��ʾ�ɹ�\n
 *         <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32 et_server_loop(et_cloud_handle cloud_handle);

/**
 * ������������
 * @note �����ڻص������е��øýӿ�
 * @param cloud_handle ��et_create_context����
 *
 * @return 0 ��ʾ�ɹ�\n
 *         <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32 et_start_server(et_cloud_handle cloud_handle);
/**
 * ֹͣ��������
 * @note �����ڻص������е��øýӿ�
 * @param cloud_handle ��et_create_context����
 *
 * @return 0 ��ʾ�ɹ�\n
 *         <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32 et_stop_server(et_cloud_handle cloud_handle);
/**
 * ��ȡ���������û�
 * @param cloud_handle ��et_create_context����
 * @param local_users_buf ���Σ������������û���UID�洢buffer
 * @param users_num local_users_buf����洢��UID���������ö�ά����ĵ�һά��С
 *
 * @return ��ȡ����UID��\n
 *         <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32 et_get_local_users(et_cloud_handle cloud_handle, et_char local_users_buf[][ET_USER_ID_LEN_MAX], et_uint32 users_num);
#endif

#if ET_CONFIG_CLOUD_EN
/**
 * ������ѯ
 * @note �û���Ҫ���û�������ѭ�����øýӿ�
 * @param cloud_handle ��et_create_context����
 *
 * @return 0 ��ʾ�ɹ�\n
 *         <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32  et_ilink_loop(et_cloud_handle cloud_handle);

/**
 * iLink����
 * @note ��et_disconnect����ʹ�ã��������ص������ @see et_reconnect;\n
 * �����ڻص������е��øýӿ�
 * @param cloud_handle ��et_create_context����
 * @param connect_para ���Ӳ��������@see et_cloud_connect_type
 *
 * @return 0 ��ʾ�ɹ�\n
 *         <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32 et_connect(et_cloud_handle cloud_handle, et_cloud_connect_type connect_para);

/**
 * ����iLink
 * @note ����������������øú����ص�
 * @param cloud_handle ��et_create_context����
 * @param connect_para ���Ӳ��������@see et_cloud_connect_type
 *
 * @param 0 ��ʾ�ɹ�\n
 *        <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32 et_reconnect(et_cloud_handle cloud_handle, et_cloud_connect_type connect_para);

/**
 * ��ȡ�û�״̬
 * @note ��ȡ�ɹ��󣬽�����Ϣ�ص����յ�MES_USER_STATUS������Ϣ
 * @param cloud_handle ��et_create_context����
 * @param userid ������ȡ���û�UID
 *
 * @return 0 ��ʾ�ɹ�\n
 *         <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32 et_get_user_state(et_cloud_handle cloud_handle, et_char *userid);

/**
 * ״̬����
 * @note ���ĳɹ���userid�û������߽�MES_USER_ONLINE��MES_USER_OFFLINE������Ϣ
 * @param cloud_handle ��et_create_context����
 * @param userid �������ĵ��û�UID
 *
 * @return 0 ��ʾ�ɹ�\n
 *         <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32 et_sub_user_state(et_cloud_handle cloud_handle, et_char *userid);

/**
 * ȡ��״̬����
 * @param cloud_handle ��et_create_context����
 * @param userid ����ȡ�����ĵ��û�UID
 *
 * @return 0 ��ʾ�ɹ�\n
 *         <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32 et_unsub_user_state(et_cloud_handle cloud_handle, et_char *userid);

/**
 * ��ȡ������Ϣ
 * @param cloud_handle ��et_create_context����
 *
 * @return 0 ��ʾ�ɹ�\n
 *         <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32 et_request_offline_message(et_cloud_handle cloud_handle);

/**
 * ��������
 * @param cloud_handle ��et_create_context����
 * @param topic ��Ϣ����
 * @param qos ��Ϣ�������@see et_mes_qos_type
 *
 * @return 0 ��ʾ�ɹ�\n
 *         <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32 et_subscribe(et_cloud_handle cloud_handle, const et_char* topic, et_mes_qos_type qos);

/**
 * ȡ����������
 * @param cloud_handle ��et_create_context����
 * @param topic ��Ϣ����
 *
 * @return 0 ��ʾ�ɹ�\n
 *         <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32 et_unsubscribe(et_cloud_handle cloud_handle, const et_char* topic);
/**
 * �Ͽ�iLink����
 * @note �����ڻص������е��øýӿ�
 * @param cloud_handle ��et_create_context����
 *
 * @return 0 ��ʾ�ɹ�\n
 *         <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32 et_disconnect(et_cloud_handle cloud_handle);
#endif
#if ET_CONFIG_CLOUD_EN || ET_CONFIG_SERVER_EN
/**
 * ���͵�Ե���Ϣ
 * @note �˴����سɹ�ֻ��ʾ�ɹ�������Ϣ���У����¼�֪ͨ��ϢID���ͳɹ�ʱ��ʾsocket���ͳɹ�
 * @param cloud_handle ��et_create_context����
 * @param data �����͵�����
 * @param len �����͵����ݳ��ȣ���λbyte
 * @param userid Ŀ��UID
 * @param send_method ���ͷ�ʽ�����@see et_send_method_type
 *
 * @return ���ͳɹ�ʱ������ϢID\n
 *         <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32 et_chat_to(et_cloud_handle cloud_handle, et_uchar *data, et_uint32 len,     \
                    const et_char *userid, et_send_method_type send_method);

/**
 * ����������Ϣ
 * @note �˴����سɹ�ֻ��ʾ�ɹ�������Ϣ���У����¼�֪ͨ��ϢID���ͳɹ�ʱ��ʾsocket���ͳɹ�
 * @param cloud_handle ��et_create_context����
 * @param data �����͵�����
 * @param len �����͵����ݳ��ȣ���λbyte
 * @param topic ��Ϣ����,�����в��ܱ����ո�#��+��&��@�������ַ�
 * @param qos ��Ϣ����
 * @param send_method ���ͷ���
 *
 * @return ET_QOS0���ͳɹ�ʱ����0,ET_QOS1��ET_QOS2������ϢUID\n
 *         <0 ��ʾʧ�ܣ����@see et_code
*/
et_int32 et_publish(et_cloud_handle cloud_handle, et_uchar *data, et_uint32 len,     \
                    const et_char *topic, et_mes_qos_type qos, et_send_method_type send_method);
#endif
#if ET_CONFIG_FILE_EN
/**
 * �����ļ���Ϣ
 * @param cloud_handle ��et_create_context����
 * @param file_str ���������ļ���Ϣ
 * @param file_info ���Σ����������ļ���Ϣ�洢�ṹ�����@see et_dfs_file_info_type
 *
 * @return 0 ��ʾ�ɹ�\n
 *         <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32 et_file_info(et_cloud_handle cloud_handle, et_char *file_str, et_dfs_file_info_type *file_info);

/**
 * �����ļ�
 * @note ����iLink����
 * @param cloud_handle ��et_create_context����
 * @param file_info �����ص��ļ���Ϣ
 * @param down_cb �ļ����ػص��������ļ�������ɺ���ã�����Ҫ�ɴ�NULL
 *
 * @return 0 ��ʾ�ɹ�\n
 *         <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32 et_download_file(et_cloud_handle cloud_handle, et_dfs_file_info_type *file_info,   \
                      et_file_down_process_type down_cb);
/**
 * �ϴ��ļ�
 * @brief ���ļ��ϴ����ļ�������,���޽��շ��������ڴ洢
 * @note ����iLink����
 * @param cloud_handle ��et_create_context����
 * @param file_name ���ϴ����ļ���
 * @param file_size ���ϴ����ļ���С����λbyte,�������0
 * @param file_info ���Σ����͵��ļ���Ϣ
 * @param upload_cb �����ļ��ص�,�û��ڻص���ʹ��TCP�����ļ�����,���@see et_file_uplaod_process_type
 *
 * @return 0 ��ʾ�ɹ�\n
 *         <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32 et_upload_file(et_cloud_handle cloud_handle, et_int8 *file_name, et_uint32 file_size,   \
                        et_dfs_file_info_type *file_info, et_file_uplaod_process_type upload_cb);

/**
 * ���������ļ���iLink�û�
 * @brief ���ļ��ϴ����ļ���������ָ������UID��ָ���Ľ��շ����յ�һ���ļ���Ϣ
 * @note ����iLink����
 * @param cloud_handle ��et_create_context����
 * @param userid ���շ�UID
 * @param file_name �����͵��ļ���
 * @param file_size �����͵��ļ���С����λbyte,�������0
 * @param file_info ���Σ����͵��ļ���Ϣ
 * @param upload_cb �����ļ��ص�,�û��ڻص���ʹ��TCP�����ļ�����,���@see et_file_uplaod_process_type
 *
 * @return 0 ��ʾ�ɹ�\n
 *         <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32 et_file_to(et_cloud_handle cloud_handle, et_int8 *userid, et_int8 *file_name, et_uint32 file_size, \
                    et_dfs_file_info_type *file_info,et_file_uplaod_process_type upload_cb);
#endif

#if !ET_CONFIG_RTOS_EN
/**
 * IM��ѯ
 * @note ���û��ڷ�RTOS������ʹ��SDKʱ����ø�API������ѯ
 * @param cloud_handle ��et_create_context����
 *
 * @return 0 ��ʾ�ɹ�
 *         <0 ��ʾʧ�ܣ����@see et_code
 */
et_int32 et_im_loop(et_cloud_handle cloud_handle);
#endif

#if ET_CONFIG_HTTP_EN

/**
 * ��Ӻ���
 * @param cloud_handle ��et_create_context����
 * @param friendid ����ID
 *
 * @return 0��ʾ�ɹ�\n
 *         <0��ʾʧ�ܣ����@see et_code
 */
et_int32 et_add_buddy(et_cloud_handle cloud_handle, const et_char * friendid);

/**
 * ��Ӻ��Ѳ�֪ͨ
 * @note ����������֪ͨ����Ҫ���û��������߲��ܲ����ɹ�
 * @param cloud_handle ��et_create_context����
 * @param friendid ����ID
 * @param notify �Ƿ�֪ͨ��ET_TRUE-֪ͨ��ET_FALSE-��֪ͨ
 *
 * @return 0��ʾ�ɹ�\n
 *         <0��ʾʧ�ܣ����@see et_code
 */
et_int32 et_add_buddy_notify(et_cloud_handle cloud_handle, const et_char * friendid, et_bool notify);

/**
 * ɾ������
 * @param cloud_handle ��et_create_context����
 * @param friendid ����ID
 *
 * @return 0��ʾ�ɹ�\n
 *         <0��ʾʧ�ܣ����@see et_code
 */
et_int32 et_remove_buddy(et_cloud_handle cloud_handle, const et_char * friendid);

/**
 * ɾ�����Ѳ�֪ͨ
 * @note ����������֪ͨ����Ҫ���û��������߲��ܲ����ɹ�
 * @param cloud_handle ��et_create_context����
 * @param friendid ����ID
 * @param notify �Ƿ�֪ͨ��ET_TRUE-֪ͨ��ET_FALSE-��֪ͨ
 *
 * @return 0��ʾ�ɹ�\n
 *         <0��ʾʧ�ܣ����@see et_code
 */
et_int32 et_remove_buddy_notify(et_cloud_handle cloud_handle, const et_char * friendid, et_bool notify);

/**
 * ��ȡ�����б�
 * @param cloud_handle ��et_create_context����
 * @param friends_list ���Σ�����UID�洢buffer
 * @param list_num friends_list��һά��С
 * @param buddies_num ���Σ��洢��friends_list�е�UID����������Ҫ��ȡ��NULL
 * @param buddies_num_max ���Σ���ȡ���ĺ���UID��������Ҫ��ȡ��NULL
 * @param summary_info ��ȡ��Ҫ��Ϣ�������ã�ET-TRUE��ֻ��ȡUID��Ϣ��ET-FALSE:��ȡ��������Ϣ������UID��nickname��username
 *
 * @return 0��ʾ�ɹ�\n
 *         <0��ʾʧ�ܣ����@see et_code
 */
et_int32 et_get_buddies(et_cloud_handle cloud_handle, et_char friends_list[ ] [ ET_USER_ID_LEN_MAX ],   \
                        et_uint32 list_num, et_int32 * buddies_num, et_int32 *buddies_num_max, et_bool summary_info);


/**
 * ����Ⱥ
 * @param cloud_handle ��et_create_context����
 * @param group_name Ⱥ����
 * @param members_list �����Ⱥ��Ⱥ��ԱUID�б�
 * @param members_num members_list�д洢��UID����
 * @param group_topic ���Σ�������Ⱥtopic��ϵͳΨһ��ʶ
 * @param group_topic_size group_topic�Ĵ�С����λbyte
 *
 * @return 0��ʾ�ɹ�\n
 *         <0��ʾʧ�ܣ����@see et_code
 */
et_int32 et_create_group(et_cloud_handle cloud_handle, const et_char * group_name,     \
                         const et_char members_list[][ET_USER_ID_LEN_MAX], et_uint32 members_num, 	\
                         et_char *group_topic, et_int32 group_topic_size);

/**
 * ��ȡȺ�б�
 * @param cloud_handle ��et_create_context����
 * @param groups_list ���Σ�Ⱥ�б�buffer
 * @param list_num  groups_list�ɴ洢��ȺTopic����������һά�Ĵ�С
 * @param groups_num ���Σ��洢��groups_list�е�Topic����������Ҫ��NULL
 * @param groups_num_max ���Σ���ȡ����ȺTopic����������Ҫ��NULL
 * @param summary_info ��ȡ��Ҫ��Ϣ���ã�ET_TRUE��ֻ��ȡȺTopic��ET_FAISE����ȡ��������Ϣ������groupname��grouptopic
 *
 * @return 0��ʾ�ɹ�\n
 *         <0��ʾʧ�ܣ����@see et_code
 */
et_int32 et_get_groups(et_cloud_handle cloud_handle, et_char groups_list[][ET_TOPIC_LEN_MAX], et_uint32 list_num, 	\
                       et_int32 *groups_num, et_int32 *groups_num_max, et_bool summary_info);
/**
 * ��ɢȺ
 * @note ֻ��Ⱥ�Ĵ����߲ſɽ�ɢȺ
 * @param cloud_handle ��et_create_context����
 * @param groups_topic ������ɢ��ȺΨһ��ʶ��Ⱥtopic
 *
 * @return 0��ʾ�ɹ�\n
 *         <0��ʾʧ�ܣ����@see et_code
 */
et_int32 et_destroy_group(et_cloud_handle cloud_handle, const et_char *groups_topic);

/**
 * �˳�Ⱥ
 * @param cloud_handle ��et_create_context����
 * @param groups_topic �����˳���ȺΨһ��ʶ��Ⱥtopic
 *
 * @return 0��ʾ�ɹ�\n
 *         <0��ʾʧ�ܣ����@see et_code
 */
et_int32 et_exit_group(et_cloud_handle cloud_handle, const et_char *group_topic);

/**
 * ���Ⱥ��Ա
 * @param cloud_handle ��et_create_context����
 * @param groups_topic ��ӵ���ȺΨһ��ʶ��Ⱥtopic
 * @param members_list ����ӵĳ�ԱUID�б�
 * @param members_num members_list�е�UID����
 *
 * @return 0��ʾ�ɹ�\n
 *         <0��ʾʧ�ܣ����@see et_code
 */
et_int32 et_add_group_members(et_cloud_handle cloud_handle, const et_char *group_topic,      \
                              const et_char members_list[][ET_USER_ID_LEN_MAX], et_uint32 members_num);

/**
 * ɾ��Ⱥ��Ա
 * @param cloud_handle ��et_create_context����
 * @param groups_topic ɾ����Ա��ȺΨһ��ʶ��Ⱥtopic
 * @param members_list ��ɾ���ĳ�ԱUID�б�
 * @param members_num members_list�е�UID����
 *
 * @return 0��ʾ�ɹ�\n
 *         <0��ʾʧ�ܣ����@see et_code
 */
et_int32 et_remove_group_members(et_cloud_handle cloud_handle, const et_char *group_topic,   \
                                 const et_char members_list[][ET_USER_ID_LEN_MAX], et_uint32 members_num);

/**
 * ��ȡȺ��Ա�б�
 * @param cloud_handle ��et_create_context����
 * @param groups_topic ������ȡ��ȺΨһ��ʶ��Ⱥtopic
 * @param members_list ���Σ�Ⱥ��ԱUID�洢buffer
 * @param list_num members_list�пɴ洢��UID����
 * @param members_num ���Σ��洢��members_list�е�UID����������Ҫʱ��NULL
 * @param members_num_max ���Σ���ȡ����Ⱥ��Ա����������Ҫʱ��NULL
 *
 * @return 0��ʾ�ɹ�\n
 *         <0��ʾʧ�ܣ����@see et_code
 */
et_int32 et_get_group_members(et_cloud_handle cloud_handle, const et_char *group_topic,      \
                              et_char members_list[][ET_USER_ID_LEN_MAX], et_uint32 list_num, 	\
                              et_int32 *members_num, et_int32 *members_num_max);

/**
 * ����û�
 * @param cloud_handle ��et_create_context����
 * @param http_server HTTP��������ַ��ӦΪ����IP���˿ڵ��ַ�����e.g."192.168.1.1:1000"
 * @note ��http_server��������NULL��ʹ��et_http_init��ʼ�����ĵ�ַ
 * @param app_key ����û�����appkey,ע��ҵ��ʱ���
 * @param secret_key ע��ҵ��ʱ���
 * @param user_name �û��˺�
 * @param userid ���Σ�ָ��洢UID��buffer
 * @param userid_size userid�Ĵ�С����λbyte
 *
 * @return 0��ʾ�ɹ�\n
 *         <0��ʾʧ�ܣ����@see et_code
 */
et_int32 et_add_user(et_cloud_handle cloud_handle, const et_char *http_server, const et_char *app_key,     \
                     const et_char *secret_key, const et_char *user_name, et_char *userid, et_int32 userid_size);
#endif
#endif

