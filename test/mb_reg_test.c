#include <stdio.h>
#include "CUnit/Basic.h"
#include "CUnit/Console.h"
#include "CUnit/Automated.h"

#include "modbus_regs.h"

void
test_modbus_regs(void)
{
#if 0
  modbus_register_list_t      mb_regs;
  uint32_t                    chnl = 0;
  modbus_register_t*          r;
  int i;

  modbus_register_list_init(&mb_regs);

  for(int  i = 0; i < 10; i++)
  {
    modbus_register_list_add(&mb_regs, 0, modbus_reg_coil,     i, chnl++);
    modbus_register_list_add(&mb_regs, 0, modbus_reg_discrete, i, chnl++);
    modbus_register_list_add(&mb_regs, 0, modbus_reg_holding,  i, chnl++);
    modbus_register_list_add(&mb_regs, 0, modbus_reg_input,    i, chnl++);
  }

  for(i = 0; i < 10; i++)
  {
    r = modbus_register_list_lookup_by_mb_type_addr(&mb_regs, 0, modbus_reg_coil, i);
    CU_ASSERT(r != NULL);

    r = modbus_register_list_lookup_by_mb_type_addr(&mb_regs, 0, modbus_reg_discrete, i);
    CU_ASSERT(r != NULL);

    r = modbus_register_list_lookup_by_mb_type_addr(&mb_regs, 0, modbus_reg_holding, i);
    CU_ASSERT(r != NULL);

    r = modbus_register_list_lookup_by_mb_type_addr(&mb_regs, 0, modbus_reg_input, i);
    CU_ASSERT(r != NULL);
  }

  i = 10;
  r = modbus_register_list_lookup_by_mb_type_addr(&mb_regs, 0, modbus_reg_coil, i);
  CU_ASSERT(r == NULL);

  r = modbus_register_list_lookup_by_mb_type_addr(&mb_regs, 0, modbus_reg_discrete, i);
  CU_ASSERT(r == NULL);

  r = modbus_register_list_lookup_by_mb_type_addr(&mb_regs, 0, modbus_reg_holding, i);
  CU_ASSERT(r == NULL);

  r = modbus_register_list_lookup_by_mb_type_addr(&mb_regs, 0, modbus_reg_input, i);
  CU_ASSERT(r == NULL);

  CU_PASS("test_modbus_regs() succeeded");
#endif
}

void
mb_reg_add_test(CU_pSuite pSuite)
{
  CU_add_test(pSuite, "test_modbus_regs", test_modbus_regs);
}
