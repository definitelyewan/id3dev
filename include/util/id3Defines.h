#ifndef ID3_DEFINES
#define ID3_DEFINES

#ifdef __cplusplus
extern "C"{
#endif

/*
    ID3 DATA TYPES
*/

//A buffer of n bytes consisting of raw data from parsed metadata
//This can be in the form of UTF8, UTF16, UTF16BE, ISO_8859_1, or unformated
typedef unsigned char * _id3buf;
#define id3buf _id3buf


//A single byte of raw metadata
//This can be in the form of UTF8, UTF16, UTF16BE, ISO_8859_1, or unformated
typedef unsigned char _id3byte;
#define id3byte _id3byte

/*
    READER
*/

//ISO_8859_1 encoding
#define ISO_8859_1 0x00

//utf16 encoding
#define UTF16 0x01

//utf16be encoding
#define UTF16BE 0x02

//utf8 encoding
#define UTF8 0x03

//unknown encoding
#define UNKNOWN_ENCODING 0x04

//utf16 bom size
#define UNICODE_BOM_SIZE 2

/*
    ID3V1
*/

//The length of the ID3v1 identifier 'TAG'
#define ID3V1_ID_LEN 3

//The max bytes of all Id3v1 tags
#define ID3V1_MAX_BYTES 128

//The max length of an individual ID3v1 tag 
#define ID3V1_TAG_LEN 30

//The max length of the ID3v1 year tag
#define ID3V1_YEAR_LEN 4

/*
    ID3V2
*/

//The length of the size bytes found within the ID3v2 header
#define ID3V2_HEADER_SIZE_LEN 4

//Max bytes used in the extended headers crc
#define ID3V2_CRC_LEN 4

//Padding bytes found in ID3v2.2s header
#define ID3V2_PADDING_SIZE 4

//Length of bytes found in ID3v2s header or frame header
#define ID3V2_HEADER_SIZE 10

//The length of the size bytes found within the ID3v2.2 frame header
#define ID3V22_SIZE_OF_SIZE_BYTES 3

//The length of the size bytes found within the ID3v2.3 frame header
#define ID3V23_SIZE_OF_SIZE_BYTES 4

//The length of the size bytes found within the ID3v2.4 frame header
#define ID3V24_SIZE_OF_SIZE_BYTES 4

//The number of bytes dedicated to flags in an ID3v2 frame header
#define ID3V2_FLAG_SIZE_OF_BYTES 2

//The number of bytes dedicated to the ID3v2 tags verion
#define ID3V2_VERSION_SIZE_OF_BYTES 2

//The number of bytes dedicated to an ID3v2 pictures format
#define ID3V2_PICTURE_FORMAT_LEN 3

//The number of bytes dedicated to an ID3v2 lenguage code
#define ID3V2_LANGUAGE_LEN 3

//The number of bytes dedicated to an ID3v2 time stamp
#define ID3V2_TIME_STAMP_LEN 4

//The number of bytes dedicated to an Id3v2 event code
#define ID3V2_EVENT_CODE_LEN 5

//The number of bytes dedicated to a date in an ID3v2 commercial or ownership frame
#define ID3V2_BASE_DATE_FORMAT_LEN 8

//The offset to the tag size header in an ID3v2.2 tag
#define ID3V2_TAG_SIZE_OFFSET 6

//The length of the 'image/' MIME type prefix
#define ID3V2_MIME_IMG_LEN 6

//zlib compression size
#define ZLIB_COMPRESSION_SIZE 4

#ifdef __cplusplus
} //extern c end
#endif

#endif