/**
 * @file id3v2.h
 * @author Ewan Jones
 * @brief Declarations used by utility functions
 * @version 2.0
 * @date 2023-12-13
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef ID3V2
#define ID3V2

#ifdef __cplusplus
extern "C"{
#endif

#include "id3v2Types.h"
#include "id3v2TagIdentity.h" // included due to dependency on freeing memory


Id3v2Tag *id3v2TagFromFile(const char *filename);
Id3v2Tag *id3v2CopyTag(Id3v2Tag *toCopy);

// util functions

bool id3v2CompareTag(Id3v2Tag *tag1, Id3v2Tag *tag2);
Id3v2Frame *id3v2ReadFrameByID(const char id[ID3V2_FRAME_ID_MAX_SIZE], Id3v2Tag *tag);
int id3v2RemoveFrameByID(const char *id, Id3v2Tag *tag);
int id3v2InsertTextFrame(const char id[ID3V2_FRAME_ID_MAX_SIZE], const uint8_t encoding, const char *string, Id3v2Tag *tag);

int id3v2ReadTagVersion(Id3v2Tag *tag);

// compatability functions a.k.a getters

char *id3v2ReadTextFrameContent(const char id[ID3V2_FRAME_ID_MAX_SIZE], Id3v2Tag *tag);
char *id3v2ReadTitle(Id3v2Tag *tag);
char *id3v2ReadArtist(Id3v2Tag *tag);
char *id3v2ReadAlbumArtist(Id3v2Tag *tag);
char *id3v2ReadAlbum(Id3v2Tag *tag);
char *id3v2ReadYear(Id3v2Tag *tag);
char *id3v2ReadGenre(Id3v2Tag *tag);
char *id3v2ReadTrack(Id3v2Tag *tag);
char *id3v2ReadComposer(Id3v2Tag *tag);
char *id3v2ReadDisc(Id3v2Tag *tag);

char *id3v2ReadLyrics(Id3v2Tag *tag);
char *id3v2ReadComment(Id3v2Tag *tag);
uint8_t *id3v2ReadPicture(uint8_t type, Id3v2Tag *tag, size_t *dataSize);

// change values within a id3v2 structure

int id3v2WriteTextFrameContent(const char id[ID3V2_FRAME_ID_MAX_SIZE], const char *string, Id3v2Tag *tag);
int id3v2WriteTitle(const char *title, Id3v2Tag *tag);
int id3v2WriteArtist(const char *artist, Id3v2Tag *tag);
int id3v2WriteAlbumArtist(const char *albumArtist, Id3v2Tag *tag);
int id3v2WriteAlbum(const char *album, Id3v2Tag *tag);
int id3v2WriteYear(const char *year, Id3v2Tag *tag);
int id3v2WriteGenre(const char *genre, Id3v2Tag *tag);
int id3v2WriteTrack(const char *track, Id3v2Tag *tag);
int id3v2WriteDisc(const char *disc, Id3v2Tag *tag);
int id3v2WriteComposer(const char *composer, Id3v2Tag *tag);

int id3v2WriteLyrics(const char *lyrics, Id3v2Tag *tag);
int id3v2WriteComment(const char *comment, Id3v2Tag *tag);
int id3v2WritePicture(uint8_t *image, size_t imageSize, const char *kind, uint8_t type, Id3v2Tag *tag);
int id3v2WritePictureFromFile(const char *filename, const char *kind, uint8_t type, Id3v2Tag *tag);

// writes

ByteStream *id3v2TagToStream(Id3v2Tag *tag);
char *id3v2TagToJSON(Id3v2Tag *tag);
int id3v2WriteTagToFile(const char *filename, Id3v2Tag *tag);

#ifdef __cplusplus
} //extern c end
#endif

#endif
