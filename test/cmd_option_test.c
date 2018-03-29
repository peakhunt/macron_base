#include "CUnit/Basic.h"
#include "CUnit/Console.h"
#include "CUnit/Automated.h"

#include "cmd_option.h"

void test_cmd_options1(void)
{
  const cmd_option*   opt;
  char* argv[] = {
    "test",
    //"-h",
    "--foreground",
    //"-f",
  }; 

  opt = cmd_opt_get();

  CU_ASSERT(opt->foreground == FALSE);

  cmd_opt_handle(3, argv);

  CU_ASSERT(opt->foreground == TRUE);
}

void
cmd_option_add_test(CU_pSuite pSuite)
{
  CU_add_test(pSuite, "test_cmd_options1", test_cmd_options1);
}
