#ifndef ID3V1_WRITE
#define ID3V1_WRITE

#ifdef __cplusplus
extern "C"{
#endif

#include "id3Types.h"

bool containsId3v1(unsigned char *buffer);
void id3v1WriteTagToFile(const char *filePath, Id3v1Tag *tag);
char *id3v1ToJSON(const Id3v1Tag *tag);


#ifdef __cplusplus
} //extern c end
#endif

#endif