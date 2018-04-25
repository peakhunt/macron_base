#include <stdio.h>
#include <stdint.h>
#include "CUnit/Basic.h"
#include "CUnit/Console.h"
#include "CUnit/Automated.h"

#include "lookup_table.h"
#include "indexer.h"

void test_indexer(void)
{
  indexer_t   indexer;
  int         ndx;
  uint32_t    v;

  indexer_init(&indexer, 16);

  indexer_set(&indexer, 0, 0);
  indexer_set(&indexer, 1, 5);
  indexer_set(&indexer, 2, 13);
  indexer_set(&indexer, 3, 16);
  indexer_set(&indexer, 4, 17);
  indexer_set(&indexer, 5, 20);
  indexer_set(&indexer, 6, 21);
  indexer_set(&indexer, 7, 22);
  indexer_set(&indexer, 8, 23);
  indexer_set(&indexer, 9, 24);
  indexer_set(&indexer, 10, 28);
  indexer_set(&indexer, 11, 30);
  indexer_set(&indexer, 12, 25);
  indexer_set(&indexer, 13, 18);
  indexer_set(&indexer, 14, 40);
  indexer_set(&indexer, 15, 50);

  indexer_build(&indexer);

  ndx = indexer_find_equal_or_bigger(&indexer, 0);
  CU_ASSERT(ndx != -1);
  v = indexer_get(&indexer, ndx);
  CU_ASSERT(v == 0);

  ndx = indexer_find_equal_or_bigger(&indexer, 100);
  CU_ASSERT(ndx == -1);

  ndx = indexer_find_equal_or_bigger(&indexer, 7);
  CU_ASSERT(ndx != -1);
  v = indexer_get(&indexer, ndx);
  CU_ASSERT(v == 13);

  ndx = indexer_find_equal_or_bigger(&indexer, 28);
  CU_ASSERT(ndx != -1);
  v = indexer_get(&indexer, ndx);
  CU_ASSERT(v == 28);

  ndx = indexer_find_equal_or_bigger(&indexer, 43);
  CU_ASSERT(ndx != -1);
  v = indexer_get(&indexer, ndx);
  CU_ASSERT(v == 50);

  ndx = indexer_find_equal_or_bigger(&indexer, 50);
  CU_ASSERT(ndx != -1);
  v = indexer_get(&indexer, ndx);
  CU_ASSERT(v == 50);

  ndx = indexer_find_equal_or_bigger(&indexer, 15);
  CU_ASSERT(ndx != -1);
  while(ndx != -1)
  {
    v = indexer_get(&indexer, ndx);
    printf("V.... %d\n", v);
    ndx = indexer_get_next(&indexer, ndx);
  }
}

void
indexer_add_test(CU_pSuite pSuite)
{
  CU_add_test(pSuite, "indexer_test", test_indexer);
}
