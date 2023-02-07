#ifndef ID3V2_HEADER
#define ID3V2_HEADER

#ifdef __cplusplus
extern "C"{
#endif

#define ID3V2_HEADER_SIZE_LEN 4
#define ID3V2_CRC_LEN 4
#define ID3V2_PADDING_SIZE 4
#define ID3V2_HEADER_SIZE 10

//includes ext header, header, and crc
#define ID3V2_FULL_HEADER_LEN 24
#include <stdbool.h>

typedef enum _Id3v2Headerversion{
    ID3V22 = 2,
    ID3V23 = 3,
    ID3V24 = 4,
    ID3V2INVLAIDVERSION = 0
}Id3v2HeaderVersion;

typedef struct _Id3v2ExtHeader{
    int size;
    int padding;

    //update flag values
    unsigned char update;

    //crc flag values
    unsigned char *crc;
    unsigned int crcLen;
    
    //tag restrictions values
    unsigned char tagSizeRestriction;
    unsigned char encodingRestriction;
    unsigned char textSizeRestriction;
    unsigned char imageEncodingRestriction;
    unsigned char imageSizeRestriction;

}Id3v2ExtHeader;


typedef struct _Id3v2header{
    int versionMinor;
    int versionMajor;
    bool unsynchronisation;
    bool experimentalIndicator;
    bool footer;
    int size;
    Id3v2ExtHeader *extendedHeader;

}Id3v2Header;

//header functions
Id3v2Header *id3v2ParseHeader(unsigned char *buffer, unsigned int bufferSiz);
Id3v2Header *id3v2NewHeader(int versionMinor, int versionMajor, bool unsynchronisation, bool experimentalIndicator, bool footer, int size, Id3v2ExtHeader *extendedHeader);
void id3v2FreeHeader(Id3v2Header *header);

//extended header functions
Id3v2ExtHeader *id3v2ParseExtendedHeader(unsigned char *buffer, Id3v2HeaderVersion version);
Id3v2ExtHeader *id3v2NewExtendedHeader(int size, int padding, unsigned char update, unsigned char *crc, unsigned char tagSizeRestriction, unsigned char encodingRestriction, unsigned char textSizeRestriction, unsigned char imageEncodingRestriction, unsigned char imageSizeRestriction);
void id3v2FreeExtHeader(Id3v2ExtHeader *extHeader);

//utils
Id3v2HeaderVersion id3v2TagVersion(Id3v2Header *tag);
bool containsId3v2(const char *filePath);

#ifdef __cplusplus
} //extern c end
#endif

#endif