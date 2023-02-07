#ifndef ID3V2
#define ID3V2

#ifdef __cplusplus
extern "C"{
#endif

#include "id3v2Header.h"
#include "id3v2Helpers.h"
#include "id3v2Frames.h"

//offsets from the start of a tag
#define ID3V2_TAG_SIZE_OFFSET 7
#define ID3V2_EXTHEADER_SIZE_OFFSET 10


typedef struct _Id3v2{

    Id3v2Header *header;
    List *frames;

}Id3v2;

void id3v2FreeTag(Id3v2 *tag);

Id3v2 *Id3v2Tag(const char *filePath);



#ifdef __cplusplus
} //extern c end
#endif

#endif