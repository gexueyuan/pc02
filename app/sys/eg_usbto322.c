/*****************************************************************************
 Copyright(C) Tendyron Corporation
 All rights reserved.
 
 @file   : eg_usbto322.c
 @brief  : usbto322
 @author : gexueyuan
 @history:
           2016-12-9    gexueyuan    Created file
           ...
           2017-04-26   gexueyuan    modified file
******************************************************************************/
#include "cv_osal.h"
    
#define OSAL_MODULE_DEBUG
#define OSAL_MODULE_DEBUG_LEVEL OSAL_DEBUG_INFO
#define MODULE_NAME "eg_usbto322"
#include "cv_osal_dbg.h"
OSAL_DEBUG_ENTRY_DEFINE(eg_usbto322);


#include <stdlib.h>

#include <errno.h> 

#include "cv_cms_def.h"
#include <time.h> 
#include "stdafx.h"
#include "luareader.h"

#include "libzmqtools.h"



#define USB_FAILED    0x6E00
#define USB_OK        0x9000 
#define EUSB_SEND_PERIOD  6000 



#define VENDOR_ID 0x1780  
#define PRODUCT_ID 0x0312

extern   void get_wl(uint8_t *id_lv,uint8_t *data_wl,int *wlen);
extern   void get_wl_4(uint8_t *id_lv,uint8_t *data_wl,int *wlen);
extern   int get_audit_data(unsigned int tag,unsigned char* strhex,int strlen,uint8_t *data_wl,int *wlen);

extern   void ubus_client_process(unsigned int tag,char* str,unsigned char* strhex,int strlen);
extern   void ubus_net_process(unsigned int tag,char* str,unsigned char* strhex,int strlen);

extern     void update_ce(void);

extern    void send_log(unsigned char* log_buffer,int len);

extern int get_rtc_data(uint8_t *data_rtc);

extern void eg_zmq_init(usb_ccid_322_t* argv);

const static int TIMEOUT=1000; /* timeout in ms */  



typedef enum _USB_CMD {
    USB_CMD_CON = 0,
    USB_CMD_DATA,        
    USB_CMD_CHK,   
    USB_CMD_DIS,
    USB_CMD_ACL,
    USB_CMD_LOG,
} E_USB_CMD;

typedef enum _CARDPOLLEVENT {
    CARDPOLLEVENT_TEST_WG = 0,
    CARDPOLLEVENT_NO_EVENT,
    CARDPOLLEVENT_GET_CNT,
    CARDPOLLEVENT_GET_LOGNUM,
    CARDPOLLEVENT_GET_LOG,
    CARDPOLLEVENT_BTN,
    CARDPOLLEVENT_POWER,
    CARDPOLLEVENT_WLNUM,
    CARDPOLLEVENT_WL,
    CARDPOLLEVENT_ID,
} E_CARDPOLLEVENT;

const unsigned char test1[] = {0xFC,0xA0,0x00,0x00,0x05,0x80,0xCA,0xCE,0x52,0x00};//{0xe0,0xfd,0x00,0x00,0x00,0x00,0x20,0xB1,0xC8,0xFE,0xF7,0x63,0x81,0xF6,0xB6,0x75,0x80,0xEF,0xD2,0xC3,0xAA,0xC4,0x7E,0x5A,0x58,0xC0,0xDD,0x5F,0x66,0x96,0x61,0x5C,0xC1,0x26,0x8B,0xA1,0xB1,0xF4,0x86};

const unsigned char check[] = {0x00,0xA4,0x04,0x00,0x00};//+len+data

const uint8_t car_detect[] = {0xFC,0xA0,0x00,0x00,0x05,0x80,0x30,0x00,0x00,0x00};


const uint8_t refuse_enter[] = {0xFC,0xA0,0x04,0x00,0x09,0x80,0x32,0x80,0x00,0x04,  0x01,0x00,0x01,0x00};

uint8_t test_all[] = {0xFC,0xA0,0x00,0x00,  /*总长*/0x23  ,0x80,0x34,0x00,0x00, /*长度*/0x18 /**温度范围**/ ,0x02,0x00,0x09,0x32,0x38,0xA1,0xE6,0x2F,0x31,0x35,0xA1,0xE6,\
                      0x03,0x00,0x04  ,0x32,0x35,0xA1,0xE6,/*天气*/0x04,0x00,0x01,0x01,/*位置北京*/0x05,0x00,0x04,0xB1,0xB1,0xBE,0xA9};

unsigned char  change_app[25] = {0xFC,0xA0,0x04,0x00,0x14,0x00,0xA4,0x04,0x00,0x0F,0x74,0x64,0x72,0x6F,0x6E,0x99,0x05,0x69,0x64,0x52,0x64,0x51,0x34,0x42,0x31};

E_CARDPOLLEVENT card_event;

uint8_t  test_cmd[] = {0xFC,0xC1, 0x01 ,0x00, 0x06,0x80,0xB0,0x55,0xAF,0x01,0x00};

 /*透传头*/
 const unsigned char transfer_head[] = {0xFC,0xA0,0x00,0x00};//+len+data

const uint8_t cfg_base_head[] = {0x00,0x25,0x00,0x00};//+len+data

const uint8_t p2pkey_head[] = {0xE0,0xFD,0x00,0x00};//+len+data

const uint8_t mackey_head[] = {0x00,0x2A,0x00,0x00};//+len+data

const uint8_t basecfg_head[] = {0x00,0x25,0x00,0x00};//+len+data

const uint8_t ctlcfg_head[] = {0x00,0x29,0x00,0x00};//+len+data

const uint8_t result_head[] = {0x00,0x22,0x00,0x00};//+len+data

const uint8_t alarm322_head[] = {0x00,0x23,0x00,0x00,0x10};//+len+data

const uint8_t alarm_op_head[] = {0x00,0x2B,0x00,0x00};//len + data

const uint8_t remote_op_head[] = {0x00,0x27,0x00,0x00};//len + data

const uint8_t audit_result_head[] = {0x80,0x32,0x80,0x00,0x00};//len + data

const uint8_t ce_pr11[] = {0xFC,0xA0,0x00,0x00,0x05,0x80,0xCA,0xCE,0x53,0x00,};

const uint8_t serial_pr11[] = {0xFC,0xA0,0x00,0x00,0x05,0x80,0xCA,0xCE,0x0E,0x00,};

const uint8_t ce_322[] = {0x80,0xCA,0xCE,0x42,0x00};
const uint8_t pid_322[] = {0x80,0xCA,0xCE,0x45,0x00};
const uint8_t sn_pr11[] = {0xFC,0xA0,0x00,0x00,0x05,0x80,0xCA,0xCE,0x09,0x00};
const uint8_t pid_pr11[] = {0xFC,0xA0,0x00,0x00,0x05,0x80,0xCA,0xCE,0x50,0x00};
const uint8_t ctrl_cfg[] = {0x80,0xCA,0xCE,0x47,0x00};

const uint8_t change_mode[] = {0xFC,0xDE,0x00,0xAA,0x00};

const uint8_t v_322[] = {0x00,0x26,0x01,0x00,0x00};
const uint8_t v_322_d21[] = {0x00,0x26,0x01,0x81,0x00};
const uint8_t v_pr11[] = {0x00,0x26,0x02,0x00,0x00};
const uint8_t v_pr11_d21[] = {0x00,0x26,0x02,0x81,0x00};

const uint8_t confirm[] = {0x90,0x00};

const uint8_t _end_confirm[] = {0x90,0x00,0x90,0x0A};
const uint8_t _end_alarm[] = {0x90,0x00,0x90,0x0A};

osal_sem_t *sem_pr11ce;

osal_sem_t *sem_322ce;

osal_sem_t *sem_ctrlinfo;


const uint8_t open_door[] = {0x80,0xDD,0x33,0x00,0x03,0x01,0x00,0x02};

const uint8_t test_p2p[] = {0xE0,0xFD,0x00,0x00,0x21,0x08,0xF8,0x19,0x94,0x85,0x3C,0x1D,0xBC,0x72,0xCA,0x6B,0x95,0xA7,0xAE,0xB5,0x6F,0xA8,0x84,0xC9,0x99,0x62,0x50,0x46,0x7F,0x06,0xBD,0x40,0xC4,0xC2,0x24,0x50,0x04,0xE7};
                                            /*0*/   /*1*/   /*2*/   /*3*/   /*4*/
