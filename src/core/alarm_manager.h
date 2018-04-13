#ifndef __ALARM_MANAGER_DEF_H__
#define __ALARM_MANAGER_DEF_H__

#include "alarm.h"

extern void alarm_manager_init(void);
extern void alarm_manager_add_alarm(alarm_t* alarm);
extern void alarm_manager_update(void);

#endif /* !__ALARM_MANAGER_DEF_H__ */
