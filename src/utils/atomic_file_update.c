#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int
atomic_file_update(const char* path, int (*content_update)(int fd))
{
  char    tmp_pathname[strlen(path) + 2];
  mode_t  default_mode;
  int     fd;

  // step 1. create a temporary path
  snprintf(tmp_pathname, sizeof(tmp_pathname), "%s~", path);

  // step 2. unlink if already exists
  unlink(tmp_pathname);

  // step 3. open the temporary path
  default_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
  fd = open(tmp_pathname,O_RDWR|O_CREAT|O_TRUNC,default_mode);

  if (fd==-1)
  {
    return -1;
  }

  // step 4. update
  if(content_update(fd) != 0)
  {
    close(fd);
    return -1;
  }

  if(fsync(fd) == -1)
  {
    close(fd);
    return -1;
  }

  close(fd);

  // step 5. move the temp file to the target path
  // XXX. guaranteed to be atomic in POSIX systems.
  if(rename(tmp_pathname, path) == -1)
  {
    return -1;
  }
  return 0;
}
