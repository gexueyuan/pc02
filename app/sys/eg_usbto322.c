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
#include "tlv_box.h"


#define USB_FAILED    0x6E00
#define USB_OK        0x9000 
#define EUSB_SEND_PERIOD  6000 



#define VENDOR_ID 0x1780  
#define PRODUCT_ID 0x0312

#define USE_TIMESTAMP
#define USB_TEST

#define NET_STATE "/tmp/pc02_connect"

extern   void get_wl(uint8_t *id_lv,uint8_t *data_wl,int *wlen);
extern   void get_wl_4(uint8_t *id_lv,uint8_t *data_wl,int *wlen);
extern   int get_audit_data(unsigned int tag,unsigned char* strhex,int strlen,uint8_t *data_wl,int *wlen);

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


const unsigned char check[] = {0x00,0xA4,0x04,0x00,0x00};//+len+data

const uint8_t car_detect[] = {0xFC,0xA0,0x00,0x00,0x05,0x80,0x30,0x00,0x00,0x00};

unsigned char  change_app[25] = {0xFC,0xA0,0x04,0x00,0x14,0x00,0xA4,0x04,0x00,0x0F,0x74,0x64,0x72,0x6F,0x6E,0x99,0x05,0x69,0x64,0x52,0x64,0x51,0x34,0x42,0x31};

E_CARDPOLLEVENT card_event;

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
const uint8_t ce_log[] = {0x80,0xCA,0xCE,0x43,0x00};

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
const uint8_t v_pr02_d21[] = {0x00,0x26,0x05,0x81,0x00};
const uint8_t v_pr11_d21[] = {0x00,0x26,0x02,0x81,0x00};

const uint8_t rd_p2pkey[] = {0x00,0xAA,0x00,0x00, 0x00};
const uint8_t rd_pr11_p2pkey[] = {0xFC, 0xA0, 0x00, 0x00, 0x05,0x00,0xAA,0x00,0x00, 0x00};

const uint8_t confirm[] = {0x90,0x00};

const uint8_t _end_confirm[] = {0x90,0x00,0x90,0x00};
const uint8_t _end_alarm[] = {0x90,0x00,0x90,0x0A};

const uint8_t reset_322usb[] = {0xF0,0xF0,0x02,0xFF,0x00};


osal_sem_t *sem_pr11ce;

osal_sem_t *sem_322ce;

osal_sem_t *sem_logce;

osal_sem_t *sem_ctrlinfo;

osal_sem_t *sem_alarm;

const uint8_t open_door[] = {0x80,0xDD,0x33,0x00,0x03,0x01,0x00,0x02};
                                     /*0*/  /*1*/  /*2*/  /*3*/ 
const char* usb_port_def[MAX_322_NUM] = {"1-1.2","1-1.3","1-1.4","1-1.5"};

 uint8_t id_reader[] = {0x00,0x01,0x00,0x02,0x01,0x01};

const uint8_t id_reader_deal_OK[] = {0x00,0x01,0x00,0x02,0x01,0x03};

const uint8_t id_info_get[] = {0x00,0xc2,0x00,0x00,0x00};

const uint8_t id_cmd_1[] = {0x80,0xCA,0xCE,0x4E,0x4,0x0,0x0,0x0,0xD0};

const uint8_t wgp_info_get[] = {0xF0,0xF2,0x01,0xFD,0x00,0x00};

const uint8_t wgp_info_clear[] = {0xF0,0xF2,0x01,0xFE,0x01,0xFF};

const uint8_t push_rtc[] = {0x80,0x19,0x00,0x00,0x10};
const uint8_t get_slow_log[] = {0xF0,0xF3,0x00,0x00,0x00};

uint8_t net_state[] = {0xFC,0xA0,0x00,0x00,0x09,0x80,0x34,0x00,0x00,0x04,0x09,0x00,0x01,0x80};


/* 统计信息 GBK*/
char *antenna_broken = "PR11天线损坏";//{0x50,0x52,0x31,0x31,0xBE,0xFC,0xC3,0xDC,0xD0,0xBE,0xC6,0xAC,0xCB,0xF0,0xBB,0xB5};
char *IC_broken = "PR11军密芯片损坏";//{0xBE,0xFC,0xC3,0xDC,0xD0,0xBE,0xC6,0xAC,0xCB,0xF0,0xBB,0xB5};
char *usb_disconnect = "PC02-322USB通讯中断";
char *log_failed = "开门失败无日志";
char *wgp_alarm = "WG+统计信息";
char *wgp_host_num = "WGP通讯控制器重发计数";
char *wgp_slave_num = "WGP通讯读卡器重发计数";
char *wgp_num = "WGP通讯控制器复位计数";
char *wgp_cnt = "WGP通讯两线通信次数";
char *wgp_time="WGP通讯开机时间（秒）";
char *_322_error="322 返回错误";
char *time_consuming = "开门指令耗时统计";
char *id0 = "寻卡开始时间";
char *id1 = "selectCmdToCard开始时间";
char *id2 = "E02C指令开始时间";
char *id3 = "beep开始时间";
char *id4 = "返cardID给9531的时间";
char *id5 = "绿灯开始时间";
char *id6 = "开门成功结束时间";
char *whitelist_resume = "白名单续传失败";
char *period_9531 = "9531指令周期间隔";
char *card_event_clear = "卡事件清除事件";
char *card_info = "M卡电压电量";
char *pr11_v = "Pr11电压超载异常";
char *pr11_c = "Pr11电流超载异常";
char *pr11_tp = "Pr11温度超载异常";
char *info_322 = "322响应数据域";

/* 统计信息 GBK*/
const char*  zmq_send_str = "send to zmq";


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


int file_exist(const char* _fileName)
{

    if((access(_fileName,F_OK))!=-1)   
    {   
        printf("\nfile : %s exist.\n",_fileName);
        return 1;
    }
    else{
        
        printf("\nfile : %s  Non-existent.\n",_fileName);
        return 0;

    }



}

int net_state_read(void)
{

    return file_exist(NET_STATE);

}

void print_array(const  char* tag,const unsigned char* send,int len)
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
        memcpy(&info_statistic[37],value,4);//value == null,clear 0
    print_array(gbk_str,info_statistic,sizeof(info_statistic));
    ubus_net_process(UBUS_CLIENT_SEND_StatisticsInfo,NULL,info_statistic,sizeof(info_statistic));


}

void StatisticsInfo_push_n(uint8_t src_dev,uint8_t *pid_dev,const char *gbk_str,uint8_t* value,int length)
{
    //uint8_t info_statistic[41] = {0};
    int len = 37 + length;
    uint8_t* info_statistic = (uint8_t*)osal_malloc(len);
	
    memset(info_statistic,0,len);
    info_statistic[0] = src_dev;//from pr11
    memcpy(&info_statistic[1],pid_dev,4);//pr11 id 4
    memcpy(&info_statistic[5],gbk_str,strlen(gbk_str));//32 gbk
    if(value != NULL)
        memcpy(&info_statistic[37],value,length);//value == null,clear 0
    print_array(gbk_str,info_statistic,len);
    ubus_net_process(UBUS_CLIENT_SEND_StatisticsInfo,NULL,info_statistic,len);
	osal_free(info_statistic);

}

int usb_transmit_detect(void **context, const unsigned char * apdu,
            int apdu_len, unsigned char * resp, int max_resp_size,usb_ccid_322_t **usb_322)
{

    int ret = 0;
    int connect_ret = 0;

    int error;

	unsigned char output[64] = {0};

	void * context_local = *context;
	usb_ccid_322_t *usb_322_local = *usb_322;
    /* Take the semaphore. */
    if(osal_sem_take(usb_322_local->sem_322, OSAL_WAITING_FOREVER) != OSAL_EOK)
    {
       printf("\n%s Semaphore return failed. \n",usb_322_local->usb_port);
       return 0;
    }

    ret = luareader_transmit(context_local, apdu, apdu_len, resp, max_resp_size,1000);


    if(ret < 0){
    
        printf("transmit return is %d\n",ret);
        memset(output, 0, sizeof(output));
        error = luareader_pop_value(context_local, (char *)output, sizeof(output));
        printf("%s-luareader_pop_value(%p)=%d(%s)\n",usb_322_local->usb_port,context_local, error, output);

        printf("reconnect\n");
		
        connect_ret = luareader_disconnect(context_local);
        
        if(connect_ret < 0){
            
            OSAL_MODULE_DBGPRT(usb_322_local->usb_port, OSAL_DEBUG_WARN, "disconnect failed\n");

			return ret;
        }
        
        luareader_term(context_local);
        msleep(200);
        context_local = luareader_new(0, NULL, NULL);
		(*usb_322)->usb_context = &context_local;
		*context = context_local;
        connect_ret = luareader_connect(context_local,usb_322_local->usb_port);

        if(connect_ret < 0){

            
            OSAL_MODULE_DBGPRT(usb_322_local->usb_port, OSAL_DEBUG_WARN, "connect failed\n");
			
        }
        else{

            OSAL_MODULE_DBGPRT(usb_322_local->usb_port, OSAL_DEBUG_WARN, "reconnect succeed!\n");
			
			ret = luareader_transmit(context_local, apdu, apdu_len, resp, max_resp_size,1000);
            
        }

    	}
    osal_sem_release(usb_322_local->sem_322);
    return ret;
}



