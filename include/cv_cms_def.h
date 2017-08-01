/*****************************************************************************
 Copyright(C) Tendyron Corporation
 All rights reserved.
 
 @file   : cv_cms_def.h
 @brief  : struct and enum define
 @author : gexueyuan
 @history:
           2017-5-17    gexueyuan    Created file
           ...
******************************************************************************/
#ifndef __CV_CMS_DEF_H__
#define __CV_CMS_DEF_H__

#include "cv_osal.h"
#include "zmq.h"

/*****************************************************************************
 * declaration of variables and functions                                    *
*****************************************************************************/

//#define RSU_TEST

/**
    prority of all the tasks in system 
*/
#define RT_SYS_THREAD_PRIORITY		TK_PRIO_DEFAULT//(TK_PRIO_DEFAULT - 15)
#define RT_VAM_THREAD_PRIORITY		TK_PRIO_DEFAULT//(TK_PRIO_DEFAULT - 22)
#define RT_VSA_THREAD_PRIORITY		TK_PRIO_DEFAULT//(TK_PRIO_DEFAULT - 23)
#define RT_MDA_THREAD_PRIORITY		TK_PRIO_DEFAULT//(TK_PRIO_DEFAULT - 19)
#define RT_GPS_THREAD_PRIORITY		TK_PRIO_DEFAULT//(TK_PRIO_DEFAULT - 21)
#define RT_HI_THREAD_PRIORITY		TK_PRIO_DEFAULT//(TK_PRIO_DEFAULT - 25)
#define RT_WNETTX_THREAD_PRIORITY   TK_PRIO_DEFAULT//(TK_PRIO_DEFAULT - 20)
#define RT_WNETRX_THREAD_PRIORITY   TK_PRIO_DEFAULT//(TK_PRIO_DEFAULT - 20)
#define RT_NTRIP_THREAD_PRIORITY		TK_PRIO_DEFAULT//(TK_PRIO_DEFAULT - 21)

#define DEF_THREAD_STACK_SIZE   (1024*4096)

#define LESS_THREAD_STACK_SIZE   (1024*1024)


#define RT_SYS_THREAD_STACK_SIZE   DEF_THREAD_STACK_SIZE//(1024*8)
#define RT_GPS_THREAD_STACK_SIZE   DEF_THREAD_STACK_SIZE//(1024*8)
#define RT_MEMS_THREAD_STACK_SIZE  DEF_THREAD_STACK_SIZE//(1024*8)
#define RT_VAM_THREAD_STACK_SIZE   DEF_THREAD_STACK_SIZE//(1024*8)
#define RT_VSA_THREAD_STACK_SIZE   DEF_THREAD_STACK_SIZE//(1024*8)
#define RT_HI_THREAD_STACK_SIZE    DEF_THREAD_STACK_SIZE//(1024*8)
#define RT_NTRIP_THREAD_STACK_SIZE   DEF_THREAD_STACK_SIZE//(1024*8)

/**
    size of all the queue in system 
*/
#define SYS_QUEUE_SIZE 64
#define VAM_QUEUE_SIZE 20
#define VSA_QUEUE_SIZE 16
#define WNET_QUEUE_SIZE 20

/* per message len */
#define QUEUE_MSG_SIZE 512
#define VAM_MQ_MSG_SIZE 128
#define VSA_MQ_MSG_SIZE 128
#define SYS_MQ_MSG_SIZE 2048
#define WNET_MQ_MSG_SIZE 1280

enum SYSTEM_MSG_TYPE{
    SYS_MSG_BASE = 0x0000,
    SYS_MSG_INITED,
    SYS_MSG_ACL_CHECK,
    SYS_MSG_CHECK_VERSION,
    SYS_MSG_INFO_PUSH,

    SYS_MSG_SEND_CE,
    SYS_MSG_SEND_DRSTATE,
    SYS_MSG_SEND_CTRLINFO,
    SYS_MSG_SEND_COSVERSION,

    SYS_MSG_GET_MACKEY,

    SYS_MSG_REMOTE_OPEN,
    
