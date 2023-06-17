#ifndef ID3V2_WRITE
#define ID3V2_WRITE

#ifdef __cplusplus
extern "C"{
#endif

#include "id3Types.h"

//inner frame files
void id3v2WritePicture(const char *fileName, Id3v2Frame *frame);
void id3v2WriteGeneralEncapsulatedObject(Id3v2Frame *frame);

#ifdef __cplusplus
} //extern c end
#endif

#endif