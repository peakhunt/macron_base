#include "trace.h"
#include "logger_sql3_common.h"

sqlite3*
logger_db_open(const char* path)
{
  int         ret;
  sqlite3*    sq3;

  ret = sqlite3_open_v2(path, &sq3, SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX, NULL);
  if(ret != SQLITE_OK)
  {
    return NULL;
  }
  
  sqlite3_busy_timeout(sq3, 5000);    // to deal with "database is locked"

#if 0
  //
  // set incremental vacuum
  //
  if(sqlite3_exec(sq3, "PRAGMA auto_vacuum = 1", NULL, NULL, NULL) != SQLITE_OK)
  {
    TRACE(DEBUG, "failed to set auto_vacuum\n");
  }
#else
  sqlite3_exec(sq3, "pragma cache_size=100", 0, 0, 0);
  sqlite3_exec(sq3, "pragma journal_mode = WAL", 0, 0, 0);
  sqlite3_exec(sq3, "pragma journal_size_limit = 40960", 0, 0, 0);
#endif

  return sq3;
}

void
logger_db_close(sqlite3* db)
{
  sqlite3_close(db);
}

bool
logger_db_insert_chnl(sqlite3* db, uint32_t chnl, unsigned long timestamp, double v)
{
  const static char*    sql_cmd_buffer = 
    "insert into channel_log (ch_num, time_stamp, eng_val) values (?1,?2,?3)";
  sqlite3_stmt*         stmt;
  const char*           sql_error;
  int                   ret;

  sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
  ret = sqlite3_prepare_v2(db, sql_cmd_buffer, strlen(sql_cmd_buffer), &stmt, &sql_error);

  if(ret != SQLITE_OK)
  {
    return FALSE;
  }

  sqlite3_bind_int(stmt, 1, chnl);
  sqlite3_bind_int64(stmt, 2, timestamp);
  sqlite3_bind_double(stmt, 3, v);

  if(sqlite3_step(stmt) != SQLITE_DONE)
  {
    return FALSE;
  }

  if(sqlite3_finalize(stmt) != SQLITE_OK)
  {
    return FALSE;
  }

  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);

  return TRUE;
}

bool
logger_db_insert_alarm(sqlite3* db, uint32_t alarm, unsigned long timestamp, logger_alarm_event_t event)
{
  const static char*    sql_cmd_buffer = 
    "insert into alarm_log (alarm, time_stamp, event) values (?1, ?2, ?3)";
  sqlite3_stmt*         stmt;
  const char*           sql_error;
  int                   ret;

  sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
  ret = sqlite3_prepare_v2(db, sql_cmd_buffer, strlen(sql_cmd_buffer), &stmt, &sql_error);
  if(ret != SQLITE_OK)
  {
    return FALSE;
  }

  sqlite3_bind_int(stmt, 1, alarm);
  sqlite3_bind_int64(stmt, 2, timestamp);
  sqlite3_bind_int(stmt, 3, event);

  if(sqlite3_step(stmt) != SQLITE_DONE)
  {
    return FALSE;
  }

  if(sqlite3_finalize(stmt) != SQLITE_OK)
  {
    return FALSE;
  }

  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
  return TRUE;
}

bool
logger_db_set_trace_channels(sqlite3* db, uint32_t* chnls, uint32_t n_chnls, unsigned long timestamp)
{
  const static char*    sql_cmd_buffer_delete = 
    "delete from channel_trace";
  const static char*    sql_cmd_buffer_insert = 
    "insert into channel_trace (chnl_num, time_started) values (?1, ?2)";
  const static char*    sql_cmd_buffer_clear =
    "delete from channel_log where not exists (select * from channel_trace where channel_trace.chnl_num = channel_log.ch_num);";
  sqlite3_stmt*         stmt;
  const char*           sql_error;
  int                   ret;

  sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
  // step 1. clear channels
  ret = sqlite3_exec(db, sql_cmd_buffer_delete, NULL, NULL, NULL);
  if(ret != SQLITE_OK)
  {
    return FALSE;
  }

  // step 2. add all new channels
  for(uint32_t n = 0; n < n_chnls; n++)
  {
    ret = sqlite3_prepare_v2(db, sql_cmd_buffer_insert, strlen(sql_cmd_buffer_insert), &stmt, &sql_error);
    if(ret != SQLITE_OK)
    {
      return FALSE;
    }

    sqlite3_bind_int(stmt, 1, chnls[n]);
    sqlite3_bind_int64(stmt, 2, timestamp);

    if(sqlite3_step(stmt) != SQLITE_DONE)
    {
      return FALSE;
    }

    if(sqlite3_finalize(stmt) != SQLITE_OK)
    {
      return FALSE;
    }
  }

  // step 3. delete all channel_logs that are not traced
  ret = sqlite3_exec(db, sql_cmd_buffer_clear, NULL, NULL, NULL);
  if(ret != SQLITE_OK)
  {
    return FALSE;
  }

  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
  return TRUE;
}

