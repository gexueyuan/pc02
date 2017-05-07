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




#define USB_FAILED    0x6E00
#define USB_OK        0x9000 
#define EUSB_SEND_PERIOD  500 



#define VENDOR_ID 0x1780  
#define PRODUCT_ID 0x0312


const static int TIMEOUT=1000; /* timeout in ms */  

#define msleep(n) usleep(n*1000)


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
    CARDPOLLEVENT_TRUE_CARD_CPU_NO_LOGEVENT,
    CARDPOLLEVENT_TRUE_CARD_15693_NO_LOGEVENT,
    CARDPOLLEVENT_TRUE_CARD_ID_NO_LOGEVENT,
    CARDPOLLEVENT_TRUE_CARD_CPU_WITH_LOGEVENT,
    CARDPOLLEVENT_TRUE_CARD_15693_WITH_LOGEVENT,
    CARDPOLLEVENT_TRUE_CARD_ID_WITH_LOGEVENT,
    CARDPOLLEVENT_FALSE_CARD_CPU_WITH_LOGEVENT,
    CARDPOLLEVENT_FALSE_CARD_CPU_NO_LOGEVENT,
} E_CARDPOLLEVENT;





const uint8_t car_detect[] = {0xFC,0xA0,0x00,0x00,0x05,0x80,0x30,0x00,0x00,0x00};

const uint8_t time_head[] = {0x80,0xA0,0x00,0x00,0x18,0x80,0x34,0x00,0x00,0x13};

const uint8_t refuse_enter[] = {0xFC,0xA0,0x04,0x00,0x09,0x80,0x32,0x80,0x00,0x04,  0x01,0x00,0x01,0x00};

uint8_t test_all[] = {0xFC,0xA0,0x04,0x00,  /*总长*/0x23  ,0x80,0x34,0x00,0x00, /*长度*/0x18 /**温度范围**/ ,0x02,0x00,0x09,0x32,0x38,0xA1,0xE6,0x2F,0x31,0x35,0xA1,0xE6,\
                      0x03,0x00,0x04  ,0x32,0x35,0xA1,0xE6,/*天气*/0x04,0x00,0x01,0x01,/*位置北京*/0x05,0x00,0x04,0xB1,0xB1,0xBE,0xA9};

unsigned char  change_app[25] = {0xFC,0xA0,0x04,0x00,0x14,0x00,0xA4,0x04,0x00,0x0F,0x74,0x64,0x72,0x6F,0x6E,0x99,0x05,0x69,0x64,0x52,0x64,0x51,0x34,0x42,0x31};

E_CARDPOLLEVENT card_event;

uint8_t  test_cmd[] = {0xFC,0xC1, 0x01 ,0x00, 0x06,0x80,0xB0,0x55,0xAF,0x01,0x00};

 /*透传头*/
 const unsigned char transfer_head[] = {0xFC,0xA0,0x00,0x00};//+len+data

const uint8_t cfg_base_head[] = {0x00,0x25,0x00,0x00};//+len+data


typedef enum _SN_322 {
    
    SN_322_1 = 0,
    SN_322_2,
    SN_322_3,
    SN_322_4,
    
    SN_322_MX,
} E_SN_322;

#define MAX_322_NUM SN_322_MX
                                            /*0*/   /*1*/   /*2*/   /*3*/
unsigned char* usb_port_def[MAX_322_NUM] = {"1-1.2","1-1.3","1-1.4","1-1.5"};


/*alloc index for 322*/
int alloc_322_index(unsigned char * port_name)
{
    unsigned char* p_str;

    int i = 0;

    for(i = 0;i < MAX_322_NUM;i++){

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
    printf("Local   time   is   %s\n",asctime(timenow));

    printf("time is %s\n",time_acs);
    memcpy(time_ptr,time_head,sizeof(time_head));
    memcpy(time_ptr + sizeof(time_head),time_acs,sizeof(time_acs));
    return 0;
}

void print_rec(unsigned char* rec,int len)
{
    int i;
    printf("\nrecv data len is :%d\n\r",len);
    for(i = 0;i < len;i++){
    
        printf("%02X",rec[i]);
    }
    printf("\n\r");


}


void print_send(unsigned char* send,int len)
{
    int i;
    printf("\nsend data len is :%d\n\r",len);
    for(i = 0;i < len;i++){
    
        printf("%02X ",send[i]);
    }
    printf("\n\r");


}

unsigned char lu_test[] = {0x00,0x84,0x00,0x00,0x08};
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
    unsigned char acl_data[256] = {0};
    unsigned char acl_cfg[256] = {0};
	unsigned char output[2048] = {0};
    unsigned char usb_port[32] = {0};
    int acl_len = 0;
    unsigned char send_data[64] = {0};
    unsigned char recv_data[64] = {0};

    usb_ccid_322_t *p_usb_ccid;

    p_usb_ccid = (usb_ccid_322_t*)parameter;
    
	void * context = luareader_new(0, NULL, NULL);

    //strcpy(usb_port,(const char*)parameter);
    ret = luareader_connect(context, p_usb_ccid->usb_port);
    if(ret < 0){
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "connect to  %s failed\n",p_usb_ccid->usb_port);
        goto out;
    }
    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "connect to  %s succeed\n",p_usb_ccid->usb_port);
    
