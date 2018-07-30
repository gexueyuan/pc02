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
#include "data_format.h"


#define USB_FAILED    0x6E00
#define USB_OK        0x9000 
#define EUSB_SEND_PERIOD  6000 



#define VENDOR_ID 0x1780  
#define PRODUCT_ID 0x0312

#define USE_TIMESTAMP

#define NET_STATE "/tmp/pc02_connect"

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
typedef enum _MAINT_SRC {
    MAINT_SRC_NONE = 0,
    MAINT_SRC_PR11 = 1,
    MAINT_SRC_322 = 2,
    MAINT_SRC_CARD = 3,
    MAINT_SRC_DOOR = 4,
    MAINT_SRC_321 = 5,
    MAINT_SRC_9531 = 6,
} E_MAINT_SRC;

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
    CARDPOLLEVENT_ANTENNA, //0x09       
    CARDPOLLEVENT_ARMYIC_BROKEN,//0x0A
    CARDPOLLEVENT_FACE,
    CARDPOLLEVENT_ID,
} E_CARDPOLLEVENT;

const unsigned char test1[] = {0xFC,0xA0,0x00,0x00,0x05,0x80,0xCA,0xCE,0x52,0x00};//{0xe0,0xfd,0x00,0x00,0x00,0x00,0x20,0xB1,0xC8,0xFE,0xF7,0x63,0x81,0xF6,0xB6,0x75,0x80,0xEF,0xD2,0xC3,0xAA,0xC4,0x7E,0x5A,0x58,0xC0,0xDD,0x5F,0x66,0x96,0x61,0x5C,0xC1,0x26,0x8B,0xA1,0xB1,0xF4,0x86};

const unsigned char check[] = {0x00,0xA4,0x04,0x00,0x00};//+len+data

const uint8_t car_detect[] = {0xFC,0xA0,0x00,0x00,0x05,0x80,0x30,0x00,0x00,0x00};


const uint8_t refuse_enter[] = {0xFC,0xA0,0x04,0x00,0x09,0x80,0x32,0x80,0x00,0x04,  0x01,0x00,0x01,0x00};

uint8_t test_all[] = {0xFC,0xA0,0x00,0x00,  /*???*/0x23  ,0x80,0x34,0x00,0x00, /*????*/0x18 /**????Χ**/ ,0x02,0x00,0x09,0x32,0x38,0xA1,0xE6,0x2F,0x31,0x35,0xA1,0xE6,\
                      0x03,0x00,0x04  ,0x32,0x35,0xA1,0xE6,/*????*/0x04,0x00,0x01,0x01,/*λ?????*/0x05,0x00,0x04,0xB1,0xB1,0xBE,0xA9};

unsigned char  change_app[25] = {0xFC,0xA0,0x04,0x00,0x14,0x00,0xA4,0x04,0x00,0x0F,0x74,0x64,0x72,0x6F,0x6E,0x99,0x05,0x69,0x64,0x52,0x64,0x51,0x34,0x42,0x31};

E_CARDPOLLEVENT card_event;

uint8_t  test_cmd[] = {0xFC,0xC1, 0x01 ,0x00, 0x06,0x80,0xB0,0x55,0xAF,0x01,0x00};

 /*????*/
 const unsigned char transfer_head[] = {0xFC,0xA0,0x00,0x00};//+len+data

const uint8_t cfg_base_head[] = {0x00,0x25,0x00,0x00};//+len+data

const uint8_t p2pkey_head[] = {0xE0,0xFD,0x00,0x00};//+len+data

const uint8_t mackey_head[] = {0x00,0x2A,0x00,0x00};//+len+data

const uint8_t basecfg_head[] = {0x00,0x25,0x00,0x00};//+len+data

const uint8_t ctlcfg_head[] = {0x00,0x29,0x00,0x00};//+len+data

const uint8_t result_head[] = {0x00,0x22,0x00,0x00};//+extend len(2 bytes)+data

const uint8_t alarm322_head[] = {0x00,0x23,0x00,0x00,0x10};//+len+data clear and check is all 16 bytes

const uint8_t alarm322_clear_head[] = {0x00,0x23,0x80,0x00,0x10};//+len+data clear and check is all 16 bytes

const uint8_t alarm_op_head[] = {0x00,0x2B,0x00,0x00};//len + data

const uint8_t remote_op_head[] = {0x00,0x27,0x00,0x00};//len + data
const uint8_t remote_idop_head[] = {0x00,0x27,0x01,0x00};//len + data
const uint8_t remote_faceop_head[] = {0x00,0x27,0x02,0x00};//len + data

const uint8_t audit_result_head[] = {0x80,0x32,0x80,0x00,0x00};//len + data

const uint8_t ce_pr11[] = {0xFC,0xA0,0x00,0x00,0x05,0x80,0xCA,0xCE,0x53,0x00,};

const uint8_t serial_pr11[] = {0xFC,0xA0,0x00,0x00,0x05,0x80,0xCA,0xCE,0x0E,0x00,};

const uint8_t ce_322[] = {0x80,0xCA,0xCE,0x42,0x00};
const uint8_t pid_322[] = {0x80,0xCA,0xCE,0x45,0x00};
const uint8_t sn_pr11[] = {0xFC,0xA0,0x00,0x00,0x05,0x80,0xCA,0xCE,0x09,0x00};
const uint8_t pid_pr11[] = {0xFC,0xA0,0x00,0x00,0x05,0x80,0xCA,0xCE,0x50,0x00};
const uint8_t ctrl_cfg[] = {0x80,0xCA,0xCE,0x47,0x00};
const uint8_t base_cfg[] = {0x80,0xCA,0xCE,0x48,0x00};

const uint8_t PR11_NO[] = {0x80,0xCA,0xCE,0x49,0x00};//(unsigned char*)"\x80\xCA\xCE\x49\x00";
unsigned char* VER_322 = (unsigned char*)"\x80\xCA\xCE\x4B\x00";



const uint8_t change_mode[] = {0xFC,0xDE,0x00,0xAA,0x00};

const uint8_t v_322[] = {0x00,0x26,0x01,0x00,0x00};
const uint8_t v_322_d21[] = {0x00,0x26,0x01,0x81,0x00};
const uint8_t v_pr11[] = {0x00,0x26,0x02,0x00,0x00};
const uint8_t v_pr02[] = {0x00,0x26,0x05,0x00,0x00};
const uint8_t v_pr11_d21[] = {0x00,0x26,0x02,0x81,0x00};

const uint8_t rd_p2pkey[] = {0x00,0xAA,0x00,0x00, 0x00};
const uint8_t rd_pr11_p2pkey[] = {0xFC, 0xA0, 0x00, 0x00, 0x05,0x00,0xAA,0x00,0x00, 0x00};

const uint8_t confirm[] = {0x90,0x00};

const uint8_t _end_confirm[] = {0x90,0x00,0x90,0x00};
const uint8_t _end_alarm[] = {0x90,0x00,0x90,0x0A};

const uint8_t reset_322usb[] = {0xF0,0xF0,0x02,0xFF,0x00};


osal_sem_t *sem_pr11ce;

osal_sem_t *sem_322ce;

osal_sem_t *sem_ctrlinfo;

osal_sem_t *sem_alarm;

const uint8_t open_door[] = {0x80,0xDD,0x33,0x00,0x03,0x01,0x00,0x02};

const uint8_t test_p2p[] = {0xE0,0xFD,0x00,0x00,0x21,0x08,0xF8,0x19,0x94,0x85,0x3C,0x1D,0xBC,0x72,0xCA,0x6B,0x95,0xA7,0xAE,0xB5,0x6F,0xA8,0x84,0xC9,0x99,0x62,0x50,0x46,0x7F,0x06,0xBD,0x40,0xC4,0xC2,0x24,0x50,0x04,0xE7};
                                            /*0*/   /*1*/   /*2*/   /*3*/   /*4*/
const char* usb_port_def[MAX_322_NUM + 1] = {"1-1.1","1-1.2","1-1.3","1-1.4","1-1.5"};

