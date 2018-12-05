#include "common_def.h"
#include "app.h"
#include "channel_manager.h"
#include "alarm_manager.h"
#include "trace.h"

#include "evloop_thread.h"
#include "evloop_timer.h"

static evloop_timer_t         _test_timer;

static double io_var_for_ch11;
static double io_var_for_ch22;
static bool   io_var_for_ch23;
static bool   io_var_ch1;

static bool   up_inc = TRUE;

static alarm_state_t    alarm_var_test;

static io_var_t   io_mappings[] =
{
  {   1,    io_var_enum_bool,   &io_var_ch1           },
  {  11,    io_var_enum_double, &io_var_for_ch11      },
  {  22,    io_var_enum_double, &io_var_for_ch22      },
  {  23,    io_var_enum_bool,   &io_var_for_ch23      },
};

static alarm_var_t    alarm_mappings[] = 
{
  {  23,    &alarm_var_test   },
};

static void
test_timer_expire(evloop_timer_t* t, void* arg)
{
  TRACE(CORE_DRIVER, "test_timer_expire\n");

  io_var_for_ch23 = !io_var_for_ch23;

  if(up_inc)
  {
    io_var_for_ch22 += 0.5;
  }
  else
  {
    io_var_for_ch22 -= 0.5;
  }

  if(io_var_for_ch22 > 20.0)
  {
    up_inc = FALSE;
    io_var_for_ch22 = 20.0;
  }

  if(io_var_for_ch22 < 0)
  {
    up_inc = TRUE;
    io_var_for_ch22 = 0.0;
  }


  evloop_timer_start(&_test_timer, 1, 0);
}

void
app_init(void)
{
  channel_manager_bind_io_vars(io_mappings, 4);
  alarm_manager_bind_alarm_vars(alarm_mappings, 1);

  io_var_for_ch22   = 15.5;
  io_var_for_ch23   = TRUE;
}

void
app_start(void)
{
  evloop_timer_init(&_test_timer, test_timer_expire, NULL);
  evloop_timer_start(&_test_timer, 1, 0);
}

void
app_run(void)
{
}

void
app_debug(void)
{
  TRACE(CORE_DRIVER, "io_var_for_ch11 %f\n", io_var_for_ch11);
  TRACE(CORE_DRIVER, "io_var_for_ch22 %f\n", io_var_for_ch22);
  TRACE(CORE_DRIVER, "io_var_ch1 %d\n", io_var_ch1);
  TRACE(CORE_DRIVER, "io_var_for_ch23 %d\n", io_var_for_ch23);
  TRACE(CORE_DRIVER, "alarm_var_test %d\n", alarm_var_test);
}
