/*****************************************************************************
 Copyright(C) Tendyron Corporation
 All rights reserved.
 
 @file   : ubus_interface.c
 @brief  : ubus interface define
 @author : gexueyuan
 @history:
           2017-5-10    gexueyuan    Created file
           ...
******************************************************************************/

#include <unistd.h>
#include <signal.h>

#include <libubox/blobmsg_json.h>
#include "libubus.h"
#include "count.h"
#include "cv_cms_def.h"
#include "ubus_mgr.h"

static struct ubus_context *ctx;
static struct blob_buf b;
unsigned char p2p_buffer[128] = {0};
unsigned char mac_buffer[128] = {0};
static long get_file_len(FILE *file)
{
    long len; 

    fseek (file , 0 , SEEK_END);  

    len =  ftell (file);  
    rewind(file);
    return len;

}


/*
 * 函数说明:  读二进制文件
*  参数描述: _fileName, 文件名称
*             _buf, 读出来的数据存放位置
*             _bufLen, 数据的长度信息
*    返回值:  0, 成功
*             -1, 失败
*
*/
int readCFG(const char* _fileName, key_buffer_t* cfg)
{

    FILE* fp = NULL;
    long len;
    int i;
    char cmd[200];
    
    if( NULL == _fileName) return (-1);

    fp = fopen(_fileName, "rb"); // 必须确保是以 二进制读取的形式打开 

    if( NULL == fp )
    {
        return (-1);
    }

    fseek (fp , 0 , SEEK_END);  

    len =  ftell (fp); 
    
    rewind(fp);

    cfg->len = len;
    

    cfg->data = (unsigned char*)malloc(len);
    printf("\nlen is %d\n",len);

    fread(cfg->data, len, 1, fp); // 二进制读
    
    printf("\n============================\n");
    for(i = 0; i< cfg->len;i++){
        printf("%02x ",cfg->data[i]);
    }
    printf("\n============================\n");
    
    fclose(fp);

    //sprintf(cmd, "rm %s", _fileName);
    //system(cmd);
    unlink(_fileName);
    
    printf("get cfg\n");
    
    return 0;        
}



/*
 * 函数说明:  读二进制文件
*  参数描述: _fileName, 文件名称
*             _buf, 读出来的数据存放位置
*             _bufLen, 数据的长度信息
*    返回值:  0, 成功
*             -1, 失败
*
*/
int readfile(const char* _fileName, unsigned char* cfg)
{

    FILE* fp = NULL;
    long len;
    int i;
    char cmd[200];
    
    if( NULL == _fileName) return (-1);

    fp = fopen(_fileName, "rb"); // 必须确保是以 二进制读取的形式打开 

    if( NULL == fp )
    {
        return (-1);
    }

    fseek (fp , 0 , SEEK_END);  

    len =  ftell (fp); 
    
    rewind(fp);

    cfg[0] = (unsigned char)((len&0xFF00)>>8);
    cfg[1] = (unsigned char)(len&0x00FF);
    

    printf("\nlen is %d\n",len);

    fread(&cfg[2], len, 1, fp); // 二进制读
    
    printf("\n==============remote buffer==============\n");
    for(i = 0; i<len + 2;i++){
        printf("%02x ",cfg[i]);
    }
    printf("\n===============remote buffer end=============\n");
    
    fclose(fp);

    //sprintf(cmd, "rm %s", _fileName);
    //system(cmd);
    unlink(_fileName);
    
    printf("get cfg\n");
    
    return 0;        
}

enum {
    
    REQ_TAG,
    REQ_STR,
    REQ_STR_HEX,
    __REQ_MAX
    
};

/* 接收的解析格式， client发送过来的调用是 blobmsg_add_u32(&b, "getcnt", ap_index); */
static const struct blobmsg_policy fun2_message_parse_policy[__REQ_MAX] = {
	[REQ_TAG] = { .name = "tag", .type = BLOBMSG_TYPE_INT32 },
    [REQ_STR] = { .name = "str", .type = BLOBMSG_TYPE_STRING },
    [REQ_STR_HEX] = { .name = "strhex", .type = BLOBMSG_TYPE_UNSPEC },
};
    
