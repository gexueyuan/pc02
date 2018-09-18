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

#include <sys/time.h>
#include <unistd.h>
#include <libubox/ustream.h>
#include "libubus.h"
#include "count.h"
#include "cv_cms_def.h"

static struct ubus_context *ctx;
static struct blob_buf b;

static unsigned char ce_send_fg = 0;
static unsigned char return_send_fg = 0;
static unsigned char return_base_fg = 0;
uint32_t id_322 = 0;
uint32_t id_net = 0;
unsigned char return_buffer[128] = {0};
#define CFG_NAME_LEN  68

osal_sem_t *sem_net_process;
osal_sem_t *sem_321_process;

unsigned char log_test[] = { 0x01,0x01,0xD9,0x02,0x00,0x00,0x00,0x00,0x00,0x46,0x03,0x72,0xB5,0x04,0x58,\
					   0x4A,0x71,0x3E,0x66,0x9A,0x70,0x8F,0x1D,0xCD,0xBA,0xB1,0xD6,0x0F,0x02,0x96,\
					   0x3B,0x66,0x33,0x2A,0x08,0x06,0xD7,0xEE,0x83,0xD9,0xA4,0x3E,0x24,0x33,0x45,\
					   0x4B,0xDB,0x74,0x77,0x56,0x8F,0xF4,0x3E,0x03,0xAC,0x75,0xC1,0xC5,0xFF,0x12,\
					   0x17,0x8F,0x97,0x5D,0x26,0x9C,0x24,0xC8,0xA4,0x1A,0xFE,0x07,0x61,0xF1,0x34,\
					   0xD1,0x1C,0x53,0xE3,0x8C,0xBB,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,\
					   0x4E,0x57,0xAB,0x0A,\
					   0x00,0x00,0x41,0x7E,\
					   0x12,0x05,0x1E,0x09,0x05,0x13,\
					   0x01,\
					   0x01,\
					   0x01,\
					   0x00,0xB5,0x46,0x4D,0x87,0xC7,0x28,0x0B,0x15,0x79,0x37,0x2E,0xBE,\
					   0xDC,0x58,0xA0,0x92,0xC7,0xFC,0xE0,0xDA,0x18,0x75,0x80,0xE7,0xCF,0x69,0x59,\
					   0x44,0xE6,0x87,0x21,0xEB,0x52,0xF5,0x93,0xB1,0x1D,0x49,0xBC,0xDC,0x0B,0xBA,\
					   0x35,0x04,0xFD,0xA9,0x9C,0xB8,0x94,0x25,0xEA,0xCD,0xCD,0x48,0x48,0xBE,0xC6,\
					   0xC3,0x61,0xF9,0x07,0x23,0x4B,0x62};

uint8_t  get_log_time(unsigned char *time_ptr)//6 bytes time in log
{
    time_t   now;
    struct   tm  *timenow;
    unsigned char time_acs[7];

    if(time_ptr == NULL){

        printf("F[%s] L[%d] ptr is NULL!!!", __FILE__, __LINE__);
        return 0;
    }
    memset(time_acs,0,sizeof(time_acs));
    time(&now);
    timenow   =   localtime(&now);
        
    time_ptr[0] = ((unsigned char)timenow-> tm_year) > 100 ? (unsigned char)timenow-> tm_year - 100:17;
    time_ptr[1] = (unsigned char)timenow-> tm_mon + 1;
    time_ptr[2] = (unsigned char)timenow-> tm_mday;
    
    
    time_ptr[3] = (unsigned char)timenow-> tm_hour;
    time_ptr[4] = (unsigned char)timenow-> tm_min;
    time_ptr[5] = (unsigned char)timenow-> tm_sec;
    //memcpy(time_ptr + sizeof(time_head) + 3,time_acs,sizeof(time_acs));//V
    return 0;
}


void ubus_321_find(void)
{

    if (ubus_lookup_id(ctx, "ubus321", &id_322)) {
        
        fprintf(stderr, "Failed to look up ubus321 object\n");
        return;
    }

    printf("321 ubus id update,0x%X\n",id_322);


}


void ubus_pc02nbi_find(void)
{

    if (ubus_lookup_id(ctx, "pc02nbi", &id_net)) {
        
        fprintf(stderr, "Failed to look up pc02nbi object\n");
        return;
    }

    printf("pc02nbi ubus id update,0x%X\n",id_net);


}


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
	uint32_t id;

	if (ubus_lookup_id(ctx, "ubus322", &id)) {
		fprintf(stderr, "Failed to look up ubus322 object\n");
		return;
	}
	blob_buf_init(&b, 0);
	
	blobmsg_add_u32(&b, "tag", UBUS_SERVER_MACKEY);
	char *a = "\x11\x22\x33\x44\x55\x66\x77\x88\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA";
	blobmsg_add_field(&b, BLOBMSG_TYPE_UNSPEC, "strhex", a, 18);
	
	ubus_invoke(ctx, id, "pushdata", b.head, get_result_data_cb, NULL, 3000);
	
	//printf("%d\n", i);
	//return ;
}

