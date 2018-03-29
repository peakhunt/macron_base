#ifndef __THREAD_DEF_H__
#define __THREAD_DEF_H__

#include <pthread.h>
#include "common_def.h"

extern void thread_setspecific(void* s);
extern void* thread_getspecific(void);
extern void thread_specific_init(void);

#endif /* !__THREAD_DEF_H__ */
