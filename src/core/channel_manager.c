#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "list.h"
#include "bhash.h"
#include "channel_manager.h"
#include "time_util.h"
#include "trace.h"
#include "logger.h"

#define CHANNEL_MANAGER_NUM_BUCKETS     1024

////////////////////////////////////////////////////////////////////////////////
//
// module privates: this module is supposed to be a singleton
//
////////////////////////////////////////////////////////////////////////////////
static LIST_HEAD_DECL(_channels);
static LIST_HEAD_DECL(_in_channels);
static LIST_HEAD_DECL(_out_channels);
static LIST_HEAD_DECL(_vir_channels);

static BHashContext   _channel_hash_by_chnl_num;

static uint32_t       _num_channels;
static uint32_t       _num_in_channels;
static uint32_t       _num_out_channels;
static uint32_t       _num_vir_channels;

static pthread_mutex_t    _chnl_mgr_lock;
static pthread_mutex_t    _chnl_mgr_trace_lock;

static uint32_t*          _trace_channels = NULL;
static uint32_t           _num_trace_channels = 0;

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
  pthread_mutex_init(&_chnl_mgr_trace_lock, NULL);
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

  TRACE(CHANNELM, "adding channel %d, %d\n", chnl->chnl_num, chnl->chnl_dir);

  switch(chnl->chnl_dir)
  {
  case channel_direction_in:
    TRACE(CHANNELM, "adding in channel %d\n", chnl->chnl_num);
    list_add_tail(&chnl->le_by_dir, &_in_channels);
    _num_in_channels++;
    break;

  case channel_direction_out:
    TRACE(CHANNELM, "adding out channel %d\n", chnl->chnl_num);
    list_add_tail(&chnl->le_by_dir, &_out_channels);
    _num_out_channels++;
    break;

  case channel_direction_virtual:
    TRACE(CHANNELM, "adding virtual channel %d\n", chnl->chnl_num);
    list_add_tail(&chnl->le_by_dir, &_vir_channels);
    _num_vir_channels++;
    break;
  }

  pthread_mutex_unlock(&_chnl_mgr_lock);
}

void
channel_manager_init_channels(void)
{
  channel_t*    chnl;

  list_for_each_entry(chnl, &_channels, le)
  {
    channel_set_eng_value(chnl, chnl->init_value);
  }
}

uint32_t
channel_manager_get_raw_value(uint32_t chnl_num)
{
  channel_t*  chnl;
  uint32_t    v;

  chnl = channel_manager_chnl_get(chnl_num);
  if(chnl == NULL)
  {
    TRACE(CHANNELM, "%s can't find channel %d\n", __func__, chnl_num);
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
    TRACE(CHANNELM, "%s can't find channel %d\n", __func__, chnl_num);
    return;
  }

  chnl->raw_value_queued    = v;
  chnl->raw_value_avail     = TRUE;

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
    TRACE(CHANNELM, "%s can't find channel %d\n", __func__, chnl_num);
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
  channel_eng_value_t   eng_v;

  chnl = channel_manager_chnl_get(chnl_num);
  if(chnl == NULL)
  {
    TRACE(CHANNELM, "%s can't find channel %d\n", __func__, chnl_num);
    return;
  }

  channel_set_eng_value(chnl, v);

  eng_v = chnl->eng_value;

  channel_manager_chnl_put(chnl);

  publisher_exec_notify(&chnl->chnl_update, (void*)&eng_v);
}

void
channel_manager_update_input(void)
{
  channel_t*    chnl;
  channel_eng_value_t   eng_v;

  TRACE(CHANNELM, "updating input\n");

  list_for_each_entry(chnl, &_in_channels, le_by_dir)
  {
    pthread_mutex_lock(&_chnl_mgr_lock);

    TRACE(CHANNELM, "updating input for channel %d\n", chnl->chnl_num);
    if(chnl->raw_value_avail == TRUE)
    {
      chnl->raw_value = chnl->raw_value_queued;

      channel_update_input(chnl);
      chnl->raw_value_avail = FALSE;
    }

    eng_v = chnl->eng_value;

    publisher_exec_notify(&chnl->chnl_update, (void*)&eng_v);

    pthread_mutex_unlock(&_chnl_mgr_lock);
  }
}

void
channel_manager_update_output(void)
{
  channel_t*    chnl;
  channel_eng_value_t   eng_v;

  TRACE(CHANNELM, "updating output\n");
  list_for_each_entry(chnl, &_out_channels, le_by_dir)
  {
    pthread_mutex_lock(&_chnl_mgr_lock);
    TRACE(CHANNELM, "updating output for channel %d\n", chnl->chnl_num);

    channel_update_output(chnl);

    chnl->raw_value_queued = chnl->raw_value;
    eng_v = chnl->eng_value;

    publisher_exec_notify(&chnl->chnl_update, (void*)&eng_v);

    pthread_mutex_unlock(&_chnl_mgr_lock);
  }
}