unsigned char* test_o = (unsigned char*)"\x00\x22\x00\x00\xBE\x01\xF6\xBA\x57\xB5\x61\xDD\x09\xE4\x39\xCF\x52\x4D\xF1\x6F\x2B\xC8\x04\xC0\xEE\xCB\xC4\x3A\x41\x20\x97\x11\x05\x19\x0F\x0F\x0F\x12\x05\x19\x0F\x0F\x0F\x99\xF0\x8B\xA6\x68\x92\xA0\x4C\xC7\x72\x0A\x4D\xC2\x29\x49\x7D\x81\x6C\x1A\x20\x94\x7A\x2A\xA0\xF2\xDE\xCC\x8E\xC1\x2F\x3D\x1D\x2A\x6E\x2B\x9D\xAF\x19\xD6\x8C\x9D\x23\x06\x28\x0B\x30\xB0\xAB\xDB\xE4\x11\x4A\x28\x2E\x2B\x56\x85\xDE\x4B\x0B\x9A\x35\xFF\xCA\xF4\xB7\x31\x9A\x15\xED\xA0\x47\xDC\x66\x4A\x95\x79\xD7\xFB\x8B\x9C\xF3\x50\x10\xFE\x75\xA4\x6B\xDF\x76\x95\x84\x27\xE9\x1D\xFB\x34\xF4\xE8\x04\x32\xF5\xE3\xB3\xBA\x83\xF2\xEF\x5B\x24\x50\x7D\x4F\x95\x76\x95\x73\x72\x71\xD6\xE9\x0A\x7D\xBF\x5F\xFB\xB6\x8E\xA6\xE3\xE9\xE6\xFC\xF5\x1C\x4A\xE9\x30\xDC\x28\xBF\x57\x87\xE6\x80\x00\x00\x00\xC5\x39\x64\x35";


const uint8_t id_reader[] = {0x00,0x01,0x00,0x02,0x01,0x01};

const uint8_t id_reader_deal_OK[] = {0x00,0x01,0x00,0x02,0x01,0x03};

const uint8_t id_info_get[] = {0x00,0xc2,0x00,0x00,0x00};

const uint8_t id_cmd_1[] = {0x80,0xCA,0xCE,0x4E,0x4,0x0,0x0,0x0,0xD0};

const uint8_t wgp_info_get[] = {0xF0,0xF2,0x01,0xFD,0x00,0x00};

const uint8_t wgp_info_clear[] = {0xF0,0xF2,0x01,0xFE,0x01,0xFF};


uint8_t net_state[] = {0xFC,0xA0,0x00,0x00,0x09,0x80,0x34,0x00,0x00,0x04,0x09,0x00,0x01,0x80};


/* ?????? GBK*/
char *antenna_broken = "PR11??????";//{0x50,0x52,0x31,0x31,0xBE,0xFC,0xC3,0xDC,0xD0,0xBE,0xC6,0xAC,0xCB,0xF0,0xBB,0xB5};
char *IC_broken = "PR11????о???";//{0xBE,0xFC,0xC3,0xDC,0xD0,0xBE,0xC6,0xAC,0xCB,0xF0,0xBB,0xB5};
char *usb_disconnect = "PC02-322USB???ж?";
char *log_failed = "????????????";
char *wgp_alarm = "WGP??λ????";
char *wgp_host_num = "WGP???????????????";
char *wgp_slave_num = "WGP???????????????";
char *wgp_num = "WGP??????????λ????";
char *wgp_cnt = "WGP????????????";
char *wgp_time="WGP???????????";
char *_322_error="322 ???????";

/* ?????? GBK*/


/*
* ???????: д?????????
* ????????: _fileName, ???????
*           _buf, ?д????滺?塣
*           _bufLen, ??滺??????
*   ?????: 0, ???
*           -1, ???
*
*/
int writeFile(const char* _fileName, void* _buf, int _bufLen)
{
    FILE * fp = NULL;
    if( NULL == _buf || _bufLen <= 0 ) return (-1);

    fp = fopen(_fileName, "ab+"); // ??????????? ??????д????????

    if( NULL == fp )
    {
        return (-1);
    }

    fwrite(_buf, _bufLen, 1, fp); //??????д

    fclose(fp);
    fp = NULL;

    return 0;    
}

/*
 * ???????:  ???????????
*  ????????: _fileName, ???????
*             _buf, ??????????????λ??
*             _bufLen, ???????????
*    ?????:  0, ???
*             -1, ???
*
*/
int readFile(const char* _fileName, void* _buf, int _bufLen)
{
    FILE* fp = NULL;
    if( NULL == _buf || _bufLen <= 0 ) return (-1);

    fp = fopen(_fileName, "rb"); // ??????????? ??????????????? 

    if( NULL == fp )
    {
        return (-1);
    }

    fread(_buf, _bufLen, 1, fp); // ???????

    fclose(fp);
    return 0;        
}


int file_exist(const char* _fileName)
{

    if((access(_fileName,F_OK))!=-1)   
    {   
        printf("\n???%s????.\n",_fileName);
        return 1;
    }
    else{
        
        printf("\n???%s??????.\n",_fileName);
        return 0;

    }



}

int net_state_read(void)
{

    return file_exist(NET_STATE);

}

void print_array(const  char* tag,unsigned char* send,int len)
{

#ifdef USE_TIMESTAMP
    int i;
    struct timeval tvl;
    struct tm * local_t;
    
    gettimeofday(&tvl, NULL);
    local_t = localtime(&tvl.tv_sec);
    
    if(osal_module_debug_level <= OSAL_DEBUG_INFO){
        printf("\n[%02d:%02d:%02d.%03d]<%s>len is :%d\n\r",local_t->tm_hour,local_t->tm_min,local_t->tm_sec,(int)(tvl.tv_usec/1000),tag,len);
        for(i = 0;i < len;i++){
        
            printf("%02X ",send[i]);
            
        }
        printf("\n\r\n");
    }

#else
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



void print_rec(unsigned char* rec,int len)
{
#ifdef USE_TIMESTAMP
        int i;
        struct timeval tvl;
        struct tm * local_t;
        
        gettimeofday(&tvl, NULL);
        local_t = localtime(&tvl.tv_sec);

        if(osal_module_debug_level <= OSAL_DEBUG_INFO){
            printf("\n[%02d:%02d:%02d.%03d]recv data len is :%d\n\r",local_t->tm_hour,local_t->tm_min,local_t->tm_sec,(int)(tvl.tv_usec/1000),len);
            for(i = 0;i < len;i++){
            
                printf("%02X ",rec[i]);
                
            }
            printf("\n\r\n");
        }

#else
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


void print_send(const unsigned char* send,int len)
{

#ifdef USE_TIMESTAMP
    int i;
    struct timeval tvl;
    struct tm * local_t;
    
    gettimeofday(&tvl, NULL);
    local_t = localtime(&tvl.tv_sec);
    
    if(osal_module_debug_level <= OSAL_DEBUG_INFO){
        printf("\n[%02d:%02d:%02d.%03d]send data len is :%d\n\r",local_t->tm_hour,local_t->tm_min,local_t->tm_sec,(int)(tvl.tv_usec/1000),len);
        for(i = 0;i < len;i++){
        
            printf("%02X ",send[i]);
            
        }
        printf("\n\r\n");
    }

#else
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
void StatisticsInfo_push(uint8_t src_dev,uint8_t *pid_dev,const char *gbk_str,uint8_t* value)
{
    uint8_t info_statistic[41] = {0};
    
    memset(info_statistic,0,sizeof(info_statistic));
    info_statistic[0] = src_dev;//from pr11
    memcpy(&info_statistic[1],pid_dev,4);//pr11 id 4
    memcpy(&info_statistic[5],gbk_str,strlen(gbk_str));//32 gbk
    //info_statistic[37] = (value&0xFF00)>>8;
    //info_statistic[38] = value&0x00FF;
    if(value != NULL)
        memcpy(&info_statistic[37],value,4);
    print_array(gbk_str,info_statistic,sizeof(info_statistic));
    ubus_net_process(UBUS_CLIENT_SEND_StatisticsInfo,NULL,info_statistic,sizeof(info_statistic));


}
int usb_transmit(void *context, const unsigned char * apdu,
            int apdu_len, unsigned char * resp, int max_resp_size,usb_ccid_322_t *usb_322)
{

    int ret = 0;

    int connect_ret = 0;

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
    
        printf("transmit return is %d\n",ret);
        memset(output, 0, sizeof(output));
        error = luareader_pop_value(context, (char *)output, sizeof(output));
        printf("%s-luareader_pop_value(%p)=%d(%s)\n",usb_322->usb_port,context, error, output);

        printf("reconnect\n");

        connect_ret = luareader_disconnect(usb_322->usb_context);
        
        if(connect_ret < 0){
            
            OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_WARN, "disconnect failed\n");
        }
        
        msleep(300);

        
        connect_ret = luareader_connect(usb_322->usb_context,usb_322->usb_port);

        if(connect_ret < 0){

            usb_322->usb_reconnect_cnt++;
            
            OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_WARN, "connect failed\n");

            if(usb_322->usb_reconnect_cnt >= 10){
                StatisticsInfo_push(MAINT_SRC_322,usb_322->pid_322,usb_disconnect,0);
                OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_WARN, "connect failed 10 times,reboot!!!\n");
                system("reboot");
            }
        }
        else{

            //msleep(200);
            OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_WARN, "reconnect succeed!\n");
            
            usb_322->usb_reconnect_cnt = 0; 
            
            //ret = usb_transmit(context,controll_eg.p2pkey.data,controll_eg.p2pkey.len,output,sizeof(output),p_usb_ccid);
/*
            ret = luareader_transmit(context, controll_eg.p2pkey.data, controll_eg.p2pkey.len, output, sizeof(output),3000);
            OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_WARN, "update p2pkey,ret = %d\n",ret);
            ret = luareader_transmit(context, controll_eg.mackey.data, controll_eg.mackey.len, output, sizeof(output),3000);
            OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_WARN, "update mackey,ret = %d\n",ret);
*/
        }
 
    }
    else{

       usb_322->usb_reconnect_cnt = 0; 

    }

    return ret;


}



