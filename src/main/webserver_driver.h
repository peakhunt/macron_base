#ifndef __WEB_SERVER_DRIVER_DEF_H__
#define __WEB_SERVER_DRIVER_DEF_H__

typedef struct
{
  char*     http_address;
  char*     doc_root;
} webserver_config_t;

extern void webserver_driver_init(void);
extern void webserver_driver_go(void);

#endif /* !__WEB_SERVER_DRIVER_DEF_H__ */
