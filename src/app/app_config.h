#ifndef __APP_CONFIG_DEF_H__
#define __APP_CONFIG_DEF_H__

#include "common_def.h"
#include "cli.h"
#include "app_modbus.h"
#include "app_core.h"

extern void app_config_init(const char* cfg_file);

// for CLI
extern void app_config_get_cli_config(cli_config_t* cli_cfg);

// for channel core
extern int app_config_get_num_channels(void);
extern void app_config_get_channel_at(int ndx, app_channel_config_t* chnl_cfg);

// for modbus slaves
extern int app_config_get_num_modbus_slaves(void);
extern void app_config_get_modbus_slaves_at(int ndx, app_modbus_slave_config_t* cfg);

#endif /* !__APP_CONFIG_DEF_H__ */
