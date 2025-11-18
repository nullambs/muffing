#include "handlers/http.h"

#include <stdio.h>
#include <string.h>
#include <utils/time.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <utils/url.h>
#include <utils/path.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#define RES_HEADERS_SIZE 1024
#define RES_BODY_SIZE 8128
#define RES_SIZE (RES_HEADERS_SIZE + RES_BODY_SIZE)

static __thread struct req {
  char *method;
  char *http;
  char *path;
} req;

#define MAX_HEADER_NAME 128
#define MAX_HEADER_VALUE 256
#define MAX_HEADERS 64
typedef struct header {
  char name[MAX_HEADER_NAME];
  char value[MAX_HEADER_VALUE];
} header_t;
static __thread header_t headers[MAX_HEADERS];
static __thread unsigned char total_headers = 0;

static __thread unsigned short code;

static __thread enum body_write_state {
  BW_IDLE,
  BW_ONGOING,
} bwstate = BW_IDLE;

int parse_request(char *);
void log_request(int);
ssize_t get_res(char *, size_t);
int handle_http_req_(char *, size_t, int);

int handle_http_req(char *buff, size_t buffsize, int destfd) {
  int res = handle_http_req_(buff, buffsize, destfd);
  shutdown(destfd, SHUT_RDWR);
  return res;
}

int handle_http_req_(char *buff, size_t buffsize, int destfd) {
  char res[RES_SIZE + 1];
  int err;
  size_t reslen = -1;
  
  err = parse_request(buff);
  if (err) {
    return 0;
  }

  log_request(err);

  do {
    if ((reslen = get_res(res, RES_SIZE - 1)) == -1) {
      fprintf(stderr, "[HTTP] failed to prepare request\n");
      return -1;
    }

    if (write(destfd, res, reslen) == -1) {
      perror("[HTTP] failed to send response");
      return -1;
    }
  } while(bwstate == BW_ONGOING);

  return 0;
}

int parse_request(char *buff ) {
  char *cr = NULL;

  req.method = buff;
  req.path = NULL;
  req.http = NULL;

  for (char *x = buff; *x; x++) {
    if (*x == '\r') { cr = x; }
    if (*x == '\n' && cr) { *cr = '\0'; break; }
    if (cr && cr != x) { cr = NULL; }

    if (*x == ' ') {
      *x = '\0';
      if (req.path == NULL) { req.path = x + 1; }
      else if (req.http == NULL) { req.http = x + 1; }
    }
  }

  return req.path == NULL ? -1 : 0;
}

int push_header(char *restrict name, char *restrict value) {
  if (total_headers >= MAX_HEADERS) {
    errno = 1;
    return -1;
  }

  strcpy(headers[total_headers].name, name);
  strcpy(headers[total_headers].value, value);
  total_headers++;

  return 0;
}

int push_content_length(size_t length) {
  char value[MAX_HEADER_VALUE];
  sprintf(value, "%zu", length);
  return push_header("Content-Length", value);
}

int push_attachment_header(char *filename) {
  char value[MAX_HEADER_VALUE];
  snprintf(value, MAX_HEADER_VALUE -1, "attachment; filename=\"%s\"", filename);
  return push_header("Content-Disposition", value);
}

ssize_t get_headers(char *restrict buff, size_t maxlen) {
  char msg[64], datestr[512], headerline[257];
  unsigned short x;

  switch (code) {
    case 200:
      strcpy(msg, "OK");
      break;
    case 400:
      strcpy(msg, "Bad request");
      break;
    case 404:
      strcpy(msg, "Not found");
    case 418:
      strcpy(msg, "I'm a teapot");
      break;
    case 518:
      strcpy(msg, "Not implemented");
      break;
  }

  gmtnowstr("%a, %d %b %Y %H:%M:%S GMT", datestr, sizeof(datestr));

  if (snprintf(buff, maxlen,
    "%s %d %s\r\n" \
    "Date: %s\r\n" \
    "Connection: close\r\n",
    req.http, code, msg, datestr
  ) == -1) {
    perror("[HTTP] failed to write default headers");
    return -1;
  }

  for (x = 0; x < total_headers; x++) {
    if (snprintf(headerline, 256, "%s: %s\r\n", headers[x].name, headers[x].value) == -1) {
      perror("[HTTP] failed to write header");
      return -1;
    }

    strncat(buff, headerline, maxlen);
  }

  strncat(buff, "\r\n", maxlen);

  return strlen(buff);
}

