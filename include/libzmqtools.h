#ifndef _LIB_ZMQ_TOOLS_H_
#define _LIB_ZMQ_TOOLS_H_

#ifdef __cplusplus
extern "C"
{
#endif

extern void *zmq_socket_new_sub(void *context,char *dest);
extern void *zmq_socket_new_pub(void *context,char *dest);

extern void *zmq_socket_new_rep(void *context,char *dest);
extern void *zmq_socket_new_req(void *context,char *dest);

extern void *zmq_socket_new_router(void *context,char *dest);
extern void *zmq_socket_new_dealer(void *context,char *dest);
extern void *zmq_socket_new_dealer_identity(void *context,char *dest, char *identity);
extern void *zmq_socket_new_dealer_svr(void *context,char *dest);

extern void *zmq_socket_new_pull(void *context,char *dest);
extern void *zmq_socket_new_push(void *context,char *dest);


extern int zmq_poll_recv(void *socket, char *pBuff, int iMaxLen, int iTimeOut);
extern int zmq_socket_send(void *socket, const unsigned char *pBuff, int iSendLen);

#ifdef __cplusplus
}
#endif

#define ZMQ_CLI_1 "tcp://127.0.0.1:18901"
#define ZMQ_CLI_2 "tcp://127.0.0.1:18902"
#define ZMQ_CLI_3 "tcp://127.0.0.1:18903"
#define ZMQ_CLI_4 "tcp://127.0.0.1:18904"

#define ZMQ_SERVER_1 "tcp://127.0.0.1:28901"
#define ZMQ_SERVER_2 "tcp://127.0.0.1:28902"
#define ZMQ_SERVER_3 "tcp://127.0.0.1:28903"
#define ZMQ_SERVER_4 "tcp://127.0.0.1:28904"

#define ZMQ_ANS_1 "tcp://127.0.0.1:38901"
#define ZMQ_ANS_2 "tcp://127.0.0.1:38902"
#define ZMQ_ANS_3 "tcp://127.0.0.1:38903"
#define ZMQ_ANS_4 "tcp://127.0.0.1:38904"

#define ZMQ_CLI_CAM "tcp://172.31.200.10:65554"
#define ZMQ_SERVER_CAM "tcp://172.31.200.1:65554"

#define MAX_BUF_LEN    2048

//GOPAN_BOX tag
#define TLV_RD_BEGIN       0x0001     //交易代码
#define TLV_IDINFO_ENC     0x0002     //身份证密文数据
#define TLV_ID_INDEX       0x0003     //身份证索引
#define TLV_ID_DN          0x0004     //身份证DN


#define READ_INFO_ONLINE   0x0101     //在线读证
#define READ_INFO_OFFLINE  0x0102     //离线读证

#endif
