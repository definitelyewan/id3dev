/**
 * @file id3dev.h
 * @author Ewan Jones
 * @brief decleations of main library functions
 * @version 0.1
 * @date 2024-01-19
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef ID3DEV
#define ID3DEV

#ifdef __cplusplus
extern "C"{
#endif

#include "id3v1/id3v1Types.h"
#include "id3v2/id3v2Types.h"

/**
 * @brief A structure of both ID3v1 and ID3v2 tags.
 * 
 */
typedef struct _ID3{

    //! An ID3v2.2, ID3v2.3, or ID3v2.4 Tag
    Id3v2Tag *id3v2;
    //! An ID3v1 or ID3v1.1 Tag
    Id3v1Tag *id3v1;

}ID3;

// mem

ID3 *id3Create(Id3v2Tag *id3v2, Id3v1Tag *id3v1);
void id3Destroy(ID3 **toDelete);

// util functions

bool id3SetPreferedStandard(uint8_t standard);
uint8_t id3GetPreferedStandard(void);

ID3 *id3FromFile(const char *filePath);
ID3 *id3Copy(ID3 *toCopy);
bool id3Compare(ID3 *metadata1, ID3 *metadata2);
bool id3ConvertId3v1ToId3v2(ID3 *metadata);
bool id3ConvertId3v2ToId3v1(ID3 *metadata);


// compatability functions a.k.a getters

char *id3ReadTitle(ID3 *metadata);
char *id3ReadArtist(ID3 *metadata);
char *id3ReadAlbumArtist(ID3 *metadata);
char *id3ReadAlbum(ID3 *metadata);
char *id3ReadYear(ID3 *metadata);
char *id3ReadGenre(ID3 *metadata);
char *id3ReadTrack(ID3 *metadata);
char *id3ReadComposer(ID3 *metadata);
char *id3ReadDisc(ID3 *metadata);
char *id3ReadLyrics(Id3v2Tag *tag);
char *id3ReadComment(Id3v2Tag *tag);
uint8_t *id3ReadPicture(uint8_t type, ID3 *metadata, size_t *dataSize);

// change values within a id3v2 structure

int id3WriteTitle(const char *title, ID3 *metadata);
int id3WriteArtist(const char *artist, ID3 *metadata);
int id3WriteAlbumArtist(const char *albumArtist, ID3 *metadata);
int id3WriteAlbum(const char *album, ID3 *metadata);
int id3WriteYear(const char *year, ID3 *metadata);
int id3WriteGenre(const char *genre, ID3 *metadata);
int id3WriteTrack(const char *track, ID3 *metadata);
int id3WriteDisc(const char *disc, ID3 *metadata);
int id3WriteComposer(const char *composer, ID3 *metadata);

int id3WriteLyrics(const char *lyrics, ID3 *metadata);
int id3WriteComment(const char *comment, ID3 *metadata);
int id3WritePicture(uint8_t *image, size_t imageSize, const char *kind, uint8_t type, ID3 *metadata);
int id3WritePictureFromFile(const char *filename, const char *kind, uint8_t type, ID3 *metadata);

// writes

ByteStream *id3ToStream(ID3 *metadata);
char *id3ToJSON(ID3 *metadata);
int id3WriteToFile(const char *filename, ID3 *metadata);

#ifdef __cplusplus
} //extern c end
#endif

#endif