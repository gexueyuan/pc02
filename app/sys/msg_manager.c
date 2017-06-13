/*****************************************************************************
 Copyright(C) Tendyron Corporation
 All rights reserved.
 
 @file   : msg_manager.c
 @brief  : deal with msg between ubus and  322
 @author : gexueyuan
 @history:
           2017-5-4    gexueyuan    Created file
           ...
******************************************************************************/

#include "cv_osal.h"

#define OSAL_MODULE_DEBUG
#define OSAL_MODULE_DEBUG_LEVEL OSAL_DEBUG_INFO
#define MODULE_NAME "msg_manager"
#include "cv_osal_dbg.h"
OSAL_DEBUG_ENTRY_DEFINE(msg_manager);
#include "cv_cms_def.h"

#include <sys/time.h>
#include <unistd.h>
#include <libubox/ustream.h>
#include "libubus.h"
#include "count.h"

static struct ubus_context *ctx;
static struct blob_buf b;

static unsigned char ce_send_fg = 0;


enum {
	RETURN_STATUS,
    RETURN_STR,
    RETURN_STRHEX,
	__RETURN_MAX,
};

static const struct blobmsg_policy return_policy[__RETURN_MAX] = {
	[RETURN_STATUS] = { .name = "status", .type = BLOBMSG_TYPE_INT32 },
    [RETURN_STR] = { .name = "str", .type = BLOBMSG_TYPE_STRING },
    [RETURN_STRHEX] = { .name = "strhex", .type = BLOBMSG_TYPE_UNSPEC },
};

static void get_result_data_cb(struct ubus_request *req,
				    int type, struct blob_attr *msg)
{
	struct blob_attr *tb[__RETURN_MAX];
	int rc;
	
	blobmsg_parse(return_policy, __RETURN_MAX, tb, blob_data(msg), blob_len(msg));
    
	if (!tb[RETURN_STATUS]) {
		fprintf(stderr, "No return code received from server\n");
		return;
	}

	rc = blobmsg_get_u32(tb[RETURN_STATUS]);


	fprintf(stderr, "return is %d \n", rc);
}


static void update_mackey(void)
{
	static struct ubus_request req;
	uint32_t id;
	int ret;
	int ap_index=2;

	if (ubus_lookup_id(ctx, "ubus322", &id)) {
		fprintf(stderr, "Failed to look up test object\n");
		return;
	}
	blob_buf_init(&b, 0);
	
	blobmsg_add_u32(&b, "tag", UBUS_SERVER_MACKEY);
	char *a = "\x11\x22\x33\x44\x55\x66\x77\x88\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA";
	blobmsg_add_field(&b, BLOBMSG_TYPE_UNSPEC, "strhex", a, 18);
	
	int i =	ubus_invoke(ctx, id, "pushdata", b.head, get_result_data_cb, NULL, 3000);
	
	//printf("%d\n", i);
	//return ;
}

static void update_cfg(void)
{
	static struct ubus_request req;
	uint32_t id;
	int ret;
	int ap_index=2;

	if (ubus_lookup_id(ctx, "ubus322", &id)) {
		fprintf(stderr, "Failed to look up test object\n");
		return;
	}
	blob_buf_init(&b, 0);
	
	blobmsg_add_u32(&b, "tag", UBUS_SERVER_BASE_CFG);
	char *a = "\x11\x22\x33\x44\x55\x66\x77\x88\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA";
	blobmsg_add_field(&b, BLOBMSG_TYPE_UNSPEC, "strhex", a, 18);
	
	int i =	ubus_invoke(ctx, id, "pushdata", b.head, NULL, NULL, 3000);
	
	//printf("%d\n", i);
	//return ;
}

