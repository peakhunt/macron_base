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

typedef struct
{
  uint32_t    interval;               // in milliseconds
  uint32_t    signal_log_keep;        // in hours
  uint32_t    alarm_log_keep;         // in days
  uint32_t    log_clean_period;       // in seconds
} logger_config_t;

extern void logger_init(void);

extern void logger_signal_log(uint32_t chnl, double v, unsigned long timestamp);
extern void logger_alarm_log(uint32_t alarm, logger_alarm_event_t evt);

extern void logger_signal_trace_set_chnls(uint32_t* chnls, uint32_t num_chnls);

#endif /*!__LOGGER_DEF_H__ */
