#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "trace.h"

#include "cli.h"

extern void cli_telnet_intf_init(int port);
extern void cli_serial_intf_init(const char* port, SerialConfig* scfg);

////////////////////////////////////////////////////////////////////////////////
//
// private definitions
//
////////////////////////////////////////////////////////////////////////////////
#define CLI_MAX_COLUMNS_PER_LINE          512
#define VERSION                           "Macron Base CLI V1.0"

#define CLI_EOL_CHAR                      '\r'

////////////////////////////////////////////////////////////////////////////////
//
// private prototypes
//
////////////////////////////////////////////////////////////////////////////////
static void cli_command_help(cli_intf_t* intf, int argc, const char** argv);
static void cli_command_version(cli_intf_t* intf, int argc, const char** argv);
static void cli_command_trace_status(cli_intf_t* intf, int argc, const char** argv);
static void cli_command_trace_set(cli_intf_t* intf, int argc, const char** argv);
static void cli_command_cli(cli_intf_t* intf, int argc, const char** argv);

////////////////////////////////////////////////////////////////////////////////
//
// private variables
//
////////////////////////////////////////////////////////////////////////////////
static LIST_HEAD(_cli_intf_list);

static cli_command_t    _core_commands[] =
{
  {
    "help",
    "show this command",
    cli_command_help,
  },
  {
    "version",
    "show version",
    cli_command_version,
  },
  {
    "trace_status",
    "show debug trace status",
    cli_command_trace_status,
  },
  {
    "trace_set",
    "enable/disable trace on a component",
    cli_command_trace_set,
  },
  {
    "cli",
    "show CLI status",
    cli_command_cli,
  },
};

static cli_command_t*   _user_commands;
static int              _num_user_commands;

static char             _print_buffer[CLI_MAX_COLUMNS_PER_LINE + 1];
static char             _prompt[128 + 8];

////////////////////////////////////////////////////////////////////////////////
//
// core cli command handlers
//
////////////////////////////////////////////////////////////////////////////////
static void
cli_command_help(cli_intf_t* intf, int argc, const char** argv)
{
  size_t i;

  cli_printf(intf, CLI_EOL);

  for(i = 0; i < sizeof(_core_commands)/sizeof(cli_command_t); i++)
  {
    cli_printf(intf, "%-20s: ", _core_commands[i].command);
    cli_printf(intf, "%s"CLI_EOL, _core_commands[i].help_str);
  }

  for(i = 0; i < _num_user_commands; i++)
  {
    cli_printf(intf, "%-20s: ", _user_commands[i].command);
    cli_printf(intf, "%s"CLI_EOL, _user_commands[i].help_str);
  }
}

static void
cli_command_version(cli_intf_t* intf, int argc, const char** argv)
{
  cli_printf(intf, CLI_EOL);
  cli_printf(intf, "%s"CLI_EOL, VERSION);
}

static void
cli_command_trace_status(cli_intf_t* intf, int argc, const char** argv)
{
  int num_trace_str;
  const char** trace_str = trace_get_strs(&num_trace_str);

  for(int i = 0; i < num_trace_str; i++)
  {
    cli_printf(intf, "%-20s:        %s"CLI_EOL,
        trace_str[i],
        trace_is_on(i) ? "enabled" : "disabled");
  }
}

static void
cli_command_trace_set(cli_intf_t* intf, int argc, const char** argv)
{
  int trace_ndx;

  cli_printf(intf, CLI_EOL);

  if(argc != 3 || 
     !(strcmp(argv[1], "on") == 0 || strcmp(argv[1], "off") == 0))
  {
    cli_printf(intf, "invalid argument!!!"CLI_EOL CLI_EOL);
    cli_printf(intf, "%s on|off component"CLI_EOL, argv[0]);
    return;
  }

  trace_ndx = trace_get_str_ndx(argv[2]);
  if(trace_ndx == -1)
  {
    cli_printf(intf, "unknown component: %s"CLI_EOL, argv[2]);
    return;
  }

  if(strcmp(argv[1], "on") == 0)
  {
    cli_printf(intf, "enabled trace for %s"CLI_EOL, argv[2]);
    trace_on(trace_ndx);
  }
  else {
    cli_printf(intf, "disabled trace for %s"CLI_EOL, argv[2]);
    trace_off(trace_ndx);
  }
}