int usb_transmit(void *context, const unsigned char * apdu,
            int apdu_len, unsigned char * resp, int max_resp_size,usb_ccid_322_t *usb_322)
{

    int ret = 0;

    int connect_ret = 0;

    int error;

	unsigned char output[64] = {0};
	/**test**/
	 struct timeval _start,_end;
	 long time_in_us,time_in_ms;
	/**test**/

    /* Take the semaphore. */
    if(osal_sem_take(usb_322->sem_322, OSAL_WAITING_FOREVER) != OSAL_EOK)
    {
       printf("\n%s Semaphore return failed. \n",usb_322->usb_port);
       return 0;
    }

    //print_array(usb_322->usb_port, apdu, apdu_len);
	
	gettimeofday( &_start, NULL );
    ret = luareader_transmit(context, apdu, apdu_len, resp, max_resp_size,2000);
    //if(ret > 0)
    	//print_array(usb_322->usb_port, resp, ret);
	gettimeofday( &_end, NULL );
	time_in_us = (_end.tv_sec - _start.tv_sec) * 1000000 + _end.tv_usec - _start.tv_usec;	
	time_in_ms = time_in_us/1000;
	if(time_in_ms > 1000){
		osal_printf("F[%s] L[%d],usb overtime ,%ldms\n",__func__, __LINE__,time_in_ms);
		log_message("usb",3,"F[%s] L[%d],usb overtime ,%ldms\n",__func__, __LINE__,time_in_ms);
	}

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
        
        //luareader_term(usb_322->usb_context);
        msleep(300);

        //usb_322->usb_context = luareader_new(0, NULL, NULL);
        connect_ret = luareader_connect(usb_322->usb_context,usb_322->usb_port);

        if(connect_ret < 0){

            usb_322->usb_reconnect_cnt++;
            
            OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_WARN, "connect failed\n");
			
			StatisticsInfo_push(MAINT_SRC_322,usb_322->pid_322,usb_disconnect,0);
            if(usb_322->usb_reconnect_cnt >= 10){
                OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_WARN, "connect failed 10 times,reboot!!!\n");
                system("reboot");
            }
        }
        else{

            //msleep(200);
            OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_WARN, "reconnect succeed!\n");
            
            usb_322->usb_reconnect_cnt = 0; 
            
            //ret = usb_transmit(context,controll_eg.p2pkey.data,controll_eg.p2pkey.len,output,sizeof(output),p_usb_ccid);
            ret = luareader_transmit(usb_322->usb_context, controll_eg.p2pkey.data, controll_eg.p2pkey.len, output, sizeof(output),3000);
            OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_WARN, "resend p2pkey\n");			
			print_array(usb_322->usb_port, controll_eg.p2pkey.data, controll_eg.p2pkey.len);
            ret = luareader_transmit(usb_322->usb_context, controll_eg.mackey.data, controll_eg.mackey.len, output, sizeof(output),3000);
            OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_WARN, "resend mackey\n");
			print_array(usb_322->usb_port, controll_eg.mackey.data, controll_eg.mackey.len);
        }
 
    }
    else{

       usb_322->usb_reconnect_cnt = 0; 

    }
    osal_sem_release(usb_322->sem_322);
    return ret;


}
int usb_transmit_fix(void **context, const unsigned char * apdu,
            int apdu_len, unsigned char * resp, int max_resp_size,usb_ccid_322_t **usb_322)
{

    int ret = 0;

    int connect_ret = 0;

    int error;

	unsigned char output[64] = {0};
	/**test**/
	 struct timeval _start,_end;
	 long time_in_us,time_in_ms;
	/**test**/

	void * context_local = *context;
	usb_ccid_322_t *usb_322_local = *usb_322;
    /* Take the semaphore. */
    if(osal_sem_take(usb_322_local->sem_322, OSAL_WAITING_FOREVER) != OSAL_EOK)
    {
       printf("\n%s Semaphore return failed. \n",usb_322_local->usb_port);
       return 0;
    }

    //print_array(usb_322_local->usb_port, apdu, apdu_len);
	
	gettimeofday( &_start, NULL );
    ret = luareader_transmit(context_local, apdu, apdu_len, resp, max_resp_size,8000);
    //if(ret > 0)
    	//print_array(usb_322_local->usb_port, resp, ret);
	gettimeofday( &_end, NULL );
	time_in_us = (_end.tv_sec - _start.tv_sec) * 1000000 + _end.tv_usec - _start.tv_usec;	
	time_in_ms = time_in_us/1000;
	if(time_in_ms > 1000){
		osal_printf("F[%s] L[%d],usb overtime ,%ldms,apdu:\n",__func__, __LINE__,time_in_ms);
		log_message("usb",3,"F[%s] L[%d],usb overtime ,%ldms\n",__func__, __LINE__,time_in_ms);
		print_array(usb_322_local->usb_port, apdu, apdu_len);
	}

    if(ret < 0){
    
        printf("transmit return is %d\n",ret);
        memset(output, 0, sizeof(output));
        error = luareader_pop_value(context_local, (char *)output, sizeof(output));
        printf("%s-luareader_pop_value(%p)=%d(%s)\n",usb_322_local->usb_port,context_local, error, output);

        printf("reconnect\n");

        connect_ret = luareader_disconnect(context_local);
        
        if(connect_ret < 0){
            
            OSAL_MODULE_DBGPRT(usb_322_local->usb_port, OSAL_DEBUG_WARN, "disconnect failed\n");
        }
        
        luareader_term(context_local);
        msleep(300);
		
        context_local = luareader_new(0, NULL, NULL);
		(*usb_322)->usb_context = &context_local;
		*context = context_local;
        connect_ret = luareader_connect(context_local,usb_322_local->usb_port);

        if(connect_ret < 0){

            usb_322_local->usb_reconnect_cnt++;
            
            OSAL_MODULE_DBGPRT(usb_322_local->usb_port, OSAL_DEBUG_WARN, "connect failed\n");
			
			StatisticsInfo_push(MAINT_SRC_322,usb_322_local->pid_322,usb_disconnect,0);
            if(usb_322_local->usb_reconnect_cnt >= 100){
                OSAL_MODULE_DBGPRT(usb_322_local->usb_port, OSAL_DEBUG_WARN, "connect failed 100 times,reboot!!!\n");
                //system("reboot");
                usb_322_local->usb_reconnect_cnt = 0;
            }
        }
        else{

            //msleep(200);
            OSAL_MODULE_DBGPRT(usb_322_local->usb_port, OSAL_DEBUG_WARN, "reconnect succeed!\n");
            
            usb_322_local->usb_reconnect_cnt = 0; 
            
            //ret = usb_transmit(context,controll_eg.p2pkey.data,controll_eg.p2pkey.len,output,sizeof(output),p_usb_ccid);
            ret = luareader_transmit(context_local, controll_eg.p2pkey.data, controll_eg.p2pkey.len, output, sizeof(output),3000);
            OSAL_MODULE_DBGPRT(context_local, OSAL_DEBUG_WARN, "resend p2pkey\n");			
			print_array(usb_322_local->usb_port, controll_eg.p2pkey.data, controll_eg.p2pkey.len);
			if(controll_eg.mackey.len != 0){
	            ret = luareader_transmit(context_local, controll_eg.mackey.data, controll_eg.mackey.len, output, sizeof(output),3000);
	            OSAL_MODULE_DBGPRT(usb_322_local->usb_port, OSAL_DEBUG_WARN, "resend mackey\n");
				print_array(usb_322_local->usb_port, controll_eg.mackey.data, controll_eg.mackey.len);
			}
			
			zmq_socket_send(usb_322_local->zmq_client,id_reader_deal_OK,sizeof(id_reader_deal_OK));
        }
 
    }
    else{

       usb_322_local->usb_reconnect_cnt = 0; 

    }
    osal_sem_release(usb_322_local->sem_322);
    return ret;


}
int whitelist_transmit(void *context, const unsigned char * apdu,
            int apdu_len, unsigned char * resp, int max_resp_size,usb_ccid_322_t *usb_322)
{

    int ret = 0;

    int connect_ret = 0;

    int error;

	unsigned char output[64] = {0};
	/**test**/
	 struct timeval _start,_end,_re;
	 long time_in_us,time_in_ms;
	/**test**/

    /* Take the semaphore. */
    if(osal_sem_take(usb_322->sem_322, OSAL_WAITING_FOREVER) != OSAL_EOK)
    {
       printf("\n%s Semaphore return failed. \n",usb_322->usb_port);
       return 0;
    }

    //print_array(usb_322->usb_port, apdu, apdu_len);
	
	gettimeofday( &_start, NULL );
    ret = luareader_transmit(context, apdu, apdu_len, resp, max_resp_size,2000);
    //if(ret > 0)
    	//print_array(usb_322->usb_port, resp, ret);
	gettimeofday( &_end, NULL );
	time_in_us = (_end.tv_sec - _start.tv_sec) * 1000000 + _end.tv_usec - _start.tv_usec;	
	time_in_ms = time_in_us/1000;
	if(time_in_ms > 1000){
		osal_printf("F[%s] L[%d],usb overtime ,%ldms\n",__func__, __LINE__,time_in_ms);
		log_message("usb",3,"F[%s] L[%d],usb overtime ,%ldms\n",__func__, __LINE__,time_in_ms);
	}

    if(ret < 0){
    
        printf("transmit return is %d\n",ret);
        memset(output, 0, sizeof(output));
        error = luareader_pop_value(context, (char *)output, sizeof(output));
        printf("%s-luareader_pop_value(%p)=%d(%s)\n",usb_322->usb_port,context, error, output);

        printf("reconnect\n");
        msleep(300);
        do{
        connect_ret = luareader_disconnect(usb_322->usb_context);
        
        if(connect_ret < 0){
            
            OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_WARN, "disconnect failed\n");
        }
        

            gettimeofday( &_re, NULL );
            connect_ret = luareader_connect(usb_322->usb_context,usb_322->usb_port);
            
            msleep(300);
            if(connect_ret < 0){

                usb_322->usb_reconnect_cnt++;
                
                OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_WARN, "connect failed\n");
    			
    			StatisticsInfo_push(MAINT_SRC_322,usb_322->pid_322,usb_disconnect,0);
                if(usb_322->usb_reconnect_cnt >= 10){
                    OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_WARN, "connect failed 10 times,reboot!!!\n");
                }
            }
            else{

                //msleep(200);
                OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_WARN, "reconnect succeed!\n");
                
                usb_322->usb_reconnect_cnt = 0; 
                
                //ret = usb_transmit(context,controll_eg.p2pkey.data,controll_eg.p2pkey.len,output,sizeof(output),p_usb_ccid);
                ret = luareader_transmit(context, controll_eg.p2pkey.data, controll_eg.p2pkey.len, output, sizeof(output),3000);
                OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_WARN, "resend p2pkey\n");			
    			print_array(usb_322->usb_port, controll_eg.p2pkey.data, controll_eg.p2pkey.len);
                ret = luareader_transmit(context, controll_eg.mackey.data, controll_eg.mackey.len, output, sizeof(output),3000);
                OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_WARN, "resend mackey\n");
    			print_array(usb_322->usb_port, controll_eg.mackey.data, controll_eg.mackey.len);

                ret = luareader_transmit(context, apdu, apdu_len, resp, max_resp_size,2000);
                OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_WARN, "resend whitelist\n");
    			print_array(usb_322->usb_port, apdu, apdu_len);
               
            }
           	time_in_us = (_re.tv_sec - _end.tv_sec) * 1000000 + _re.tv_usec - _end.tv_usec;	
	        time_in_ms = time_in_us/1000; 
        }while((!usb_322->usb_reconnect_cnt)&&(time_in_ms <= 2000));
        if(usb_322->usb_reconnect_cnt){

            StatisticsInfo_push(MAINT_SRC_322,usb_322->pid_322,whitelist_resume,NULL);
            
        }
    }
    else{

       usb_322->usb_reconnect_cnt = 0; 

    }
    //}
    
    osal_sem_release(usb_322->sem_322);
    return ret;


}
int whitelist_transmit_fix(void **context, const unsigned char * apdu,
            int apdu_len, unsigned char * resp, int max_resp_size,usb_ccid_322_t **usb_322)
{

    int ret = 0;

    int connect_ret = 0;

    int error;

	unsigned char output[64] = {0};
	/**test**/
	 struct timeval _start,_end,_re;
	 long time_in_us,time_in_ms;
	/**test**/

	void * context_local = *context;
	usb_ccid_322_t *usb_322_local = *usb_322;
    /* Take the semaphore. */
    if(osal_sem_take(usb_322_local->sem_322, OSAL_WAITING_FOREVER) != OSAL_EOK)
    {
       printf("\n%s Semaphore return failed. \n",usb_322_local->usb_port);
       return 0;
    }

    //print_array(usb_322->usb_port, apdu, apdu_len);
	
	gettimeofday( &_start, NULL );
    ret = luareader_transmit(context_local, apdu, apdu_len, resp, max_resp_size,2000);
    //if(ret > 0)
    	//print_array(usb_322->usb_port, resp, ret);
	gettimeofday( &_end, NULL );
	time_in_us = (_end.tv_sec - _start.tv_sec) * 1000000 + _end.tv_usec - _start.tv_usec;	
	time_in_ms = time_in_us/1000;
	if(time_in_ms > 1000){
		osal_printf("F[%s] L[%d],usb overtime ,%ldms\n",__func__, __LINE__,time_in_ms);
		log_message("usb",3,"F[%s] L[%d],usb overtime ,%ldms\n",__func__, __LINE__,time_in_ms);
	}

    if(ret < 0){
    
        printf("transmit return is %d\n",ret);
        memset(output, 0, sizeof(output));
        error = luareader_pop_value(context_local, (char *)output, sizeof(output));
        printf("%s-luareader_pop_value(%p)=%d(%s)\n",usb_322_local->usb_port,context_local, error, output);

        printf("reconnect\n");
		do{
        connect_ret = luareader_disconnect(context_local);
        
        if(connect_ret < 0){
            
            OSAL_MODULE_DBGPRT(usb_322_local->usb_port, OSAL_DEBUG_WARN, "disconnect failed\n");
        }
        
        luareader_term(context_local);
        msleep(30);
		gettimeofday( &_re, NULL );
        context_local = luareader_new(0, NULL, NULL);
		(*usb_322)->usb_context = &context_local;
		*context = context_local;
        connect_ret = luareader_connect(context_local,usb_322_local->usb_port);

        if(connect_ret < 0){

            usb_322_local->usb_reconnect_cnt++;
            
            OSAL_MODULE_DBGPRT(usb_322_local->usb_port, OSAL_DEBUG_WARN, "connect failed\n");
			
			StatisticsInfo_push(MAINT_SRC_322,usb_322_local->pid_322,usb_disconnect,0);
            if(usb_322_local->usb_reconnect_cnt >= 200){
                OSAL_MODULE_DBGPRT(usb_322_local->usb_port, OSAL_DEBUG_WARN, "connect failed 200 times,reboot!!!\n");
                //system("reboot");
            }
        }
        else{

            //msleep(200);
            OSAL_MODULE_DBGPRT(usb_322_local->usb_port, OSAL_DEBUG_WARN, "reconnect succeed!\n");
            
            usb_322_local->usb_reconnect_cnt = 0; 
            
            //ret = usb_transmit(context,controll_eg.p2pkey.data,controll_eg.p2pkey.len,output,sizeof(output),p_usb_ccid);
            ret = luareader_transmit(context_local, controll_eg.p2pkey.data, controll_eg.p2pkey.len, output, sizeof(output),3000);
            OSAL_MODULE_DBGPRT(context_local, OSAL_DEBUG_WARN, "resend p2pkey\n");			
			print_array(usb_322_local->usb_port, controll_eg.p2pkey.data, controll_eg.p2pkey.len);
			if(controll_eg.mackey.len != 0){
	            ret = luareader_transmit(context_local, controll_eg.mackey.data, controll_eg.mackey.len, output, sizeof(output),3000);
	            OSAL_MODULE_DBGPRT(usb_322_local->usb_port, OSAL_DEBUG_WARN, "resend mackey\n");
				print_array(usb_322_local->usb_port, controll_eg.mackey.data, controll_eg.mackey.len);
			}
		    ret = luareader_transmit(context_local, apdu, apdu_len, resp, max_resp_size,2000);
            OSAL_MODULE_DBGPRT(usb_322_local->usb_port, OSAL_DEBUG_WARN, "resend whitelist\n");
			print_array(usb_322_local->usb_port, apdu, apdu_len);
        }
			time_in_us = (_re.tv_sec - _end.tv_sec) * 1000000 + _re.tv_usec - _end.tv_usec;	
	        time_in_ms = time_in_us/1000; 
			osal_printf("reconnect time is %ld,%d\n",time_in_ms,usb_322_local->usb_reconnect_cnt);
			}while((usb_322_local->usb_reconnect_cnt)&&(time_in_ms <= 2000));
        if(usb_322_local->usb_reconnect_cnt){

            StatisticsInfo_push(MAINT_SRC_322,usb_322_local->pid_322,whitelist_resume,NULL);
            
        }
    }
    else{

       usb_322_local->usb_reconnect_cnt = 0; 

    }
    osal_sem_release(usb_322_local->sem_322);
    return ret;


}

