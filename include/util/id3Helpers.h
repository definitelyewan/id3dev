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
Id3List *id3NewList(void (*deleteFunction)(void* toDelete), void *(*copyFunction)(void* toCopy));
Id3List *id3CopyList(Id3List *list);
Id3Node *id3NewNode(void *data);
void id3PushList(Id3List *list, void *toAdd);
void *id3RemoveList(Id3List *list, int pos);
void id3FreeList(Id3List *list);
void id3DestroyList(Id3List *list);

ListIter *id3NewListIter(Id3List *list);
ListIter *id3CopyListIter(ListIter *li);
void *id3NextListIter(ListIter *li);
bool id3HasNextListIter(ListIter *li);
void id3FreeListIter(ListIter *li);

#ifdef __cplusplus
} //extern c end
#endif

#endif