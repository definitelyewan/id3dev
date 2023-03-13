#ifndef ID3V1
#define ID3V1

//built according to https://id3.org/ID3v1
#ifdef __cplusplus
extern "C"{
#endif

#include <stdbool.h>
#include "id3Types.h"
#include "id3Reader.h"
#define ID3V1_ID_LEN 3
#define ID3V1_MAX_BYTES 128
#define ID3V1_TAG_LEN 30
#define ID3V1_YEAR_LEN 4

Id3v1Tag *id3v1TagFromFile(const char* filePath);
Id3v1Tag *id3v1TagFromBuffer(unsigned char *buffer);
Id3v1Tag *id3v1CopyTag(Id3v1Tag *toCopy);
Id3v1Tag *id3v1NewTag(unsigned char *title, unsigned char *artist, unsigned char *albumTitle, int year, int trackNumber, unsigned char *comment, Genre genre);
void id3v1FreeTag(void *toDelete);
bool containsId3v1(unsigned char *buffer);

char *genreFromTable(Genre val);

/*
void setTitleId3v1(char *title, Id3v1 *tag);
void setArtistId3v1(char *artist, Id3v1 *tag);
void setAlbumId3v1(char *album, Id3v1 *tag);
void setYearId3v1(int year, Id3v1 *tag);
void setCommentId3v1(char *comment, Id3v1 *tag);
void setGenreId3v1(Genre genre, Id3v1 *tag);
void setTrackId3v1(int trackNumber, Id3v1 *tag);


void writeId3v1(char *filePath, Id3v1 *tag);


char *id3v1ToJSON(Id3v1 *tag);
*/
#ifdef __cplusplus
} //extern c end
#endif

#endif