    SYS_MSG_UPDATE_MACKEY,    
    SYS_MSG_UPDATE_P2PKEY,    
    SYS_MSG_UPDATE_BASECFG,
    SYS_MSG_UPDATE_TIMECFG,    
    SYS_MSG_UPDATE_ALERTCFG,    
    SYS_MSG_UPDATE_READERCFG,

    ZMQ_MSG_ID,
    
    SYS_MSG_ALARM_ACTIVE,
    SYS_MSG_XXX,
};

enum HI_OUT_TYPE{
    HI_OUT_NONE = 0,
    HI_OUT_SYS_INIT,
    HI_OUT_BSM_UPDATE,
    HI_OUT_BSM_NONE,
    HI_OUT_GPS_CAPTURED,
    HI_OUT_GPS_LOST,
    HI_OUT_CRD_ALERT,
    HI_OUT_CRD_CANCEL,
    HI_OUT_CRD_REAR_ALERT,
    HI_OUT_CRD_REAR_CANCEL,
    HI_OUT_VBD_ALERT,
    HI_OUT_VBD_CANCEL,
    HI_OUT_VBD_STATUS,
    HI_OUT_VBD_STOP,
    HI_OUT_EBD_ALERT,
    HI_OUT_EBD_CANCEL,
    HI_OUT_EBD_STATUS,
    HI_OUT_EBD_STOP,
    HI_OUT_CANCEL_ALERT,
};

enum HI_IN_TYPE{
    HI_IN_NONE = 0,
    HI_IN_KEY_PRESSED,
    HI_IN_KEY_RELEASE,
};


/**
    misc definitions 
*/
#define OS_TICK_PER_SECOND  (100)
#define MS_TO_TICK(n)     (n)
#define SECOND_TO_TICK(n) ((n)*1000)


/*****************************************************************************
 * declaration of structs                                                    *
*****************************************************************************/

/**
    structure of system global message 
*/
typedef struct _sys_msg{
    uint16_t id;
    uint16_t len;
    uint32_t argc; 
    void    *argv;
}sys_msg_t;

typedef enum _DOOR_CFG_INDEX {
    BASE_CFG_BASE = 0x200,

    /*base cfg  start*/
    BASE_CFG_DOOR_NO = 0x201,
    BASE_CFG_OPEN_TIME = 0x202,
    BASE_CFG_OPEN_TIME_DISABLE = 0x203,
    BASE_CFG_OPEN_DELAY = 0x204,
    BASE_CFG_CLOSE_LOCK = 0x205,
    BASE_CFG_MUL_CARD_DELY = 0x206,
    BASE_CFG_OUT_DOOR_BTN = 0x207,
    BASE_CFG_OUT_TZ_INDEX = 0x208,
    BASE_CFG_IN_TZ_INDEX = 0x209,
    BASE_CFG_GMC_CONNECT = 0x20A,
    BASE_CFG_COERCION_CODE = 0x20B,  
    /*base cfg end*/

    TIME_CFG_DOOR_NO = 0x301,
    TIME_CFG_MON = 0x302,
    TIME_CFG_DOOR_TUE = 0x303,
    TIME_CFG_DOOR_WEN = 0x304,
    TIME_CFG_DOOR_THU = 0x305,
    TIME_CFG_DOOR_FRI = 0x306,
    TIME_CFG_DOOR_SAT = 0x307,
    TIME_CFG_DOOR_SUN = 0x308,
    TIME_CFG_DOOR_HOL = 0x309,

    ALARM_DOOR_NO = 0x401,
    ALARM_INPUT   = 0x402,
    ALARM_CTL_DESTROY = 0x403,
    ALARM_FORCE_OP = 0x404,
    ALARM_LOSE_POWER = 0x405,
    ALARM_UPS_VOL  = 0x406,
    ALARM_SERVER_LOST = 0x407,
    ALARM_READER_LOST = 0x408,
    ALARM_READER_DESTROY= 0x409,
    ALARM_READER_CER_FAILED= 0x40A,
    ALARM_COERCION = 0x40B,


    ALARM_DURATION = 0x501,
    ALARM_CANCEL   = 0x502,
    ALARM_RELAY    = 0x503,
    ALARM_CTL_BEEP     = 0x504,
    ALARM_RDIN_BEEP  = 0x505,
    ALARM_RDOUT_BEEP = 0x506,
    ALARM_DOOR_STATE = 0x507,


    READER1_CFG = 0x601,
    READER2_CFG = 0x602,
    READER3_CFG = 0x603,
    READER4_CFG = 0x604,
    READER5_CFG = 0x605,
    READER6_CFG = 0x606,
    READER7_CFG = 0x607,
    READER8_CFG = 0x608,
    CTL_SYS_CFG = 0x609,
    LEGACY_WG_CFG = 0x60A,
        
} E_DOOR_CFG_INDEX;


