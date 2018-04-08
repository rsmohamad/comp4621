#ifndef HTTPRES_H
#define HTTPRES_H

struct HTTPRes {
  int chunked;
  int gzipped;
  char *type;
  char *date;
  char *server;
  char *status;
  size_t len;
  unsigned char *content;
};

void cleanup(struct HTTPRes *);
void setContent(struct HTTPRes *, char *, int);
void setCurrentDate(struct HTTPRes *);
void writeToSocket(struct HTTPRes *, int);

#endif
