#ifndef __LOOKUP_TABLE_DEF_H__
#define __LOOKUP_TABLE_DEF_H__

typedef struct 
{
  float     v1;
  float     v2;
} lookup_table_item_t;

typedef struct
{
  lookup_table_item_t*        items;          // sorted
  int*                        tree_ndx;
  int                         size;
  int                         tree_ndx_size;
} lookup_table_t;

extern void lookup_table_init(lookup_table_t* table, int size);
extern void lookup_table_add(lookup_table_t* table, float v1, float v2, int ndx);
extern void lookup_table_build_btree(lookup_table_t* table);
extern void lookup_table_deinit(lookup_table_t* table);
extern float lookup_table_interpolate(lookup_table_t* table, float v);

#endif /* !__LOOKUP_TABLE_DEF_H__ */
