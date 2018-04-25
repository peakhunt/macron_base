#ifndef __ALARM_DEF_H__
#define __ALARM_DEF_H__

#include <time.h>
#include "common_def.h"
#include "list.h"
#include "bhash.h"
#include "channel.h"
#include "publish_observe.h"

typedef enum
{
  alarm_state_inactive,
  alarm_state_active_pending,
  alarm_state_inactive_pending,
  alarm_state_active
} alarm_state_t;

typedef enum
{
  alarm_delay_state_occurring,
  alarm_delay_state_clearing,
  alarm_delay_state_occurred,
  alarm_delay_state_cleared,
} alarm_delay_state_t;

typedef enum
{
  alarm_severity_minor,
  alarm_severity_major,
  alarm_severity_critical
} alarm_severity_t;

typedef enum
{
  alarm_trigger_digital,
  alarm_trigger_low,
  alarm_trigger_high
} alarm_trigger_t;

typedef union
{
  double      f;
  bool        b;
} alarm_setpoint_t;

typedef struct
{
  struct list_head      le;
  struct list_head      le_for_active;
  BHashElement          hash_by_alarm_num;

  uint32_t              alarm_num;
  uint32_t              chnl_num;
  alarm_state_t         state;
  alarm_delay_state_t   delay_state;
  alarm_severity_t      severity;
  alarm_trigger_t       trigger_type;
  alarm_setpoint_t      set_point;

  uint32_t              delay;
  unsigned long         start_time;

  time_t                occur_time;

  observer_t            chnl_obs;
} alarm_t;

extern alarm_t* alarm_alloc(uint32_t alarm_num, uint32_t chnl_num, alarm_severity_t severity,
    alarm_trigger_t trigger_type, alarm_setpoint_t set_point, uint32_t delay);
extern alarm_t* alarm_alloc_digital(uint32_t alarm_num, uint32_t chnl_num, alarm_severity_t severity,
    bool set_point, uint32_t delay);
extern alarm_t* alarm_alloc_analog(uint32_t alarm_num, uint32_t chnl_num, alarm_severity_t severity,
    bool high, double set_point, uint32_t delay);

extern void alarm_update(alarm_t* alarm, channel_eng_value_t v);
extern void alarm_ack(alarm_t* alarm);
extern const char* alarm_get_string_state(alarm_state_t state);

#endif /* !__ALARM_DEF_H__ */
