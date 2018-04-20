#ifndef __WEBSERVER_API_DEF_H__
#define __WEBSERVER_API_DEF_H__

#include "mongoose.h"

extern bool webserver_api_handler(struct mg_connection* nc, struct http_message* hm);

#endif /* !__WEBSERVER_API_DEF_H__ */
