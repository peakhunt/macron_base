#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "config_reader.h"
#include "debug_log.h"

static char*
load_file_to_buffer(const char* filename)
{
  int         fd,
              ret;
  char*       buffer;
  struct stat sb;

  fd = open(filename,  O_RDONLY);
  if(fd < 0)
  {
    debug_log("failed to open config file: %s, %s\n", filename, strerror(errno));
    goto error1;
  }

  if(fstat(fd, &sb) == -1)
  {
    debug_log("fcntl failed: %s\n", strerror(errno));
    goto error2;
  }

  buffer = malloc(sb.st_size + 1);
  if(buffer == NULL)
  {
    debug_log("failed to allocate buffer %s\n");
    goto error2;
  }

  ret = read(fd, buffer, sb.st_size);
  if(ret != sb.st_size)
  {
    debug_log("failed in reading. %d, %d, %s\n", ret, sb.st_size, strerror(errno));
    goto error3;
  }

  buffer[sb.st_size] = '\0';
  close(fd);
  return buffer;

error3:
  free(buffer);
error2:
  close(fd);
error1:
  return NULL;
}


cJSON*
config_reader_init(const char* filename)
{
  char* buffer;
  cJSON* root;

  buffer = load_file_to_buffer(filename);
  if(buffer == NULL)
  {
    return NULL;
  }

  root = cJSON_Parse(buffer);
  free(buffer);

  if(root == NULL)
  {
    return NULL;
  }

  return root;
}

void
config_reader_fini(cJSON* root)
{
  cJSON_Delete(root);
}
