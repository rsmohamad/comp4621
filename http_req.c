#include <stdlib.h>

#include "http_req.h"
#include "strutils.h"

struct HTTPReq *parseRequest(char *buf) {
  char **lines = tokenize(buf, "\r\n");
  char **_lines = lines;
  struct HTTPReq *request = malloc(sizeof(struct HTTPReq));

  // Get the path
  char **firstline = tokenize(*lines++, " ");
  request->path = firstline[1];
  free(firstline);

  // Get the fields
  char *current;
  while ((current = *lines++)) {
    char *value;
    if ((value = getValue(current, "Host", ": ")))
      request->host = value;
    else if ((value = getValue(current, "Accept-Encoding", ": ")))
      request->gzip = contains(value, "gzip");
  }

  free(_lines);
  return request;
}
