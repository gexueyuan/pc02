#ifndef _OSAL_SEM_H_
#define _OSAL_SEM_H_

#include "os_core.h"
#include "osal_cmn.h"

#define OSAL_NO_WAIT                 (0)
#define OSAL_WAITING_FOREVER         (-1)

osal_sem_t *     osal_sem_create(const char* name, uint32_t count);
osal_status_t    osal_sem_delete(osal_sem_t *sem_id);
osal_status_t    osal_sem_wait(osal_sem_t sem_id, uint32_t timeout_msec);
osal_status_t    osal_sem_post(osal_sem_t sem_id);

#endif

