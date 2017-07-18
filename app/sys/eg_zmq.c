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


#define ZMQ_1 18901
#define ZMQ_2 18902
#define ZMQ_3 18903
#define ZMQ_4 18904

#define ZMQ_THREAD_STACK_SIZE   (256*1024)

unsigned char* zmq_tk[] = {"zmq-1","zmq-2","zmq-3","zmq-4"};

int zmq_pc02 (void)
{
    printf ("Connecting to hello world server¡­\n");
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "tcp://localhost:18901");

    int request_nbr;
    for (request_nbr = 0; request_nbr != 10; request_nbr++) {
        char buffer [10];
        printf ("Sending Hello %d¡­\n", request_nbr);
        zmq_send (requester, "Hello", 5, 0);
        zmq_recv (requester, buffer, 10, 0);
        printf ("Received World %d\n", request_nbr);
    }
    zmq_close (requester);
    zmq_ctx_destroy (context);
    return 0;
}

void *eg_zmq_thread_entry(void *parameter)
{

    usb_ccid_322_t *p_zmq_322;

    unsigned char buffer[2048];
    /* Create an empty ?MQ message */
    zmq_msg_t msg;
    int rc = zmq_msg_init (&msg);
    //assert (rc == 0);

    p_zmq_322 = (usb_ccid_322_t*)parameter;

    p_zmq_322->context = zmq_ctx_new ();
    p_zmq_322->requester = zmq_socket (p_zmq_322->context, ZMQ_REQ);
    zmq_connect (p_zmq_322->requester, "tcp://localhost:18901");


        //zmq_send (p_zmq_322->requester, "Hello", 5, 0);
        //zmq_recv (p_zmq_322->requester, buffer, 10, 0);

    while(1){

        
/*
        zmq_recv (p_zmq_322->requester, buffer, 10, 0);
        printf("no-block\n");
*/
        
        /* Block until a message is available to be received from socket */
        rc = zmq_msg_recv (&msg, p_zmq_322->requester, 0);
        //assert (rc != -1);
        
        printf("no-block\n");


    }


    /* Release message */
    zmq_msg_close (&msg);
    zmq_close (p_zmq_322->requester);
    zmq_ctx_destroy (p_zmq_322->context);
    
    return 0;



}
void eg_zmq_init(void)
{


    osal_task_t *tid;


    tid = osal_task_create(zmq_tk[0],
                        eg_zmq_thread_entry,
                        &controll_eg.usb_ccid_322[1],ZMQ_THREAD_STACK_SIZE, RT_SYS_THREAD_PRIORITY);

    osal_assert(tid != NULL);






}


