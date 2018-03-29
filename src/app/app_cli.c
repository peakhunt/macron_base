#include <stdio.h>
#include "app_cli.h"
#include "app_config.h"
#include "cli.h"

void
app_cli_init(void)
{
  cli_config_t    cfg;

  app_config_get_cli_config(&cfg);

  cli_init(&cfg, NULL, 0);
}
