#ifndef ID3V2_HEADER
#define ID3V2_HEADER

#ifdef __cplusplus
extern "C"{
#endif

#include "id3Types.h"
#include <stdbool.h>

//header functions
Id3v2Header *id3v2ParseHeader(id3buf buffer, unsigned int bufferSize);
Id3v2Header *id3v2NewHeader(int versionMinor, int versionMajor, bool unsynchronisation, bool experimentalIndicator, bool footer, Id3v2ExtHeader *extendedHeader);
Id3v2Header *id3v2CopyHeader(Id3v2Header *toCopy);
void id3v2FreeHeader(Id3v2Header *header);

//extended header functions
Id3v2ExtHeader *id3v2ParseExtendedHeader(id3buf buffer, Id3v2HeaderVersion version);
Id3v2ExtHeader *id3v2NewExtHeader(int size, int padding, id3byte update, id3buf crc, id3byte tagSizeRestriction, id3byte encodingRestriction, id3byte textSizeRestriction, id3byte imageEncodingRestriction, id3byte imageSizeRestriction);
Id3v2ExtHeader *id3v2CopyExtendedHeader(Id3v2ExtHeader *toCopy);
void id3v2FreeExtHeader(Id3v2ExtHeader *extHeader);

//utils
Id3v2HeaderVersion id3v2TagVersion(Id3v2Header *tag);
bool containsId3v2(id3buf buffer);
bool id3v2IsValidVersion(int version);

#ifdef __cplusplus
} //extern c end
#endif

#endif