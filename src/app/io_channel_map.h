#ifndef __IO_CHANNEL_MAP_DEF_H__
#define __IO_CHANNEL_MAP_DEF_H__

#include "common_def.h"
#include "modbus_regs.h"

typedef struct
{
  modbus_address_t            mb_addr;
  int                         chnl_num;
} io_channel_map_modbus_t;

extern void io_channel_map_init(void);
extern int io_channel_map_lookup_modbus(int group_ndx, modbus_address_t*  mb_addr);

#endif /* !__IO_CHANNEL_MAP_DEF_H__ */