static void update_cfg(void)
{
	uint32_t id;

	if (ubus_lookup_id(ctx, "ubus322", &id)) {
		fprintf(stderr, "Failed to look up ubus322 object\n");
		return;
	}
	blob_buf_init(&b, 0);
	
	blobmsg_add_u32(&b, "tag", UBUS_SERVER_BASE_CFG);
	char *a = "\x11\x22\x33\x44\x55\x66\x77\x88\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA";
	blobmsg_add_field(&b, BLOBMSG_TYPE_UNSPEC, "strhex", a, 18);
	
	ubus_invoke(ctx, id, "pushdata", b.head, NULL, NULL, 3000);
	
	//printf("%d\n", i);
	//return ;
}

void update_ce(void)
{
	uint32_t id;

	if (ubus_lookup_id(ctx, "pc02nbi", &id)) {
		fprintf(stderr, "Failed to look up pc02nbi object\n");
		return;
	}
	blob_buf_init(&b, 0);
	
	blobmsg_add_u32(&b, "tag", UBUS_CLIENT_SENDCE);
	//char *a = "\x11\x22\x33\x44\x55\x66\x77\x88\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA";
	//blobmsg_add_field(&b, BLOBMSG_TYPE_UNSPEC, "strhex", a, 18);
	
	ubus_invoke(ctx, id, "pushdata", b.head, NULL, NULL, 3000);
	
	//printf("%d\n", i);
	//return ;
}


void send_log(unsigned char* log_buffer,int len)
{

}

