#include "CUnit/Basic.h"
#include "CUnit/Console.h"
#include "CUnit/Automated.h"

#include <stdlib.h>
#include <stdio.h>

extern void cmd_option_add_test(CU_pSuite pSuite);
extern void lookup_table_add_test(CU_pSuite pSuite);
extern void mb_reg_add_test(CU_pSuite pSuite);

int init_suite_success(void) { return 0; }
int init_suite_failure(void) { return -1; }
int clean_suite_success(void) { return 0; }
int clean_suite_failure(void) { return -1; }

int
main(void)
{
  CU_pSuite pSuite = NULL;

  /* initialize the CUnit test registry */
  if (CUE_SUCCESS != CU_initialize_registry())
    return CU_get_error();

  /* add a suite to the registry */
  pSuite = CU_add_suite("Suite_success", init_suite_success, clean_suite_success);
  if (NULL == pSuite) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  cmd_option_add_test(pSuite);
  lookup_table_add_test(pSuite);
  mb_reg_add_test(pSuite);

  /* Run all tests using the basic interface */
  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  printf("\n");
  CU_basic_show_failures(CU_get_failure_list());
  printf("\n\n");

  /* Run all tests using the automated interface */
  //CU_automated_run_tests();
  //CU_list_tests_to_file();

  /* Run all tests using the console interface */
  //CU_console_run_tests();

  /* Clean up registry and return */
  CU_cleanup_registry();
  return CU_get_error();
}
