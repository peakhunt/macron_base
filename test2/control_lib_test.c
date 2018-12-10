#include <stdio.h>
#include <unistd.h>
#include "CUnit/Basic.h"
#include "CUnit/Console.h"
#include "CUnit/Automated.h"

#include "control_lib.h"

////////////////////////////////////////////////////////////////////////////////
//
// timer simulator
//
static unsigned long _tick_ms = 0;

unsigned long
time_now_in_ms(void)
{
  return _tick_ms;
}

unsigned long
time_delta_in_ms(unsigned long start)
{
  return _tick_ms - start;
}

void
time_util_reset(void)
{
  _tick_ms = 0;
}

void
time_util_inc(unsigned long inc)
{
  _tick_ms += inc;
}
////////////////////////////////////////////////////////////////////////////////

static void
test_sr_latch(void)
{
  sr_latch_t    sr;

  sr_latch_init(&sr, FALSE);
  sr_latch(&sr, FALSE, FALSE);
  CU_ASSERT(sr.q == FALSE);

  sr_latch_init(&sr, TRUE);
  sr_latch(&sr, FALSE, FALSE);
  CU_ASSERT(sr.q == TRUE);

  sr_latch_init(&sr, FALSE);
  sr_latch(&sr, FALSE, TRUE);
  CU_ASSERT(sr.q == FALSE);

  sr_latch_init(&sr, TRUE);
  sr_latch(&sr, FALSE, TRUE);
  CU_ASSERT(sr.q == FALSE);

  sr_latch_init(&sr, FALSE);
  sr_latch(&sr, TRUE, FALSE);
  CU_ASSERT(sr.q == TRUE);

  sr_latch_init(&sr, TRUE);
  sr_latch(&sr, TRUE, FALSE);
  CU_ASSERT(sr.q == TRUE);

  sr_latch_init(&sr, FALSE);
  sr_latch(&sr, TRUE, TRUE);
  CU_ASSERT(sr.q == TRUE);

  sr_latch_init(&sr, TRUE);
  sr_latch(&sr, TRUE, TRUE);
  CU_ASSERT(sr.q == TRUE);
}

static void
test_rs_latch(void)
{
  rs_latch_t    rs;

  rs_latch_init(&rs, FALSE);
  rs_latch(&rs, FALSE, FALSE);
  CU_ASSERT(rs.q == FALSE);

  rs_latch_init(&rs, TRUE);
  rs_latch(&rs, FALSE, FALSE);
  CU_ASSERT(rs.q == TRUE);

  rs_latch_init(&rs, FALSE);
  rs_latch(&rs, FALSE, TRUE);
  CU_ASSERT(rs.q == FALSE);

  rs_latch_init(&rs, TRUE);
  rs_latch(&rs, FALSE, TRUE);
  CU_ASSERT(rs.q == FALSE);

  rs_latch_init(&rs, FALSE);
  rs_latch(&rs, TRUE, FALSE);
  CU_ASSERT(rs.q == TRUE);

  rs_latch_init(&rs, TRUE);
  rs_latch(&rs, TRUE, FALSE);
  CU_ASSERT(rs.q == TRUE);

  rs_latch_init(&rs, FALSE);
  rs_latch(&rs, TRUE, TRUE);
  CU_ASSERT(rs.q == FALSE);

  rs_latch_init(&rs, TRUE);
  rs_latch(&rs, TRUE, TRUE);
  CU_ASSERT(rs.q == FALSE);
}

static void
test_rtrig(void)
{
  r_trig_t    rt;

  r_trig_init(&rt);

  r_trig(&rt, TRUE);
  CU_ASSERT(rt.q == TRUE);

  r_trig(&rt, TRUE);
  CU_ASSERT(rt.q == FALSE);

  r_trig(&rt, FALSE);
  CU_ASSERT(rt.q == FALSE);

  r_trig(&rt, FALSE);
  CU_ASSERT(rt.q == FALSE);

  r_trig(&rt, TRUE);
  CU_ASSERT(rt.q == TRUE);

  r_trig(&rt, TRUE);
  CU_ASSERT(rt.q == FALSE);
}

