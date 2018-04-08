#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "http_req.h"
#include "http_res.h"

#define MAXLINE 2048
#define PORT 80

int listenfd;

void *serveFile(void *sock) {
  int *sockfd = (int *)sock;
  char txbuf[MAXLINE] = {0};

  if (read(*sockfd, txbuf, MAXLINE) <= 0) {
    printf("No http request received\n");
    close(*sockfd);
    free(sockfd);
    return 0;
  }

  struct HTTPReq *req = parseRequest(txbuf);
  printf("host: %s\n", req->host);
  printf("path: %s\n", req->path);
  printf("gzip: %d\n\n", req->gzip);

  struct HTTPRes res;
  res.server = "comp4621";
  setCurrentDate(&res);
  setContent(&res, req->path, req->gzip);
  writeToSocket(&res, *sockfd);

  cleanup(&res);
  close(*sockfd);
  free(sockfd);
  free(req);
  return 0;
}

void terminateHandler(int num) {
  printf("\nExiting\n");
  close(listenfd);
  sleep(1);
  exit(0);
}

int main(int argc, char **argv) {
  signal(SIGINT, terminateHandler);

  struct sockaddr_in servaddr, cliaddr;
  socklen_t len = sizeof(struct sockaddr_in);

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);

  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr, "Error: cannot open a socket\n");
    return 1;
  }

  if (bind(listenfd, (struct sockaddr *)&servaddr, len) < 0) {
    fprintf(stderr, "Error: cannot bind to port %d\n", PORT);
    return 1;
  }

  if (listen(listenfd, 5) < 0) {
    fprintf(stderr, "Error: cannot listen on port %d\n", PORT);
    return 1;
  }

  printf("Listening on port %d ...\n", PORT);

  while (1) {
    int *sockfd = malloc(sizeof(int));
    *sockfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len);

    if (*sockfd < 0) {
      fprintf(stderr, "Error: cannot accept incoming request\n");
      continue;
    }

    char ip_str[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &(cliaddr.sin_addr), ip_str, INET_ADDRSTRLEN);
    printf("\nIncoming connection from %s\n", ip_str);

    pthread_t tid;
    if (pthread_create(&tid, NULL, serveFile, (void *)sockfd) < 0) {
      fprintf(stderr, "Error: cannot create thread for %s\n", ip_str);
      close(*sockfd);
    }
  }

  return 0;
}
