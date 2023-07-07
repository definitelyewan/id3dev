#ifndef ID3V2_WRITE
#define ID3V2_WRITE

#ifdef __cplusplus
extern "C"{
#endif

#include "id3Types.h"

//size functions
unsigned int id3v2CalculateTagSize(Id3v2Tag *tag);

//inner frame files
void id3v2WritePicture(const char *fileName, Id3v2Frame *frame);
void id3v2WriteGeneralEncapsulatedObject(Id3v2Frame *frame);

//writers
void id3v2WriteTagToFile(const char *fileName, Id3v2Tag *tag);

//tag converts
id3buf id3v2TagToBuffer(unsigned int *len, Id3v2Tag *tag);
id3buf id3v2HeaderToBuffer(unsigned int *len, unsigned int tagLength, Id3v2Header *header);
id3buf id3v2ExtendedHeaderToBuffer(unsigned int *len, Id3v2HeaderVersion version, Id3v2ExtHeader *ext);


//frame converts
id3buf id3v2FrameToBuffer(unsigned int *len, Id3v2HeaderVersion version, Id3v2Frame *frame);
id3buf id3v2FrameFlagContentToBuffer(unsigned int *len, Id3v2HeaderVersion version, Id3v2FlagContent *flags);
id3buf id3v2FrameHeaderToBuffer(unsigned int *len, Id3v2HeaderVersion version, Id3v2FrameHeader *frameHeader);

id3buf id3v2TextFrameToBuffer(unsigned int *len, Id3v2HeaderVersion version, Id3v2Frame *frame);
#ifdef __cplusplus
} //extern c end
#endif

#endif