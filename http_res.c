#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "gzip.h"
#include "http_res.h"

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

void setCurrentDate(struct HTTPRes *response) {
  time_t now = time(NULL);
  struct tm *tm = localtime(&now);
  response->date = malloc(512);
  strftime(response->date, 512, "%a, %d %b %Y %H:%M:%S %Z", tm);
}

void setContentType(struct HTTPRes *response, char *path) {
  char *ext = strrchr(path, '.');
  response->type = "text/plain";

  for (int ptr = 0; ext && mime[ptr].ext != 0; ptr++)
    if (strcmp(mime[ptr].ext, ext) == 0)
      response->type = mime[ptr].type;
}

void set404(struct HTTPRes *response) {
  setContent(response, "404.html", 0);
  response->status = "404 Not Found";
}

void getHeaderStr(struct HTTPRes *response, char *h) {
  sprintf(h, "HTTP/1.1 %s\r\n", response->status);
  sprintf(h + strlen(h), "Date: %s\r\n", response->date);
  sprintf(h + strlen(h), "Server: %s\r\n", response->server);
  sprintf(h + strlen(h), "Content-Type: %s\r\n", response->type);

  if (response->gzipped)
    sprintf(h + strlen(h), "Content-Encoding: gzip\r\n");

  if (response->chunked)
    sprintf(h + strlen(h), "Transfer-Encoding: chunked\r\n");

  sprintf(h + strlen(h), "Connection: close\r\n");
  sprintf(h + strlen(h), "\r\n");
}

int readFile(struct HTTPRes *response, char *path) {
  FILE *f = fopen(path, "r");
  if (f == NULL)
    return -1;

  fseek(f, 0L, SEEK_END);
  response->len = ftell(f);
  rewind(f);

  response->content = malloc(response->len);
  fread(response->content, response->len, 1, f);
  fclose(f);
  return 0;
}

void setContent(struct HTTPRes *response, char *path, int useGzip) {
  if (strcmp(path, "/") == 0 || strlen(path) == 0)
    path = "index.html";
  else if (strstr(path, "/") == path)
    path++;

  if (readFile(response, path) < 0)
    return set404(response);

  if (useGzip) {
    unsigned char *compressed = malloc(response->len);
    size_t complen = response->len;
    int err = compressToGzip(compressed, &complen, response->content, complen);
    free(response->content);
    response->content = compressed;
    response->len = complen;
  }

  response->gzipped = useGzip;
  response->status = "200 OK";
  response->chunked = 0;
  setContentType(response, path);
}

void writeToSocket(struct HTTPRes *response, int sockfd) {
  char header[1024];
  getHeaderStr(response, header);

  printf("%s", header);

  write(sockfd, header, strlen(header));
  write(sockfd, response->content, response->len);
}

void cleanup(struct HTTPRes *response) {
  if (response->date)
    free(response->date);
  if (response->content)
    free(response->content);
}
