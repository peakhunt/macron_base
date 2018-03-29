#include "thread.h"

static pthread_key_t      _key;

void
thread_setspecific(void* s)
{
  pthread_setspecific(_key, s);
}

void*
thread_getspecific(void)
{
  return pthread_getspecific(_key);
}

void
thread_specific_init(void)
{
  pthread_key_create(&_key, NULL);
}
