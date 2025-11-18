#include "sig.h"
#include <sys/socket.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

extern int sfd;

void sighandl(int _) {
  puts("\x1B[0mExiting...\n");
  shutdown(sfd, SHUT_RDWR);
  close(sfd);
  exit(EXIT_SUCCESS);
}

void reg_sig_handlers() {
  signal(SIGINT, sighandl);
  signal(SIGTERM, sighandl);
  signal(SIGKILL, sighandl);
}

