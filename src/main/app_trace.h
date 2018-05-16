#ifndef __APP_TRACE_DEF_H__
#define __APP_TRACE_DEF_H__

#include "trace.h"

#define APP_TRACE_DEFS      \
  TRACE_DEF(APP_START),     \
  TRACE_DEF(CLI_DRIVER),    \
  TRACE_DEF(WEBS_DRIVER),   \
  TRACE_DEF(WEBS_REQUEST),  \
  TRACE_DEF(MBS_DRIVER),    \
  TRACE_DEF(MBM_DRIVER),    \
  TRACE_DEF(CORE_DRIVER),   \
  TRACE_DEF(LOGGER), 


#define APP_TRACE_STRS  \
  "APP_START"           \
  "CLI_DRIVER"          \
  "WEBS_DRIVER"         \
  "WEBS_REQUEST"        \
  "MBS_DRIVER"          \
  "MBM_DRIVER"          \
  "CORE_DRIVER"         \
  "LOGGER"


#endif /*!__APP_TRACE_DEF_H__ */
