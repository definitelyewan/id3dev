#ifndef ID3V2_HELPERS
#define ID3V2_HELPERS

#ifdef __cplusplus
extern "C"{
#endif

#include <stdbool.h>

typedef struct _List{
    int size;
    void (*deleteData)(void* toDelete);
    struct _node *head;
}List;

typedef struct _node{
    void *data;
    struct _node *next;
}Node;

//byte functions
int getBits8(unsigned char *bytes, int byteNum);
unsigned int syncint_decode(int value);

//list functions
List *newList(void (*deleteFunction)(void* toDelete));
Node *newNode(void *data);
void listPush(List *list, void *toAdd);
void *listRemove(List *list, int pos);
void freeList(List *list);
void destroyList(List *list);

#ifdef __cplusplus
} //extern c end
#endif

#endif