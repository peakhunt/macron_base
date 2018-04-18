#ifndef __ATOMIC_FILE_UPDATE_DEF_H__
#define __ATOMIC_FILE_UPDATE_DEF_H__

extern int atomic_file_update(const char* path, int (*content_update)(int fd));

#endif /* !__ATOMIC_FILE_UPDATE_DEF_H__ */
