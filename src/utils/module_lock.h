#ifndef __MODULE_LOCK_DEF_H__
#define __MODULE_LOCK_DEF_H__

#include "pthread.h"

#define MODULE_LOCK_NAME(name)    _module_lock_ ## name

#define MODULE_LOCK_DEF(name)     \
static pthread_mutex_t _module_lock_ ## name

#define MODULE_LOCK(name)\
  pthread_mutex_lock(&(MODULE_LOCK_NAME(name))

#define MODULE_UNLOCK(name)\
  pthread_mutex_unlock(&(MODULE_LOCK_NAME(name))

#endif /* !__MODULE_LOCK_DEF_H__ */
