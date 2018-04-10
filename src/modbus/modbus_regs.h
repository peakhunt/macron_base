#ifndef __MODBUS_REGS_DEF_H__
#define __MODBUS_REGS_DEF_H__

#include "common_def.h"
#include "list.h"
#include "bhash.h"

#define MODBUS_REGS_NUM_BUCKETS       512

typedef enum
{
  modbus_reg_coil,
  modbus_reg_discrete,
  modbus_reg_holding,
  modbus_reg_input
} modbus_reg_type_t;

typedef enum
{
  modbus_reg_op_read,
  modbus_reg_op_write
} modbus_reg_op_t;

typedef struct
{
  uint32_t              slave_id;
  uint32_t              mb_address;
  modbus_reg_type_t     reg_type;
} modbus_address_t;


typedef struct
{
  struct list_head      le;
  BHashElement          bh_by_mb_addr;
  modbus_address_t      mb_addr;
  uint32_t              chnl_num;
} modbus_register_t;

typedef struct
{
  struct list_head      regs_list;
  BHashContext          hash_by_mb_addr;
  uint32_t              num_regs;
} modbus_register_list_t;

extern void modbus_register_list_init(modbus_register_list_t* mb_list);
extern void modbus_register_list_add(modbus_register_list_t* mb_list,
    uint32_t slave_id, modbus_reg_type_t reg_type, uint32_t mb_addr, uint32_t chnl_num);
extern modbus_register_t* modbus_register_list_lookup_by_mb_type_addr(modbus_register_list_t* mb_list,
    uint32_t slave_id, modbus_reg_type_t reg_type, uint32_t mb_addr);


#endif /* !__MODBUS_REGS_DEF_H__ */
