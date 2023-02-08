#ifndef EC_READER
#define EC_READER

#ifdef __cplusplus
extern "C"{
#endif

#include <stdbool.h>

//text encodings
#define ISO_8859_1 0x00
#define UTF16 0x01
#define UTF16BE 0x02
#define UTF8 0x03
#define UNKNOWN_ENCODING 0x04

//bom
#define UNICODE_BOM_SIZE 2

typedef struct _Id3Reader{
    unsigned char *buffer;
    size_t bufferSize;
    size_t cursor;

}Id3Reader; 

//mem functions
Id3Reader *id3NewReader(unsigned char *buffer, size_t bufferSize);
void id3FreeReader(Id3Reader *toDelete);
unsigned int id3ReaderAllocationAdd(unsigned char encoding);

//move
void id3ReaderRead(Id3Reader *reader, unsigned char *dest, unsigned int size);
void id3ReaderSeek(Id3Reader *reader, size_t dest, const int seekOption);
size_t id3ReaderReadEncodedSize(Id3Reader *reader, unsigned char encoding);
unsigned char *id3ReaderEncodedRemainder(Id3Reader *reader, unsigned char encoding);
unsigned char *id3ReaderCursor(Id3Reader *reader);
int id3ReaderGetCh(Id3Reader *reader);

//unicode functions
bool hasBOM(unsigned char *buffer);
size_t strlenUTF16(unsigned char *buffer);
size_t strlenUTF16BE(unsigned char *buffer);

//extra
void id3ReaderPrintf(Id3Reader *reader);

#ifdef __cplusplus
} //extern c end
#endif

#endif