/*alloc index for 322*/
int alloc_322_index(char * port_name)
{

    int i = 0;

    for(i = 0;i < MAX_322_NUM + 1;i++){

        if(strcmp(usb_port_def[i],port_name) == 0){
            
            return  i;
            
        }

    }

    return -1;
}
uint8_t alarm_fileter(unsigned char* log,int len,int type)//alarm 0103
{
	alarm_log_t* get_log = NULL;

	if(len != sizeof(alarm_log_t)){
		printf("alarm len wrong");
		return 0;
	}
	
	get_log = (alarm_log_t*)malloc(len);

	memcpy(get_log,log,sizeof(alarm_log_t));
	return get_log->alarm_type;

	free(get_log);

}
void eg_usb_main_proc(char *data,int len)
{
}

/*****************************************************************************
 @funcname: get_sys_time
 @brief   : get systime as acsii
 @param   : 19byte,
// struct tm {
//   int tm_sec;         // ????Χ?? 0 ?? 59				
//   int tm_min;         // ?????Χ?? 0 ?? 59				
//   int tm_hour;        // С?????Χ?? 0 ?? 23				
//   int tm_mday;        // ????е???????Χ?? 1 ?? 31	                
//   int tm_mon;         //?·????Χ?? 0 ?? 11				
//   int tm_year;       //  ?? 1900 ???????				
//   int tm_wday;       //  ????е???????Χ?? 0 ?? 6		        
//   int tm_yday;      //   ????е???????Χ?? 0 ?? 365	                
//   int tm_isdst;     //   ?????							
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
    time_ptr[sizeof(time_head) + 3] = ((unsigned char)timenow-> tm_year) > 100 ? (unsigned char)timenow-> tm_year - 100:17;//最晚2017年
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


int check_card(usb_ccid_322_t  *usb_322,unsigned char* rd_data,int buffer_len)
{
    uint8_t resp_code[] = {0x90,0x00};
    uint8_t resp_code_2[] = {0x90,0x01};
    
    
    uint8_t alarm_code[] = {0x90,0x0A};
    uint8_t wg_error[] = {0x93,0x01};

    uint8_t wgp_info[]={0x6F,0xFF};

	
    uint8_t reader_offline[]={0x90,0x0A};

    //read_buffer = (unsigned char*)malloc(buffer_len);

    //memcpy(read_buffer,rd_data,buffer_len);


    

    if(buffer_len == 5)//0,1=pr 11;2=door state;3,4=322
    {
        if(memcmp(rd_data,resp_code_2,sizeof(resp_code_2)) == 0){//pr11 return no card 0,1;
            
            if(memcmp(&rd_data[3],resp_code,sizeof(resp_code)) == 0){//3,4 322 return 90 00,no alarm
                
                    //??????,??????????????????? 
                    
                    usb_322->now_door_state[1] = rd_data[2];//door state=2
             
                    if(usb_322->now_door_state[1] != usb_322->pre_door_state[1]){
             
                         usb_322->pre_door_state[1] = usb_322->now_door_state[1];
             
                         sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_SEND_DRSTATE,2,0,usb_322->now_door_state);
                         
                    } 
                     return 0;
            } else if(memcmp(&rd_data[3],alarm_code,sizeof(alarm_code)) == 0){
            
                //??????????322????
                //OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_INFO, "90 0A ALARM!\n");
                //free(read_buffer);
                return 2;
                
            }

            
        }
        


        
    }

    if(buffer_len == 2){
        
        if(memcmp(rd_data,wg_error,sizeof(wg_error)) == 0){
            
            OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_INFO, "wg error 9301,sleep 1 sec\n");
            sleep(3);
            //free(read_buffer);
            return -3;
        }

        if(memcmp(rd_data,wgp_info,sizeof(wgp_info)) == 0){
            
            OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_INFO, "wg error 6FFF,get info\n");
                      
            return -4;
        }
		if(memcmp(rd_data,reader_offline,sizeof(reader_offline)) == 0){
			
			OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_INFO, "reader offline 90 0A \n");
					  
			return 2;
		}
		
    }
       
    if(buffer_len < 5){

        //OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_INFO, "reader return short\n");
        //free(read_buffer);
        return -2;
    }

/*???900A???,??????9000????900A????????????????ж?
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

const char *cardtype[] = {"none","15693","sfz","ID cards","cpu","12.5K","MIFARE"};
int parse_data(unsigned char* rd_data,int buffer_len,unsigned char* wl_data,int *wl_len,usb_ccid_322_t  *usb_322)
{
    unsigned char *read_buffer;
    int len;
    int card_event;
    int ret;
    len = buffer_len;
    //card id 4Byte 20170531
    unsigned char card_id[4] = {0};

    unsigned char audit_buffer[2048] = {0};

    uint8_t info_statistic[39] = {0};

    int audit_len = 1024;

    memset(card_id,0,sizeof(card_id));

    memset(audit_buffer,0,sizeof(audit_buffer));

    read_buffer = (unsigned char*)malloc(buffer_len);

    memcpy(read_buffer,rd_data,buffer_len);

    //old cos version has no door state,when detect card,
    //now the newest cos always has the door state
    
    card_event = read_buffer[0];//0-card event;1-card type;2~5-card id;6-card battery

    switch(card_event)
    {
        case CARDPOLLEVENT_NO_EVENT:

            //card type
            if(read_buffer[1] == 0x02){

                printf("find Id card\n");

                if(controll_eg.network_state){
                    
                    printf("\nbegin online gfread\n");
                    return CARDPOLLEVENT_ID;

                }
                else{
                    printf("\nbegin offline gfread\n");
                }


            }
            
            if(read_buffer[1] == 0x03){

                //printf("\ncard's battery is %d\n",read_buffer[6]);
				OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_INFO, "\ncard's battery is %d\n",read_buffer[6]);
				//print_rec(read_buffer, 6);

                ubus_net_process(UBUS_CLIENT_SEND_BAT,NULL,&read_buffer[2],5);

				
				//print_rec(read_buffer, 6);


            }

            
 #if 1           
            if(len < 64){

                memcpy(card_id,&read_buffer[2],4);

                printf("card id is \n");
			
				OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_INFO, "card id is \n");
                print_rec(card_id,4);
                
/*
                gettimeofday( &_start, NULL );
                printf("start : %d.%d\n", _start.tv_sec, _start.tv_usec);
*/
                
                //ubus_net_process(UBUS_CLIENT_SEND_ID_INFO,NULL,1,1);
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
       case CARDPOLLEVENT_ANTENNA:
           if((controll_eg.push_flag & 0x02) == false){
            
                memset(info_statistic,0,sizeof(info_statistic));
                info_statistic[0] = MAINT_SRC_PR11;//from pr11
                memcpy(&info_statistic[1],usb_322->pid_pr11,4);//pr11 id 4           
                memcpy(&info_statistic[5],antenna_broken,strlen(antenna_broken));

                ubus_net_process(UBUS_CLIENT_SEND_StatisticsInfo,NULL,info_statistic,sizeof(info_statistic));
                controll_eg.push_flag |= 0x02;
            }
            break;
       case CARDPOLLEVENT_ARMYIC_BROKEN:
           if((controll_eg.push_flag & 0x04) == false){
            
                memset(info_statistic,0,sizeof(info_statistic));
                info_statistic[0] = MAINT_SRC_PR11;//from pr11
                memcpy(&info_statistic[1],usb_322->pid_pr11,4);//pr11 id 4
                memcpy(&info_statistic[5],IC_broken,strlen(IC_broken));
                ubus_net_process(UBUS_CLIENT_SEND_StatisticsInfo,NULL,info_statistic,sizeof(info_statistic));
                controll_eg.push_flag |= 0x04;
            }
            break;
        default:
            //OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_WARN, "error data,%d\n",card_event);
            break;

    }


//end:
    free(read_buffer);

    return card_event;

}