static int fun2_handler(struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg)
{
    struct blob_attr *tb[__REQ_MAX];
    unsigned  int if_tag;
    int i;
    int len ;
    uint8_t *pStr;
    unsigned char *data ;
    uint8_t path_name[256];
    blobmsg_parse(fun2_message_parse_policy, __REQ_MAX, tb, blob_data(msg), blob_len(msg));

    if (!tb[REQ_TAG])
        return UBUS_STATUS_INVALID_ARGUMENT;

    if_tag = blobmsg_get_u32(tb[REQ_TAG]);

    if(tb[REQ_STR_HEX] != NULL){
        data = (unsigned char *)blobmsg_data(tb[REQ_STR_HEX]);
    
    if(data){

    
        len = blobmsg_data_len(tb[REQ_STR_HEX]);

        if(len == 0)
            return -1;

        printf("get ubus len is %d\n",len);

    for(i = 0; i < len;i++){
        printf("%02X ",data[i]);
    }
    printf("\n");
    }
    }
    if(tb[REQ_STR] != NULL) {
        pStr = blobmsg_get_string(tb[REQ_STR]);
    }
    blob_buf_init(&b, 0);

#if 0
    if(tb[PUSHDATA_TAG] != NULL) {
        tag = blobmsg_get_u32(tb[PUSHDATA_TAG]);
        if(tb[PUSHDATA_STR] != NULL) {
            pStr = blobmsg_get_string(tb[PUSHDATA_STR]);
            }
        //printf("UBUS TAG:%x\n", tag);
        //获取hexstr参数
        if(tb[PUSHDATA_HEXSTR] != NULL) {
            hexLen = blobmsg_data_len(tb[PUSHDATA_HEXSTR]);
            pHexStr = (unsigned char *)blobmsg_data(tb[PUSHDATA_HEXSTR]);
            //Debug_hexstring("ubus in", pHexStr, hexLen);
            //printf("hexlen:%d\n", hexLen);
            }
        else {
            //printf("No hexstr.\n");       
            }   
        }
#endif
    memset(p2p_buffer,0,sizeof(p2p_buffer));
    memset(mac_buffer,0,sizeof(mac_buffer));
    switch(if_tag){

        case UBUS_SERVER_P2P:

            
            controll_eg.p2pkey.data = p2p_buffer;//(unsigned char *)malloc(len);//p2p_buffer;//(unsigned char *)malloc(len);
            
            controll_eg.p2pkey.len = len;

            //printf("controll_eg.p2pkey.len is %d\n",controll_eg.p2pkey.len);
            
            if(controll_eg.p2pkey.data == NULL){

                printf("F[%s] L[%d] malloc failed!!!", __FILE__, __LINE__);
                break;
            }
            
            memcpy(controll_eg.p2pkey.data,data,len);
            //printf("controll_eg.p2pkey.len is %d\n",controll_eg.p2pkey.len);
            sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_UPDATE_P2PKEY,0,0,NULL);
            sleep(1);
            break;
        

        case UBUS_SERVER_MACKEY:

            
            
            p_controll_eg->mackey.data = mac_buffer;//(unsigned char *)malloc(len);//mac_buffer;//(unsigned char *)malloc(len);

            p_controll_eg->mackey.len = len;

            //printf("controll_eg.mackey.len is %d\n",controll_eg.mackey.len);

            if(controll_eg.mackey.data == NULL){

                printf("F[%s] L[%d] malloc failed!!!", __FILE__, __LINE__);
                break;
            }
            //printf("%p,%p,%p\n",controll_eg.mackey.data,&controll_eg.mackey.data,mac_buffer);
            memcpy(controll_eg.mackey.data,data,len);
/*
                for(i = 0; i < len;i++){
        printf("%02X ",mac_buffer[i]);
    }
    printf("\n");
*/
            sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_UPDATE_MACKEY,0,0,NULL);
            
            sleep(1);
            break;

        case UBUS_SERVER_TMSYNC:
            printf("================sync  time===============\n");

            for(i = 0;i < MAX_322_NUM;i++ ){
            
                if(controll_eg.usb_ccid_322[i].ccid322_exist){

                   sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_INFO_PUSH,0,controll_eg.usb_ccid_322[i].ccid322_index,NULL);
                    
                   // state_alternate(USB_COMM_REMOTE_OPEN,&controll_eg.usb_ccid_322[i]);
                    //break;
                }
                
            
            }
            break;
            
        case UBUS_SERVER_BASE_CFG:

            
/*
            if(controll_eg.basecfg.data != NULL){

                free(&controll_eg.basecfg.data);

            }
*/
            
            printf("base cfg path:%s\n",pStr);

            readCFG(pStr,&controll_eg.basecfg);
            

            sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_UPDATE_BASECFG,0,0,NULL);
            
            break;
            
            
        case UBUS_SERVER_READER_CFG:
            
/*
            if(controll_eg.ctlcfg.data != NULL){

                free(&controll_eg.ctlcfg.data);

            }
*/
            printf("controll_eg.ctlcfg.data is %02X\n",controll_eg.ctlcfg.data);
            printf("ctrl cfg path:%s\n",pStr);
            
            readCFG(pStr,&controll_eg.ctlcfg);

            sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_UPDATE_READERCFG,0,0,NULL);
            break;
            
        case UBUS_SERVER_TIME_CFG:
            break;
        
        case UBUS_SERVER_ALARM_CFG:
            break;



        case UBUS_SERVER_UPGRADE_COSVERSION:
            printf("check cos version\n");
            
            break;

        case UBUS_SERVER_ALARM:
            controll_eg.alarm_buffer[0] = len;
            memcpy(&controll_eg.alarm_buffer[1],data,len);
            sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_ALARM_ACTIVE,0,0,NULL);
            break;
            
        case UBUS_SERVER_REMOTE:
            
