#ifndef ID3V1
#define ID3V1

//built according to https://id3.org/ID3v1
#ifdef __cplusplus
extern "C"{
#endif

#include <stdbool.h>
#include "id3Types.h"
#include "id3Reader.h"
#include "id3v1Write.h"
#include "id3v1Manipulate.h"

Id3v1Tag *id3v1TagFromFile(const char* filePath);
Id3v1Tag *id3v1TagFromBuffer(unsigned char *buffer);
Id3v1Tag *id3v1CopyTag(Id3v1Tag *toCopy);
Id3v1Tag *id3v1NewTag(id3buf title, id3buf artist, id3buf albumTitle, int year, int trackNumber, id3buf comment, Genre genre);
void id3v1FreeTag(void *toDelete);

char *genreFromTable(Genre val);

#ifdef __cplusplus
} //extern c end
#endif

#endif