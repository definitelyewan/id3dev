/**
 * @file id3v1.h
 * @author Ewan Jones
 * @brief Declarations used by utility functions
 * @version 2.0
 * @date 2023-10-03
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef ID3V1
#define ID3V1

#ifdef __cplusplus
extern "C"{
#endif

#include "id3v1Types.h"
#include "id3v1Parser.h"

Id3v1Tag *id3v1TagFromFile(const char *filePath);
Id3v1Tag *id3v1CopyTag(Id3v1Tag *toCopy);


//functions to change values within a id3v1 structure

int id3v1WriteTitle(char *title, Id3v1Tag *tag);
int id3v1WriteArtist(char *artist, Id3v1Tag *tag);
int id3v1WriteAlbum(char *album, Id3v1Tag *tag);
int id3v1WriteYear(int year, Id3v1Tag *tag);
int id3v1WriteComment(char *comment, Id3v1Tag *tag);
int id3v1WriteGenre(Genre genre, Id3v1Tag *tag);
int id3v1WriteTrack(int track, Id3v1Tag *tag);

//util functions

bool id3v1CompareTag(Id3v1Tag *tag1, Id3v1Tag *tag2);
char *id3v1GenreFromTable(Genre val);

//compatability functions a.k.a getters

char *id3v1ReadTitle(Id3v1Tag *tag);
char *id3v1ReadArtist(Id3v1Tag *tag);
char *id3v1ReadAlbum(Id3v1Tag *tag);
int id3v1ReadYear(Id3v1Tag *tag);
char *id3v1ReadComment(Id3v1Tag *tag);
Genre id3v1ReadGenre(Id3v1Tag *tag);
int id3v1ReadTrack(Id3v1Tag *tag);

//writes

char *id3v1ToJSON(const Id3v1Tag *tag);
int id3v1WriteTagToFile(const char *filePath, Id3v1Tag *tag);

#ifdef __cplusplus
} //extern c end
#endif

#endif