/*alloc index for 322*/
int alloc_322_index(char * port_name)
{

    int i = 0;

    for(i = 0;i < MAX_322_NUM;i++){//

        if(strcmp(usb_port_def[i],port_name) == 0){
            
            return  i;
            
        }

    }
	osal_printf("\nnode 1-1.1 is 321,so drop it\n");
    return -1;
}

/*****************************************************************************
 * implementation of functions                                               *
*****************************************************************************/
void init_alarm_mask_list(void)
{
    INIT_LIST_HEAD(&controll_eg.alarm_mask_list);

}

static void remove_alarm_list(alarm_node_t *node)
{

    list_del(&node->list);

}
int alarm_mask(uint32_t id,E_ALARM_LIST type)
{
    alarm_node_t *pos;

    int err; 
    err = osal_sem_take(controll_eg.sem_mask_alarm, OSAL_WAITING_FOREVER);
    osal_assert(err == OSAL_STATUS_SUCCESS);

    list_for_each_entry(pos, alarm_node_t, &controll_eg.alarm_mask_list, list){
        if (id == pos->id_322 && type == pos->alarm_type){
			
		    osal_printf("mask alarm  id=0X%X and type = 0X%X\n",id,type);       
			remove_alarm_list(pos);
			osal_free(pos);
			osal_sem_release(controll_eg.sem_mask_alarm);
            return 0;//drop this alarm
        }
    }
    osal_sem_release(controll_eg.sem_mask_alarm);
	osal_printf("not mask alarm id=0X%X and type = 0X%X\n",id,type); 	  
	return 1;//don't drop this alarm	
}

void alarm_add(uint32_t id,E_ALARM_LIST type)
{

	alarm_node_t *p_alarm = NULL;
	
    alarm_node_t *pos = NULL;

    int err; 
    err = osal_sem_take(controll_eg.sem_mask_alarm, OSAL_WAITING_FOREVER);
    osal_assert(err == OSAL_STATUS_SUCCESS);

	list_for_each_entry(pos, alarm_node_t, &controll_eg.alarm_mask_list, list){
        if ((pos->id_322 == id) && (pos->alarm_type == type)){
            p_alarm = pos;
			
		    osal_printf("\nfind exist id=0X%X and type = 0X%X\n",id,type);       
            break;
        }
    }
	if(p_alarm == NULL){
		p_alarm = (alarm_node_t*)malloc(sizeof(alarm_node_t));
		if(p_alarm != NULL){
		    memset(p_alarm,0,sizeof(alarm_node_t));
		    //memcpy(p_alarm->pid,temporary_id,RCP_TEMP_ID_LEN);
			p_alarm->id_322 = id;
			p_alarm->alarm_type = type;
		    list_add(&p_alarm->list,&controll_eg.alarm_mask_list);
			
			osal_printf("\nadd alarm id=0X%X and type = 0X%X\n",id,type); 	  
		}
		else{
		    osal_printf("malloc error!");       
		}
	}
    osal_sem_release(controll_eg.sem_mask_alarm);

}

void empty_list(void)
{
    alarm_node_t *pos = NULL;
/*
    list_for_each_entry(pos, test_comm_node_t, &test_comm_list, list){
        list_del(&pos->list);
        free(pos);
    }
*/
    while (!list_empty(&controll_eg.alarm_mask_list)) {
        pos = list_first_entry(&controll_eg.alarm_mask_list, alarm_node_t, list);
        list_del(&pos->list);
        free(pos);
        }
    
}

