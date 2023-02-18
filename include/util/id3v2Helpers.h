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