void update_ce(void)
{
	static struct ubus_request req;
	uint32_t id;
	int ret;
	int ap_index=2;

	if (ubus_lookup_id(ctx, "pc02nbi", &id)) {
		fprintf(stderr, "Failed to look up test object\n");
		return;
	}
	blob_buf_init(&b, 0);
	
	blobmsg_add_u32(&b, "tag", UBUS_CLIENT_SENDCE);
	//char *a = "\x11\x22\x33\x44\x55\x66\x77\x88\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA";
	//blobmsg_add_field(&b, BLOBMSG_TYPE_UNSPEC, "strhex", a, 18);
	
	int i =	ubus_invoke(ctx, id, "pushdata", b.head, NULL, NULL, 3000);
	
	//printf("%d\n", i);
	//return ;
}


void send_log(unsigned char* log_buffer,int len)
{

}

void ubus_client_process(unsigned int tag,char* str,unsigned char* strhex,int strlen)
{

	static struct ubus_request req;
	uint32_t id;
	int ret;
	int ap_index=2;

    unsigned char* str_buffer = NULL;

	if (ubus_lookup_id(ctx, "ubus321", &id)) {
		fprintf(stderr, "Failed to look up ubus321 object\n");
		return;
	}
	blob_buf_init(&b, 0);
	
	blobmsg_add_u32(&b, "tag", tag);

    switch(tag)
    {
        case UBUS_CLIENT_GETWL:
            break;

        case UBUS_CLIENT_GETWLCFG:
            break;

        case UBUS_CLIENT_SENDVERSION:
            str_buffer = (unsigned char*)malloc(strlen);
            memcpy(str_buffer,strhex,strlen);
            break;

        case UBUS_CLIENT_LOG:
            str_buffer = (unsigned char*)malloc(strlen);
            memcpy(str_buffer,strhex,strlen);
            break;

        default:
            break;
        



    }
   
	//char *a = "\x11\x22\x33\x44\x55\x66\x77\x88\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA";
	blobmsg_add_field(&b, BLOBMSG_TYPE_UNSPEC, "strhex", str_buffer, strlen);
	
	int i =	ubus_invoke(ctx, id, "pushdata", b.head, NULL, NULL, 3000);

    if(str_buffer != NULL)
        free(str_buffer);
}

void ubus_net_process(unsigned int tag,char* str,unsigned char* strhex,int strlen)
{

	static struct ubus_request req;
	uint32_t id;
	int ret;
	int ap_index=2;

    unsigned char* str_buffer = NULL;

	if (ubus_lookup_id(ctx, "pc02nbi", &id)) {
		fprintf(stderr, "Failed to look up test object\n");
		return;
	}

	blob_buf_init(&b, 0);
	
	blobmsg_add_u32(&b, "tag", tag);

    switch(tag)
    {
        case UBUS_CLIENT_GETWL:
            break;

        case UBUS_CLIENT_GETWLCFG:
            break;

        case UBUS_CLIENT_SEND_ALARM:
            str_buffer = (unsigned char*)malloc(strlen);
            memcpy(str_buffer,strhex,strlen);
            break;

        case UBUS_CLIENT_LOG:
            str_buffer = (unsigned char*)malloc(strlen);
            memcpy(str_buffer,strhex,strlen);
            break;

        default:
            break;
        



    }
   
	//char *a = "\x11\x22\x33\x44\x55\x66\x77\x88\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA";
	blobmsg_add_field(&b, BLOBMSG_TYPE_UNSPEC, "strhex", str_buffer, strlen);
	
	int i =	ubus_invoke(ctx, id, "pushdata", b.head, NULL, NULL, 3000);

    if(str_buffer != NULL)
        free(str_buffer);
}

