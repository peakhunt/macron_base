#ifndef __CHANNEL_DEF_H__
#define __CHANNEL_DEF_H__

#include "common_def.h"
#include "list.h"
#include "bhash.h"

typedef enum
{
  channel_type_digital,
  channel_type_analog,
} channel_type_t;

typedef enum
{
  channel_direction_in,
  channel_direction_out,
  channel_direction_virtual,
} channel_direction_t;

typedef union
{
  float   f;
  bool    b;
} channel_eng_value_t;

typedef struct
{
  struct list_head    le;
  struct list_head    le_by_dir;
  BHashElement        hash_by_chnl_num;

  uint32_t            chnl_num;
  channel_type_t      chnl_type;
  channel_direction_t chnl_dir;

  channel_eng_value_t eng_value;
  uint32_t            raw_value;
  uint32_t            raw_value_queued;
  /* FIXME lookup table for analog channel */
} channel_t;

extern channel_t* channel_alloc(uint16_t chnl_num, channel_type_t chnl_type, channel_direction_t chnl_dir);
extern void channel_update_raw_value(channel_t* chnl);
extern void channel_update_eng_value(channel_t* chnl);


#endif /* !__CHANNEL_DEF_H__ */