extern bool
logger_db_get_trace_channels(sqlite3* db, uint32_t** chnls, uint32_t* n_chnls)
{
  const static char*    sql_cmd_buffer_count = 
    "select count(*) from channel_trace";
  const static char*    sql_cmd_buffer_select =
    "select chnl_num from channel_trace";
  sqlite3_stmt*         stmt;
  const char*           sql_error;
  int                   ret;
  uint32_t              *tmp = NULL,
                        num_chnls;

  sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);    // for database lock. I believe this works ;)

  ret = sqlite3_prepare_v2(db, sql_cmd_buffer_count, strlen(sql_cmd_buffer_count), &stmt, &sql_error);
  if(ret != SQLITE_OK)
  {
    return FALSE;
  }

  ret = sqlite3_step(stmt);

  // gotta be only one row. one column
  if(ret != SQLITE_ROW)
  {
    return FALSE;
  }

  num_chnls = sqlite3_column_int(stmt, 0);

  sqlite3_finalize(stmt);

  ret = sqlite3_prepare_v2(db, sql_cmd_buffer_select, strlen(sql_cmd_buffer_select), &stmt, &sql_error);
  if(ret != SQLITE_OK)
  {
    return FALSE;
  }

  if(num_chnls > 0)
  {
    tmp = malloc(sizeof(uint32_t) * num_chnls);
    if(tmp == NULL)
    {
      return FALSE;
    }
  }

  for(uint32_t i = 0; i < num_chnls; i++)
  {
    sqlite3_step(stmt);
    tmp[i] = (uint32_t)sqlite3_column_int(stmt, 0);
  }

  sqlite3_finalize(stmt);
  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);


  *chnls = tmp;
  *n_chnls = num_chnls;

  return TRUE;
}

bool
logger_db_get_all_trace_channels(sqlite3* db, sqlite3_callback cb, void* cb_data)
{
  const static char*    sql_cmd_buffer = 
    "select * from channel_trace";
  int                   ret;

  ret = sqlite3_exec(db, sql_cmd_buffer, cb, cb_data, NULL);
  if(ret != SQLITE_OK)
  {
    return FALSE;
  }
  return TRUE;
}

static void
__create_query_for_get_channel_log(char* buf, size_t buf_size, unsigned long start, unsigned long end, uint32_t* chnls, uint32_t num_chnls)
{
  /*
     select * from channel_log
     where ((ch_num = 1 or ch_num = 2) and (time_stamp >= 1822310417 and time_stamp <= 1822312615))
     order by ch_num asc, time_stamp asc;
   */

  int         len = 0;
  uint32_t    ndx;

  len += snprintf(&buf[len], buf_size - len, "select * from channel_log ");
  if(len >= buf_size) return;

  len += snprintf(&buf[len], buf_size - len, "where ((");
  if(len >= buf_size) return;

  for(ndx = 0; ndx < num_chnls; ndx++)
  { 
    len += snprintf(&buf[len], buf_size - len,  " ch_num = %d ", chnls[ndx]);
    if(len >= buf_size) return;

    if(ndx < (num_chnls - 1))
    {
      len += snprintf(&buf[len], buf_size - len, " or ");
      if(len >= buf_size) return;
    }
  }
  len += snprintf(&buf[len], buf_size - len, " ) and ");
  if(len >= buf_size) return;

  len += snprintf(&buf[len], buf_size - len, "(time_stamp >= %lu and time_stamp <= %lu)) ", start, end);
  if(len >= buf_size) return;

  // len += snprintf(&buf[len], buf_size - len, "order by ch_num asc, time_stamp asc");
  // much easier this way on client side
  len += snprintf(&buf[len], buf_size - len, "order by time_stamp asc");
}

bool
logger_db_get_channel_log(sqlite3* db, unsigned long start, unsigned long end, uint32_t* chnls, uint32_t num_chnls,
    sqlite3_callback cb, void* cb_data)
{
  char sql_cmd_buffer[512];
  int  ret;

  __create_query_for_get_channel_log(sql_cmd_buffer, 512, start, end, chnls, num_chnls);

  TRACE(DEBUG, "query: %s\n", sql_cmd_buffer);

  ret = sqlite3_exec(db, sql_cmd_buffer, cb, cb_data, NULL);

  if(ret != SQLITE_OK)
  {
    TRACE(DEBUG, "query failed: %s\n", sqlite3_errmsg(db));
    return FALSE;
  }

  return TRUE;
}

