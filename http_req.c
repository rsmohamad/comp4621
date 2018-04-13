#include <stdlib.h>

#include "http_req.h"
#include "strutils.h"

/*
 * Returns -1 if there is a parsing error
 */
int parseRequest(struct HTTPReq *request, char *buf) {
  char **lines = tokenize(buf, "\r\n");
  char **_lines = lines;

  if (lines == NULL)
    return -1;

  // Get the path
  char **firstline = tokenize(*lines++, " ");
  if (firstline == NULL || firstline[1] == NULL) {
    free(_lines);
    return -1;
  }

  request->path = firstline[1];
  request->host = "";
  request->gzip = 0;
  request->persistent = 1;
  free(firstline);

  // Get the fields
  char *current;
  while ((current = *lines++)) {
    char *value;
    if ((value = getValue(current, "Host", ": ")))
      request->host = value;
    else if ((value = getValue(current, "Accept-Encoding", ": ")))
      request->gzip = contains(value, "gzip");
    else if ((value = getValue(current, "Connection", ": ")))
      request->persistent = !contains(value, "close");
  }

  free(_lines);
  return 0;
}
