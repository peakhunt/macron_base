#include <stdio.h>
#include <pthread.h>
#include "list.h"
#include "bhash.h"
#include "channel_manager.h"
#include "trace.h"

#define CHANNEL_MANAGER_NUM_BUCKETS     1024

#define MODULE_TRACE      CHANNELM

////////////////////////////////////////////////////////////////////////////////
//
// module privates: this module is supposed to be a singleton
//
////////////////////////////////////////////////////////////////////////////////
static LIST_HEAD(_channels);
static LIST_HEAD(_in_channels);
static LIST_HEAD(_out_channels);
static LIST_HEAD(_vir_channels);

static BHashContext   _channel_hash_by_chnl_num;

static uint32_t       _num_channels;
static uint32_t       _num_in_channels;
static uint32_t       _num_out_channels;
static uint32_t       _num_vir_channels;

static pthread_mutex_t    _chnl_mgr_lock;

////////////////////////////////////////////////////////////////////////////////
//
// utilities
//
////////////////////////////////////////////////////////////////////////////////
static inline channel_t*
channel_manager_chnl_get(uint32_t chnl_num)
{
  channel_t* chnl;

  pthread_mutex_lock(&_chnl_mgr_lock);

  chnl = bhash_lookup(&_channel_hash_by_chnl_num, (void*)&chnl_num);
  if(chnl == NULL)
  {
    pthread_mutex_unlock(&_chnl_mgr_lock);
  }

  return chnl;
}

static inline void
channel_manager_chnl_put(channel_t* chnl)
{
  pthread_mutex_unlock(&_chnl_mgr_lock);
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
channel_manager_init(void)
{
  bhash_init_malloc(&_channel_hash_by_chnl_num,
                    CHANNEL_MANAGER_NUM_BUCKETS,
                    offsetof(channel_t, hash_by_chnl_num),
                    offsetof(channel_t, chnl_num),
                    sizeof(uint32_t),
                    NULL);

  _num_channels       = 0;
  _num_in_channels    = 0;
  _num_out_channels   = 0;
  _num_vir_channels   = 0;

  pthread_mutex_init(&_chnl_mgr_lock, NULL);
}

void
channel_manager_add_channel(channel_t* chnl)
{
  INIT_LIST_HEAD(&chnl->le);
  INIT_LIST_HEAD(&chnl->le_by_dir);

  pthread_mutex_lock(&_chnl_mgr_lock);

  bhash_add(&_channel_hash_by_chnl_num, (void*)chnl);
  list_add_tail(&chnl->le, &_channels);

  _num_channels++;

  TRACE(MODULE_TRACE, "adding channel %d, %d\n", chnl->chnl_num, chnl->chnl_dir);

  switch(chnl->chnl_dir)
  {
  case channel_direction_in:
    TRACE(MODULE_TRACE, "adding in channel %d\n", chnl->chnl_num);
    list_add_tail(&chnl->le_by_dir, &_in_channels);
    _num_in_channels++;
    break;

  case channel_direction_out:
    TRACE(MODULE_TRACE, "adding out channel %d\n", chnl->chnl_num);
    list_add_tail(&chnl->le_by_dir, &_out_channels);
    _num_out_channels++;
    break;

  case channel_direction_virtual:
    TRACE(MODULE_TRACE, "adding virtual channel %d\n", chnl->chnl_num);
    list_add_tail(&chnl->le_by_dir, &_vir_channels);
    _num_vir_channels++;
    break;
  }

  pthread_mutex_unlock(&_chnl_mgr_lock);
}

uint32_t
channel_manager_get_raw_value(uint32_t chnl_num)
{
  channel_t*  chnl;
  uint32_t    v;

  chnl = channel_manager_chnl_get(chnl_num);
  if(chnl == NULL)
  {
    TRACE(MODULE_TRACE, "%s can't find channel %d\n", __func__, chnl_num);
    return 0;
  }

  v = chnl->raw_value_queued;
  channel_manager_chnl_put(chnl);

  return v;
}

void
channel_manager_set_raw_value(uint32_t chnl_num, uint32_t v)
{
  channel_t* chnl;

  chnl = channel_manager_chnl_get(chnl_num);
  if(chnl == NULL)
  {
    TRACE(MODULE_TRACE, "%s can't find channel %d\n", __func__, chnl_num);
    return;
  }

  chnl->raw_value_queued = v;
  channel_manager_chnl_put(chnl);
}

channel_eng_value_t
channel_manager_get_eng_value(uint32_t chnl_num)
{
  channel_t*  chnl;
  channel_eng_value_t v;

  v.f = 0.0f;

  chnl = channel_manager_chnl_get(chnl_num);
  if(chnl == NULL)
  {
    TRACE(MODULE_TRACE, "%s can't find channel %d\n", __func__, chnl_num);
    return v;
  }

  v = chnl->eng_value;
  channel_manager_chnl_put(chnl);

  return v;
}

void
channel_manager_set_eng_value(uint32_t chnl_num, channel_eng_value_t v)
{
  channel_t* chnl;

  chnl = channel_manager_chnl_get(chnl_num);
  if(chnl == NULL)
  {
    TRACE(MODULE_TRACE, "%s can't find channel %d\n", __func__, chnl_num);
    return;
  }

  chnl->eng_value = v;

  channel_update_raw_value(chnl);

  channel_manager_chnl_put(chnl);
}

void
channel_manager_update_input(void)
{
  channel_t*    chnl;

  TRACE(MODULE_TRACE, "updating input\n");

  list_for_each_entry(chnl, &_in_channels, le_by_dir)
  {
    pthread_mutex_lock(&_chnl_mgr_lock);

    TRACE(MODULE_TRACE, "updating input for channel %d\n", chnl->chnl_num);

    chnl->raw_value = chnl->raw_value_queued;

    channel_update_eng_value(chnl);

    pthread_mutex_unlock(&_chnl_mgr_lock);

  }
}

void
channel_manager_update_output(void)
{
  channel_t*    chnl;

  TRACE(MODULE_TRACE, "updating output\n");
  list_for_each_entry(chnl, &_out_channels, le_by_dir)
  {
    pthread_mutex_unlock(&_chnl_mgr_lock);
    TRACE(MODULE_TRACE, "updating output for channel %d\n", chnl->chnl_num);

    chnl->raw_value_queued = chnl->raw_value;

    pthread_mutex_unlock(&_chnl_mgr_lock);
  }
}

void
channel_manager_update_lookup_table(uint32_t chnl_num, lookup_table_t* lookup_table)
{
  channel_t* chnl;

  chnl = channel_manager_chnl_get(chnl_num);
  if(chnl == NULL)
  {
    TRACE(MODULE_TRACE, "%s can't find channel %d\n", __func__, chnl_num);
    return;
  }

  channel_set_lookup_table(chnl, lookup_table);

  channel_manager_chnl_put(chnl);
}
