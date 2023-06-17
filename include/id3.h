#ifndef ID3
#define ID3

#ifdef __cplusplus
extern "C"{
#endif

#include "id3Types.h"
#include "id3v2.h"
#include "id3v1.h"
#include "id3Helpers.h"

Id3Metadata *id3NewMetadataFromFile(const char *filePath);
Id3Metadata *id3NewMetadataFromBuffer(unsigned char *buffer, int size);
Id3Metadata *id3CopyMetadata(Id3Metadata *toCopy);
Id3Metadata *id3NewMetadata(Id3v1Tag *v1, Id3v2Tag *v2);
bool hasId3v1(Id3Metadata *metadata);
bool hasId3v2(Id3Metadata *metadata);

void id3FreeMetadata(Id3Metadata *toDelete);

#ifdef __cplusplus
} //extern c end
#endif

#endif