#include "thread.h"

void
lib_utils_init(void)
{
  //
  // initialize key for thread specific data storage
  //
  thread_specific_init();
}
