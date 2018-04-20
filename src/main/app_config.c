#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "app_config.h"
#include "config_reader.h"
#include "debug_log.h"
#include "cJSON.h"
#include <pthread.h>


#define APP_CONFIG_DEFAULT_FILE           "config/macron_base.json"
#define APP_CONFIG_ERROR_EXIT_VALUE       -3  

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// module privates
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static cJSON*             _jroot = NULL;
static pthread_rwlock_t   _jroot_lock;
static const char*        _json_str;
static int                _json_str_len;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// utilities
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
app_config_read_lock(void)
{
  pthread_rwlock_rdlock(&_jroot_lock);
}

void
app_config_write_lock(void)
{
  pthread_rwlock_wrlock(&_jroot_lock);
}

void
app_config_unlock(void)
{
  pthread_rwlock_unlock(&_jroot_lock);
}


static inline cJSON*
app_config_get_node(cJSON* node, const char* name)
{
  cJSON* child;

  child = cJSON_GetObjectItem(node, name);
  if(child == NULL)
  {
    debug_log("configuration error: %s doesn't exist under %s\n", name, node->string);
    exit(APP_CONFIG_ERROR_EXIT_VALUE);
  }
  return child;
}

static inline bool
app_config_get_bool(cJSON* node, const char* name)
{
  return (bool)(app_config_get_node(node, name)->valueint);
}

static inline int
app_config_get_int(cJSON* node, const char* name)
{
  return app_config_get_node(node, name)->valueint;
}

static inline double
app_config_get_double(cJSON* node, const char* name)
{
  return app_config_get_node(node, name)->valuedouble;
}

static inline char*
app_config_get_str(cJSON* node, const char* name)
{
  return app_config_get_node(node, name)->valuestring;
}

static inline SerialParity
app_config_get_serial_parity(cJSON* node, const char* name)
{
  char* str;

  str = app_config_get_node(node, name)->valuestring;

  if(strcmp(str, "none") == 0) return SerialParity_None;
  if(strcmp(str, "odd") == 0) return SerialParity_Odd;
  if(strcmp(str, "even") == 0) return SerialParity_Even;

  debug_log("configuration error: invalid serial parity: %s, for %s under %s\n", str, name, node->string);
  exit(APP_CONFIG_ERROR_EXIT_VALUE);
}

static inline void
app_config_get_serial_cfg(cJSON* node, SerialConfig* scfg)
{
  scfg->baud = app_config_get_int(node, "baud");
  scfg->data_bit = app_config_get_int(node, "data_bit");
  scfg->stop_bit = app_config_get_int(node, "stop_bit");
  scfg->parity = app_config_get_serial_parity(node, "parity");
}

static inline modbus_reg_type_t
app_config_get_modbus_reg_type(cJSON* node, const char* name)
{
  const char* str;
  modbus_reg_type_t   ret;

  str = app_config_get_str(node, name);

  if(strcmp(str, "coil") == 0)
  {
    ret = modbus_reg_coil;
  }
  else if(strcmp(str, "disc") == 0)
  {
    ret = modbus_reg_discrete;
  }
  else if(strcmp(str, "hold") == 0)
  {
    ret = modbus_reg_holding;
  }
  else
  {
    ret = modbus_reg_input;
  }

  return ret;
}

