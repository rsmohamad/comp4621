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
  response->type = "text/plain";

  for (int ptr = 0; mime[ptr].ext != 0; ptr++)
    if (strcmp(mime[ptr].ext, ext) == 0) response->type = mime[ptr].type;
}

void set404(struct HTTPRes *response) {
    response->status = "404 Not found";
    response->content = (unsigned char *)"File not found\n";
    response->len = strlen((char *)response->content);
    response->type = "text/plain";
    response->gzipped = 0;
    response->chunked = 0;
}

void getHeaderStr(struct HTTPRes *response, char *h) {
  sprintf(h, "HTTP/1.1 %s\r\n", response->status);
  sprintf(h + strlen(h), "Date: %s\r\n", response->date);
  sprintf(h + strlen(h), "Server: %s\r\n", response->server);
  sprintf(h + strlen(h), "Content-Type: %s\r\n", response->type);

  if (response->gzipped)
    sprintf(h + strlen(h), "Content-Encoding: deflate\r\n");

  if (response->chunked)
    sprintf(h + strlen(h), "Transfer-Encoding: chucked\r\n");

  sprintf(h + strlen(h), "Connection: close\r\n");
  sprintf(h + strlen(h), "\r\n");
}

int readFile(struct HTTPRes *response, char *path){
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

void readContent(struct HTTPRes *response, char *path, int useGzip) {
  if (strcmp(path, "/") == 0 || strlen(path) == 0) 
    path = "index.html";
  else
    path++;

  if (readFile(response, path))
    return set404(response);

  response->gzipped = 0;

  if (useGzip) {
    unsigned char *compressed = malloc(response->len);
    size_t complen = response->len;
    int err = compress(compressed, &complen, response->content, complen);
    free(response->content);
    response->gzipped = 1;
    response->content = compressed;
    response->len = complen;    
  }

  response->status = "200 OK";
  response->chunked = 0;
  setContentType(response, path);
}

void writeToSocket(struct HTTPRes *response, int sockfd) {
  char header[1024];
  getHeaderStr(response, header);
  
  printf("%s", header);
  printf("%s\n", response->content);

  write(sockfd, header, strlen(header));
  write(sockfd, response->content, response->len);
}
