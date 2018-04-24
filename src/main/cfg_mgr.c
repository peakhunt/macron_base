#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "cfg_mgr.h"
#include "config_reader.h"
#include "debug_log.h"
#include "cJSON.h"
#include "bit_util.h"
#include "atomic_file_update.h"
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
static char*              _json_str = NULL;
static int                _json_str_len = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// utilities
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
cfg_mgr_regenerate_in_memory_string(void)
{
  if(_json_str)
  {
    free(_json_str);
  }

  _json_str     = cJSON_Print(_jroot);
  _json_str_len = strlen(_json_str);
}

void
cfg_mgr_read_lock(void)
{
  pthread_rwlock_rdlock(&_jroot_lock);
}

void
cfg_mgr_write_lock(void)
{
  pthread_rwlock_wrlock(&_jroot_lock);
}

void
cfg_mgr_unlock(void)
{
  pthread_rwlock_unlock(&_jroot_lock);
}


static inline cJSON*
cfg_mgr_get_node(cJSON* node, const char* name)
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
cfg_mgr_get_bool(cJSON* node, const char* name)
{
  return (bool)(cfg_mgr_get_node(node, name)->valueint);
}

static inline bool
cfg_mgr_get_bool_optional(cJSON* node, const char* name, bool default_value)
{
  cJSON* child;

  child = cJSON_GetObjectItem(node, name);
  if(child == NULL)
  {
    return default_value;
  }
  return (bool)(cfg_mgr_get_node(node, name)->valueint);
}

static inline int
cfg_mgr_get_int(cJSON* node, const char* name)
{
  return cfg_mgr_get_node(node, name)->valueint;
}

static inline double
cfg_mgr_get_double(cJSON* node, const char* name)
{
  return cfg_mgr_get_node(node, name)->valuedouble;
}

static inline char*
cfg_mgr_get_str(cJSON* node, const char* name)
{
  return cfg_mgr_get_node(node, name)->valuestring;
}

static inline const char*
cfg_mgr_get_str_optional(cJSON* node, const char* name, const char* default_value)
{
  cJSON* child;

  child = cJSON_GetObjectItem(node, name);
  if(child == NULL)
  {
    return default_value;
  }
  return child->valuestring;
}

static inline SerialParity
cfg_mgr_get_serial_parity(cJSON* node, const char* name)
{
  char* str;

  str = cfg_mgr_get_node(node, name)->valuestring;

  if(strcmp(str, "none") == 0) return SerialParity_None;
  if(strcmp(str, "odd") == 0) return SerialParity_Odd;
  if(strcmp(str, "even") == 0) return SerialParity_Even;

  debug_log("configuration error: invalid serial parity: %s, for %s under %s\n", str, name, node->string);
  exit(APP_CONFIG_ERROR_EXIT_VALUE);
}

static inline void
cfg_mgr_get_serial_cfg(cJSON* node, SerialConfig* scfg)
{
  scfg->baud = cfg_mgr_get_int(node, "baud");
  scfg->data_bit = cfg_mgr_get_int(node, "data_bit");
  scfg->stop_bit = cfg_mgr_get_int(node, "stop_bit");
  scfg->parity = cfg_mgr_get_serial_parity(node, "parity");
}

