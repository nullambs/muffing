#include "utils/time.h"

#include <time.h>
#include <stddef.h>

struct tm *localnow(long offset) {
  time_t t = time(NULL);
  t += offset;
  return localtime(&t);
}

char *gmtnowstr(char *restrict fmt, char *restrict buff, size_t buffsize) {
  struct tm *t = localnow(0);
  strftime(buff, buffsize, fmt, localnow(-t->tm_gmtoff));
  return buff;
}

char* nowstr(char *restrict fmt, char *restrict buff, size_t buffsize) {
  strftime(buff, buffsize, fmt, localnow(0));
  return buff;
}
