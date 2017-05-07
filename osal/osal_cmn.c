/*****************************************************************************
 Copyright(C) Tendyron Corporation
 All rights reserved.
 
 @file   : osal_cmn.c
 @brief  : common osal api
 @author : gexueyuan
 @history:
           2016-12-22    gexueyuan    Created file
           ...
******************************************************************************/
#include "osal_cmn.h"

uint64_t osal_current_time(void)
{
    uint64_t time = 0;

    time = os_tick_get();
    return time;
}

void osal_udelay(int32_t microseconds)
{
    os_delay(microseconds);
    return;
}

void osal_mdelay(int32_t milliseconds)
{
    os_delay(milliseconds*1000);
}

void osal_sleep(int32_t milliseconds)
{
    os_sleep(milliseconds);
    return;
}

/*lint -restore*/