uint8_t alarm_fileter(unsigned char* log,int len,usb_ccid_322_t  *usb_322)//alarm 0103
{
	alarm_log_t* get_log = NULL;

	int ret = 1;

	//printf("sizeof(alarm_log_t) = %d\n",sizeof(alarm_log_t));
	if(len != sizeof(alarm_log_t)){
		printf("\nalarm len wrong,don't drop\n");
		return ret;
	}
	
	//get_log = (alarm_log_t*)malloc(len);

	//memcpy(get_log,log,sizeof(alarm_log_t));
	
	//return get_log->alarm_type;
	get_log = (alarm_log_t*)log;
	
	OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_INFO, "\n alarm_fileter 322id = 0x%X,type = 0X%X\n",get_log->ctrl_322_id,get_log->alarm_type);
	if((get_log->alarm_type == ALARM_322_force)||\
		(get_log->alarm_type == ALARM_322_abnormal))
		ret = 0;//alarm_mask(get_log->ctrl_322_id,get_log->alarm_type);
	
	//free(get_log);

	return ret;//don't drop this alarm

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
void process_door_state(usb_ccid_322_t *usb_322,uint8_t door_state)
{

	
	usb_322->now_door_state[1] = door_state;//door state=2
	
	if(usb_322->now_door_state[1] != usb_322->pre_door_state[1]){
	
		 if(usb_322->pre_door_state[1] == 0x02){
			usb_322->door_action = 0xAA;
			OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_INFO, "\ndoor opened\n");
			}
		 else if(usb_322->pre_door_state[1] == 0x01){
			usb_322->door_action = 0x55;
			OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_INFO, "\ndoor closed\n");
		 }
		 usb_322->pre_door_state[1] = usb_322->now_door_state[1];
	
		 sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_SEND_DRSTATE,2,0,usb_322->now_door_state);

}
}
int check_card(usb_ccid_322_t *usb_322,unsigned char* rd_data,int buffer_len)
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
                
                    //在无卡片,无警报的时候处理门状态上报
                    
                    usb_322->now_door_state[1] = rd_data[2];//door state=2
             
                    if(usb_322->now_door_state[1] != usb_322->pre_door_state[1]){

						 if(usb_322->pre_door_state[1] == 0x02){
						 	usb_322->door_action = 0xAA;
						 	OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_INFO, "\ndoor opened\n");
						 	}
						 else if(usb_322->pre_door_state[1] == 0x01){
						 	usb_322->door_action = 0x55;
						 	OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_INFO, "\ndoor closed\n");
						 }
                         usb_322->pre_door_state[1] = usb_322->now_door_state[1];
             
                         sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_SEND_DRSTATE,2,0,usb_322->now_door_state);
                         
                    } 
                     return 0;
            } else if(memcmp(&rd_data[3],alarm_code,sizeof(alarm_code)) == 0){
            
                //无卡时，处理322警报
                //OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_INFO, "90 0A ALARM!\n");
                //free(read_buffer);
                return 2;
                
            }

            
        }
        


        
    }

    if(buffer_len == 2){
        
        if(memcmp(rd_data,wg_error,sizeof(wg_error)) == 0){
            
            OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_INFO, "wg error 9301,sleep 1 sec\n");
            msleep(500);
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

/*添加900A以后,这个字段9000或者900A都是正确的，以后不予判断

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

int check_card_tlv(usb_ccid_322_t  *usb_322,unsigned char* rd_data,int buffer_len,unsigned char* out,int *out_len)
{

	#define  reader_tag   0x0A01	
	#define  _322_tag     0x0A02
	#define  door_tag     0x0B01
	#define  wg_tag	      0x0B02
	#define  n531_tag	  0x0B03
	#define  alarm_tag	  0x0B04
	#define  pr11_event   0x0B05
	#define  _322_event   0x0B06
	#define  pr11_voltage    0x0B07
	#define  pr11_current    0x0B08
	#define  pr11_temper     0x0B09

    uint8_t wg_error[] = {0x93,0x01};

    uint8_t wgp_info[]={0x6F,0xFF};

	
    uint8_t reader_offline[]={0x90,0x0A};

	int ret = 0;

    if(buffer_len == 2){
        
        if(memcmp(rd_data,wg_error,sizeof(wg_error)) == 0){
            
            OSAL_MODULE_DBGPRT(usb_322->usb_port, OSAL_DEBUG_INFO, "wg error 9301,sleep 1 sec\n");
            //sleep(1);
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
    
	tlv_box_t *parsedBoxes =
	  tlv_box_parse (rd_data, buffer_len - 2);
	
	//LOG ("parsedBoxes parse success, %d bytes \n", tlv_box_get_size (parsedBoxes));
	
/*
	tlv_box_t *parsedBox_reader;
	if (tlv_box_get_object (parsedBoxes, reader_tag, &parsedBox_reader) != 0)
	  {
		LOG ("tlv_box_get_object parsedBox_reader failed !\n");
		return -1;
	  }
	
	LOG ("parsedBox_reader parse success, %d bytes \n", tlv_box_get_size (parsedBox_reader));
*/
{
    unsigned char value[32];
    short length = 32;
    if (tlv_box_get_bytes (parsedBoxes, reader_tag, value, &length) != 0)
      {
		//LOG ("parsedBox_reader tlv_box_get_bytes failed !\n");
      }
    
/*
        int i = 0;
        for (i = 0; i < length; i++)
          {
        LOG ("0x%x-", value[i]);
          }
        LOG ("\n");
*/
    
    //LOG ("parsedBox_reader tlv_box_get_bytes success:  ");
	if(length > 2){
		memcpy(out,value,length);
		*out_len = length;
		ret = 1;
	}
/*
	int i = 0;
    for (i = 0; i < length; i++)
      {
	LOG ("0x%x-", value[i]);
      }
    LOG ("\n");
*/
}

	tlv_box_t *parsedBox_322;
	if (tlv_box_get_object (parsedBoxes, _322_tag, &parsedBox_322) != 0)
	  {
		LOG ("tlv_box_get_object  parsedBox_322 failed !\n");
		return -1;
	  }
	
	//LOG ("parsedBox_322 parse success, %d bytes \n", tlv_box_get_size (parsedBox_322));

#if 1
//int i = 0;
//for (i = 0; i < parsedBox_322->m_serialized_bytes; i++)
//  {
//LOG ("0x%X-", parsedBox_322->m_serialized_buffer[i]);
//  }
//LOG ("\n");


{
   unsigned char value[256];
   short length = 256;
   if (tlv_box_get_bytes (parsedBox_322, alarm_tag, value, &length) != 0){
	   //LOG ("get alarm  failed !\n");
   }
   else{
	   
	   LOG ("get alarm success(%d):    ",length);
	   int i = 0;
	   for (i = 0; i < length; i++)
		 {
	   LOG ("%X ", value[i]);
		 }
	   LOG ("\n");
	   //LOG("alarm type is %X\n",(_alarm_log_v2*)value->);
    
       ubus_client_process(UBUS_CLIENT_LOG,NULL,value,length);
   }
}

{
	
	unsigned char value[256];
	short length = 256;
	if ((tlv_box_get_bytes (parsedBox_322, wg_tag, value, &length) == 0)||(tlv_box_get_bytes (parsedBox_322, pr11_event, value, &length) == 0)\
		||(tlv_box_get_bytes (parsedBox_322, _322_event, value, &length) == 0)||(tlv_box_get_bytes (parsedBox_322, pr11_voltage, value, &length) == 0)\
		||(tlv_box_get_bytes (parsedBox_322, pr11_current, value, &length) == 0)||(tlv_box_get_bytes (parsedBox_322, pr11_temper, value, &length) == 0)){
		
		StatisticsInfo_push_n(MAINT_SRC_322,usb_322->pid_322,info_322,parsedBox_322->m_serialized_buffer,parsedBox_322->m_serialized_bytes);
	}


}
#else 0

{
    char door_state;
    if (tlv_box_get_char (parsedBox_322, door_tag, &door_state) != 0)
      {
		LOG ("no door state !\n");
      }
    //LOG ("get door state success %c \n", door_state);
}
	

 {
    unsigned char value[20];
    short length = 20;
    if (tlv_box_get_bytes (parsedBox_322, wg_tag, value, &length) != 0){
		//LOG ("get wg info  failed !\n");
    }
	else{
		
	    LOG ("get wg info success(len=%d):  ",length);
	    int i = 0;
	    for (i = 0; i < length; i++)
	      {
		LOG ("0x%X-", value[i]);
	      }
	    LOG ("\n");
		
		StatisticsInfo_push_n(MAINT_SRC_322,usb_322->pid_322,wgp_alarm,value,length);
	}
 }	
	
 {
	unsigned char value[4];
	short length = 4;
	uint32_t  get_9531cost = 0;
	if (tlv_box_get_bytes (parsedBox_322, n531_tag, value, &length) != 0){
		//LOG ("get 9531 info  failed !\n");
	}
	else{
		
		LOG ("get 9531 info success(%d):	",length);
		int i = 0;
		for (i = 0; i < length; i++)
		  {
		LOG ("0x%X-", value[i]);
		  }
		LOG ("\n");
		get_9531cost = bytesToIntBig(value, 0);
		if(get_9531cost > 2048)
			log_message("9531 info",3,"F[%s] L[%d],9531 overtime ,%X,%X,%X,%X\n",__func__, __LINE__,value[0],value[1],value[2],value[3]);
        StatisticsInfo_push(MAINT_SRC_322,usb_322->pid_322,period_9531,value);

	}
 }
		
{
   unsigned char value[256];
   short length = 256;
   if (tlv_box_get_bytes (parsedBox_322, alarm_tag, value, &length) != 0){
	   //LOG ("get alarm  failed !\n");
   }
   else{
	   
	   LOG ("get alarm success(%d):    ",length);
	   int i = 0;
	   for (i = 0; i < length; i++)
		 {
	   LOG ("%d-", value[i]);
		 }
	   LOG ("\n");
    
       ubus_client_process(UBUS_CLIENT_LOG,NULL,value,length);
   }
}

  {
	unsigned char value[16];
	short length = 16;
	if (tlv_box_get_bytes (parsedBox_322, pr11_event, value, &length) != 0){
		//LOG ("get 9531 info  failed !\n");
	}
	else{
		
		LOG ("get pr11 event  info success(%d):	",length);
		int i = 0;
		for (i = 0; i < length; i++)
		  {
		LOG ("0x%X-", value[i]);
		  }
		LOG ("\n");
        StatisticsInfo_push_n(MAINT_SRC_322,usb_322->pid_322,card_event_clear,value,length);

	}
 }
 {
   unsigned char value[16];
   short length = 16;
   if (tlv_box_get_bytes (parsedBox_322, _322_event, value, &length) != 0){
	   //LOG ("get 9531 info  failed !\n");
   }
   else{
	   
	   LOG ("get 322 event  info success(%d): ",length);
	   int i = 0;
	   for (i = 0; i < length; i++)
		 {
	   LOG ("0x%X-", value[i]);
		 }
	   LOG ("\n");
	   StatisticsInfo_push_n(MAINT_SRC_322,usb_322->pid_322,card_info,value,length);

   }
}
 {
   unsigned char value[16];
   short length = 16;
   if (tlv_box_get_bytes (parsedBox_322, pr11_voltage, value, &length) != 0){
	   //LOG ("get 9531 info  failed !\n");
   }
   else{
	   
	   LOG ("get 322 event  info success(%d): ",length);
	   int i = 0;
	   for (i = 0; i < length; i++)
		 {
	   LOG ("0x%X-", value[i]);
		 }
	   LOG ("\n");
	   StatisticsInfo_push_n(MAINT_SRC_322,usb_322->pid_322,pr11_v,value,length);

   }
}
{
   unsigned char value[16];
   short length = 16;
   if (tlv_box_get_bytes (parsedBox_322, pr11_current, value, &length) != 0){
	   //LOG ("get 9531 info  failed !\n");
   }
   else{
	   
	   LOG ("get 322 event	info success(%d): ",length);
	   int i = 0;
	   for (i = 0; i < length; i++)
		 {
	   LOG ("0x%X-", value[i]);
		 }
	   LOG ("\n");
	   StatisticsInfo_push_n(MAINT_SRC_322,usb_322->pid_322,pr11_c,value,length);

   }
}
{
   unsigned char value[16];
   short length = 16;
   if (tlv_box_get_bytes (parsedBox_322, pr11_temper, value, &length) != 0){
	   //LOG ("get 9531 info  failed !\n");
   }
   else{
	   
	   LOG ("get 322 event	info success(%d): ",length);
	   int i = 0;
	   for (i = 0; i < length; i++)
		 {
	   LOG ("0x%X-", value[i]);
		 }
	   LOG ("\n");
	   StatisticsInfo_push_n(MAINT_SRC_322,usb_322->pid_322,pr11_tp,value,length);

   }
}
#endif

	tlv_box_destroy (parsedBoxes);
	tlv_box_destroy (parsedBox_322);
	return ret;
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

                //printf("find Id card\n");
				
				print_array("find Id card:", &read_buffer[2], 4);
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
				//print_rec(read_buffer, buffer_len);
				
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
	unsigned char pr11_rec[32] = {0};
	int pr11_rec_len;
    unsigned char send_data[1024] = {0};

    unsigned char apdu_data[1024] = {0};

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

    uint8_t alarm_clear_rt_data[64] = {0};//8+2+index(1)+all 322 index(max 4 + 1(321))
    uint8_t remote_open_rt_data[16] = {0};//8+2++index+all322 index(max 4)

    uint8_t info_statistic[39] = {0};
    /**test**/
    
    //struct timeval _start,_end;
    /**test**/

	unsigned char zmq_output[2048] = {0};

	unsigned char face_req[3] = {0x01,0x90,0x00};

	face_remote_log_t  face_remote_log;
	ID_log_t  ID_log;
	remote_log_t  remote_log;
	double_element_log_t double_element_log;
	unsigned char log_result = 0;


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
	#ifdef  USB_TEST
	ret = usb_transmit_fix(&context,PR11_NO,sizeof(PR11_NO),output,sizeof(output),&p_usb_ccid);
	#else
	ret = usb_transmit(context,PR11_NO,sizeof(PR11_NO),output,sizeof(output),p_usb_ccid);
	#endif
	print_rec(output,ret);
	


    sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_INITED,0,p_usb_ccid->ccid322_index - 1,NULL);

