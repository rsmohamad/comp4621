#ifndef HTTPRES_H
#define HTTPRES_H

struct HTTPRes {
  int persistent;
  int gzipped;
  char *fname;

  char date[512];
  char *type;
  char *server;
  char *status;
  size_t len;

  // Keep alive properties
  int max;
  int to;
};

void setContent(struct HTTPRes *, char *, int);
void set400(struct HTTPRes *res);
void set501(struct HTTPRes *res);
void writeToSocket(struct HTTPRes *, int);

#endif
