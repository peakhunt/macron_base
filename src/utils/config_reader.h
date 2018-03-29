#ifndef __CONFIG_READER_DEF_H__
#define __CONFIG_READER_DEF_H__

#include "common_def.h"
#include "cJSON.h"

extern cJSON* config_reader_init(const char* filename);
extern void config_reader_fini(cJSON* root);

#endif /* !__CONFIG_READER_DEF_H__ */
