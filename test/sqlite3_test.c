#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <errno.h>

#define LOGGER_DB     "db/logging_db.sq3"

static sqlite3*       _db = NULL;

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
    fprintf(stderr, "failed to set auto_vacuum\n");
  }
  //sqlite3_exec(sq3, "pragma journal_mode = delete", 0, 0, 0);
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

int
test_delete(sqlite3* db)
{
  const static char*  sql_cmd_buffer = "delete from channel_log";
  int                   ret;

  sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
  // step 1. clear channels
  ret = sqlite3_exec(db, sql_cmd_buffer, NULL, NULL, NULL);
  if(ret != SQLITE_OK)
  {
    return -1;
  }
  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
  return 0;
}

static void
logger_db_init(void)
{
  _db = logger_db_open(LOGGER_DB);
  if(_db == NULL)
  {
    fprintf(stderr, "failed to open logger db %s\n", LOGGER_DB);
    exit(-1);
  }
  fprintf(stdout, "logger db %s opened\n", LOGGER_DB);
}

int
main(void)
{
  logger_db_init();

  test_delete(_db);

  logger_db_close(_db);
}