void ubus_client_process(unsigned int tag,char* str,unsigned char* strhex,int strlen)
{

	uint32_t id;

    unsigned char* str_buffer = NULL;
	 /**test**/
    struct timeval _start,_end;
	long time_in_us,time_in_ms;
    /**test**/

	gettimeofday( &_start, NULL );
    /* Take the semaphore. */
    if(osal_sem_take(sem_321_process, 3500) != OSAL_EOK){
     
	     printf("Semaphore return failed. \n");
	     
	     //osal_sem_release(sem_321_process);

		return;

    }
	gettimeofday( &_end, NULL );
    time_in_us = (_end.tv_sec - _start.tv_sec) * 1000000 + _end.tv_usec - _start.tv_usec;	
	time_in_ms = time_in_us/1000;
	if(time_in_ms > 1000){
		osal_printf("F[%s] L[%d],get sem overtime ,%ldms\n",__func__, __LINE__,time_in_ms);
		log_message("sem",3,"F[%s] L[%d] T[0X%X],get sem overtime ,%ldms\n",__func__, __LINE__,tag,time_in_ms);
	}
	
    id = id_322;

    if(id == 0){

        
    	if (ubus_lookup_id(ctx, "ubus321", &id)) {
    		fprintf(stderr, "Failed to look up 321 object\n");
            
            osal_sem_release(sem_321_process);
    		return;
    	}

        id_322 = id;

        printf("\n===========321id is %X,ubus client=============\n",id);
        
    }

    
	blob_buf_init(&b, 0);
	
	blobmsg_add_u32(&b, "tag", tag);

    switch(tag)
    {
        case UBUS_CLIENT_GETWL:
            break;

        case UBUS_CLIENT_GETWLCFG:
            break;

        case UBUS_CLIENT_GETP2P:
            break;
            
        case UBUS_CLIENT_GETMAC:
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

	
	gettimeofday( &_start, NULL );
	ubus_invoke(ctx, id, "pushdata", b.head, NULL, NULL, 4000);
	gettimeofday( &_end, NULL );

    time_in_us = (_end.tv_sec - _start.tv_sec) * 1000000 + _end.tv_usec - _start.tv_usec;	
	time_in_ms = time_in_us/1000;
	if(time_in_ms > 1000){
		osal_printf("F[%s] L[%d],ubus overtime ,%ldms\n",__func__, __LINE__,time_in_ms);
		log_message("ubus",3,"F[%s] L[%d] T[0X%X],ubus overtime ,%ldms\n",__func__, __LINE__,tag,time_in_ms);
	}

    if(str_buffer != NULL)
        free(str_buffer);
    
    osal_sem_release(sem_321_process);
    
}

void ubus_net_process(unsigned int tag,char* str,unsigned char* strhex,int strlen)
{

	uint32_t id;

	int ret;

    unsigned char* str_buffer = NULL;

	if(controll_eg.network_state == 0){

		printf("\noffline  push  failed!\n");

		return;
	}

	  /**test**/
	 struct timeval _start,_end;
	 long time_in_us,time_in_ms;
	 /**test**/
	 
	 gettimeofday( &_start, NULL );

     /* Take the semaphore. */
     if(osal_sem_take(sem_net_process, 1000) != OSAL_EOK){
         
         printf("Semaphore return failed. \n");
         
         //osal_sem_release(sem_net_process);
         //osal_sem_release(p_usb_ccid->sem_state);
         //break;
         return;
     }
	 gettimeofday( &_end, NULL );
	 time_in_us = (_end.tv_sec - _start.tv_sec) * 1000000 + _end.tv_usec - _start.tv_usec;	 
	 time_in_ms = time_in_us/1000;
	 if(time_in_ms > 1000){
		 osal_printf("F[%s] L[%d],get sem overtime ,%ldms\n",__func__, __LINE__,time_in_ms);
	 	 log_message("sem",3,"F[%s] L[%d] T[0X%X],get sem overtime ,%ldms\n",__func__, __LINE__,tag,time_in_ms);
	 }


	 
     id = id_net;

    if(id == 0){

        
    	if (ubus_lookup_id(ctx, "pc02nbi", &id)) {
    		fprintf(stderr, "Failed to look up pc02nbi object\n");
            
            osal_sem_release(sem_net_process);
    		return;
    	}

        id_net = id;

        printf("\n===========pc02nbi id is %X,ubus client=============\n",id);
        
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

        case UBUS_CLIENT_SEND_DOOR_INFO:
            str_buffer = (unsigned char*)malloc(strlen);
            memcpy(str_buffer,strhex,strlen);
            break;

        case UBUS_CLIENT_SEND_ID_INFO:
            str_buffer = (unsigned char*)malloc(strlen);
            memcpy(str_buffer,strhex,strlen);
            break;

        case UBUS_CLIENT_RETURN:
            str_buffer = (unsigned char*)malloc(strlen);
            memcpy(str_buffer,strhex,strlen);
            break;
            
        case UBUS_CLIENT_SEND_BAT:
            str_buffer = (unsigned char*)malloc(strlen);
            memcpy(str_buffer,strhex,strlen);
            break;
            

        default:
            str_buffer = (unsigned char*)malloc(strlen);
            memcpy(str_buffer,strhex,strlen);
            break;
        



    }
   
	//char *a = "\x11\x22\x33\x44\x55\x66\x77\x88\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA";
	blobmsg_add_field(&b, BLOBMSG_TYPE_UNSPEC, "strhex", str_buffer, strlen);
	
	gettimeofday( &_start, NULL );
	ret = ubus_invoke(ctx, id, "pushdata", b.head, NULL, NULL, 4000);
	gettimeofday( &_end, NULL );

    time_in_us = (_end.tv_sec - _start.tv_sec) * 1000000 + _end.tv_usec - _start.tv_usec;	
	time_in_ms = time_in_us/1000;
	if(time_in_ms > 1000){
		osal_printf("F[%s] L[%d],ubus overtime ,%ldms\n",__func__, __LINE__,time_in_ms);
		log_message("ubus",3,"F[%s] L[%d] T[0X%X],ubus overtime ,%ldms,return is %d\n",__func__, __LINE__,tag,time_in_ms,ret);
	}
	
    if(str_buffer != NULL)
        free(str_buffer);

    osal_sem_release(sem_net_process);
}

static void get_wl_data_cb(struct ubus_request *req,
				    int type, struct blob_attr *msg)
{
	struct blob_attr *tb[__RETURN_MAX];
	int len;
    key_buffer_t *wl_lv;
	blobmsg_parse(return_policy, __RETURN_MAX, tb, blob_data(msg), blob_len(msg));
    
	if (!tb[RETURN_STATUS]) {
		fprintf(stderr, "No return code received from server\n");
		return;
	}

    wl_lv = (key_buffer_t *)req->priv;
        
	blobmsg_get_u32(tb[RETURN_STATUS]);
    
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
	uint32_t id;
    uint8_t id_data[33];//0-len,1~n data
    uint8_t id_len;

    key_buffer_t wl_buffer;

    wl_buffer.data = data_wl;

    //wlen = &wl_buffer.len;

    *wlen = wl_buffer.len;

	if (ubus_lookup_id(ctx, "ubus321", &id)) {
		fprintf(stderr, "Failed to look up ubus321 object\n");
		return;
	}
	blob_buf_init(&b, 0);
	
	blobmsg_add_u32(&b, "tag", 0x3001);

    id_len = id_lv[0] < sizeof(id_data) ? id_lv[0]:sizeof(id_data);

    memcpy(id_data,&id_lv[1],id_len);

    
	blobmsg_add_field(&b, BLOBMSG_TYPE_UNSPEC, "strhex", id_data, id_len);
    
	ubus_invoke(ctx, id, "pushdata", b.head, get_wl_data_cb, (void*)&wl_buffer, 3000);
	
	//printf("%d\n", i);
	//return ;
}


static void get_wl_4_cb(struct ubus_request *req,
				    int type, struct blob_attr *msg)
{
	struct blob_attr *tb[__RETURN_MAX];
	int len;
    key_buffer_t *wl_lv;
	blobmsg_parse(return_policy, __RETURN_MAX, tb, blob_data(msg), blob_len(msg));
    
	if (!tb[RETURN_STATUS]) {
		fprintf(stderr, "No return code received from server\n");
		return;
	}

    wl_lv = (key_buffer_t *)req->priv;
        
	blobmsg_get_u32(tb[RETURN_STATUS]);
    
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



	uint32_t id;
    uint8_t id_data[4] = {0};//len = 4
    key_buffer_t wl_buffer;
    
/*
	struct timeval _start,_end;

    gettimeofday( &_start, NULL );
    printf("ubus look start : %d.%d\n", _start.tv_sec, _start.tv_usec);
*/
	 /* Take the semaphore. */
	 if(osal_sem_take(sem_321_process, 3500) != OSAL_EOK){
	  
		  printf("Semaphore return failed. \n");
		  
		  //osal_sem_release(sem_321_process);
		  return;
	 
	 }
     id = id_322;

    if(id == 0){

        
    	if (ubus_lookup_id(ctx, "ubus321", &id)) {
    		fprintf(stderr, "Failed to look up ubus321 object\n");
    		return;
    	}

        id_322 = id;

        printf("\n===========321id is 0x%X,get wl=============\n",id);
        
    }

/*
        //gettimeofday( &_end, NULL );
    //printf("ubus look end    : %d.%d\n",_end.tv_sec,_end.tv_usec);

        */

    

    
	blob_buf_init(&b, 0);
	
	blobmsg_add_u32(&b, "tag", 0x3001);


    memcpy(id_data,p_id,ID_LEN);

    wl_buffer.data = data_wl;
    
	blobmsg_add_field(&b, BLOBMSG_TYPE_UNSPEC, "strhex", id_data, ID_LEN);

/*
    gettimeofday( &_start, NULL );
    printf("invoke start : %d.%d\n", _start.tv_sec, _start.tv_usec);
*/

	ubus_invoke(ctx, id, "pushdata", b.head, get_wl_4_cb, (void*)&wl_buffer, 3000);
    
/*
    gettimeofday( &_end, NULL );
    printf("invoke end   : %d.%d\n",_end.tv_sec,_end.tv_usec);
*/

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
	osal_sem_release(sem_321_process);

}



static void get_audit_cb(struct ubus_request *req,
				    int type, struct blob_attr *msg)
{
	struct blob_attr *tb[__RETURN_MAX];
	int len;
    key_buffer_t *wl_lv;
	blobmsg_parse(return_policy, __RETURN_MAX, tb, blob_data(msg), blob_len(msg));
    
	if (!tb[RETURN_STATUS]) {
		fprintf(stderr, "No return code received from server\n");
		return;
	}

    wl_lv = (key_buffer_t *)req->priv;
        
	blobmsg_get_u32(tb[RETURN_STATUS]);
    
    
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
	uint32_t id;
    uint8_t id_data[4] = {0};//len = 4

    key_buffer_t wl_buffer;

    memset(&wl_buffer,0,sizeof(wl_buffer));
    
	if(osal_sem_take(sem_321_process, 3500) != OSAL_EOK){
	 
		 printf("Semaphore return failed. \n");
		 
		 //osal_sem_release(sem_321_process);
		 return -1;

	}

     id = id_322;

    if(id == 0){

        
    	if (ubus_lookup_id(ctx, "ubus321", &id)) {
    		fprintf(stderr, "Failed to look up ubus321 object\n");
    		return -1;
    	}

        id_322 = id;

        printf("\n===========321id is 0x%X,get wl=============\n",id);
        
    }


	blob_buf_init(&b, 0);
	
	blobmsg_add_u32(&b, "tag", tag);
    //printf("F[%s] L[%d],tag is %d\n", __FILE__, __LINE__,tag);

    wl_buffer.data = data_wl;

    //printf("F[%s] L[%d],p is %p\n", __FILE__, __LINE__,data_wl);

    memcpy(id_data,strhex,strlen);

    printf("\n%d-%d-%d-%d\n",id_data[0],id_data[1],id_data[2],id_data[3]);
    
	blobmsg_add_field(&b, BLOBMSG_TYPE_UNSPEC, "strhex", id_data, strlen);
    
	ubus_invoke(ctx, id, "pushdata", b.head, get_audit_cb, (void*)&wl_buffer, 3000);
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
	osal_sem_release(sem_321_process);
	return 0 ;
}


static void get_rtc_cb(struct ubus_request *req,
				    int type, struct blob_attr *msg)
{
	struct blob_attr *tb[__RETURN_MAX];
	int len;
    uint8_t *rtc_data;
	blobmsg_parse(return_policy, __RETURN_MAX, tb, blob_data(msg), blob_len(msg));
    
	if (!tb[RETURN_STATUS]) {
		fprintf(stderr, "No return code received from server\n");
		return;
	}

    rtc_data = (uint8_t *)req->priv;
        
	blobmsg_get_u32(tb[RETURN_STATUS]);
    
    
    if(tb[RETURN_STRHEX] != NULL){
        
        unsigned char *data = (unsigned char *)blobmsg_data(tb[RETURN_STRHEX]);
        
        if(data != NULL){

        
            len = blobmsg_data_len(tb[RETURN_STRHEX]);

            if(len > 16){
                
                printf("error:F[%s] L[%d],len is %d\n", __FILE__, __LINE__,len);

                len = 16;

            }

                            
            memcpy(rtc_data,data,len);

           // req->priv = &wl_lv;

        }
        else{

            printf("rtc return NULL\n");

        }
    }
    else{

        printf("rtc str hex is NULL\n");
    }
    
    
	//fprintf(stderr, "return is %d \n", rc);
}

int get_rtc_data(uint8_t *data_rtc)
{
	uint32_t id;

	if(osal_sem_take(sem_321_process, 3500) != OSAL_EOK){
	 
		 printf("Semaphore return failed. \n");
		 
		 //osal_sem_release(sem_321_process);
		 return -1;

	}
    id = id_322;

    if(id == 0){

        
    	if (ubus_lookup_id(ctx, "ubus321", &id)) {
    		fprintf(stderr, "Failed to look up ubus321 object\n");
    		return -1;
    	}

        id_322 = id;

        printf("\n===========321id is 0X%X=============\n",id);
        
    }
    

	blob_buf_init(&b, 0);
	
	blobmsg_add_u32(&b, "tag", UBUS_CLIENT_GETRTC);
    
    
	//blobmsg_add_field(&b, BLOBMSG_TYPE_UNSPEC, "strhex", NULL, 0);
    
	ubus_invoke(ctx, id, "pushdata", b.head, get_rtc_cb, (void*)data_rtc, 3000);
	osal_sem_release(sem_321_process);
	return 0 ;
}
void ubus_clien_init(void)
{

    const char *ubus_socket = NULL;

    uloop_init();

    ctx = ubus_connect(ubus_socket);
    if (!ctx) {
        fprintf(stderr, "Failed to connect to ubus\n");
        return;
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
        err = osal_queue_send(p_sys->queue_msg, p_msg, len, 0, 3000);
    }

    if (err != OSAL_STATUS_SUCCESS) {
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_WARN, "%s: failed=[%d], msg=%04x,tg=%d\n",\
                           __FUNCTION__, err, msg_id,msg_argc);
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
    //printf("state before\n");
    ccid->usb_state = state_d;
    ccid->toggle_state = 0xAA;
    //printf("state after\n");
    return 0;
}



void sys_manage_proc(msg_manager_t *p_sys, sys_msg_t *p_msg)
{

    int i;
    unsigned char door_state[2];
    //msg_manager_t *p_sys = &p_controll_eg->msg_manager;
    uint8_t index_cnt = 0;
    
    switch(p_msg->id){
        
    case SYS_MSG_INITED:

       OSAL_MODULE_DBGPRT(p_controll_eg->usb_ccid_322[p_msg->argc].usb_port, OSAL_DEBUG_INFO, "port %d: state init\n",p_msg->argc);
       
        state_alternate(USB_COMM_STATE_INIT,&p_controll_eg->usb_ccid_322[p_msg->argc]);
       
        break;
        
    case SYS_MSG_SEND_CE:
        
        ce_send_fg++;
        index_cnt = 0;
        if(ce_send_fg >= p_controll_eg->cnt_322){

            update_ce();
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "send 322 ce\n");

            sleep(1);
            
            printf("===========request p2p key===============\n");
            ubus_client_process(UBUS_CLIENT_GETP2P,NULL,NULL,0);

            
            printf("\n===========list active 322+pr11===============\n");
            
            for(i = 0;i < MAX_322_NUM;i++ ){
            
                if(controll_eg.usb_ccid_322[i].pr11_exist){
                    
                    controll_eg.index_322[index_cnt] = controll_eg.usb_ccid_322[i].ccid322_index;
                    printf("%02X ",controll_eg.index_322[index_cnt]);    
                    index_cnt++;
            
                }
                
            
            }
            printf("\n==============================================\n");
        }
     
        break;


    case SYS_MSG_SEND_DRSTATE:

        memcpy(door_state,p_msg->argv,2);
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "send door state:%d,%d\n",door_state[0],door_state[1]);
        ubus_net_process(UBUS_CLIENT_SEND_DOOR_INFO,NULL,door_state,2);

        break;

/*
    case SYS_MSG_SEND_CTRLINFO:
        
        info_send_fg++;
        
        if(info_send_fg >= p_controll_eg->cnt_322){

            update_ce();
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "send ctrl info\n");

            sleep(1);
            
        }


        break;
*/

    case SYS_MSG_SEND_COSVERSION:
        
        for(i = 0;i < MAX_322_NUM;i++ ){
        
            if(controll_eg.usb_ccid_322[i].ccid322_exist){//use 322 as flag

                
                state_alternate(USB_COMM_STATE_VERSION,&controll_eg.usb_ccid_322[i]);
                //break;
            }
            

        }

        break;
                
    case SYS_MSG_UPDATE_MACKEY:
        
        for(i = 0;i < MAX_322_NUM;i++ ){
        
            if(controll_eg.usb_ccid_322[i].pr11_exist){

                
                state_alternate(USB_COMM_STATE_MACKEY,&controll_eg.usb_ccid_322[i]);
                controll_eg.usb_ccid_322[i].init_flag |= INIT_MASK_MAC;
            }
            
        
        }

        break;
            
    case SYS_MSG_UPDATE_P2PKEY:

        printf("================find 321 id===============\n");
        ubus_321_find();
        printf("================find nbi id===============\n");
        ubus_pc02nbi_find();
        
        for(i = 0;i < MAX_322_NUM;i++ ){
        
            if(controll_eg.usb_ccid_322[i].ccid322_exist){

                
                state_alternate(USB_COMM_STATE_P2P,&controll_eg.usb_ccid_322[i]);
                controll_eg.usb_ccid_322[i].init_flag |= INIT_MASK_P2P;
            }
            
        
        }

        break;        

    case SYS_MSG_UPDATE_BASECFG:
        //update_cfg();
        index_cnt = 0;
        
        for(i = 0;i < MAX_322_NUM;i++ ){
        
            if(controll_eg.usb_ccid_322[i].pr11_exist){
                
                //memcpy(&controll_eg.ctrl_cfg_rt[8 + index_cnt],);//8,9 is return data;10,index
                controll_eg.base_cfg_rt[11 + index_cnt] = controll_eg.usb_ccid_322[i].ccid322_index;
                index_cnt++;

            }
            
        
        }
        for(i = 0;i < MAX_322_NUM;i++ ){
        
            if(controll_eg.usb_ccid_322[i].pr11_exist){
                
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

        index_cnt = 0;
        
        for(i = 0;i < MAX_322_NUM;i++ ){
        
            if(controll_eg.usb_ccid_322[i].pr11_exist){
                
                //memcpy(&controll_eg.ctrl_cfg_rt[8 + index_cnt],);//8,9 is return data;10-index
                controll_eg.ctrl_cfg_rt[11 + index_cnt] = controll_eg.usb_ccid_322[i].ccid322_index;
                index_cnt++;

            }
            
        
        }
        
        for(i = 0;i < MAX_322_NUM;i++ ){
        
            if(controll_eg.usb_ccid_322[i].pr11_exist){
                
                //controll_eg.usb_ccid_322[i].usb_state = USB_COMM_STATE_P2P;
                state_alternate(USB_COMM_STATE_RDCFG,&controll_eg.usb_ccid_322[i]);
                controll_eg.usb_ccid_322[i].init_flag |= INIT_MASK_CRLCFG;
            }
            
        
        }
    
        break;
    case SYS_MSG_ALARM_ACTIVE:
        
        for(i = 0;i < MAX_322_NUM;i++ ){
        
            if(controll_eg.usb_ccid_322[i].pr11_exist){
                
                //controll_eg.usb_ccid_322[i].usb_state = USB_COMM_STATE_P2P;
                state_alternate(USB_COMM_ALARM_OPEN,&controll_eg.usb_ccid_322[i]);
                //break;
            }
            
        
        }
        break;
        
    case SYS_MSG_INFO_PUSH:
        
        //printf("get push info %d\n",p_msg->argc);
        state_alternate(USB_COMM_STATE_PUSH,&controll_eg.usb_ccid_322[p_msg->argc]);
        break;
	
    case SYS_MSG_RTC_PUSH:
	   
	   //printf("get push info %d\n",p_msg->argc);
	   state_alternate(USB_COMM_RTC_PUSH,&controll_eg.usb_ccid_322[p_msg->argc]);
	   break;


   case SYS_MSG_REMOTE_OPEN:
    
       for(i = 0;i < MAX_322_NUM;i++ ){
       
           if(controll_eg.usb_ccid_322[i].pr11_exist){
               
               state_alternate(USB_COMM_REMOTE_OPEN,&controll_eg.usb_ccid_322[i]);
               //break;
           }
           
       
       }
        break;

   case SYS_MSG_FACE_REMOTE_OPEN:
    
       for(i = 0;i < MAX_322_NUM;i++ ){
       
           if(controll_eg.usb_ccid_322[i].pr11_exist){

                printf("\nnumber %d 322 = 0X%X 0X%X 0X%X 0X%X\n",i,controll_eg.usb_ccid_322[i].pid_322[0],controll_eg.usb_ccid_322[i].pid_322[1],\
                    controll_eg.usb_ccid_322[i].pid_322[2],controll_eg.usb_ccid_322[i].pid_322[3]);

                printf("\ndes 322 = 0X%X 0X%X 0X%X 0X%X\n",controll_eg.remote_buffer[10],controll_eg.remote_buffer[11],\
                    controll_eg.remote_buffer[12],controll_eg.remote_buffer[13]);

                if(memcmp(&controll_eg.remote_buffer[10],&controll_eg.usb_ccid_322[i].pid_322[0],4) == 0) {             
                    printf("\nopen this 322 door\n");
                    state_alternate(USB_FACE_REMOTE_OPEN,&controll_eg.usb_ccid_322[i]);
                }
                else{
                    printf("\n322 id do not match\n");
                }
               //break;
           }
           
       
       }
        break;
        
   case SYS_MSG_ID_REMOTE_OPEN:
    
       for(i = 0;i < MAX_322_NUM;i++ ){
       
           if(controll_eg.usb_ccid_322[i].pr11_exist){

                printf("\nnumber %d 322 = 0X%X 0X%X 0X%X 0X%X\n",i,controll_eg.usb_ccid_322[i].pid_322[0],controll_eg.usb_ccid_322[i].pid_322[1],\
                    controll_eg.usb_ccid_322[i].pid_322[2],controll_eg.usb_ccid_322[i].pid_322[3]);

                printf("\ndes 322 = 0X%X 0X%X 0X%X 0X%X\n",controll_eg.remote_buffer[10],controll_eg.remote_buffer[11],\
                    controll_eg.remote_buffer[12],controll_eg.remote_buffer[13]);

                if(memcmp(&controll_eg.remote_buffer[10],&controll_eg.usb_ccid_322[i].pid_322[0],4) == 0) {             
                    printf("\nopen this 322 door\n");
                    state_alternate(USB_ID_REMOTE_OPEN,&controll_eg.usb_ccid_322[i]);
                }
                else{
                    printf("\n322 id do not match\n");
                }
               //break;
           }
           
       
       }
        break;
        
    case ZMQ_MSG_ID:
        OSAL_MODULE_DBGPRT(controll_eg.usb_ccid_322[p_msg->argc].usb_port, OSAL_DEBUG_INFO, "get zmq msg\n");
        state_alternate(USB_COMM_ID_READ,&controll_eg.usb_ccid_322[p_msg->argc]);
        
        break;

    case ZMQ_RESULT:
        state_alternate(USB_COMM_ID_DOOR_SERVER,&controll_eg.usb_ccid_322[p_msg->argc]);
        break;
        
    case SYS_MSG_ALARM_CLEAR:
     
        for(i = 0;i < MAX_322_NUM;i++ ){
        
            if(controll_eg.usb_ccid_322[i].pr11_exist){
                
                state_alternate(USB_COMM_CLEAR_ALARM,&controll_eg.usb_ccid_322[i]);
                //break;
            }
            
        
        }
         
         
     break;

    case SYS_MSG_322_USBTEST:
            
            //printf("322 usb test %d\n",p_msg->argc);
            state_alternate(USB_322_TEST,&controll_eg.usb_ccid_322[p_msg->argc]);
            break;

    case SYS_MSG_322_RETURN:
        
        //sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_322_RETURN,USB_COMM_STATE_RDCFG,return_req,NULL);
        
        if(p_msg->len == USB_COMM_STATE_RDCFG){
            
            return_send_fg++;
            printf("\nport %d ctrl cfg return\n",(0x00FF0000&p_msg->argc)>>16);
           if( (0x9000 != (0x0000FFFF&p_msg->argc))||(0x900A != (0x0000FFFF&p_msg->argc))){
               controll_eg.ctrl_cfg_rt[68] = 0x90;
               controll_eg.ctrl_cfg_rt[69] = 0x00;
            }else{
            
               controll_eg.ctrl_cfg_rt[68] = (0x0000FF00&p_msg->argc)>>8;
               controll_eg.ctrl_cfg_rt[69] = (0x000000FF&p_msg->argc);

            }
            
            if(return_send_fg >= p_controll_eg->cnt_322){

                
                
                //memcpy(return_buffer,controll_eg.ctrl_cfg_rt,51);//ctrl_cfg_rt is 51 Byte buffers
                
                //memcpy(return_buffer[51],(uint8_t*)(&return_array),return_send_fg * 3);
                
                printf("\n");
                for(i = 0;i < 71;i++){

                    printf("0x%x ",controll_eg.ctrl_cfg_rt[i]);
                }
                printf("\n");
                controll_eg.ctrl_cfg_rt[70] = 1;
                ubus_net_process(UBUS_CLIENT_RETURN,NULL,controll_eg.ctrl_cfg_rt,71);//path:68,return:2,num:1
                return_send_fg = 0;
                osal_sem_release(controll_eg.sem_ctrl_cfg);        
            }


        }
        else if(p_msg->len == USB_COMM_STATE_CFG){
            

             return_base_fg++;
            printf("\nport %d base cfg return\n",(0x00FF0000&p_msg->argc)>>16);
            if( (0x9000 != (0x0000FFFF&p_msg->argc))||(0x900A != (0x0000FFFF&p_msg->argc))){
                controll_eg.base_cfg_rt[68] = 0x90;
                controll_eg.base_cfg_rt[69] = 0x00;
             }else{
             
                controll_eg.base_cfg_rt[68] = (0x0000FF00&p_msg->argc)>>8;
                controll_eg.base_cfg_rt[69] = (0x000000FF&p_msg->argc);

             }
             
             if(return_base_fg >= p_controll_eg->cnt_322){

                 
                 
                 printf("\n");
                 for(i = 0;i < 71;i++){

                     printf("0x%x ",controll_eg.base_cfg_rt[i]);
                 }
                 controll_eg.base_cfg_rt[70] = 1;
                 ubus_net_process(UBUS_CLIENT_RETURN,NULL,controll_eg.base_cfg_rt,71);//path:68,return:2,num:1
                 return_base_fg = 0;
                 osal_sem_release(controll_eg.sem_base_cfg);        
             }


        }

        break;

    case SYS_MSG_NET_STATE:

        for(i = 0;i < MAX_322_NUM;i++ ){
        
            if(controll_eg.usb_ccid_322[i].pr11_exist){
                
                state_alternate(USB_NET_STATE,&controll_eg.usb_ccid_322[i]);
            }
            
        
        }
        
        break;

	case SYS_MSG_RFU:

		printf("\nsend log to server,len is %d\n",sizeof(log_test));//for test
		get_log_time(&log_test[98]);
		ubus_client_process(UBUS_CLIENT_LOG, NULL, log_test, sizeof(log_test));
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
            //osal_free(p_msg);
        }
        else{
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_ERROR, "%s: osal_queue_recv error [%d]\n", __FUNCTION__, err);
        }
        
    }



}





void msg_manager_init(void)
{

    msg_manager_t *p_msg = &p_controll_eg->msg_manager;


    sem_321_process= osal_sem_create("sem_321_process", 1);
    osal_assert(sem_321_process != NULL);
    
    sem_net_process= osal_sem_create("sem_net_process", 1);
    osal_assert(sem_net_process != NULL);
    
    ubus_clien_init();
    /* object for sys */
    p_msg->queue_msg = osal_queue_create("q-msg", SYS_QUEUE_SIZE, SYS_MQ_MSG_SIZE);
    osal_assert(p_msg->queue_msg != NULL);

    p_msg->task_msg = osal_task_create("task-msg",
                           msg_thread_entry, p_msg,
                           PC02_MSG_THREAD_STACK_SIZE, PC02_MSG_THREAD_PRIORITY);
    osal_assert(p_msg->task_msg != NULL); 


}


