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


#endif /* !__COMMON_DEF_H__ */
