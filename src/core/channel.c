#include <stdlib.h>
#include "channel.h"

channel_t*
channel_alloc(uint16_t chnl_num, channel_type_t chnl_type, channel_direction_t chnl_dir)
{
  channel_t*    chnl;

  chnl = malloc(sizeof(channel_t));

  chnl->chnl_num  = chnl_num;
  chnl->chnl_type = chnl_type;
  chnl->chnl_dir  = chnl_dir;

  chnl->eng_value.f       = 0.0f;
  chnl->raw_value         = 0;
  chnl->raw_value_queued  = 0;

  return chnl;
}

void
channel_update_raw_value(channel_t* chnl)
{
  // FIXME
}

void
channel_update_eng_value(channel_t* chnl)
{
  // FIXME
}
