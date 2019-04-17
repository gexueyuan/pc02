/*****************************************************************************
 Copyright(C) Tendyron Corporation
 All rights reserved.
 
 @file   : main.c
 @brief  : main
 @author : gexueyuan
 @history:
           2016-12-22    gexueyuan    Created file
           ...
******************************************************************************/



#include "cv_osal.h"
#define OSAL_MODULE_DEBUG
#define OSAL_MODULE_DEBUG_LEVEL OSAL_DEBUG_INFO
#define MODULE_NAME "INIT"
#include "cv_osal_dbg.h"

#include "queue_msg.h"

#define RELEASE

#define FIRMWARE_VERSION "V1.2.6.6" 

#ifdef RELEASE
#define FIRMWARE_IDEN "rel" 
#else
#define FIRMWARE_IDEN "dbg" 
#endif
#include "cv_cms_def.h"

extern void osal_dbg_init(void);
extern void eg_usbto322_init(void);
extern void ubus_interface_init(void);
extern void msg_manager_init(void);
extern void eg_zmq_test(void);
extern void eg_monitor_init(void);

eg_global_var_t eg_envar,*p_eg_envar;

Controller_t controll_eg,*p_controll_eg;

char _version[200];

void get_version(void)
{
  osal_printf("\nPC02322 ver: %s[%s,%s %s]\n\n", FIRMWARE_VERSION, FIRMWARE_IDEN, __TIME__, __DATE__);

  sprintf(_version,"PC02322 ver: %s[%s,%s %s]",FIRMWARE_VERSION, FIRMWARE_IDEN, __TIME__, __DATE__);
}

    
void global_init(void)
{
    p_eg_envar = &eg_envar;

    memset(p_eg_envar,0,sizeof(eg_global_var_t));

    p_controll_eg = &controll_eg;

    memset(p_controll_eg,0,sizeof(Controller_t));

    
    system("rm -rf /tmp/322ce");

    system("mkdir /tmp/322ce");

    
    //get_version();
}


int main(int argc, char *argv[])
{
//    system("./hostapd -d /etc/hostapd.conf -B");

    if (argc > 1 && !strcmp(argv[1], "-v")){
        
        //printf("\n%s\n",_version);
        
        get_version();
        return 0;
        
    }
    global_init();

    ubus_interface_init();
    msg_manager_init();
    //sleep(1);
    eg_usbto322_init();

	
	eg_monitor_init();
    //eg_zmq_test();
    //eg_net_init();
    //acl_local_init();
    //eg_comm_init();
    //ubus_server_thread_entry(NULL);
    //msg_manager_init();
    printf("\ncome in main thread\n");
	signal(SIGINT, SIG_DFL);
    
	signal(SIGTERM, SIG_DFL);
    
    while (1){
        osal_sleep(1000);
		//hidapi_test();
		//osal_printf("\nFirm: %s[%s,%s %s]\n\n", FIRMWARE_VERSION, FIRMWARE_IDEN, __TIME__, __DATE__);
        
        //eg_tcp_client();  
        //test();
        if(controll_eg.cnt_322 == 0)
        return 0;

    }
}

