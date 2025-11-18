#include "server.h"
#include "sig.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define DEFAULT_PORT 8080

uint16_t port;

void print_help();
int parse_args(int, char **);

int main(int argc, char **argv) {
  if (parse_args(argc, argv)) {
    print_help();
    return 1;
  }

  init_server();
  reg_sig_handlers();
  run_server();

  return 0;
}

void print_help() {
    printf("http-server [-h] [(-p | --port) <PORT>]\n\n" \
           "  DEFAULTS\n" \
           "  port     8080\n\n");

}

int parse_args(int argc, char **argv) {
  char *err;

  port = DEFAULT_PORT;

  for (int x = 1; x < argc; x++) {
    if (!strcmp(argv[x], "-p") || !strcmp(argv[x], "--port")) {
      if (x + 1 == argc) {
        return 1;
      }

      port = strtol(argv[x + 1], &err, 10);
      if (*err) return 1;
      x++;
    } else if (!strcmp(argv[x], "-h")) {
      return 1;
    }
  }

  return 0;
}
