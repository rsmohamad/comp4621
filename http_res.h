#ifndef HTTPRES_H
#define HTTPRES_H

struct HTTPRes {
  unsigned char *content;
  char *contentEnc;
  char *contentType;
  char *date;
  char *server;
  char *status;
  char *transferEnc;
};

void readContent(struct HTTPRes *, char *, int);
void setCurrentDate(struct HTTPRes *);
void writeToSocket(struct HTTPRes *, int);

#endif
