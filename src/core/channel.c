#include <stdlib.h>
#include "channel.h"

static void
channel_update_bound_io_var(channel_t* chnl)
{
  if(chnl->io_var == NULL)
  {
    return;
  }

  switch(chnl->io_var->vtype)
  {
  case io_var_enum_bool:
    *((bool*)chnl->io_var->varptr) = chnl->eng_value.b;
    break;

  case io_var_enum_double:
    *((double*)chnl->io_var->varptr) = chnl->eng_value.f;
    break;

  case io_var_enum_unsigned_int:
    *((uint32_t*)chnl->io_var->varptr) = (uint32_t)chnl->eng_value.f;
    break;

  case io_var_enum_int:
    *((int32_t*)chnl->io_var->varptr) = (int32_t)chnl->eng_value.f;
    break;
  }
}

static void
channel_update_from_bound_io_var(channel_t* chnl)
{
  if(chnl->io_var == NULL)
  {
    return;
  }

  switch(chnl->io_var->vtype)
  {
  case io_var_enum_bool:
    chnl->eng_value.b = *((bool*)chnl->io_var->varptr);
    break;

  case io_var_enum_double:
    chnl->eng_value.f = *((double*)chnl->io_var->varptr);
    break;

  case io_var_enum_unsigned_int:
    chnl->eng_value.f = *((uint32_t*)chnl->io_var->varptr);
    break;

  case io_var_enum_int:
    chnl->eng_value.f = *((int32_t*)chnl->io_var->varptr);
    break;
  }
}

static void
channel_convert_raw_to_sensor_value(channel_t* chnl)
{
  switch(chnl->sensor_type)
  {
  case channel_sensor_wago_4_20ma:
    // FIXME
    chnl->raw_sensor_value = chnl->raw_value;
    break;
  case channel_sensor_wago_pt100:
    // FIXME
    chnl->raw_sensor_value = chnl->raw_value;
    break;

  default:
    chnl->raw_sensor_value = chnl->raw_value;
    break;
  }
}

static void
channel_convert_sensor_value_to_raw(channel_t* chnl)
{
  switch(chnl->sensor_type)
  {
  case channel_sensor_wago_4_20ma:
    // FIXME
    chnl->raw_value = (uint32_t)chnl->raw_sensor_value;
    break;
  case channel_sensor_wago_pt100:
    // FIXME
    chnl->raw_value = (uint32_t)chnl->raw_sensor_value;
    break;

  default:
    chnl->raw_value = (uint32_t)chnl->raw_sensor_value;
    break;
  }
}

static void
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
      chnl->raw_sensor_value = lookup_table_interpolate_reverse(
          chnl->lookup_table, chnl->eng_value.f);
    }
    else
    {
      chnl->raw_sensor_value = chnl->eng_value.f;
    }

    channel_convert_sensor_value_to_raw(chnl);
    break;
  }
}

channel_t*
channel_alloc(uint32_t chnl_num, channel_type_t chnl_type, channel_direction_t chnl_dir)
{
  channel_t*    chnl;

  chnl = malloc_zero(sizeof(channel_t));

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

  chnl->raw_sensor_value  = 0.0f;

  chnl->raw_value         = 0;
  chnl->raw_value_queued  = 0;
  chnl->raw_value_avail   = FALSE;

  chnl->lookup_table      = NULL;

  publisher_init(&chnl->chnl_update);

  chnl->io_var    = NULL;

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
channel_set_eng_value(channel_t* chnl, channel_eng_value_t eng)
{
  // step 1. set engineering value
  chnl->eng_value = eng;

  // step 2. update raw value
  channel_update_raw_value(chnl);

  // step 3. update bound io var
  channel_update_bound_io_var(chnl);
}

void
channel_update_input(channel_t* chnl)
{
  switch(chnl->chnl_type)
  {
  case channel_type_digital:
    chnl->eng_value.b = chnl->raw_value == 0 ? FALSE : TRUE;
    break;

  case channel_type_analog:
    channel_convert_raw_to_sensor_value(chnl);

    if(chnl->lookup_table != NULL)
    {
      chnl->eng_value.f = lookup_table_interpolate(
          chnl->lookup_table, chnl->raw_sensor_value);
    }
    else
    {
      chnl->eng_value.f = chnl->raw_sensor_value;
    }
    break;
  }
  channel_update_bound_io_var(chnl);
}

void
channel_update_output(channel_t* chnl)
{
  channel_update_from_bound_io_var(chnl);

  channel_update_raw_value(chnl);
}

void
channel_add_observer(channel_t* chnl, observer_t* obs)
{
  publisher_add_observer(&chnl->chnl_update, obs);
}

void
channel_set_io_var(channel_t* chnl, io_var_t* io_var)
{
  chnl->io_var = io_var;

  channel_update_bound_io_var(chnl);
}
