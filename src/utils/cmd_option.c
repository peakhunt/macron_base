//
// cmd_opt.c
// written by hkim
//
// revision history
// - Mar/26/2018    - initial release
//
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "cmd_option.h"

///////////////////////////////////////////////////////////////////////////////
//
// private variables
//
///////////////////////////////////////////////////////////////////////////////
static cmd_option _cmd_option =
{
  .foreground   = FALSE,
};

//
// option config
// ':'  means required_argument
// '::' means optional_argument
//
const char*             _short_opts = "fh";
static struct option    _long_opts[] = 
{
  {
    .name     = "foreground",
    .has_arg  = no_argument,
    .flag     = NULL,
    .val      = 'f',
  },
  {
    .name     = "help",
    .has_arg  = no_argument,
    .flag     = NULL,
    .val      = 'h',
  },
  // last element: must be all zero
  {
    .name     = NULL,
    .has_arg  = 0,
    .flag     = NULL,
    .val      = 0,
  }
};

///////////////////////////////////////////////////////////////////////////////
//
// privates interfaces
//
///////////////////////////////////////////////////////////////////////////////
#define HELP_TEXT                             \
"--help       : show this command\n"          \
"--foreground : run in foreground mode\n"      

static void
help_cmd_option(void)
{
  fprintf(stderr, "%s", HELP_TEXT);
  exit(-1);
}

static void
handle_cmd_options(int c)
{
  switch(c)
  {
  case 'f':
    _cmd_option.foreground = TRUE;
    break;

  case 'h':
    help_cmd_option();
    break;

  case '?':   // unknown
  default:
    help_cmd_option();
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
///////////////////////////////////////////////////////////////////////////////
const cmd_option*
cmd_opt_get(void)
{
  return &_cmd_option;
}

void
cmd_opt_handle(int argc, char** argv)
{
  int   c;
  int   option_index;

  while(TRUE)
  {
    c = getopt_long(argc, argv, _short_opts, _long_opts, &option_index);
    if(c == -1)
    {
      break;
    }

    handle_cmd_options(c);
  }
}
