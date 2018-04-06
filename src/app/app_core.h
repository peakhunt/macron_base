#ifndef __APP_CORE_DEF_H__
#define __APP_CORE_DEF_H__

#include <stdio.h>
#include "channel.h"

typedef struct
{
  uint32_t            chnl_num;
  channel_type_t      chnl_type;
  channel_direction_t chnl_dir;
} app_channel_config_t;

extern void app_core_init(void);

#endif /* !__APP_CORE_DEF_H__ */