unsigned char* usb_port_def[MAX_322_NUM + 1] = {"1-1.1","1-1.2","1-1.3","1-1.4","1-1.5"};

unsigned char* test_o = (unsigned char*)"\x00\x22\x00\x00\xBE\x01\xF6\xBA\x57\xB5\x61\xDD\x09\xE4\x39\xCF\x52\x4D\xF1\x6F\x2B\xC8\x04\xC0\xEE\xCB\xC4\x3A\x41\x20\x97\x11\x05\x19\x0F\x0F\x0F\x12\x05\x19\x0F\x0F\x0F\x99\xF0\x8B\xA6\x68\x92\xA0\x4C\xC7\x72\x0A\x4D\xC2\x29\x49\x7D\x81\x6C\x1A\x20\x94\x7A\x2A\xA0\xF2\xDE\xCC\x8E\xC1\x2F\x3D\x1D\x2A\x6E\x2B\x9D\xAF\x19\xD6\x8C\x9D\x23\x06\x28\x0B\x30\xB0\xAB\xDB\xE4\x11\x4A\x28\x2E\x2B\x56\x85\xDE\x4B\x0B\x9A\x35\xFF\xCA\xF4\xB7\x31\x9A\x15\xED\xA0\x47\xDC\x66\x4A\x95\x79\xD7\xFB\x8B\x9C\xF3\x50\x10\xFE\x75\xA4\x6B\xDF\x76\x95\x84\x27\xE9\x1D\xFB\x34\xF4\xE8\x04\x32\xF5\xE3\xB3\xBA\x83\xF2\xEF\x5B\x24\x50\x7D\x4F\x95\x76\x95\x73\x72\x71\xD6\xE9\x0A\x7D\xBF\x5F\xFB\xB6\x8E\xA6\xE3\xE9\xE6\xFC\xF5\x1C\x4A\xE9\x30\xDC\x28\xBF\x57\x87\xE6\x80\x00\x00\x00\xC5\x39\x64\x35";


const uint8_t id_reader[] = {0x00,0x01,0x00,0x02,0x01,0x01};

const uint8_t id_info_get[] = {0x00,0x2c,0x00,0x00};

/*
* 函数说明: 写二进制文件
* 参数描述: _fileName, 文件名称
*           _buf, 要写的内存缓冲。
*           _bufLen, 内存缓冲的长度
*   返回值: 0, 成功
*           -1, 失败
*
*/
int writeFile(const char* _fileName, void* _buf, int _bufLen)
{
    FILE * fp = NULL;
    if( NULL == _buf || _bufLen <= 0 ) return (-1);

    fp = fopen(_fileName, "ab+"); // 必须确保是以 二进制写入的形式打开

    if( NULL == fp )
    {
        return (-1);
    }

    fwrite(_buf, _bufLen, 1, fp); //二进制写

    fclose(fp);
    fp = NULL;

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
int readFile(const char* _fileName, void* _buf, int _bufLen)
{
    FILE* fp = NULL;
    if( NULL == _buf || _bufLen <= 0 ) return (-1);

    fp = fopen(_fileName, "rb"); // 必须确保是以 二进制读取的形式打开 

    if( NULL == fp )
    {
        return (-1);
    }

    fread(_buf, _bufLen, 1, fp); // 二进制读

    fclose(fp);
    return 0;        
}

int usb_transmit(void *context, const unsigned char * apdu,
            int apdu_len, unsigned char * resp, int max_resp_size,usb_ccid_322_t  *usb_322)
{

    int ret = 0;

    int error;

	unsigned char output[64] = {0};

    /* Take the semaphore. */
    if(osal_sem_take(usb_322->sem_322, OSAL_WAITING_FOREVER) != OSAL_EOK)
    {
       printf("\n%s Semaphore return failed. \n",usb_322->usb_port);
       return 0;
    }
    
    //print_send(apdu,apdu_len);
    ret = luareader_transmit(context, apdu, apdu_len, resp, max_resp_size,3000);

    osal_sem_release(usb_322->sem_322);

    if(ret < 0){
    
        
        memset(output, 0, sizeof(output));
        error = luareader_pop_value(context, (char *)output, sizeof(output));
        printf("%s-luareader_pop_value(%p)=%d(%s)\n",usb_322->usb_port,context, error, output);
        
 
    }

    return ret;


}



/*alloc index for 322*/
int alloc_322_index(unsigned char * port_name)
{
    unsigned char* p_str;

    int i = 0;

    for(i = 0;i < MAX_322_NUM + 1;i++){

        if(strcmp(usb_port_def[i],port_name) == 0){
            
            return  i;
            
        }

    }

    return -1;
}

void eg_usb_main_proc(char *data,int len)
{
}

/*****************************************************************************
 @funcname: get_sys_time
 @brief   : get systime as acsii
 @param   : 19byte,
// struct tm {
//   int tm_sec;         // 秒，范围从 0 到 59				
//   int tm_min;         // 分，范围从 0 到 59				
//   int tm_hour;        // 小时，范围从 0 到 23				
//   int tm_mday;        // 一月中的第几天，范围从 1 到 31	                
//   int tm_mon;         //月份，范围从 0 到 11				
//   int tm_year;       //  自 1900 起的年数				
//   int tm_wday;       //  一周中的第几天，范围从 0 到 6		        
//   int tm_yday;      //   一年中的第几天，范围从 0 到 365	                
//   int tm_isdst;     //   夏令时							
//}
 @return  : 
*****************************************************************************/
//const uint8_t time_head[] = {0xFC,0xA0,0x00,0x00,0x19,0x80,0x34,0x00,0x00,0x13};
const uint8_t time_head[] = {0xFC,0xA0,0x00,0x00,0x0F,0x80,0x34,0x00,0x00,0x0A};
#if 0
uint8_t   get_sys_time(unsigned char *time_ptr)//len must be more than 19
{
    time_t   now;
    struct   tm  *timenow;
    char time_acs[17];
    short int ascDay;

    if(time_ptr == NULL){

        printf("F[%s] L[%d] ptr is NULL!!!", __FILE__, __LINE__);
        return 0;
    }
    memset(time_acs,0,sizeof(time_acs));
    time(&now);
    timenow   =   localtime(&now);

    sprintf(time_acs,"%04d/%02d/%02d",timenow->tm_year+ 1900,timenow->tm_mon+1,timenow->tm_mday);

    switch(timenow->tm_wday)
			{
				case 0:
					ascDay = 0xC8D5;
					break;
				case 1:
					ascDay = 0xD2BB;
					break;
				case 2:
					ascDay = 0xB6FE;
					break;
				case 3:
					ascDay = 0xC8FD;
					break;
				case 4:
					ascDay = 0xCBC4;
					break;
				case 5:
					ascDay = 0xCEE5;
					break;
				case 6:
					ascDay = 0xC1F9;
					break;
				default:
					ascDay = 0x20202020;
					break;				
			}
    //sprintf(&time_acs[10],"%X",ascDay);
    memcpy(&time_acs[10],&ascDay,sizeof(ascDay));
    sprintf(&time_acs[12],"%02d:%02d",timenow->tm_hour,timenow->tm_min);
    //printf("Local   time   is   %s\n",asctime(timenow));

   // printf("time is %s\n",time_acs);
    memcpy(time_ptr,time_head,sizeof(time_head));
    time_ptr[sizeof(time_head)] = 0x01;
    time_ptr[sizeof(time_head) + 1] = 0x00;
    time_ptr[sizeof(time_head) + 2] = 0x11;//TLV
    memcpy(time_ptr + sizeof(time_head) + 3,time_acs,sizeof(time_acs));
    return 0;
}
#else
uint8_t   get_sys_time(unsigned char *time_ptr)//len must be more than 19
{
    time_t   now;
    struct   tm  *timenow;
    unsigned char time_acs[7];
    short int ascDay;

    if(time_ptr == NULL){

        printf("F[%s] L[%d] ptr is NULL!!!", __FILE__, __LINE__);
        return 0;
    }
    memset(time_acs,0,sizeof(time_acs));
    time(&now);
    timenow   =   localtime(&now);
    
    memcpy(time_ptr,time_head,sizeof(time_head));//head
    
    time_ptr[sizeof(time_head)] = 0x01;//T
    time_ptr[sizeof(time_head) + 1] = 0x00;
    time_ptr[sizeof(time_head) + 2] = 0x07;//L
    //v
    time_ptr[sizeof(time_head) + 3] = ((unsigned char)timenow-> tm_year) > 100 ? (unsigned char)timenow-> tm_year - 100:17;
    time_ptr[sizeof(time_head) + 4] = (unsigned char)timenow-> tm_mon + 1;
    time_ptr[sizeof(time_head) + 5] = (unsigned char)timenow-> tm_mday;
    
    if(timenow-> tm_wday == 0){
        
        time_ptr[sizeof(time_head) + 6] = 7;
        //printf("sunday\n");
    }
    else{
        
        time_ptr[sizeof(time_head) + 6] = (unsigned char)timenow-> tm_wday;
        
        //printf("1-6\n");
    }
    
    time_ptr[sizeof(time_head) + 7] = (unsigned char)timenow-> tm_hour;
    time_ptr[sizeof(time_head) + 8] = (unsigned char)timenow-> tm_min;
    time_ptr[sizeof(time_head) + 9] = (unsigned char)timenow-> tm_sec;
    //memcpy(time_ptr + sizeof(time_head) + 3,time_acs,sizeof(time_acs));//V
    return 0;
}
#endif

void print_rec(unsigned char* rec,int len)
{
#if 1
    int i;

    if(osal_module_debug_level <= OSAL_DEBUG_INFO){
        printf("\nrecv data len is :%d\n\r",len);
        for(i = 0;i < len;i++){
        
            printf("%02X ",rec[i]);
            
        }
        printf("\n\r\n");
    }
#endif

}


void print_send(unsigned char* send,int len)
{
#if 1
    int i;

    if(osal_module_debug_level <= OSAL_DEBUG_INFO){
        
        printf("\nsend data len is :%d\n\r",len);
        for(i = 0;i < len;i++){
        
            printf("%02X ",send[i]);
        }
        printf("\n\r");
        
    }
#endif
}

int check_card(usb_ccid_322_t  *usb_322,unsigned char* rd_data,int buffer_len)
{
    unsigned char *read_buffer;
    int len;
    uint8_t resp_code[] = {0x90,0x00};
    uint8_t resp_code_nocard[] = {0x90,0x01,0x90,0x00};
    uint8_t resp_code_alarm[] = {0x90,0x01,0x90,0x0A};
    uint8_t alarm_code[] = {0x90,0x0A};
    uint8_t wg_error[] = {0x93,0x01};

    //read_buffer = (unsigned char*)malloc(buffer_len);

    //memcpy(read_buffer,rd_data,buffer_len);


    

    if(buffer_len == 5)
    {
        if(memcmp(&rd_data[buffer_len - 4],resp_code_nocard,sizeof(resp_code_nocard)) == 0){
            
            //OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_INFO, "nocard\n");
            //free(read_buffer);
            //在无卡片的时候处理门状态上报 
           usb_322->now_door_state[1] = rd_data[0];
    
           if(usb_322->now_door_state[1] != usb_322->pre_door_state[1]){
    
                usb_322->pre_door_state[1] = usb_322->now_door_state[1];
    
                //ubus_net_process(UBUS_CLIENT_SEND_DOOR_INFO,NULL,usb_322->now_door_state,1);
                sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_SEND_DRSTATE,2,0,usb_322->now_door_state);
                
           } 
            return 0;
        }
        if(memcmp(&rd_data[buffer_len - 4],resp_code_alarm,sizeof(resp_code_alarm)) == 0){
            
            //OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_INFO, "90 0A ALARM!\n");
            //free(read_buffer);
            return 2;
        }

        
    }

    if(buffer_len == 2){
        if(memcmp(rd_data,wg_error,sizeof(wg_error)) == 0){
            
            OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_INFO, "wg error 9301,sleep 1 sec\n");
            sleep(3);
            //free(read_buffer);
            return -3;
        }
    }
    
    if(buffer_len < 5){

        //OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_INFO, "reader return short\n");
        //free(read_buffer);
        return -2;
    }

