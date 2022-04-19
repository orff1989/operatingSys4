#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct node{
    char* value;
    struct node* next;
}Node;

typedef struct stack{
    Node* Stack_head;
    size_t size;
}Stack;

void push(Stack* st, char* str){
    Node* n = (Node*) malloc(sizeof(Node));
    if(!n) return;

    n->value= (char*) malloc(sizeof(char)*strlen(str)+6);
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
        str=(char*) malloc(sizeof(char)*1024);
        strcpy(str,"OUTPUT: ");
        strcat(str,st->Stack_head->value);
        return str;
    }
    else puts("ERROR: <there nothing in this stack [TOP]>");
    pthread_mutex_unlock(&mutex);
    return NULL;
}

Stack* createStack(){
    Stack* st= (Stack*) malloc(sizeof(Stack));
    st->Stack_head=NULL;
    st->size=0;
    return st;
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
    for (size_t i = n; i < strlen(str)+2; i++)
        str[k++]=str[i];
}

void gets_data(){
    Stack* st = createStack();
    char txt[1024];
    char str[1024];

    while(1){
        fgets(txt, sizeof(txt), stdin);
        if(txt[strlen(txt)-1]='\n') txt[strlen(txt)-1]='\0';

        if(strncmp(txt, "PUSH ", 5)==0){
            remove_first_n_chars(txt,5);
            push(st,txt );
        }
        else if(strncmp(txt, "POP", 3)==0){
            pop(st);
        }
        else if(strncmp(txt, "TOP", 3)==0){
            char* s = top(st);
            puts(s);
            free(s);
        }
        else if (strncmp(txt, "EXIT",4)==0) break;
        else if (strncmp(txt, "PRINTS",6)==0) printStack(st);
    }
    freeStack(st);

}


