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
  sqlite3_bind_int(stmt, 2, timestamp);
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
  sqlite3_bind_int(stmt, 2, timestamp);
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
logger_db_insert_trace_channel(sqlite3* db, uint32_t chnl, unsigned long timestamp)
{
  const static char*    sql_cmd_buffer = 
    "insert into channel_trace (chnl_num, time_started) values (?1, ?2)";
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
  sqlite3_bind_int(stmt, 2, timestamp);

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
logger_db_delete_trace_channel_and_data(sqlite3* db, uint32_t chnl)
{
  const static char*    sql_cmd_buffer = 
    "delete from channel_trace where chnl_num = ?1";
  const static char*    sql_cmd_buffer_chnl_log = 
    "delete from channel_log where ch_num = ?1";
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

  if(sqlite3_step(stmt) != SQLITE_DONE)
  {
    return FALSE;
  }

  if(sqlite3_finalize(stmt) != SQLITE_OK)
  {
    return FALSE;
  }

  ret = sqlite3_prepare_v2(db, sql_cmd_buffer_chnl_log, strlen(sql_cmd_buffer_chnl_log), &stmt, &sql_error);
  if(ret != SQLITE_OK)
  {
    return FALSE;
  }

  sqlite3_bind_int(stmt, 1, chnl);

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

bool
logger_db_check_if_channel_traced(sqlite3* db, uint32_t chnl)
{
  const static char*    sql_cmd_buffer = 
    "select * from channel_trace where chnl_num = ?1";
  sqlite3_stmt*         stmt;
  const char*           sql_error;
  int                   ret;

  ret = sqlite3_prepare_v2(db, sql_cmd_buffer, strlen(sql_cmd_buffer), &stmt, &sql_error);
  if(ret != SQLITE_OK)
  {
    return FALSE;
  }

  sqlite3_bind_int(stmt, 1, chnl);

  // gotta be exactly one row
  if(sqlite3_step(stmt) != SQLITE_ROW)
  {
    return FALSE;
  }

  if(sqlite3_finalize(stmt) != SQLITE_OK)
  {
    return FALSE;
  }
  return TRUE;
}
