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

extern bool logger_db_set_trace_channels(sqlite3* db, uint32_t* chnls, uint32_t n_chnls, unsigned long timestamp);
extern bool logger_db_get_trace_channels(sqlite3* db, uint32_t** chnls, uint32_t* n_chnls);

extern bool logger_db_get_all_trace_channels(sqlite3* db, sqlite3_callback cb, void* cb_data);

extern bool logger_db_get_channel_log(sqlite3* db, unsigned long start, unsigned long end, uint32_t* chnls, uint32_t num_chnls,
    sqlite3_callback cb, void* cb_data);

extern bool logger_db_get_alarm_log(sqlite3* db, unsigned long start_time, unsigned long end_time, sqlite3_callback cb, void* cb_data);

extern bool logger_db_clean_up_old_channel_logs(sqlite3* db, unsigned long older_than);
extern bool logger_db_clean_up_old_alarm_logs(sqlite3* db, unsigned long older_than);

extern bool logger_db_clean_up_old_channel_alarm_logs(sqlite3* db, unsigned long channel_older_than, unsigned long alarm_older_than);

#endif /* !__LOGGER_SQL3_COMMON_DEF_H__ */
