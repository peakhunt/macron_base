#ifndef __CFG_MGR_DEF_H__
#define __CFG_MGR_DEF_H__

#include "common_def.h"
#include "cli.h"
#include "modbus_slave_driver.h"
#include "modbus_master_driver.h"
#include "webserver_driver.h"
#include "core_driver.h"
#include "alarm_manager.h"
#include "channel_manager.h"

extern void cfg_mgr_init(const char* cfg_file);

// for CLI
extern void cfg_mgr_get_cli_config(cli_config_t* cli_cfg);

// for channel core
extern int cfg_mgr_get_num_channels(void);
extern void cfg_mgr_get_channel_at(int ndx, core_driver_channel_config_t* chnl_cfg);

// for analog lookup table
extern int cfg_mgr_get_num_lookup_table_of_channel_at(int ndx);
extern void cfg_mgr_get_lookup_table_of_channel_at(int chnl_ndx, int lt_ndx, double* raw, double* eng);

// for alarm core
extern int cfg_mgr_get_num_alarms(void);
extern void cfg_mgr_get_alarm_at(int ndx, core_driver_alarm_config_t* alm_cfg);

// for modbus slaves
extern int cfg_mgr_get_num_modbus_slaves(void);
extern void cfg_mgr_get_modbus_slave_at(int ndx, modbus_slave_driver_config_t* cfg);
extern int cfg_mgr_get_modbus_slave_num_regs(int slave_ndx);
extern void cfg_mgr_get_modbus_slave_reg(int slave_ndx, int reg_ndx, modbus_address_t* mb_reg, uint32_t* chnl, modbus_reg_mapping_to_t* mapping_to, modbus_reg_codec_t* codec);

// for modbus masters
extern int cfg_mgr_get_num_modbus_masters(void);
extern void cfg_mgr_get_modbus_master_at(int ndx, modbus_master_driver_config_t* cfg);
extern int cfg_mgr_get_modbus_master_num_regs(int master_ndx);
extern void cfg_mgr_get_modbus_master_reg(int master_ndx, int reg_ndx, modbus_address_t* mb_reg, uint32_t* chnl,
    modbus_reg_codec_t* codec);
extern int cfg_mgr_get_modbus_master_num_request_schedules(int master_ndx);
extern void cfg_mgr_get_modbus_slave_request_schedule(int master_ndx, int req_ndx, modbus_master_driver_request_config_t* cfg);

extern const char* cfg_mgr_get_json_string_use_lock_before_call(int* len);

extern void cfg_mgr_get_webserver_config(webserver_config_t* cfg);

extern void cfg_mgr_read_lock(void);
extern void cfg_mgr_write_lock(void);
extern void cfg_mgr_unlock(void);

extern bool cfg_mgr_update_alarm_cfg(uint32_t alarm_num, alarm_runtime_config_t* cfg);
extern bool cfg_mgr_update_channel_cfg(uint32_t chnl_num, channel_runtime_config_t* cfg);
extern bool cfg_mgr_update_lookup_table(uint32_t chnl_num, lookup_table_t* lookup_table);

#endif /* !__CFG_MGR_DEF_H__ */
