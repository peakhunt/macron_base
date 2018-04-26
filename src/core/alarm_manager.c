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
static LIST_HEAD_DECL(_alarms);

static BHashContext   _alarm_hash_by_alarm_num;

static uint32_t       _num_alarms;

static pthread_mutex_t    _alarm_mgr_lock;

////////////////////////////////////////////////////////////////////////////////
//
// utilities
//
////////////////////////////////////////////////////////////////////////////////
static inline alarm_t*
alarm_manager_alarm_get(uint32_t alarm_num)
{
  alarm_t* alarm;

  pthread_mutex_lock(&_alarm_mgr_lock);

  alarm = bhash_lookup(&_alarm_hash_by_alarm_num, (void*)&alarm_num);
  if(alarm == NULL)
  {
    pthread_mutex_unlock(&_alarm_mgr_lock);
  }

  return alarm;
}

static inline void
alarm_manager_alarm_put(alarm_t* alarm)
{
  pthread_mutex_unlock(&_alarm_mgr_lock);
}

////////////////////////////////////////////////////////////////////////////////
//
// channel listener
//
////////////////////////////////////////////////////////////////////////////////
static void
__on_channel_update(observer_t* obs, void* arg)
{
  alarm_t*    alarm = container_of(obs, alarm_t, chnl_obs);
  channel_eng_value_t*    ev = (channel_eng_value_t*)arg;

  pthread_mutex_lock(&_alarm_mgr_lock);

  alarm_update(alarm, *ev);

  pthread_mutex_unlock(&_alarm_mgr_lock);
}

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

  alarm->chnl_obs.notify = __on_channel_update;
  channel_manager_add_observer(alarm->chnl_num, &alarm->chnl_obs);

  pthread_mutex_unlock(&_alarm_mgr_lock);
}

void
alarm_manager_update(void)
{
}

bool
alarm_manager_ack_alarm(uint32_t alarm_num)
{
  alarm_t*    alarm;

  alarm = alarm_manager_alarm_get(alarm_num);
  if(alarm == NULL)
  {
    return FALSE;
  }

  alarm_ack(alarm);

  alarm_manager_alarm_put(alarm);
  return TRUE;
}

int
alarm_manager_get_alarm_status(uint32_t alarm_num, alarm_status_t* status)
{
  alarm_t*    alarm;

  alarm = alarm_manager_alarm_get(alarm_num);
  if(alarm == NULL)
  {
    return -1;
  }

  status->state       = alarm->state;
  status->occur_time  = alarm->occur_time;

  alarm_manager_alarm_put(alarm);

  return 0;
}

bool
alarm_manager_update_alarm_config(uint32_t alarm_num, alarm_runtime_config_t* cfg)
{
  alarm_t*    alarm;

  alarm = alarm_manager_alarm_get(alarm_num);
  if(alarm == NULL)
  {
    return FALSE;
  }

  alarm->delay      = cfg->delay;
  alarm->set_point  = cfg->set_point;

  alarm_manager_alarm_put(alarm);

  return TRUE;
}
