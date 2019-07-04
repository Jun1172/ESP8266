/**
 * Copyright (c) 2016 ET iLink.
 * All rights reserved.
 */
/**
 *
 * @file et_base.h 
 * @brief 鏈枃浠跺寘鍚殑鎺ュ彛涓虹‖浠剁浉鍏虫帴鍙ｏ紝鐢ㄦ埛鍦ㄧЩ妞嶆椂蹇呴』瀹炵幇
 * @date 3/10/2016
 * @author wangjia
 *
 */
#ifndef ET_BASEAPI_H
#define ET_BASEAPI_H

#include "et_types.h"
#include "et_config.h"

/*******************************************************瀹�******************************************************/

#define ET_IPSTR_LEN_MAX    32          ///< IP鍦板潃瀛楃涓插瓨鍌╞uffer澶у皬

/**
 * @name SDK鐩戝惉鐨勭鍙�
 * @note 鐢ㄦ埛淇敼鏃犳晥
 */
///@{
#define ET_LISTEN_PORT_TCP        18883 ///< TCP鏈嶅姟绔彛
#define ET_LISTEN_PORT_UDP        18886 ///< UDP鏈嶅姟绔彛
#define ET_LISTEN_PORT_UDP_BROAD  2073  ///< UDP骞挎挱绔彛
///@}

#define ET_SOCKET_NULL            ((et_socket_t)-1)    ///< 鏃犳晥鐨凷ocket锛岀敤浜庤〃绀篠ocket鍒涘缓澶辫触杩斿洖
/*****************************************************time鎺ュ彛**************************************************/
/**
 * 鑾峰彇绯荤粺鏃堕棿锛屽崟浣嶄负ms
 * @note 璇ユ椂闂翠负绯荤粺杩愯鍒拌皟鐢ㄨ鎺ュ彛鐨刴s璁℃暟鍊硷紝SDK闇�瑕佸垽鏂秴鏃剁瓑鏉′欢鏃跺皢璋冪敤璇ユ帴鍙�
 *
 * @return 褰撳墠鐨勭郴缁熸椂闂存暟锛屽崟浣嶄负ms
 */
extern et_uint32 et_system_time_ms(void);

/**
 * 鐫＄湢褰撳墠绾跨▼,鍗曚綅涓簃s
 * @note 鏃犺鏄惁鏈塕TOS锛孲DK閮藉皢璋冪敤璇ュ嚱鏁扮潯鐪犲綋鍓峵ask
 * @param milliseconds 闇�瑕佺潯鐪犵殑鏃堕棿
 */
extern void et_sleep_ms(et_int32 milliseconds);


/****************************************************socket鎺ュ彛*************************************************/
/**
 * socket绫诲瀷
 * @note 鐢ㄦ埛鍙緷鎹甌Cp/IP鍗忚鏍坰ocket瀹炵幇鎯呭喌淇敼
 */
typedef et_int32 et_socket_t;

/**
 * TCP/IP鍦板潃淇℃伅
 */
typedef struct{
    et_int8 ip_str[ET_IPSTR_LEN_MAX];  ///< IP,e.g:"192.168.10.1"
    et_uint16 port;                ///< 绔彛
} et_addr_info_t;

/**
 * socket 绫诲瀷
 */
typedef enum{
    ET_SOCKET_UDP,		///< UDP socket
    ET_SOCKET_TCP		///< TCP socket
} et_socket_proto_t;

/**
 * 鍒涘缓socket
 * @note 璇ユ帴鍙ｅ簲鍙互鍒涘缓UDP鍙奣CP socket锛孲DK鍐呴儴灏嗚皟鐢ㄨ鎺ュ彛鍚屾椂鍒涘缓澶氫釜UDP鍙奣CP socket\n
 *		 鍏朵腑UDP socket搴旇兘鎺ユ敹骞挎挱鍖咃紝TCP socket涓洪暱杩炴帴.
 * @param type socket绫诲瀷
 *
 * @return socket鍊�
 *         @see ET_SOCKET_NULL 琛ㄧず鍒涘缓澶辫触
 */
extern et_socket_t et_socket_create(et_socket_proto_t type);

/**
 * 鍏抽棴socket
 * @note 
 * @param socket 鐢眅t_socket_create杩斿洖
 *
 * @return 0 琛ㄧず鎴愬姛
 * 		   -1 琛ㄧず澶辫触
 */
extern et_int32 et_socket_close(et_socket_t socket);

