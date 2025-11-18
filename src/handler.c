#include "handler.h"
#include "handlers/http.h"

#include <string.h>

int select_handler(char *buff) {
  for (char *x = buff; *x; x++) {
    if (!strncmp(x, "HTTP/", 5)) {
      return HN_HTTP;
    }
  }

  return HN_UNKNOWN;
}

int handle_request(char *buff, size_t buffsize, int client) {
  int handler;

  handler = select_handler(buff);
  switch (handler) {
    case HN_HTTP:
      return handle_http_req(buff, buffsize, client);
      break;
    case HN_UNKNOWN:
      return -1;
      break;
  }

  return -1;
}
