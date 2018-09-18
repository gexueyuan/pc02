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

//extern int writeFile(const char* _fileName, void* _buf, int _bufLen);

const char* zmq_tk[] = {"zmq-1","zmq-2","zmq-3","zmq-4"};
const char* zmq_s_tk[] = {"zmq-s-1","zmq-s-2","zmq-s-3","zmq-s-4"};

extern void print_array(const  char* tag,unsigned char* send,int len);


/*
* 鍑芥暟璇存槑: 鍐欎簩杩涘埗鏂囦欢
* 鍙傛暟鎻忚堪: _fileName, 鏂囦欢鍚嶇О
*           _buf, 瑕佸啓鐨勫唴瀛樼紦鍐层??
*           _bufLen, 鍐呭瓨缂撳啿鐨勯暱搴?
*   杩斿洖鍊?: 0, 鎴愬姛
*           -1, 澶辫触
*
*/
int writeFile_over(const char* _fileName, void* _buf, int _bufLen)
{
    FILE * fp = NULL;
    if( NULL == _buf || _bufLen <= 0 ) return (-1);

    fp = fopen(_fileName, "wb+"); // 蹇呴』纭繚鏄互 浜岃繘鍒跺啓鍏ョ殑褰㈠紡鎵撳紑

    if( NULL == fp )
    {
        return (-1);
    }

    fwrite(_buf, _bufLen, 1, fp); //浜岃繘鍒跺啓

    fclose(fp);
    fp = NULL;

    return 0;    
}		
void *eg_zmq_thread_entry(void *parameter)
{
    //int ret = 0;
    usb_ccid_322_t *p_zmq_322;
	int  rc = 0;
	int  timeout = 0;

	unsigned char recv_buf[MAX_BUF_LEN];
	int  recv_len = 0;
    
    char *zmq_cli_addr = NULL;
    
    char *zmq_server_addr = NULL;
    
    char *zmq_answer_addr = NULL;
    

    /* Create an empty ?MQ message */
    
    p_zmq_322 = (usb_ccid_322_t*)parameter;

	p_zmq_322->context = zmq_ctx_new();

    if(p_zmq_322->ccid322_index == 1){

        zmq_cli_addr = ZMQ_CLI_1;
        zmq_server_addr = ZMQ_SERVER_1;
        zmq_answer_addr = ZMQ_ANS_1;
    }
    else if(p_zmq_322->ccid322_index == 2){
        zmq_cli_addr = ZMQ_CLI_2;
        zmq_server_addr = ZMQ_SERVER_2;
        zmq_answer_addr = ZMQ_ANS_2;
    }
	else if(p_zmq_322->ccid322_index == 3){
        zmq_cli_addr = ZMQ_CLI_3;
        zmq_server_addr = ZMQ_SERVER_3;
        zmq_answer_addr = ZMQ_ANS_3;
    }
	else if(p_zmq_322->ccid322_index == 4){
        zmq_cli_addr = ZMQ_CLI_4;
        zmq_server_addr = ZMQ_SERVER_4;
        zmq_answer_addr = ZMQ_ANS_4;
    }
    
    p_zmq_322->zmq_client = zmq_socket_new_dealer(p_zmq_322->context,zmq_cli_addr);
    
	p_zmq_322->zmq_server = zmq_socket_new_dealer_svr(p_zmq_322->context, zmq_server_addr);
	
	p_zmq_322->zmq_answer = zmq_socket_new_dealer_svr(p_zmq_322->context, zmq_answer_addr);

    printf("\nzmq ctx is %p,%p,%p\n",p_zmq_322->context,p_zmq_322->zmq_server,p_zmq_322->zmq_answer);

	zmq_pollitem_t pollitems[1];
	
	memset(pollitems, 0, sizeof(pollitems));
	pollitems[0].socket = p_zmq_322->zmq_server;  
	pollitems[0].events = ZMQ_POLLIN;
      
    //接收消息
    timeout = -1;//block
    
    while(1){

		     /* Take the semaphore. */
		if(osal_sem_take(p_zmq_322->sem_zmq, 1000) != OSAL_EOK){
		 
			printf("p_zmq_322->sem_zmq  return failed. \n");

			//osal_sem_release(p_zmq_322->sem_zmq);
			//osal_sem_release(p_usb_ccid->sem_state);
			continue;
		}
		memset(p_zmq_322->zmq_magicnum,0,5);
        rc = zmq_poll(pollitems, 1, timeout);
        
        if(rc <= 0) {
            timeout = -1;
			printf("poll failed %s %d\n",__func__, __LINE__);
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
            printf("\n------------------------------\n");
            printf("usb zmq recive:\n");
			print_array("zmq recieve", recv_buf, recv_len);
            printf("\n------------------------------\n");
		#ifdef ZMQ_NUM
            memcpy(p_zmq_322->zmq_magicnum,recv_buf,5);
            memcpy(p_zmq_322->zmq_buffer,&recv_buf[5],recv_len - 5);

            p_zmq_322->zmq_len = recv_len - 5;
        #else
			memcpy(p_zmq_322->zmq_buffer,recv_buf,recv_len);

            p_zmq_322->zmq_len = recv_len;
		#endif
            sys_add_event_queue(&controll_eg.msg_manager,ZMQ_MSG_ID,0,p_zmq_322->ccid322_index - 1,NULL);
        }




    }

//clear:
    /* Release message */
	zmq_close(p_zmq_322->zmq_server);
	zmq_close(p_zmq_322->zmq_answer);
	zmq_term(p_zmq_322->context);

    
    return 0;



}

