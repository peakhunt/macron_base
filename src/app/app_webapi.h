#ifndef __APP_WEBAPI_DEF_H__
#define __APP_WEBAPI_DEF_H__

#include "mongoose.h"

extern bool app_webapi_handler(struct mg_connection* nc, struct http_message* hm);

#endif /* !__APP_WEBAPI_DEF_H__ */
