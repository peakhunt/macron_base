#ifndef __TRACE_DEF_H__
#define __TRACE_DEF_H__

#include "common_def.h"
#include "debug_log.h"
#include "hex_dump.h"
#include "app_trace.h"

#define TRACE_COMP(__component__)           (__TRACE__ ## __component__)
#define TRACE_ON(__component__)             trace_on(TRACE_COMP(__component__))
#define TRACE_OFF(__component__)            trace_off(TRACE_COMP(__component__))
#define TRACE_IS_ON(__component__)          trace_is_on(TRACE_COMP(__component__))

#ifdef TRACE_ENABLED

#define TRACE(__component__, fmt,...)                                                 \
  if(TRACE_IS_ON(__component__))                                                      \
  {                                                                                   \
    debug_log("%s:%s:%d:"fmt, #__component__, __FILE__, __LINE__,  ##__VA_ARGS__);    \
  }

#define TRACE_DUMP(__component__, title, buf, len)                \
  if(TRACE_IS_ON(__component__))                                  \
  {                                                               \
    hex_dump_buffer(title, #__component__, buf, len);             \
  }

#else

#define TRACE(__component__, fmt,...)
#define TRACE_DUMP(__component__, title, buf, len)

#endif

#define TRACE_DEF(__component__)           __TRACE__ ## __component__

enum {
  TRACE_DEF(MAIN),
  TRACE_DEF(TEST),
  TRACE_DEF(TASK),
  TRACE_DEF(SOCK_ERR),
  TRACE_DEF(SERIAL),
  TRACE_DEF(STREAM),
  TRACE_DEF(CLI_TELNET),
  TRACE_DEF(CLI_SERIAL),
  TRACE_DEF(CLI),
  TRACE_DEF(MB_TCP_SLAVE),
  TRACE_DEF(MBAP),
  TRACE_DEF(MB_RTU_SLAVE),
  TRACE_DEF(MB_RTU_MASTER),
  TRACE_DEF(MB_TCP_MASTER),
  TRACE_DEF(MB_MASTER),
  APP_TRACE_DEFS
  __TRACE_MAX,
} trace_item_t;

#define NUM_TRACE_ARRAY         (1 + __TRACE_MAX / 32)

extern void trace_init(uint32_t* inits, int num_elem);
extern void trace_on(uint32_t);
extern void trace_off(uint32_t);
extern bool trace_is_on(uint32_t);
extern int  trace_get_str_ndx(const char* name);
extern const char** trace_get_strs(int* num_elem);

#endif /* !__TRACE_DEF_H__ */
