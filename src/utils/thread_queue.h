#ifndef __THREAD_QUEUE_DEF_H__
#define __THREAD_QUEUE_DEF_H__

#include <pthread.h>
#include "common_def.h"
#include "list.h"

typedef struct
{
  struct list_head        q;
  uint32_t                count;
  pthread_mutex_t         q_lock;
} thread_queue_t;

extern void thread_queue_init(thread_queue_t* tq);
extern void thread_queue_lock(thread_queue_t* tq);
extern void thread_queue_unlock(thread_queue_t* tq);
extern void thread_queue_add(thread_queue_t* tq, struct list_head* qi);

#endif /* !__THREAD_QUEUE_DEF_H__ */
