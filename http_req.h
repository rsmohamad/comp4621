#ifndef HTTP_REQ_H
#define HTTP_REQ_H

struct HTTPReq {
  char *path;
  char *host;
  char *type;
  int gzip;
  int persistent;
};

int parseRequest(struct HTTPReq *, char *);

#endif
