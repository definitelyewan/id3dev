#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "id3Helpers.h"

int getBits8(unsigned char *bytes, int byteNum){
    
    int byteAsInt = 0x00;
    for(int i = 0; i < byteNum; i++){
        byteAsInt = byteAsInt << 8;
        byteAsInt = byteAsInt | (unsigned char) bytes[i];
    }

    return byteAsInt;	  
}

unsigned int syncint_decode(int value){
    unsigned int a, b, c, d, result = 0x00;
    a = value & 0xFF;
    b = (value >> 8) & 0xFF;
    c = (value >> 16) & 0xFF;
    d = (value >> 24) & 0xFF;

    result = result | a;
    result = result | (b << 7);
    result = result | (c << 14);
    result = result | (d << 21);

    return result;
}

char *integerToCharPointer(int value){

    int n = log10(value) + 1;
    char *numberArray = calloc(n, sizeof(char));

    for(int i = n-1; i >= 0; --i, value /= 10){
        numberArray[i] = (value % 10) + '0';
    }

    return numberArray;
}

void addressFree(void **pptr){
    if(pptr && *pptr){
        free(*pptr);
        *pptr = NULL;
        pptr = NULL;
    }
}

List *newList(void (*deleteFunction)(void* toDelete), void *(*copyFunction)(void* toCopy)){

    List *list = malloc(sizeof(List));

    list->head = NULL;
    list->size = 0;
    list->deleteData = deleteFunction;
    list->copyData = copyFunction;

    return list;
}

Node *newNode(void *data){

    Node *node = malloc(sizeof(Node));

    node->data = data;
    node->next = NULL;

    return node;
}

void listPush(List *list, void *toAdd){

    if(list == NULL || toAdd == NULL){
        return;
    }
    Node *node = newNode(toAdd);
    
    if(list->size == 0){
        list->head = node;
    }else{
        Node *tmp = list->head;
        node->next = tmp;
        list->head = node;
    }

    list->size = list->size + 1;
}

void freeList(List *list){
    
    if(list == NULL){
        return;
    }

    if(list->deleteData == NULL){
        return;
    }

    Node *curr = list->head;
    for(int i = 0; i < list->size; i++){
        if(curr == NULL){
            break;
        }

        Node *tmp = curr;
        curr = tmp->next;
        list->deleteData(tmp->data);
        free(tmp);

    }

    list->size = 0;
}

void destroyList(List *list){
    
    if(list == NULL){
        return;
    }

    if(list->deleteData == NULL){
        return;
    }

    freeList(list);
    free(list);
}

void *listRemove(List *list, int pos){
	
    if(list == NULL){
        return NULL;
    }
    
    if(pos <= 0 || pos > list->size){
        return NULL;
    }

    Node *temp = list->head;
    Node *prev = list->head;
    void *toReturn;
    for(int i = 0; i < pos; i++) {
        if(i == 0 && pos == 1) {
            toReturn = list->head->data;
            list->head = list->head->next;
            free(temp);
            return toReturn;
        }else{
            if(i == pos - 1 && temp) {
                toReturn = temp->data;
                prev->next = temp->next;
                free(temp);
            }else {
                prev = temp;

                if(prev == NULL){
                    break;
                }
                    
                temp = temp->next;
            }
        }
    }   
    return NULL;
}

List *copyList(List *list){

    if(list == NULL){
        return NULL;
    }

    if(list->copyData == NULL){
        return NULL;
    }

    List *ret = newList(list->deleteData, list->copyData);
    Node *n = list->head;

    while(n != NULL){
        listPush(ret, ret->copyData(n->data));
        n = n->next;
    }

    return ret;
}

ListIter *newListIter(List *list){

    if(list == NULL){
        return NULL;
    }

    ListIter *li = malloc(sizeof(ListIter));

    li->l = list;
    li->curr = list->head;

    return li;
}
ListIter *copyListIter(ListIter *li){

    if(li == NULL){
        return NULL;
    }

    ListIter *liCopy = malloc(sizeof(ListIter)); 

    liCopy->l = li->l;
    liCopy->curr = li->curr;

    return liCopy;

}
void *nextListIter(ListIter *li){
    
    if(li == NULL){
        return NULL;
    }
    
    void *data = NULL;

    if(li->curr){
        data = li->curr->data;
        li->curr = li->curr->next;
    }

    return data;
}


bool hasNextListIter(ListIter *li){
    return (li->curr != NULL) ? true : false;

}
void freeListIter(ListIter *li){
    if(li != NULL){
        free(li);
    }
}