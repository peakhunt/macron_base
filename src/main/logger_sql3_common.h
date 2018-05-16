#ifndef __LOGGER_SQL3_COMMON_DEF_H__
#define __LOGGER_SQL3_COMMON_DEF_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <errno.h>

#include "logger.h"
#include "common_def.h"

extern sqlite3* logger_db_open(const char* path);
extern void logger_db_close(sqlite3* db);
extern bool logger_db_insert_chnl(sqlite3* db, uint32_t chnl, unsigned long timestamp, double v);
extern bool logger_db_insert_alarm(sqlite3* db, uint32_t alarm, unsigned long timestamp, logger_alarm_event_t event);
extern bool logger_db_insert_trace_channel(sqlite3* db, uint32_t chnl, unsigned long timestamp);
extern bool logger_db_delete_trace_channel_and_data(sqlite3* db, uint32_t chnl);
extern bool logger_db_get_all_trace_channels(sqlite3* db, sqlite3_callback cb, void* cb_data);

#endif /* !__LOGGER_SQL3_COMMON_DEF_H__ */
