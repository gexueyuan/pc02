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
extern int zmq_socket_send(void *socket, char *pBuff, int iSendLen);

#ifdef __cplusplus
}
#endif

#define ZMQ_CLI_1 "tcp://localhost:18901"
#define ZMQ_CLI_2 "tcp://localhost:18902"
#define ZMQ_CLI_3 "tcp://localhost:18903"
#define ZMQ_CLI_4 "tcp://localhost:18904"

#define ZMQ_SERVER_1 "tcp://localhost:28901"
#define ZMQ_SERVER_2 "tcp://localhost:28902"
#define ZMQ_SERVER_3 "tcp://localhost:28903"
#define ZMQ_SERVER_4 "tcp://localhost:28904"


#define MAX_BUF_LEN    10240

//GOPAN_BOX tag
#define TLV_RD_BEGIN       0x0001     //���״���
#define TLV_IDINFO_ENC     0x0002     //����֤��������
#define TLV_ID_INDEX       0x0003     //����֤����
#define TLV_ID_DN          0x0004     //����֤DN


#define READ_INFO_ONLINE   0x0101     //���߶�֤
#define READ_INFO_OFFLINE  0x0102     //���߶�֤

#endif