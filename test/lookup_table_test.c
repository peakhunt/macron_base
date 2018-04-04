#include <stdio.h>
#include "CUnit/Basic.h"
#include "CUnit/Console.h"
#include "CUnit/Automated.h"

#include "lookup_table.h"

void test_lookup_table(void)
{
  #define SIZE        1024

  lookup_table_t      table;
  float               v;

  lookup_table_init(&table, SIZE);

  for(int i = 0; i < SIZE; i++)
  {
    lookup_table_add(&table, i, i * 100, i);
  }

  lookup_table_build_btree(&table);

  v = lookup_table_interpolate(&table, 5.5);
  printf("5.5 => %.2f\n", v);
  v = lookup_table_interpolate(&table, 0.5);
  printf("0.5 => %.2f\n", v);
  v = lookup_table_interpolate(&table, 9.5);
  printf("9.5 => %.2f\n", v);
  v = lookup_table_interpolate(&table, 7.5);
  printf("7.5 => %.2f\n", v);
  v = lookup_table_interpolate(&table, SIZE - 1);
  printf("%d => %.2f\n", SIZE - 1, v);
  v = lookup_table_interpolate(&table, SIZE);
  printf("%d => %.2f\n", SIZE, v);
  v = lookup_table_interpolate(&table, -1);
  printf("-1 => %.2f\n", v);

  lookup_table_deinit(&table);

}

void
lookup_table_add_test(CU_pSuite pSuite)
{
  CU_add_test(pSuite, "test_lookup_table", test_lookup_table);
}
