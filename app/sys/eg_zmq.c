/*****************************************************************************
 Copyright(C) Tendyron Corporation
 All rights reserved.
 
 @file   : eg_zmq.c
 @brief  : zmq module
 @author : gexueyuan
 @history:
           2017-7-12    gexueyuan    Created file
           ...
******************************************************************************/
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "cv_cms_def.h"
#include "libzmqtools.h"
#include "libtlv.h"

#define R_BUFFER 2048

#define ZMQ_THREAD_STACK_SIZE   (256*1024)

unsigned char* zmq_tk[] = {"zmq-1","zmq-2","zmq-3","zmq-4"};

int zmq_pc02 (void)
{
    printf ("Connecting to hello world server…\n");
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "tcp://localhost:18901");

    int request_nbr;
    for (request_nbr = 0; request_nbr != 10; request_nbr++) {
        char buffer [10];
        printf ("Sending Hello %d…\n", request_nbr);
        zmq_send (requester, "Hello", 5, 0);
        zmq_recv (requester, buffer, 10, 0);
        printf ("Received World %d\n", request_nbr);
    }
    zmq_close (requester);
    zmq_ctx_destroy (context);
    return 0;
}
		
static unsigned short msg_parse_packet(char *recv_buf, int recv_len, char *send_buf, int *send_len)
{		
	int  rc;
	int  tmp_len;
	int  timeout;
	int  socket_fd;
	unsigned short tran_code = 0;
	
	//获取交易码
	iFindTlvList(recv_buf, recv_len, TLV_RD_BEGIN, TLV_TYPE_16, (unsigned char *)&tran_code, &tmp_len);

	
	return tran_code;
}
void *eg_zmq_thread_entry(void *parameter)
{
    //int ret = 0;
    unsigned short ret;
    usb_ccid_322_t *p_zmq_322;
	int  rc = 0;
	int  timeout = 0;

	char recv_buf[MAX_BUF_LEN];
	int  recv_len = 0;
	char send_buf[MAX_BUF_LEN];
	int  send_len = 0;
    

    unsigned char buffer[2048];
    /* Create an empty ?MQ message */
    
    p_zmq_322 = (usb_ccid_322_t*)parameter;

	p_zmq_322->context = zmq_ctx_new();
    
	p_zmq_322->zmq_server = zmq_socket_new_dealer_svr(p_zmq_322->context, ZMQ_SERVER_1);
	
	p_zmq_322->zmq_answer = zmq_socket_new_dealer_svr(p_zmq_322->context, ZMQ_ANS_1);

    printf("\nzmq ctx is %p,%p,%p\n",p_zmq_322->context,p_zmq_322->zmq_server,p_zmq_322->zmq_answer);

	zmq_pollitem_t pollitems[1];
	
	memset(pollitems, 0, sizeof(pollitems));
	pollitems[0].socket = p_zmq_322->zmq_server;  
	pollitems[0].events = ZMQ_POLLIN;
      
    //接收消息
    timeout = -1;//block
    
    while(1){
       
        rc = zmq_poll(pollitems, 1, timeout);
        
        if(rc <= 0) {
            timeout = -1;
            continue;
        }

        //zmq
        if(pollitems[0].revents & ZMQ_POLLIN){
            
        	recv_len = zmq_recv(p_zmq_322->zmq_server, recv_buf,sizeof(recv_buf), 0); 
            
        	if (recv_len <= 0){
                
        		printf("zmq_recv error=[%d]", zmq_errno());
        		continue;
                
        	}
            
        	//解析报文处理
        	//ret = msg_parse_packet(recv_buf, recv_len, send_buf, &send_len);

            memcpy(p_zmq_322->zmq_buffer,recv_buf,recv_len);

            p_zmq_322->zmq_len = recv_len;
            
            sys_add_event_queue(&controll_eg.msg_manager,ZMQ_MSG_ID,0,p_zmq_322->ccid322_index,NULL);
        }




    }

clear:
    /* Release message */
    zmq_close (p_zmq_322->zmq_client);
    zmq_ctx_destroy (p_zmq_322->context);
    
    return 0;



}
void eg_zmq_init(usb_ccid_322_t* argv)
{


    osal_task_t *tid;

    printf("zmq name is %s\n",zmq_tk[argv->ccid322_index]);
    tid = osal_task_create(zmq_tk[argv->ccid322_index],
                        eg_zmq_thread_entry,
                        argv,ZMQ_THREAD_STACK_SIZE, RT_SYS_THREAD_PRIORITY);

    osal_assert(tid != NULL);

/*
    tid = osal_task_create("zmq_server_2",
                        eg_zmq_rep_entry,
                        &controll_eg.usb_ccid_322[1],ZMQ_THREAD_STACK_SIZE, RT_SYS_THREAD_PRIORITY);

    osal_assert(tid != NULL);
*/




}


