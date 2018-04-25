#ifndef __INDEXER_DEF_H__
#define __INDEXER_DEF_H__

typedef struct
{
  uint32_t*     ndx_array;
  int           size;
} indexer_t;

extern void indexer_init(indexer_t* indexer, int size);
extern void indexer_set(indexer_t* indexer, int ndx, uint32_t val);
extern uint32_t indexer_get(indexer_t* indexer, int ndx);
extern void indexer_build(indexer_t* indexer);
extern int indexer_find_equal_or_bigger(indexer_t* indexer, uint32_t val);
extern int indexer_get_next(indexer_t* indexer, int current);

#endif /* !__INDEXER_DEF_H__ */
