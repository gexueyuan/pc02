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
//#include "cv_osal.h"

//extern Controller_t controll_eg;
//extern Controller_t *p_controll_eg;
//extern osal_status_t sys_add_event_queue(msg_manager_t *p_sys, 
//                             uint16_t msg_id, 
//                             uint16_t msg_len, 
//                             uint32_t msg_argc,
//                             void    *msg_argv);

static struct ubus_context *ctx;
static struct blob_buf b;
unsigned char p2p_buffer[128] = {0};
unsigned char mac_buffer[128] = {0};

unsigned char base_cfg_buffer[128] = {0};
//unsigned char base_cfg_rt[128] = {0};

unsigned char ctrl_cfg_buffer[128] = {0};
//unsigned char ctrl_cfg_rt[128] = {0};
extern void ubus_321_find(void);


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
int readCFG(const  char * _fileName, unsigned char* cfg_buffer)
{

    FILE* fp = NULL;
    int len;
    int i;
    //char cmd[200];
    
    if( NULL == _fileName) return (-1);

    fp = fopen(_fileName, "rb"); // 必须确保是以 二进制读取的形式打开 

    if( NULL == fp )
    {
        return (-1);
    }

    fseek (fp , 0 , SEEK_END);  

    len =  ftell (fp); 
    
    rewind(fp);

    //cfg->len = len;
    

    //cfg->data = (unsigned char*)malloc(len);
    
    printf("\nlen is %d\n",len);

    if(len < 128)
        fread(cfg_buffer, len, 1, fp); // 二进制读
    else{
            printf("len too long\n");
            return -1;
        }
    
    printf("\n============================\n");
    for(i = 0; i< len;i++){
        printf("%02x ",cfg_buffer[i]);
    }
    printf("\n============================\n");
    
    fclose(fp);

    //sprintf(cmd, "rm %s", _fileName);
    //system(cmd);
    unlink(_fileName);
    
    printf("get cfg\n");
    
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
int readfile(const char* _fileName, unsigned char* cfg)
{

    FILE* fp = NULL;
    int len;
    int i;
    //char cmd[200];
    
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
    int i = 0;
    int len = 0;
    char *pStr = NULL;
    unsigned char *data=NULL;
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

		if(UBUS_SERVER_NETWORK_STATE  != if_tag){
	        printf("get ubus len is %d\n",len);

		    for(i = 0; i < len;i++){
		        printf("%02X ",data[i]);
		    }
	    	printf("\n");
		}
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
    //memset(p2p_buffer,0,sizeof(p2p_buffer));
    //memset(mac_buffer,0,sizeof(mac_buffer));
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
            
                if(controll_eg.usb_ccid_322[i].pr11_exist){

                   controll_eg.usb_ccid_322[i].rtc_sync = 0xAA;
                    

                   sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_INFO_PUSH,0,controll_eg.usb_ccid_322[i].ccid322_index - 1,NULL);
				   sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_RTC_PUSH,0,controll_eg.usb_ccid_322[i].ccid322_index - 1,NULL);
                   // state_alternate(USB_COMM_REMOTE_OPEN,&controll_eg.usb_ccid_322[i]);
                    //break;
                }
                
            
            }
            break;

        case UBUS_SERVER_321UBUS_UPDATE:
            printf("================321 startup ,update 321ubus id===============\n");
            ubus_321_find();

            break;
            
        case UBUS_SERVER_BASE_CFG:

            
            /* Take the semaphore. */
            printf("\nget base cfg\n");
            if(osal_sem_take(controll_eg.sem_base_cfg, OSAL_WAITING_FOREVER) != OSAL_EOK)
            {
               printf("\n Semaphore return failed. \n");
               return 0;
            }

            controll_eg.basecfg.data = base_cfg_buffer;
            printf("base cfg path:%s\n",pStr);

            //strcpy(controll_eg.base_cfg_rt,pStr);/*/tmp/pc02nbi/xxxxx.bin*/

            StrToHex(controll_eg.base_cfg_rt,&pStr[13],2);
            StrToHex(&controll_eg.base_cfg_rt[2],&pStr[UBUS_STR_NUM_OFFSET],UBUS_STR_NUM_CN/2);

            controll_eg.basecfg.len = readCFG(pStr,base_cfg_buffer);
            

            sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_UPDATE_BASECFG,0,0,NULL);
            
            break;
            
            
        case UBUS_SERVER_READER_CFG:
            
            
            printf("\nget ctrl cfg\n");
            /* Take the semaphore. */
            if(osal_sem_take(controll_eg.sem_ctrl_cfg, OSAL_WAITING_FOREVER) != OSAL_EOK)
            {
               printf("\n Semaphore return failed. \n");
               return 0;
            }          
            controll_eg.ctlcfg.data = ctrl_cfg_buffer;
            //printf("\ncontroll_eg.ctlcfg.data address is %02X\n",controll_eg.ctlcfg.data);
            printf("ctrl cfg path:%s\n",pStr);
            
            //memcpy(controll_eg.ctrl_cfg_rt,pStr,strlen(pStr));
            //strcpy(controll_eg.ctrl_cfg_rt,pStr);/*/tmp/pc02nbi/xxxxx.bin*/
            
            StrToHex(controll_eg.ctrl_cfg_rt,&pStr[13],2);
            StrToHex(&controll_eg.ctrl_cfg_rt[2],&pStr[UBUS_STR_NUM_OFFSET],UBUS_STR_NUM_CN/2);

            
            controll_eg.ctlcfg.len = readCFG(pStr,ctrl_cfg_buffer);

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
            