while(1){

#if 1
    memset(output,0,sizeof(output));
    memset(zmq_output,0,sizeof(zmq_output));
    memset(apdu_data,0,sizeof(apdu_data));
    //memset(cfg_return,0,sizeof(cfg_return));
    acl_len = 1024;
    log_len = 2048;
    remote_len = 0;
    //printf("state is %d\n",p_usb_ccid->usb_state);
    //context = p_usb_ccid->usb_context;
    if(p_usb_ccid->toggle_state == 0xAA){
    
        p_usb_ccid->toggle_state = 0;
        
        switch (p_usb_ccid->usb_state) {

            
        case USB_COMM_STATE_P2P:

                     
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "send p2pkey\n");

                //printf("controll_eg.p2pkey.len is %d\n",controll_eg.p2pkey.len);

                controll_eg.p2pkey.len = controll_eg.p2pkey.len > 64 ? 38:controll_eg.p2pkey.len;

                print_send(controll_eg.p2pkey.data,controll_eg.p2pkey.len);
				#ifdef	USB_TEST
				ret = usb_transmit_fix(&context,controll_eg.p2pkey.data,controll_eg.p2pkey.len,output,sizeof(output),&p_usb_ccid);
				#else
                ret = usb_transmit(context,controll_eg.p2pkey.data,controll_eg.p2pkey.len,output,sizeof(output),p_usb_ccid);
				#endif
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
                
				sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_RTC_PUSH,0,p_usb_ccid->ccid322_index - 1,NULL);
                sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_SEND_COSVERSION,0,0,NULL);
                osal_sem_release(p_usb_ccid->sem_state);
                //p_usb_ccid->usb_state = USB_COMM_STATE_MACKEY;//USB_COMM_STATE_RDCFG;//USB_COMM_STATE_MACKEY;//USB_COMM_STATE_CFG;//USB_COMM_STATE_MACKEY;
                
            break;
            
        case USB_COMM_STATE_MACKEY:

                
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "send MACKEY\n");
                
                
                controll_eg.mackey.len = controll_eg.mackey.len > 64 ? 37:controll_eg.mackey.len;
                print_send(controll_eg.mackey.data,controll_eg.mackey.len);
				#ifdef USB_TEST
                ret = usb_transmit_fix(&context,controll_eg.mackey.data,controll_eg.mackey.len,output,sizeof(output),&p_usb_ccid);
				#else
                ret = usb_transmit(context,controll_eg.mackey.data,controll_eg.mackey.len,output,sizeof(output),p_usb_ccid);
				#endif
				print_rec(output,ret);

                
                osal_sem_release(p_usb_ccid->sem_state);
                //p_usb_ccid->usb_state = USB_COMM_STATE_RDCFG;//USB_COMM_STATE_CFG;//USB_COMM_STATE_INIT_END;//USB_COMM_STATE_CFG;//USB_COMM_STATE_VERSION;//USB_COMM_STATE_P2P;

            break;

        case USB_COMM_STATE_CFG:

                
                memset(apdu_data,0,sizeof(apdu_data));
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "send base cfg\n");
                
                memcpy(apdu_data,basecfg_head,sizeof(basecfg_head));
                apdu_data[sizeof(basecfg_head)] = controll_eg.basecfg.len;//(unsigned char)(0x00FF&controll_eg.mackey.len);
                memcpy(&apdu_data[sizeof(basecfg_head) + 1],&controll_eg.basecfg.data[0],controll_eg.basecfg.len);  
                
    /*
                printf("cfg len is %d\n",controll_eg.basecfg.len);
                printf("\n===========cfg==============\n");
                for(i = 0; i< controll_eg.basecfg.len;i++){
                    printf("%02x ",controll_eg.basecfg.data[i]);
                }
                printf("\n===========cfg==============\n");
    */
                print_send(apdu_data,sizeof(basecfg_head) + 1 + controll_eg.basecfg.len);
				#ifdef USB_TEST
                ret = usb_transmit_fix(&context,apdu_data,sizeof(basecfg_head) + 1 + controll_eg.basecfg.len,output,sizeof(output),&p_usb_ccid);
				#else
                ret = usb_transmit(context,apdu_data,sizeof(basecfg_head) + 1 + controll_eg.basecfg.len,output,sizeof(output),p_usb_ccid);
                #endif	
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

                
                memset(apdu_data,0,sizeof(apdu_data));
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "send ctrl cfg\n");
                
                memcpy(apdu_data,ctlcfg_head,sizeof(ctlcfg_head));
                apdu_data[sizeof(ctlcfg_head)] = controll_eg.ctlcfg.len;//(unsigned char)(0x00FF&controll_eg.mackey.len);
                memcpy(&apdu_data[sizeof(ctlcfg_head) + 1],&controll_eg.ctlcfg.data[0],controll_eg.ctlcfg.len); 


                
                print_send(apdu_data,sizeof(ctlcfg_head) + 1 + controll_eg.ctlcfg.len);
				#ifdef USB_TEST
                ret = usb_transmit_fix(&context,apdu_data,sizeof(ctlcfg_head) + 1 + controll_eg.ctlcfg.len,output,sizeof(output),&p_usb_ccid);
				#else
                ret = usb_transmit(context,apdu_data,sizeof(ctlcfg_head) + 1 + controll_eg.ctlcfg.len,output,sizeof(output),p_usb_ccid);
				#endif
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
            

        case USB_COMM_STATE_VERSION://考虑到每个322分支可能存在不同情况，例如读卡器的有无，所以每个线程都进行读取

            //if(sw_version == 0)
                {
                
                sw_version = 0xAA;
                /**********************************322 version***********************************************/
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "get 322 version:");
				#ifdef USB_TEST
                ret = usb_transmit_fix(&context,v_322,sizeof(v_322),&output[1],sizeof(output) - 1,&p_usb_ccid);
				#else
                ret = usb_transmit(context,v_322,sizeof(v_322),&output[1],sizeof(output) - 1,p_usb_ccid);
				#endif
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
				#ifdef USB_TEST
                ret = usb_transmit_fix(&context,v_322_d21,sizeof(v_322_d21),&output[1],sizeof(output) - 1,&p_usb_ccid); 
				#else
                ret = usb_transmit(context,v_322_d21,sizeof(v_322_d21),&output[1],sizeof(output) - 1,p_usb_ccid); 
				#endif
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
				#ifdef USB_TEST
				ret = usb_transmit_fix(&context,v_pr11_d21,sizeof(v_pr11_d21),&output[1],sizeof(output) - 1,&p_usb_ccid);
				#else
				ret = usb_transmit(context,v_pr11_d21,sizeof(v_pr11_d21),&output[1],sizeof(output) - 1,p_usb_ccid);
				#endif
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
			/***************************pr02-d21********************************/
			memset(output,0,sizeof(output));
			OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "get pr02-d21 version:");
			#ifdef USB_TEST
				ret = usb_transmit_fix(&context,v_pr02_d21,sizeof(v_pr02_d21),&output[1],sizeof(output) - 1,&p_usb_ccid);
			#else
				ret = usb_transmit(context,v_pr02_d21,sizeof(v_pr02_d21),&output[1],sizeof(output) - 1,p_usb_ccid);
			#endif
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
					OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "bad pr02-d21 version:\n");
			}
				
			/***************************pr02-d21*******************************/	
				
                /**********************************pr11 or pr02 version***********************************************/
                memset(output,0,sizeof(output));
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "get pr11 version:");
				#ifdef  USB_TEST
                ret = usb_transmit_fix(&context,v_pr11,sizeof(v_pr11),&output[1],sizeof(output) - 1,&p_usb_ccid);
				#else
                ret = usb_transmit(context,v_pr11,sizeof(v_pr11),&output[1],sizeof(output) - 1,p_usb_ccid);
				#endif
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
						#ifdef USB_TEST
                        ret = usb_transmit_fix(&context,v_pr02,sizeof(v_pr02),&output[1],sizeof(output) - 1,&p_usb_ccid);
						#else
                        ret = usb_transmit(context,v_pr02,sizeof(v_pr02),&output[1],sizeof(output) - 1,p_usb_ccid);
						#endif
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
                                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "bad pr02 version\n");
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
            #ifdef USB_TEST
            ret = usb_transmit_fix(&context,ce_322,sizeof(ce_322),&output[3],sizeof(output) - 3,&p_usb_ccid);
			#else			
            ret = usb_transmit(context,ce_322,sizeof(ce_322),&output[3],sizeof(output) - 3,p_usb_ccid);
			#endif
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


            /*****************************************read logce*************************************************/
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "reading log ce\n");
            //read 322 ce
            #ifdef USB_TEST
            ret = usb_transmit_fix(&context,ce_log,sizeof(ce_log),&output[3],sizeof(output) - 3,&p_usb_ccid);
			#else			
            ret = usb_transmit(context,ce_log,sizeof(ce_log),&output[3],sizeof(output) - 3,p_usb_ccid);
			#endif
            if(ret <= 0){
            
                
                memset(output, 0, sizeof(output));
                ret = luareader_pop_value(context, (char *)output, sizeof(output));
                printf("luareader_pop_value(%p)=%d(%s)\n", context, ret, output);
                
                log_message(p_usb_ccid->usb_port,3,"log ce read error\n");
                //osal_sem_release(p_usb_ccid->sem_state);
                //break;
                
            
            }else{
            
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "log ce,len is %d\n",ret - 2);//minus 90 00
                //print_rec(&output[3],ret);
                output[0] = p_usb_ccid->ccid322_index;
                output[1] = (unsigned char)(((ret - 2)&0xFF00)>>8);
                output[2] = (unsigned char)(((ret - 2)&0x00FF));
                
                /* Take the semaphore. */
                if(osal_sem_take(sem_logce, OSAL_WAITING_FOREVER) != OSAL_EOK){
                    
                    printf("Semaphore return failed. \n");
                    
                    //osal_sem_release(p_usb_ccid->sem_state);
                    //break;
                }else{
                    writeFile(CEPATH_LOG,output,ret - 2 + 3);
                    osal_sem_release(sem_logce);
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
		#ifdef USB_TEST
        ret = usb_transmit_fix(&context,ctrl_cfg,sizeof(ctrl_cfg),output,sizeof(output),&p_usb_ccid);
		#else
        ret = usb_transmit(context,ctrl_cfg,sizeof(ctrl_cfg),output,sizeof(output),p_usb_ccid);
        #endif	
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
		#ifdef USB_TEST
        ret = usb_transmit_fix(&context,pid_322,sizeof(pid_322),output,sizeof(output),&p_usb_ccid);
		#else
        ret = usb_transmit(context,pid_322,sizeof(pid_322),output,sizeof(output),p_usb_ccid);
        #endif	
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
		#ifdef USB_TEST
        ret = usb_transmit_detect(&context,sn_pr11,sizeof(sn_pr11),output,sizeof(output),&p_usb_ccid);
		#else
        ret = usb_transmit(context,sn_pr11,sizeof(sn_pr11),output,sizeof(output),p_usb_ccid);
        #endif
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
		#ifdef USB_TEST
        ret = usb_transmit_detect(&context,pid_pr11,sizeof(pid_pr11),output,sizeof(output),&p_usb_ccid);
		#else
        ret = usb_transmit(context,pid_pr11,sizeof(pid_pr11),output,sizeof(output),p_usb_ccid);
        #endif
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
		p_usb_ccid->init_flag |= INIT_MASK_CE;
		
		sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_SEND_CE,0,0,NULL);
		
		osal_sem_release(p_usb_ccid->sem_state);
		break;
        /*********controller info*********/