while(1){

#if 1
    memset(output,0,sizeof(output));

    switch (p_usb_ccid->usb_state) {
        
    case USB_COMM_STATE_INIT:
        
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "%s init fished\n",p_usb_ccid->usb_port);


        p_usb_ccid->usb_state = USB_COMM_STATE_POLL;
        break;
        
    case USB_COMM_STATE_POLL:
          ret = luareader_transmit(context, car_detect, sizeof(car_detect), output, sizeof(output));
          OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_TRACE, "polling\n");
          print_rec(output,ret);
          
          if(output[1] == 0x01){//[0] =  reader num,
            
            if(output[2] != 0x02){

                memset(acl_data,0,sizeof(acl_data));
                //get acl data
                //get_sd(recv_data[0],recv_data[2],&recv_data[5],acl_data,&acl_len);//recv_data[4] = id length
                if(0 == acl_len){
                    printf("this card is not in acl,refuse!\n");
                    p_usb_ccid->usb_state = USB_COMM_STATE_IDLE;
                    break;
                }else{
                    print_rec(acl_data,acl_len);
                    p_usb_ccid->usb_state = USB_COMM_STATE_ACL;
                }
                printf("other card!\n");
                break;
            }
            else{//身份证

                printf("身份证\n");
                //check_IDcard();
                
                p_usb_ccid->usb_state = USB_COMM_STATE_TRANSFER;
                break;
            }

            
          }
          usb_state = USB_COMM_STATE_IDLE;
          break;
          
   case USB_COMM_STATE_ACL:

         p_usb_ccid->usb_state = USB_COMM_STATE_IDLE;//USB_COMM_STATE_TRANSFER;//
         break;

   case USB_COMM_STATE_INVALID:
              
        break;

   case USB_COMM_STATE_CFG:

        
        break;

    case USB_COMM_STATE_PUSH:
        

        get_sys_time(send_data);
        //x_TransmitApdu_hid_hs(&USB_HID_1,send_data,sizeof(time_head)+20,recv_data,&rec_len,timeout);
        ret = luareader_transmit(context, send_data, sizeof(time_head)+20, output, sizeof(output));
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_TRACE, "push time\n");
        print_send(send_data,sizeof(time_head)+20);
        print_rec(output,ret);
        
        //x_TransmitApdu_hid_hs(&USB_HID_1,test_all,sizeof(test_all),recv_data,&rec_len,timeout);
        ret = luareader_transmit(context, test_all, sizeof(test_all), output, sizeof(output));
        printf("send weather data!\n");
        print_send(test_all,sizeof(test_all));
        print_rec(recv_data,ret);
        
        p_usb_ccid->usb_state = USB_COMM_STATE_IDLE;
        break;
        
    case USB_COMM_STATE_IDLE:
        msleep(100);
        printf("IDLE!\n");
        //x_TransmitApdu_hid_hs(&USB_HID_1,usb_poll,sizeof(usb_poll),recv_data,&rec_len,timeout);
        //usb_state = USB_COMM_STATE_POLL;
        continue;
        break;
    case USB_COMM_STATE_TRANSFER:
        msleep(100);
        p_usb_ccid->usb_state = USB_COMM_STATE_TRANSFER;
        printf("in transfer mode\n");
        break;
    default:
        break;
    }
#else
sleep(2);
#endif
}





//{

//	ret = luareader_transmit(context, lu_test, sizeof(lu_test), output, sizeof(output));
//    
//    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "luareader_transmit(%p)=%d\n", context, ret);

//    sleep(2);

//}


out: 
        ret = luareader_disconnect(context);
    
        memset(output, 0, sizeof(output));
        ret = luareader_pop_value(context, (char *)output, sizeof(output));
        printf("luareader_pop_value(%p)=%d(%s)\n", context, ret, output);
    
        luareader_term(context);
        return ret;


}






