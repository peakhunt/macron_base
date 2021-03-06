#include <stdio.h>
#include <stdlib.h>
#include "modbus_regs.h"

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
modbus_register_list_init(modbus_register_list_t* mb_list)
{
  INIT_LIST_HEAD(&mb_list->regs_list);

  mb_list->num_regs = 0;

  bhash_init_malloc(&mb_list->hash_by_mb_addr,
      MODBUS_REGS_NUM_BUCKETS, 
      offsetof(modbus_register_t, bh_by_mb_addr),
      offsetof(modbus_register_t, mb_addr),
      sizeof(modbus_address_t),
      NULL);
}

void
modbus_register_list_add(modbus_register_list_t* mb_list, uint32_t slave_id,
    modbus_reg_type_t reg_type, uint32_t mb_addr, uint32_t chnl_num, modbus_reg_mapping_to_t mapping_to,
    modbus_reg_codec_t* codec)
{
  modbus_register_t*    reg;

  reg = (modbus_register_t*)malloc_zero(sizeof(modbus_register_t));

  INIT_LIST_HEAD(&reg->le);

  reg->mb_addr.slave_id     = slave_id;
  reg->mb_addr.mb_address   = mb_addr;
  reg->mb_addr.reg_type     = reg_type;
  reg->chnl_num             = chnl_num;
  reg->mapping_to           = mapping_to;

  if(codec)
  {
    reg->codec               = *codec;
  }
  else
  {
    reg->codec.codec_type   = modbus_reg_codec_type_data_and_status;
    reg->codec.d_mask       = 0xffff;
    reg->codec.s_mask       = 0x0;
    reg->codec.d_shift      = 0;
    reg->codec.s_shift      = 0;
    reg->codec.fault        = 0;
    reg->codec.ok           = 0;
  }

  bhash_add(&mb_list->hash_by_mb_addr, (void*)reg);

  list_add_tail(&reg->le, &mb_list->regs_list);
}

modbus_register_t*
modbus_register_list_lookup_by_mb_type_addr(modbus_register_list_t* mb_list,
    uint32_t slave_id, modbus_reg_type_t reg_type, uint32_t mb_addr)
{
  modbus_address_t  a;

  a.slave_id      = slave_id;
  a.mb_address    = mb_addr;
  a.reg_type      = reg_type;

  return (modbus_register_t*)bhash_lookup(&mb_list->hash_by_mb_addr, (void*)&a);
}