bool
channel_manager_update_lookup_table(uint32_t chnl_num, lookup_table_t* lookup_table)
{
  channel_t* chnl;

  chnl = channel_manager_chnl_get(chnl_num);
  if(chnl == NULL)
  {
    TRACE(CHANNELM, "%s can't find channel %d\n", __func__, chnl_num);
    return FALSE;
  }

  channel_set_lookup_table(chnl, lookup_table);

  channel_manager_chnl_put(chnl);
  return TRUE;
}

void
channel_manager_set_sensor_fault_status(uint32_t chnl_num, bool status)
{
  channel_t* chnl;

  chnl = channel_manager_chnl_get(chnl_num);
  if(chnl == NULL)
  {
    TRACE(CHANNELM, "%s can't find channel %d\n", __func__, chnl_num);
    return;
  }

  //
  // FIXME add sensor fault handling logic here
  //
  chnl->sensor_fault  = status;

  channel_manager_chnl_put(chnl);
}

bool
channel_manager_get_sensor_fault_status(uint32_t chnl_num)
{
  channel_t*  chnl;
  bool        ret;

  chnl = channel_manager_chnl_get(chnl_num);
  if(chnl == NULL)
  {
    TRACE(CHANNELM, "%s can't find channel %d\n", __func__, chnl_num);
    return FALSE;
  }

  ret = chnl->sensor_fault;

  channel_manager_chnl_put(chnl);

  return ret;
}

void
channel_manager_add_observer(uint32_t chnl_num, observer_t* obs)
{
  channel_t* chnl;

  chnl = channel_manager_chnl_get(chnl_num);
  if(chnl == NULL)
  {
    TRACE(CHANNELM, "%s can't find channel %d\n", __func__, chnl_num);
    return;
  }

  channel_add_observer(chnl, obs);

  channel_manager_chnl_put(chnl);
}

int
channel_manager_get_channel_stat(uint32_t chnl_num, channel_status_t* status)
{
  channel_t* chnl;

  chnl = channel_manager_chnl_get(chnl_num);
  if(chnl == NULL)
  {
    return -1;
  }

  status->chnl_type       = chnl->chnl_type;
  status->eng_val         = chnl->eng_value;
  status->raw_sensor_val  = chnl->raw_sensor_value;
  status->raw_val         = chnl->raw_value;
  status->sensor_fault    = chnl->sensor_fault;

  channel_manager_chnl_put(chnl);

  return 0;
}

bool
channel_manager_update_channel_config(uint32_t chnl_num, channel_runtime_config_t* cfg)
{
  channel_t* chnl;

  chnl = channel_manager_chnl_get(chnl_num);
  if(chnl == NULL)
  {
    return FALSE;
  }

  chnl->init_value      = cfg->init_value;
  chnl->failsafe_value  = cfg->failsafe_value;
  chnl->min_val         = cfg->min_val;
  chnl->max_val         = cfg->max_val;

  channel_set_eng_value(chnl, cfg->init_value);

  channel_manager_chnl_put(chnl);

  return TRUE;
}

bool
channel_manager_set_trace_channels(uint32_t* chnls, uint32_t n_chnls)
{
  uint32_t* new_chnls;

  if(n_chnls > 0)
  {
    new_chnls = malloc(sizeof(uint32_t) * n_chnls);
    if(new_chnls == NULL)
    {
      return FALSE;
    }
    memcpy(new_chnls, chnls, sizeof(uint32_t) * n_chnls);
  }
  else
  {
    new_chnls = NULL;
  }

  pthread_mutex_lock(&_chnl_mgr_trace_lock);

  if(_trace_channels != NULL)
  {
    free(_trace_channels);
  }

  _trace_channels = new_chnls;
  _num_trace_channels = n_chnls;

  pthread_mutex_unlock(&_chnl_mgr_trace_lock);

  return TRUE;
}

void
channel_manager_do_log(void)
{
  uint32_t        n;
  channel_t*      chnl;
  unsigned long   timestamp   = time_util_get_current_time_in_ms();

  pthread_mutex_lock(&_chnl_mgr_trace_lock);

  for(n = 0; n < _num_trace_channels; n++)
  {
    chnl = channel_manager_chnl_get(_trace_channels[n]);
    if(chnl == NULL)
    {
      continue;
    }

    if(chnl->chnl_type == channel_type_analog)
    {
      logger_signal_log(chnl->chnl_num, chnl->eng_value.f, timestamp);
    }
    else
    {
      logger_signal_log(chnl->chnl_num, chnl->eng_value.b, timestamp);
    }

    channel_manager_chnl_put(chnl);
  }

  pthread_mutex_unlock(&_chnl_mgr_trace_lock);
}

void
channel_manager_bind_io_vars(io_var_t* io_vars, int num_io_vars)
{
  channel_t*    chnl;

  for(int i = 0; i < num_io_vars; i++)
  {
    chnl = channel_manager_chnl_get(io_vars[i].chnl_num);
    if(chnl == NULL)
    {
      TRACE(CHANNELM, "%s can't find channel %d\n", __func__, io_vars[i].chnl_num);
      continue;
    }

    channel_set_io_var(chnl, &io_vars[i]);

    channel_manager_chnl_put(chnl);
  }
}
