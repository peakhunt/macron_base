#include <stdio.h>
#include "publish_observe.h"

void
publisher_init(publisher_t* pub)
{
  INIT_LIST_HEAD(&pub->head);
}

void
publisher_add_observer(publisher_t* pub, observer_t* obs)
{
  INIT_LIST_HEAD(&obs->le);
  list_add_tail(&obs->le, &pub->head);
}

void
publisher_exec_notify(publisher_t* pub, void* arg)
{
  observer_t*   obs;

  list_for_each_entry(obs, &pub->head, le)
  {
    obs->notify(obs, arg);
  }
}
