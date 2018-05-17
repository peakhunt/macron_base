#include <stdlib.h>
#include "channel.h"

channel_t*
channel_alloc(uint32_t chnl_num, channel_type_t chnl_type, channel_direction_t chnl_dir)
{
  channel_t*    chnl;

  chnl = malloc(sizeof(channel_t));

  chnl->chnl_num  = chnl_num;
  chnl->chnl_type = chnl_type;
  chnl->chnl_dir  = chnl_dir;

  chnl->eng_value.f       = 0.0f;
  chnl->eng_value.b       = FALSE;

  chnl->init_value.f      = 0.0f;
  chnl->init_value.b      = FALSE;

  chnl->failsafe_value.f  = 0.0f;
  chnl->failsafe_value.b  = FALSE;

  chnl->min_val           = 0.0f;
  chnl->max_val           = 0.0f;

  chnl->sensor_fault      = FALSE;

  chnl->raw_value         = 0;
  chnl->raw_value_queued  = 0;
  chnl->raw_value_avail   = FALSE;

  chnl->lookup_table      = NULL;

  publisher_init(&chnl->chnl_update);

  return chnl;
}

channel_t*
channel_alloc_digital(uint32_t chnl_num, channel_direction_t chnl_dir)
{
  return channel_alloc(chnl_num, channel_type_digital, chnl_dir);
}

channel_t*
channel_alloc_analog(uint32_t chnl_num, channel_direction_t chnl_dir, lookup_table_t* lookup_table)
{
  channel_t* chnl;

  chnl = channel_alloc(chnl_num, channel_type_analog, chnl_dir);
  if(chnl != NULL)
  {
    chnl->lookup_table = lookup_table;
  }
  return chnl;
}

void
channel_set_lookup_table(channel_t* chnl, lookup_table_t* lookup_table)
{
  if(chnl->lookup_table != NULL)
  {
    lookup_table_deinit(chnl->lookup_table);
  }

  chnl->lookup_table = lookup_table;
}

void
channel_update_raw_value(channel_t* chnl)
{
  switch(chnl->chnl_type)
  {
  case channel_type_digital:
    chnl->raw_value = chnl->eng_value.b;
    break;

  case channel_type_analog:
    if(chnl->lookup_table != NULL)
    {
      chnl->raw_value = (uint32_t)lookup_table_interpolate_reverse(
          chnl->lookup_table, chnl->eng_value.f);
    }
    else
    {
      chnl->raw_value = (uint32_t)chnl->eng_value.f;
    }
    break;
  }
}

void
channel_update_eng_value(channel_t* chnl)
{
  switch(chnl->chnl_type)
  {
  case channel_type_digital:
    chnl->eng_value.b = chnl->raw_value == 0 ? FALSE : TRUE;
    break;

  case channel_type_analog:
    if(chnl->lookup_table != NULL)
    {
      chnl->eng_value.f = lookup_table_interpolate(
          chnl->lookup_table, chnl->raw_value);
    }
    else
    {
      chnl->eng_value.f = chnl->raw_value;
    }
    break;
  }
}

void
channel_add_observer(channel_t* chnl, observer_t* obs)
{
  publisher_add_observer(&chnl->chnl_update, obs);
}
