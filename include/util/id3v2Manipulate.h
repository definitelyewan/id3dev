#ifndef ID3V2_MANIPULATE
#define ID3V2_MANIPULATE

#ifdef __cplusplus
extern "C"{
#endif

#include <stdbool.h>
#include "id3Types.h"


//tag header should not be touched probably a bad idea to edit that


//frame header functions


//flag functions

//frame content functions

//util
void id3v2SavePicture(const char *fileName, Id3v2Frame *frame);
void id3v2SaveEncapsulatedObject(Id3v2Frame *frame);

//compatability functions a.k.a getters


int id3v2GetTagSizeRestriction(Id3v2Tag *tag);
int id3v2GetEncodingRestriction(Id3v2Tag *tag);
int id3v2GetTextSizeRestriction(Id3v2Tag *tag);
int id3v2GetImageEncodingRestriction(Id3v2Tag *tag);
int id3v2GetImageSizeRestriction(Id3v2Tag *tag);
unsigned char *id3v2GetCrc(Id3v2Tag *tag);

int id3v2GetVersion(Id3v2Tag *tag);
bool id3v2UnsynchronizedIndicator(Id3v2Tag *tag);
bool id3v2ExperimentalIndicator(Id3v2Tag *tag);
bool id3v2ExtendedIndicator(Id3v2Tag *tag);
bool id3v2FooterIndicator(Id3v2Tag *tag);
size_t id3v2GetTagSize(Id3v2Tag *tag);

bool id3v2FrameAlterPreservationIndicator(Id3v2Frame *frame);
bool id3v2FrameFileAlterPreservationIndicator(Id3v2Frame *frame);
bool id3v2FrameReadOnlyIndicator(Id3v2Frame *frame);
bool id3v2FrameUnsynchronizationIndicator(Id3v2Frame *frame);
size_t id3v2GetFrameDataLengthSize(Id3v2Frame *frame);
unsigned char id3v2GetFrameEncryptionMethod(Id3v2Frame *frame);
unsigned char id3v2GetFrameGroup(Id3v2Frame *frame);

char *id3v2GetFrameStrID(Id3v2Frame *frame);
size_t id3v2GetFrameSize(Id3v2Frame *frame);
size_t id3v2GetFrameHeaderSize(Id3v2Frame *frame);
Id3v2FrameId id3v2GetFrameID(Id3v2Frame *frame);

int id3v2GetEncoding(Id3v2Frame *frame);
unsigned char *id3v2GetDescription(Id3v2Frame *frame);
unsigned char *id3v2GetTextValue(Id3v2Frame *frame);
unsigned char *id3v2GetURLValue(Id3v2Frame *frame);
unsigned char *id3v2GetInvolvedPeopleListValue(Id3v2Frame *frame);
unsigned char *id3v2GetCDIDValue(Id3v2Frame *frame);
int id3v2GetTimeStampFormat(Id3v2Frame *frame);
unsigned char id3v2GetEventTimeCodeType(Id3v2Frame *frame);
long id3v2GetEventTimeCodeTimeStamp(Id3v2Frame *frame);
void id3v2ResetEventTimeCodeIter(Id3v2Frame *frame);
unsigned char *id3v2GetSyncedTempoCodesFrameValue(Id3v2Frame *frame);
unsigned char *id3v2GetLanguage(Id3v2Frame *frame);
unsigned char *id3v2GetUnsynchronizedLyrics(Id3v2Frame *frame);
int id3v2GetSynchronizedLyricsContentType(Id3v2Frame *frame);
unsigned char *id3v2GetSynchronizedLyricsValue(Id3v2Frame *frame);
long id3v2GetSynchronizedLyricsTimeStamp(Id3v2Frame *frame);
void id3v2ResetSynchronizedLyricsIter(Id3v2Frame *frame);
unsigned char *id3v2GetCommentValue(Id3v2Frame *frame);
unsigned char *id3v2GetSubjectiveValue(Id3v2Frame *frame);
unsigned char *id3v2GetRelativeVolumeAdjustmentValue(Id3v2Frame *frame);
unsigned char *id3v2GetEqualisationValue(Id3v2Frame *frame);
unsigned char *id3v2GetReverbValue(Id3v2Frame *frame);
unsigned char *id3v2GetMIMEType(Id3v2Frame *frame);
int id3v2GetPictureType(Id3v2Frame *frame);
unsigned char *id3v2GetPictureValue(Id3v2Frame *frame);
unsigned char *id3v2GetObjectFileName(Id3v2Frame *frame);
unsigned char *id3v2GetGeneralEncapsulatedObjectValue(Id3v2Frame *frame);
int id3v2GetPlayCount(Id3v2Frame *frame);
unsigned char *id3v2GetEmail(Id3v2Frame *frame);
int id3v2GetRating(Id3v2Frame *frame);
unsigned char *id3v2GetOwnerIdentifier(Id3v2Frame *frame);
unsigned char *id3v2GetEncryptedMetaValue(Id3v2Frame *frame);
unsigned char *id3v2GetPreviewStart(Id3v2Frame *frame);
int id3v2GetPreviewLength(Id3v2Frame *frame);
unsigned char *id3v2GetAudioEncryptionValue(Id3v2Frame *frame);
unsigned char *id3v2GetUniqueFileIdentifierValue(Id3v2Frame *frame);
long id3v2GetPositionSynchronisationValue(Id3v2Frame *frame);
unsigned char *id3v2GetTermsOfUseValue(Id3v2Frame *frame);
/*
//functions to change values within a id3v1 structure
void id3v1SetTitle(char *title, Id3v1Tag *tag);
void id3v1SetArtist(char *artist, Id3v1Tag *tag);
void id3v1SetAlbum(char *album, Id3v1Tag *tag);
void id3v1SetYear(int year, Id3v1Tag *tag);
void id3v1SetComment(char *comment, Id3v1Tag *tag);
void id3v1SetGenre(Genre genre, Id3v1Tag *tag);
void id3v1SetTrack(int track, Id3v1Tag *tag);

//edit functions
void id3v1ClearTagInformation(Id3v1Tag *tag);
bool id3v1CompareTag(Id3v1Tag *tag1, Id3v1Tag *tag2);

//compatability functions a.k.a getters
char *id3v1GetTitle(Id3v1Tag *tag);
char *id3v1GetArtist(Id3v1Tag *tag);
char *id3v1GetAlbum(Id3v1Tag *tag);
int id3v1GetYear(Id3v1Tag *tag);
char *id3v1GetComment(Id3v1Tag *tag);
Genre id3v1GetGenre(Id3v1Tag *tag);
int id3v1GetTrack(Id3v1Tag *tag);
*/
#ifdef __cplusplus
} //extern c end
#endif

#endif