/*
    if((memcmp(&rd_data[buffer_len - 2],resp_code,2)) ||
        (memcmp(&rd_data[buffer_len - 4],resp_code,2))){
        //OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_WARN, "reader return error\n");
        //sleep(1);
        //free(read_buffer);
        return -1;
    }
*/
    //free(read_buffer);
    return 1;

}

uint8_t *cardtype[] = {"none","15693","sfz","ID cards","cpu","12.5K","MIFARE"};
int parse_data(unsigned char* rd_data,int buffer_len,unsigned char* wl_data,int *wl_len)
{
    unsigned char *read_buffer;
    int len;
    int card_event;
    card_data_t card;
    int i;
    int ret;
    len = buffer_len;
    struct timeval _start,_end;
    //card id 4Byte 20170531
    unsigned char card_id[4] = {0};

    unsigned char audit_buffer[2048] = {0};

    int audit_len = 1024;

    memset(card_id,0,sizeof(card_id));

    memset(audit_buffer,0,sizeof(audit_buffer));

    read_buffer = (unsigned char*)malloc(buffer_len);

    memcpy(read_buffer,rd_data,buffer_len);
    
    card_event = read_buffer[0];

    switch(card_event)
    {
        case CARDPOLLEVENT_NO_EVENT:

            //card type
            if(read_buffer[1] == 0x02){

                printf("find Id card\n");

                return CARDPOLLEVENT_ID;


            }

            
 #if 1           
            if(len < 64){

                memcpy(card_id,&read_buffer[2],4);

                printf("card id is \n");
                print_rec(card_id,4);
                
/*
                gettimeofday( &_start, NULL );
                printf("start : %d.%d\n", _start.tv_sec, _start.tv_usec);
*/
                
                get_wl_4(card_id,wl_data,wl_len);
                
/*
                gettimeofday( &_end, NULL );
                printf("end   : %d.%d\n",_end.tv_sec,_end.tv_usec);
*/
                
                printf("wl len is %d\n",*wl_len);
                print_rec(wl_data,*wl_len);


            }
            #endif
            //ret = CARDPOLLEVENT_NO_EVENT;
            break;

            
        case CARDPOLLEVENT_GET_CNT:
            break;

            
        case CARDPOLLEVENT_GET_LOGNUM:
            
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "log num:\n");
            print_rec(read_buffer,buffer_len);
            

            
            memcpy(card_id,read_buffer,4);//0-07,1-08

            audit_buffer[0] = 0x05;//tag
            audit_buffer[1] = 0x00;
            audit_buffer[2] = 0x02;//len

            ret = get_audit_data(UBUS_CLIENT_AUDIT_LOG,card_id,1,&audit_buffer[3],&audit_len);
            
            if(ret < 0)
                break;
            

            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "log num:%d,%d,%d\n",*((short*)&audit_buffer[3]),\
                audit_buffer[3],audit_buffer[4]);

            
/*
            audit_buffer[3] = HV
            audit_buffer[4] = LV
*/

            audit_buffer[5] = 0x06;//tag

            audit_len = 2048;
            ret = get_audit_data(UBUS_CLIENT_AUDIT_LOG,&card_id[1],3,&audit_buffer[8],&audit_len);
            if(ret < 0)
                break;
            
            audit_buffer[6] = (unsigned char)((audit_len&0xFF00)>>8);
            
            audit_buffer[7] = (unsigned char)(audit_len&0x00FF);

            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "log len:%d,%d,%d\n",audit_len,\
                audit_buffer[6],audit_buffer[7]);

            audit_len =  audit_len + 8;
            
            
            print_rec(audit_buffer,audit_len);

            memcpy(wl_data,audit_buffer,audit_len);
            *wl_len = audit_len;

            break;

            
        case CARDPOLLEVENT_GET_LOG:
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "LOG:\n");
            
            print_rec(read_buffer,buffer_len);           
            
            memcpy(card_id,read_buffer,3);//0-07,1-08

            audit_buffer[0] = 0x06;//tag
            audit_buffer[1] = 0x00;
            audit_buffer[2] = 0x00;//len

            audit_len = 2048;
            ret = get_audit_data(UBUS_CLIENT_AUDIT_LOG,card_id,3,&audit_buffer[3],&audit_len);
            if(ret < 0)
                break;
            
            audit_buffer[1] = (unsigned char)((audit_len&0xFF00)>>8);
            
            audit_buffer[2] = (unsigned char)(audit_len&0x00FF);

            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "log len:%d,%d,%d\n",audit_len,\
                audit_buffer[1],audit_buffer[2]);

            audit_len =  audit_len + 3;
            
            
            print_rec(audit_buffer,audit_len);

            memcpy(wl_data,audit_buffer,audit_len);
            *wl_len = audit_len;

            break;

            
        case CARDPOLLEVENT_BTN:
            break;

            
        case CARDPOLLEVENT_POWER:
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "card power on\n");
            //ret = CARDPOLLEVENT_POWER;
            break;
            
        case CARDPOLLEVENT_WLNUM:
            
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "wl num:\n");
            
            print_rec(read_buffer,buffer_len);
            
            memcpy(card_id,read_buffer,4);//0-07,1-08

            audit_buffer[0] = 0x07;//tag
            audit_buffer[1] = 0x00;
            audit_buffer[2] = 0x02;//len

            ret = get_audit_data(UBUS_CLIENT_AUDIT_WL,card_id,1,&audit_buffer[3],&audit_len);
            
            if(ret < 0)
                break;
            

            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "wl num:%d,%d,%d\n",*((short*)&audit_buffer[3]),\
                audit_buffer[3],audit_buffer[4]);

            
