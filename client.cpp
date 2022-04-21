#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#define MAXDATASIZE 1024 // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void cientThread()
{
    int sockfd;
    char txt[MAXDATASIZE];
    char buff[MAXDATASIZE];
    struct sockaddr_in server_addr;
    socklen_t addr_len;


    if((sockfd = socket(PF_INET, SOCK_STREAM, 0))==-1){
        perror("client: socket");
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(3490);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);

    addr_len = sizeof server_addr;
    if (connect(sockfd, (struct sockaddr *) &server_addr, addr_len) == -1){
        close(sockfd);
        perror("client: connect");
        return;
    }

    while(1) {
        strcpy(txt,"");
        fgets(txt, sizeof(txt), stdin);
        if(txt[strlen(txt)-1]=='\n') txt[strlen(txt)-1]='\0';

        if (send(sockfd, txt, strlen(txt), 0) < 0) {
            printf("fail in sending\n");
        }


        if (recv(sockfd, buff, MAXDATASIZE, 0) < 0) {
            printf("fail in recv\n");
        }

        printf("%s",buff);
    }
    close(sockfd);

}
int main(){
    cientThread();

    return 0;
}