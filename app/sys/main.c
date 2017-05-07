/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

/**
 * @addtogroup STM32
 */
/*@{*/


#include "cv_osal.h"
#define OSAL_MODULE_DEBUG
#define OSAL_MODULE_DEBUG_LEVEL OSAL_DEBUG_INFO
#define MODULE_NAME "INIT"
#include "cv_osal_dbg.h"

#include "queue_msg.h"

#define FIRMWARE_VERSION "V2.0.000" 
#ifdef RELEASE
#define FIRMWARE_IDEN "rel" 
#else
#define FIRMWARE_IDEN "dbg" 
#endif
#include "cv_cms_def.h"

extern void osal_dbg_init(void);


extern  void eg_usbto322_init();



//extern    void eg_net_init(void);

//extern    void acl_local_init(void);


eg_global_var_t eg_envar,*p_eg_envar;

Controller_t controll_eg,*p_controll_eg;


void global_init(void)
{
    p_eg_envar = &eg_envar;

    memset(p_eg_envar,0,sizeof(eg_global_var_t));

    p_controll_eg = &controll_eg;

    memset(p_controll_eg,0,sizeof(Controller_t));
    
    
}

int main(int argc, char *argv[])
{
//    system("./hostapd -d /etc/hostapd.conf -B");
    global_init();
    eg_usbto322_init();
    //eg_net_init();
    //acl_local_init();
    while (1){
        osal_sleep(1000);
		//hidapi_test();
		//osal_printf("\nFirm: %s[%s,%s %s]\n\n", FIRMWARE_VERSION, FIRMWARE_IDEN, __TIME__, __DATE__);
        
        //eg_tcp_client();  
        //test();
    }
}

void get_version(void)
{
  osal_printf("\nFirm: %s[%s,%s %s]\n\n", FIRMWARE_VERSION, FIRMWARE_IDEN, __TIME__, __DATE__);
}