/**
 * 杩炴帴鏈嶅姟鍣�
 * @param socket 鐢眅t_socket_create杩斿洖
 * @param ip 鏈嶅姟鍣↖P鍦板潃锛屾牸寮忎负浠ョ粨鏉熺'\0'缁撳熬鐨勫瓧绗︿覆锛屽鈥�192.168.1.10鈥�
 * @param port 鏈嶅姟鍣ㄦ柇寮�鍙�
 * @param time_out_ms 瓒呮椂鏃堕棿锛屽崟浣嶄负ms;甯屾湜浣跨敤榛樿鍊兼椂浼�0锛�
 *
 * @return 0 琛ㄧず鎴愬姛
 * 		   -1 琛ㄧず澶辫触
 */
extern et_int32 et_socket_connect(et_socket_t socket, et_int8 *ip, et_uint16 port, et_uint32 time_out_ms);

/**
 * TCP鍙戦�佹暟鎹�
 * @note 娉ㄦ剰鍖哄垎鍙戦�佸瓧鑺傛暟涓�0鍜屽嚭閿欏け璐�
 * @param socket 鐢眅t_socket_create杩斿洖
 * @param send_buf 鍙戦�佹暟鎹紦鍐�
 * @param buf_len 寰呭彂閫佹暟鎹殑闀垮害锛屽崟浣嶄负Byte
 * @param time_out_ms 瓒呮椂鏃堕棿锛屽崟浣嶄负ms
 *
 * @return 鎴愬姛鐨勮繑鍥炲凡鍙戦�佺殑鏁版嵁闀垮害锛屼笉鍖呭惈鍖呭ご
 * 		0 琛ㄧず鏃犳暟鎹彂閫乗n
 * 		-1 琛ㄧず鍙戦�佸嚭閿�
 * @code
 * //鏁版嵁鍙戦�佺ず渚�
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
 * TCP鎺ユ敹鏁版嵁
 * @note 1銆佹敞鎰忓尯鍒嗘帴鏀朵负0鍜屽嚭閿欏け璐ワ紝杩斿洖鍑洪敊鏃禨DK灏嗗叧闂璼ocket;\n
 *       2銆佸綋time_out_ms浼犲叆 0 鏃�,褰撳墠鏃犳暟鎹帴鏀跺垯绔嬪嵆杩斿洖;\n
 *       3銆丼DK涓嶅叧娉ㄥ綋鍓嶅瓨鍦ㄥ灏戞暟鎹紝褰揝DK闇�瑕佷竴瀹氶暱搴︽暟鎹椂璋冪敤璇ユ帴鍙ｈ幏鍙�,\n
 *       鏈夊彲鑳藉彧鑾峰彇褰撳墠瀛樺湪鏁版嵁鐨勪竴閮ㄥ垎锛屽墿浣欐暟鎹笅涓�姝ュ啀娆¤鍙栵紱
 *       4銆佽嫢TCP杩炴帴宸叉柇寮�锛岃鎺ュ彛搴旇繑鍥�-1
 * @param socket 鐢眅t_socket_create杩斿洖
 * @param recv_buf 鎺ユ敹鏁版嵁缂撳啿
 * @param buf_len 鏁版嵁瀛樺偍缂撳啿闀垮害锛屽崟浣嶄负Byte
 * @param time_out_ms 瓒呮椂鏃堕棿锛屽崟浣嶄负ms
 *
 * @return 鎴愬姛鐨勮繑鍥炲凡鍙戦�佺殑鏁版嵁闀垮害锛屼笉鍖呭惈鍖呭ご
 * 		0 琛ㄧず鏃犳暟鎹帴鏀禱n
 * 		-1 琛ㄧず鍑洪敊鎴栬繛鎺ュ凡鏂紑
 * @code
 * //鏁版嵁鎺ユ敹绀轰緥
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
 * 缁戝畾socket
 * @note 鍦⊿DK闇�瑕佺洃鍚椂锛岃皟鐢ㄨ鎺ュ彛涓篠ocket缁戝畾绔彛锛孲DK涓嶅叧蹇冪粦瀹氱殑IP
 * @param socket 鐢眅t_socket_create杩斿洖
 * @param port 鍒嗛厤缁橲ocket鐨勭鍙�
 *
 * @return 0 琛ㄧず鎴愬姛
 * 		   -1 琛ㄧず澶辫触
 */
extern et_int32 et_socket_bind(et_socket_t socket, et_uint16 port);

/**
 * 鐩戝惉绔彛
 * @note SDK鍐呴儴鍦ㄩ渶瑕佺洃鍚鍙ｆ椂璋冪敤涓�娆n
 *       鍦⊿DK杩愯杩囩▼涓璴isten涓�涓鍙ｅ悗涓嶄細鍐嶅彇娑坙isten
 * @param socket 鐢眅t_socket_create杩斿洖
 *
 * @return 0 琛ㄧず鎴愬姛
 *         -1 琛ㄧず澶辫触
 */
