#include <stdio.h>
#include "CUnit/Basic.h"
#include "CUnit/Console.h"
#include "CUnit/Automated.h"

#include "lookup_table.h"

void test_lookup_table(void)
{
  lookup_table_t      table;

  lookup_table_init(&table, 10);

  for(int i = 0; i < 10; i++)
  {
    lookup_table_add(&table, i, i * 10, i);
  }

  lookup_table_build_btree(&table);

  printf("\n========tree ndx========\n");
  for(int i = 0; i < 10; i++)
  {
    printf("%d\n", table.tree_ndx[i]);
  }
}

void
lookup_table_add_test(CU_pSuite pSuite)
{
  CU_add_test(pSuite, "test_lookup_table", test_lookup_table);
}