JUMP:
        /************************************************finish************************************************************/
            p_usb_ccid->init_flag |= INIT_MASK_CE;
	
            sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_SEND_CE,0,0,NULL);

            osal_timer_stop(p_usb_ccid->timer_322);//begin to poll,init finished  
			
			printf("stop port %s,index is %d\n",p_usb_ccid->usb_port,p_usb_ccid->ccid322_index);
			osal_timer_delete(p_usb_ccid->timer_322);	
            
            osal_sem_release(p_usb_ccid->sem_state);
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
			#ifdef USB_TEST
            ret = usb_transmit_fix(&context,send_data,sizeof(time_head)+10,output,sizeof(output),&p_usb_ccid);
			#else
            ret = usb_transmit(context,send_data,sizeof(time_head)+10,output,sizeof(output),p_usb_ccid);
			#endif
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "push time\n");
			print_rec(output,ret);
            osal_sem_release(p_usb_ccid->sem_state);
            break;

	    case USB_COMM_RTC_PUSH:

		    get_rtc_data(rtc);
			
	   		memset(apdu_data,0,sizeof(apdu_data));
		
			memcpy(apdu_data,push_rtc,sizeof(push_rtc));

			memcpy(&apdu_data[sizeof(push_rtc)],rtc,sizeof(rtc));
			
 		    OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "push RTC:\n");
			//print_send(apdu_data,sizeof(push_rtc)+sizeof(rtc));
			#ifdef USB_TEST
 		    ret = usb_transmit_fix(&context,apdu_data,sizeof(push_rtc)+sizeof(rtc),output,sizeof(output),&p_usb_ccid); 
			#else
 		    ret = usb_transmit(context,apdu_data,sizeof(push_rtc)+sizeof(rtc),output,sizeof(output),p_usb_ccid); 
			#endif
			print_rec(output,ret);
 		    osal_sem_release(p_usb_ccid->sem_state);
 		    break;
           
        case USB_COMM_ALARM_OPEN:
        
           OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "alarm open\n");
           
           memcpy(apdu_data,alarm_op_head,sizeof(alarm_op_head));
           apdu_data[sizeof(alarm_op_head)] =  controll_eg.alarm_buffer[0];//
           memcpy(&apdu_data[sizeof(alarm_op_head) + 1],&controll_eg.alarm_buffer[1],controll_eg.alarm_buffer[0]); 

           print_send(apdu_data,sizeof(alarm_op_head) + 1 + controll_eg.alarm_buffer[0]);
		   #ifdef USB_TEST
           ret = usb_transmit_fix(&context,apdu_data,sizeof(alarm_op_head) + 1 + controll_eg.alarm_buffer[0],output,sizeof(output),&p_usb_ccid);
		   #else
           ret = usb_transmit(context,apdu_data,sizeof(alarm_op_head) + 1 + controll_eg.alarm_buffer[0],output,sizeof(output),p_usb_ccid);
		   #endif	
		   print_rec(output,ret - 2);

           //ubus_net_process(UBUS_CLIENT_SEND_ALARM,NULL,output,ret - 2);
           ubus_client_process(UBUS_CLIENT_LOG,NULL,output,ret - 2);
           OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "\nsend alarm log to 321\n");
           osal_sem_release(p_usb_ccid->sem_state);
            break;
            
        case USB_COMM_REMOTE_OPEN://0,1=length;2~34=reserve bytes;34~end=data
            
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "remote open\n");
            
            memcpy(apdu_data,remote_op_head,sizeof(remote_op_head));
            apdu_data[sizeof(remote_op_head)] =  0x00;//extend data
            
            remote_len = ((controll_eg.remote_buffer[0]<<8)|(controll_eg.remote_buffer[1]&0x00FF));//indeed remote data length
            memcpy(&apdu_data[sizeof(remote_op_head) + 1],&controll_eg.remote_buffer,2); 
            memcpy(&apdu_data[sizeof(remote_op_head) + 1 + 2],&controll_eg.remote_buffer[34],remote_len);//0,1=len;2~33=ubus data; 34~=remote data
            
            print_send(apdu_data,sizeof(remote_op_head) + 1 + 2 + remote_len);
			#ifdef USB_TEST
            ret = usb_transmit_fix(&context,apdu_data,sizeof(remote_op_head) + 1 + 2 + remote_len,output,sizeof(output),&p_usb_ccid);
			#else
            ret = usb_transmit(context,apdu_data,sizeof(remote_op_head) + 1 + 2 + remote_len,output,sizeof(output),p_usb_ccid);
			#endif
			print_rec(output,ret);

            memset(remote_open_rt_data,0,sizeof(remote_open_rt_data));
			
			memcpy(remote_open_rt_data,&controll_eg.remote_buffer[2],8);
			
            if(ret > 2){
				
                log_len = ret - 2;
                memcpy(log_data,output,log_len);
                ubus_client_process(UBUS_CLIENT_LOG,NULL,log_data,log_len);

                
                memcpy(remote_open_rt_data,&controll_eg.remote_buffer[2],8);

				if(log_len == sizeof(remote_log_t)){

					
					memcpy(&remote_log,output,log_len);
					log_result = remote_log.result;
					osal_printf("远程开门\n");
				
				}
				else if(log_len == sizeof(double_element_log_t)){
					
					memcpy(&double_element_log,output,log_len);
					log_result = double_element_log.result;
					
					osal_printf("双因子开门\n");
				}
				
                if(log_result == 0x01){//197 = result
                        remote_open_rt_data[8] = 0x90;
                        remote_open_rt_data[9] = 0x00;
                    }
                else{
                        remote_open_rt_data[8] = 0x00;
                        remote_open_rt_data[9] = log_result;
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
		    #ifdef  USB_TEST
            ret = usb_transmit_fix(&context,p_usb_ccid->zmq_buffer,p_usb_ccid->zmq_len,output,sizeof(output),&p_usb_ccid);
			#else
            ret = usb_transmit(context,p_usb_ccid->zmq_buffer,p_usb_ccid->zmq_len,output,sizeof(output),p_usb_ccid);
			#endif
            //OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "322 Id return:\n");
		#ifdef ZMQ_NUM
			if(ret > 0){
                print_rec(output,ret);
				memcpy(zmq_output,p_usb_ccid->zmq_magicnum,5);
				memcpy(&zmq_output[5],output,ret);
				print_array("zmq send: ", zmq_output, ret + 5);
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
			
			osal_sem_release(p_usb_ccid->sem_zmq);

            osal_sem_release(p_usb_ccid->sem_state);
            break;
            
        case  USB_COMM_ID_DOOR_SERVER:
            
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "Id info to server\n");
            
            print_send(id_info_get,sizeof(id_info_get));
		    #ifdef USB_TEST
            ret = usb_transmit_fix(&context,id_info_get,sizeof(id_info_get),output,sizeof(output),&p_usb_ccid);
			#else
            ret = usb_transmit(context,id_info_get,sizeof(id_info_get),output,sizeof(output),p_usb_ccid);
			#endif
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "322 Id info return:\n");            
            print_rec(output,ret);
            
            if((ret >2)&&(memcmp(&output[ret -2],confirm,sizeof(confirm)) == 0)){
                
                    OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "make id data:\n");
                    apdu_data[0] = p_usb_ccid->door_index;
                    
                    memcpy(&apdu_data[1],output,ret -2);
                    memcpy(&apdu_data[1 + ret -2],&zmq_ans[4],rec_zmq - 4);
                    //memcpy(&apdu_data[1+rec_zmq - 4],output,ret -2);
                    
                    print_send(apdu_data,1+ rec_zmq - 4 + ret -2);
                    ubus_net_process(UBUS_CLIENT_SEND_ID_INFO,NULL,apdu_data,1+rec_zmq - 4+ret -2);
            }
            osal_sem_release(p_usb_ccid->sem_state);
            break;

        case USB_ID_REMOTE_OPEN:
            
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "ID remote open\n");
            
            memcpy(apdu_data,remote_idop_head,sizeof(remote_idop_head));
            apdu_data[sizeof(remote_idop_head)] =  0x00;//extend data
           
            
            remote_len = ((controll_eg.remote_buffer[0]<<8)|(controll_eg.remote_buffer[1]&0x00FF));//indeed remote data length
            memcpy(&apdu_data[sizeof(remote_idop_head) + 1],&controll_eg.remote_buffer,2); 
            memcpy(&apdu_data[sizeof(remote_idop_head) + 1 + 2],&controll_eg.remote_buffer[34],remote_len);//0,1=len;2~33=ubus data; 34~=remote data
            
            print_send(apdu_data,sizeof(remote_idop_head) + 1 + 2 + remote_len);
			#ifdef  USB_TEST
            ret = usb_transmit_fix(&context,apdu_data,sizeof(remote_idop_head) + 1 + 2 + remote_len,output,sizeof(output),&p_usb_ccid);
			#else
            ret = usb_transmit(context,apdu_data,sizeof(remote_idop_head) + 1 + 2 + remote_len,output,sizeof(output),p_usb_ccid);
			#endif
			print_rec(output,ret);
            

            memset(remote_open_rt_data,0,sizeof(remote_open_rt_data));
			memcpy(remote_open_rt_data,&controll_eg.remote_buffer[2],8);
            if(ret > 2){
				
                log_len = ret - 2;
                memcpy(log_data,output,log_len);
                ubus_client_process(UBUS_CLIENT_LOG,NULL,log_data,log_len);

                
                memcpy(remote_open_rt_data,&controll_eg.remote_buffer[2],8);
				
				if(log_len == sizeof(ID_log_t)){

					
					memcpy(&ID_log,output,log_len);
					log_result = ID_log.result;
					osal_printf("身份证远程开门\n");
				
				}
				else if(log_len == sizeof(double_element_log_t)){
					
					memcpy(&double_element_log,output,log_len);
					log_result = double_element_log.result;
					
					osal_printf("双因子开门\n");
				}
				
                if(log_result == 0x01){//124 = result
                //if(output[124] == 0x01){//124 = result
                        remote_open_rt_data[8] = 0x90;
                        remote_open_rt_data[9] = 0x00;
                    }
                else{
                        remote_open_rt_data[8] = 0x00;
                        remote_open_rt_data[9] = log_result;
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


            osal_sem_release(controll_eg.sem_remote);
            osal_sem_release(p_usb_ccid->sem_state);
            
            break;
        case USB_FACE_REMOTE_OPEN:
            
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "FACE remote open\n");
            
            memcpy(apdu_data,remote_faceop_head,sizeof(remote_faceop_head));
            apdu_data[sizeof(remote_faceop_head)] =  0x00;//extend data
                       
            remote_len = ((controll_eg.remote_buffer[0]<<8)|(controll_eg.remote_buffer[1]&0x00FF));//indeed remote data length
            memcpy(&apdu_data[sizeof(remote_faceop_head) + 1],&controll_eg.remote_buffer,2); 
            memcpy(&apdu_data[sizeof(remote_faceop_head) + 1 + 2],&controll_eg.remote_buffer[34],remote_len);//0,1=len;2~33=ubus data; 34~=remote data
            
            print_send(apdu_data,sizeof(remote_faceop_head) + 1 + 2 + remote_len);
			#ifdef USB_TEST
            ret = usb_transmit_fix(&context,apdu_data,sizeof(remote_faceop_head) + 1 + 2 + remote_len,output,sizeof(output),&p_usb_ccid);
			#else
            ret = usb_transmit(context,apdu_data,sizeof(remote_faceop_head) + 1 + 2 + remote_len,output,sizeof(output),p_usb_ccid);
			#endif
			print_rec(output,ret);
            

            memset(remote_open_rt_data,0,sizeof(remote_open_rt_data));
			
			memcpy(remote_open_rt_data,&controll_eg.remote_buffer[2],8);
            if(ret > 2){
			
                log_len = ret - 2;
                memcpy(log_data,output,log_len);
                ubus_client_process(UBUS_CLIENT_LOG,NULL,log_data,log_len);

				if(log_len == sizeof(face_remote_log_t)){

					
					memcpy(&face_remote_log,output,log_len);
					log_result = face_remote_log.result;
					osal_printf("人脸开门\n");
				
				}
				else if(log_len == sizeof(double_element_log_t)){
					
					memcpy(&double_element_log,output,log_len);
					log_result = double_element_log.result;
					
					osal_printf("双因子开门\n");
				}
				
                if(log_result == 0x01){//96 = result
                        remote_open_rt_data[8] = 0x90;
                        remote_open_rt_data[9] = 0x00;
                    }
                else{
                        remote_open_rt_data[8] = 0x00;
                        remote_open_rt_data[9] = log_result;
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


            osal_sem_release(controll_eg.sem_remote);
            osal_sem_release(p_usb_ccid->sem_state);
            
            break;




     case USB_COMM_STATE_DEFAULT:
            break;

      case USB_COMM_CLEAR_ALARM:
            
            OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "alarm clear\n");
            
            memcpy(apdu_data,alarm322_clear_head,sizeof(alarm322_clear_head));
            
            memcpy(&apdu_data[sizeof(alarm322_clear_head) ],&controll_eg.alarm_clear[32],16); 
            
            print_send(apdu_data,sizeof(alarm322_clear_head) + 16);
			#ifdef USB_TEST
            ret = usb_transmit_fix(&context,apdu_data,sizeof(alarm322_clear_head) + 16,output,sizeof(output),&p_usb_ccid);
			#else
            ret = usb_transmit(context,apdu_data,sizeof(alarm322_clear_head) + 16,output,sizeof(output),p_usb_ccid);
			#endif
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
#if 0
            alarm_clear_rt_data[10] = p_usb_ccid->ccid322_index;
            
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
#else
            //alarm_clear_rt_data[10] = p_usb_ccid->ccid322_index;
            
            if(controll_eg.alarm_clear[8] == 0x01){

				
				alarm_clear_rt_data[10] = controll_eg.cnt_322;
                                	
                memcpy(&alarm_clear_rt_data[11],&controll_eg.index_322,controll_eg.cnt_322);
				alarm_clear_rt_data[11+controll_eg.cnt_322] = p_usb_ccid->ccid322_index;
				alarm_clear_rt_data[11+controll_eg.cnt_322 + 1] = ret -2;
				memcpy(&alarm_clear_rt_data[11+controll_eg.cnt_322 + 2],output,ret -2);
                printf("\nsend alarm clear return,only 322\n");
                print_send(alarm_clear_rt_data,11 + controll_eg.cnt_322 + ret );
                ubus_net_process(UBUS_CLIENT_CLEAR_RETURN,NULL,alarm_clear_rt_data,11 + controll_eg.cnt_322 + ret);


            }
            else if(controll_eg.alarm_clear[8] == 0x02){

			
				alarm_clear_rt_data[10] = controll_eg.cnt_322 + 1;
                                	
                alarm_clear_rt_data[11] = 0xFF;
                memcpy(&alarm_clear_rt_data[12],&controll_eg.index_322,controll_eg.cnt_322);
				alarm_clear_rt_data[12+controll_eg.cnt_322] = p_usb_ccid->ccid322_index;
				alarm_clear_rt_data[12+controll_eg.cnt_322 + 1] = ret -2;
				memcpy(&alarm_clear_rt_data[12+controll_eg.cnt_322 + 2],output,ret -2);
                printf("\nsend alarm clear return,both 322 & 321\n");
                print_send(alarm_clear_rt_data,12 + controll_eg.cnt_322 + ret );
                ubus_net_process(UBUS_CLIENT_CLEAR_RETURN,NULL,alarm_clear_rt_data,12 + controll_eg.cnt_322 + ret );
            }
#endif
            osal_sem_release(p_usb_ccid->sem_state);
            
            break;

    case  USB_322_TEST:
        OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "322 usb reset\n");
		#ifdef USB_TEST
        usb_transmit_fix(&context,reset_322usb,sizeof(reset_322usb),output,sizeof(output),&p_usb_ccid);
		#else
        usb_transmit(context,reset_322usb,sizeof(reset_322usb),output,sizeof(output),p_usb_ccid);
        #endif
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

/*
        OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "send net_state to pr11\n");
        print_send(net_state,sizeof(net_state));
        #ifdef USB_TEST
        ret = usb_transmit_fix(&context,net_state,sizeof(net_state),output,sizeof(output),&p_usb_ccid);
        #else
        ret = usb_transmit(context,net_state,sizeof(net_state),output,sizeof(output),p_usb_ccid);
        #endif
        print_rec(output,ret);
*/
        
        if((controll_eg.network_state)&&(controll_eg.network_state_pre != controll_eg.network_state))
            zmq_socket_send(p_usb_ccid->zmq_client,id_reader_deal_OK,sizeof(id_reader_deal_OK));

		controll_eg.network_state_pre = controll_eg.network_state;
        osal_sem_release(p_usb_ccid->sem_state);
        break;

        default:
            break;
        }
        //continue;
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
			#ifdef  USB_TEST
            ret = usb_transmit_fix(&context,car_detect,sizeof(car_detect),output,sizeof(output),&p_usb_ccid);
			#else
            ret = usb_transmit(context,car_detect,sizeof(car_detect),output,sizeof(output),p_usb_ccid);
			#endif	
			//print_rec(output,ret);//打印寻卡结果
			
			//osal_printf("F[%s] L[%d]\n",__func__, __LINE__);
			if((output[0] != 0x0A))
          	tail_check = check_card(p_usb_ccid,output,ret);
			else	
			tail_check = check_card_tlv(p_usb_ccid,output,ret,pr11_rec,&pr11_rec_len);
			//osal_printf("F[%s] L[%d]\n",__func__, __LINE__);
			
			//OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "check card! \n");