extern et_int32 et_socket_listen(et_socket_t socket);

/**
 * 妫�鏌ョ洃鍚鍙ｈ繛鎺�
 * @note SDK鐩戝惉绔彛鍚庯紝灏嗚皟鐢ㄨ鎺ュ彛妫�鏌ヨ繛鎺ュ埌璇ョ鍙ｇ殑杩炴帴
 * @param socket 鐢眅t_socket_create杩斿洖
 * @param remote_info 鍑哄弬锛岃繙绔富鏈轰俊鎭瓨鍌ㄦ寚閽�
 *
 * @return 寤虹珛杩炴帴鐨勬湰鍦皊ocket
 *         @see ET_SOCKET_NULL琛ㄧず鏃犺繛鎺�
 */
extern et_socket_t et_socket_accept(et_socket_t socket, et_addr_info_t *remote_info);

/**
 * UDP鍙戦�佹暟鎹�
 * @note 褰撳惎鐢ㄥ唴缃戞湇鍔℃椂闇�瀹炵幇璇ユ帴鍙�
 * @param socket 鐢眅t_socket_create杩斿洖
 * @param send_buf 鎸囧悜寰呭彂閫佺殑鏁版嵁buffer
 * @param buf_len 寰呭彂閫佺殑鏁版嵁闀垮害锛屽崟浣嶄负Byte
 * @param remote_info 杩滅涓绘満淇℃伅
 * @param time_out_ms 瓒呮椂鏃堕棿锛屽崟浣嶄负ms
 *
 * @return 鍙戦�佹垚鍔熺殑鏁版嵁闀垮害\n
 *         -1 琛ㄧず澶辫触
 */
extern et_int32 et_socket_send_to(et_socket_t socket, et_uint8 *send_buf, et_uint32 buf_len, et_addr_info_t *remote_info,
                                  et_uint32 time_out_ms);

/**
 * UDP鎺ユ敹鏁版嵁
 * @note 褰撳惎鐢ㄥ唴缃戞湇鍔℃椂闇�瑕佸疄鐜拌鎺ュ彛
 * @param socket 鐢眅t_socket_create杩斿洖
 * @param recv_buf 鎸囧悜鎺ユ敹buffer
 * @param buf_len 鎺ユ敹buffer澶у皬锛屽崟浣嶄负Byte
 * @param remote_info 鍑哄弬锛岃繙绔富鏈轰俊鎭�
 * @param time_out_ms 瓒呮椂鏃堕棿锛屽崟浣嶄负ms
 *
 * @return 鎺ユ敹鏁版嵁闀垮害
 *         -1 琛ㄧず澶辫触
*/
extern et_int32 et_socket_recv_from(et_socket_t socket, et_uint8 *recv_buf, et_uint32 buf_len, et_addr_info_t *remote_info,
                                    et_uint32 time_out_ms);

/**
 * 鑾峰彇鏈満IP
 * @param local_ip 鏈満鐨処P瀛樺偍Buffer锛屾牸寮忎负瀛楃涓诧紝渚嬪"192.168.1.10"
 * @param size local_ip澶у皬锛屽崟浣峛yte
 *
 * @return 0 鑾峰彇鎴愬姛
 *         -1 鑾峰彇澶辫触
 */
extern et_int32 et_get_localip(et_int8 *local_ip, et_uint32 size);
#endif

/**
 * 閫氳繃鍩熷悕瑙ｆ瀽鑾峰緱鍦板潃淇℃伅
 * @note 瑙ｆ瀽杩斿洖鐨処P鍦板潃淇℃伅蹇呴』涓轰互'\0'缁撳熬鐨勫瓧绗︿覆\n
         璇ユ帴鍙ｅ簲涓哄悓姝ユ帴鍙ｏ紝SDK璋冪敤璇ユ帴鍙ｆ椂闃诲鐩磋嚦杩斿洖鎴愬姛鎴栧け璐�
 * @param name 鍩熷悕鎴朓P绔彛淇℃伅锛屾牸寮忎负浠ョ粨鏉熺'\0'缁撳熬鐨勫瓧绗︿覆锛屽"www.kaifakuai.com"鎴栤��192.168.1.10鈥�
 * @param addr 鍑哄弬锛屽瓨鍌ㄨ幏鍙栧埌鐨処P鍦板潃,@see et_addr_info_t涓殑绔彛淇℃伅鍙笉浼�
 * @param time_out_ms 瓒呮椂鏃堕棿锛屽崟浣嶄负ms
 *
 * @return 0 琛ㄧず鎴愬姛\n
 *         -1 琛ㄧず澶辫触
 *
 */
extern et_int32 et_gethostbyname(et_int8 *name, et_addr_info_t *addr, et_uint32 time_out_ms);


#endif