static inline void
app_config_get_modbus_reg_cfg(cJSON* node, modbus_address_t* mb_reg, uint32_t* chnl)
{
  mb_reg->slave_id    = (uint32_t)app_config_get_int(node, "slave");

  mb_reg->reg_type = app_config_get_modbus_reg_type(node, "reg");
  mb_reg->mb_address  = (uint32_t)app_config_get_int(node, "address");
  *chnl               = app_config_get_int(node, "channel");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// app config initializer
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
app_config_init(const char* cfg_file)
{
  const char* file;

  file = cfg_file == NULL ? APP_CONFIG_DEFAULT_FILE : cfg_file;

  pthread_rwlock_init(&_jroot_lock, NULL);

  _jroot = config_reader_init(file);
  if(_jroot == NULL)
  {
    debug_log("failed to load config file: %s\n", file);
    exit(-2);
  }

  _json_str = cJSON_PrintUnformatted(_jroot);
  _json_str_len = strlen(_json_str);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CLI configuration parameter
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
app_config_get_cli_config(cli_config_t* cli_cfg)
{
  cJSON   *cli,
          *node;

  app_config_read_lock();

  cli = app_config_get_node(_jroot, "cli");

  cli_cfg->prompt = app_config_get_node(cli, "prompt")->valuestring;

  node = app_config_get_node(cli, "telnet");
  cli_cfg->telnet_enabled = app_config_get_bool(node, "enabled");
  cli_cfg->tcp_port       = app_config_get_int(node, "port");

  node = app_config_get_node(cli, "serial");
  cli_cfg->serial_enabled = app_config_get_bool(node, "enabled");
  cli_cfg->serial_port = app_config_get_str(node, "serial_port");

  app_config_get_serial_cfg(node, &cli_cfg->serial_cfg);

  app_config_unlock();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Modbus Slave configuration parameter
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int
app_config_get_num_modbus_slaves(void)
{
  cJSON   *slave_list;
  int     n;

  app_config_read_lock();

  slave_list = app_config_get_node(_jroot, "modbus_slave_list");

  n  = cJSON_GetArraySize(slave_list);

  app_config_unlock();

  return n;
}

void
app_config_get_modbus_slave_at(int ndx, modbus_slave_driver_config_t* cfg)
{
  cJSON       *slave_list,
              *node,
              *param;
  const char  *str;

  app_config_read_lock();

  slave_list = app_config_get_node(_jroot, "modbus_slave_list");
  node = cJSON_GetArrayItem(slave_list, ndx);

  str = app_config_get_str(node, "protocol");
  if(strcmp(str, "tcp") == 0)
  {
    cfg->protocol = modbus_slave_driver_type_tcp;
  }
  else
  {
    cfg->protocol = modbus_slave_driver_type_rtu;
  }

  cfg->address = (uint16_t)app_config_get_int(node, "address");

  param = app_config_get_node(node, "param");

  if(cfg->protocol == modbus_slave_driver_type_tcp)
  {
    cfg->tcp_port = app_config_get_int(param, "port");
  }
  else
  {
    cfg->serial_port = app_config_get_str(param, "serial_port");
    app_config_get_serial_cfg(param, &cfg->serial_cfg);
  }

  app_config_unlock();
}

int
app_config_get_modbus_slave_num_regs(int slave_ndx)
{
  cJSON       *slave_list,
              *slave,
              *reg_map;
  int         n;

  app_config_read_lock();

  slave_list  = app_config_get_node(_jroot, "modbus_slave_list");
  slave       = cJSON_GetArrayItem(slave_list, slave_ndx);
  reg_map     = app_config_get_node(slave, "reg_map");

  n =  cJSON_GetArraySize(reg_map);

  app_config_unlock();

  return n;
}

void
app_config_get_modbus_slave_reg(int slave_ndx, int reg_ndx, modbus_address_t* mb_reg, uint32_t* chnl)
{
  cJSON       *slave_list,
              *slave,
              *reg_map,
              *reg;

  app_config_read_lock();

  slave_list  = app_config_get_node(_jroot, "modbus_slave_list");
  slave       = cJSON_GetArrayItem(slave_list, slave_ndx);
  reg_map     = app_config_get_node(slave, "reg_map");
  reg         = cJSON_GetArrayItem(reg_map, reg_ndx);

  app_config_get_modbus_reg_cfg(reg, mb_reg, chnl);

  app_config_unlock();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Modbus Master configuration parameter
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int
app_config_get_num_modbus_masters(void)
{
  cJSON   *master_list;
  int     n;

  app_config_read_lock();

  master_list = app_config_get_node(_jroot, "modbus_master_list");

  n  = cJSON_GetArraySize(master_list);

  app_config_unlock();

  return n;
}

void
app_config_get_modbus_master_at(int ndx, modbus_master_driver_config_t* cfg)
{
  cJSON       *master_list,
              *node,
              *param;
  const char  *str;

  app_config_read_lock();

  master_list = app_config_get_node(_jroot, "modbus_master_list");
  node = cJSON_GetArrayItem(master_list, ndx);

  cfg->schedule_finish_delay  = app_config_get_int(node, "schedule_finish_delay");
  cfg->inter_request_delay    = app_config_get_int(node, "inter_request_delay");
  cfg->timeout  = (uint32_t)app_config_get_int(node, "timeout");

  str = app_config_get_str(node, "protocol");
  if(strcmp(str, "tcp") == 0)
  {
    cfg->protocol   = modbus_master_driver_type_tcp;
    cfg->dest_ip    = app_config_get_str(node, "dest_ip");
    cfg->dest_port  = app_config_get_int(node, "dest_port");
  }
  else
  {
    cfg->protocol = modbus_master_driver_type_rtu;
    param = app_config_get_node(node, "param");
    cfg->serial_port = app_config_get_str(param, "serial_port");
    app_config_get_serial_cfg(param, &cfg->serial_cfg);
  }

  app_config_unlock();
}

int
app_config_get_modbus_master_num_regs(int master_ndx)
{
  cJSON       *master_list,
              *master,
              *reg_map;
  int         n;

  app_config_read_lock();

  master_list = app_config_get_node(_jroot, "modbus_master_list");
  master      = cJSON_GetArrayItem(master_list, master_ndx);
  reg_map     = app_config_get_node(master, "reg_map");

  n =  cJSON_GetArraySize(reg_map);

  app_config_unlock();
  return n;
}

void
app_config_get_modbus_master_reg(int master_ndx, int reg_ndx, modbus_address_t* mb_reg, uint32_t* chnl)
{
  cJSON     *master_list,
            *master,
            *reg_map,
            *reg;

  app_config_read_lock();

  master_list = app_config_get_node(_jroot, "modbus_master_list");
  master      = cJSON_GetArrayItem(master_list, master_ndx);
  reg_map     = app_config_get_node(master, "reg_map");
  reg         = cJSON_GetArrayItem(reg_map, reg_ndx);
  app_config_get_modbus_reg_cfg(reg, mb_reg, chnl);

  app_config_unlock();
}

int
app_config_get_modbus_master_num_request_schedules(int master_ndx)
{
  cJSON       *master_list,
              *master,
              *request_schedule;
  int         n;

  app_config_read_lock();

  master_list         = app_config_get_node(_jroot, "modbus_master_list");
  master              = cJSON_GetArrayItem(master_list, master_ndx);
  request_schedule    = app_config_get_node(master, "request_schedule");

  n =  cJSON_GetArraySize(request_schedule);

  app_config_unlock();
  return n;
}

void
app_config_get_modbus_slave_request_schedule(int master_ndx, int req_ndx, modbus_master_driver_request_config_t* cfg)
{
  cJSON       *master_list,
              *master,
              *request_schedule,
              *request;
  const char* str;

  app_config_read_lock();

  master_list         = app_config_get_node(_jroot, "modbus_master_list");
  master              = cJSON_GetArrayItem(master_list, master_ndx);
  request_schedule    = app_config_get_node(master, "request_schedule");
  request             = cJSON_GetArrayItem(request_schedule, req_ndx);

  cfg->slave_id   = app_config_get_int(request, "slave");

  str = app_config_get_str(request, "op");
  if(strcmp(str, "write") == 0)
  {
    cfg->op = modbus_reg_op_write;
  }
  else
  {
    cfg->op = modbus_reg_op_read;
  }

  cfg->reg        = app_config_get_modbus_reg_type(request, "reg");
  cfg->start_addr = app_config_get_int(request, "reg_addr");
  cfg->num_regs   = app_config_get_int(request, "num_regs");

  app_config_unlock();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// channel loading
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int
app_config_get_num_channels(void)
{
  cJSON       *channels;
  int         n;

  app_config_read_lock();

  channels = app_config_get_node(_jroot, "channels");

  n =  cJSON_GetArraySize(channels);

  app_config_unlock();

  return n;
}

void
app_config_get_channel_at(int ndx, core_driver_channel_config_t* chnl_cfg)
{
  cJSON       *channels,
              *node;
  const char* str;

  app_config_read_lock();

  channels = app_config_get_node(_jroot, "channels");
  node = cJSON_GetArrayItem(channels, ndx);

  chnl_cfg->chnl_num = app_config_get_int(node, "chnl_num");

  str = app_config_get_str(node, "chnl_type");
  if(strcmp(str, "digital") == 0)
  {
    chnl_cfg->chnl_type = channel_type_digital;
  }
  else
  {
    chnl_cfg->chnl_type = channel_type_analog;
  }

  str = app_config_get_str(node, "chnl_dir");
  if(strcmp(str, "input") == 0)
  {
    chnl_cfg->chnl_dir = channel_direction_in;
  }
  else if(strcmp(str, "output") == 0)
  {
    chnl_cfg->chnl_dir = channel_direction_out;
  }
  else
  {
    chnl_cfg->chnl_dir = channel_direction_virtual;
  }

  app_config_unlock();
}

int
app_config_get_num_lookup_table_of_channel_at(int ndx)
{
  cJSON       *channels,
              *channel,
              *lookup_table;
  int         n;

  app_config_read_lock();

  channels      = app_config_get_node(_jroot, "channels");
  channel       = cJSON_GetArrayItem(channels, ndx);
  lookup_table  = app_config_get_node(channel, "lookup_table");

  n =  cJSON_GetArraySize(lookup_table);
  app_config_unlock();

  return n;
}

void
app_config_get_lookup_table_of_channel_at(int chnl_ndx, int lt_ndx, float* raw, float* eng)
{
  cJSON       *channels,
              *channel,
              *lookup_table,
              *entry;

  app_config_read_lock();

  channels      = app_config_get_node(_jroot, "channels");
  channel       = cJSON_GetArrayItem(channels, chnl_ndx);
  lookup_table  = app_config_get_node(channel, "lookup_table");
  entry         =  cJSON_GetArrayItem(lookup_table, lt_ndx);

  *raw          = (float)app_config_get_double(entry, "raw");
  *eng          = (float)app_config_get_double(entry, "eng");

  app_config_unlock();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// alarm loading
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int
app_config_get_num_alarms(void)
{
  cJSON       *alarms;
  int         n;

  app_config_read_lock();

  alarms = app_config_get_node(_jroot, "alarms");

  n =  cJSON_GetArraySize(alarms);

  app_config_unlock();

  return n;
}

void
app_config_get_alarm_at(int ndx, core_driver_alarm_config_t* alm_cfg)
{
  cJSON       *alarms,
              *node;

  const char* str;

  app_config_read_lock();

  alarms = app_config_get_node(_jroot, "alarms");
  node = cJSON_GetArrayItem(alarms, ndx);

  alm_cfg->alarm_num  = app_config_get_int(node, "alarm_num");
  alm_cfg->chnl_num   = app_config_get_int(node, "chnl_num");

  str = app_config_get_str(node, "trigger_type");
  if(strcmp(str, "digital") == 0)
  {
    alm_cfg->trigger_type = alarm_trigger_digital;
  }
  else if(strcmp(str, "low") == 0)
  {
    alm_cfg->trigger_type = alarm_trigger_low;
  }
  else
  {
    alm_cfg->trigger_type = alarm_trigger_high;
  }

  if(alm_cfg->trigger_type == alarm_trigger_digital)
  {
    str = app_config_get_str(node, "set_point");
    if(strcmp(str, "on") == 0)
    {
      alm_cfg->set_point.b = TRUE;
    }
    else
    {
      alm_cfg->set_point.b = FALSE;
    }
  }
  else
  {
    alm_cfg->set_point.f = (float)app_config_get_double(node, "set_point");
  }

  str = app_config_get_str(node, "severity");
  if(strcmp(str, "minor") == 0)
  {
    alm_cfg->severity = alarm_severity_minor;
  }
  else if(strcmp(str, "major") == 0)
  {
    alm_cfg->severity = alarm_severity_major;
  }
  else
  {
    alm_cfg->severity = alarm_severity_critical;
  }

  alm_cfg->delay = app_config_get_int(node, "delay");

  app_config_unlock();
}

//
// XXX should be called with read lock on config
//
const char*
app_config_get_json_string_use_lock_before_call(int* len)
{
  *len = _json_str_len;
  return _json_str;
}
