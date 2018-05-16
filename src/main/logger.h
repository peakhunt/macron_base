#ifndef __LOGGER_DEF_H__
#define __LOGGER_DEF_H__

#include "common_def.h"

#define LOGGER_DB     "db/logging_db.sq3"

typedef enum
{
  logger_alarm_event_occur,
  logger_alarm_event_ack,
  logger_alarm_event_clear,
} logger_alarm_event_t;

extern void logger_init(void);

extern void logger_signal_log(uint32_t chnl, double v);

extern void logger_signal_trace_set(uint32_t chnl);
extern void logger_signal_trace_clear(uint32_t chnl);

extern void logger_alarm_log(uint32_t alarm, logger_alarm_event_t evt);

#endif /*!__LOGGER_DEF_H__ */
