#define _POSIX_C_SOURCE 200809L
#include "list.h"
#include "service.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int createList(List *list){
    Node *head = createNode();
    if (head == NULL){
        return ENOMEM;
    }

    list->head = head;
    initMutexSuccessAssertion(&list->head->mtx, NULL, "createList");

    return SUCCESS_CODE;
}

Node *createNode(){
    Node *res = (Node*)malloc(sizeof(Node));
    if (res == NULL){
        errno = ENOMEM;
        return NULL;
    }

    res->next = NULL;
    res->str[0] = '\0';
    initMutexSuccessAssertion(&res->mtx, NULL, "createNode");

    return res;
}

int addElement(List *list, const char *str){
    if (list == NULL || str == NULL)
        return EINVAL;

    Node *new_el = createNode();
    if (new_el == NULL)
        return ENOMEM;

    size_t len = strnlen(str, MAX_LEN);
    char *new_el_data = (char*)malloc(len * sizeof(char));
    if (new_el_data == NULL)
        return ENOMEM;
    strcpy(new_el_data, str);

    //need to lock only list head
    lockSuccessAssertion(&list->head->mtx, "addElement");

    Node *head = list->head;

    strncpy(new_el->str, str, MAX_LEN + 1);
    new_el->str[MAX_LEN] = '\0';

    Node *prev = head->next;

    head->next = new_el;
    new_el->next = prev;

    unlockSuccessAssertion(&list->head->mtx, "addElement");
    return SUCCESS_CODE;
}

int showList(List *list){
    if (list == NULL){
        return EINVAL;
    }

    lockSuccessAssertion(&list->head->mtx, "showList");
    Node *node = list->head->next;
    while (node != NULL){
        printf("%s\n", node->str);
        node = node->next;
    }
    unlockSuccessAssertion(&list->head->mtx, "showList");
    
    return SUCCESS_CODE;
}

int destroyList(List *list){
    if (list == NULL){
        return EINVAL;
    }
    
    if (list->head == NULL)
        return SUCCESS_CODE;

    //free data
    Node *next;
    Node *head = list->head;

    while (head != NULL){
        lockSuccessAssertion(&head->mtx, "destroyList");
        next = head->next;
        unlockSuccessAssertion(&head->mtx, "destroyList");
        //destroy mtx
        int err = pthread_mutex_destroy(&head->mtx);
        assertSuccess("destroySuccess", err);
        
        free(head);
        
        head = next;
    }

    return SUCCESS_CODE;
}

static void swap(Node **a, Node **b){
    if (a == NULL || b == NULL){
        errno = EINVAL;
        return;
    }

    Node *tmp = *a;
    *a = *b;
    *b = tmp;
}

int sortList(List *list){
    if (list == NULL){
        return EINVAL;
    }

    lockSuccessAssertion(&list->head->mtx, "sortList");
    char sorted = list->head->next == NULL ? TRUE : FALSE;

    while (sorted == FALSE){
        sorted = TRUE;

        Node *curr = list->head->next;
        Node *prev_curr = list->head;
        lockSuccessAssertion(&curr->mtx, "sortList");
        Node *follow = curr->next;
        while (follow != NULL){
            lockSuccessAssertion(&follow->mtx, "sortList");
            int cmp = strcmp(curr->str, follow->str);
            if (cmp > 0){
                sorted = FALSE;
                prev_curr->next = follow;
                curr->next = follow->next;
                follow->next = curr;

                swap(&curr, &follow);
            }
            
            pthread_mutex_t  *old_prev_curr_mtx;
            old_prev_curr_mtx = &prev_curr->mtx;

            follow = follow->next;
            curr = curr->next;
            prev_curr = prev_curr->next;
            
            unlockSuccessAssertion(old_prev_curr_mtx, "sortList");
        }

        unlockSuccessAssertion(&curr->mtx, "sortList");
        unlockSuccessAssertion(&prev_curr->mtx, "sortList");

        lockSuccessAssertion(&list->head->mtx, "sortList");
    }

    unlockSuccessAssertion(&list->head->mtx, "sortList");

    return SUCCESS_CODE;
}
