/*****************************************************************************
 Copyright(C) Beijing Carsmart Technology Co., Ltd.
 All rights reserved.
 
 @file   : cv_cms_def.h
 @brief  : This file include the cms global definition 
 @author : wangyifeng
 @history:
           2014-6-16    wangyifeng    Created file
           ...
******************************************************************************/
#ifndef __CV_CMS_DEF_H__
#define __CV_CMS_DEF_H__

#include "cv_osal.h"


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

#define DEF_THREAD_STACK_SIZE   (1024*8192)

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
#define SYS_QUEUE_SIZE 16
#define VAM_QUEUE_SIZE 20
#define VSA_QUEUE_SIZE 16
#define WNET_QUEUE_SIZE 20

/* per message len */
#define QUEUE_MSG_SIZE 512
#define VAM_MQ_MSG_SIZE 128
#define VSA_MQ_MSG_SIZE 128
#define SYS_MQ_MSG_SIZE 128
#define WNET_MQ_MSG_SIZE 1280

enum SYSTEM_MSG_TYPE{
    SYS_MSG_BASE = 0x0000,
    SYS_MSG_INITED,
    SYS_MSG_KEY_PRESSED,
    SYS_MSG_KEY_RELEASED,
    SYS_MSG_START_ALERT,
    SYS_MSG_STOP_ALERT,
    SYS_MSG_ALARM_ACTIVE,
    SYS_MSG_ALARM_CANCEL,
    SYS_MSG_GPS_UPDATE,
    SYS_MSG_BSM_UPDATE,
    SYS_MSG_HI_IN_UPDATE,
    SYS_MSG_HI_OUT_UPDATE,
    SYS_MSG_XXX,

    VAM_MSG_BASE = 0x0200,
    VAM_MSG_START,
    VAM_MSG_STOP,
    VAM_MSG_RCPTX,
    VAM_MSG_RCPRX,
    VAM_MSG_NEIGH_TIMEOUT,
    VAM_MSG_GPSDATA,


    VSA_MSG_BASE = 0x0300,
    VSA_MSG_MANUAL_BC,   
    VSA_MSG_EEBL_BC,
    VSA_MSG_AUTO_BC,
    
    VSA_MSG_CFCW_ALARM,
    VSA_MSG_CRCW_ALARM,
    VSA_MSG_OPPOSITE_ALARM,
    VSA_MSG_SIDE_ALARM,

    VSA_MSG_ACC_RC,
    VSA_MSG_EEBL_RC,
    VSA_MSG_X_RC,
    VSA_MSG_XX_RC,
    VSA_MSG_XXX_RC
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

    USB_COMM_STATE_IDLE = 0,
    USB_COMM_STATE_INIT,
    USB_COMM_STATE_POLL,
    USB_COMM_STATE_CFG,
    USB_COMM_STATE_PUSH,        
    USB_COMM_STATE_TRANSFER,
    USB_COMM_STATE_ACL ,
    USB_COMM_STATE_INVALID,
    USB_COMM_STATE_LOG,
} E_USB_COMM_STATE;



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

    uint16_t len;

    uint8_t * data_buffer;
    
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





typedef struct _pr11 {
    /*attribute  start*/
    
    /*attribute end*/

    /*cfg*/
    /*cfg*/
} pr11_t;

typedef struct _usb_ccid_322 {

    uint8_t ccid322_exist;
    uint8_t pr11_exist;/*0= no exist, 1= exist*/
    unsigned char usb_port[16];
    uint8_t ccid322_index;
    uint8_t door_cfg_index;
    
    /*state machine*/
    E_USB_COMM_STATE usb_state;
    uint8_t  toggle;
    uint8_t toggle_transmit;
    /**/
    
    /*os task start*/
    osal_task_t   *task_322;
    osal_queue_t *queue_322;

    osal_timer_t *timer_322;
    
    osal_sem_t *sem_322;
    /*os task end*/
    
} usb_ccid_322_t;



typedef struct _Controller {

/*322 obj */
    usb_ccid_322_t usb_ccid_322[4];
/*obj end*/

/*cfg*/
    config_door_t door_cfg[4];
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


#endif /* __CV_CMS_DEF_H__ */