/*
            audit_buffer[3] = HV
            audit_buffer[4] = LV
*/

            audit_buffer[5] = 0x08;//tag

            audit_len = 2048;
            ret = get_audit_data(UBUS_CLIENT_AUDIT_WL,&card_id[1],3,&audit_buffer[8],&audit_len);
            if(ret < 0)
                break;
            
            audit_buffer[6] = (unsigned char)((audit_len&0xFF00)>>8);
            
            audit_buffer[7] = (unsigned char)(audit_len&0x00FF);

            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "wl len:%d,%d,%d\n",audit_len,\
                audit_buffer[6],audit_buffer[7]);

            audit_len =  audit_len + 8;
            
            
            print_rec(audit_buffer,audit_len);

            memcpy(wl_data,audit_buffer,audit_len);
            *wl_len = audit_len;

            break;
            
        case CARDPOLLEVENT_WL:
            
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "WL:\n");
            
            print_rec(read_buffer,buffer_len);           
            
            memcpy(card_id,read_buffer,3);//0-07,1-08

            audit_buffer[0] = 0x08;//tag
            audit_buffer[1] = 0x00;
            audit_buffer[2] = 0x00;//len

            audit_len = 2048;
            ret = get_audit_data(UBUS_CLIENT_AUDIT_WL,card_id,3,&audit_buffer[3],&audit_len);
            if(ret < 0)
                break;
            
            audit_buffer[1] = (unsigned char)((audit_len&0xFF00)>>8);
            
            audit_buffer[2] = (unsigned char)(audit_len&0x00FF);

            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "WL len:%d,%d,%d\n",audit_len,\
                audit_buffer[1],audit_buffer[2]);

            audit_len =  audit_len + 3;
            
            
            print_rec(audit_buffer,audit_len);

            memcpy(wl_data,audit_buffer,audit_len);
            *wl_len = audit_len;

            
            break;            

        default:
            //OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_WARN, "error data,%d\n",card_event);
            break;

    }


end:
    free(read_buffer);

    return card_event;

}


unsigned char lu_test[] = {0x00,0x84,0x00,0x00,0x08};
volatile  int cnt = 0;
volatile  uint8_t sw_version = 0;

/*****************************************************************************
 @funcname: eg_usb_thread_entry
 @brief   : thread function
 @param   : void *parameter  
 @return  : void
*****************************************************************************/
void *eg_usb_thread_entry(void *parameter)
{
    int ret = 0;
    int i = 0;
    unsigned char parse_tag = 0;
    unsigned char acl_data[2048] = {0};
    unsigned char acl_cfg[256] = {0};
	unsigned char output[2048] = {0};
    unsigned char usb_port[32] = {0};
    int acl_len = 1024;
    unsigned char send_data[1024] = {0};
    unsigned char recv_data[1024] = {0};

    unsigned char apud_data[1024] = {0};
    int apud_len = 0;

    unsigned char log_data[2048] = {0};
    int log_len = 0;

    int remote_len = 0;
    int audit_len = 0;
    int tail_check = 0;
    unsigned char rtc[16] = {0};
    unsigned char ctrl_info[25] = {0};

    usb_ccid_322_t *p_usb_ccid;

    int rec_zmq = 0;

    uint8_t zmq_ans[256] = {0};
    

    /**test**/
    
    struct timeval _start,_end;
    /**test**/

    p_usb_ccid = (usb_ccid_322_t*)parameter;
    
	void * context = luareader_new(0, NULL, NULL);

    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "context is %p\n",context);

    //strcpy(usb_port,(const char*)parameter);
    ret = luareader_connect(context, p_usb_ccid->usb_port);
    if(ret < 0){
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "connect to  %s failed\n",p_usb_ccid->usb_port);
        goto out;
    }
    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "connect to  %s succeed\n",p_usb_ccid->usb_port);


    sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_INITED,0,p_usb_ccid->ccid322_index,NULL);

