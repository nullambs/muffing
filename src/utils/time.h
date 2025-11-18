#ifndef MUTLFS_TIME_H
#define MUTLFS_TIME_H

#include <stddef.h>

char *gmtnowstr(char * restrict fmt, char * restrict buff, size_t buffsize);
char *nowstr(char * restrict fmt, char * restrict buff, size_t buffsize);

#endif // !MUTLFS_TIME_H
