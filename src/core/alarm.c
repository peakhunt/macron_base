#include <stdio.h>
#include <stdlib.h>
#include "alarm.h"

alarm_t*
alarm_alloc(uint32_t alarm_num, uint32_t chnl_num, alarm_severity_t severity,
    alarm_trigger_t trigger_type, alarm_setpoint_t set_point, uint32_t delay)
{
  alarm_t*    alarm;

  alarm = malloc(sizeof(alarm_t));

  alarm->alarm_num    = alarm_num;
  alarm->chnl_num     = chnl_num;
  alarm->state        = alarm_state_inactive;
  alarm->delay_state  = alarm_delay_state_idle;
  alarm->severity     = severity;
  alarm->trigger_type = trigger_type;
  alarm->set_point    = set_point;
  alarm->delay        = delay;

  return alarm;
}

alarm_t*
alarm_alloc_digital(uint32_t alarm_num, uint32_t chnl_num, alarm_severity_t severity,
    bool set_point, uint32_t delay)
{
  alarm_setpoint_t p;

  p.b = set_point;

  return alarm_alloc(alarm_num, chnl_num, severity, alarm_trigger_digital, p, delay);
}

alarm_t*
alarm_alloc_analog(uint32_t alarm_num, uint32_t chnl_num, alarm_severity_t severity,
    bool high, float set_point, uint32_t delay)
{
  alarm_setpoint_t p;

  p.f = set_point;

  return alarm_alloc(alarm_num, chnl_num, severity, high ? alarm_trigger_high : alarm_trigger_low, p, delay);
}

void
alarm_update(alarm_t* alarm)
{
  // FIXME
  switch(alarm->state)
  {
  case alarm_state_inactive:
    break;

  case alarm_state_pending:
    break;

  case alarm_state_inactive_pending:
    break;

  case alarm_state_active:
    break;
  }
}

void
alarm_ack(alarm_t* alarm)
{
  // FIXME
  switch(alarm->state)
  {
  case alarm_state_inactive:
    break;

  case alarm_state_pending:
    break;

  case alarm_state_inactive_pending:
    break;

  case alarm_state_active:
    break;
  }
}
