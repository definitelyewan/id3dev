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

Id3v1Tag *id3v1TagFromFile(const char *filePath);
Id3v1Tag *id3v1CopyTag(Id3v1Tag *toCopy);


// Functions to change values within an id3v1 structure

int id3v1WriteTitle(const char *title, Id3v1Tag *tag);
int id3v1WriteArtist(const char *artist, Id3v1Tag *tag);
int id3v1WriteAlbum(const char *album, Id3v1Tag *tag);
int id3v1WriteYear(int year, Id3v1Tag *tag);
int id3v1WriteComment(const char *comment, Id3v1Tag *tag);
int id3v1WriteGenre(Genre genre, Id3v1Tag *tag);
int id3v1WriteTrack(int track, Id3v1Tag *tag);

// Util functions
bool id3v1CompareTag(const Id3v1Tag *tag1, const Id3v1Tag *tag2);
char *id3v1GenreFromTable(Genre val);

// Compatability functions a.k.a getters

char *id3v1ReadTitle(const Id3v1Tag *tag);
char *id3v1ReadArtist(const Id3v1Tag *tag);
char *id3v1ReadAlbum(const Id3v1Tag *tag);
int id3v1ReadYear(const Id3v1Tag *tag);
char *id3v1ReadComment(const Id3v1Tag *tag);
Genre id3v1ReadGenre(const Id3v1Tag *tag);
int id3v1ReadTrack(const Id3v1Tag *tag);

// Writes

char *id3v1ToJSON(const Id3v1Tag *tag);
int id3v1WriteTagToFile(const char *filePath, const Id3v1Tag *tag);

#ifdef __cplusplus
} //extern c end
#endif

#endif