static void get_wl_data_cb(struct ubus_request *req,
				    int type, struct blob_attr *msg)
{
	struct blob_attr *tb[__RETURN_MAX];
	int rc;
	int len;
    key_buffer_t *wl_lv;
	blobmsg_parse(return_policy, __RETURN_MAX, tb, blob_data(msg), blob_len(msg));
    
	if (!tb[RETURN_STATUS]) {
		fprintf(stderr, "No return code received from server\n");
		return;
	}

    wl_lv = (key_buffer_t *)req->priv;
        
	rc = blobmsg_get_u32(tb[RETURN_STATUS]);
    
    unsigned char *data = (unsigned char *)blobmsg_data(tb[RETURN_STRHEX]);
    
    if(data != NULL){

    
        len = blobmsg_data_len(tb[RETURN_STRHEX]);

        wl_lv->len = len;
        
        memcpy(wl_lv->data,data,len);

       // req->priv = &wl_lv;

    }
    else{

        printf("WL return NULL\n");

    }

	//fprintf(stderr, "return is %d \n", rc);
}

void get_wl(uint8_t *id_lv,uint8_t *data_wl,int *wlen)
{
	static struct ubus_request req;
	uint32_t id;
	int ret;
    uint8_t id_data[33];//0-len,1~n data
    uint8_t id_len;

    key_buffer_t wl_buffer;

    wl_buffer.data = data_wl;

    //wlen = &wl_buffer.len;

    *wlen = wl_buffer.len;

	if (ubus_lookup_id(ctx, "ubus321", &id)) {
		fprintf(stderr, "Failed to look up test object\n");
		return;
	}
	blob_buf_init(&b, 0);
	
	blobmsg_add_u32(&b, "tag", 0x3001);

    id_len = id_lv[0] < sizeof(id_data) ? id_lv[0]:sizeof(id_data);

    memcpy(id_data,&id_lv[1],id_len);

    
	blobmsg_add_field(&b, BLOBMSG_TYPE_UNSPEC, "strhex", id_data, id_len);
    
	int i =	ubus_invoke(ctx, id, "pushdata", b.head, get_wl_data_cb, (void*)&wl_buffer, 3000);
	
	//printf("%d\n", i);
	//return ;
}


static void get_wl_4_cb(struct ubus_request *req,
				    int type, struct blob_attr *msg)
{
	struct blob_attr *tb[__RETURN_MAX];
	int rc;
	int len;
    key_buffer_t *wl_lv;
	blobmsg_parse(return_policy, __RETURN_MAX, tb, blob_data(msg), blob_len(msg));
    
	if (!tb[RETURN_STATUS]) {
		fprintf(stderr, "No return code received from server\n");
		return;
	}

    wl_lv = (key_buffer_t *)req->priv;
        
	rc = blobmsg_get_u32(tb[RETURN_STATUS]);
    
    unsigned char *data = (unsigned char *)blobmsg_data(tb[RETURN_STRHEX]);
    
    if(data != NULL){

    
        len = blobmsg_data_len(tb[RETURN_STRHEX]);

        wl_lv->len = len;
        
        memcpy(wl_lv->data,data,len);

       // req->priv = &wl_lv;

    }
    else{

        printf("WL return NULL\n");

    }

	//fprintf(stderr, "return is %d \n", rc);
}

#define ID_LEN 4
void get_wl_4(uint8_t *p_id,uint8_t *data_wl,int *wlen)
{
	static struct ubus_request req;
	uint32_t id;
	int ret;
    uint8_t id_data[4] = {0};//len = 4
    uint8_t id_len;

    key_buffer_t wl_buffer;

	if (ubus_lookup_id(ctx, "ubus321", &id)) {
		fprintf(stderr, "Failed to look up test object\n");
		return;
	}
	blob_buf_init(&b, 0);
	
	blobmsg_add_u32(&b, "tag", 0x3001);


    memcpy(id_data,p_id,ID_LEN);

    wl_buffer.data = data_wl;
    
	blobmsg_add_field(&b, BLOBMSG_TYPE_UNSPEC, "strhex", id_data, ID_LEN);
    
	int i =	ubus_invoke(ctx, id, "pushdata", b.head, get_wl_4_cb, (void*)&wl_buffer, 3000);

    //data_wl = wl_buffer.data;
    if(wl_buffer.len < *wlen){
        
        memcpy(data_wl,wl_buffer.data,wl_buffer.len);
        *wlen = wl_buffer.len;
    }
    else{

        printf("F[%s] L[%d] wl len too long!!!\n", __FILE__, __LINE__);
    }
	//printf("%d\n", i);
	//return ;
}