#ifdef ZMQ_CAM
void *eg_zmq_cam_thread_entry(void *parameter)
{
    //int ret = 0;
    unsigned short ret;
	int  rc = 0;
	int  timeout = 0;
	#define CAM_BUF_LEN 16

	unsigned char recv_buf[CAM_BUF_LEN];
	int  recv_len = 0;
	unsigned char send_buf[CAM_BUF_LEN];
	int  send_len = 0;
    
    unsigned char *zmq_cli_addr;    
    unsigned char *zmq_server_addr;    
	
	void* zmq_context;
	void* zmq_client_cam;
	void* zmq_server_cam;
    
    unsigned char buffer[16];

    int i;
    /* Create an empty ?MQ message */
    

	zmq_context = zmq_ctx_new();


    zmq_cli_addr = (unsigned char *)ZMQ_CLI_CAM;
	zmq_server_addr = (unsigned char *)ZMQ_SERVER_CAM;
    
    zmq_client_cam = zmq_socket_new_dealer(zmq_context,zmq_cli_addr);
    
	zmq_server_cam = zmq_socket_new_dealer_svr(zmq_context, zmq_server_addr);
	
	zmq_pollitem_t pollitems[1];
	
	memset(pollitems, 0, sizeof(pollitems));
	pollitems[0].socket = zmq_server_cam;  
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
            
        	recv_len = zmq_recv(zmq_server_cam, recv_buf,sizeof(recv_buf), 0); 
            
        	if (recv_len <= 0){
                
        		printf("zmq_recv error=[%d]", zmq_errno());
        		continue;
                
        	}
            
        	//解析报文处理
        	//ret = msg_parse_packet(recv_buf, recv_len, send_buf, &send_len);
            printf("\n------------------------------\n");
            printf("camera zmq recive:%d\n",recv_len);
			
            for(i = 0;i < recv_len;i++){

                printf("%02X ",recv_buf[i]);
				
                if(recv_buf[i] == 0xAA){

					writeFile_over("/tmp/camera_config",&recv_buf[i + 1], 1);
					printf("\nwrite %X to /tmp/camera_config\n",recv_buf[i + 1]);

				}

            }
			
            printf("\n------------------------------\n");

            
        }




    }

clear:
    /* Release message */
	zmq_close(zmq_client_cam);
	zmq_close(zmq_server_cam);
	zmq_term(zmq_context);

    
    return 0;



}
#endif