static inline modbus_reg_type_t
cfg_mgr_get_modbus_reg_type(cJSON* node, const char* name)
{
  const char* str;
  modbus_reg_type_t   ret;

  str = cfg_mgr_get_str(node, name);

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
cfg_mgr_get_modbus_reg_cfg(cJSON* node, modbus_address_t* mb_reg, uint32_t* chnl)
{
  mb_reg->slave_id    = (uint32_t)cfg_mgr_get_int(node, "slave");

  mb_reg->reg_type = cfg_mgr_get_modbus_reg_type(node, "reg");
  mb_reg->mb_address  = (uint32_t)cfg_mgr_get_int(node, "address");

  *chnl               = cfg_mgr_get_int(node, "channel");
}

static inline void
cfg_mgr_get_modbus_reg_mapping_to(cJSON* node, modbus_reg_mapping_to_t* mapping_to)
{
  const char* str;

  str =  cfg_mgr_get_str_optional(node, "mapping_to", "channel");

  if(strcmp(str, "channel") == 0)
  {
    *mapping_to = modbus_reg_mapping_to_channel;
  }
  else if(strcmp(str, "alarm") == 0)
  {
    *mapping_to = modbus_reg_mapping_to_alarm;
  }
}

static inline void
cfg_mgr_get_modbus_reg_codec(cJSON* node, modbus_reg_codec_t* codec)
{
  cJSON* jcodec;

  jcodec = cJSON_GetObjectItem(node, "codec");

  if(jcodec == NULL)
  {
    codec->codec_type   = modbus_reg_codec_type_data_and_status;
    codec->d_mask       = 0xffff;
    codec->s_mask       = 0x0;
    codec->d_shift      = 0;
    codec->s_shift      = 0;
    codec->fault        = 0;
    codec->ok           = 0;
  }
  else
  {
    int         d_start, d_end, s_start, s_end,
                fault, ok;

    d_start                 = cfg_mgr_get_int(jcodec, "d_start");
    d_end                   = cfg_mgr_get_int(jcodec, "d_end");
    s_start                 = cfg_mgr_get_int(jcodec, "s_start");
    s_end                   = cfg_mgr_get_int(jcodec, "s_end");
    fault                   = cfg_mgr_get_int(jcodec, "fault");
    ok                      = cfg_mgr_get_int(jcodec, "ok");

    u16_mask(d_start, d_end, &codec->d_mask, &codec->d_shift);
    u16_mask(s_start, s_end, &codec->s_mask, &codec->s_shift);

    codec->codec_type   = modbus_reg_codec_type_data_and_status;
    codec->fault        = (uint16_t)fault;
    codec->ok           = (uint16_t)ok;

#if 1
    debug_log("d_start: %d, d_end: %d, s_start: %d, s_end: %d, fault: %d, ok: %d\n",
        d_start,
        d_end,
        s_start,
        s_end,
        fault,
        ok);

    debug_log("d_mask : %x, d_shift: %d, s_mask: %x, s_shift: %d, fault: %d, ok:%d\n",
        codec->d_mask, 
        codec->d_shift,
        codec->s_mask,
        codec->s_shift,
        codec->fault,
        codec->ok);
#endif
  }
}

static inline cJSON*
cfg_mgr_find_alarm(uint32_t alarm_num)
{
  cJSON       *alarms,
              *alarm;
  int         num_alarms;

  alarms      = cfg_mgr_get_node(_jroot, "alarms");
  num_alarms  =  cJSON_GetArraySize(alarms);

  for(int i = 0; i < num_alarms; i++)
  {
    alarm = cJSON_GetArrayItem(alarms, i);
    if((uint32_t)cfg_mgr_get_int(alarm, "alarm_num") == alarm_num)
    {
      return alarm;
    }
  }
  return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// file update
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int
cfg_mgr_atomic_file_update_cb(int fd)
{
  int   nwritten = 0,
        ret;

  while(nwritten < _json_str_len)
  {
    ret = write(fd, &_json_str[nwritten], _json_str_len - nwritten);
    if(ret <= 0)
    {
      return -1;
    }
    nwritten += ret;
  }
  return 0;
}

static void
cfg_mgr_inc_revision(void)
{
  char str_buf[256];
  time_t    now = time(NULL);
  cJSON*    update_info;
  cJSON*    revision;
  int       r;

  ctime_r(&now, str_buf);

  update_info = cJSON_GetObjectItem(_jroot, "update_info");

  cJSON_DeleteItemFromObject(update_info, "update_time");
  cJSON_AddStringToObject(update_info, "update_time", str_buf);

  revision = cJSON_GetObjectItem(update_info, "revision");
  r = revision->valueint;
  r++;
  cJSON_SetNumberValue(revision, r);
}

static void
cfg_mgr_udpate_confg_file(void)
{
  cfg_mgr_inc_revision();
  cfg_mgr_regenerate_in_memory_string();
  atomic_file_update(APP_CONFIG_DEFAULT_FILE, cfg_mgr_atomic_file_update_cb);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// app config initializer
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
cfg_mgr_init(const char* cfg_file)
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

  cfg_mgr_regenerate_in_memory_string();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CLI configuration parameter
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
cfg_mgr_get_cli_config(cli_config_t* cli_cfg)
{
  cJSON   *cli,
          *node;

  cfg_mgr_read_lock();

  cli = cfg_mgr_get_node(_jroot, "cli");

  cli_cfg->prompt = cfg_mgr_get_node(cli, "prompt")->valuestring;

  node = cfg_mgr_get_node(cli, "telnet");
  cli_cfg->telnet_enabled = cfg_mgr_get_bool(node, "enabled");
  cli_cfg->tcp_port       = cfg_mgr_get_int(node, "port");

  node = cfg_mgr_get_node(cli, "serial");
  cli_cfg->serial_enabled = cfg_mgr_get_bool(node, "enabled");
  cli_cfg->serial_port = cfg_mgr_get_str(node, "serial_port");

  cfg_mgr_get_serial_cfg(node, &cli_cfg->serial_cfg);

  cfg_mgr_unlock();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Modbus Slave configuration parameter
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int
cfg_mgr_get_num_modbus_slaves(void)
{
  cJSON   *slave_list;
  int     n;

  cfg_mgr_read_lock();

  slave_list = cfg_mgr_get_node(_jroot, "modbus_slave_list");

  n  = cJSON_GetArraySize(slave_list);

  cfg_mgr_unlock();

  return n;
}

void
cfg_mgr_get_modbus_slave_at(int ndx, modbus_slave_driver_config_t* cfg)
{
  cJSON       *slave_list,
              *node,
              *param;
  const char  *str;

  cfg_mgr_read_lock();

  slave_list = cfg_mgr_get_node(_jroot, "modbus_slave_list");
  node = cJSON_GetArrayItem(slave_list, ndx);

  str = cfg_mgr_get_str(node, "protocol");
  if(strcmp(str, "tcp") == 0)
  {
    cfg->protocol = modbus_slave_driver_type_tcp;
  }
  else
  {
    cfg->protocol = modbus_slave_driver_type_rtu;
  }

  cfg->address = (uint16_t)cfg_mgr_get_int(node, "address");

  param = cfg_mgr_get_node(node, "param");

  if(cfg->protocol == modbus_slave_driver_type_tcp)
  {
    cfg->tcp_port = cfg_mgr_get_int(param, "port");
  }
  else
  {
    cfg->serial_port = cfg_mgr_get_str(param, "serial_port");
    cfg_mgr_get_serial_cfg(param, &cfg->serial_cfg);
  }

  cfg_mgr_unlock();
}

int
cfg_mgr_get_modbus_slave_num_regs(int slave_ndx)
{
  cJSON       *slave_list,
              *slave,
              *reg_map;
  int         n;

  cfg_mgr_read_lock();

  slave_list  = cfg_mgr_get_node(_jroot, "modbus_slave_list");
  slave       = cJSON_GetArrayItem(slave_list, slave_ndx);
  reg_map     = cfg_mgr_get_node(slave, "reg_map");

  n =  cJSON_GetArraySize(reg_map);

  cfg_mgr_unlock();

  return n;
}

void
cfg_mgr_get_modbus_slave_reg(int slave_ndx, int reg_ndx, modbus_address_t* mb_reg, uint32_t* chnl, modbus_reg_mapping_to_t* mapping_to, modbus_reg_codec_t* codec)
{
  cJSON       *slave_list,
              *slave,
              *reg_map,
              *reg;

  cfg_mgr_read_lock();

  slave_list  = cfg_mgr_get_node(_jroot, "modbus_slave_list");
  slave       = cJSON_GetArrayItem(slave_list, slave_ndx);
  reg_map     = cfg_mgr_get_node(slave, "reg_map");
  reg         = cJSON_GetArrayItem(reg_map, reg_ndx);

  cfg_mgr_get_modbus_reg_cfg(reg, mb_reg, chnl);
  cfg_mgr_get_modbus_reg_codec(reg, codec);
  cfg_mgr_get_modbus_reg_mapping_to(reg, mapping_to);

  cfg_mgr_unlock();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Modbus Master configuration parameter
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int
cfg_mgr_get_num_modbus_masters(void)
{
  cJSON   *master_list;
  int     n;

  cfg_mgr_read_lock();

  master_list = cfg_mgr_get_node(_jroot, "modbus_master_list");

  n  = cJSON_GetArraySize(master_list);

  cfg_mgr_unlock();

  return n;
}

void
cfg_mgr_get_modbus_master_at(int ndx, modbus_master_driver_config_t* cfg)
{
  cJSON       *master_list,
              *node,
              *param;
  const char  *str;

  cfg_mgr_read_lock();

  master_list = cfg_mgr_get_node(_jroot, "modbus_master_list");
  node = cJSON_GetArrayItem(master_list, ndx);

  cfg->schedule_finish_delay  = cfg_mgr_get_int(node, "schedule_finish_delay");
  cfg->inter_request_delay    = cfg_mgr_get_int(node, "inter_request_delay");
  cfg->timeout  = (uint32_t)cfg_mgr_get_int(node, "timeout");

  str = cfg_mgr_get_str(node, "protocol");
  if(strcmp(str, "tcp") == 0)
  {
    cfg->protocol   = modbus_master_driver_type_tcp;
    cfg->dest_ip    = cfg_mgr_get_str(node, "dest_ip");
    cfg->dest_port  = cfg_mgr_get_int(node, "dest_port");
  }
  else
  {
    cfg->protocol = modbus_master_driver_type_rtu;
    param = cfg_mgr_get_node(node, "param");
    cfg->serial_port = cfg_mgr_get_str(param, "serial_port");
    cfg_mgr_get_serial_cfg(param, &cfg->serial_cfg);
  }

  cfg_mgr_unlock();
}

int
cfg_mgr_get_modbus_master_num_regs(int master_ndx)
{
  cJSON       *master_list,
              *master,
              *reg_map;
  int         n;

  cfg_mgr_read_lock();

  master_list = cfg_mgr_get_node(_jroot, "modbus_master_list");
  master      = cJSON_GetArrayItem(master_list, master_ndx);
  reg_map     = cfg_mgr_get_node(master, "reg_map");

  n =  cJSON_GetArraySize(reg_map);

  cfg_mgr_unlock();
  return n;
}

void
cfg_mgr_get_modbus_master_reg(int master_ndx, int reg_ndx, modbus_address_t* mb_reg, uint32_t* chnl,
    modbus_reg_codec_t* codec)
{
  cJSON     *master_list,
            *master,
            *reg_map,
            *reg;

  cfg_mgr_read_lock();

  master_list = cfg_mgr_get_node(_jroot, "modbus_master_list");
  master      = cJSON_GetArrayItem(master_list, master_ndx);
  reg_map     = cfg_mgr_get_node(master, "reg_map");
  reg         = cJSON_GetArrayItem(reg_map, reg_ndx);

  cfg_mgr_get_modbus_reg_cfg(reg, mb_reg, chnl);
  cfg_mgr_get_modbus_reg_codec(reg, codec);

  cfg_mgr_unlock();
}

int
cfg_mgr_get_modbus_master_num_request_schedules(int master_ndx)
{
  cJSON       *master_list,
              *master,
              *request_schedule;
  int         n;

  cfg_mgr_read_lock();

  master_list         = cfg_mgr_get_node(_jroot, "modbus_master_list");
  master              = cJSON_GetArrayItem(master_list, master_ndx);
  request_schedule    = cfg_mgr_get_node(master, "request_schedule");

  n =  cJSON_GetArraySize(request_schedule);

  cfg_mgr_unlock();
  return n;
}

void
cfg_mgr_get_modbus_slave_request_schedule(int master_ndx, int req_ndx, modbus_master_driver_request_config_t* cfg)
{
  cJSON       *master_list,
              *master,
              *request_schedule,
              *request;
  const char* str;

  cfg_mgr_read_lock();

  master_list         = cfg_mgr_get_node(_jroot, "modbus_master_list");
  master              = cJSON_GetArrayItem(master_list, master_ndx);
  request_schedule    = cfg_mgr_get_node(master, "request_schedule");
  request             = cJSON_GetArrayItem(request_schedule, req_ndx);

  cfg->slave_id   = cfg_mgr_get_int(request, "slave");

  str = cfg_mgr_get_str(request, "op");
  if(strcmp(str, "write") == 0)
  {
    cfg->op = modbus_reg_op_write;
  }
  else
  {
    cfg->op = modbus_reg_op_read;
  }

  cfg->reg        = cfg_mgr_get_modbus_reg_type(request, "reg");
  cfg->start_addr = cfg_mgr_get_int(request, "reg_addr");
  cfg->num_regs   = cfg_mgr_get_int(request, "num_regs");

  cfg_mgr_unlock();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// channel loading
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int
cfg_mgr_get_num_channels(void)
{
  cJSON       *channels;
  int         n;

  cfg_mgr_read_lock();

  channels = cfg_mgr_get_node(_jroot, "channels");

  n =  cJSON_GetArraySize(channels);

  cfg_mgr_unlock();

  return n;
}

void
cfg_mgr_get_channel_at(int ndx, core_driver_channel_config_t* chnl_cfg)
{
  cJSON       *channels,
              *node;
  const char* str;

  cfg_mgr_read_lock();

  channels = cfg_mgr_get_node(_jroot, "channels");
  node = cJSON_GetArrayItem(channels, ndx);

  chnl_cfg->chnl_num = cfg_mgr_get_int(node, "chnl_num");

  str = cfg_mgr_get_str(node, "chnl_type");
  if(strcmp(str, "digital") == 0)
  {
    chnl_cfg->chnl_type       = channel_type_digital;
    chnl_cfg->init_val.b      = cfg_mgr_get_bool(node, "init_val");
    chnl_cfg->failsafe_val.b  = cfg_mgr_get_bool(node, "failsafe_val");
  }
  else
  {
    chnl_cfg->chnl_type       = channel_type_analog;
    chnl_cfg->init_val.f      = (float)cfg_mgr_get_double(node, "init_val");
    chnl_cfg->failsafe_val.f  = (float)cfg_mgr_get_double(node, "failsafe_val");
    chnl_cfg->min_val         = (float)cfg_mgr_get_double(node, "min_val");
    chnl_cfg->max_val         = (float)cfg_mgr_get_double(node, "max_val");
  }

  str = cfg_mgr_get_str(node, "chnl_dir");
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

  cfg_mgr_unlock();
}

int
cfg_mgr_get_num_lookup_table_of_channel_at(int ndx)
{
  cJSON       *channels,
              *channel,
              *lookup_table;
  int         n;

  cfg_mgr_read_lock();

  channels      = cfg_mgr_get_node(_jroot, "channels");
  channel       = cJSON_GetArrayItem(channels, ndx);
  lookup_table  = cfg_mgr_get_node(channel, "lookup_table");

  n =  cJSON_GetArraySize(lookup_table);
  cfg_mgr_unlock();

  return n;
}

void
cfg_mgr_get_lookup_table_of_channel_at(int chnl_ndx, int lt_ndx, float* raw, float* eng)
{
  cJSON       *channels,
              *channel,
              *lookup_table,
              *entry;

  cfg_mgr_read_lock();

  channels      = cfg_mgr_get_node(_jroot, "channels");
  channel       = cJSON_GetArrayItem(channels, chnl_ndx);
  lookup_table  = cfg_mgr_get_node(channel, "lookup_table");
  entry         =  cJSON_GetArrayItem(lookup_table, lt_ndx);

  *raw          = (float)cfg_mgr_get_double(entry, "raw");
  *eng          = (float)cfg_mgr_get_double(entry, "eng");

  cfg_mgr_unlock();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// alarm loading
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int
cfg_mgr_get_num_alarms(void)
{
  cJSON       *alarms;
  int         n;

  cfg_mgr_read_lock();

  alarms = cfg_mgr_get_node(_jroot, "alarms");

  n =  cJSON_GetArraySize(alarms);

  cfg_mgr_unlock();

  return n;
}

void
cfg_mgr_get_alarm_at(int ndx, core_driver_alarm_config_t* alm_cfg)
{
  cJSON       *alarms,
              *node;

  const char* str;

  cfg_mgr_read_lock();

  alarms = cfg_mgr_get_node(_jroot, "alarms");
  node = cJSON_GetArrayItem(alarms, ndx);

  alm_cfg->alarm_num  = cfg_mgr_get_int(node, "alarm_num");
  alm_cfg->chnl_num   = cfg_mgr_get_int(node, "chnl_num");

  str = cfg_mgr_get_str(node, "trigger_type");
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
    str = cfg_mgr_get_str(node, "set_point");
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
    alm_cfg->set_point.f = (float)cfg_mgr_get_double(node, "set_point");
  }

  str = cfg_mgr_get_str(node, "severity");
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

  alm_cfg->delay = cfg_mgr_get_int(node, "delay");

  cfg_mgr_unlock();
}

//
// XXX should be called with read lock on config
//
const char*
cfg_mgr_get_json_string_use_lock_before_call(int* len)
{
  *len = _json_str_len;
  return _json_str;
}

void
cfg_mgr_get_webserver_config(webserver_config_t* cfg)
{
  cJSON*      web;

  cfg_mgr_read_lock();

  web = cfg_mgr_get_node(_jroot, "web");

  cfg->http_address = cfg_mgr_get_str(web, "http_address");
  cfg->doc_root     = cfg_mgr_get_str(web, "doc_root");

  cfg_mgr_unlock();
}

bool
cfg_mgr_update_alarm_cfg(uint32_t alarm_num, alarm_runtime_config_t* cfg)
{
  cJSON         *alarm;
  const char    *str;

  cfg_mgr_write_lock();

  alarm = cfg_mgr_find_alarm(alarm_num);
  if(alarm == NULL)
  {
    debug_log("can't find alarm %d\n", alarm_num);
    cfg_mgr_unlock();
    return FALSE;
  }

  // step 1. update JSON database
  str = cfg_mgr_get_str(alarm, "trigger_type");

  cJSON_DeleteItemFromObject(alarm, "set_point");

  if(strcmp(str, "digital") == 0)
  {
    cJSON_AddStringToObject(alarm, "set_point", cfg->set_point.b == TRUE ? "on" : "off");
  }
  else
  {
    cJSON_AddNumberToObject(alarm, "set_point", cfg->set_point.f);
  }

  cJSON_DeleteItemFromObject(alarm, "delay");
  cJSON_AddNumberToObject(alarm, "delay", cfg->delay);

  // step 2. update alarm manager
  alarm_manager_update_alarm_config(alarm_num, cfg);

  // step 3. regenerate in memeory config and save
  cfg_mgr_udpate_confg_file();

  cfg_mgr_unlock();
  return TRUE;
}
