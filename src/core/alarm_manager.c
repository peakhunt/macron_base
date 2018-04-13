#include <stdio.h>
#include <pthread.h>
#include "list.h"
#include "bhash.h"
#include "alarm_manager.h"
#include "channel_manager.h"
#include "trace.h"

#define ALARM_MANAGER_NUM_BUCKETS     1024

////////////////////////////////////////////////////////////////////////////////
//
// module privates: this module is supposed to be a singleton
//
////////////////////////////////////////////////////////////////////////////////
static LIST_HEAD(_alarms);

static BHashContext   _alarm_hash_by_alarm_num;

static uint32_t       _num_alarms;

static pthread_mutex_t    _alarm_mgr_lock;

////////////////////////////////////////////////////////////////////////////////
//
// utilities
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
alarm_manager_init(void)
{
  bhash_init_malloc(&_alarm_hash_by_alarm_num,
                    ALARM_MANAGER_NUM_BUCKETS,
                    offsetof(alarm_t, hash_by_alarm_num),
                    offsetof(alarm_t, alarm_num),
                    sizeof(uint32_t),
                    NULL);

  _num_alarms         = 0;
  pthread_mutex_init(&_alarm_mgr_lock, NULL);
}

void
alarm_manager_add_alarm(alarm_t* alarm)
{
  INIT_LIST_HEAD(&alarm->le);
  INIT_LIST_HEAD(&alarm->le_for_active);

  pthread_mutex_lock(&_alarm_mgr_lock);

  bhash_add(&_alarm_hash_by_alarm_num, (void*)alarm);
  list_add_tail(&alarm->le, &_alarms);

  _num_alarms++;

  channel_manager_add_observer(alarm->chnl_num, &alarm->chnl_obs);
  pthread_mutex_unlock(&_alarm_mgr_lock);
}

void
alarm_manager_update(void)
{
}