while(1){

#if 1
    memset(output,0,sizeof(output));
    memset(apud_data,0,sizeof(apud_data));
    apud_len = 1024;
    acl_len = 1024;
    log_len = 2048;
    remote_len = 0;
    //printf("state is %d\n",p_usb_ccid->usb_state);
    if(p_usb_ccid->toggle_state == 0xAA){
    
        p_usb_ccid->toggle_state = 0;
        
        switch (p_usb_ccid->usb_state) {

            
        case USB_COMM_STATE_P2P:

                     
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "send p2pkey\n");

                //printf("controll_eg.p2pkey.len is %d\n",controll_eg.p2pkey.len);

                controll_eg.p2pkey.len = controll_eg.p2pkey.len > 64 ? 38:controll_eg.p2pkey.len;

                print_send(controll_eg.p2pkey.data,controll_eg.p2pkey.len);
                ret = usb_transmit(context,controll_eg.p2pkey.data,controll_eg.p2pkey.len,output,sizeof(output),p_usb_ccid);
                print_rec(output,ret);

                
                sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_SEND_COSVERSION,0,0,NULL);
                osal_sem_release(p_usb_ccid->sem_state);
                //p_usb_ccid->usb_state = USB_COMM_STATE_MACKEY;//USB_COMM_STATE_RDCFG;//USB_COMM_STATE_MACKEY;//USB_COMM_STATE_CFG;//USB_COMM_STATE_MACKEY;
                
            break;
            
        case USB_COMM_STATE_MACKEY:

                
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "send MACKEY\n");
                
                
                controll_eg.p2pkey.len = controll_eg.mackey.len > 64 ? 37:controll_eg.mackey.len;
                print_send(controll_eg.mackey.data,controll_eg.mackey.len);
                ret = usb_transmit(context,controll_eg.mackey.data,controll_eg.mackey.len,output,sizeof(output),p_usb_ccid);
                print_rec(output,ret);

                
                osal_sem_release(p_usb_ccid->sem_state);
                //p_usb_ccid->usb_state = USB_COMM_STATE_RDCFG;//USB_COMM_STATE_CFG;//USB_COMM_STATE_INIT_END;//USB_COMM_STATE_CFG;//USB_COMM_STATE_VERSION;//USB_COMM_STATE_P2P;

            break;

        case USB_COMM_STATE_CFG:

                
                memset(apud_data,0,sizeof(apud_data));
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "send base cfg\n");
                
                memcpy(apud_data,basecfg_head,sizeof(basecfg_head));
                apud_data[sizeof(basecfg_head)] = controll_eg.basecfg.len;//(unsigned char)(0x00FF&controll_eg.mackey.len);
                memcpy(&apud_data[sizeof(basecfg_head) + 1],&controll_eg.basecfg.data[0],controll_eg.basecfg.len);  
                
    /*
                printf("cfg len is %d\n",controll_eg.basecfg.len);
                printf("\n===========cfg==============\n");
                for(i = 0; i< controll_eg.basecfg.len;i++){
                    printf("%02x ",controll_eg.basecfg.data[i]);
                }
                printf("\n===========cfg==============\n");
    */
                print_send(apud_data,sizeof(basecfg_head) + 1 + controll_eg.basecfg.len);
                ret = usb_transmit(context,apud_data,sizeof(basecfg_head) + 1 + controll_eg.basecfg.len,output,sizeof(output),p_usb_ccid);
                
                //ret = usb_transmit(context,&controll_eg.basecfg.data,controll_eg.basecfg.len,output,sizeof(output),p_usb_ccid);
                print_rec(output,ret);
                
                osal_sem_release(p_usb_ccid->sem_state);
                //p_usb_ccid->usb_state = USB_COMM_STATE_RDCFG;

            break;
            
        case USB_COMM_STATE_RDCFG:

                
                memset(apud_data,0,sizeof(apud_data));
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "send ctrl cfg\n");
                
                memcpy(apud_data,ctlcfg_head,sizeof(ctlcfg_head));
                apud_data[sizeof(ctlcfg_head)] = controll_eg.ctlcfg.len;//(unsigned char)(0x00FF&controll_eg.mackey.len);
                memcpy(&apud_data[sizeof(ctlcfg_head) + 1],&controll_eg.ctlcfg.data[0],controll_eg.ctlcfg.len); 


                
                print_send(apud_data,sizeof(ctlcfg_head) + 1 + controll_eg.ctlcfg.len);
                ret = usb_transmit(context,apud_data,sizeof(ctlcfg_head) + 1 + controll_eg.ctlcfg.len,output,sizeof(output),p_usb_ccid);
               // print_rec(output,ret);
                //ret = usb_transmit(context,&controll_eg.ctlcfg.data,controll_eg.ctlcfg.len,output,sizeof(output),p_usb_ccid);
                print_rec(output,ret);

                
                osal_sem_release(p_usb_ccid->sem_state);
                //p_usb_ccid->usb_state = USB_COMM_STATE_VERSION;
                

            break;
            

        case USB_COMM_STATE_VERSION:

            if(sw_version == 0)
                {
                
                sw_version = 0xAA;
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "get 322 version:");
                ret = usb_transmit(context,v_322,sizeof(v_322),&output[1],sizeof(output) - 1,p_usb_ccid);
                output[0] = 0x01;
                print_rec(output,ret + 1);
                if(ret > 0)
                    ubus_client_process(UBUS_CLIENT_SENDVERSION,NULL,output,ret - 1);

                
                memset(output,0,sizeof(output));
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "get 322-d21 version:");
                ret = usb_transmit(context,v_322_d21,sizeof(v_322_d21),&output[1],sizeof(output) - 1,p_usb_ccid);      
                output[0] = 0x02;
                print_rec(output,ret + 1);
                if(ret > 0)
                    ubus_client_process(UBUS_CLIENT_SENDVERSION,NULL,output,ret - 1);
                
                memset(output,0,sizeof(output));
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "get pr11 version:");
                ret = usb_transmit(context,v_pr11,sizeof(v_pr11),&output[1],sizeof(output) - 1,p_usb_ccid);
                output[0] = 0x03;
                print_rec(output,ret + 1);
                if(ret > 0)
                    ubus_client_process(UBUS_CLIENT_SENDVERSION,NULL,output,ret - 1);
                
                memset(output,0,sizeof(output));
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "get pr11-d21 version:");
                ret = usb_transmit(context,v_pr11_d21,sizeof(v_pr11_d21),&output[1],sizeof(output) - 1,p_usb_ccid);
                output[0] = 0x04;
                print_rec(output,ret + 1);
                if(ret > 0)
                    ubus_client_process(UBUS_CLIENT_SENDVERSION,NULL,output,ret - 1);

                //printf("================find 321 id===============\n");
                //ubus_321_find();

            }
            
/*
            printf("================push time===============\n");
            sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_INFO_PUSH,0,p_usb_ccid->ccid322_index,NULL);
*/
            
            osal_sem_release(p_usb_ccid->sem_state);
            //p_usb_ccid->usb_state = USB_COMM_STATE_INIT_END;//;
            break;
            
        case USB_COMM_STATE_INIT:
            
            //printf("F[%s] L[%d] usbstate is %d!!!\n", __FILE__, __LINE__,p_usb_ccid->usb_state);
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "init begin\n");