typedef enum _USB_COMM_STATE {
    
    USB_COMM_STATE_DEFAULT = 0,
    USB_COMM_STATE_IDLE,
    USB_COMM_STATE_INIT,
    USB_COMM_STATE_INIT_END,
    USB_COMM_STATE_POLL,
    USB_COMM_STATE_CFG,
    USB_COMM_STATE_RDCFG,
    USB_COMM_STATE_PUSH,        
    USB_COMM_STATE_TRANSFER,
    USB_COMM_STATE_REMOTE,
    USB_COMM_STATE_ACL ,
    USB_COMM_STATE_INVALID,
    USB_COMM_STATE_LOG,
    
    USB_COMM_STATE_MACKEY,
    USB_COMM_STATE_P2P,
    USB_COMM_STATE_VERSION,
    USB_COMM_CTRL_INFO,

    USB_COMM_ALARM_OPEN,
    USB_COMM_REMOTE_OPEN,

    USB_COMM_ID_READ,
    
    USB_COMM_ID_DOOR_SERVER,
} E_USB_COMM_STATE;

typedef enum _DOOR_STATE {
    
    DOOR_STATE_INIT = 0,
    DOOR_STATE_OPEN,
    DOOR_STATE_CLOSE,
    DOOR_STATE_NORMAL,
    
} E_DOOR_STATE;

typedef enum _DAY {
    
    DAY_MON,
    DAY_TUE,
    DAY_WEN,
    DAY_THU,
    DAY_FRI,
    DAY_SAT,
    DAY_SUN,
    DAY_HOL,
} E_DAY;

typedef enum _UBUS_INTERFACE {
    
    UBUS_SERVER_MACKEY = 0x0101,
    UBUS_SERVER_P2P = 0x0102,
    UBUS_SERVER_TMSYNC = 0x0103,
    UBUS_SERVER_BASE_CFG = 0x0201,
    UBUS_SERVER_TIME_CFG = 0x0301,
    UBUS_SERVER_ALARM_CFG = 0x0401,
    UBUS_SERVER_READER_CFG = 0x0601,

    
    UBUS_SERVER_UPGRADE_COSVERSION = 0x0026,

    
    UBUS_SERVER_CLEAR_ALARM = 0x0023,
    
    UBUS_SERVER_ALARM = 0x002B,
    
    UBUS_SERVER_REMOTE = 0x0B02,
} E_UBUS_INTERFACE;


typedef enum _UBUS_CLIENT {

    
    UBUS_CLIENT_GETRTC = 0x2000,

    UBUS_CLIENT_SENDCE = 0x2002,
        
    UBUS_CLIENT_GETWL = 0x3001,
    UBUS_CLIENT_GETWLCFG = 0x3002,

    UBUS_CLIENT_GETP2P = 0x4003,
    UBUS_CLIENT_GETMAC = 0x4004,
    
    UBUS_CLIENT_SENDVERSION = 0x4002,

    UBUS_CLIENT_LOG = 0x4001,

    UBUS_CLIENT_AUDIT_LOG = 0x4005,
    
    UBUS_CLIENT_AUDIT_WL = 0x4006,


    UBUS_CLIENT_SEND_ALARM = 0x9002,

    UBUS_CLIENT_SEND_DOOR_INFO = 0x9003,
} E_UBUS_CLIENT;


