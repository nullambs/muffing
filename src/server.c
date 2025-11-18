#ifndef MUFFING_SERVER_H
#define MUFFING_SERVER_H

#include "server.h"
#include "handler.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>

int sfd;
extern uint16_t port;
#define BUFFER_SIZE 4096

int init_server() {
  int                 err;
  char                ipstr[16], hostname[128]; struct sockaddr_in  addr;
  struct addrinfo     hints, *result;

  if (gethostname(hostname, sizeof(hostname) - 1) == -1) {
    fputs("Failed to get hostname", stderr);
    return EXIT_FAILURE;
  }

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  err = getaddrinfo(hostname, NULL, &hints, &result);
  if (err) {
    fprintf(stderr, "Failed to get host IP address: %s\n", gai_strerror(err));
    exit(EXIT_FAILURE);
  }

  if (inet_ntop(AF_INET, &((struct sockaddr_in*)result->ai_addr)->sin_addr, ipstr, sizeof(ipstr)) == NULL) {
    fputs("Failed to get host IP address\n", stderr);
    return errno;
  }

  freeaddrinfo(result);

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) {
    perror("Failed to create socket");
    exit(errno);
  }

  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  if (bind(sfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1) {
    perror("Failed to bind socket");
    exit(errno);
  }

  if (listen(sfd, 0) == -1) {
    perror("Failed to start listening");
    exit(errno);
  }

  printf("\x1B[1;32mmuffing is ready to accept connections\n\n"
    " \x1B[36maccess points\n"
    "  \x1B[0mHTTP\n"
    "  \x1B[0m- localhost    \x1B[33mhttp://localhost:%hu\x1B[0m\n"
    "  \x1B[0m- lan          \x1B[33mhttp://%s:%hu\x1B[0m\n\n",
    port, ipstr, port);

  return 0;
}

int run_server() {
  char     buff[BUFFER_SIZE];
  int      client;

  for (;;) {
    if ((client = accept(sfd, NULL, NULL)) == -1 ) {
      perror("Failed to accept connection");
      continue;
    }

    if (recv(client, buff, BUFFER_SIZE, 0) == -1) {
      perror("Failed to read message");
      continue;
    }

    handle_request(buff, sizeof(buff), client);
  }

  return 0;
}

#endif // MUFFING_SERVER_H
