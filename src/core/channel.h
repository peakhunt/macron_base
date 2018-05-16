#ifndef __CHANNEL_DEF_H__
#define __CHANNEL_DEF_H__

#include "common_def.h"
#include "list.h"
#include "bhash.h"
#include "lookup_table.h"
#include "publish_observe.h"

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
  double  f;
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
  channel_eng_value_t init_value;
  channel_eng_value_t failsafe_value;

  double              min_val;
  double              max_val;

  bool                sensor_fault;

  uint32_t            raw_value;
  uint32_t            raw_value_queued;     /* for input input buffer, for output output buffer */
  bool                raw_value_avail;
  lookup_table_t*     lookup_table;

  publisher_t         chnl_update;
  bool                trace;
} channel_t;

extern channel_t* channel_alloc(uint32_t chnl_num, channel_type_t chnl_type, channel_direction_t chnl_dir);
extern channel_t* channel_alloc_digital(uint32_t chnl_num, channel_direction_t chnl_dir);
extern channel_t* channel_alloc_analog(uint32_t chnl_num, channel_direction_t chnl_dir, lookup_table_t* lookup_table);

extern void channel_set_lookup_table(channel_t* chnl, lookup_table_t* lookup_table);
extern void channel_update_raw_value(channel_t* chnl);
extern void channel_update_eng_value(channel_t* chnl);
extern void channel_add_observer(channel_t* chnl, observer_t* obs);

#endif /* !__CHANNEL_DEF_H__ */