static void
test_ftrig(void)
{
  f_trig_t    ft;

  f_trig_init(&ft);

  f_trig(&ft, FALSE);
  CU_ASSERT(ft.q == FALSE);

  f_trig(&ft, TRUE);
  CU_ASSERT(ft.q == FALSE);

  f_trig(&ft, FALSE);
  CU_ASSERT(ft.q == TRUE);

  f_trig(&ft, FALSE);
  CU_ASSERT(ft.q == FALSE);
}

static void
test_ton(void)
{
  ton_t     t;

  time_util_reset();

  ton_init(&t);
  CU_ASSERT(t.q == FALSE);

  // start timer
  ton(&t, TRUE, 10);
  CU_ASSERT(t.start_time == _tick_ms);
  CU_ASSERT(t.q == FALSE);
  CU_ASSERT(t.tmr_running == TRUE);

  // timeout
  time_util_inc(10);
  ton(&t, TRUE, 10);
  CU_ASSERT(t.q == TRUE);
  CU_ASSERT(t.tmr_running == FALSE);

  // still timeout
  time_util_inc(10);
  ton(&t, TRUE, 10);
  CU_ASSERT(t.q == TRUE);
  CU_ASSERT(t.tmr_running == FALSE);

  // timer disarmbed
  ton(&t, FALSE, 10);
  CU_ASSERT(t.q == FALSE);
  CU_ASSERT(t.tmr_running == FALSE);

  // let time pass
  time_util_inc(10);

  // start timer again
  ton(&t, TRUE, 10);
  CU_ASSERT(t.q == FALSE);
  CU_ASSERT(t.tmr_running == TRUE);

  // timer cancelled
  time_util_inc(5);
  ton(&t, FALSE, 10);
  CU_ASSERT(t.q == FALSE);
  CU_ASSERT(t.tmr_running == FALSE);
}

static void
test_toff(void)
{
  toff_t      t;

  time_util_reset();

  toff_init(&t);
  CU_ASSERT(t.q == TRUE);

  // stop timer
  toff(&t, TRUE, 0);
  CU_ASSERT(t.q == TRUE);
  CU_ASSERT(t.tmr_running == FALSE);

  time_util_inc(5);
  // start timer
  toff(&t, FALSE, 10);
  CU_ASSERT(t.q == TRUE);
  CU_ASSERT(t.tmr_running == TRUE);

  // let time pass 
  time_util_inc(5);
  toff(&t, FALSE, 10);
  CU_ASSERT(t.q == TRUE);
  CU_ASSERT(t.tmr_running == TRUE);

  // let it timeout
  time_util_inc(5);
  toff(&t, FALSE, 10);
  CU_ASSERT(t.q == FALSE);
  CU_ASSERT(t.tmr_running == FALSE);

  // let time pass
  time_util_inc(5);
  toff(&t, FALSE, 10);
  CU_ASSERT(t.q == FALSE);
  CU_ASSERT(t.tmr_running == FALSE);

  // stop timer
  toff(&t, TRUE, 10);
  CU_ASSERT(t.q == TRUE);
  CU_ASSERT(t.tmr_running == FALSE);

  time_util_inc(5);

  // start timer again
  toff(&t, FALSE, 10);
  CU_ASSERT(t.q == TRUE);
  CU_ASSERT(t.tmr_running == TRUE);

  time_util_inc(5);

  // cancel timer
  toff(&t, TRUE, 10);
  CU_ASSERT(t.q == TRUE);
  CU_ASSERT(t.tmr_running == FALSE);

  // let time pass
  time_util_inc(5);
  toff(&t, TRUE, 10);
  CU_ASSERT(t.q == TRUE);
  CU_ASSERT(t.tmr_running == FALSE);

  // start timer
  toff(&t, FALSE, 10);
  CU_ASSERT(t.q == TRUE);
  CU_ASSERT(t.tmr_running == TRUE);

  // let time pass
  time_util_inc(5);
  toff(&t, FALSE, 10);
  CU_ASSERT(t.q == TRUE);
  CU_ASSERT(t.tmr_running == TRUE);

  // let it timeout
  time_util_inc(5);
  toff(&t, FALSE, 10);
  CU_ASSERT(t.q == FALSE);
  CU_ASSERT(t.tmr_running == FALSE);

  // let time pass
  time_util_inc(20);
  toff(&t, FALSE, 10);
  CU_ASSERT(t.q == FALSE);
  CU_ASSERT(t.tmr_running == FALSE);
}

