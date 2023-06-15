#ifndef ID3V1_MANIPULATE
#define ID3V1_MANIPULATE

#ifdef __cplusplus
extern "C"{
#endif

#include "id3Types.h"

//functions to change values within a id3v1 structure
void id3v1WriteTitle(id3buf title, Id3v1Tag *tag);
void id3v1WriteArtist(id3buf artist, Id3v1Tag *tag);
void id3v1WriteAlbum(id3buf album, Id3v1Tag *tag);
void id3v1WriteYear(int year, Id3v1Tag *tag);
void id3v1WriteComment(id3buf comment, Id3v1Tag *tag);
void id3v1WriteGenre(Genre genre, Id3v1Tag *tag);
void id3v1WriteTrack(int track, Id3v1Tag *tag);

//util functions
void id3v1ClearTagInformation(Id3v1Tag *tag);
bool id3v1CompareTag(Id3v1Tag *tag1, Id3v1Tag *tag2);

//compatability functions a.k.a getters
id3buf id3v1ReadTitle(Id3v1Tag *tag);
id3buf id3v1ReadArtist(Id3v1Tag *tag);
id3buf id3v1ReadAlbum(Id3v1Tag *tag);
int id3v1ReadYear(Id3v1Tag *tag);
id3buf id3v1ReadComment(Id3v1Tag *tag);
Genre id3v1ReadGenre(Id3v1Tag *tag);
int id3v1ReadTrack(Id3v1Tag *tag);

#ifdef __cplusplus
} //extern c end
#endif

#endif
