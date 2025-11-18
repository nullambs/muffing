#include "utils/url.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define URL_SIZE 256

static __thread char url[256];

char *encodeurl(char *restrict decoded) {
  char *x, *u = url; for (x = decoded; *x; x++, u++) {
    switch (*x) {
      case ' ':
      case ':':
      case '?':
      case '#':
      case '[':
      case ']':
      case '@':
      case '!':
      case '$':
      case '&':
      case '\'':
      case '(':
      case ')':
      case '*':
      case '+':
      case ',':
      case ';':
      case '=':
        sprintf(u, "%%%x", (short)*x);
        u += 2;
        break;
      default:
        *u = *x;
    }
  }

  *u = '\0';

  return url;
}

char *decodeurl(char *restrict encoded) {
  char *x, *u = url, hex[3] = {0};
  for (x = encoded; *x; x++) {
    if (*x == '%') {
      strncpy(hex, x + 1, 2);
      *u = (char)strtol(hex, NULL, 16);
      x += 2;
    } else {
      *u = *x;
    }
  }

  return url;
}
