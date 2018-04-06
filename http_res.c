#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "http_res.h"
#include "zlib/zlib.h"

const struct {
  char *ext;
  char *type;
} mime[] = {{".css", "text/css"},
            {".jpg", "image/jpeg"},
            {".pdf", "application/pdf"},
            {".html", "text/html"},
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
  response->contentType = "text/plaintext";

  for (int ptr = 0; mime[ptr].ext != 0; ptr++)
    if (strcmp(mime[ptr].ext, ext) == 0)
        response->contentType = mime[ptr].type;
  
}

void getHeaderStr(struct HTTPRes *response, char *h) {
  sprintf(h, "HTTP/1.1 %s\r\n", response->status);
  sprintf(h + strlen(h), "Date: %s\r\n", response->date);
  sprintf(h + strlen(h), "Server: %s\r\n", response->server);
  sprintf(h + strlen(h), "Content-Type: %s\r\n", response->contentType);

  if (response->contentEnc)
    sprintf(h + strlen(h), "Content-Encoding: %s\r\n", response->contentEnc);

  if (response->transferEnc)
    sprintf(h + strlen(h), "Transfer-Encoding: %s\r\n", response->transferEnc);

  sprintf(h + strlen(h), "Connection: Closed\r\n");
  sprintf(h + strlen(h), "\r\n");
}

void writeToSocket(struct HTTPRes *response, int sockfd) {
  char header[1024];
  getHeaderStr(response, header);
  printf("%s", header);
  write(sockfd, header, strlen(header));
  write(sockfd, response->content, strlen(response->content));
}

void readContent(struct HTTPRes *response, char *path, int useGzip) {
  if (strcmp(path, "/") == 0 || strlen(path) == 0)
    path = "index.html";
  else
    path++;

  FILE *f = fopen(path, "r");

  if (f == NULL) {
    response->status = "404 Not found";
    response->content = "File not found";
    response->contentType = "text/plaintext";
    response->contentEnc = NULL;
    response->transferEnc = NULL;
    return;
  }

  fseek(f, 0L, SEEK_END);
  size_t len = ftell(f);
  rewind(f);

  unsigned char *buffer = malloc(len + 1);
  fread(buffer, len, 1, f);
  buffer[len] = 0;
  fclose(f);

  if (useGzip) {
    unsigned char *compressed = malloc(len + 1);
    compress(compressed, NULL, buffer, len);
    response->contentEnc = "gzip";
    response->content = compressed;
    free(buffer);
  } else {
    response->contentEnc = NULL;
    response->content = buffer;
  }

  response->status = "200 OK";
  response->transferEnc = NULL;
  setContentType(response, path);
}
