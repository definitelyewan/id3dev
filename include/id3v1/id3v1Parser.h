#ifndef ID3V1_PARSER
#define ID3V1_PARSER

#ifdef __cplusplus
extern "C"{
#endif

#include "id3v1Types.h"


bool id3v1HasTag(uint8_t *buffer);
Id3v1Tag *id3v1NewTag(uint8_t *title, uint8_t *artist, uint8_t *albumTitle, int year, int track, uint8_t *comment, Genre genre);
void id3v1FreeTag(Id3v1Tag *tag);
void id3v1DestroyTag(void *toDelete);


Id3v1Tag *id3v1TagFromBuffer(uint8_t *buffer);


#ifdef __cplusplus
} //extern c end
#endif

#endif