/*
int get_data(void * context,usb_ccid_322_t *p_usb_ccid,const unsigned char  *apdu,int apdu_len)
{
	unsigned char output[2048] = {0};

    int ret = 0;

    ret = usb_transmit(context,apdu,apdu_len,output,sizeof(output),p_usb_ccid);
    
    if(ret <= 0){
    
        
        memset(output, 0, sizeof(output));
        ret = luareader_pop_value(context, (char *)output, sizeof(output));
        printf("luareader_pop_value(%p)=%d(%s)\n", context, ret, output);
            
    }else{
    
        if(memcmp(&output[ret - 2],confirm,sizeof(confirm)) == 0){
            
                print_rec(output,ret);
    
                p_usb_ccid->door_index = output[8];////door no get
                p_usb_ccid->pre_door_state[0] = p_usb_ccid->door_index;
                p_usb_ccid->now_door_state[0] = p_usb_ccid->door_index;
            }
        else{
            
            print_rec(output,ret);
            
        }
    }
    

}
*/
//void get_info_from_cos(uint8_t * apdu,uint16_t apdu_len,)
//{












//}

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
    unsigned char parse_tag = 0;
    unsigned char acl_data[2048] = {0};
	unsigned char output[2048] = {0};
    int acl_len = 1024;
    unsigned char send_data[1024] = {0};

    unsigned char apud_data[1024] = {0};

    unsigned char log_data[2048] = {0};
    int log_len = 0;

    int remote_len = 0;
    int audit_len = 0;
    int tail_check = 0;
    unsigned short wl_len = 0;
    unsigned char rtc[16] = {0};
    unsigned char ctrl_info[25] = {0};

    usb_ccid_322_t *p_usb_ccid;

    int rec_zmq = 0;

    uint8_t zmq_ans[256] = {0};
    
    uint32_t return_req = 0;

    uint8_t alarm_clear_rt_data[16] = {0};//8+2+index(1)+all 322 index(max 4 + 1(321))
    uint8_t remote_open_rt_data[16] = {0};//8+2++index+all322 index(max 4)

    uint8_t info_statistic[39] = {0};
    /**test**/
    
    //struct timeval _start,_end;
    /**test**/

	unsigned char zmq_output[2048] = {0};

	unsigned char face_req[3] = {0x01,0x90,0x00};

    p_usb_ccid = (usb_ccid_322_t*)parameter;

    p_usb_ccid->usb_context = luareader_new(0, NULL, NULL);
    
	void * context = p_usb_ccid->usb_context;

    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "context is %p\n",context);

    //strcpy(usb_port,(const char*)parameter);
    ret = luareader_connect(context, p_usb_ccid->usb_port);
    if(ret < 0){
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "connect to  %s failed\n",p_usb_ccid->usb_port);
        goto out;
    }
    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "connect to  %s succeed\n",p_usb_ccid->usb_port);

    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "%s pr11 state\n",p_usb_ccid->usb_port);
	print_send(PR11_NO,sizeof(PR11_NO));
	ret = usb_transmit(context,PR11_NO,sizeof(PR11_NO),output,sizeof(output),p_usb_ccid);
	print_rec(output,ret);



    sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_INITED,0,p_usb_ccid->ccid322_index,NULL);

while(1){

#if 1
    memset(output,0,sizeof(output));
    memset(zmq_output,0,sizeof(zmq_output));
    memset(apud_data,0,sizeof(apud_data));
    //memset(cfg_return,0,sizeof(cfg_return));
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

                if(memcmp(output,confirm,sizeof(confirm)) == 0){
                    
                    p_usb_ccid->rtc_sync = 0xAA;
                    OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "begin to get alarm log\n");

                }
                /************************TEST**************************
                sleep(1);
                memset(output,0,sizeof(output));
                print_send(rd_p2pkey,sizeof(rd_p2pkey));
                ret = usb_transmit(context,rd_p2pkey,sizeof(rd_p2pkey),output,sizeof(output),p_usb_ccid);
                print_rec(output,ret);

                if(memcmp(output,confirm,sizeof(confirm)) == 0){
                    
                    p_usb_ccid->rtc_sync = 0xAA;

                }
                memset(output,0,sizeof(output));
                print_send(rd_pr11_p2pkey,sizeof(rd_pr11_p2pkey));
                ret = usb_transmit(context,rd_pr11_p2pkey,sizeof(rd_pr11_p2pkey),output,sizeof(output),p_usb_ccid);
                print_rec(output,ret);
                ************************TEST**************************/
                
                sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_SEND_COSVERSION,0,0,NULL);
                osal_sem_release(p_usb_ccid->sem_state);
                //p_usb_ccid->usb_state = USB_COMM_STATE_MACKEY;//USB_COMM_STATE_RDCFG;//USB_COMM_STATE_MACKEY;//USB_COMM_STATE_CFG;//USB_COMM_STATE_MACKEY;
                
            break;
            
        case USB_COMM_STATE_MACKEY:

                
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "send MACKEY\n");
                
                
                controll_eg.mackey.len = controll_eg.mackey.len > 64 ? 37:controll_eg.mackey.len;
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


                return_req = (0x0000000F&p_usb_ccid->ccid322_index)<<16;
                return_req |= (((0x00FF&output[ret - 2])<<8)|output[ret - 1]);
                //printf("get base cfg is %04X\n",return_req);
                
                //sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_322_RETURN,USB_COMM_STATE_CFG,return_req,NULL);
                controll_eg.base_cfg_rt[8] = output[ret - 2];
                controll_eg.base_cfg_rt[9] = output[ret - 1];

                controll_eg.base_cfg_rt[10] = p_usb_ccid->ccid322_index;

                printf("\nsend base return\n");
                print_send(controll_eg.base_cfg_rt,11 + controll_eg.cnt_322);
                ubus_net_process(UBUS_CLIENT_RETURN,NULL,controll_eg.base_cfg_rt,11 + controll_eg.cnt_322);

                
                osal_sem_release(controll_eg.sem_base_cfg);        
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

                return_req = (0x0000000F&p_usb_ccid->ccid322_index)<<16;
                return_req |= (((0x00FF&output[ret - 2])<<8)|output[ret - 1]);
                //printf("get ctrl cfg is %04X\n",return_req);
                controll_eg.ctrl_cfg_rt[8] = output[ret - 2];
                controll_eg.ctrl_cfg_rt[9] = output[ret - 1];
                controll_eg.ctrl_cfg_rt[10] = p_usb_ccid->ccid322_index;

                
                printf("\nsend ctrl return\n");
                print_rec(controll_eg.ctrl_cfg_rt,11 + controll_eg.cnt_322);
                ubus_net_process(UBUS_CLIENT_RETURN,NULL,controll_eg.ctrl_cfg_rt,11 + controll_eg.cnt_322);
                //sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_322_RETURN,USB_COMM_STATE_RDCFG,return_req,NULL);
                
                osal_sem_release(controll_eg.sem_ctrl_cfg);        
                osal_sem_release(p_usb_ccid->sem_state);
                
                

            break;
            

        case USB_COMM_STATE_VERSION://????????322???????????????????????????????????????????????ж??

            //if(sw_version == 0)
                {
                
                sw_version = 0xAA;
                /**********************************322 version***********************************************/
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "get 322 version:");
                ret = usb_transmit(context,v_322,sizeof(v_322),&output[1],sizeof(output) - 1,p_usb_ccid);
				#ifdef MULTIVERSION
				output[0] = p_usb_ccid->ccid322_index + 1 +'0';
				#else
				output[0] = 0x01;
				#endif
                print_rec(output,ret + 1);
                if(ret > 0){
                    if(memcmp(confirm,&output[ret - 1],sizeof(confirm)) == 0)//ret~ret +1
                        ubus_client_process(UBUS_CLIENT_SENDVERSION,NULL,output,ret - 1);//sum -2 = ret + 1 - 2=ret -1
                    else
                        OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "bad 322 version:");
                }
                /**********************************322 version end***********************************************/


                /**********************************322-d21 version***********************************************/
                memset(output,0,sizeof(output));
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "get 322-d21 version:");
                ret = usb_transmit(context,v_322_d21,sizeof(v_322_d21),&output[1],sizeof(output) - 1,p_usb_ccid);      
#ifdef MULTIVERSION
				output[0] = p_usb_ccid->ccid322_index + 1 +'0';
#else
				output[0] = 0x02;
#endif
                print_rec(output,ret + 1);
                if(ret > 0){
                    if(memcmp(confirm,&output[ret - 1],sizeof(confirm)) == 0)//ret~ret +1
                        ubus_client_process(UBUS_CLIENT_SENDVERSION,NULL,output,ret - 1);
                    else
                        OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "bad 322-d21 version:");
                }
                /**********************************322-d21 version end***********************************************/
				/**********************************pr11-d21 version***********************************************/
				memset(output,0,sizeof(output));
				OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "get pr11-d21 version:");
				ret = usb_transmit(context,v_pr11_d21,sizeof(v_pr11_d21),&output[1],sizeof(output) - 1,p_usb_ccid);