/*
            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "%s change mode\n",p_usb_ccid->usb_port);
            usb_transmit(context,check,sizeof(check),output,sizeof(output),p_usb_ccid);        
            print_rec(output,ret);
*/

            /*****************************************read 322ce*************************************************/
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "reading 322 ce\n");
            //read 322 ce
            ret = usb_transmit(context,ce_322,sizeof(ce_322),&output[3],sizeof(output) - 3,p_usb_ccid);
            printf("read over\n");
            if(ret <= 0){
            
                
                memset(output, 0, sizeof(output));
                ret = luareader_pop_value(context, (char *)output, sizeof(output));
                printf("luareader_pop_value(%p)=%d(%s)\n", context, ret, output);
                
                log_message(p_usb_ccid->usb_port,3,"322 ce read error\n");
                //osal_sem_release(p_usb_ccid->sem_state);
                //break;
                
            
            }else{
            
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "322 ce,len is %d\n",ret - 2);//minus 90 00
                //print_rec(&output[3],ret);
                output[0] = p_usb_ccid->ccid322_index;
                output[1] = (unsigned char)(((ret - 2)&0xFF00)>>8);
                output[2] = (unsigned char)(((ret - 2)&0x00FF));
                
                /* Take the semaphore. */
                if(osal_sem_take(sem_322ce, OSAL_WAITING_FOREVER) != OSAL_EOK){
                    
                    printf("Semaphore return failed. \n");
                    
                    //osal_sem_release(p_usb_ccid->sem_state);
                    //break;
                }else{
                    writeFile(CEPATH_322,output,ret - 2 + 3);
                    osal_sem_release(sem_322ce);
                    print_rec(output,ret + 3);
                }
            }
        /**********************************************pr11 ce**********************************************************/
        //read pr11 ce
        #if 0
        ret = usb_transmit(context,ce_pr11,sizeof(ce_pr11),&output[3],sizeof(output) - 3,p_usb_ccid);
        
        if(ret <= 0){
        
            
            memset(output, 0, sizeof(output));
            ret = luareader_pop_value(context, (char *)output, sizeof(output));
            printf("luareader_pop_value(%p)=%d(%s)\n", context, ret, output);           
            log_message(p_usb_ccid->usb_port,3,"pr11 ce read error\n");
            //osal_sem_release(p_usb_ccid->sem_state);
            //break;
            
        
        }
        else{
            
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "pr11 ce,len is %d\n",ret - 4);//minus 90 00
            //print_rec(&output[3],ret);
            output[0] = p_usb_ccid->ccid322_index + 1;
            output[1] = (unsigned char)(((ret - 4)&0xFF00)>>8);
            output[2] = (unsigned char)(((ret - 4)&0x00FF));
            
            /* Take the semaphore. */
            if(osal_sem_take(sem_pr11ce, OSAL_WAITING_FOREVER) != OSAL_EOK){
                
                printf("Semaphore return failed. \n");            
                //osal_sem_release(p_usb_ccid->sem_state);
                //break;
            }
            else{
                writeFile(CEPATH_PR11,output,ret - 4 + 3);
                osal_sem_release(sem_pr11ce);
                print_rec(output,ret + 3);
            }
        }
        #endif
        
        /*********read ctrl cfg*********/
        OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "reading ctrl cfg\n");
        memset(output, 0, sizeof(output));
        ret = usb_transmit(context,ctrl_cfg,sizeof(ctrl_cfg),output,sizeof(output),p_usb_ccid);
        
        if(ret <= 0){
        
            
            memset(output, 0, sizeof(output));
            ret = luareader_pop_value(context, (char *)output, sizeof(output));
            printf("luareader_pop_value(%p)=%d(%s)\n", context, ret, output);
            
            log_message(p_usb_ccid->usb_port,3,"ctrl cfg read error\n");
        
        }else{
        
            if(memcmp(&output[ret - 2],confirm,sizeof(confirm)) == 0){
                    OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "ctrl_cfg,len is %d,door index is %d\n",ret - 2,output[ret - 4]);//minus 90 00
                    //memcpy(p_usb_ccid->pid_322,output,4);//322 id len si 4
                    print_rec(output,ret);

                    p_usb_ccid->door_index = output[8];////door no get
                    p_usb_ccid->pre_door_state[0] = p_usb_ccid->door_index;
                    p_usb_ccid->now_door_state[0] = p_usb_ccid->door_index;
                }
            else{
                
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "ctrl_cfg,return error\n");//
                print_rec(output,ret);
                
            }
        }
        
        /*********read ctrl cfg end*********/

        /*********controller info*********/
        
        //OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "reading 322 id\n");
        memset(output, 0, sizeof(output));
        ret = usb_transmit(context,pid_322,sizeof(pid_322),output,sizeof(output),p_usb_ccid);
        
        if(ret <= 0){
        
            
            memset(output, 0, sizeof(output));
            ret = luareader_pop_value(context, (char *)output, sizeof(output));
            printf("luareader_pop_value(%p)=%d(%s)\n", context, ret, output);
            
            log_message(p_usb_ccid->usb_port,3,"322 pid read error\n");
            //osal_sem_release(p_usb_ccid->sem_state);
            //break;
            
        
        }else{
        
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "322 pid,len is %d\n",ret - 2);//minus 90 00
            memcpy(p_usb_ccid->pid_322,output,4);//322 id len si 4
            print_rec(output,ret);
        }

        
        OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "reading pr11 sn\n");
        memset(output, 0, sizeof(output));
        ret = usb_transmit(context,sn_pr11,sizeof(sn_pr11),output,sizeof(output),p_usb_ccid);
        
        if(ret <= 0){
        
            
            memset(output, 0, sizeof(output));
            ret = luareader_pop_value(context, (char *)output, sizeof(output));
            printf("luareader_pop_value(%p)=%d(%s)\n", context, ret, output);
            
            log_message(p_usb_ccid->usb_port,3,"sn pr11 read error\n");
            //osal_sem_release(p_usb_ccid->sem_state);
            //break;
            
            goto out;
            
        
        }else{

            if((memcmp(&output[ret - 4],_end_confirm,sizeof(_end_confirm)) == 0)||(memcmp(&output[ret - 4],_end_alarm,sizeof(_end_alarm)) == 0)){

                    OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "pr11 sn,len is %d\n",ret - 2);//minus 90 00
                    memcpy(p_usb_ccid->sn_pr11,output,16);//pr11 sn len is 12
                    print_rec(output,ret);
                }
            else{

                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "read pr11 sn error,thread exit\n");//minus 90 00
                goto out;
            }
        }

        
        memset(output, 0, sizeof(output));
        ret = usb_transmit(context,pid_pr11,sizeof(pid_pr11),output,sizeof(output),p_usb_ccid);
        
        if(ret <= 0){
        
            
            memset(output, 0, sizeof(output));
            ret = luareader_pop_value(context, (char *)output, sizeof(output));
            printf("luareader_pop_value(%p)=%d(%s)\n", context, ret, output);
            
            log_message(p_usb_ccid->usb_port,3,"pr11 pid read error\n");
            //osal_sem_release(p_usb_ccid->sem_state);
            //break;
            
            goto out;
            
        
        }else{

            if((memcmp(&output[ret - 4],_end_confirm,sizeof(_end_confirm)) == 0)||(memcmp(&output[ret - 4],_end_alarm,sizeof(_end_alarm)) == 0)){

                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "pr11 pid,len is %d\n",ret - 2);//minus 90 00
                memcpy(p_usb_ccid->pid_pr11,output,4);//pr11 id len si 4
                print_rec(output,ret);
                
            }
            else{
                
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "read pr11 pid error,thread exit\n");//minus 90 00
                goto out;
            }
        }
        //write to file
        
        /* Take the semaphore. */
        if(osal_sem_take(sem_ctrlinfo, OSAL_WAITING_FOREVER) != OSAL_EOK){
            
            printf("Semaphore return failed. \n");
            
            //osal_sem_release(p_usb_ccid->sem_state);
            //break;
        }else{

            ctrl_info[0] = p_usb_ccid->ccid322_index;
            memcpy(&ctrl_info[1],p_usb_ccid->pid_322,4);
            memcpy(&ctrl_info[5],p_usb_ccid->sn_pr11,16);
            memcpy(&ctrl_info[21],p_usb_ccid->pid_pr11,4);
            print_send(ctrl_info,25);
            writeFile(CEPATH_CTRLINFO,ctrl_info,25);
            osal_sem_release(sem_ctrlinfo);
        }
            
        /*********controller info*********/
        
        /************************************************finish************************************************************/
            p_usb_ccid->init_flag |= INIT_MASK_CE;
            //update_ce();       
            sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_SEND_CE,0,0,NULL);
            //sleep(1);
            //sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_SEND_CTRLINFO,0,0,NULL);
            osal_sem_release(p_usb_ccid->sem_state);
            osal_timer_start(p_usb_ccid->timer_322);//begin to poll,init finished  
            //sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_UPDATE_BASECFG,0,0,NULL);
            //p_usb_ccid->usb_state = USB_COMM_STATE_P2P;//USB_COMM_STATE_CFG;//USB_COMM_STATE_P2P;//USB_COMM_STATE_CFG;//USB_COMM_STATE_P2P;
            break;

        case USB_COMM_CTRL_INFO:
            
            break;
        case USB_COMM_STATE_INIT_END:

            msleep(100);
           // p_usb_ccid->usb_state = USB_COMM_STATE_POLL;
            //osal_timer_start(p_usb_ccid->timer_322);//begin to poll,init finished
            printf("poll process START!!!\n");
            break;
                     

        case USB_COMM_STATE_PUSH:
            

            get_sys_time(send_data);
            //x_TransmitApdu_hid_hs(&USB_HID_1,send_data,sizeof(time_head)+20,recv_data,&rec_len,timeout);
           // ret = luareader_transmit(context, send_data, sizeof(time_head)+20, output, sizeof(output));
           
            //print_send(send_data,sizeof(time_head)+10);
            ret = usb_transmit(context,send_data,sizeof(time_head)+10,output,sizeof(output),p_usb_ccid);
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_TRACE, "push time\n");
            //print_rec(output,ret);
           
            
            osal_sem_release(p_usb_ccid->sem_state);
           // p_usb_ccid->usb_state = USB_COMM_STATE_IDLE;
            break;
       case USB_COMM_ALARM_OPEN:
        
           OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "alarm open\n");
           
           memcpy(apud_data,alarm_op_head,sizeof(alarm_op_head));
           apud_data[sizeof(alarm_op_head)] =  controll_eg.alarm_buffer[0];//
           memcpy(&apud_data[sizeof(alarm_op_head) + 1],&controll_eg.alarm_buffer[1],controll_eg.alarm_buffer[0]); 

           print_send(apud_data,sizeof(alarm_op_head) + 1 + controll_eg.alarm_buffer[0]);
           ret = usb_transmit(context,apud_data,sizeof(alarm_op_head) + 1 + controll_eg.alarm_buffer[0],output,sizeof(output),p_usb_ccid);
           print_rec(output,ret - 2);

           ubus_net_process(UBUS_CLIENT_SEND_ALARM,NULL,output,ret - 2);
           osal_sem_release(p_usb_ccid->sem_state);
            break;
        case USB_COMM_REMOTE_OPEN:
            
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "remote open\n");
            
            memcpy(apud_data,remote_op_head,sizeof(remote_op_head));
            apud_data[sizeof(remote_op_head)] =  0x00;//extend data
            
            remote_len = ((controll_eg.remote_buffer[0]<<8)|(controll_eg.remote_buffer[1]&0x00FF));
            memcpy(&apud_data[sizeof(remote_op_head) + 1],&controll_eg.remote_buffer,remote_len + 2); 
            
            print_send(apud_data,sizeof(remote_op_head) + 1 + 2 + remote_len);
            ret = usb_transmit(context,apud_data,sizeof(remote_op_head) + 1 + 2 + remote_len,output,sizeof(output),p_usb_ccid);
            print_rec(output,ret);
            if(ret > 2){
                
                log_len = ret - 2;
                memcpy(log_data,output,log_len);
                ubus_client_process(UBUS_CLIENT_LOG,NULL,log_data,log_len);
            }

            osal_sem_release(p_usb_ccid->sem_state);

            break;
            
        case  USB_COMM_ID_READ:
            
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "Id transmit\n");
            ret = usb_transmit(context,p_usb_ccid->zmq_buffer,p_usb_ccid->zmq_len,output,sizeof(output),p_usb_ccid);
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "322 Id return:\n");
            print_rec(output,ret);
            zmq_socket_send(p_usb_ccid->zmq_server,output,ret);
            osal_sem_release(p_usb_ccid->sem_state);
            break;
            
        case  USB_COMM_ID_DOOR_SERVER:
            
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "Id info to server\n");
            ret = usb_transmit(context,id_info_get,sizeof(id_info_get),output,sizeof(output),p_usb_ccid);
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "322 Id info return:\n");
            print_rec(output,ret);
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "make id data:\n");
            apud_data[0] = p_usb_ccid->door_index;
            
            osal_sem_release(p_usb_ccid->sem_state);
            break;

        default:
            break;
        }
        continue;
        }
    /***************POLL***************/
            
          //OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "poll start\n");
            //log_message(p_usb_ccid->usb_port,3,"poll start\n");
          ret = usb_transmit(context,car_detect,sizeof(car_detect),output,sizeof(output),p_usb_ccid);
          //OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "poll end\n");
          //print_rec(output,ret);
         //log_message(p_usb_ccid->usb_port,3,"poll end\n");
