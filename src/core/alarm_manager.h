#ifndef __ALARM_MANAGER_DEF_H__
#define __ALARM_MANAGER_DEF_H__

#include "alarm.h"

typedef struct
{
  alarm_state_t         state;
} alarm_status_t;

extern void alarm_manager_init(void);
extern void alarm_manager_add_alarm(alarm_t* alarm);
extern void alarm_manager_update(void);

extern void alarm_manager_ack_alarm(uint32_t alarm_num);
extern int alarm_manager_get_alarm_status(uint32_t alarm_num, alarm_status_t* status);

#endif /* !__ALARM_MANAGER_DEF_H__ */
