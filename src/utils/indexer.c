#include <stdlib.h>
#include "common_def.h"
#include "indexer.h"

int
__compare_index(const void* a, const void* b)
{
  uint32_t    *_a, *_b;

  _a = (uint32_t*)a;
  _b = (uint32_t*)b;

  return (int)(*_a - *_b);
}

void
indexer_init(indexer_t* indexer, int size)
{
  indexer->ndx_array  = malloc(sizeof(uint32_t)*size);
  indexer->size       = size;
}

void
indexer_set(indexer_t* indexer, int ndx, uint32_t val)
{
  indexer->ndx_array[ndx] = val;
}

uint32_t
indexer_get(indexer_t* indexer, int ndx)
{
  return indexer->ndx_array[ndx];
}

void
indexer_build(indexer_t* indexer)
{
  qsort(indexer->ndx_array, indexer->size, sizeof(uint32_t), __compare_index);
}

int
indexer_find_equal_or_bigger(indexer_t* indexer, uint32_t val)
{
  int   low = 0,
        high = indexer->size,
        mid = -1;
  bool  flag = FALSE;

  while(low < high)
  {
    mid = (low + high) / 2;
    if(indexer->ndx_array[mid] == val)
    {
      flag = TRUE;
      break;
    }
    else if(indexer->ndx_array[mid ]< val)
      low = mid + 1;
    else
      high = mid;
  }

  if(flag)
  {
    return mid;
  }
  else
  {
    if(low >= indexer->size)
    {
      return -1;
    }
    else
    {
      return low;
      //high will give next smaller
    }
  }
}

int
indexer_get_next(indexer_t* indexer, int current)
{
  current++;
  if(current >= indexer->size)
  {
    return -1;
  }
  return current;
}