ssize_t get_body_dir(char *restrict buff, size_t maxlen, char *cwdp) {
  DIR *cwd;
  struct dirent *ent = NULL;
  char dirsstr[RES_BODY_SIZE / 2], *dstr = dirsstr, *entpath, *url, c;
  ssize_t len = 0;

  cwd = opendir(cwdp);
  for (ent = readdir(cwd); ent; ent = readdir(cwd)) {
    entpath = pathjoin(3, "/", req.path, ent->d_name);
    url = entpath;
    len = snprintf(dstr, RES_BODY_SIZE / 2 - len, "<a href=\"%s\">%s<br/></a>\r\n", url, ent->d_name);
    if (len < 0) {
      perror("[HTTP] couldn't dir entry link");
      return -1;
    }

    dstr += len;
  }
  closedir(cwd);

  if ((len = snprintf(buff, RES_BODY_SIZE,
    "<!DOCTYPE html>\r\n"
    "<html>\r\n"
    "<head>\r\n"
    "<title>listing of %s</title>\r\n"
    "</head>\r\n"
    "<body>\r\n"
    "<h1>Listing of %s</h1><input id=\"filter\" placeholder=\"Filter items... Hit '/' to focus\" /><hr/>"
    "%s\r\n"
    "<hr/>"
    "</body>\r\n"
    "<script>d=document;a=d.getElementById('filter');b=d.querySelectorAll('a');d.addEventListener('keydown',e=>{if(e.key=='/'){e.preventDefault();setTimeout(_=>{a.focus();a.select()})}});a.addEventListener('input',e=>b.forEach(f=>{if(f.innerText.toLowerCase().includes(e.target.value.toLowerCase()))f.style.display='block';else f.style.display='none'}))</script>\r\n"
    "</html>\r\n",
    req.path, req.path, dirsstr
  )) <= 0) {
    perror("[HTTP] couldn't write body");
    return -1;
  }

  if (push_content_length(strlen(buff)) == -1) {
    perror("[HTTP] failed to write content-length");
    return -1;
  }

  if (push_header("Content-Type", "text/html") == -1) {
    perror("[HTTP] failed to write content-type");
    return -1;
  }
  return len;
}

ssize_t get_body_404(char *restrict buff, size_t maxlen) {
  ssize_t len;
  if ((len = snprintf(buff, RES_BODY_SIZE,
    "<!DOCTYPE html>\r\n"
    "<html>\r\n"
    "<head>\r\n"
    "<title>No such file or directory</title>\r\n"
    "</head>\r\n"
    "<body>\r\n"
    "<h1>404 No such file or directory</h1><hr/>"
    "</body>\r\n"
    "</html>\r\n"
  )) < 0) {
    perror("[HTTP] couldn't write body");
    return -1;
  }

  if (push_content_length(strlen(buff)) == -1) {
    perror("[HTTP] failed to write content length");
    return -1;
  }

  return len;
}

ssize_t get_body_file(char *restrict buff, size_t maxlen, char *file) {
  ssize_t len;
  int fd;
  static FILE *f;
  struct stat fs;


  if (bwstate == BW_IDLE) {
    fd = open(file, 0);
    fstat(fd, &fs);
    f = fopen(file, "rb");
    push_attachment_header(filename(file));
    push_content_length(fs.st_size);
    push_header("Content-Type", "application/octet-stream");
    bwstate = BW_ONGOING;
  }

  len = fread(buff, sizeof(char), maxlen, f);
  buff[len] = '\0';
  if (feof(f)) {
    bwstate = BW_IDLE;
    fclose(f);
  } else if (ferror(f)) {
    bwstate = BW_IDLE;
    fclose(f);
    perror("[HTTP] failed to read file");
    return -1;
  }

  return len;
}

ssize_t get_body(char *restrict buff, size_t maxlen) {
  char *cwdp;
  struct stat fstat;
  ssize_t len = 0;

  cwdp = pathjoin(2, "./", req.path);
  if (stat(cwdp, &fstat) == -1) {
    if (errno == ENOENT) { code = 404; }
    else {
      perror("[HTTP] failed to get file stat");
      return -1;
    }
  }
  
  if (code == 404) {
    if ((len = get_body_404(buff, RES_BODY_SIZE)) == -1) {
      return -1;
    }
  } else if (fstat.st_mode & S_IFDIR) {
    if ((len = get_body_dir(buff, RES_BODY_SIZE, cwdp)) == -1) {
      return -1;
    }
  } else if (fstat.st_mode & S_IFREG) {
    if ((len = get_body_file(buff, RES_BODY_SIZE, cwdp)) == -1) {
      return -1;
    }
  }

  return len;
}

ssize_t get_res(char *restrict buff, size_t maxlen) {
  char headers[RES_HEADERS_SIZE + 1],
       body[RES_BODY_SIZE + 1];
  ssize_t bodylen = -1, headerslen = -1;
  code = 200;
  total_headers = 0;

  if (bwstate == BW_ONGOING) return get_body(buff, maxlen);

  if (strcmp("GET", req.method)) { code = 418; }

  if ((bodylen = get_body(body, RES_BODY_SIZE)) == -1) {
    perror("[HTTP] failed to prepare body");
    return -1;
  }

  if ((headerslen = get_headers(headers, RES_HEADERS_SIZE)) == -1) {
    perror("[HTTP] failed to prepare headers");
    return -1;
  }

  memcpy(buff, headers, headerslen);
  memcpy(buff + headerslen, body, bodylen);

  return headerslen + bodylen;
}

void log_request(int err) {
  char log[256], timestr[80];
  short bad_method;

  if (err == -1) {
    sprintf(log, "%s", "\x1B[31mBad request");
  } else if (!strcmp("GET", req.method)) {
    sprintf(log, "\x1B[32m%s\x1B[0m via %s: %s", req.method, req.http, req.path);
  } else {
    bad_method = 1;
    sprintf(log, "\x1B[31m%s\x1B[0m via %s: %s", req.method, req.http, req.path);
  }

  nowstr("%Y-%m-%D %H:%M:%S", timestr, 80);
  printf("  [%s] %s\n", timestr, log);
}

