#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "app_config.h"
#include "config_reader.h"
#include "debug_log.h"
#include "cJSON.h"

#define APP_CONFIG_DEFAULT_FILE           "config/macron_base.json"
#define APP_CONFIG_ERROR_EXIT_VALUE       -3  

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// module privates
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static cJSON*     _jroot = NULL;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// utilities
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static inline cJSON*
app_config_get_node(cJSON* node, const char* name)
{
  cJSON* child;

  child = cJSON_GetObjectItem(node, name);
  if(child == NULL)
  {
    debug_log("configuration error: %s doesn't exist under %s", name, node->string);
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

  debug_log("configuration error: invalid serial parity: %s, for %s under %s", str, name, node->string);
  exit(APP_CONFIG_ERROR_EXIT_VALUE);
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

  _jroot = config_reader_init(file);
  if(_jroot == NULL)
  {
    debug_log("failed to load config file: %s\n", file);
    exit(-2);
  }
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

  cli = app_config_get_node(_jroot, "cli");

  cli_cfg->prompt = app_config_get_node(cli, "prompt")->valuestring;

  node = app_config_get_node(cli, "telnet");
  cli_cfg->telnet_enabled = app_config_get_bool(node, "enabled");
  cli_cfg->tcp_port       = app_config_get_int(node, "port");

  node = app_config_get_node(cli, "serial");
  cli_cfg->serial_enabled = app_config_get_bool(node, "enabled");
  cli_cfg->serial_port = app_config_get_str(node, "serial_port");
  cli_cfg->serial_cfg.baud = app_config_get_int(node, "baud");
  cli_cfg->serial_cfg.data_bit = app_config_get_int(node, "data_bit");
  cli_cfg->serial_cfg.stop_bit = app_config_get_int(node, "stop_bit");
  cli_cfg->serial_cfg.parity = app_config_get_serial_parity(node, "parity");
}
