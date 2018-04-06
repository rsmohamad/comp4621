#ifndef HTTPRES_H
#define HTTPRES_H

struct HTTPRes {
  int status;
  int bodyFd;
  char *date;
  char *server;
  char *contentType;
  char *contentEnc;
  char *transferEnc;
};

void writeToSocket(struct HTTPRes *, int);

#endif
