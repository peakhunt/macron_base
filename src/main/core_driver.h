#ifndef __CORE_DRIVER_DEF_H__
#define __CORE_DRIVER_DEF_H__

#include <stdio.h>
#include "channel.h"
#include "alarm.h"

typedef struct
{
  uint32_t            chnl_num;
  channel_type_t      chnl_type;
  channel_eng_value_t init_val;
  channel_eng_value_t failsafe_val;
  channel_direction_t chnl_dir;
  //
  // only for analog channels
  //
  double              min_val;
  double              max_val;
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

typedef struct
{
  uint32_t            loop_interval;
} core_driver_config_t;

typedef struct
{
  uint32_t        input_scan_min;
  uint32_t        input_scan_max;
  uint32_t        input_scan_avg;

  uint32_t        output_scan_min;
  uint32_t        output_scan_max;
  uint32_t        output_scan_avg;

  uint32_t        app_min;
  uint32_t        app_max;
  uint32_t        app_avg;
} core_driver_stat_t;

extern void core_driver_init(void);
extern void core_driver_get_stat(core_driver_stat_t* stat);

#endif /* !__CORE_DRIVER_DEF_H__ */