static void get_audit_cb(struct ubus_request *req,
				    int type, struct blob_attr *msg)
{
	struct blob_attr *tb[__RETURN_MAX];
	int rc;
	int len;
    key_buffer_t *wl_lv;
	blobmsg_parse(return_policy, __RETURN_MAX, tb, blob_data(msg), blob_len(msg));
    
	if (!tb[RETURN_STATUS]) {
		fprintf(stderr, "No return code received from server\n");
		return;
	}

    wl_lv = (key_buffer_t *)req->priv;
        
	rc = blobmsg_get_u32(tb[RETURN_STATUS]);
    
    
    if(tb[RETURN_STRHEX] != NULL){
        
        unsigned char *data = (unsigned char *)blobmsg_data(tb[RETURN_STRHEX]);
        
        if(data != NULL){

        
            len = blobmsg_data_len(tb[RETURN_STRHEX]);

            wl_lv->len = len;
            
            memcpy(wl_lv->data,data,len);

           // req->priv = &wl_lv;

        }
        else{

            printf("WL return NULL\n");

        }
    }else{

        printf("str hex is NULL\n");
    }
    
    
	//fprintf(stderr, "return is %d \n", rc);
}
int get_audit_data(unsigned int tag,unsigned char* strhex,int strlen,uint8_t *data_wl,int *wlen)
{
	static struct ubus_request req;
	uint32_t id;
	int ret;
    uint8_t id_data[4] = {0};//len = 4
    uint8_t id_len;

    key_buffer_t wl_buffer;

    memset(&wl_buffer,0,sizeof(wl_buffer));
	if (ubus_lookup_id(ctx, "ubus321", &id)) {
		fprintf(stderr, "Failed to look up ubus321 object\n");
		return -1;
	}
	blob_buf_init(&b, 0);
	
	blobmsg_add_u32(&b, "tag", tag);
    //printf("F[%s] L[%d],tag is %d\n", __FILE__, __LINE__,tag);

    wl_buffer.data = data_wl;

    //printf("F[%s] L[%d],p is %p\n", __FILE__, __LINE__,data_wl);

    memcpy(id_data,strhex,strlen);

    printf("\n%d-%d-%d-%d\n",id_data[0],id_data[1],id_data[2],id_data[3]);
    
	blobmsg_add_field(&b, BLOBMSG_TYPE_UNSPEC, "strhex", id_data, strlen);
    
	int i =	ubus_invoke(ctx, id, "pushdata", b.head, get_audit_cb, (void*)&wl_buffer, 3000);
    printf("F[%s] L[%d],len is %d,%d\n", __FILE__, __LINE__,wl_buffer.len,*wlen);
    //data_wl = wl_buffer.data;
    if(wl_buffer.len < *wlen){
        
        memcpy(data_wl,wl_buffer.data,wl_buffer.len);
        *wlen = wl_buffer.len;
    }
    else{

        printf("F[%s] L[%d] wl len too long!!!\n", __FILE__, __LINE__);
        return -1;
    }
	//printf("%d\n", i);
	return 0 ;
}



