#ifndef __CHANNEL_MANAGER_DEF_H__
#define __CHANNEL_MANAGER_DEF_H__

#include "channel.h"
#include "lookup_table.h"

extern void channel_manager_init(void);
extern void channel_manager_add_channel(channel_t* chnl);

extern uint32_t channel_manager_get_raw_value(uint32_t chnl_num);
extern void channel_manager_set_raw_value(uint32_t chnl_num, uint32_t v);

extern channel_eng_value_t channel_manager_get_eng_value(uint32_t chnl_num);
extern void channel_manager_set_eng_value(uint32_t chnl_num, channel_eng_value_t v);

extern void channel_manager_update_input(void);
extern void channel_manager_update_output(void);

extern void channel_manager_update_lookup_table(uint32_t chnl_num, lookup_table_t* lookup_table);
extern void channel_manager_add_observer(uint32_t chnl_num, observer_t* obs);

#endif /* !__CHANNEL_MANAGER_DEF_H__ */
