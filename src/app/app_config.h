#ifndef __APP_CONFIG_DEF_H__
#define __APP_CONFIG_DEF_H__

#include "common_def.h"
#include "cli.h"

extern void app_config_init(const char* cfg_file);
extern void app_config_get_cli_config(cli_config_t* cli_cfg);

#endif /* !__APP_CONFIG_DEF_H__ */
