#include <stdio.h>
#include <stdlib.h>
#include "alarm.h"
#include "time_util.h"
#include "trace.h"

typedef enum
{
  alarm_event_occur,
  alarm_event_clear,
  alarm_event_ack,
} alarm_event_t;

////////////////////////////////////////////////////////////////////////////////
//
// alarm state machine core
//
////////////////////////////////////////////////////////////////////////////////
static inline void
alarm_mark_occur_time(alarm_t* alarm)
{
  alarm->occur_time = time(NULL);
}

static inline void
alarm_move_state(alarm_t* alarm, alarm_state_t new_state)
{
  alarm->state = new_state;
}

static inline void
alarm_move_delay_state(alarm_t* alarm, alarm_delay_state_t new_state)
{
  alarm->delay_state = new_state;
}

static void
handle_alarm_event(alarm_t* alarm, alarm_event_t event)
{
  switch(alarm->state)
  {
  case alarm_state_inactive:
    if(event == alarm_event_occur)
    {
      alarm_move_state(alarm, alarm_state_active_pending);
      alarm_mark_occur_time(alarm);
    }
    break;

  case alarm_state_active_pending:
    if(event == alarm_event_ack)
    {
      alarm_move_state(alarm, alarm_state_active);
    }
    else if(event == alarm_event_clear)
    {
      alarm_move_state(alarm, alarm_state_inactive_pending);
    }
    break;

  case alarm_state_inactive_pending:
    if(event == alarm_event_ack)
    {
      alarm_move_state(alarm, alarm_state_inactive);
    }
    else if(event == alarm_event_occur)
    {
      alarm_move_state(alarm, alarm_state_active_pending);
    }
    break;

  case alarm_state_active:
    if(event == alarm_event_clear)
    {
      alarm_move_state(alarm, alarm_state_inactive);
    }
    break;
  }
}

static void
handle_alarm_state_machine(alarm_t* alarm, alarm_event_t event)
{
  long elapsed;

  if(alarm->delay == 0)
  {
    handle_alarm_event(alarm, event);
    return;
  }

  //
  // measure time delay and generate alarm event accordingly
  //
  switch(alarm->delay_state)
  {
  case alarm_delay_state_occurring:
    if(event == alarm_event_occur)
    {
      elapsed = time_util_get_sys_clock_elapsed_in_ms(alarm->start_time);
      if(elapsed >= alarm->delay)
      {
        alarm_move_delay_state(alarm, alarm_delay_state_occurred);
        handle_alarm_event(alarm, alarm_event_occur);
      }
    }
    else
    {
      alarm_move_delay_state(alarm, alarm_delay_state_clearing);
      alarm->start_time = time_util_get_sys_clock_in_ms();
    }
    break;

  case alarm_delay_state_clearing:
    if(event == alarm_event_clear)
    {
      elapsed = time_util_get_sys_clock_elapsed_in_ms(alarm->start_time);
      if(elapsed >= alarm->delay)
      {
        alarm_move_delay_state(alarm, alarm_delay_state_cleared);
        handle_alarm_event(alarm, alarm_event_clear);
      }
    }
    else
    {
      alarm_move_delay_state(alarm, alarm_delay_state_occurring);
      alarm->start_time = time_util_get_sys_clock_in_ms();
    }
    break;

  case alarm_delay_state_occurred:
    if(event == alarm_event_occur)
    {
      handle_alarm_event(alarm, alarm_event_occur);
    }
    else
    {
      alarm_move_delay_state(alarm, alarm_delay_state_clearing);
      alarm->start_time = time_util_get_sys_clock_in_ms();
    }
    break;

  case alarm_delay_state_cleared:
    if(event == alarm_event_clear)
    {
      handle_alarm_event(alarm, alarm_event_clear);
    }
    else
    {
      alarm_move_delay_state(alarm, alarm_delay_state_occurring);
      alarm->start_time = time_util_get_sys_clock_in_ms();
    }
    break;
  }
}

static void
__on_channel_update(observer_t* obs, void* arg)
{
  alarm_t*    alarm = container_of(obs, alarm_t, chnl_obs);
  channel_t*  chnl = (channel_t*)arg;

  alarm_update(alarm, chnl->eng_value);
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////
alarm_t*
alarm_alloc(uint32_t alarm_num, uint32_t chnl_num, alarm_severity_t severity,
    alarm_trigger_t trigger_type, alarm_setpoint_t set_point, uint32_t delay)
{
  alarm_t*    alarm;

  alarm = malloc(sizeof(alarm_t));

  alarm->alarm_num    = alarm_num;
  alarm->chnl_num     = chnl_num;
  alarm->state        = alarm_state_inactive;
  alarm->delay_state  = alarm_delay_state_cleared;
  alarm->severity     = severity;
  alarm->trigger_type = trigger_type;
  alarm->set_point    = set_point;
  alarm->delay        = delay;

  alarm->chnl_obs.notify = __on_channel_update;

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
alarm_update(alarm_t* alarm, channel_eng_value_t v)
{
  switch(alarm->trigger_type)
  {
  case alarm_trigger_digital:
    if(v.b == alarm->set_point.b)
    {
      handle_alarm_state_machine(alarm, alarm_event_occur);
    }
    else
    {
      handle_alarm_state_machine(alarm, alarm_event_clear);
    }
    break;

  case alarm_trigger_low:
    if(v.f < alarm->set_point.f)
    {
      handle_alarm_state_machine(alarm, alarm_event_occur);
    }
    else
    {
      handle_alarm_state_machine(alarm, alarm_event_clear);
    }
    break;

  case alarm_trigger_high:
    if(v.f > alarm->set_point.f)
    {
      handle_alarm_state_machine(alarm, alarm_event_occur);
    }
    else
    {
      handle_alarm_state_machine(alarm, alarm_event_clear);
    }
    break;
  }
}

void
alarm_ack(alarm_t* alarm)
{
  handle_alarm_event(alarm, alarm_event_ack);
}
