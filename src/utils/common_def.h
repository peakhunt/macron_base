#ifndef __COMMON_DEF_H__
#define __COMMON_DEF_H__

#include <stdint.h>

typedef uint8_t bool;

#define TRUE          1
#define FALSE         0

#define CRASH()                 \
{                               \
  char*   __bullshit = NULL;    \
  *__bullshit = 0;              \
}

#define NARRAY(a)       (sizeof(a)/sizeof(a[0]))
#define UNUSED(a)       (void)(a)


#endif /* !__COMMON_DEF_H__ */
