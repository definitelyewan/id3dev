#ifndef ID3V1_MANIPULATE
#define ID3V1_MANIPULATE

#ifdef __cplusplus
extern "C"{
#endif

#include <stdbool.h>
#include "id3Types.h"

//functions to change values within a id3v1 structure
void id3v1SetTitle(id3buf title, Id3v1Tag *tag);
void id3v1SetArtist(id3buf artist, Id3v1Tag *tag);
void id3v1SetAlbum(id3buf album, Id3v1Tag *tag);
void id3v1SetYear(int year, Id3v1Tag *tag);
void id3v1SetComment(id3buf comment, Id3v1Tag *tag);
void id3v1SetGenre(Genre genre, Id3v1Tag *tag);
void id3v1SetTrack(int track, Id3v1Tag *tag);

//edit functions
void id3v1ClearTagInformation(Id3v1Tag *tag);
bool id3v1CompareTag(Id3v1Tag *tag1, Id3v1Tag *tag2);

//compatability functions a.k.a getters
id3buf id3v1GetTitle(Id3v1Tag *tag);
id3buf id3v1GetArtist(Id3v1Tag *tag);
id3buf id3v1GetAlbum(Id3v1Tag *tag);
int id3v1GetYear(Id3v1Tag *tag);
id3buf id3v1GetComment(Id3v1Tag *tag);
Genre id3v1GetGenre(Id3v1Tag *tag);
int id3v1GetTrack(Id3v1Tag *tag);

#ifdef __cplusplus
} //extern c end
#endif

#endif
