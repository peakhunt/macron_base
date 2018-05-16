#ifndef __LOGGER_DEF_H__
#define __LOGGER_DEF_H__

#include "common_def.h"

typedef enum
{
  logger_alarm_event_occur,
  logger_alarm_event_ack,
  logger_alarm_event_clear,
} logger_alarm_event_t;

extern void logger_init(void);
extern void logger_signal_log(uint32_t chnl, double f, bool b);
extern void logger_alarm_log(uint32_t alarm, logger_alarm_event_t evt);

#endif /*!__LOGGER_DEF_H__ */
