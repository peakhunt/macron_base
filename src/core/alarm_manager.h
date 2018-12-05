#ifndef __ALARM_MANAGER_DEF_H__
#define __ALARM_MANAGER_DEF_H__

#include "alarm.h"

typedef struct
{
  alarm_state_t         state;
  time_t                occur_time;
} alarm_status_t;

typedef struct
{
  alarm_setpoint_t    set_point;
  uint32_t            delay;
} alarm_runtime_config_t;

extern void alarm_manager_init(void);
extern void alarm_manager_add_alarm(alarm_t* alarm);
extern void alarm_manager_update(void);

extern bool alarm_manager_ack_alarm(uint32_t alarm_num);
extern int alarm_manager_get_alarm_status(uint32_t alarm_num, alarm_status_t* status);

extern bool alarm_manager_update_alarm_config(uint32_t alarm_num, alarm_runtime_config_t* cfg);

extern void alarm_manager_bind_alarm_vars(alarm_var_t* alarm_vars, int num_alarm_vars);

#endif /* !__ALARM_MANAGER_DEF_H__ */