#if 1
                /* Take the semaphore. */
            if(osal_sem_take(controll_eg.sem_remote, OSAL_WAITING_FOREVER) != OSAL_EOK)
            {
               printf("\n Semaphore return failed. \n");
               return 0;
            }
            memset(p_controll_eg->remote_buffer,0,sizeof(p_controll_eg->remote_buffer));//32+data
            
            if(len > 32)
                len = len - 32;//len=remote data exclude 32bytes 
            else
                printf("\nremote buffer len is %d,something wrong!\n",len);
            
            controll_eg.remote_buffer[0] = (unsigned char)((len&0xFF00)>>8);
            controll_eg.remote_buffer[1] = (unsigned char)(len&0x00FF);
            
            memcpy(&controll_eg.remote_buffer[2],data,len + 32);//copy all data

            sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_REMOTE_OPEN,0,0,NULL);
#else
            remote_buffer = (unsigned char*)malloc(len + 2);

            if(remote_buffer == NULL){

                printf("malloc failed\n");
                break;

            }
            memset(remote_buffer,0,len + 2);
            remote_buffer[0] = (unsigned char)((len&0xFF00)>>8);
            remote_buffer[1] = (unsigned char)(len&0x00FF);

            memcpy(&remote_buffer[2],data,len);

            sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_REMOTE_OPEN,0,(uint32_t)remote_buffer,NULL);