typedef enum _SN_322 {
    
    SN_322_1 = 0,
    SN_322_2,
    SN_322_3,
    SN_322_4,
    
    SN_322_MX,
} E_SN_322;

typedef enum _INIT_MASK {
    
    INIT_MASK_CE  = 0x01,
    INIT_MASK_MAC = 0x02,
    INIT_MASK_P2P = 0x04,
    INIT_MASK_VERSION = 0x08,
    INIT_MASK_BASECFG = 0x10,    
    INIT_MASK_CRLCFG = 0x20,

    
} E_INIT_MASK;



#define MAX_322_NUM SN_322_MX

#define CEPATH_PR11 "/tmp/322ce/pr11ce"
#define CEPATH_322 "/tmp/322ce/322ce"
#define CEPATH_CTRLINFO "/tmp/322ce/controller_info"
#define msleep(n) usleep(n*1000)
typedef struct _key_buffer {
    int len;
    unsigned char *data;
} key_buffer_t;


typedef struct _config_base_door {
    uint8_t open_time;/*开门时间1-10s*/
    uint8_t open_time_disable;/*开门时间(残疾人)1-60s*/
    uint8_t open_delay;/*开门延迟时间1-60s*/
    uint8_t close_lock;/*闭门回锁,0x01=  是,0x02=否*/
    uint8_t mul_card_dely;/*多卡开门刷卡延迟,1-60s*/
    uint8_t out_door_btn;/*出门按钮连接方式,0x01常闭,0x02常开*/
    uint16_t out_timezone_index;/* 出门有效时区索引*/
    uint16_t in_timezone_index;/*进门有效时区索引*/
    uint8_t gmc_connect;/*门磁连接方式,0x01常闭,0x02常开*/
    int  coercion_code;/*8位数字,胁迫密码*/
} config_door_base_t;

typedef struct _config_time_door {

    uint8_t days;/*0~6 = week1~week7 7 = holiday*/

    short len;

    unsigned char  *data_buffer;
    
} config_time_door_t;



typedef struct _config_door {
    
    uint8_t door_no;

/*base cfg*/
    uint8_t cfg_base_len;

    uint8_t* cfg_base_buffer;
/*end*/

/*time cfg*/
    config_time_door_t cfg_time_door[8];
/*end*/

/*alarm cfg*/
    
/*end*/   
} config_door_t;



typedef struct _eg_door {
    uint8_t door_index;
    unsigned char usb_port[16];    
    uint8_t d21_322; /*0-no exist 1-exist*/
    uint8_t d21_pr11;/*0-no exist 1-exist*/
    config_door_t base_conf;/* 门禁基础配置*/   
} eg_door_t;


typedef struct _reader_cfg {
    uint8_t reader_no;
    uint8_t reader_doorno;
    uint8_t reader_direct;
    uint16_t reader_area;    
} reader_cfg_t;

typedef struct _card_data {
    uint8_t card_type;
    uint8_t card_auth;
    //uint8_t cardid_len;
    uint8_t carid[64];
} card_data_t;



typedef struct _pr11 {
    /*attribute  start*/
    
    /*attribute end*/

    /*cfg*/
    /*cfg*/
} pr11_t;

typedef struct _usb_ccid_322 {

    uint8_t ccid322_exist;
    uint8_t door_index;/*0= no exist, 1= exist*/
    uint8_t ccid322_index;    
    uint8_t init_flag;//init mask
    unsigned char usb_port[16];
    unsigned char pid_322[4];
    unsigned char sn_pr11[16];
    unsigned char pid_pr11[4];
    uint8_t  now_door_state[2];
    uint8_t  pre_door_state[2];  
    
    
    /*state machine*/
    E_USB_COMM_STATE usb_state;
    uint8_t toggle_state;
    uint8_t  toggle;
    uint8_t toggle_transmit;
    uint8_t toggle_alarm;
    /**/

    /*zmq start*/
    void *context;
    void *zmq_client;
    void *zmq_server;
    void *zmq_answer;
    void *zmq_sn;
    unsigned char *zmq_cli_addr;
    unsigned char *zmq_server_addr;   
    unsigned char *zmq_answer_addr;
    int zmq_len;
    uint8_t zmq_buffer[10240];
	zmq_pollitem_t pollitems;
    /*zmq end*/
    
    /*os task start*/
    osal_task_t   *task_322;
    osal_queue_t *queue_322;

    osal_timer_t *timer_322;
    
    osal_sem_t *sem_322;

    osal_sem_t *sem_state;
    /*os task end*/
    
} usb_ccid_322_t;

