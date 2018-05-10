#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "http_req.h"
#include "http_res.h"

#define MAXLINE 2048
#define PORT 80
#define QUEUE_LEN 10
#define TIMEOUT 15
#define MAX_REQ 100

int listenfd;

void *serveFile(void *sock) {
  int sockfd = (int)sock, served = 0;
  char txbuf[MAXLINE] = {0};

  while (served++ < MAX_REQ) {
    memset(txbuf, 0, MAXLINE);
    if (read(sockfd, txbuf, MAXLINE) <= 0)
      break;

    printf("----Request-----\n");
    printf("sockfd: %d\n", sockfd);
    printf("%s\n", txbuf);

    struct HTTPReq req;
    struct HTTPRes res;

    if (parseRequest(&req, txbuf)) {
      fprintf(stderr, "Error: bad http request\n");
      set400(&res);
      writeToSocket(&res, sockfd);
      break;
    }

    if (strcmp(req.type, "GET")) {
      fprintf(stderr, "Error: not a GET request\n");
      set501(&res);
      writeToSocket(&res, sockfd);
      break;
    }

    printf("----Response----\n");
    res.server = "comp4621";
    res.persistent = req.persistent;
    res.to = TIMEOUT;
    res.max = MAX_REQ;
    setContent(&res, req.path, req.gzip);
    writeToSocket(&res, sockfd);

    if (!req.persistent)
      break;
  }

  close(sockfd);
  return 0;
}

void terminateHandler(int num) {
  printf("\nExiting\n");
  close(listenfd);
  sleep(1);
  exit(0);
}

int main(int argc, char **argv) {
  char ip_str[INET_ADDRSTRLEN] = {0};
  struct sockaddr_in servaddr, cliaddr;
  socklen_t len = sizeof(struct sockaddr_in);
  struct timeval to;

  memset(&servaddr, 0, sizeof(servaddr));
  signal(SIGINT, terminateHandler);
  signal(SIGPIPE, SIG_IGN);
  chdir("files");
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);
  to.tv_sec = TIMEOUT;
  to.tv_usec = 0;

  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr, "Error: cannot open a socket\n");
    return 1;
  }

  if (bind(listenfd, (struct sockaddr *)&servaddr, len) < 0) {
    fprintf(stderr, "Error: cannot bind to port %d\n", PORT);
    return 1;
  }

  if (listen(listenfd, QUEUE_LEN) < 0) {
    fprintf(stderr, "Error: cannot listen on port %d\n", PORT);
    return 1;
  }

  printf("Listening on port %d ...\n", PORT);

  while (1) {
    int sockfd;
    if ((sockfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
      fprintf(stderr, "Error: cannot accept incoming request\n");
      continue;
    }

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&to, sizeof(to));
    inet_ntop(AF_INET, &(cliaddr.sin_addr), ip_str, INET_ADDRSTRLEN);
    printf("\nIncoming connection from %s\n", ip_str);

    pthread_t tid;
    if (pthread_create(&tid, NULL, serveFile, (void *)sockfd) < 0) {
      fprintf(stderr, "Error: cannot create thread for %s\n", ip_str);
      close(sockfd);
    }
    pthread_detach(tid);
  }

  return 0;
}
