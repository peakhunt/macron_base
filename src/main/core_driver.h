#ifndef __CORE_DRIVER_DEF_H__
#define __CORE_DRIVER_DEF_H__

#include <stdio.h>
#include "channel.h"
#include "alarm.h"

typedef struct
{
  uint32_t            chnl_num;
  channel_type_t      chnl_type;
  channel_direction_t chnl_dir;
} core_driver_channel_config_t;

typedef struct
{
  uint32_t            alarm_num;
  uint32_t            chnl_num;
  alarm_trigger_t     trigger_type;
  alarm_setpoint_t    set_point;
  alarm_severity_t    severity;
  uint32_t            delay;
} core_driver_alarm_config_t;

extern void core_driver_init(void);

#endif /* !__CORE_DRIVER_DEF_H__ */