typedef struct _msg_manager {
        /*os task start*/
    osal_task_t   *task_msg;
    osal_queue_t *queue_msg;

    osal_timer_t *timer_msg;

    osal_sem_t *sem_msg;
    /*os task end*/
} msg_manager_t;

#pragma pack(4)

typedef struct _Controller {

/*322 obj */
    usb_ccid_322_t usb_ccid_322[MAX_322_NUM + 1];
    uint8_t cnt_322;
    msg_manager_t msg_manager;

/*obj end*/

/*key*/
    key_buffer_t mackey;
    key_buffer_t p2pkey;
   
/*key*/
/*alarm*/
    uint8_t rtc_encrypt[16];
    uint8_t alarm_buffer[64];
    uint8_t remote_buffer[1200];
/*alarm*/
/*cfg*/
    key_buffer_t basecfg;
    key_buffer_t ctlcfg;
    reader_cfg_t reader_cfg[8];
    uint8_t sys_ctrl;
    uint8_t legacy_WG;
/*cfg end*/    
} Controller_t;

/**
    structure of system configure parameters 
*/

/** 
    structure of system manager module's environment variable 
*/

extern Controller_t controll_eg,*p_controll_eg;

static inline uint16_t cv_ntohs(uint16_t s16)
{
	uint16_t ret = 0;
	uint8_t *s, *d;

	#ifndef __LITTLE_ENDIAN	
	ret = s16;
	#else
	s = (uint8_t *)(&s16);
	d = (uint8_t *)(&ret) + 1;
	#endif

	*d-- = *s++;
	*d-- = *s++;

	return ret;
}

static inline uint32_t cv_ntohl(uint32_t l32)
{
	uint32_t ret = 0;
	uint8_t *s, *d;

	//#ifdef BIG_ENDIAN	
	#ifndef __LITTLE_ENDIAN	
	ret = l32;
	#else
 	s = (uint8_t *)(&l32);
 	d = (uint8_t *)(&ret) + 3;
	#endif

	*d-- = *s++;
	*d-- = *s++;
	*d-- = *s++;
	*d-- = *s++;

	return ret;
}

static inline float cv_ntohf(float f32)
{
	float ret;
	uint8_t *s, *d;

    #ifndef __LITTLE_ENDIAN	
	ret = f32;
	#else
	s = (uint8_t *)(&f32);
	d = (uint8_t *)(&ret) + 3;
	#endif

	*d-- = *s++;
	*d-- = *s++;
	*d-- = *s++;
	*d-- = *s++;

	return ret;
}


/*****************************************************************************
 * declare of global functions and variables                                 *
*****************************************************************************/
/*
extern cms_global_t cms_envar, *p_cms_envar;
extern cfg_param_t cms_param, *p_cms_param;

osal_status_t sys_add_event_queue(sys_envar_t *p_sys, 
                             uint16_t msg_id, 
                             uint16_t msg_len, 
                             uint32_t msg_argc,
                             void    *msg_argv);
osal_status_t hi_add_event_queue(sys_envar_t *p_sys, 
                             uint16_t msg_id, 
                             uint16_t msg_len, 
                             uint32_t msg_argc,
                             void    *msg_argv);
*/
osal_status_t sys_add_event_queue(msg_manager_t *p_sys, 
                             uint16_t msg_id, 
                             uint16_t msg_len, 
                             uint32_t msg_argc,
                             void    *msg_argv);

void log_message (char *name, int priority, const char *format, ...);


#endif /* __CV_CMS_DEF_H__ */

