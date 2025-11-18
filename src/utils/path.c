#include "utils/path.h"

#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define MAX_PATH 2048
static __thread pathc_t path[MAX_PATH];

void truncpathend(char *src, size_t len) {
  pathc_t *p = src + len - 1;

  while (p > src) {
    if (*p == '/') { *p = '\0'; }
    else { break; }
  }
}

pathc_t *truncpathstart(char *src) {
  pathc_t *p;

  for (p = src; *p == '/'; p++);
  return p;
}

path_t pathjoin(unsigned count, ...) {
  va_list ap;
  path_t arg;
  pathc_t arg_cp[MAX_PATH], *arg_cp_ptr;
  size_t arglen, len = 0, x;

  strcpy(path, "");
  if (!count) return path;

  va_start(ap, count);

  for (x = 0; x < count; x++) {
    arg = va_arg(ap, path_t);
    strcpy(arg_cp, arg);

    arglen = strlen(arg_cp);
    truncpathend(arg_cp, arglen);
    if (x) arg_cp_ptr = truncpathstart(arg_cp);
    else arg_cp_ptr = arg_cp;

    strcat(path, arg_cp_ptr);
    if (path[strlen(path) - 1] != '/') strcat(path, "/");
  }

  va_end(ap);

  truncpathend(path, strlen(path));

  return path;
}

path_t filename(path_t p) {
  pathc_t *c;
  for (c = p + strlen(p) - 1; c != p && *c != '/'; c--);
  return c + 1;
}