bool
logger_db_get_alarm_log(sqlite3* db, unsigned long start_time, unsigned long end_time, sqlite3_callback cb, void* cb_data)
{
  char sql_cmd_buffer[512];
  int  ret;

  snprintf(sql_cmd_buffer, 512, "select * from alarm_log where time_stamp >= %lu and time_stamp < %lu "
      "order by time_stamp asc",
      start_time, end_time);

  TRACE(DEBUG, "query: %s\n", sql_cmd_buffer);

  ret = sqlite3_exec(db, sql_cmd_buffer, cb, cb_data, NULL);

  if(ret != SQLITE_OK)
  {
    TRACE(DEBUG, "query failed: %s\n", sqlite3_errmsg(db));
    return FALSE;
  }

  return TRUE;
}

bool
logger_db_clean_up_old_channel_logs(sqlite3* db, unsigned long older_than)
{
  char sql_cmd_buffer[512];
  int  ret;

  snprintf(sql_cmd_buffer, 512, "delete from channel_log where time_stamp <= %lu", older_than);

  sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);

  ret = sqlite3_exec(db, sql_cmd_buffer, NULL, NULL, NULL);

  if(ret != SQLITE_OK)
  {
    TRACE(DEBUG, "cmd failed: %s\n", sqlite3_errmsg(db));
    return FALSE;
  }

  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
  return TRUE;
}

bool
logger_db_clean_up_old_alarm_logs(sqlite3* db, unsigned long older_than)
{
  char sql_cmd_buffer[512];
  int  ret;

  snprintf(sql_cmd_buffer, 512, "delete from alarm_log where time_stamp <= %lu", older_than);

  sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL); 

  ret = sqlite3_exec(db, sql_cmd_buffer, NULL, NULL, NULL);

  if(ret != SQLITE_OK)
  {
    TRACE(DEBUG, "cmd failed: %s\n", sqlite3_errmsg(db));
    return FALSE;
  }

  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
  return TRUE;
}

bool
logger_db_clean_up_old_channel_alarm_logs(sqlite3* db, unsigned long channel_older_than, unsigned long alarm_older_than)
{
  char sql_cmd_buffer[512];
  int  ret;

  snprintf(sql_cmd_buffer, 512, "delete from channel_log where time_stamp <= %lu", channel_older_than);

  sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);

  ret = sqlite3_exec(db, sql_cmd_buffer, NULL, NULL, NULL);

  if(ret != SQLITE_OK)
  {
    TRACE(DEBUG, "cmd failed: %s\n", sqlite3_errmsg(db));
    return FALSE;
  }

  snprintf(sql_cmd_buffer, 512, "delete from alarm_log where time_stamp <= %lu", alarm_older_than);
  ret = sqlite3_exec(db, sql_cmd_buffer, NULL, NULL, NULL);

  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
  if(ret != SQLITE_OK)
  {
    TRACE(DEBUG, "cmd failed: %s\n", sqlite3_errmsg(db));
    return FALSE;
  }

  return TRUE;
}

bool
logger_db_clean_up_old_channel_alarm_logs2(sqlite3* db, uint32_t* chnls, uint32_t num_chnl,
    unsigned long channel_older_than, unsigned long alarm_older_than)
{
  char sql_cmd_buffer[512];
  int  ret;

  sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);

  for(uint32_t ndx = 0; ndx < num_chnl; ndx++)
  {
    snprintf(sql_cmd_buffer, 512, "delete from channel_log where time_stamp <= %lu and ch_num = %u", channel_older_than,
        chnls[ndx]);
    TRACE(DEBUG, "%s\n", sql_cmd_buffer);

    sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);

    ret = sqlite3_exec(db, sql_cmd_buffer, NULL, NULL, NULL);

    if(ret != SQLITE_OK)
    {
      TRACE(DEBUG, "cmd failed: %s\n", sqlite3_errmsg(db));
      return FALSE;
    }
  }

  snprintf(sql_cmd_buffer, 512, "delete from alarm_log where time_stamp <= %lu", alarm_older_than);
  ret = sqlite3_exec(db, sql_cmd_buffer, NULL, NULL, NULL);

  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
  if(ret != SQLITE_OK)
  {
    TRACE(DEBUG, "cmd failed: %s\n", sqlite3_errmsg(db));
    return FALSE;
  }

  return TRUE;
}
