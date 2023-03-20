#ifndef ID3V2
#define ID3V2

#ifdef __cplusplus
extern "C"{
#endif

#include "id3Types.h"
#include "id3v2Header.h"

//tag functions makers and deleters
Id3v2Tag *id3v2ParseTagFromFile(const char *filePath);
Id3v2Tag *id3v2ParseTagFromBuffer(unsigned char *buffer, int tagSize);
Id3v2Tag *id3v2CopyTag(Id3v2Tag *toCopy);
Id3v2Tag *id3v2NewTag(Id3v2Header *header, List *frames);
void id3v2FreeTag(void *toDelete);



#ifdef __cplusplus
} //extern c end
#endif

#endif