/*
          if(ret < 0){
          
              
              memset(output, 0, sizeof(output));
              ret = luareader_pop_value(context, (char *)output, sizeof(output));
              printf("luareader_pop_value(%p)=%d(%s)\n", context, ret, output);
              
          
          }
*/
          
          tail_check = check_card(p_usb_ccid,output,ret);
          
if(tail_check == 1){

    parse_tag = parse_data(output,ret,acl_data,&acl_len);

    switch(parse_tag){

        
        case CARDPOLLEVENT_NO_EVENT:
        
            if(acl_len == 17){

                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_WARN, "not passed\n");
                
                memcpy(apud_data,result_head,sizeof(result_head));

                apud_data[sizeof(result_head)] = 17;

                memcpy(&apud_data[sizeof(result_head) + 1],acl_data,acl_len);

                //print_send(apud_data,sizeof(result_head) + 1 + 17);
                
                ret = usb_transmit(context,apud_data,sizeof(result_head) + 1 + 17,output,sizeof(output),p_usb_ccid);
            }
            else{

                memcpy(apud_data,result_head,sizeof(result_head));
                
                apud_data[sizeof(result_head)] = 17 + 1 + acl_data[207] + 168;//len

                //printf("data len is %d\n",apud_data[sizeof(result_head)]);
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "data len is %d\n",apud_data[sizeof(result_head)]);

                memcpy(&apud_data[sizeof(result_head) + 1],acl_data,17);//result:1 + RTC:16

                memcpy(&apud_data[sizeof(result_head) + 1 + 17],&acl_data[207],1);//name-L:1
                
                memcpy(&apud_data[sizeof(result_head) + 1 + 17 + 1],&acl_data[208],acl_data[207]);//name-V:v

                memcpy(&apud_data[sizeof(result_head) + 1 + 17 + 1 + acl_data[207]],&acl_data[17],168);

                print_send(apud_data,sizeof(result_head) + 1 + 17 + 1 + acl_data[207] + 168);
                
                //ret = usb_transmit(context,open_door,sizeof(open_door),output,sizeof(output),p_usb_ccid);
                
                //print_send(test_o,195);
                //ret = usb_transmit(context,test_o,195,output,sizeof(output),p_usb_ccid);
                ret = usb_transmit(context,apud_data,sizeof(result_head) + 1 + 17 + 1 + acl_data[207] + 168,output,sizeof(output),p_usb_ccid);

            }           

            printf("log return\n");
            print_rec(output,ret);
            if(ret > 2){
                
                log_len = ret - 2;
                memcpy(log_data,output,log_len);
                
/*
                gettimeofday( &_start, NULL );
                printf("start : %d.%d\n", _start.tv_sec, _start.tv_usec);
*/
                
                ubus_client_process(UBUS_CLIENT_LOG,NULL,log_data,log_len);
                
/*
                gettimeofday( &_end, NULL );
                printf("start : %d.%d\n", _end.tv_sec, _end.tv_usec);
*/
                
            }

            break;
            
        case CARDPOLLEVENT_WLNUM:

           /***********************transfer cmd**************************/
           memcpy(apud_data,transfer_head,sizeof(transfer_head));
           
           apud_data[sizeof(transfer_head)] = 0;//extend length

           apud_data[sizeof(transfer_head) + 1] = 0;//extend length char 1

           apud_data[sizeof(transfer_head) + 2] = 0;//extend length char 2

           /************************return data*************************/
           
           memcpy(&apud_data[sizeof(transfer_head) + 3],audit_result_head,sizeof(audit_result_head));//+lc

           apud_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head)] = (unsigned char)((acl_len&0xFF00)>>8); 
           apud_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head) + 1] = (unsigned char)(acl_len&0x00FF); 

           memcpy(&apud_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head) + 2],acl_data,acl_len);

           audit_len = sizeof(audit_result_head) + 2 + acl_len;

           apud_data[sizeof(transfer_head) + 1] = (unsigned char)((audit_len&0xFF00)>>8);
           apud_data[sizeof(transfer_head) + 2] = (unsigned char)(audit_len&0x00FF);//extend length char 2
           
           print_send(apud_data,sizeof(transfer_head) + 3 + audit_len);

           ret = usb_transmit(context,apud_data,sizeof(transfer_head) + 3 + audit_len,output,sizeof(output),p_usb_ccid);
           print_rec(output,ret);
            
            break;

        case CARDPOLLEVENT_WL:
            
            /***********************transfer cmd**************************/
            memcpy(apud_data,transfer_head,sizeof(transfer_head));
            
            apud_data[sizeof(transfer_head)] = 0;//extend length
            
            apud_data[sizeof(transfer_head) + 1] = 0;//extend length char 1
            
            apud_data[sizeof(transfer_head) + 2] = 0;//extend length char 2
            
            /************************return data*************************/
            
            memcpy(&apud_data[sizeof(transfer_head) + 3],audit_result_head,sizeof(audit_result_head));//+lc
            
            apud_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head)] = (unsigned char)((acl_len&0xFF00)>>8); 
            apud_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head) + 1] = (unsigned char)(acl_len&0x00FF); 
            
            memcpy(&apud_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head) + 2],acl_data,acl_len);
            
            audit_len = sizeof(audit_result_head) + 2 + acl_len;
            
            apud_data[sizeof(transfer_head) + 1] = (unsigned char)((audit_len&0xFF00)>>8);
            apud_data[sizeof(transfer_head) + 2] = (unsigned char)(audit_len&0x00FF);//extend length char 2
            
            print_send(apud_data,sizeof(transfer_head) + 3 + audit_len);
            
            ret = usb_transmit(context,apud_data,sizeof(transfer_head) + 3 + audit_len,output,sizeof(output),p_usb_ccid);
            print_rec(output,ret);
            
            break;

        case CARDPOLLEVENT_GET_LOGNUM:
            
            /***********************transfer cmd**************************/
            memcpy(apud_data,transfer_head,sizeof(transfer_head));
            
            apud_data[sizeof(transfer_head)] = 0;//extend length
            
            apud_data[sizeof(transfer_head) + 1] = 0;//extend length char 1
            
            apud_data[sizeof(transfer_head) + 2] = 0;//extend length char 2
            
            /************************return data*************************/
            
            memcpy(&apud_data[sizeof(transfer_head) + 3],audit_result_head,sizeof(audit_result_head));//+lc
            
            apud_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head)] = (unsigned char)((acl_len&0xFF00)>>8); 
            apud_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head) + 1] = (unsigned char)(acl_len&0x00FF); 
            
            memcpy(&apud_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head) + 2],acl_data,acl_len);
            
            audit_len = sizeof(audit_result_head) + 2 + acl_len;
            
            apud_data[sizeof(transfer_head) + 1] = (unsigned char)((audit_len&0xFF00)>>8);
            apud_data[sizeof(transfer_head) + 2] = (unsigned char)(audit_len&0x00FF);//extend length char 2
            
            print_send(apud_data,sizeof(transfer_head) + 3 + audit_len);
            
            ret = usb_transmit(context,apud_data,sizeof(transfer_head) + 3 + audit_len,output,sizeof(output),p_usb_ccid);
            print_rec(output,ret);
        

            
            break;

        case CARDPOLLEVENT_GET_LOG:
            /***********************transfer cmd**************************/
            memcpy(apud_data,transfer_head,sizeof(transfer_head));
            
            apud_data[sizeof(transfer_head)] = 0;//extend length
            
            apud_data[sizeof(transfer_head) + 1] = 0;//extend length char 1
            
            apud_data[sizeof(transfer_head) + 2] = 0;//extend length char 2
            
            /************************return data*************************/
            
            memcpy(&apud_data[sizeof(transfer_head) + 3],audit_result_head,sizeof(audit_result_head));//+lc
            
            apud_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head)] = (unsigned char)((acl_len&0xFF00)>>8); 
            apud_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head) + 1] = (unsigned char)(acl_len&0x00FF); 
            
            memcpy(&apud_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head) + 2],acl_data,acl_len);
            
            audit_len = sizeof(audit_result_head) + 2 + acl_len;
            
            apud_data[sizeof(transfer_head) + 1] = (unsigned char)((audit_len&0xFF00)>>8);
            apud_data[sizeof(transfer_head) + 2] = (unsigned char)(audit_len&0x00FF);//extend length char 2
            
            print_send(apud_data,sizeof(transfer_head) + 3 + audit_len);
            
            ret = usb_transmit(context,apud_data,sizeof(transfer_head) + 3 + audit_len,output,sizeof(output),p_usb_ccid);
            print_rec(output,ret);
            break;

       case CARDPOLLEVENT_ID:
            zmq_socket_send(p_usb_ccid->zmq_client,id_reader,sizeof(id_reader));
            break;
            
        default:
            break;    

    }

}
else if(tail_check == 2){

    
/*
    get_rtc_data(rtc);
    printf("rtc data is \n");
    print_rec(rtc,16);
    OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "\nget 322 alarm,send rtc encrypt:\n");


    
    memcpy(apud_data,alarm322_head,sizeof(alarm322_head));
    memcpy(&apud_data[sizeof(alarm322_head)],rtc,sizeof(rtc));
    print_send(apud_data,sizeof(alarm322_head) + sizeof(rtc));
    ret = usb_transmit(context,apud_data,sizeof(alarm322_head) + sizeof(rtc),output,sizeof(output),p_usb_ccid);

    
    OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "\nget 322 alarm log:\n");
    
    print_rec(output,ret);

    
    ubus_net_process(UBUS_CLIENT_SEND_ALARM,NULL,output,ret - 2);
*/
    

}

          /***************POLL  END***************/
          
          //printf("F[%s] L[%d] ptr is NULL!!!\n", __FILE__, __LINE__);

        rec_zmq = zmq_recv(p_usb_ccid->zmq_answer,zmq_ans,sizeof(zmq_ans),ZMQ_DONTWAIT);

        if(rec_zmq <= 0){

            
            //printf("no answer\n");


        }
        else{

            printf("get zmq\n");

            print_rec(zmq_ans,rec_zmq);
        }

        msleep(30);
