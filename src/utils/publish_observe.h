#ifndef __PUBLISH_OBSERVE_DEF_H__
#define __PUBLISH_OBSERVE_DEF_H__

#include "common_def.h"
#include "list.h"

struct __observer_t;
typedef struct __observer_t observer_t;

struct __observer_t
{
  struct list_head      le;
  void (*notify)(observer_t* obs, void* arg);
};

typedef struct
{
  struct list_head    head;
} publisher_t;

extern void publisher_init(publisher_t* pub);
extern void publisher_add_observer(publisher_t* pub, observer_t* obs);
extern void publisher_exec_notify(publisher_t* pub, void* arg);

#endif /* !__PUBLISH_OBSERVE_DEF_H__ */