#ifdef MULTIVERSION
								output[0] = p_usb_ccid->ccid322_index + 1 +'0';
#else
								output[0] = 0x04;
#endif
				print_rec(output,ret + 1);
				if(ret > 0){
					if(memcmp(confirm,&output[ret - 1],sizeof(confirm)) == 0)//ret~ret +1
						ubus_client_process(UBUS_CLIENT_SENDVERSION,NULL,output,ret - 1);
					else
						OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "bad pr11-d21 version:\n");
				}
			/**********************************pr11-d21 version end***********************************************/


                /**********************************pr11 or pr02 version***********************************************/
                memset(output,0,sizeof(output));
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "get pr11 version:");
                ret = usb_transmit(context,v_pr11,sizeof(v_pr11),&output[1],sizeof(output) - 1,p_usb_ccid);
#ifdef MULTIVERSION
								output[0] = p_usb_ccid->ccid322_index + 1 +'0';
#else
								output[0] = 0x03;
#endif
                print_rec(output,ret + 1);
                if(ret > 0){
                    if(memcmp(confirm,&output[ret - 1],sizeof(confirm)) == 0)//ret~ret +1
                        ubus_client_process(UBUS_CLIENT_SENDVERSION,NULL,output,ret - 1);
                    else{
                        
                        OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "bad pr11 version,try to get pr02 version\n");

                        memset(output,0,sizeof(output));
                        OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "get pr02 version:");
                        ret = usb_transmit(context,v_pr02,sizeof(v_pr02),&output[1],sizeof(output) - 1,p_usb_ccid);
#ifdef MULTIVERSION
										output[0] = p_usb_ccid->ccid322_index + 1 +'0';
#else
										output[0] = 0x03;
#endif
                        print_rec(output,ret + 1);
                        if(ret > 0){
                            if(memcmp(confirm,&output[ret - 1],sizeof(confirm)) == 0)//ret~ret +1
                                ubus_client_process(UBUS_CLIENT_SENDVERSION,NULL,output,ret - 1);
                            else{
                                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "bad pr02 version,try to get pr02 version\n");
								//osal_sem_release(p_usb_ccid->sem_state);
								//goto out;
                                                                		
                            }
                        }
                        
                    }
                }

                /**********************************pr11 or pr02 version end***********************************************/


            }
            
            
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
			p_usb_ccid->pr11_exist = 0; 		
            goto JUMP;
            
        }else{
        
            if((memcmp(&output[ret - 4],_end_confirm,sizeof(_end_confirm)) == 0)||(memcmp(&output[ret - 4],_end_alarm,sizeof(_end_alarm)) == 0)){
            

                    OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "pr11 sn,len is %d\n",ret - 2);//minus 90 00
                    memcpy(p_usb_ccid->sn_pr11,output,16);//pr11 sn len is 12
                    print_rec(output,ret);
                }
            else{
            
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "read pr11 sn error,thread exit\n");//minus 90 00
                
                //osal_sem_release(p_usb_ccid->sem_state);
                p_usb_ccid->pr11_exist = 0; 		
                goto JUMP;
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
			p_usb_ccid->pr11_exist = 0; 		
            goto JUMP;
        
        }else{
        
            if((memcmp(&output[ret - 4],_end_confirm,sizeof(_end_confirm)) == 0)||(memcmp(&output[ret - 4],_end_alarm,sizeof(_end_alarm)) == 0)){
            

                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "pr11 pid,len is %d\n",ret - 2);//minus 90 00
                memcpy(p_usb_ccid->pid_pr11,output,4);//pr11 id len si 4
                print_rec(output,ret);
                
            }
            else{
                
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "read pr11 pid error,thread exit\n");//minus 90 00

                
                //osal_sem_release(p_usb_ccid->sem_state);
                p_usb_ccid->pr11_exist = 0; 		
                goto JUMP;
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
JUMP:
        /************************************************finish************************************************************/
            p_usb_ccid->init_flag |= INIT_MASK_CE;
            //update_ce(); 
            //msleep(1000);//waiting for another thread detect pr11 or 322	
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
            //p_usb_ccid->rtc_sync = 0xAA;
/*
            print_send(read_mac,strlen(read_mac));
            ret = usb_transmit(context,read_mac,strlen(read_mac),output,sizeof(output),p_usb_ccid);            
            print_rec(output,ret);
            
            print_send(read_p2p,strlen(read_p2p));
            ret = usb_transmit(context,read_p2p,strlen(read_p2p),output,sizeof(output),p_usb_ccid);            
            print_rec(output,ret);
*/
            
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

           //ubus_net_process(UBUS_CLIENT_SEND_ALARM,NULL,output,ret - 2);
           ubus_client_process(UBUS_CLIENT_LOG,NULL,output,ret - 2);
           OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "\nsend alarm log to 321\n");
           osal_sem_release(p_usb_ccid->sem_state);
            break;
            
        case USB_COMM_REMOTE_OPEN://0,1=length;2~34=reserve bytes;34~end=data
            
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "remote open\n");
            
            memcpy(apud_data,remote_op_head,sizeof(remote_op_head));
            apud_data[sizeof(remote_op_head)] =  0x00;//extend data
            
            remote_len = ((controll_eg.remote_buffer[0]<<8)|(controll_eg.remote_buffer[1]&0x00FF));//indeed remote data length
            memcpy(&apud_data[sizeof(remote_op_head) + 1],&controll_eg.remote_buffer,2); 
            memcpy(&apud_data[sizeof(remote_op_head) + 1 + 2],&controll_eg.remote_buffer[34],remote_len);//0,1=len;2~33=ubus data; 34~=remote data
            
            print_send(apud_data,sizeof(remote_op_head) + 1 + 2 + remote_len);
            ret = usb_transmit(context,apud_data,sizeof(remote_op_head) + 1 + 2 + remote_len,output,sizeof(output),p_usb_ccid);
            print_rec(output,ret);

            memset(remote_open_rt_data,0,sizeof(remote_open_rt_data));
			
			memcpy(remote_open_rt_data,&controll_eg.remote_buffer[2],8);
			
            if(ret > 2){
                
                log_len = ret - 2;
                memcpy(log_data,output,log_len);
                ubus_client_process(UBUS_CLIENT_LOG,NULL,log_data,log_len);

                
                memcpy(remote_open_rt_data,&controll_eg.remote_buffer[2],8);
                if(output[197] == 0x01){//197 = result
                        remote_open_rt_data[8] = 0x90;
                        remote_open_rt_data[9] = 0x00;
                    }
                else{
                        remote_open_rt_data[8] = 0x00;
                        remote_open_rt_data[9] = output[197];
                    }
                    
                
            }
            else{

                    remote_open_rt_data[8] = output[ret - 2];
                    remote_open_rt_data[9] = output[ret - 1];

            }
            remote_open_rt_data[10] =  p_usb_ccid->ccid322_index;
            memcpy(&remote_open_rt_data[11],&controll_eg.index_322,controll_eg.cnt_322);
            printf("\nsend remote open return\n");
            print_send(remote_open_rt_data,11 + controll_eg.cnt_322);
            ubus_net_process(UBUS_CLIENT_RETURN,NULL,remote_open_rt_data,11 + controll_eg.cnt_322);
            osal_sem_release(controll_eg.sem_remote);
            osal_sem_release(p_usb_ccid->sem_state);

            break;
            
        case  USB_COMM_ID_READ:
            
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "322 usb transmit:");
            print_send(p_usb_ccid->zmq_buffer,p_usb_ccid->zmq_len);
            ret = usb_transmit(context,p_usb_ccid->zmq_buffer,p_usb_ccid->zmq_len,output,sizeof(output),p_usb_ccid);
            //OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "322 Id return:\n");
		#ifdef ZMQ_NUM
			if(ret > 0){
                print_rec(output,ret);
				memcpy(zmq_output,p_usb_ccid->zmq_magicnum,5);
				memcpy(&zmq_output[5],output,ret);
                zmq_socket_send(p_usb_ccid->zmq_server,zmq_output,ret + 5);
				
            }else {       
				
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "322 usb transmit error\n");
				
				memcpy(zmq_output,p_usb_ccid->zmq_magicnum,5);
				
				memcpy(&zmq_output[5],_322_error,strlen(_322_error));
				
				zmq_socket_send(p_usb_ccid->zmq_server,zmq_output,5 + sizeof(_322_error));
			}
		#else
			if(ret > 0){
	                print_rec(output,ret);
	                zmq_socket_send(p_usb_ccid->zmq_server,output,ret);
	            }else            
	                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "322 usb transmit error\n");
		#endif
			p_usb_ccid->usb_state = USB_COMM_STATE_DEFAULT;
            osal_sem_release(p_usb_ccid->sem_state);
			
			osal_sem_release(p_usb_ccid->sem_zmq);
            break;
            
        case  USB_COMM_ID_DOOR_SERVER:
            
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "Id info to server\n");
            
            print_send(id_info_get,sizeof(id_info_get));
            ret = usb_transmit(context,id_info_get,sizeof(id_info_get),output,sizeof(output),p_usb_ccid);
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "322 Id info return:\n");            
            print_rec(output,ret);
            
            if((ret >2)&&(memcmp(&output[ret -2],confirm,sizeof(confirm)) == 0)){
                
                    OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "make id data:\n");
                    apud_data[0] = p_usb_ccid->door_index;
                    
                    memcpy(&apud_data[1],output,ret -2);
                    memcpy(&apud_data[1 + ret -2],&zmq_ans[4],rec_zmq - 4);
                    //memcpy(&apud_data[1+rec_zmq - 4],output,ret -2);
                    
                    print_send(apud_data,1+ rec_zmq - 4 + ret -2);
                    ubus_net_process(UBUS_CLIENT_SEND_ID_INFO,NULL,apud_data,1+rec_zmq - 4+ret -2);
            }
            osal_sem_release(p_usb_ccid->sem_state);
            break;

        case USB_ID_REMOTE_OPEN:
            
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "ID remote open\n");
            
            memcpy(apud_data,remote_idop_head,sizeof(remote_idop_head));
            apud_data[sizeof(remote_idop_head)] =  0x00;//extend data
            
