#ifndef ID3_READER
#define ID3_READER

#ifdef __cplusplus
extern "C"{
#endif

#include "id3Types.h"

//mem functions
Id3Reader *id3NewReader(unsigned char *buffer, size_t bufferSize);
void id3FreeReader(Id3Reader *toDelete);
unsigned int id3ReaderAllocationAdd(id3byte encoding);

//move
void id3ReaderRead(Id3Reader *reader, id3buf dest, unsigned int size);
void id3ReaderSeek(Id3Reader *reader, size_t dest, const int seekOption);
size_t id3ReaderReadEncodedSize(Id3Reader *reader, id3byte encoding);
id3buf id3ReaderEncodedRemainder(Id3Reader *reader, id3byte encoding);
id3buf id3ReaderCursor(Id3Reader *reader);
int id3ReaderGetCh(Id3Reader *reader);

//unicode functions
bool hasBOM(id3buf buffer);
size_t strlenUTF16(id3buf buffer);
size_t strlenUTF16BE(id3buf buffer);
size_t strlenUTF8(id3buf buffer);
size_t id3strlen(id3buf buffer, id3byte encoding);
id3buf id3TextFormatConvert(id3buf buffer, size_t bufferLen, id3byte desiredEncoding);

//extra
void id3ReaderPrintf(Id3Reader *reader);
bool id3ValidEncoding(id3byte encoding);

#ifdef __cplusplus
} //extern c end
#endif

#endif