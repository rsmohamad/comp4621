#include <stdlib.h>

#include "http_req.h"
#include "strutils.h"

/*
 * Returns NULL if there is a parsing error
 */
struct HTTPReq *parseRequest(char *buf) {
  char **lines = tokenize(buf, "\r\n");
  char **_lines = lines;

  if (lines == NULL)
    return NULL;

  // Get the path
  char **firstline = tokenize(*lines++, " ");
  if (firstline == NULL || firstline[1] == NULL)
    return NULL;

  struct HTTPReq *request = malloc(sizeof(struct HTTPReq));
  request->path = firstline[1];
  request->host = "";
  request->gzip = 0;
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