/*
            if(p_controll_eg->ctlcfg.data != NULL){

                free(p_controll_eg->ctlcfg.data);

            }
*/
            //printf("sizeof(p_controll_eg->remote_buffer) is %d\n",sizeof(p_controll_eg->remote_buffer));
            memset(p_controll_eg->remote_buffer,0,sizeof(p_controll_eg->remote_buffer));
            //printf("ctrl remote path:%s\n",pStr);
            
            //readCFG(pStr,p_controll_eg->remote_buffer);

            controll_eg.remote_buffer[0] = (unsigned char)((len&0xFF00)>>8);
            controll_eg.remote_buffer[1] = (unsigned char)(len&0x00FF);

            //printf("len is %d,buffer[0]:%d,buffer[1]:%d\n",len,p_controll_eg->remote_buffer[0],p_controll_eg->remote_buffer[1]);
            
            memcpy(&controll_eg.remote_buffer[2],data,len);

            sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_REMOTE_OPEN,0,0,NULL);
            break;
        default:
            break;

    }
    
    //返回客户端请求的station个数
    blobmsg_add_u32(&b, "status", 0);
    //发送
    ubus_send_reply(ctx, req, b.head);
    return 0;
    
}
static const struct ubus_method test_methods[] = {
	UBUS_METHOD("pushdata", fun2_handler, fun2_message_parse_policy),
};

static struct ubus_object_type test_object_type =
	UBUS_OBJECT_TYPE("ubus322", test_methods);

static struct ubus_object test_object = {
	.name = "ubus322",
	.type = &test_object_type,
	.methods = test_methods,
	.n_methods = ARRAY_SIZE(test_methods),
};

/*
static void uloop_handler_INT(int signo)
{
	    printf("away we go\n");
	    //uloop_loop = 0;
}
	
static void uloop_setup_signals(void)
{
    struct sigaction s1, s2;
    memset(&s1, 0, sizeof(struct sigaction));
    memset(&s2, 0, sizeof(struct sigaction));
    s1.sa_handler = uloop_handler_INT;
    s1.sa_flags = 0;
    sigaction(SIGINT, &s1, NULL);
    s2.sa_handler = uloop_handler_INT;
    s2.sa_flags = 0;
    sigaction(SIGTERM, &s2, NULL);
}
*/


/*
static void server_main(void)
{
	int ret;

	ret = ubus_add_object(ctx, &test_object);
	if (ret)
		fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));
    
    signal(SIGINT, SIG_DFL);
    
	signal(SIGTERM, SIG_DFL);
	uloop_run();
}



void ubus_server_thread_entry(void * parameter)
{
	const char *ubus_socket = NULL;
	int ch;

	uloop_init();
	//signal(SIGINT | SIGTERM, SIG_DFL);

	ctx = ubus_connect(ubus_socket);
	if (!ctx) {
		fprintf(stderr, "Failed to connect to ubus\n");
		return -1;
	}

	ubus_add_uloop(ctx);

	server_main();
   // printf("ubus server done\n");
	ubus_free(ctx);
	uloop_done();

	return 0;
}
*/
void ubus_event_cb(evutil_socket_t fd, short what, void *arg)
{ 
    struct ubus_context *ctx = arg;
    ubus_handle_event(ctx);
}

int add_ubus_event()
{	
    e = event_new(evloop, ctx->sock.fd, EV_READ | EV_PERSIST, ubus_event_cb, ctx);
    event_add(e, NULL);
}

int ubus_release(){	ubus_free(ctx); }
int ubus_init()
{
    int ret;
    const char *ubus_socket = "/var/run/ubus.sock";
    ctx = ubus_connect(ubus_socket);
    if (!ctx) {
        fprintf(stderr, "Failed to connect to ubus\n");
        return -1;
        }   
    if (ubus_add_object (ctx, &test_object))
        return -1;
}  
void ubus_server_thread_entry(void * parameter)
{

    ubus_init();
    evloop = event_base_new();
    add_ubus_event();
    //添加ubus event
    //add_alarm_timer_event();
    event_base_dispatch(evloop);

    ubus_release();

}

void ubus_interface_init(void)
{
    osal_task_t *tid;


    tid = osal_task_create("tk_ubus_server",
                        ubus_server_thread_entry,
                        NULL,LESS_THREAD_STACK_SIZE, RT_SYS_THREAD_PRIORITY);

    osal_assert(tid != NULL);


}
