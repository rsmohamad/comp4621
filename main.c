#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXLINE 2048

void *serveFile(void *sock) {
  int sockfd = *(int *)sock;
  char txbuf[MAXLINE] = {0};
  sprintf(txbuf, "Hello world\n");
  write(sockfd, txbuf, strlen(txbuf));
  close(sockfd);
  usleep(1000);
  return 0;
}

int main(int argc, char **argv) {
  int listenfd, sockfd;
  struct sockaddr_in servaddr, cliaddr;
  socklen_t len = sizeof(struct sockaddr_in);

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(80);

  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr, "Error: cannot open a socket\n");
    return 1;
  }

  if (bind(listenfd, (struct sockaddr *)&servaddr, len) < 0) {
    fprintf(stderr, "Error: cannot bind to port 80\n");
    return 1;
  }

  if (listen(listenfd, 5) < 0) {
    fprintf(stderr, "Error: cannot listen on port 80\n");
    return 1;
  }

  printf("Listening on port 80 ...\n");

  while (1) {
    if ((sockfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
      fprintf(stderr, "Error: cannot accept incoming request\n");
      continue;
    }

    char ip_str[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &(cliaddr.sin_addr), ip_str, INET_ADDRSTRLEN);
    printf("Incoming connection from %s\n", ip_str);

    pthread_t tid;
    if (pthread_create(&tid, NULL, serveFile, (void *)&sockfd) < 0) {
      fprintf(stderr, "Error: cannot create thread for %s\n", ip_str);
      close(sockfd);
    }

    usleep(1000);
  }

  return 0;
}
