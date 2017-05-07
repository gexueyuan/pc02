
#ifndef __QUEUE_MSG_H__
#define __QUEUE_MSG_H__

#include "cv_osal.h"


enum MSG_TYPE
{
    EG_MSG_BASE = 0x0000,       
    EG_MSG_INITED,
    
    USB_MSG_CONNECT_SERVER,


    ACL_MSG_BASE = 0x0200,
    ACL_MSG_SERCH,
    ACL_MSG_STORE,


    TCP_MSG_BASE = 0x0400,
    TCP_MSG_SEND,//transfer to server
    TCP_MSG_REC,
    TCP_MSG_XXX
};
typedef struct _usb_var {


    /* os related */
    osal_task_t   *task_usb_r;
    osal_task_t    *task_usb_w; 
    osal_queue_t  *queue_usb;
} usb_var_t;

typedef struct _acl_var {

    /* os related */
    osal_task_t   *task_acl;
    osal_queue_t  *queue_acl;
} acl_var_t;

typedef struct _acl_local {
    /* os related */
    osal_task_t   *task_acl_l;
    osal_queue_t  *queue_acl_l;
} acl_local_t;

typedef struct _network {

    
    /* os related */
    osal_task_t   *task_net;
    osal_queue_t  *queue_net;
} network_t;

typedef struct _eg_global_var {

    usb_var_t   usb;
    acl_local_t acl_l;
    acl_var_t   acl;
    network_t net;
    
} eg_global_var_t;

typedef struct _acl_data {
    char id[9];
    char payload[17];
} acl_data_t;



#endif

