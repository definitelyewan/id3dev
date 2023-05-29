#ifndef ID3V2_HELPERS
#define ID3V2_HELPERS

#ifdef __cplusplus
extern "C"{
#endif

#include "id3Types.h"
#include <stdint.h>


//byte functions
int getBits8(unsigned char *bytes, int byteNum);
unsigned int syncint_decode(int value);
void addressFree(void **pptr);
char *integerToCharPointer(int value);

//unicode helpers
unsigned char *utf16ToUtf8(unsigned char *src, size_t srcSize);
unsigned char *utf8ToUtf16(unsigned char *src, size_t srcSize, unsigned int utfv);
bool isISO_8859_1(const unsigned char *str);
bool isUTF16(const unsigned char *str, size_t length);
bool isUTF16BE(const unsigned char *str, size_t length);
bool isUTF8(const unsigned char *str);

//list functions
Id3List *id3NewList(void (*deleteFunction)(void* toDelete), void *(*copyFunction)(void* toCopy));
Id3List *id3CopyList(Id3List *list);
Id3Node *id3NewNode(void *data);
void id3PushList(Id3List *list, void *toAdd);
void *id3RemoveList(Id3List *list, int pos);
void id3FreeList(Id3List *list);
void id3DestroyList(Id3List *list);

Id3ListIter *id3NewListIter(Id3List *list);
Id3ListIter *id3CopyListIter(Id3ListIter *li);
void *id3NextListIter(Id3ListIter *li);
bool id3HasNextListIter(Id3ListIter *li);
void id3FreeListIter(Id3ListIter *li);

#ifdef __cplusplus
} //extern c end
#endif

#endif