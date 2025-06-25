/**
 * @file id3v2TagIdentity.h
 * @author Ewan Jones
 * @brief Declarations for id3v2 tag headers and extended headers
 * @version 2.0
 * @date 2023-12-13
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef ID3V2_TAG_IDENTITY
#define ID3V2_TAG_IDENTITY

#ifdef __cplusplus
extern "C"{
#endif


#include "id3v2Types.h"


/*
    Tag Header
*/

//mem functions

Id3v2TagHeader *id3v2CreateTagHeader(const uint8_t majorVersion, const uint8_t minorVersion,
                                     const uint8_t flags,
                                     Id3v2ExtendedTagHeader *extendedHeader);
void id3v2DestroyTagHeader(Id3v2TagHeader **toDelete);

//flag functions

bool id3v2WriteUnsynchronisationIndicator(Id3v2TagHeader *header, const bool bit);
bool id3v2WriteCompressionIndicator(Id3v2TagHeader *header, const bool bit);
bool id3v2WriteExtendedHeaderIndicator(Id3v2TagHeader *header, const bool bit);
bool id3v2WriteExperimentalIndicator(Id3v2TagHeader *header, const bool bit);
bool id3v2WriteFooterIndicator(Id3v2TagHeader *header, const bool bit);

int id3v2ReadUnsynchronisationIndicator(Id3v2TagHeader *header);
int id3v2ReadCompressionIndicator(Id3v2TagHeader *header);
int id3v2ReadExtendedHeaderIndicator(Id3v2TagHeader *header);
int id3v2ReadExperimentalIndicator(Id3v2TagHeader *header);
int id3v2ReadFooterIndicator(Id3v2TagHeader *header);

/*
    Extended Tag Header
*/

//mem functions

Id3v2ExtendedTagHeader *id3v2CreateExtendedTagHeader(const uint32_t padding, const uint32_t crc,
                                                     const bool update,
                                                     const bool tagRestrictions,
                                                     const uint8_t restrictions);
void id3v2DestroyExtendedTagHeader(Id3v2ExtendedTagHeader **toDelete);

//tag restriction functions

bool id3v2WriteTagSizeRestriction(Id3v2TagHeader *header, const uint8_t bits);
bool id3v2WriteTextEncodingRestriction(Id3v2TagHeader *header, const bool bit);
bool id3v2WriteTextFieldsSizeRestriction(Id3v2TagHeader *header, const uint8_t bits);
bool id3v2WriteImageEncodingRestriction(Id3v2TagHeader *header, const bool bit);
bool id3v2WriteImageSizeRestriction(Id3v2TagHeader *header, const uint8_t bits);

int id3v2ReadTagSizeRestriction(Id3v2TagHeader *header);
int id3v2ReadTextEncodingRestriction(Id3v2TagHeader *header);
int id3v2ReadTextFieldsSizeRestriction(Id3v2TagHeader *header);
int id3v2ReadImageEncodingRestriction(Id3v2TagHeader *header);
int id3v2ReadImageSizeRestriction(Id3v2TagHeader *header);

bool id3v2ClearTagRestrictions(Id3v2TagHeader *header);

/*
    Tag
*/

//mem

Id3v2Tag *id3v2CreateTag(Id3v2TagHeader *header, List *frames);
void id3v2DestroyTag(Id3v2Tag **toDelete);


/*
    output
*/

uint8_t *id3v2ExtendedTagHeaderSerialize(Id3v2ExtendedTagHeader *ext, const uint8_t version,
                                         size_t *outLength);
char *id3v2ExtendedTagHeaderToJSON(const Id3v2ExtendedTagHeader *ext, const uint8_t version);

uint8_t *id3v2TagHeaderSerialize(Id3v2TagHeader *header, const uint32_t uintSize,
                                 size_t *outLength);
char *id3v2TagHeaderToJSON(const Id3v2TagHeader *header);


#ifdef __cplusplus
} //extern c end
#endif

#endif