static void
test_tp(void)
{
  tp_t      t;

  time_util_reset();

  tp_init(&t);
  CU_ASSERT(t.q == FALSE);

  // start
  tp(&t, TRUE, 10);
  CU_ASSERT(t.q == TRUE);

  time_util_inc(5);
  tp(&t, TRUE, 10);
  CU_ASSERT(t.q == TRUE);

  // timeout
  time_util_inc(5);
  tp(&t, TRUE, 10);
  CU_ASSERT(t.q == FALSE);
  tp(&t, FALSE, 10);

  time_util_inc(5);

  // start again
  tp(&t, TRUE, 10);
  CU_ASSERT(t.q == TRUE);

  // oscillation during running
  time_util_inc(1);
  tp(&t, FALSE, 10);
  CU_ASSERT(t.q == TRUE);

  time_util_inc(1);
  tp(&t, TRUE, 10);
  CU_ASSERT(t.q == TRUE);

  time_util_inc(1);
  tp(&t, FALSE, 10);
  CU_ASSERT(t.q == TRUE);

  time_util_inc(7);
  // timeout
  tp(&t, FALSE, 10);
  CU_ASSERT(t.q == FALSE);

  time_util_inc(5);
  // start again
  tp(&t, TRUE, 10);
  CU_ASSERT(t.q == TRUE);

  // timeout
  time_util_inc(10);
  tp(&t, TRUE, 10);
  CU_ASSERT(t.q == FALSE);

  time_util_inc(5);
  tp(&t, FALSE, 10);
  CU_ASSERT(t.q == FALSE);
}

static void
test_blink(void)
{
  blink_t   bl;

  time_util_reset();
  blink_init(&bl);
  CU_ASSERT(bl.q == FALSE);

  time_util_inc(5);
  CU_ASSERT(bl.q == FALSE);

  time_util_inc(5);

  // start blinking
  blink(&bl, TRUE, 10);
  CU_ASSERT(bl.q == FALSE);

  time_util_inc(5);
  blink(&bl, TRUE, 10);
  CU_ASSERT(bl.q == TRUE);

  time_util_inc(5);
  blink(&bl, TRUE, 10);
  CU_ASSERT(bl.q == FALSE);

  time_util_inc(5);
  blink(&bl, TRUE, 10);
  CU_ASSERT(bl.q == TRUE);

  // stop
  blink(&bl, FALSE, 10);
  CU_ASSERT(bl.q == FALSE);
}

void
control_lib_test(CU_pSuite pSuite)
{
  CU_add_test(pSuite, "sr latch", test_sr_latch);
  CU_add_test(pSuite, "rs latch", test_rs_latch);
  CU_add_test(pSuite, "rtrig", test_rtrig);
  CU_add_test(pSuite, "ftrig", test_ftrig);
  CU_add_test(pSuite, "ton", test_ton);
  CU_add_test(pSuite, "toff", test_toff);
  CU_add_test(pSuite, "tp", test_tp);
  CU_add_test(pSuite, "blink", test_blink);
}
