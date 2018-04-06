#include <stdio.h>
#include "CUnit/Basic.h"
#include "CUnit/Console.h"
#include "CUnit/Automated.h"

#include "lookup_table.h"

#define SIZE        5000000
static lookup_table_item_t   items[SIZE];

void test_lookup_table(void)
{
  lookup_table_t      table;
  float               v;
  lookup_table_item_t test_set[] = 
  {
    {   
      .v1 = 5.5f,
      .v2 = 550.0f
    },
    {   
      .v1 = 0.5f,
      .v2 = 50.0f,
    },
    {   
      .v1 = 9.5f,
      .v2 = 950.0f
    },
    {   
      .v1 = 7.5f,
      .v2 = 750.0f
    },
    {   
      .v1 = SIZE - 1,
      .v2 = 499999904.00f,
    },
    {   
      .v1 = SIZE,
      .v2 = 500000000.00f,
    },
    {   
      .v1 = -1,
      .v2 = -100.0f
    },
  };

  for(int i = 0; i < SIZE; i++)
  {
    items[i].v1 = i;
    items[i].v2 = i * 100;
  }

  lookup_table_init(&table, items, SIZE);

  for(int i = 0; i < sizeof(test_set)/sizeof(lookup_table_item_t); i++)
  {
    v = lookup_table_interpolate(&table, test_set[i].v1);
    CU_ASSERT(v == test_set[i].v2);

    v = lookup_table_interpolate_reverse(&table, test_set[i].v2);
    CU_ASSERT(v == test_set[i].v1);
  }
}

void
lookup_table_add_test(CU_pSuite pSuite)
{
  CU_add_test(pSuite, "test_lookup_table", test_lookup_table);
}
