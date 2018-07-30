//#include "global_sys.h"
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "libzmqtools.h"


void *zmq_socket_new_pub(void *context,char *dest){
	void *zsocket = zmq_socket (context, ZMQ_PUB);
	zmq_bind(zsocket,dest);
	return zsocket;
}

void *zmq_socket_new_sub(void *context,char *dest){
	void *zsocket = zmq_socket (context, ZMQ_SUB);
	zmq_connect (zsocket, dest);
	return zsocket;
}


void *zmq_socket_new_rep(void *context,char *dest){
	void *zsocket = zmq_socket (context, ZMQ_REP);
	zmq_bind (zsocket, dest);
	return zsocket;
}

void *zmq_socket_new_req(void *context,char *dest){
	int linger = 1;// -1: 表示无限的停留时间，0-表示没有停留时间，其他-ms
	void *zsocket = zmq_socket (context, ZMQ_REQ);
	zmq_setsockopt(zsocket, ZMQ_LINGER, &linger, sizeof(linger));
	zmq_connect (zsocket, dest);
	return zsocket;
}


void *zmq_socket_new_router(void *context,char *dest){
	void *zsocket = zmq_socket (context, ZMQ_ROUTER);
	zmq_bind(zsocket,dest);
	return zsocket;
}

void *zmq_socket_new_dealer(void *context,char *dest){
	return zmq_socket_new_dealer_identity(context, dest, NULL);
}

void *zmq_socket_new_dealer_identity(void *context,char *dest, char *identity){
	int linger = 1;
    int rc;
	void *zsocket = zmq_socket (context, ZMQ_DEALER);
	zmq_setsockopt(zsocket, ZMQ_LINGER, &linger, sizeof(linger));
	if (identity)
		zmq_setsockopt(zsocket, ZMQ_IDENTITY, identity, strlen(identity));
	rc = zmq_connect(zsocket,dest);
    if(rc != 0){
        printf("zmq connect error\n");
    }
	return zsocket;
}

void *zmq_socket_new_dealer_svr(void *context,char *dest) {
    int linger = 1;
    int rc = 0;
    void *zsocket = zmq_socket (context, ZMQ_DEALER);
    zmq_setsockopt(zsocket, ZMQ_LINGER, &linger, sizeof(linger));
    rc = zmq_bind(zsocket,dest);
    if (rc == -1) {
        printf ("bind failed: %s\n", strerror (errno));
        //return -1;
    }

    return zsocket;
}

void *zmq_socket_new_pull(void *context,char *dest){
	void *zsocket = zmq_socket (context, ZMQ_PULL);
	zmq_bind(zsocket,dest);
	return zsocket;
}

void *zmq_socket_new_push(void *context,char *dest){
	int linger = 1; 
	void *zsocket = zmq_socket (context, ZMQ_PUSH);
	zmq_setsockopt(zsocket, ZMQ_LINGER, &linger, sizeof(linger));
	zmq_connect (zsocket, dest);
	return zsocket;
}


int zmq_poll_recv(void *socket, char *pBuff, int iMaxLen, int iTimeOut){
	int iRet = 0;
	int iRcvLen = 0;

	if(iTimeOut > 0){
		zmq_pollitem_t pollset [1] = { { socket, 0, ZMQ_POLLIN, 0 } };
		iRet = zmq_poll (pollset, 1, iTimeOut * 1000);
		if(iRet == -1){
			return -1;
		}
		if(pollset[0].revents & ZMQ_POLLIN){
			iRcvLen = zmq_recv (socket, pBuff, iMaxLen, 0);
		}
		else{
			return -2;
        }
    }
    else{
		iRcvLen = zmq_recv (socket, pBuff,iMaxLen, 0);
	}

    return iRcvLen;
}

int zmq_socket_send(void *socket, const unsigned char *pBuff, int iSendLen)
{
	return zmq_send(socket,pBuff,iSendLen,0);
}
