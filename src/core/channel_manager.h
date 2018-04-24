#ifndef __CHANNEL_MANAGER_DEF_H__
#define __CHANNEL_MANAGER_DEF_H__

#include "channel.h"
#include "lookup_table.h"

typedef struct
{
  channel_type_t          chnl_type;
  channel_eng_value_t     eng_val;
  uint32_t                raw_val;
  bool                    sensor_fault;
} channel_status_t;

typedef struct
{
  channel_eng_value_t init_value;
  channel_eng_value_t failsafe_value;
  double              min_val;
  double              max_val;
} channel_runtime_config_t;

extern void channel_manager_init(void);
extern void channel_manager_add_channel(channel_t* chnl);
extern void channel_manager_init_channels(void);

extern uint32_t channel_manager_get_raw_value(uint32_t chnl_num);
extern void channel_manager_set_raw_value(uint32_t chnl_num, uint32_t v);

extern channel_eng_value_t channel_manager_get_eng_value(uint32_t chnl_num);
extern void channel_manager_set_eng_value(uint32_t chnl_num, channel_eng_value_t v);

extern void channel_manager_update_input(void);
extern void channel_manager_update_output(void);

extern bool channel_manager_update_lookup_table(uint32_t chnl_num, lookup_table_t* lookup_table);
extern void channel_manager_add_observer(uint32_t chnl_num, observer_t* obs);

extern void channel_manager_set_sensor_fault_status(uint32_t chnl_num, bool status);
extern bool channel_manager_get_sensor_fault_status(uint32_t chnl_num);

extern int channel_manager_get_channel_stat(uint32_t chnl_num, channel_status_t* status);

extern bool channel_manager_update_channel_config(uint32_t chnl_num, channel_runtime_config_t* cfg);

#endif /* !__CHANNEL_MANAGER_DEF_H__ */
