#include "thread_queue.h"

void
thread_queue_init(thread_queue_t* tq)
{
  INIT_LIST_HEAD(&tq->q);
  tq->count       = 0;

  pthread_mutex_init(&tq->q_lock, NULL);
}

void
thread_queue_lock(thread_queue_t* tq)
{
  pthread_mutex_lock(&tq->q_lock);
}

void
thread_queue_unlock(thread_queue_t* tq)
{
  pthread_mutex_unlock(&tq->q_lock);
}

void
thread_queue_add(thread_queue_t* tq, struct list_head* qi)
{
  INIT_LIST_HEAD(qi);

  thread_queue_lock(tq);

  list_add_tail(qi, &tq->q);
  tq->count += 1;

  thread_queue_unlock(tq);
}