#endif
            break;
            
         case UBUS_SERVER_CLEAR_ALARM:
             memset(p_controll_eg->alarm_clear,0,sizeof(p_controll_eg->alarm_clear));
             if(len == 16)               
                memcpy(p_controll_eg->alarm_clear,data,16);
             else if(len == 48)
                memcpy(p_controll_eg->alarm_clear,data,sizeof(p_controll_eg->alarm_clear));
             sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_ALARM_CLEAR,0,0,NULL);
            break;
            
        case UBUS_SERVER_FACE:
            /* Take the semaphore. */
            if(osal_sem_take(controll_eg.sem_remote, OSAL_WAITING_FOREVER) != OSAL_EOK)
            {
               printf("\n Semaphore return failed. \n");
               return 0;
            }
            memset(p_controll_eg->remote_buffer,0,sizeof(p_controll_eg->remote_buffer));//32+data
            
            if(len > 32)
                len = len - 32;//len=remote data exclude 32bytes 
            else
                printf("\nID remote buffer len is %d,something wrong!\n",len);
            
            controll_eg.remote_buffer[0] = (unsigned char)((len&0xFF00)>>8);
            controll_eg.remote_buffer[1] = (unsigned char)(len&0x00FF);
            
            memcpy(&controll_eg.remote_buffer[2],data,len + 32);//copy all data

            sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_FACE_REMOTE_OPEN,0,0,NULL);

            
            break;
            
        case UBUS_SERVER_REMOTE_ID:
            
            /* Take the semaphore. */
            if(osal_sem_take(controll_eg.sem_remote, OSAL_WAITING_FOREVER) != OSAL_EOK)
            {
               printf("\n Semaphore return failed. \n");
               return 0;
            }
            memset(p_controll_eg->remote_buffer,0,sizeof(p_controll_eg->remote_buffer));//32+data
            
            if(len > 32)
                len = len - 32;//len=remote data exclude 32bytes 
            else
                printf("\nID remote buffer len is %d,something wrong!\n",len);
            
            controll_eg.remote_buffer[0] = (unsigned char)((len&0xFF00)>>8);
            controll_eg.remote_buffer[1] = (unsigned char)(len&0x00FF);
            
            memcpy(&controll_eg.remote_buffer[2],data,len + 32);//copy all data

            sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_ID_REMOTE_OPEN,0,0,NULL);

            
            break;
    case UBUS_SERVER_NETWORK_STATE:
        
            //controll_eg.network_state = data[0];
            //pr11默认状态是在线状态
            if(0 == data[0]){

                    if(1 == controll_eg.network_state){


                        osal_printf("\nnetwork offline\n");
                        //sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_NET_STATE,0,controll_eg.network_state,NULL);
                    }
                }
            else{

                //printf("\nnetwork online\n");
                
                    if(0 == controll_eg.network_state){
                    
                    
                        osal_printf("\nnetwork online\n");
                        //sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_NET_STATE,0,controll_eg.network_state,NULL);
                    }
            }
            
            controll_eg.network_state = data[0];
            sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_NET_STATE,0,controll_eg.network_state,NULL);
            break;
	case UBUS_SERVER_RFU:

			
			printf("\nubus get log\n");
            sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_RFU,0,0,NULL);
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

int add_ubus_event(void)
{	
    e = event_new(evloop, ctx->sock.fd, EV_READ | EV_PERSIST, ubus_event_cb, ctx);
    event_add(e, NULL);
	return 0;
}

int ubus_release(void){	ubus_free(ctx);return 0; }
int ubus_init(void)
{
    //int ret = 0;
    const char *ubus_socket = "/var/run/ubus.sock";
    ctx = ubus_connect(ubus_socket);
    if (!ctx) {
        fprintf(stderr, "Failed to connect to ubus\n");
        return -1;
        }   
    if (ubus_add_object (ctx, &test_object))
        return -1;

	return 0;
}  
void* ubus_server_thread_entry(void * parameter)
{

    ubus_init();
    evloop = event_base_new();
    add_ubus_event();
	
    //添加ubus event
    //add_alarm_timer_event();
    event_base_dispatch(evloop);
    ubus_release();

	return NULL;

}

void ubus_interface_init(void)
{
    osal_task_t *tid;

    memset(p2p_buffer,0,sizeof(p2p_buffer));
    memset(mac_buffer,0,sizeof(mac_buffer));
    memset(ctrl_cfg_buffer,0,sizeof(ctrl_cfg_buffer));    
    memset(base_cfg_buffer,0,sizeof(base_cfg_buffer));
    tid = osal_task_create("tk_ubus_server",
                        ubus_server_thread_entry,
                        NULL,PC02_UBUS_THREAD_STACK_SIZE, PC02_UBUS_THREAD_PRIORITY);

    osal_assert(tid != NULL);

    

}