/*
            remote_len = ((controll_eg.remote_buffer[0]<<8)|(controll_eg.remote_buffer[1]&0x00FF));
            memcpy(&apud_data[sizeof(remote_idop_head) + 1],&controll_eg.remote_buffer,remote_len + 2); 
            
            print_send(apud_data,sizeof(remote_idop_head) + 1 + 2 + remote_len);
            ret = usb_transmit(context,apud_data,sizeof(remote_idop_head) + 1 + 2 + remote_len,output,sizeof(output),p_usb_ccid);
            print_rec(output,ret);
*/

            
            remote_len = ((controll_eg.remote_buffer[0]<<8)|(controll_eg.remote_buffer[1]&0x00FF));//indeed remote data length
            memcpy(&apud_data[sizeof(remote_idop_head) + 1],&controll_eg.remote_buffer,2); 
            memcpy(&apud_data[sizeof(remote_idop_head) + 1 + 2],&controll_eg.remote_buffer[34],remote_len);//0,1=len;2~33=ubus data; 34~=remote data
            
            print_send(apud_data,sizeof(remote_idop_head) + 1 + 2 + remote_len);
            ret = usb_transmit(context,apud_data,sizeof(remote_idop_head) + 1 + 2 + remote_len,output,sizeof(output),p_usb_ccid);
            print_rec(output,ret);
            

            memset(remote_open_rt_data,0,sizeof(remote_open_rt_data));
			memcpy(remote_open_rt_data,&controll_eg.remote_buffer[2],8);
            if(ret > 2){
                
                log_len = ret - 2;
                memcpy(log_data,output,log_len);
                ubus_client_process(UBUS_CLIENT_LOG,NULL,log_data,log_len);

                
                memcpy(remote_open_rt_data,&controll_eg.remote_buffer[2],8);
                if(output[124] == 0x01){//124 = result
                        remote_open_rt_data[8] = 0x90;
                        remote_open_rt_data[9] = 0x00;
                    }
                else{
                        remote_open_rt_data[8] = 0x00;
                        remote_open_rt_data[9] = output[124];
                    }
                    
                
            }
            else{
            
                    remote_open_rt_data[8] = output[ret - 2];
                    remote_open_rt_data[9] = output[ret - 1];
            
            }
            remote_open_rt_data[10] =  p_usb_ccid->ccid322_index;//only one 322 can return
            
            remote_open_rt_data[11] =  p_usb_ccid->ccid322_index;//only one 322 can return
           // memcpy(&remote_open_rt_data[11],&controll_eg.index_322,controll_eg.cnt_322);
            
            printf("\nsend ID open return\n");
            //print_send(remote_open_rt_data,11 + controll_eg.cnt_322);
            print_send(remote_open_rt_data,12);//modified by gxy 20171127
            //ubus_net_process(UBUS_CLIENT_RETURN,NULL,remote_open_rt_data,11 + controll_eg.cnt_322);
            ubus_net_process(UBUS_CLIENT_RETURN,NULL,remote_open_rt_data,12);


                
            osal_sem_release(p_usb_ccid->sem_state);
            
            osal_sem_release(controll_eg.sem_remote);
            break;
        case USB_FACE_REMOTE_OPEN:
            
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "FACE remote open\n");
            
            memcpy(apud_data,remote_faceop_head,sizeof(remote_faceop_head));
            apud_data[sizeof(remote_faceop_head)] =  0x00;//extend data
            
/*
            remote_len = ((controll_eg.remote_buffer[0]<<8)|(controll_eg.remote_buffer[1]&0x00FF));
            memcpy(&apud_data[sizeof(remote_idop_head) + 1],&controll_eg.remote_buffer,remote_len + 2); 
            
            print_send(apud_data,sizeof(remote_idop_head) + 1 + 2 + remote_len);
            ret = usb_transmit(context,apud_data,sizeof(remote_idop_head) + 1 + 2 + remote_len,output,sizeof(output),p_usb_ccid);
            print_rec(output,ret);
*/

            
            remote_len = ((controll_eg.remote_buffer[0]<<8)|(controll_eg.remote_buffer[1]&0x00FF));//indeed remote data length
            memcpy(&apud_data[sizeof(remote_faceop_head) + 1],&controll_eg.remote_buffer,2); 
            memcpy(&apud_data[sizeof(remote_faceop_head) + 1 + 2],&controll_eg.remote_buffer[34],remote_len);//0,1=len;2~33=ubus data; 34~=remote data
            
            print_send(apud_data,sizeof(remote_faceop_head) + 1 + 2 + remote_len);
            ret = usb_transmit(context,apud_data,sizeof(remote_faceop_head) + 1 + 2 + remote_len,output,sizeof(output),p_usb_ccid);
            print_rec(output,ret);
            

            memset(remote_open_rt_data,0,sizeof(remote_open_rt_data));
			
			memcpy(remote_open_rt_data,&controll_eg.remote_buffer[2],8);
            if(ret > 2){
                
                log_len = ret - 2;
                memcpy(log_data,output,log_len);
                ubus_client_process(UBUS_CLIENT_LOG,NULL,log_data,log_len);

                
                memcpy(remote_open_rt_data,&controll_eg.remote_buffer[2],8);
                if(output[96] == 0x01){//96 = result
                        remote_open_rt_data[8] = 0x90;
                        remote_open_rt_data[9] = 0x00;
                    }
                else{
                        remote_open_rt_data[8] = 0x00;
                        remote_open_rt_data[9] = output[96];
                    }
                    
                
            }
            else{
            
                    remote_open_rt_data[8] = output[ret - 2];
                    remote_open_rt_data[9] = output[ret - 1];
            
            }
            remote_open_rt_data[10] =  p_usb_ccid->ccid322_index;//only one 322 can return
            
            remote_open_rt_data[11] =  p_usb_ccid->ccid322_index;//only one 322 can return
           // memcpy(&remote_open_rt_data[11],&controll_eg.index_322,controll_eg.cnt_322);
            
            printf("\nFACE open return\n");
            //print_send(remote_open_rt_data,11 + controll_eg.cnt_322);
            print_send(remote_open_rt_data,12);//modified by gxy 20171127
            //ubus_net_process(UBUS_CLIENT_RETURN,NULL,remote_open_rt_data,11 + controll_eg.cnt_322);
            ubus_net_process(UBUS_CLIENT_RETURN,NULL,remote_open_rt_data,12);


                
            osal_sem_release(p_usb_ccid->sem_state);
            
            osal_sem_release(controll_eg.sem_remote);
            break;




     case USB_COMM_STATE_DEFAULT:
            break;

      case USB_COMM_CLEAR_ALARM:
            
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "alarm clear\n");
            
            memcpy(apud_data,alarm322_clear_head,sizeof(alarm322_clear_head));
            
            memcpy(&apud_data[sizeof(alarm322_clear_head) ],&controll_eg.alarm_clear[32],16); 
            
            print_send(apud_data,sizeof(alarm322_clear_head) + 16);
            ret = usb_transmit(context,apud_data,sizeof(alarm322_clear_head) + 16,output,sizeof(output),p_usb_ccid);
            print_rec(output,ret);