/*****************************************************************************
 @funcname: eg_usbto322_thread_entry
 @brief   : thread function
 @param   : void *parameter  
 @return  : void
*****************************************************************************/
void *eg_usb12_thread_entry(void *parameter)
{
    int ret = 0;
    int i = 0;  
	unsigned char output[2048] = {0};
    const static int timeout=2000; /* timeout in ms */ 
    
	void * context = luareader_new(0, NULL, NULL);
	
    ret = luareader_connect(context, "1-1.2");
    if(ret < 0){

    goto out;
    }
	printf("luareader_connect to 1-1.2(%p)=%d\n", context, ret);
while(1){

	ret = luareader_transmit(context, lu_test, sizeof(lu_test), output, sizeof(output));
	printf("luareader_transmit(%p)=%d\n", context, ret);
	for(i = 0 ; i < ret;i++){
	printf("1-1.2 return is 0x%X \n",output[i]);
    }

    sleep(2);

}


out: 
        ret = luareader_disconnect(context);
    
        memset(output, 0, sizeof(output));
        ret = luareader_pop_value(context, (char *)output, sizeof(output));
        printf("luareader_pop_value(%p)=%d(%s)\n", context, ret, output);
    
        luareader_term(context);
        return ret;


}

void *eg_usb13_thread_entry(void *parameter)
{
    int ret = 0;
    int i = 0;  
	unsigned char output[2048] = {0};
    const static int timeout=2000; /* timeout in ms */ 
    
	void * context = luareader_new(0, NULL, NULL);
	
    ret = luareader_connect(context, "1-1.3");
    if(ret < 0){

    goto out;
    }
	printf("luareader_connect to 1-1.3(%p)=%d\n", context, ret);
while(1){

	ret = luareader_transmit(context, (unsigned char*)"\x00\x84\x00\x00\x08", 5, output, sizeof(output));
	printf("luareader_transmit(%p)=%d\n", context, ret);
	for(i = 0 ; i < ret;i++){
	printf("1-1.3 return is 0x%X \n",output[i]);
    }

    sleep(2);

}


out: 
        ret = luareader_disconnect(context);
    
        memset(output, 0, sizeof(output));
        ret = luareader_pop_value(context, (char *)output, sizeof(output));
        printf("luareader_pop_value(%p)=%d(%s)\n", context, ret, output);
    
        luareader_term(context);
        return ret;


}    
void *eg_usb14_thread_entry(void *parameter)
{
    int ret = 0;
    int i = 0;  
    unsigned char output[2048] = {0};
    const static int timeout=2000; /* timeout in ms */ 
    
    void * context = luareader_new(0, NULL, NULL);
    
    ret = luareader_connect(context, "1-1.4");
    if(ret < 0){

    goto out;
    }
    printf("luareader_connect to 1-1.4(%p)=%d\n", context, ret);
while(1){

    ret = luareader_transmit(context, (unsigned char*)"\x00\x84\x00\x00\x08", 5, output, sizeof(output));
    printf("luareader_transmit(%p)=%d\n", context, ret);
    for(i = 0 ; i < ret;i++){
    printf("1-1.4 return is 0x%X \n",output[i]);
    }

    sleep(2);

}


out: 
        ret = luareader_disconnect(context);
    
        memset(output, 0, sizeof(output));
        ret = luareader_pop_value(context, (char *)output, sizeof(output));
        printf("luareader_pop_value(%p)=%d(%s)\n", context, ret, output);
    
        luareader_term(context);
        return ret;


}

void *eg_usb15_thread_entry(void *parameter)
{
    int ret = 0;
    int i = 0;  
    unsigned char output[2048] = {0};
    const static int timeout=2000; /* timeout in ms */ 
    
    void * context = luareader_new(0, NULL, NULL);
    
    ret = luareader_connect(context, "1-1.5");
    if(ret < 0){

    goto out;
    }
    printf("luareader_connect to 1-1.5(%p)=%d\n", context, ret);
while(1){

    ret = luareader_transmit(context, (unsigned char*)"\x00\x84\x00\x00\x08", 5, output, sizeof(output));
    printf("luareader_transmit(%p)=%d\n", context, ret);
    for(i = 0 ; i < ret;i++){
    printf("1-1.5 return is 0x%X \n",output[i]);
    }

    sleep(2);

}


out: 
        ret = luareader_disconnect(context);
    
        memset(output, 0, sizeof(output));
        ret = luareader_pop_value(context, (char *)output, sizeof(output));
        printf("luareader_pop_value(%p)=%d(%s)\n", context, ret, output);
    
        luareader_term(context);
        return ret;


}



