#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include<pthread.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

// got some help from: https://dzone.com/articles/parallel-tcpip-socket-server-with-multi-threading

#define PORT "3490"  // the port users will be connecting to
#define BACKLOG 10   // how many pending connections queue will hold
typedef struct node{
    char* value;
    struct node* next;
}Node;

typedef struct stack{
    Node* Stack_head;
    size_t size;
}Stack;

Stack* createStack(){
    Stack* st= (Stack*) calloc(sizeof(Stack),1);
    st->Stack_head=NULL;
    st->size=0;
    return st;
}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
Stack* st = createStack();

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}



void push(Stack* st, char* str){
    Node* n = (Node*) calloc(sizeof(Node),1);
    if(!n) return;

    n->value= (char*) calloc(sizeof(char),strlen(str)+1);
    if(!n->value) return;

    strcpy(n->value,str);

    pthread_mutex_unlock(&mutex);
    n->next= st->Stack_head;
    st->Stack_head= n;
    st->size++;
    pthread_mutex_unlock(&mutex);
}

void pop(Stack* st){
    pthread_mutex_unlock(&mutex);
    Node* n = st->Stack_head;
    if(st->size>0){
        st->Stack_head=n->next;
        free(n);
        st->size--;
    }
    else puts("ERROR: <there nothing in this stack [POP]>");
    pthread_mutex_unlock(&mutex);
}

char* top(Stack* st){
    char* str;
    pthread_mutex_unlock(&mutex);
    if (st->size>0){
        str=(char*) calloc(sizeof(char),1024);
        strcpy(str,"OUTPUT: ");
        strcat(str,st->Stack_head->value);
        return str;
    }
    else puts("ERROR: <there nothing in this stack [TOP]>");
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void printStack(Stack* st){
    Node* n = st->Stack_head;
    while (st->size>0 && n!=NULL) {
        printf("DEBUG: %s\n",n->value);
        n=n->next;
    }
}

void freeStack(Stack* st){
    while (st->size>0) pop(st);
    free(st);
}

void remove_first_n_chars(char* str, int n){
    int k=0;
    for (size_t i = n; i < strlen(str)+1; i++)
        str[k++]=str[i];
}

void * sendMSG(void *arg)
{
    int newSocket = *((int *)arg);
    char txt[1024];

    while(1){
        strcpy(txt,"");
        recv(newSocket , txt , 1024 , 0);


        if(strncmp(txt, "PUSH ", 5)==0){
            remove_first_n_chars(txt,5);
            push(st,txt );
        }
        else if(strncmp(txt, "POP", 3)==0){
            pop(st);
        }
        else if(strncmp(txt, "TOP", 3)==0){
            char* s = top(st);
            if(s) {
                puts(s);
                free(s);
            }
        }
        else if (strncmp(txt, "EXIT",4)==0) break;
        else if (strncmp(txt, "PRINTS",6)==0) printStack(st);

        if (send(newSocket, "Hello, world!", 13, 0) == -1)
            perror("send");
    }
    freeStack(st);
    close(newSocket);
    pthread_exit(NULL);
}

int main(){
    int sockfd, new_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    int i = 0;
    int j=0, yes=1;
    pthread_t thread_arr[80];

    if((sockfd = socket(PF_INET, SOCK_STREAM, 0))==-1){
        perror("server: socket");
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(3490);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);

    if(bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr))==-1){
        close(sockfd);
        perror("server: bind");
        return -1;
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1){ // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        printf("server: got new connection\n");

        if(pthread_create(&thread_arr[i++], NULL, sendMSG, &new_fd) != 0 )
            printf("Failed to create thread\n");

        if(i > 50){
            j = 0;
            i = 0;
            while(j < 50){
                pthread_join(thread_arr[j++], NULL);
            }
        }
    }
    return 0;
}