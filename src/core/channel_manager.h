#ifndef __CHANNEL_MANAGER_DEF_H__
#define __CHANNEL_MANAGER_DEF_H__

#include "channel.h"

extern void channel_manager_init(void);
extern void channel_manager_add_channel(channel_t* chnl);

extern uint32_t channel_manager_get_raw_value(uint32_t chnl_num);
extern void channel_manager_set_raw_value(uint32_t chnl_num, uint32_t v);

extern channel_eng_value_t channel_manager_get_eng_value(uint32_t chnl_num);
extern void channel_manager_set_eng_value(uint32_t chnl_num, channel_eng_value_t v);

extern void channel_manager_update_input(void);
extern void channel_manager_update_output(void);

#endif /* !__CHANNEL_MANAGER_DEF_H__ */
