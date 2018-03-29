#ifndef __CMD_OPTION_DEF_H__
#define __CMD_OPTION_DEF_H__

#include "common_def.h"

typedef struct
{
  bool        foreground;
} cmd_option;

extern void cmd_opt_handle(int argc, char** argv);
extern const cmd_option* cmd_opt_get(void);

#endif /* !__CMD_OPTION_DEF_H__ */
