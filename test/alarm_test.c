#include <stdio.h>
#include <unistd.h>
#include "CUnit/Basic.h"
#include "CUnit/Console.h"
#include "CUnit/Automated.h"

#include "alarm.h"

void test_alarm_digital_basic(void)
{
  alarm_t*      alarm;
  alarm_setpoint_t    setpoint;
  channel_eng_value_t v;

  setpoint.b = TRUE;

  alarm = alarm_alloc(1, 1, alarm_severity_minor, alarm_trigger_digital, setpoint, 0);
  CU_ASSERT(alarm->state == alarm_state_inactive);

  v.b = FALSE;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive);

  v.b = TRUE;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_active_pending);

  alarm_ack(alarm);
  CU_ASSERT(alarm->state == alarm_state_active);

  v.b = FALSE;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive);

  v.b = TRUE;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_active_pending);

  v.b = FALSE;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive_pending);

  alarm_ack(alarm);
  CU_ASSERT(alarm->state == alarm_state_inactive);
}

void test_alarm_analog_low(void)
{
  alarm_t*      alarm;
  alarm_setpoint_t    setpoint;
  channel_eng_value_t v;

  setpoint.f = 10.0f;

  alarm = alarm_alloc(1, 1, alarm_severity_minor, alarm_trigger_low, setpoint, 0);
  CU_ASSERT(alarm->state == alarm_state_inactive);

  v.f = 20.0f;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive);

  v.f = 5.0f;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_active_pending);

  alarm_ack(alarm);
  CU_ASSERT(alarm->state == alarm_state_active);

  v.f = 20.0f;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive);

  v.f = 9.0f;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_active_pending);

  v.f = 20.0f;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive_pending);

  alarm_ack(alarm);
  CU_ASSERT(alarm->state == alarm_state_inactive);
}

void test_alarm_analog_high(void)
{
  alarm_t*      alarm;
  alarm_setpoint_t    setpoint;
  channel_eng_value_t v;

  setpoint.f = 10.0f;

  alarm = alarm_alloc(1, 1, alarm_severity_minor, alarm_trigger_high, setpoint, 0);
  CU_ASSERT(alarm->state == alarm_state_inactive);

  v.f = 5.0f;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive);

  v.f = 15.0f;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_active_pending);

  alarm_ack(alarm);
  CU_ASSERT(alarm->state == alarm_state_active);

  v.f = 5.0f;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive);

  v.f = 19.0f;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_active_pending);

  v.f = 3.0f;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive_pending);

  alarm_ack(alarm);
  CU_ASSERT(alarm->state == alarm_state_inactive);
}

void test_alarm_diital_delay(void)
{
  alarm_t*      alarm;
  alarm_setpoint_t    setpoint;
  channel_eng_value_t v;

  setpoint.b = TRUE;

  alarm = alarm_alloc(1, 1, alarm_severity_minor, alarm_trigger_digital, setpoint, 500);
  CU_ASSERT(alarm->state == alarm_state_inactive);

  v.b = FALSE;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive);

  v.b = TRUE;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive);

  sleep(1);
  v.b = TRUE;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_active_pending);

  alarm_ack(alarm);
  CU_ASSERT(alarm->state == alarm_state_active);

  v.b = FALSE;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_active);
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_active);
  
  sleep(1);
  v.b = FALSE;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive);

  v.b = TRUE;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive);
  sleep(1);
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_active_pending);

  v.b = FALSE;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_active_pending);
  sleep(1);
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive_pending);

  alarm_ack(alarm);
  CU_ASSERT(alarm->state == alarm_state_inactive);
}

void test_alarm_analog_low_delay(void)
{
  alarm_t*      alarm;
  alarm_setpoint_t    setpoint;
  channel_eng_value_t v;

  setpoint.f = 10.0f;

  alarm = alarm_alloc(1, 1, alarm_severity_minor, alarm_trigger_low, setpoint, 100);
  CU_ASSERT(alarm->state == alarm_state_inactive);

  v.f = 20.0f;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive);

  v.f = 5.0f;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive);
  sleep(1);
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_active_pending);

  alarm_ack(alarm);
  CU_ASSERT(alarm->state == alarm_state_active);

  v.f = 20.0f;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_active);
  sleep(1);
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive);

  v.f = 9.0f;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive);
  sleep(1);
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_active_pending);

  v.f = 20.0f;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_active_pending);
  sleep(1);
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive_pending);

  alarm_ack(alarm);
  CU_ASSERT(alarm->state == alarm_state_inactive);
}

void test_alarm_analog_high_delay(void)
{
  alarm_t*      alarm;
  alarm_setpoint_t    setpoint;
  channel_eng_value_t v;

  setpoint.f = 10.0f;

  alarm = alarm_alloc(1, 1, alarm_severity_minor, alarm_trigger_high, setpoint, 100);
  CU_ASSERT(alarm->state == alarm_state_inactive);

  v.f = 5.0f;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive);

  v.f = 15.0f;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive);
  sleep(1);
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_active_pending);

  alarm_ack(alarm);
  CU_ASSERT(alarm->state == alarm_state_active);

  v.f = 5.0f;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_active);
  sleep(1);
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive);

  v.f = 19.0f;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive);
  sleep(1);
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_active_pending);

  v.f = 3.0f;
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_active_pending);
  sleep(1);
  alarm_update(alarm, v);
  CU_ASSERT(alarm->state == alarm_state_inactive_pending);

  alarm_ack(alarm);
  CU_ASSERT(alarm->state == alarm_state_inactive);
}

void
alarm_add_test(CU_pSuite pSuite)
{
  CU_add_test(pSuite, "test_alarm_digital_basic", test_alarm_digital_basic);
  CU_add_test(pSuite, "test_alarm_analog_low", test_alarm_analog_low);
  CU_add_test(pSuite, "test_alarm_analog_high", test_alarm_analog_high);
  CU_add_test(pSuite, "test_alarm_diital_delay", test_alarm_diital_delay);
  CU_add_test(pSuite, "test_alarm_analog_low_delay", test_alarm_analog_low_delay);
  CU_add_test(pSuite, "test_alarm_analog_high_delay", test_alarm_analog_high_delay);
}
