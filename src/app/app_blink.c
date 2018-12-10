#include "app_blink.h"
#include "control_lib.h"

bool io_var_for_ch23;

static blink_t    bl;

void
app_blink_init(void)
{
  io_var_for_ch23 = FALSE;

  blink_init(&bl);
}

void
app_blink_run(void)
{
  BLINK(bl, TRUE, io_var_for_ch23, 1000);
}
