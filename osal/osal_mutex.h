#ifndef _OSAL_MUTEX_H_
#define _OSAL_MUTEX_H_

#include "os_core.h"
#include "osal_cmn.h"

osal_mutex_t *osal_mutex_create(const char *mut_name);
osal_status_t osal_mutex_delete(osal_mutex_t *mutex);
osal_status_t osal_mutex_lock(osal_mutex_t *mutex);
osal_status_t osal_mutex_unlock(osal_mutex_t *mutex);
osal_status_t osal_mutex_trylock(osal_mutex_t *mutex);

#endif

