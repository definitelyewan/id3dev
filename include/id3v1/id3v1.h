#ifndef ID3V1
#define ID3V1

#ifdef __cplusplus
extern "C"{
#endif

#include "id3v1Types.h"
#include "id3v1Parser.h"

Id3v1Tag *id3v1TagFromFile(const char *filePath);
Id3v1Tag *id3v1CopyTag(Id3v1Tag *toCopy);

void id3v1WriteTagToFile(const char *filePath, Id3v1Tag *tag);
char *id3v1ToJSON(const Id3v1Tag *tag);

//functions to change values within a id3v1 structure
void id3v1WriteTitle(uint8_t *title, Id3v1Tag *tag);
void id3v1WriteArtist(uint8_t *artist, Id3v1Tag *tag);
void id3v1WriteAlbum(uint8_t *album, Id3v1Tag *tag);
void id3v1WriteYear(int year, Id3v1Tag *tag);
void id3v1WriteComment(uint8_t *comment, Id3v1Tag *tag);
void id3v1WriteGenre(Genre genre, Id3v1Tag *tag);
void id3v1WriteTrack(int track, Id3v1Tag *tag);

//util functions
bool id3v1CompareTag(Id3v1Tag *tag1, Id3v1Tag *tag2);

//compatability functions a.k.a getters
char *id3v1ReadTitle(Id3v1Tag *tag);
char *id3v1ReadArtist(Id3v1Tag *tag);
char *id3v1ReadAlbum(Id3v1Tag *tag);
int id3v1ReadYear(Id3v1Tag *tag);
char *id3v1ReadComment(Id3v1Tag *tag);
Genre id3v1ReadGenre(Id3v1Tag *tag);
int id3v1ReadTrack(Id3v1Tag *tag);

#ifdef __cplusplus
} //extern c end
#endif

#endif