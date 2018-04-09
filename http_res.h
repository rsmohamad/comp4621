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

  // Keep alive properties
  int max;
  int timeout;
};

void cleanup(struct HTTPRes *);
void setKeepAlive(struct HTTPRes *, int, int);
void setContent(struct HTTPRes *, char *, int);
void setCurrentDate(struct HTTPRes *);
void writeToSocket(struct HTTPRes *, int);

#endif