static void
cli_command_cli(cli_intf_t* intf, int argc, const char** argv)
{
  if(argc != 2)
  {
    goto invalid_command;
  }

  if(strcmp(argv[1], "status") == 0)
  {
    cli_intf_t* i;

    list_for_each_entry(i, &_cli_intf_list, le)
    {
      if(i->print_status)
      {
        i->print_status(intf);
      }
    }
  }
  else
  {
    goto invalid_command;
  }

  return;

invalid_command:
  cli_printf(intf, "invalid argument!!!"CLI_EOL CLI_EOL);
  cli_printf(intf, "%s status"CLI_EOL, argv[0]);
  return;
}

////////////////////////////////////////////////////////////////////////////////
//
// cli core
//
////////////////////////////////////////////////////////////////////////////////
static void
cli_execute_command(cli_intf_t* intf, char* cmd)
{
  static const char*    argv[CLI_COMMAND_MAX_ARGS];
  int                   argc = 0;
  size_t                i;
  char                  *s, *t;

  while((s = strtok_r(argc  == 0 ? cmd : NULL, " \t", &t)) != NULL)
  {
    if(argc >= CLI_COMMAND_MAX_ARGS)
    {
      cli_printf(intf, CLI_EOL"Error: too many arguments"CLI_EOL);
      return;
    }
    argv[argc++] = s;
  }

  if(argc == 0)
  {
    return;
  }

  for(i = 0; i < sizeof(_core_commands)/sizeof(cli_command_t); i++)
  {
    if(strcmp(_core_commands[i].command, argv[0]) == 0)
    {
      cli_printf(intf, CLI_EOL"Executing %s"CLI_EOL, argv[0]);
      _core_commands[i].handler(intf, argc, argv);
      return;
    }
  }

  for(i = 0; i < _num_user_commands; i++)
  {
    if(strcmp(_user_commands[i].command, argv[0]) == 0)
    {
      cli_printf(intf, CLI_EOL"Executing %s"CLI_EOL, argv[0]);
      _user_commands[i].handler(intf, argc, argv);
      return;
    }
  }
  cli_printf(intf, "%s", CLI_EOL"Unknown Command: ");
  cli_printf(intf, "%s", argv[0]);
  cli_printf(intf, "%s", CLI_EOL);
}

void
cli_printf(cli_intf_t* intf, const char* fmt, ...)
{
  va_list   args;
  int       len;

  va_start(args, fmt);
  len = vsnprintf(_print_buffer, CLI_MAX_COLUMNS_PER_LINE, fmt, args);
  va_end(args);

  intf->put_tx_data(intf, _print_buffer, len);
}

static inline void
cli_prompt(cli_intf_t* intf)
{
  intf->put_tx_data(intf, _prompt, strlen(_prompt));
}

////////////////////////////////////////////////////////////////////////////////
//
// public interface
//
////////////////////////////////////////////////////////////////////////////////
void
cli_init(cli_config_t* cfg, cli_command_t* cmds, int num_cmds)
{
  _user_commands      = cmds;
  _num_user_commands  = num_cmds;

  snprintf(_prompt, 128,"\r\n%s", cfg->prompt);

  if(cfg->telnet_enabled)
  {
    cli_telnet_intf_init(cfg->tcp_port);
  }

  if(cfg->serial_enabled)
  {
    cli_serial_intf_init(cfg->serial_port, &cfg->serial_cfg);
  }
}

void
cli_handle_rx(cli_intf_t* intf, uint8_t* data, int len)
{
  char    b;
  int     i;

  for(i = 0; i < len; i++)
  {
    b = (char)data[i];

    if(b != CLI_EOL_CHAR && intf->cmd_buffer_ndx < CLI_MAX_COMMAND_LEN)
    {
      if(b == '\b' || b == 0x7f)
      {
        if(intf->cmd_buffer_ndx > 0)
        {
          //cli_printf(intf, "%c%c%c", b, 0x20, b);
          cli_printf(intf, "%c%c%c", 0x08, 0x20, 0x08);
          intf->cmd_buffer_ndx--;
        }
      }
      else
      {
        cli_printf(intf, "%c", b);
        intf->cmd_buffer[intf->cmd_buffer_ndx++] = b;
      }
    }
    else if(b == CLI_EOL_CHAR)
    {
      intf->cmd_buffer[intf->cmd_buffer_ndx++] = '\0';

      cli_execute_command(intf, (char*)intf->cmd_buffer);

      intf->cmd_buffer_ndx = 0;
      cli_prompt(intf);
    }
  }
}

void
cli_intf_register(cli_intf_t* intf)
{
  list_add_tail(&intf->le, &_cli_intf_list);
}

void
cli_intf_unregister(cli_intf_t* intf)
{
  list_del_init(&intf->le);
}