void ubus_clien_init(void)
{

    const char *ubus_socket = NULL;
    int ch;

    uloop_init();

    ctx = ubus_connect(ubus_socket);
    if (!ctx) {
        fprintf(stderr, "Failed to connect to ubus\n");
        return -1;
    }


    //client_main();



}
osal_status_t sys_add_event_queue(msg_manager_t *p_sys, 
                             uint16_t msg_id, 
                             uint16_t msg_len, 
                             uint32_t msg_argc,
                             void    *msg_argv)
{
    int err = OSAL_STATUS_NOMEM;
    sys_msg_t *p_msg;
    uint32_t len = sizeof(sys_msg_t);
    p_msg = (sys_msg_t *)osal_malloc(len);
    if (p_msg) {
        p_msg->id = msg_id;
        p_msg->len = msg_len;
        p_msg->argc = msg_argc;
        p_msg->argv = msg_argv;
        err = osal_queue_send(p_sys->queue_msg, p_msg, len, 0, 0);
    }

    if (err != OSAL_STATUS_SUCCESS) {
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_WARN, "%s: failed=[%d], msg=%04x\n",\
                           __FUNCTION__, err, msg_id);
        osal_free(p_msg);                   
    }

    return err;
}

int state_alternate(E_USB_COMM_STATE state_d,usb_ccid_322_t * ccid)
{
    /* Take the semaphore. */
    if(osal_sem_take(ccid->sem_state, OSAL_WAITING_FOREVER) != OSAL_EOK){
        
        printf("Semaphore return failed. \n");
        return -1;
    }

    ccid->usb_state = state_d;
    ccid->toggle_state = 0xAA;

    return 0;
}



void sys_manage_proc(msg_manager_t *p_sys, sys_msg_t *p_msg)
{

    uint32_t type = 0; 
    int i;
    //msg_manager_t *p_sys = &p_controll_eg->msg_manager;
    
    switch(p_msg->id){
        
    case SYS_MSG_INITED:
        
        for(i = 0;i < MAX_322_NUM;i++ ){
        
            if(controll_eg.usb_ccid_322[i].ccid322_exist){

                
                state_alternate(USB_COMM_STATE_INIT,&controll_eg.usb_ccid_322[i]);
            }
            
        
        }
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "%s: initialize complete\n", __FUNCTION__);

        break;
        
    case SYS_MSG_SEND_CE:
        
        ce_send_fg++;
        
        if(ce_send_fg >= p_controll_eg->cnt_322){

            update_ce();
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "send 322 ce\n");
        }
        

        break;

    case SYS_MSG_SEND_COSVERSION:
        
        for(i = 0;i < MAX_322_NUM;i++ ){
        
            if(controll_eg.usb_ccid_322[i].ccid322_exist){

                
                state_alternate(USB_COMM_STATE_VERSION,&controll_eg.usb_ccid_322[i]);
                //break;
            }
            
        
        }

        break;
                
    case SYS_MSG_UPDATE_MACKEY:
        
        for(i = 0;i < MAX_322_NUM;i++ ){
        
            if(controll_eg.usb_ccid_322[i].ccid322_exist){

                
                state_alternate(USB_COMM_STATE_MACKEY,&controll_eg.usb_ccid_322[i]);
                controll_eg.usb_ccid_322[i].init_flag |= INIT_MASK_MAC;
            }
            
        
        }

        break;
            
    case SYS_MSG_UPDATE_P2PKEY:
        
        for(i = 0;i < MAX_322_NUM;i++ ){
        
            if(controll_eg.usb_ccid_322[i].ccid322_exist){

                
                state_alternate(USB_COMM_STATE_P2P,&controll_eg.usb_ccid_322[i]);
                controll_eg.usb_ccid_322[i].init_flag |= INIT_MASK_P2P;
            }
            
        
        }

        break;        

    case SYS_MSG_UPDATE_BASECFG:
        //update_cfg();
        
        for(i = 0;i < MAX_322_NUM;i++ ){
        
            if(controll_eg.usb_ccid_322[i].ccid322_exist){
                
                //controll_eg.usb_ccid_322[i].usb_state = USB_COMM_STATE_P2P;
                
                state_alternate(USB_COMM_STATE_CFG,&controll_eg.usb_ccid_322[i]);
                controll_eg.usb_ccid_322[i].init_flag |= INIT_MASK_BASECFG;
            }
            
        
        }
        break;
        
    case SYS_MSG_UPDATE_TIMECFG:
    
        break;
    
    case SYS_MSG_UPDATE_ALERTCFG:
    
        break;
        
    case SYS_MSG_UPDATE_READERCFG:
        
        for(i = 0;i < MAX_322_NUM;i++ ){
        
            if(controll_eg.usb_ccid_322[i].ccid322_exist){
                
                //controll_eg.usb_ccid_322[i].usb_state = USB_COMM_STATE_P2P;
                state_alternate(USB_COMM_STATE_RDCFG,&controll_eg.usb_ccid_322[i]);
                controll_eg.usb_ccid_322[i].init_flag |= INIT_MASK_CRLCFG;
            }
            
        
        }
    
        break;
    case SYS_MSG_ALARM_ACTIVE:
        
        for(i = 0;i < MAX_322_NUM;i++ ){
        
            if(controll_eg.usb_ccid_322[i].ccid322_exist){
                
                //controll_eg.usb_ccid_322[i].usb_state = USB_COMM_STATE_P2P;
                state_alternate(USB_COMM_ALARM_OPEN,&controll_eg.usb_ccid_322[i]);
                //break;
            }
            
        
        }
        break;
        
    case SYS_MSG_INFO_PUSH:
        
