#ifndef _LIST_H_
#define _LIST_H_
#include <pthread.h>

#define MAX_LEN 80

typedef struct Node{
    struct Node *next;
    char *str;
    pthread_mutex_t mtx;
} Node;

typedef struct List{
    Node *head;
} List;

int createList(List *list);
Node *createNode();
int showList(List *list);
int addElement(List *list, const char *str);
int destroyList(List *list);
int sortList(List *list);

#endif
