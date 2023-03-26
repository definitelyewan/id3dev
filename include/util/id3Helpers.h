#ifndef ID3V2_HELPERS
#define ID3V2_HELPERS

#ifdef __cplusplus
extern "C"{
#endif

#include "id3Types.h"
#include <stdbool.h>


//byte functions
int getBits8(unsigned char *bytes, int byteNum);
unsigned int syncint_decode(int value);
void addressFree(void **pptr);
char *integerToCharPointer(int value);


//list functions
List *newList(void (*deleteFunction)(void* toDelete), void *(*copyFunction)(void* toCopy));
List *copyList(List *list);
Node *newNode(void *data);
void listPush(List *list, void *toAdd);
void *listRemove(List *list, int pos);
void freeList(List *list);
void destroyList(List *list);

ListIter *newListIter(List *list);
ListIter *copyListIter(ListIter *li);
void *nextListIter(ListIter *li);
bool hasNextListIter(ListIter *li);
void freeListIter(ListIter *li);

#ifdef __cplusplus
} //extern c end
#endif

#endif