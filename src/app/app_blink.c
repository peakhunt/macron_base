#include "app_blink.h"
#include "control_lib.h"

bool io_var_for_ch23;

static ton_t    t1;
static ton_t    t2;

static bool     c1;

void
app_blink_init(void)
{
  io_var_for_ch23 = FALSE;

  c1 = FALSE;

  ton_init(&t1);
  ton_init(&t2);
}

void
app_blink_run(void)
{
  TON(t1, !c1, io_var_for_ch23, 1000);
  TON(t2, io_var_for_ch23, c1, 1000);
}
