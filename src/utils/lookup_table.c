#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lookup_table.h"
#include "math_util.h"

#define LEFT_NODE(n)      ( 2 * n + 1)
#define RIGHT_NODE(n)     ( 2 * n + 2)

////////////////////////////////////////////////////////////////////////////////
//
// private utilities
//
////////////////////////////////////////////////////////////////////////////////
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

int
__compare_by_v1(const void* a, const void* b)
{
  const lookup_table_item_t   *_a, *_b;

  _a = (const lookup_table_item_t*)a;
  _b = (const lookup_table_item_t*)b;

  return (int)(_a->v1 - _b->v1);
}

int
__compare_by_v2(const void* a, const void* b)
{
  const lookup_table_item_t   *_a, *_b;

  _a = (const lookup_table_item_t*)a;
  _b = (const lookup_table_item_t*)b;

  return (int)(_a->v2 - _b->v2);
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
lookup_table_init(lookup_table_t* table, lookup_table_item_t* items, int size)
{
  table->items            = malloc(sizeof(lookup_table_item_t) * size);
  table->items_reverse    = malloc(sizeof(lookup_table_item_t) * size);
  table->tree_ndx_size    = (size -1)*2 + 2;
  table->tree_ndx         = malloc(sizeof(int) * table->tree_ndx_size);
  table->size             = size;

  memset(table->tree_ndx, -1, sizeof(int)*table->tree_ndx_size);

  memcpy(table->items, items, sizeof(lookup_table_item_t) * size);
  qsort(table->items, table->size, sizeof(lookup_table_item_t), __compare_by_v1);

  memcpy(table->items_reverse, items, sizeof(lookup_table_item_t) * size);
  qsort(table->items_reverse, table->size, sizeof(lookup_table_item_t), __compare_by_v2);

  __build_binary_tree(table, 0, table->size - 1, 0);
}

void
lookup_table_deinit(lookup_table_t* table)
{
  free(table->items);
  free(table->items_reverse);
  free(table->tree_ndx);
}

double
lookup_table_interpolate(lookup_table_t* table, double v)
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

  return linear_interpolate_double(
              first->v1, first->v2,
              second->v1, second->v2,
              v);
}

double
lookup_table_interpolate_reverse(lookup_table_t* table, double v)
{
  int   ndx, adj, node;
  lookup_table_item_t *item, *first, *second;
  int   search_cnt = 0;

  node = 0;
  ndx = table->tree_ndx[node];
  item = &table->items_reverse[ndx];

  while(1)
  {
    search_cnt++;
    if(v < item->v2)
      node = LEFT_NODE(node);
    else
      node = RIGHT_NODE(node);

    if(node < table->tree_ndx_size && table->tree_ndx[node] != -1)
    {
      ndx = table->tree_ndx[node];
      item = &table->items_reverse[ndx];
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


  if(v < item->v2)
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

  first   = ndx < adj ? &table->items_reverse[ndx] : &table->items_reverse[adj];
  second  = ndx < adj ? &table->items_reverse[adj] : &table->items_reverse[ndx];

  return linear_interpolate_double(
              first->v2, first->v1,
              second->v2, second->v1,
              v);
}
