#include <stdio.h>
#include "CUnit/Basic.h"
#include "CUnit/Console.h"
#include "CUnit/Automated.h"

#include "lookup_table.h"

void test_lookup_table(void)
{
  #define SIZE        5000000

  lookup_table_t      table;
  float               v;

  lookup_table_init(&table, SIZE);

  for(int i = 0; i < SIZE; i++)
  {
    lookup_table_add(&table, i, i * 100, i);
  }

  lookup_table_build_btree(&table);

  v = lookup_table_interpolate(&table, 5.5);
  CU_ASSERT(v == 550.00f);

  v = lookup_table_interpolate(&table, 0.5);
  CU_ASSERT(v == 50.00f);

  v = lookup_table_interpolate(&table, 9.5);
  CU_ASSERT(v == 950.00f);

  v = lookup_table_interpolate(&table, 7.5);
  CU_ASSERT(v == 750.00f);

  v = lookup_table_interpolate(&table, SIZE - 1);
  CU_ASSERT(v == 499999904.00f);

  v = lookup_table_interpolate(&table, SIZE);
  CU_ASSERT(v == 500000000.00f);

  v = lookup_table_interpolate(&table, -1);
  CU_ASSERT(v == -100.00f);

  lookup_table_deinit(&table);

  CU_PASS("test_lookup_table() succeeded");

}

void
lookup_table_add_test(CU_pSuite pSuite)
{
  CU_add_test(pSuite, "test_lookup_table", test_lookup_table);
}