/*
        for(i = 0;i < MAX_322_NUM;i++ ){
        
            if(controll_eg.usb_ccid_322[i].ccid322_exist){
                
                //controll_eg.usb_ccid_322[i].usb_state = USB_COMM_STATE_P2P;
                state_alternate(USB_COMM_STATE_PUSH,&controll_eg.usb_ccid_322[i]);
                //break;
            }
            
        
        }
*/
        //printf("get push info %d\n",p_msg->argc);
        state_alternate(USB_COMM_STATE_PUSH,&controll_eg.usb_ccid_322[p_msg->argc]);
        break;


   case SYS_MSG_REMOTE_OPEN:
    
       for(i = 0;i < MAX_322_NUM;i++ ){
       
           if(controll_eg.usb_ccid_322[i].ccid322_exist){
               
               state_alternate(USB_COMM_REMOTE_OPEN,&controll_eg.usb_ccid_322[i]);
               //break;
           }
           
       
       }
        
        
    break;
        
    default:
        break;
    }
}




void * msg_thread_entry(void *parameter)
{
    int err;
    sys_msg_t *p_msg;
    msg_manager_t *p_sys = (msg_manager_t *)parameter;

    uint32_t len = 0;
    uint8_t buf[SYS_MQ_MSG_SIZE];
    p_msg = (sys_msg_t *)buf;

    while(1){
        
        memset(buf, 0, SYS_MQ_MSG_SIZE);        
        err = osal_queue_recv(p_sys->queue_msg, buf, &len, OSAL_WAITING_FOREVER);
        if (err == OSAL_STATUS_SUCCESS){
            sys_manage_proc(p_sys, p_msg);
            osal_free(p_msg);
        }
        else{
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_ERROR, "%s: osal_queue_recv error [%d]\n", __FUNCTION__, err);
        }
        
    }



}





void msg_manager_init(void)
{

    msg_manager_t *p_msg = &p_controll_eg->msg_manager;


    osal_task_t *tid;

    ubus_clien_init();
    /* object for sys */
    p_msg->queue_msg = osal_queue_create("q-msg", SYS_QUEUE_SIZE, SYS_MQ_MSG_SIZE);
    osal_assert(p_msg->queue_msg != NULL);

    p_msg->task_msg = osal_task_create("task-msg",
                           msg_thread_entry, p_msg,
                           LESS_THREAD_STACK_SIZE, RT_SYS_THREAD_PRIORITY);
    osal_assert(p_msg->task_msg != NULL);      

}


