#include <stdio.h>
#include <stdlib.h>
#include "modbus_regs.h"
#include "io_channel_map.h"
#include "app_config.h"
#include "trace.h"

static int                          _num_modbus_groups;
static modbus_register_list_t*      _modbus_groups;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// privates
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
io_channel_map_init_modbus(void)
{
  int num_groups;

  TRACE(MAIN, "initializing modbus io channel map\n");
  num_groups = app_config_get_num_modbus_reg_list();

  TRACE(MAIN, "got %d modbus groups\n", num_groups);

  _num_modbus_groups = num_groups;

  _modbus_groups = malloc(sizeof(modbus_register_list_t) * num_groups);

  for(int group = 0; group < num_groups; group++)
  {
    int num_regs;

    TRACE(MAIN, "initializing modbus io channel group %d\n", group);
    modbus_register_list_init(&_modbus_groups[group]);


    num_regs = app_config_get_num_modbus_regs_for_a_group(group);

    TRACE(MAIN, "got %d registers for modbus channel group  %d\n", num_regs, group);

    for(int reg_ndx = 0; reg_ndx < num_regs; reg_ndx++)
    {
      io_channel_map_modbus_t   map;

      app_config_get_modbus_reg_in_group(group, reg_ndx, &map);
#if 0
      TRACE(MAIN, "adding modbus reg map: %d, %d, %d, %d\n", group,
          map.mb_addr.reg_type, map.mb_addr.mb_address, map.chnl_num);
#endif

      modbus_register_list_add(&_modbus_groups[group], 
          map.mb_addr.reg_type, map.mb_addr.mb_address, map.chnl_num);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// publics
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
io_channel_map_init(void)
{
  io_channel_map_init_modbus();
}

int
io_channel_map_lookup_modbus(int group_ndx, modbus_address_t*  mb_addr)
{
  modbus_register_t* reg;

  if(group_ndx < 0 || group_ndx >= _num_modbus_groups)
  {
    return -1;
  }

  reg = modbus_register_list_lookup_by_mb_type_addr(&_modbus_groups[group_ndx],
                                                    mb_addr->reg_type,
                                                    mb_addr->mb_address);

  if(reg == NULL)
  {
    return -1;
  }
  return reg->chnl_num;
}
