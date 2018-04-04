#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lookup_table.h"

#define LEFT_NODE(n)      ( 2 * n + 1)
#define RIGHT_NODE(n)     ( 2 * n + 2)

static void
__build_binary_tree(lookup_table_t* table, int start, int end, int current)
{
  int   mid;

  mid = start + (end - start)/2;

  if(start > end)
  {
    return;
  }

  table->tree_ndx[current] = mid;

  __build_binary_tree(table, start, mid - 1, LEFT_NODE(current));
  __build_binary_tree(table, mid + 1, end, RIGHT_NODE(current));
}

void
lookup_table_init(lookup_table_t* table, int size)
{
  table->items          = malloc(sizeof(lookup_table_item_t) * size);
  table->tree_ndx_size  = (size -1)*2 + 2;
  table->tree_ndx       = malloc(sizeof(int) * table->tree_ndx_size);
  table->size           = size;

  memset(table->tree_ndx, -1, sizeof(int)*table->tree_ndx_size);
}

void
lookup_table_add(lookup_table_t* table, float v1, float v2, int ndx)
{
  table->items[ndx].v1    = v1;
  table->items[ndx].v2    = v2;
}

void
lookup_table_build_btree(lookup_table_t* table)
{
  __build_binary_tree(table, 0, table->size - 1, 0);
}

void
lookup_table_deinit(lookup_table_t* table)
{
  free(table->items);
  free(table->tree_ndx);
}

float
lookup_table_interpolate(lookup_table_t* table, float v)
{
  int   ndx, adj, node;
  lookup_table_item_t *item, *first, *second;
  int   search_cnt = 0;

  node = 0;
  ndx = table->tree_ndx[node];
  item = &table->items[ndx];

  while(1)
  {
    search_cnt++;
    if(v < item->v1)
      node = LEFT_NODE(node);
    else
      node = RIGHT_NODE(node);

    if(node < table->tree_ndx_size && table->tree_ndx[node] != -1)
    {
      ndx = table->tree_ndx[node];
      item = &table->items[ndx];
    }
    else
    {
      break;
    }
  }

  if(ndx == table->tree_ndx[0])
  {
    // abnormal case
    // only root node in the tree
    return 0.0f;
  }


  if(v < item->v1)
  {
    adj = ndx - 1;
    if(adj < 0)
      adj = ndx + 1;
  }
  else
  {
    adj = ndx + 1;
    if(adj >= (table->size - 1))
    {
      adj = ndx - 1;
    }
  }

  first   = ndx < adj ? &table->items[ndx] : &table->items[adj];
  second  = ndx < adj ? &table->items[adj] : &table->items[ndx];

  // FIXME
  (void)first;
  (void)second;
  return 0.0f;
}

#if 0
int
main()
{
//#define SIZE      500000
#define SIZE      32

  lookup_table_t      table;

  lookup_table_init(&table, SIZE);

  for(int i = 0; i < SIZE; i++)
  {
    lookup_table_add(&table, i, i * 100, i);
  }

  lookup_table_build_btree(&table);

  lookup_table_interpolate(&table, 5.5);
  lookup_table_interpolate(&table, 0.5);
  lookup_table_interpolate(&table, 9.5);
  lookup_table_interpolate(&table, 7.5);
  lookup_table_interpolate(&table, SIZE - 1);
  lookup_table_interpolate(&table, SIZE);
  lookup_table_interpolate(&table, -1);

  lookup_table_deinit(&table);
}
#endif
