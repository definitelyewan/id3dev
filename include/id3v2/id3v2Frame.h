#ifndef ID3V2_FRAME
#define ID3V2_FRAME

#ifdef __cplusplus
extern "C"{
#endif

#include "id3v2Types.h"

Id3v2FrameHeader *id3v2CreateFrameHeader(uint8_t id[ID3V2_FRAME_ID_MAX_SIZE], bool tagAlter, bool fileAlter, bool readOnly, bool unsync, uint32_t decompressionSize, uint8_t encryptionSymbol, uint8_t groupSymbol);
void id3v2DestroyFrameHeader(Id3v2FrameHeader **toDelete);

Id3v2ContentEntry *id3v2CreateContentEntry(void *entry, size_t size);

// List/Hash API required functions

void id3v2DeleteContentEntry(void *toBeDeleted);
int id3v2CompareContentEntry(const void *first, const void *second);
char *id3v2PrintContentEntry(const void *toBePrinted);
void *id3v2CopyContentEntry(const void *toBeCopied);

Id3v2Frame *id3v2CreateFrame(Id3v2FrameHeader *header, List *context, List *entries);
void id3v2DestroyFrame(Id3v2Frame **toDelete);

#ifdef __cplusplus
} //extern c end
#endif

#endif