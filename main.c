#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define MAXLINE 1024

void *serveFile(void *sock){
    int sockfd = *(int*)sock;
    char txbuf[MAXLINE] = {0};
    sprintf(txbuf, "Hello world\n");
    write(sockfd, txbuf, strlen(txbuf));
    close(sockfd);
    return 0;
}

int main(int argc, char**argv) {
    int listenfd, sockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(struct sockaddr_in);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(80);

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Error: cannot open a socket\n");
        return 1;
    }

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        printf("Error: cannot bind to port 80\n");
        return 1;
    }

    if (listen(listenfd, 5) < 0) {
        printf("Error: cannot listen to port 80\n");
        return 1;
    }

    while (1) {
        if ((sockfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
            printf("Error: cannot accept incoming TCP request\n");
            return 1;
        }

        char ip_str[INET_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET, &(cliaddr.sin_addr), ip_str, INET_ADDRSTRLEN);
        printf("Incoming connection from %s\n", ip_str);
        
        pthread_t tid;
        pthread_create(&tid, NULL, serveFile, (void *)&sockfd);

    }


    return 0;
}
