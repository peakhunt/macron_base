#ifndef __COMPLETION_DEF_H__
#define __COMPLETION_DEF_H__

#include <semaphore.h>

typedef struct
{
  sem_t       wait;
} completion_t;

static inline void
completion_init(completion_t* c)
{
  sem_init(&c->wait, 0, 0);
}

static inline void
completion_wait(completion_t* c)
{
  sem_wait(&c->wait);
}

static inline void
completion_signal(completion_t* c)
{
  sem_post(&c->wait);
}

static inline void
completion_reset(completion_t* c)
{
  sem_destroy(&c->wait);
  completion_init(c);
}

#endif /* !__COMPLETION_DEF_H__ */
