#ifndef __TIME_UTIL_DEF_H__
#define __TIME_UTIL_DEF_H__

#include <time.h>

static inline unsigned long
time_util_get_sys_clock_in_ms(void)
{
  struct timespec now;
  unsigned long   ret;

  clock_gettime(CLOCK_MONOTONIC, &now);

  ret = now.tv_sec * 1000.0 +  now.tv_nsec / 1000000.0;
  return ret;
}

static inline unsigned long
time_util_get_sys_clock_elapsed_in_ms(long start)
{
  unsigned long now = time_util_get_sys_clock_in_ms();

  return now - start;
}

#endif /* !__TIME_UTIL_DEF_H__ */
