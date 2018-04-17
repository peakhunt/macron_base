#ifndef __APP_CONFIG_DEF_H__
#define __APP_CONFIG_DEF_H__

#include "common_def.h"
#include "cli.h"
#include "app_modbus_slave.h"
#include "app_modbus_master.h"
#include "app_core.h"

extern void app_config_init(const char* cfg_file);

// for CLI
extern void app_config_get_cli_config(cli_config_t* cli_cfg);

// for channel core
extern int app_config_get_num_channels(void);
extern void app_config_get_channel_at(int ndx, app_channel_config_t* chnl_cfg);

// for analog lookup table
extern int app_config_get_num_lookup_table_of_channel_at(int ndx);
extern void app_config_get_lookup_table_of_channel_at(int chnl_ndx, int lt_ndx, float* raw, float* eng);

// for alarm core
extern int app_config_get_num_alarms(void);
extern void app_config_get_alarm_at(int ndx, app_alarm_config_t* alm_cfg);

// for modbus slaves
extern int app_config_get_num_modbus_slaves(void);
extern void app_config_get_modbus_slave_at(int ndx, app_modbus_slave_config_t* cfg);
extern int app_config_get_modbus_slave_num_regs(int slave_ndx);
extern void app_config_get_modbus_slave_reg(int slave_ndx, int reg_ndx, modbus_address_t* mb_reg, uint32_t* chnl);

// for modbus masters
extern int app_config_get_num_modbus_masters(void);
extern void app_config_get_modbus_master_at(int ndx, app_modbus_master_config_t* cfg);
extern int app_config_get_modbus_master_num_regs(int master_ndx);
extern void app_config_get_modbus_master_reg(int master_ndx, int reg_ndx, modbus_address_t* mb_reg, uint32_t* chnl);
extern int app_config_get_modbus_master_num_request_schedules(int master_ndx);
extern void app_config_get_modbus_slave_request_schedule(int master_ndx, int req_ndx, app_modbus_master_request_config_t* cfg);

#endif /* !__APP_CONFIG_DEF_H__ */
