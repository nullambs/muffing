#ifndef MUFFING_HANDLER_H
#define MUFFING_HANDLER_H

#include <stddef.h>

enum handlers {
  HN_HTTP,
  HN_UNKNOWN
};

int handle_request(char *, size_t, int);

#endif // !MUFFING_HANDLER_H
