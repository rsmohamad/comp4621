#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "http_res.h"
#include "zlib/zlib.h"

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
  strftime(res->date, 512, "%a, %d %b %Y %H:%M:%S %Z", tm);
}

void setContentType(struct HTTPRes *res) {
  char *ext = strrchr(res->fname, '.');
  res->type = "text/plain";

  for (int ptr = 0; ext && mime[ptr].ext != 0; ptr++)
    if (strcmp(mime[ptr].ext, ext) == 0)
      res->type = mime[ptr].type;
}

void getHeaderStr(struct HTTPRes *res, char *h) {
  sprintf(h, "HTTP/1.1 %s\r\n", res->status);
  sprintf(h + strlen(h), "Date: %s\r\n", res->date);
  sprintf(h + strlen(h), "Server: %s\r\n", res->server);
  sprintf(h + strlen(h), "Content-Type: %s\r\n", res->type);
  if (res->gzipped)
    sprintf(h + strlen(h), "Content-Encoding: gzip\r\n");
  sprintf(h + strlen(h), "Transfer-Encoding: chunked\r\n");
  if (res->persistent) {
    sprintf(h + strlen(h), "Keep-Alive: ");
    sprintf(h + strlen(h), "timeout=%d, max=%d\r\n", res->to, res->max);
    sprintf(h + strlen(h), "Connection: keep-alive\r\n");
  } else
    sprintf(h + strlen(h), "Connection: close\r\n");
  sprintf(h + strlen(h), "\r\n");
}

void compressAndWrite(char *fname, int sockfd, int chunk) {
  int compressed, filefd = open(fname, O_RDONLY, 0);
  unsigned char out[chunk], in[chunk];
  char header[16];
  z_stream s;

  s.zalloc = s.zfree = s.opaque = NULL;
  deflateInit2(&s, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 | 16, 8,
               Z_DEFAULT_STRATEGY);

  while ((s.avail_in = read(filefd, in, chunk)) > 0) {
    s.avail_out = chunk;
    s.next_out = out;
    s.next_in = in;
    deflate(&s, Z_SYNC_FLUSH);

    compressed = chunk - s.avail_out;
    sprintf(header, "%X\r\n", compressed);
    write(sockfd, header, strlen(header));
    write(sockfd, out, compressed);
    write(sockfd, "\r\n", 2);
  }

  write(sockfd, "0\r\n\r\n", 5);
  close(filefd);
  deflateEnd(&s);
}

void readChunkAndWrite(char *fname, int sockfd, int chunk) {
  int filefd = open(fname, O_RDONLY, 0), len;
  char buf[chunk], header[16];

  while ((len = read(filefd, buf, chunk)) > 0) {
    sprintf(header, "%X\r\n", len);
    write(sockfd, header, strlen(header));
    write(sockfd, buf, len);
    write(sockfd, "\r\n", 2);
  }
  write(sockfd, "0\r\n\r\n", 5);
  close(filefd);
}

void setContent(struct HTTPRes *res, char *path, int useGzip) {
  if (strcmp(path, "/") == 0 || strlen(path) == 0)
    res->fname = "index.html";
  else if (strstr(path, "/") == path)
    res->fname = path + 1;

  res->gzipped = useGzip;
  res->status = "200 OK";
  if (access(res->fname, F_OK) == -1) {
    res->fname = "404.html";
    res->status = "404 Not Found";
  }
  setCurrentDate(res);
  setContentType(res);
}

void writeToSocket(struct HTTPRes *res, int sockfd) {
  char header[1024];
  getHeaderStr(res, header);
  write(sockfd, header, strlen(header));
  printf("%s", header);

  if (res->gzipped)
    compressAndWrite(res->fname, sockfd, CHUNK_SIZE);
  else
    readChunkAndWrite(res->fname, sockfd, CHUNK_SIZE);
}