#if 1			
if(tail_check == 1){
	if((output[0] != 0x0A))
		
    	parse_tag = parse_data(output,ret,acl_data,&acl_len,p_usb_ccid);
	else
    	parse_tag = parse_data(pr11_rec,pr11_rec_len,acl_data,&acl_len,p_usb_ccid);
	//osal_printf("F[%s] L[%d]\n",__func__, __LINE__);
    switch(parse_tag){

        
        case CARDPOLLEVENT_NO_EVENT:
        
            if(acl_len == 17){

                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_WARN, "not passed\n");
                
                memcpy(apdu_data,result_head,sizeof(result_head));

                apdu_data[sizeof(result_head)] = 17;

                memcpy(&apdu_data[sizeof(result_head) + 1],acl_data,acl_len);

                //print_send(apdu_data,sizeof(result_head) + 1 + 17);
/*
                osal_printf("start sleep 2s\n");
                sleep(2);
                osal_printf("sleep over\n");
*/
                #ifdef  USB_TEST
                ret = whitelist_transmit_fix(&context,apdu_data,sizeof(result_head) + 1 + 17,output,sizeof(output),&p_usb_ccid);
				#else
                ret = usb_transmit(context,apdu_data,sizeof(result_head) + 1 + 17,output,sizeof(output),p_usb_ccid);
			    #endif
            }
            else if(acl_len > 17){

                memcpy(apdu_data,result_head,sizeof(result_head));
                
                apdu_data[sizeof(result_head)] = 0;//extend len
                if(308 == acl_len){
					//acl_len = 1+ rtc(16) + whitelist(232) + whitelistconfig(59) = 308
	                wl_len =  acl_len;//

	                apdu_data[sizeof(result_head) + 1] = (0xFF00&wl_len)>>8;//extend len
	                apdu_data[sizeof(result_head) + 2] = (0x00FF&wl_len);//extend len

	                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "data len is %d\n",wl_len);

	                memcpy(&apdu_data[sizeof(result_head) + 3],acl_data,17);//result:1 + RTC:16

	                memcpy(&apdu_data[sizeof(result_head) + 3 + 17],&acl_data[249],59);//config
	                
	                memcpy(&apdu_data[sizeof(result_head) + 3 + 17 + 59],&acl_data[17],232);//white list

	                print_send(apdu_data,sizeof(result_head) + 3 + 17 + 59 + 232);
	                
					#ifdef USB_TEST
	                ret = whitelist_transmit_fix(&context,apdu_data,sizeof(result_head) + 3 + 17 + 59 + 232,output,sizeof(output),&p_usb_ccid);
					#else
	                ret = whitelist_transmit(context,apdu_data,sizeof(result_head) + 3 + 17 + 59 + 232,output,sizeof(output),p_usb_ccid);
					#endif
                }
				else if(301 == acl_len){
					//acl_len = 1+ rtc(16) + whitelist(232) + whitelistconfig(52) = 301
	                wl_len =  17 + 1 + acl_data[271] + 232;//1+rtc(16)+name(l+v = 1+acl_data[271])+232

	                apdu_data[sizeof(result_head) + 1] = (0xFF00&wl_len)>>8;//extend len
	                apdu_data[sizeof(result_head) + 2] = (0x00FF&wl_len);//extend len

	                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "data len is %d\n",wl_len);

	                memcpy(&apdu_data[sizeof(result_head) + 3],acl_data,17);//result:1 + RTC:16

	                memcpy(&apdu_data[sizeof(result_head) + 3 + 17],&acl_data[271],1);//name-L:1
	                
	                memcpy(&apdu_data[sizeof(result_head) + 3 + 17 + 1],&acl_data[272],acl_data[271]);//name-V:v

	                memcpy(&apdu_data[sizeof(result_head) + 3 + 17 + 1 + acl_data[271]],&acl_data[17],232);

	                print_send(apdu_data,sizeof(result_head) + 3 + 17 + 1 + acl_data[271] + 232);
	                
					#ifdef USB_TEST
	                ret = whitelist_transmit_fix(&context,apdu_data,sizeof(result_head) + 3 + 17 + 1 + acl_data[271] + 232,output,sizeof(output),&p_usb_ccid);
					#else
	                ret = whitelist_transmit(context,apdu_data,sizeof(result_head) + 3 + 17 + 1 + acl_data[271] + 232,output,sizeof(output),p_usb_ccid);
					#endif
				}
			}           
			else{
				
				OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "321 give wl error!get ubus len is %d\n",acl_len);
				

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



    				if(output[106] == 0x04){

    					OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "get slow log\n");
    					memset(output,0,sizeof(output));
					#ifdef USB_TEST
    					ret = usb_transmit_fix(&context,get_slow_log,sizeof(get_slow_log),output,sizeof(output),&p_usb_ccid);
					#else
    					ret = usb_transmit(context,get_slow_log,sizeof(get_slow_log),output,sizeof(output),p_usb_ccid);
					#endif
    					if(ret > 2){

							StatisticsInfo_push_n(MAINT_SRC_322,p_usb_ccid->pid_322,time_consuming,output,ret -2);
    					}
    					else{
    						if(ret > 0)
    							print_rec(output,ret);
    						OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "get slow log error\n");
    						
    					}
								

					} 
	                
	            }
            }
			else if(2 == ret){
				uint8_t req[2] = {0x90,0x00};
				if(memcmp(output,req,ret) == 0){
					
					osal_printf("F[%s] L[%d],double element ,no log\n",__func__, __LINE__);
					
				}
				else{
					osal_printf("F[%s] L[%d],undefine return  of log\n",__func__, __LINE__);
					log_message("log return",3,"F[%s] L[%d],undefine return  of log\n",__func__, __LINE__);

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
           memcpy(apdu_data,transfer_head,sizeof(transfer_head));
           
           apdu_data[sizeof(transfer_head)] = 0;//extend length

           apdu_data[sizeof(transfer_head) + 1] = 0;//extend length char 1

           apdu_data[sizeof(transfer_head) + 2] = 0;//extend length char 2

           /************************return data*************************/
           
           memcpy(&apdu_data[sizeof(transfer_head) + 3],audit_result_head,sizeof(audit_result_head));//+lc

           apdu_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head)] = (unsigned char)((acl_len&0xFF00)>>8); 
           apdu_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head) + 1] = (unsigned char)(acl_len&0x00FF); 

           memcpy(&apdu_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head) + 2],acl_data,acl_len);

           audit_len = sizeof(audit_result_head) + 2 + acl_len;

           apdu_data[sizeof(transfer_head) + 1] = (unsigned char)((audit_len&0xFF00)>>8);
           apdu_data[sizeof(transfer_head) + 2] = (unsigned char)(audit_len&0x00FF);//extend length char 2
           
           print_send(apdu_data,sizeof(transfer_head) + 3 + audit_len);
			#ifdef USB_TEST
           ret = usb_transmit_fix(&context,apdu_data,sizeof(transfer_head) + 3 + audit_len,output,sizeof(output),&p_usb_ccid);
			#else
           ret = usb_transmit(context,apdu_data,sizeof(transfer_head) + 3 + audit_len,output,sizeof(output),p_usb_ccid);
			#endif
           print_rec(output,ret);
            
            break;

        case CARDPOLLEVENT_WL:
            
            /***********************transfer cmd**************************/
            memcpy(apdu_data,transfer_head,sizeof(transfer_head));
            
            apdu_data[sizeof(transfer_head)] = 0;//extend length
            
            apdu_data[sizeof(transfer_head) + 1] = 0;//extend length char 1
            
            apdu_data[sizeof(transfer_head) + 2] = 0;//extend length char 2
            
            /************************return data*************************/
            
            memcpy(&apdu_data[sizeof(transfer_head) + 3],audit_result_head,sizeof(audit_result_head));//+lc
            
            apdu_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head)] = (unsigned char)((acl_len&0xFF00)>>8); 
            apdu_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head) + 1] = (unsigned char)(acl_len&0x00FF); 
            
            memcpy(&apdu_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head) + 2],acl_data,acl_len);
            
            audit_len = sizeof(audit_result_head) + 2 + acl_len;
            
            apdu_data[sizeof(transfer_head) + 1] = (unsigned char)((audit_len&0xFF00)>>8);
            apdu_data[sizeof(transfer_head) + 2] = (unsigned char)(audit_len&0x00FF);//extend length char 2
            
            print_send(apdu_data,sizeof(transfer_head) + 3 + audit_len);
            #ifdef USB_TEST
            ret = usb_transmit_fix(&context,apdu_data,sizeof(transfer_head) + 3 + audit_len,output,sizeof(output),&p_usb_ccid);
			#else
            ret = usb_transmit(context,apdu_data,sizeof(transfer_head) + 3 + audit_len,output,sizeof(output),p_usb_ccid);
			#endif
            print_rec(output,ret);
            
            break;

        case CARDPOLLEVENT_GET_LOGNUM:
            
            /***********************transfer cmd**************************/
            memcpy(apdu_data,transfer_head,sizeof(transfer_head));
            
            apdu_data[sizeof(transfer_head)] = 0;//extend length
            
            apdu_data[sizeof(transfer_head) + 1] = 0;//extend length char 1
            
            apdu_data[sizeof(transfer_head) + 2] = 0;//extend length char 2
            
            /************************return data*************************/
            
            memcpy(&apdu_data[sizeof(transfer_head) + 3],audit_result_head,sizeof(audit_result_head));//+lc
            
            apdu_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head)] = (unsigned char)((acl_len&0xFF00)>>8); 
            apdu_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head) + 1] = (unsigned char)(acl_len&0x00FF); 
            
            memcpy(&apdu_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head) + 2],acl_data,acl_len);
            
            audit_len = sizeof(audit_result_head) + 2 + acl_len;
            
            apdu_data[sizeof(transfer_head) + 1] = (unsigned char)((audit_len&0xFF00)>>8);
            apdu_data[sizeof(transfer_head) + 2] = (unsigned char)(audit_len&0x00FF);//extend length char 2
            
            print_send(apdu_data,sizeof(transfer_head) + 3 + audit_len);
            #ifdef USB_TEST
            ret = usb_transmit_fix(&context,apdu_data,sizeof(transfer_head) + 3 + audit_len,output,sizeof(output),&p_usb_ccid);
		    #else
            ret = usb_transmit(context,apdu_data,sizeof(transfer_head) + 3 + audit_len,output,sizeof(output),p_usb_ccid);
			#endif
            print_rec(output,ret);
        

            
            break;

        case CARDPOLLEVENT_GET_LOG:
            /***********************transfer cmd**************************/
            memcpy(apdu_data,transfer_head,sizeof(transfer_head));
            
            apdu_data[sizeof(transfer_head)] = 0;//extend length
            
            apdu_data[sizeof(transfer_head) + 1] = 0;//extend length char 1
            
            apdu_data[sizeof(transfer_head) + 2] = 0;//extend length char 2
            
            /************************return data*************************/
            
            memcpy(&apdu_data[sizeof(transfer_head) + 3],audit_result_head,sizeof(audit_result_head));//+lc
            
            apdu_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head)] = (unsigned char)((acl_len&0xFF00)>>8); 
            apdu_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head) + 1] = (unsigned char)(acl_len&0x00FF); 
            
            memcpy(&apdu_data[sizeof(transfer_head) + 3 + sizeof(audit_result_head) + 2],acl_data,acl_len);
            
            audit_len = sizeof(audit_result_head) + 2 + acl_len;
            
            apdu_data[sizeof(transfer_head) + 1] = (unsigned char)((audit_len&0xFF00)>>8);
            apdu_data[sizeof(transfer_head) + 2] = (unsigned char)(audit_len&0x00FF);//extend length char 2
            
            print_send(apdu_data,sizeof(transfer_head) + 3 + audit_len);
            #ifdef  USB_TEST		
            ret = usb_transmit_fix(&context,apdu_data,sizeof(transfer_head) + 3 + audit_len,output,sizeof(output),&p_usb_ccid);
		    #else
            ret = usb_transmit(context,apdu_data,sizeof(transfer_head) + 3 + audit_len,output,sizeof(output),p_usb_ccid);
			#endif
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
	   		print_array("send to zmq", id_reader, sizeof(id_reader));
            zmq_socket_send(p_usb_ccid->zmq_client,id_reader,sizeof(id_reader));
            break;
            
        default:
            break;    

    }

}
else if(tail_check == 2){


    //if((p_usb_ccid->rtc_sync == 0xAA)&&(p_usb_ccid->alarm_period == 0xAA)){
	if(p_usb_ccid->rtc_sync == 0xAA){
		
        get_rtc_data(rtc);
        p_usb_ccid->alarm_period = 0;
        OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "\nget 322 alarm,send rtc encrypt:\n");


        memcpy(apdu_data,alarm322_head,sizeof(alarm322_head));
        memcpy(&apdu_data[sizeof(alarm322_head)],rtc,sizeof(rtc));
        print_send(apdu_data,sizeof(alarm322_head) + sizeof(rtc));

		#ifdef  USB_TEST
        ret = usb_transmit_fix(&context,apdu_data,sizeof(alarm322_head) + sizeof(rtc),output,sizeof(output),&p_usb_ccid);
		#else
        ret = usb_transmit(context,apdu_data,sizeof(alarm322_head) + sizeof(rtc),output,sizeof(output),p_usb_ccid);
		#endif


        OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "\nget 322 alarm log:\n");

        print_rec(output,ret);

  			
    	controll_eg.alarm_flag = alarm_fileter(output, ret - 2, p_usb_ccid);

		if(controll_eg.alarm_flag){

				OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "send alarm to 321!\n");
				ubus_client_process(UBUS_CLIENT_LOG,NULL,output,ret - 2);
		}
		else{
			
			if(controll_eg.alarm_flag == 0){
				
				OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "open door,drop alarm!\n");			   
				
			}
		}
			
        
        }
    }
