/**
 * Copyright (c) 2016 ET iLink.
 * All rights reserved.
 */
/**
 *
 * @file et_base.h 
 * @brief ���ļ������Ľӿ�ΪӲ����ؽӿڣ��û�����ֲʱ����ʵ��
 * @date 3/10/2016
 * @author wangjia
 *
 */
#ifndef ET_BASEAPI_H
#define ET_BASEAPI_H

#include "et_types.h"
#include "et_config.h"

/*******************************************************��******************************************************/

#define ET_IPSTR_LEN_MAX    32          ///< IP��ַ�ַ����洢buffer��С

/**
 * @name SDK�����Ķ˿�
 * @note �û��޸���Ч
 */
///@{
#define ET_LISTEN_PORT_TCP        18883 ///< TCP����˿�
#define ET_LISTEN_PORT_UDP        18886 ///< UDP����˿�
#define ET_LISTEN_PORT_UDP_BROAD  2073  ///< UDP�㲥�˿�
///@}

#define ET_SOCKET_NULL            ((et_socket_t)-1)    ///< ��Ч��Socket�����ڱ�ʾSocket����ʧ�ܷ���
/*****************************************************time�ӿ�**************************************************/
/**
 * ��ȡϵͳʱ�䣬��λΪms
 * @note ��ʱ��Ϊϵͳ���е����øýӿڵ�ms����ֵ��SDK��Ҫ�жϳ�ʱ������ʱ�����øýӿ�
 *
 * @return ��ǰ��ϵͳʱ��������λΪms
 */
extern et_uint32 et_system_time_ms(void);

/**
 * ˯�ߵ�ǰ�߳�,��λΪms
 * @note �����Ƿ���RTOS��SDK�������øú���˯�ߵ�ǰtask
 * @param milliseconds ��Ҫ˯�ߵ�ʱ��
 */
extern void et_sleep_ms(et_int32 milliseconds);


/****************************************************socket�ӿ�*************************************************/
/**
 * socket����
 * @note �û�������TCp/IPЭ��ջsocketʵ������޸�
 */
typedef et_int32 et_socket_t;

/**
 * TCP/IP��ַ��Ϣ
 */
typedef struct{
    et_int8 ip_str[ET_IPSTR_LEN_MAX];  ///< IP,e.g:"192.168.10.1"
    et_uint16 port;                ///< �˿�
} et_addr_info_t;

/**
 * socket ����
 */
typedef enum{
    ET_SOCKET_UDP,		///< UDP socket
    ET_SOCKET_TCP		///< TCP socket
} et_socket_proto_t;

/**
 * ����socket
 * @note �ýӿ�Ӧ���Դ���UDP��TCP socket��SDK�ڲ������øýӿ�ͬʱ�������UDP��TCP socket\n
 *		 ����UDP socketӦ�ܽ��չ㲥����TCP socketΪ������.
 * @param type socket����
 *
 * @return socketֵ
 *         @see ET_SOCKET_NULL ��ʾ����ʧ��
 */
extern et_socket_t et_socket_create(et_socket_proto_t type);

/**
 * �ر�socket
 * @note 
 * @param socket ��et_socket_create����
 *
 * @return 0 ��ʾ�ɹ�
 * 		   -1 ��ʾʧ��
 */
extern et_int32 et_socket_close(et_socket_t socket);

/**
 * ���ӷ�����
 * @param socket ��et_socket_create����
 * @param ip ������IP��ַ����ʽΪ�Խ�����'\0'��β���ַ������硰192.168.1.10��
 * @param port �������Ͽ���
 * @param time_out_ms ��ʱʱ�䣬��λΪms;ϣ��ʹ��Ĭ��ֵʱ��0��
 *
 * @return 0 ��ʾ�ɹ�
 * 		   -1 ��ʾʧ��
 */
extern et_int32 et_socket_connect(et_socket_t socket, et_int8 *ip, et_uint16 port, et_uint32 time_out_ms);

