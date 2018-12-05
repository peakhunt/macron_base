#ifndef __IO_VAR_DEF_H__
#define __IO_VAR_DEF_H__

#include "common_def.h"

typedef enum
{
  io_var_enum_bool,
  io_var_enum_double,
  io_var_enum_unsigned_int,
  io_var_enum_int,
} io_var_enum_t;

typedef struct
{
  uint32_t            chnl_num;
  io_var_enum_t       vtype;
  void*               varptr;
} io_var_t;

#endif /* !__IO_VAR_DEF_H__ */