uint8_t usb_wb;

void timer_usb_callback(void* parameter)
{

    usb_ccid_322_t *p_usb_timer = (usb_ccid_322_t *)parameter;
    
    printf("timer in port %s,index is %d\n",p_usb_timer->usb_port,p_usb_timer->ccid322_index);
    if(p_usb_timer->usb_state == USB_COMM_STATE_IDLE){

        
        if(p_usb_timer->toggle >= 10){

            p_usb_timer->toggle = 0;
            p_usb_timer->usb_state = USB_COMM_STATE_PUSH;
           // back_poll();
           printf("timer in port %s,index is %d\n",p_usb_timer->usb_port);
        }
        else{
            
            p_usb_timer->toggle++;
            p_usb_timer->usb_state = USB_COMM_STATE_POLL;
        }
    }

    if(p_usb_timer->usb_state == USB_COMM_STATE_TRANSFER){

        if(p_usb_timer->toggle_transmit >= 8){

            //back_poll();
            p_usb_timer->toggle_transmit = 0;
          }
        else
            p_usb_timer->toggle_transmit++;
    }
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

        if(dev_index >= MAX_322_NUM){

            OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "number of device is over %d\n",MAX_322_NUM);
            break;
        }

        
        if((output[i] == 0)&&(dev_index <= MAX_322_NUM)){
            
            memcpy(device_str[dev_index++],&output[j],i - j + 1);
            j = i + 1;
        }


    }
    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "there is  %d devices on pc02\n",dev_index);

    /*max device = 4*/
    dev_index = (dev_index < MAX_322_NUM ? dev_index : MAX_322_NUM);
    /**/
    
    for(i = 0;i < dev_index;i++){

        
        ret = alloc_322_index(device_str[i]);
        
        p_usb_ccid = &(controll_eg.usb_ccid_322[ret]);
        p_usb_ccid->ccid322_index = ret;
        p_usb_ccid->usb_state = USB_COMM_STATE_INIT;
        p_usb_ccid->ccid322_exist = 1;
        
        strcpy(&(p_usb_ccid->usb_port),device_str[i]);
            
        OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "create device pthread %s\n",device_str[i]);
        
        tid = osal_task_create(device_str[i],
                            eg_usb_thread_entry,
                            p_usb_ccid,RT_SYS_THREAD_STACK_SIZE, RT_SYS_THREAD_PRIORITY);
        
        osal_assert(tid != NULL);

        p_usb_ccid->timer_322 = osal_timer_create("tm-usb",timer_usb_callback,p_usb_ccid,\
                            EUSB_SEND_PERIOD, TIMER_INTERVAL|TIMER_STOPPED, TIMER_PRIO_NORMAL);
        osal_assert(p_usb_ccid->timer_322 != NULL);
        
        //osal_timer_start(p_usb_ccid->timer_322);
    

    }  
    
    
/*
    tid = osal_task_create("tk_usb_1.2",
                        eg_usb12_thread_entry,
                        NULL,RT_SYS_THREAD_STACK_SIZE, RT_SYS_THREAD_PRIORITY);

    osal_assert(tid != NULL)
        

    tid = osal_task_create("tk_usb_1.3",
                        eg_usb13_thread_entry,
                        NULL,RT_SYS_THREAD_STACK_SIZE, RT_SYS_THREAD_PRIORITY);

    osal_assert(tid != NULL)

    tid = osal_task_create("tk_usb_1.4",
                    eg_usb14_thread_entry,
                    NULL,RT_SYS_THREAD_STACK_SIZE, RT_SYS_THREAD_PRIORITY);

    osal_assert(tid != NULL)

    tid = osal_task_create("tk_usb_1.5",
                    eg_usb15_thread_entry,
                    NULL,RT_SYS_THREAD_STACK_SIZE, RT_SYS_THREAD_PRIORITY);

    osal_assert(tid != NULL)
*/

    
/*
    timer_usb = osal_timer_create("tm-usb",timer_usb_callback,NULL,\
                        EUSB_SEND_PERIOD, TIMER_INTERVAL|TIMER_STOPPED, TIMER_PRIO_NORMAL);
    osal_assert(timer_usb != NULL);
    
    osal_timer_start(timer_usb);
*/

    OSAL_MODULE_DBGPRT(MODULE_NAME, OSAL_DEBUG_INFO, "module initial finished\n");

}