/**
 * TCP��������
 * @note ע�����ַ����ֽ���Ϊ0�ͳ���ʧ��
 * @param socket ��et_socket_create����
 * @param send_buf �������ݻ���
 * @param buf_len ���������ݵĳ��ȣ���λΪByte
 * @param time_out_ms ��ʱʱ�䣬��λΪms
 *
 * @return �ɹ��ķ����ѷ��͵����ݳ��ȣ���������ͷ
 * 		0 ��ʾ�����ݷ���\n
 * 		-1 ��ʾ���ͳ���
 *
 * @code
 * //���ݷ���ʾ��
 * et_int32 et_socket_send(et_socket_t socket, et_uint8 *send_buf, et_uint32 buf_len, et_uint32 time_out_ms)
 * {
 *  	struct timeval timer = {time_out_ms/1000, time_out_ms%1000*1000};
 *  	fd_set write_set;
 *  	et_int32 rc = 0;
 *
 *  	FD_ZERO(&write_set);
 *  	FD_SET(socket, &write_set);
 *
 *  	if((rc = select(socket+1, NULL, &write_set, NULL, &timer)) > 0)
 *  		rc = send(socket, send_buf, buf_len, 0);
 *  	if(rc < 0 && !(errno == 0 || errno == EINPROGRESS || errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
 *  	{
 *  		rc = ET_FAILURE;
 *  	}
 *  	return rc;
 *  }
 * @endcode
 */
extern et_int32 et_socket_send(et_socket_t socket, et_uint8 *send_buf, et_uint32 buf_len, et_uint32 time_out_ms);

/**
 * TCP��������
 * @note 1��ע�����ֽ���Ϊ0�ͳ���ʧ�ܣ����س���ʱSDK���ر�socket;\n
 *       2����time_out_ms���� 0 ʱ,��ǰ�����ݽ�������������;\n
 *       3��SDK����ע��ǰ���ڶ������ݣ���SDK��Ҫһ����������ʱ���øýӿڻ�ȡ,\n
 *       �п���ֻ��ȡ��ǰ�������ݵ�һ���֣�ʣ��������һ���ٴζ�ȡ��
 *       4����TCP�����ѶϿ����ýӿ�Ӧ����-1
 * @param socket ��et_socket_create����
 * @param recv_buf �������ݻ���
 * @param buf_len ���ݴ洢���峤�ȣ���λΪByte
 * @param time_out_ms ��ʱʱ�䣬��λΪms
 *
 * @return �ɹ��ķ����ѷ��͵����ݳ��ȣ���������ͷ
 * 		0 ��ʾ�����ݽ���\n
 * 		-1 ��ʾ����������ѶϿ�
 * @code
 * //���ݽ���ʾ��
 * et_int32 et_socket_recv(et_socket_t socket, et_uint8 *recv_buf, et_uint32 buf_len, et_uint32 time_out_ms)
 * {
 *  	struct timeval timer = {0, 0};
 *  	fd_set read_set;
 *  	et_int32 rc =0;
 *      timer.tv_sec = time_out_ms/1000;
 *  	timer.tv_usec = time_out_ms%1000*1000;
 *  	FD_ZERO(&read_set);
 *  	FD_SET(socket, &read_set);
 *
 *  	if((rc = select(socket+1, &read_set, NULL, NULL, &timer)) > 0)
 *  		rc = recv(socket, recv_buf, buf_len, 0);
 *  	if(rc < 0 && !(errno == 0 || errno == EINTR || errno == EAGAIN || errno == EINPROGRESS || errno == EWOULDBLOCK))	///< Socket error
 *  	{
 *  		ET_LOG_USER("recv failed %d\n", errno);
 *  		rc = ET_FAILURE;
 *  	}
 *  	return rc;
 *  }
 * @endcode
 */
extern et_int32 et_socket_recv(et_socket_t socket, et_uint8 *recv_buf, et_uint32 buf_len, et_uint32 time_out_ms);

