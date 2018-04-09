#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "gzip.h"
#include "http_res.h"

#define CHUNK_SIZE 4096

const struct {
  char *ext;
  char *type;
} mime[] = {{".css", "text/css"},
            {".jpg", "image/jpeg"},
            {".jpeg", "image/jpeg"},
            {".png", "image/png"},
            {".pdf", "application/pdf"},
            {".html", "text/html"},
            {".ppt", "application/vnd.ms-powerpoint"},
            {".pptx",
             "application/"
             "vnd.openxmlformats-officedocument.presentationml.presentation"},
            {0, 0}};

void setCurrentDate(struct HTTPRes *res) {
  time_t now = time(NULL);
  struct tm *tm = localtime(&now);
  res->date = malloc(512);
  strftime(res->date, 512, "%a, %d %b %Y %H:%M:%S %Z", tm);
}

void setContentType(struct HTTPRes *res, char *path) {
  char *ext = strrchr(path, '.');
  res->type = "text/plain";

  for (int ptr = 0; ext && mime[ptr].ext != 0; ptr++)
    if (strcmp(mime[ptr].ext, ext) == 0)
      res->type = mime[ptr].type;
}

void setKeepAlive(struct HTTPRes *res, int timeout, int max) {
  res->max = max;
  res->timeout = timeout;
}

void set404(struct HTTPRes *res) {
  setContent(res, "404.html", 0);
  res->status = "404 Not Found";
}

void getHeaderStr(struct HTTPRes *res, char *h) {
  sprintf(h, "HTTP/1.1 %s\r\n", res->status);
  sprintf(h + strlen(h), "Date: %s\r\n", res->date);
  sprintf(h + strlen(h), "Server: %s\r\n", res->server);
  sprintf(h + strlen(h), "Content-Type: %s\r\n", res->type);

  if (res->gzipped)
    sprintf(h + strlen(h), "Content-Encoding: gzip\r\n");

  if (res->chunked)
    sprintf(h + strlen(h), "Transfer-Encoding: chunked\r\n");
  else
    sprintf(h + strlen(h), "Content-Length: %zu\r\n", res->len);

  // sprintf(h + strlen(h), "Connection: close\r\n");

  sprintf(h + strlen(h), "Keep-Alive: timeout=%d, max=%d\r\n", res->timeout,
          res->max);
  sprintf(h + strlen(h), "Connection: keep-alive\r\n");

  sprintf(h + strlen(h), "\r\n");
}

int readFile(struct HTTPRes *res, char *path) {
  FILE *f = fopen(path, "r");
  if (f == NULL)
    return -1;

  fseek(f, 0L, SEEK_END);
  res->len = ftell(f);
  rewind(f);

  res->content = malloc(res->len);
  fread(res->content, res->len, 1, f);
  fclose(f);
  return 0;
}

void setContent(struct HTTPRes *res, char *path, int useGzip) {
  if (strcmp(path, "/") == 0 || strlen(path) == 0)
    path = "index.html";
  else if (strstr(path, "/") == path)
    path++;

  if (readFile(res, path) < 0)
    return set404(res);

  if (useGzip) {
    unsigned char *compressed = malloc(res->len);
    size_t complen = res->len;
    compressToGzip(compressed, &complen, res->content, complen);
    free(res->content);
    res->content = compressed;
    res->len = complen;
  }

  res->gzipped = useGzip;
  res->status = "200 OK";
  res->chunked = 1;
  setContentType(res, path);
}

void writeToSocket(struct HTTPRes *res, int sockfd) {
  char header[1024];
  getHeaderStr(res, header);
  printf("%s", header);
  write(sockfd, header, strlen(header));

  if (!res->chunked)
    write(sockfd, res->content, res->len);
  else {
    size_t remains = res->len;
    unsigned char *ptr = res->content;

    while (remains > 0) {
      int written = remains > CHUNK_SIZE ? CHUNK_SIZE : remains;
      sprintf(header, "%X\r\n", written);
      write(sockfd, header, strlen(header));
      write(sockfd, ptr, written);
      write(sockfd, "\r\n", 2);
      ptr += written;
      remains -= written;
    }

    write(sockfd, "0\r\n\r\n", 5);
  }
}

void cleanup(struct HTTPRes *res) {
  if (res->date)
    free(res->date);
  if (res->content)
    free(res->content);
}