#else
sleep(2);
#endif
}




out: 
        luareader_disconnect(context);
       
        luareader_term(context);

        
        p_usb_ccid->ccid322_exist = 0;

        controll_eg.cnt_322--;

        OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "thread exit!\n");
        //return NULL;


}



uint8_t usb_wb;

void timer_usb_callback(void* parameter)
{
    unsigned char rtc[16];
    usb_ccid_322_t *p_usb_timer = (usb_ccid_322_t *)parameter;
    
    //printf("timer in port %s,index is %d\n",p_usb_timer->usb_port,p_usb_timer->ccid322_index);
    
   // if(p_usb_timer->toggle_ubus == 0xAA){

        
        if(p_usb_timer->toggle >= 10){

            p_usb_timer->toggle = 0;
            //p_usb_timer->usb_state = USB_COMM_STATE_PUSH;
            //printf("push index %d\n",p_usb_timer->ccid322_index);
            sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_INFO_PUSH,0,p_usb_timer->ccid322_index,NULL);

/*
                get_rtc_data(rtc);

                print_rec(rtc,16);
*/

        }
        else{
            
            p_usb_timer->toggle ++;
            

        }
   // }

    
}


void eg_usbto322_init()
{

    osal_task_t *tid;

    osal_timer_t *timer_usb;

	
	unsigned char output[1024] = {0};
    unsigned char device_str[MAX_322_NUM][32] = {0};
	int ret;
	int i = 0,j = 0,dev_index = 0;

    usb_ccid_322_t *p_usb_ccid;

    
	void * context = luareader_new(0, NULL, NULL);


    ret = luareader_get_list(context, (char *)output, sizeof(output));

    //OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "there are %d 322 on pc02\n",ret);

    
    for(i = 0;i < ret - 1;i++){

        if(dev_index >= MAX_322_NUM + 1){

            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "number of device is over %d\n",MAX_322_NUM);
            break;
        }

        
        if((output[i] == 0)&&(dev_index <= MAX_322_NUM + 1)){
            
            memcpy(device_str[dev_index++],&output[j],i - j + 1);
            j = i + 1;
        }


    }
    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "there is  %d devices on pc02\n",dev_index);

    /*max device = 4*/
    dev_index = (dev_index < (MAX_322_NUM + 1) ? dev_index : (MAX_322_NUM + 1));
    /**/
    
    sem_pr11ce = osal_sem_create("sem_pr11ce", 1);
    osal_assert(sem_pr11ce != NULL);
    //printf("\n%p\n",sem_pr11ce);

    sem_322ce = osal_sem_create("sem_322ce", 1);
    osal_assert(sem_322ce != NULL);
    //printf("\n%p\n",sem_322ce);

    sem_ctrlinfo= osal_sem_create("sem_ctrlinfo", 1);
    osal_assert(sem_ctrlinfo != NULL);
    
    for(i = 0;i < dev_index;i++){

        
        ret = alloc_322_index(device_str[i]);
        //printf("ret is %d\n",ret);
        if(ret == 0)
            continue;
        controll_eg.cnt_322++;
        p_usb_ccid = &(controll_eg.usb_ccid_322[ret]);
        p_usb_ccid->ccid322_index = ret;
        p_usb_ccid->usb_state = USB_COMM_STATE_DEFAULT;//USB_COMM_STATE_INIT;//USB_COMM_STATE_INIT_END;//USB_COMM_STATE_INIT;
        p_usb_ccid->toggle_state = 0;
        p_usb_ccid->ccid322_exist = 1;
        p_usb_ccid->init_flag = 0;
        p_usb_ccid->now_door_state[0] = 0x01;
        p_usb_ccid->now_door_state[1] = 0x02;
        p_usb_ccid->pre_door_state[0] = 0x01;
        p_usb_ccid->pre_door_state[1] = 0x02;//门初始状态 关闭
        
        strcpy(&(p_usb_ccid->usb_port),device_str[i]);

/*
       for(i = 0;i < 16;i++){

        printf("0x%X ",p_usb_ccid->usb_port[i]);

       }

        printf("\n--\n%p,%p\n--\n",p_usb_ccid->usb_port,&(p_usb_ccid->usb_port));
*/
            
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "create device pthread %s\n",device_str[i]);

        p_usb_ccid->sem_322 = osal_sem_create(device_str[i], 1);
        osal_assert(p_usb_ccid->sem_322 != NULL);

        p_usb_ccid->sem_state = osal_sem_create(device_str[i], 1);
        osal_assert(p_usb_ccid->sem_state != NULL);
        
        tid = osal_task_create(device_str[i],
                            eg_usb_thread_entry,
                            p_usb_ccid,RT_SYS_THREAD_STACK_SIZE, RT_SYS_THREAD_PRIORITY);
        
        osal_assert(tid != NULL);

        p_usb_ccid->timer_322 = osal_timer_create(device_str[i],timer_usb_callback,p_usb_ccid,\
                            EUSB_SEND_PERIOD, TIMER_INTERVAL|TIMER_STOPPED, TIMER_PRIO_NORMAL);
        osal_assert(p_usb_ccid->timer_322 != NULL);
        
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_WARN, "create timer %s\n",device_str[i]);
        //osal_timer_start(p_usb_ccid->timer_322);

        eg_zmq_init(p_usb_ccid);

    }  
    printf("322 num:%d\n",controll_eg.cnt_322);
    
    //OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "module initial finished\n");

}