#if ET_CONFIG_SERVER_EN
/**
 * ��socket
 * @note ��SDK��Ҫ����ʱ�����øýӿ�ΪSocket�󶨶˿ڣ�SDK�����İ󶨵�IP
 * @param socket ��et_socket_create����
 * @param port �����Socket�Ķ˿�
 *
 * @return 0 ��ʾ�ɹ�
 * 		   -1 ��ʾʧ��
 */
extern et_int32 et_socket_bind(et_socket_t socket, et_uint16 port);

/**
 * �����˿�
 * @note SDK�ڲ�����Ҫ�����˿�ʱ����һ��\n
 *       ��SDK���й�����listenһ���˿ں󲻻���ȡ��listen
 * @param socket ��et_socket_create����
 *
 * @return 0 ��ʾ�ɹ�
 *         -1 ��ʾʧ��
 */
extern et_int32 et_socket_listen(et_socket_t socket);

/**
 * �������˿�����
 * @note SDK�����˿ں󣬽����øýӿڼ�����ӵ��ö˿ڵ�����
 * @param socket ��et_socket_create����
 * @param remote_info ���Σ�Զ��������Ϣ�洢ָ��
 *
 * @return �������ӵı���socket
 *         @see ET_SOCKET_NULL��ʾ������
 */
extern et_socket_t et_socket_accept(et_socket_t socket, et_addr_info_t *remote_info);

/**
 * UDP��������
 * @note ��������������ʱ��ʵ�ָýӿ�
 * @param socket ��et_socket_create����
 * @param send_buf ָ������͵�����buffer
 * @param buf_len �����͵����ݳ��ȣ���λΪByte
 * @param remote_info Զ��������Ϣ
 * @param time_out_ms ��ʱʱ�䣬��λΪms
 *
 * @return ���ͳɹ������ݳ���\n
 *         -1 ��ʾʧ��
 */
extern et_int32 et_socket_send_to(et_socket_t socket, et_uint8 *send_buf, et_uint32 buf_len, et_addr_info_t *remote_info,
                                  et_uint32 time_out_ms);

/**
 * UDP��������
 * @note ��������������ʱ��Ҫʵ�ָýӿ�
 * @param socket ��et_socket_create����
 * @param recv_buf ָ�����buffer
 * @param buf_len ����buffer��С����λΪByte
 * @param remote_info ���Σ�Զ��������Ϣ
 * @param time_out_ms ��ʱʱ�䣬��λΪms
 *
 * @return �������ݳ���
 *         -1 ��ʾʧ��
*/
extern et_int32 et_socket_recv_from(et_socket_t socket, et_uint8 *recv_buf, et_uint32 buf_len, et_addr_info_t *remote_info,
                                    et_uint32 time_out_ms);

/**
 * ��ȡ����IP
 * @param local_ip ������IP�洢Buffer����ʽΪ�ַ���������"192.168.1.10"
 * @param size local_ip��С����λbyte
 *
 * @return 0 ��ȡ�ɹ�
 *         -1 ��ȡʧ��
 */
extern et_int32 et_get_localip(et_int8 *local_ip, et_uint32 size);
#endif

/**
 * ͨ������������õ�ַ��Ϣ
 * @note �������ص�IP��ַ��Ϣ����Ϊ��'\0'��β���ַ���\n
         �ýӿ�ӦΪͬ���ӿڣ�SDK���øýӿ�ʱ����ֱ�����سɹ���ʧ��
 * @param name ������IP�˿���Ϣ����ʽΪ�Խ�����'\0'��β���ַ�������"www.kaifakuai.com"��192.168.1.10��
 * @param addr ���Σ��洢��ȡ����IP��ַ,@see et_addr_info_t�еĶ˿���Ϣ�ɲ���
 * @param time_out_ms ��ʱʱ�䣬��λΪms
 *
 * @return 0 ��ʾ�ɹ�\n
 *         -1 ��ʾʧ��
 *
 */
extern et_int32 et_gethostbyname(et_int8 *name, et_addr_info_t *addr, et_uint32 time_out_ms);


#endif
