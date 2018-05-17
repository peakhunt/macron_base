#ifndef __WEBSERVER_API_LOGGERIF_DEF_H__
#define __WEBSERVER_API_LOGGERIF_DEF_H__

#include "webserver_api_common.h"

extern void webserver_api_loggerif_init(void);
extern bool webserver_api_loggerif_handler(struct mg_connection* nc, struct http_message* hm);

#endif /* !__WEBSERVER_API_LOGGERIF_DEF_H__ */