/*
            if(ret > 2){
                
                log_len = ret - 2;
                memcpy(log_data,output,log_len);
                ubus_client_process(UBUS_CLIENT_LOG,NULL,log_data,log_len);
            }
*/          
            memcpy(alarm_clear_rt_data,&controll_eg.alarm_clear,8);
            alarm_clear_rt_data[8] = output[ret - 2];
            alarm_clear_rt_data[9] = output[ret - 1];
            alarm_clear_rt_data[10] = p_usb_ccid->ccid322_index;
            
/*
            for(i = 0;i < MAX_322_NUM;i++ ){
            
                if(controll_eg.usb_ccid_322[i].ccid322_exist){
                    
                    alarm_clear_rt_data[11 + index] = controll_eg.usb_ccid_322[i].ccid322_index
                    index++;
                }
                
            
            }
*/
            if(controll_eg.alarm_clear[8] == 0x01){
                
                memcpy(&alarm_clear_rt_data[11],&controll_eg.index_322,controll_eg.cnt_322);
                printf("\nsend alarm clear return,only 322\n");
                print_send(alarm_clear_rt_data,11 + controll_eg.cnt_322);
                ubus_net_process(UBUS_CLIENT_RETURN,NULL,alarm_clear_rt_data,11 + controll_eg.cnt_322);


            }
            else if(controll_eg.alarm_clear[8] == 0x02){
                
                alarm_clear_rt_data[11] = 0xFF;
                memcpy(&alarm_clear_rt_data[12],&controll_eg.index_322,controll_eg.cnt_322);
                printf("\nsend alarm clear return,both 322 & 321\n");
                print_send(alarm_clear_rt_data,12 + controll_eg.cnt_322);
                ubus_net_process(UBUS_CLIENT_RETURN,NULL,alarm_clear_rt_data,12 + controll_eg.cnt_322);
            }
            osal_sem_release(p_usb_ccid->sem_state);
            
            break;

    case  USB_322_TEST:
        OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "322 usb reset\n");
        usb_transmit(context,reset_322usb,sizeof(reset_322usb),output,sizeof(output),p_usb_ccid);
        
        osal_sem_release(p_usb_ccid->sem_state);
        break;

    case  USB_NET_STATE:

        //net_state[sizeof(net_state) - 1] = (controll_eg.network_state << 7);

        if(controll_eg.network_state){

            net_state[sizeof(net_state) - 1] = 0x00;

        }
        else{

            net_state[sizeof(net_state) - 1] = 0x80;

        }

        OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "send net_state to pr11\n");
        print_send(net_state,sizeof(net_state));
        ret = usb_transmit(context,net_state,sizeof(net_state),output,sizeof(output),p_usb_ccid);
        print_rec(output,ret);
        
        if((controll_eg.network_state)&&(controll_eg.network_state_pre != controll_eg.network_state))
            zmq_socket_send(p_usb_ccid->zmq_client,id_reader_deal_OK,sizeof(id_reader_deal_OK));

		controll_eg.network_state_pre = controll_eg.network_state;
        osal_sem_release(p_usb_ccid->sem_state);
        break;

        default:
            break;
        }
        continue;
        }
    /***************POLL***************/
            
if(p_usb_ccid->pr11_exist == 1){
//          if(0xAA == p_usb_ccid->WG_ERROR){
//            
//              memset(output,0,sizeof(output));
//              OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "get pr11-d21 version:");
//              ret = usb_transmit(context,v_pr11_d21,sizeof(v_pr11_d21),&output[1],sizeof(output) - 1,p_usb_ccid);
//              output[0] = 0x04;
//              print_rec(output,ret + 1);
//              if(ret > 0){
//                  if(memcmp(confirm,&output[ret - 1],sizeof(confirm)) == 0){//ret~ret +1
//                      OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "clear WG ERROR FLAG\n");
//                      p_usb_ccid->WG_ERROR = 0;
//                  }
//                  else
//                      OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "bad pr11-d21 version:\n");
//              }


//          }
          //else
		  	{
            //controll_eg.WG_ERROR = 0;
            
            memset(output,0,sizeof(output));
            ret = usb_transmit(context,car_detect,sizeof(car_detect),output,sizeof(output),p_usb_ccid);

          
          	tail_check = check_card(p_usb_ccid,output,ret);

			
//			OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "check card! \n");
//			print_rec(output,ret);//打印寻卡结果
			
if(tail_check == 1){

    parse_tag = parse_data(output,ret,acl_data,&acl_len,p_usb_ccid);

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
                
                apud_data[sizeof(result_head)] = 0;//extend len

                wl_len =  17 + 1 + acl_data[271] + 232;//1+rtc(16)+name(l+v = 1+acl_data[271])+232

                apud_data[sizeof(result_head) + 1] = (0xFF00&wl_len)>>8;//extend len
                apud_data[sizeof(result_head) + 2] = (0x00FF&wl_len);//extend len

                //printf("data len is %d\n",apud_data[sizeof(result_head)]);
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "data len is %d\n",wl_len);

                memcpy(&apud_data[sizeof(result_head) + 3],acl_data,17);//result:1 + RTC:16

                memcpy(&apud_data[sizeof(result_head) + 3 + 17],&acl_data[271],1);//name-L:1
                
                memcpy(&apud_data[sizeof(result_head) + 3 + 17 + 1],&acl_data[272],acl_data[271]);//name-V:v

                memcpy(&apud_data[sizeof(result_head) + 3 + 17 + 1 + acl_data[271]],&acl_data[17],232);

                print_send(apud_data,sizeof(result_head) + 3 + 17 + 1 + acl_data[271] + 232);
                
                //ret = usb_transmit(context,open_door,sizeof(open_door),output,sizeof(output),p_usb_ccid);
                
                //print_send(test_o,195);
                //ret = usb_transmit(context,test_o,195,output,sizeof(output),p_usb_ccid);
                ret = usb_transmit(context,apud_data,sizeof(result_head) + 3 + 17 + 1 + acl_data[271] + 232,output,sizeof(output),p_usb_ccid);

            }           

            printf("log return\n");
            print_rec(output,ret);
            if(ret > 2){

				if((ret == 3)&&(memcmp(output,face_req,sizeof(face_req)) == 0)){

					
					ubus_net_process(UBUS_CLIENT_SEND_CAM322ID,NULL,p_usb_ccid->pid_322,4);

				}
				else{
					
	                log_len = ret - 2;
	                memcpy(log_data,output,log_len);
	                
					/*
	                gettimeofday( &_start, NULL );
	                printf("start : %d.%d\n", _start.tv_sec, _start.tv_usec);
					*/
	                
	                ubus_client_process(UBUS_CLIENT_LOG,NULL,log_data,log_len);


	                if(output[106] == 0x01)//open success
	                {
	                
	                    controll_eg.alarm_flag = 0xAA;

	                    OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "begin to block alarm\n");

	                    osal_timer_change(controll_eg.timer_alarm,6000);// 6 sec

	                    osal_timer_start(controll_eg.timer_alarm);


	                }
	/*
	                gettimeofday( &_end, NULL );
	                printf("start : %d.%d\n", _end.tv_sec, _end.tv_usec);
	*/
	                
	            }
            }
			else{

				OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "\nopen door failed and no-log\n");
				//StatisticsInfo_push(MAINT_SRC_322,p_usb_ccid->pid_322 ,log_failed,0);
				memset(info_statistic,0,sizeof(info_statistic));
				info_statistic[0] = MAINT_SRC_322;//from 322
				memcpy(&info_statistic[1],p_usb_ccid->pid_322,4);//322 id 4
				memcpy(&info_statistic[5],log_failed,strlen(log_failed));
				memcpy(&info_statistic[37],output,ret);
				ubus_net_process(UBUS_CLIENT_SEND_StatisticsInfo,NULL,info_statistic,sizeof(info_statistic));

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

        case CARDPOLLEVENT_POWER:
/*
            ret = usb_transmit(context, controll_eg.p2pkey.data, controll_eg.p2pkey.len, output, sizeof(output),3000);
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_WARN, "update p2pkey,ret = %d\n",ret);
            ret = usb_transmit(context, controll_eg.mackey.data, controll_eg.mackey.len, output, sizeof(output),3000);
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_WARN, "update mackey,ret = %d\n",ret);
*/
            break;

       case CARDPOLLEVENT_FACE:
			ubus_net_process(UBUS_CLIENT_SEND_CAM322ID,NULL,p_usb_ccid->pid_322,4);
            break;

       case CARDPOLLEVENT_ID:
            zmq_socket_send(p_usb_ccid->zmq_client,id_reader,sizeof(id_reader));
            break;
            
        default:
            break;    

    }

}
else if(tail_check == 2){


    if((p_usb_ccid->rtc_sync == 0xAA)&&(p_usb_ccid->alarm_period == 0xAA)){
        
        get_rtc_data(rtc);
        p_usb_ccid->alarm_period = 0;
        OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "\nget 322 alarm,send rtc encrypt:\n");


        memcpy(apud_data,alarm322_head,sizeof(alarm322_head));
        memcpy(&apud_data[sizeof(alarm322_head)],rtc,sizeof(rtc));
        print_send(apud_data,sizeof(alarm322_head) + sizeof(rtc));
        ret = usb_transmit(context,apud_data,sizeof(alarm322_head) + sizeof(rtc),output,sizeof(output),p_usb_ccid);


        OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "\nget 322 alarm log:\n");

        print_rec(output,ret);

  
        
            if(controll_eg.alarm_flag == 0xAA){
                
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "open door,drop alarm!\n");
                
            }
            else{
                
                if(controll_eg.alarm_flag == 0){
                    
/*
                    OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "send alarm to server!\n");
                    ubus_net_process(UBUS_CLIENT_SEND_ALARM,NULL,output,ret - 2);
*/
                    OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "send alarm to 321!\n");
					ubus_client_process(UBUS_CLIENT_LOG,NULL,output,ret - 2);
                    
                }
            }

        
        }
    }
else if(tail_check == -4 ){


        print_send(wgp_info_get,sizeof(wgp_info_get));
        ret = usb_transmit(context,wgp_info_get,sizeof(wgp_info_get),output,sizeof(output),p_usb_ccid);
        printf("wgp info:\n");
        print_rec(output,ret);
        if(ret == 22){
            
            if((controll_eg.push_flag & 0x01) == false){
                
                StatisticsInfo_push(MAINT_SRC_322,p_usb_ccid->pid_322,wgp_alarm,NULL);
                StatisticsInfo_push(MAINT_SRC_322,p_usb_ccid->pid_322,wgp_host_num,output);
                StatisticsInfo_push(MAINT_SRC_322,p_usb_ccid->pid_322,wgp_slave_num,&output[4]);
                StatisticsInfo_push(MAINT_SRC_322,p_usb_ccid->pid_322,wgp_num,&output[8]);          
                StatisticsInfo_push(MAINT_SRC_322,p_usb_ccid->pid_322,wgp_cnt,&output[12]);
                StatisticsInfo_push(MAINT_SRC_322,p_usb_ccid->pid_322,wgp_time,&output[16]);

                
                controll_eg.push_flag |= 0x01;
            }
            else
                printf("\ndo not push wgp info\n");

                p_usb_ccid->WG_ERROR = 0xAA;
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "WG ERROR FLAG UP!\n");
        }

/*
        print_send(wgp_info_clear,sizeof(wgp_info_clear));
        ret = usb_transmit(context,wgp_info_clear,sizeof(wgp_info_clear),output,sizeof(output),p_usb_ccid);
        printf("wgp clear:\n");
        print_rec(output,ret);
*/


}
}

          /***************POLL  END***************/
          
          //printf("F[%s] L[%d] ptr is NULL!!!\n", __FILE__, __LINE__);

        rec_zmq = zmq_recv(p_usb_ccid->zmq_answer,zmq_ans,sizeof(zmq_ans),ZMQ_DONTWAIT);

        if(rec_zmq <= 0){

            
            //printf("no answer\n");


        }
        else{

            printf("get zmq answer\n");

            print_rec(zmq_ans,rec_zmq);

            if(rec_zmq > 14)
                sys_add_event_queue(&controll_eg.msg_manager,ZMQ_RESULT,0,p_usb_ccid->ccid322_index,NULL);
            
            
            printf("switch value is %d\n",p_usb_ccid->toggle_state);
        }
}
        msleep(5);
#else
sleep(2);
#endif
}




out: 
		
		//sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_SEND_CE,0,0,NULL);
		
        luareader_disconnect(context);

        luareader_term(context);

        osal_sem_delete(p_usb_ccid->sem_322);

        osal_sem_delete(p_usb_ccid->sem_state);

        osal_timer_delete(p_usb_ccid->timer_322);

        p_usb_ccid->pr11_exist = 0;

        controll_eg.cnt_322--;

        OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "thread exit!\n");

        printf("322 thread num is %d\n",controll_eg.cnt_322);
		sleep(1);

		return NULL;
}



uint8_t usb_wb;

void timer_usb_callback(void* parameter)
{
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
            p_usb_timer->alarm_period = 0xAA;

        }
   // }
//test usb interrupt
   //sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_322_USBTEST,0,p_usb_timer->ccid322_index,NULL);
}


void timer_alarm_callback(void* parameter)
{
    Controller_t *p_ctl = (Controller_t *)parameter;

    p_ctl->alarm_flag = 0;
    

    printf("\nend to block alarm\n");
    

}
//0-wgp 1-antenna 2-ic broken
void timer_push_callback(void* parameter)
{
    Controller_t *p_ctl = (Controller_t *)parameter;

/*
    if(p_ctl->push_flag & 0x01 ==  TRUE)//0001
        {}
    else if(p_ctl->push_flag & 0x02 ==  TRUE)//0010
        {}
    else if(p_ctl->push_flag & 0x04 ==  TRUE)//0100
        {}
*/
    p_ctl->push_flag = 0;
    printf("\nreset push flag\n");
    

}


void eg_usbto322_init(void)
{

    osal_task_t *tid;

	
	unsigned char output[1024] = {0};
    char device_str[MAX_322_NUM][32] = {0};
	int ret;
	int i = 0,j = 0,dev_index = 0;

    usb_ccid_322_t *p_usb_ccid;

    
	void * context = luareader_new(0, NULL, NULL);


    ret = luareader_get_list(context, (char *)output, sizeof(output));

    //OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "there are %d 322 on pc02\n",ret);

    
    for(i = 0;i < ret - 1;i++){

        if(dev_index >= MAX_322_NUM + 1){//MAX_322_NUM = 4

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

    sem_alarm = osal_sem_create("sem_alarm", 1);
    osal_assert(sem_alarm != NULL);

    controll_eg.sem_remote = osal_sem_create("sem_remote", 1);
    osal_assert(controll_eg.sem_remote != NULL);

    controll_eg.sem_base_cfg = osal_sem_create("sem_base_cfg", 1);
    osal_assert(controll_eg.sem_base_cfg != NULL);

    controll_eg.sem_ctrl_cfg = osal_sem_create("sem_ctrl_cfg", 1);
    osal_assert(controll_eg.sem_ctrl_cfg != NULL);

    controll_eg.timer_alarm = osal_timer_create("alarm timer",timer_alarm_callback,p_controll_eg,\
                        EUSB_SEND_PERIOD, TIMER_ONESHOT|TIMER_STOPPED, TIMER_PRIO_NORMAL);
    osal_assert(controll_eg.timer_alarm != NULL);

    controll_eg.timer_push_statics = osal_timer_create("alarm push",timer_push_callback,p_controll_eg,\
                        600*EUSB_SEND_PERIOD, TIMER_INTERVAL|TIMER_STOPPED, TIMER_PRIO_NORMAL);
    osal_assert(controll_eg.timer_push_statics != NULL);

    osal_timer_start(controll_eg.timer_push_statics);

    controll_eg.network_state = net_state_read();

    if(controll_eg.network_state)
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "\ninit----network online\n");
    else
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "\ninit----network offline\n");
    
    for(i = 0;i < dev_index;i++){

        
        ret = alloc_322_index(device_str[i]);
        //printf("ret is %d\n",ret);
        if(ret == 0)
            continue;
        controll_eg.cnt_322++;
        p_usb_ccid = &(controll_eg.usb_ccid_322[ret]);
        p_usb_ccid->usb_state = USB_COMM_STATE_DEFAULT;//USB_COMM_STATE_INIT;//USB_COMM_STATE_INIT_END;//USB_COMM_STATE_INIT;
        p_usb_ccid->ccid322_index = ret;
        p_usb_ccid->toggle_state = 0;
        p_usb_ccid->ccid322_exist = 1;
        p_usb_ccid->pr11_exist = 1;//defualt 1,has a reader
        p_usb_ccid->init_flag = 0;
        p_usb_ccid->rtc_sync = 0;//RTC sync
        p_usb_ccid->now_door_state[0] = 0x01;
        p_usb_ccid->now_door_state[1] = 0x02;
        p_usb_ccid->pre_door_state[0] = 0x01;
        p_usb_ccid->pre_door_state[1] = 0x02;//?????? ???
        
        strcpy(p_usb_ccid->usb_port,device_str[i]);

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

        p_usb_ccid->sem_zmq = osal_sem_create(device_str[i], 1);
        osal_assert(p_usb_ccid->sem_zmq != NULL);
        
        tid = osal_task_create(device_str[i],
                            eg_usb_thread_entry,
                            p_usb_ccid,PC02_USB__THREAD_STACK_SIZE, PC02_USB_THREAD_PRIORITY);
        
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
