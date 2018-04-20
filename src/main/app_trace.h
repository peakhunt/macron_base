#ifndef __APP_TRACE_DEF_H__
#define __APP_TRACE_DEF_H__

#include "trace.h"

#define APP_TRACE_DEFS      \
  TRACE_DEF(APP_START),     \
  TRACE_DEF(APP_CLI),       \
  TRACE_DEF(APP_WEB),       \
  TRACE_DEF(APP_MB_SLAVE),  \
  TRACE_DEF(APP_MB_MASTER), \
  TRACE_DEF(APP_CORE), 


#define APP_TRACE_STRS  \
  "APP_START"           \
  "APP_CLI"             \
  "APP_WEB"             \
  "APP_MB_SLAVE"        \
  "APP_MB_MASTER"       \
  "APP_CORE"


#endif /*!__APP_TRACE_DEF_H__ */
