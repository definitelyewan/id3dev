#ifndef ID3V2_HEADER
#define ID3V2_HEADER

#ifdef __cplusplus
extern "C"{
#endif

#include "id3Types.h"
#include <stdbool.h>

//header functions
Id3v2Header *id3v2ParseHeader(unsigned char *buffer, unsigned int bufferSize);
Id3v2Header *id3v2NewHeader(int versionMinor, int versionMajor, bool unsynchronisation, bool experimentalIndicator, bool footer, size_t size, Id3v2ExtHeader *extendedHeader);
Id3v2Header *id3v2CopyHeader(Id3v2Header *toCopy);
void id3v2FreeHeader(Id3v2Header *header);

//extended header functions
Id3v2ExtHeader *id3v2ParseExtendedHeader(unsigned char *buffer, Id3v2HeaderVersion version);
Id3v2ExtHeader *id3v2NewExtendedHeader(int size, int padding, unsigned char update, unsigned char *crc, unsigned char tagSizeRestriction, unsigned char encodingRestriction, unsigned char textSizeRestriction, unsigned char imageEncodingRestriction, unsigned char imageSizeRestriction);
Id3v2ExtHeader *id3v2CopyExtendedHeader(Id3v2ExtHeader *toCopy);
void id3v2FreeExtHeader(Id3v2ExtHeader *extHeader);

//utils
Id3v2HeaderVersion id3v2TagVersion(Id3v2Header *tag);
bool containsId3v2(unsigned char *buffer);

#ifdef __cplusplus
} //extern c end
#endif

#endif