else if(tail_check == -4 ){


        print_send(wgp_info_get,sizeof(wgp_info_get));
		#ifdef  USB_TEST
        ret = usb_transmit_fix(&context,wgp_info_get,sizeof(wgp_info_get),output,sizeof(output),&p_usb_ccid);
		#else
        ret = usb_transmit(context,wgp_info_get,sizeof(wgp_info_get),output,sizeof(output),p_usb_ccid);
		#endif
        printf("wgp info:\n");
        print_rec(output,ret);
        if(ret > 2){
            
            if((controll_eg.push_flag & 0x01) == false){
                
                StatisticsInfo_push_n(MAINT_SRC_322,p_usb_ccid->pid_322,wgp_alarm,output,ret - 2);
                
                controll_eg.push_flag |= 0x01;
            }
            else
                printf("\ndo not push wgp info\n");

                p_usb_ccid->WG_ERROR = 0xAA;
                OSAL_MODULE_DBGPRT(p_usb_ccid->usb_port, OSAL_DEBUG_INFO, "WG ERROR FLAG UP!\n");
        }

}
#endif
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
                sys_add_event_queue(&controll_eg.msg_manager,ZMQ_RESULT,0,p_usb_ccid->ccid322_index - 1,NULL);
            
            
            printf("switch value is %d\n",p_usb_ccid->toggle_state);
        }
}
        osal_sleep(20);
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
		msleep(100);

		return NULL;
}



uint8_t usb_wb;

void timer_usb_callback(void* parameter)
{
    usb_ccid_322_t *p_usb_timer = (usb_ccid_322_t *)parameter;
    
    //printf("timer in port %s,index is %d\n",p_usb_timer->usb_port,p_usb_timer->ccid322_index);
    //     printf("timer 322 come in\n");	
   // if(p_usb_timer->toggle_ubus == 0xAA){

        
        if(p_usb_timer->toggle >= 10){

            p_usb_timer->toggle = 0;
            //p_usb_timer->usb_state = USB_COMM_STATE_PUSH;
            //printf("push index %d\n",p_usb_timer->ccid322_index);
            sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_INFO_PUSH,0,p_usb_timer->ccid322_index - 1,NULL);
			sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_RTC_PUSH,0,p_usb_timer->ccid322_index - 1,NULL);
/*
                get_rtc_data(rtc);

                print_rec(rtc,16);
*/

        }
        else{
            
            p_usb_timer->toggle++;
            p_usb_timer->alarm_period = 0xAA;

        }
   // }
//test usb interrupt
   //sys_add_event_queue(&controll_eg.msg_manager,SYS_MSG_322_USBTEST,0,p_usb_timer->ccid322_index,NULL);
   
	//printf("timer 322 come out\n"); 
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

        if(dev_index > MAX_USB_NUM){ //MAX_322_NUM = 4

            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "number of device is over %d\n",MAX_322_NUM);
            break;
        }

        
        if((output[i] == 0)&&(dev_index <= MAX_USB_NUM)){
            
            memcpy(device_str[dev_index++],&output[j],i - j + 1);
            j = i + 1;
        }


    }
    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "there is  %d devices on pc02\n",dev_index);
    /*max device = 4*/
    dev_index = (dev_index < MAX_USB_NUM ? dev_index : MAX_USB_NUM);
    /**/
    
    sem_pr11ce = osal_sem_create("sem_pr11ce", 1);
    osal_assert(sem_pr11ce != NULL);
    //printf("\n%p\n",sem_pr11ce);

    sem_322ce = osal_sem_create("sem_322ce", 1);
    osal_assert(sem_322ce != NULL);
	
    sem_logce = osal_sem_create("sem_logce", 1);
    osal_assert(sem_logce != NULL);

    sem_ctrlinfo= osal_sem_create("sem_ctrlinfo", 1);
    osal_assert(sem_ctrlinfo != NULL);

    controll_eg.sem_remote = osal_sem_create("sem_remote", 1);
    osal_assert(controll_eg.sem_remote != NULL);

    controll_eg.sem_base_cfg = osal_sem_create("sem_base_cfg", 1);
    osal_assert(controll_eg.sem_base_cfg != NULL);

    controll_eg.sem_ctrl_cfg = osal_sem_create("sem_ctrl_cfg", 1);
    osal_assert(controll_eg.sem_ctrl_cfg != NULL);

	controll_eg.sem_mask_alarm = osal_sem_create("sem_mask_alarm", 1);
    osal_assert(controll_eg.sem_mask_alarm != NULL);

	init_alarm_mask_list();

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
        if(ret == -1)//321-"1-1.1"
            continue;
        //if(ret == 1)
            //continue;
        controll_eg.cnt_322++;
        p_usb_ccid = &(controll_eg.usb_ccid_322[ret]);
        p_usb_ccid->usb_state = USB_COMM_STATE_DEFAULT;//USB_COMM_STATE_INIT;//USB_COMM_STATE_INIT_END;//USB_COMM_STATE_INIT;
        p_usb_ccid->ccid322_index = ret + 1;
        p_usb_ccid->toggle_state = 0;
        p_usb_ccid->ccid322_exist = 1;
        p_usb_ccid->pr11_exist = 1;//defualt 1,has a reader
        p_usb_ccid->init_flag = 0;
        p_usb_ccid->rtc_sync = 0;//RTC sync
        p_usb_ccid->now_door_state[0] = 0x01;
        p_usb_ccid->now_door_state[1] = 0x02;
        p_usb_ccid->pre_door_state[0] = 0x01;//门序号
        p_usb_ccid->pre_door_state[1] = 0x02;//门状态
        
        strcpy(p_usb_ccid->usb_port,device_str[i]);
            
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
                            EUSB_SEND_PERIOD, TIMER_INTERVAL |TIMER_STOPPED, TIMER_PRIO_NORMAL);
        osal_assert(p_usb_ccid->timer_322 != NULL);
        
        ret = osal_timer_start(p_usb_ccid->timer_322);
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_WARN, "create timer %s,start status is %d\n",device_str[i],ret);

        eg_zmq_init(p_usb_ccid);

    }  
        
    printf("322 num:%d\n",controll_eg.cnt_322);
    
    //OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "module initial finished\n");

}
