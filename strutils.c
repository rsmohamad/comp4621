#include "strutils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **tokenize(char *text, char *delimiter) {
  size_t size = 16, ptr = 0;
  char **tokens = malloc(sizeof(char *) * size);
  char *token = strtok(text, delimiter);

  if (token == NULL) {
    free(tokens);
    return NULL;
  }

  do {
    tokens[ptr++] = token;
    if (ptr == size) {
      size += 16;
      tokens = realloc(tokens, size);
    }
  } while ((token = strtok(NULL, delimiter)));

  tokens[ptr] = NULL;
  return tokens;
}

char *getValue(char *text, char *key, char *delim) {
  char temp[128];
  sprintf(temp, "%s%s", key, delim);
  if (strstr(text, temp) == text) {
    return text + strlen(temp);
  }
  return NULL;
}

int contains(char *haystack, char *needle) {
  return strstr(haystack, needle) != NULL;
}

void cleanPath(char *path) {}