void eg_zmq_init(usb_ccid_322_t* argv)
{


    osal_task_t *tid;

    printf("zmq name is %s\n",zmq_tk[argv->ccid322_index - 1]);
    
    tid = osal_task_create(zmq_tk[argv->ccid322_index - 1],
                        eg_zmq_thread_entry,
                        argv,PC02_ZMQ_THREAD_STACK_SIZE, PC02_ZMQ_THREAD_PRIORITY);

    osal_assert(tid != NULL);

#ifdef ZMQ_CAM
	tid = osal_task_create("camera config",
						eg_zmq_cam_thread_entry,
						NULL,PC02_ZMQ_THREAD_STACK_SIZE>>1, PC02_ZMQ_THREAD_PRIORITY);

	osal_assert(tid != NULL);

#endif

}
#ifdef ZMQ_TEST

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


void *eg_zmq_test_thread(void *parameter)
{
    //int ret = 0;
    unsigned short ret;
    usb_ccid_322_t *p_zmq_322;
	int  rc = 0;
	int  timeout = 0;
    int i = 0;

	char recv_buf[MAX_BUF_LEN];
	int  recv_len = 0;
	char send_buf[MAX_BUF_LEN];
	int  send_len = 0;
    

    unsigned char buffer[2048];
    /* Create an empty ?MQ message */
    
    void *context;
    void *zmq_sd;
    void *zmq_rv;
    void *zmq_result;
    
	context = zmq_ctx_new();
    
	zmq_rv = zmq_socket_new_dealer_svr(context, ZMQ_CLI_1);
	
	zmq_sd = zmq_socket_new_dealer(context, ZMQ_SERVER_1);

    zmq_result = zmq_socket_new_dealer(context, ZMQ_ANS_1);

	zmq_pollitem_t pollitems[1];
	
	memset(pollitems, 0, sizeof(pollitems));
	pollitems[0].socket = zmq_rv;  
	pollitems[0].events = ZMQ_POLLIN;
      
    //接收消息
    timeout = -1;//block
    
    while(1){
        printf("poll wait for zmq msg\n");
        rc = zmq_poll(pollitems, 1, timeout);
        printf("recieve zmq msg\n");
        if(rc <= 0) {
            timeout = -1;
            continue;
        }

        //zmq
        if(pollitems[0].revents & ZMQ_POLLIN){
            
        	recv_len = zmq_recv(zmq_rv, recv_buf,sizeof(recv_buf), 0); 
        	if (recv_len <= 0){
                
        		printf("zmq_recv error=[%d]", zmq_errno());
        		continue;
                
        	}
            printf("\n");
            for(i = 0;i < recv_len;i++){

                printf("0x%X ",recv_buf[i]);
                

            }
            printf("\n");

          memcpy(send_buf,ZMQ_SERVER_1,sizeof(ZMQ_SERVER_1));
          
          for(i = 0;i < 10;i++)  {
    		  zmq_send(zmq_sd, send_buf, sizeof(ZMQ_SERVER_1), 0);	
              zmq_poll_recv(zmq_sd,recv_buf,sizeof(recv_buf),0);
              printf("server recive:\n");
              for(i = 0;i < recv_len;i++){
              
                  printf("0x%X ",recv_buf[i]);
                  
              
              }
              printf("\n");
            }

          memset(send_buf,0,sizeof(send_buf));
          memcpy(send_buf,ZMQ_ANS_1,sizeof(ZMQ_ANS_1));

            rc = zmq_send(zmq_result, send_buf, sizeof(ZMQ_ANS_1), 0); 
        }




    }

clear:
    /* Release message */


    
    return 0;



}
void eg_zmq_test(void)
{
    osal_task_t *tid;
    tid = osal_task_create("test",
                    eg_zmq_test_thread,
                    NULL,PC02_ZMQ_THREAD_STACK_SIZE, PC02_ZMQ_THREAD_PRIORITY);

    osal_assert(tid != NULL);


}
#endif
