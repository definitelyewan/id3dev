/**
 * @file id3v2Frame.h
 * @author Ewan Jones
 * @brief function declarations for frame operations
 * @version 0.1
 * @date 2024-02-26
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef ID3V2_FRAME
#define ID3V2_FRAME

#ifdef __cplusplus
extern "C"{
#endif

#include "id3v2Types.h"

/*
    Frame header
*/
Id3v2FrameHeader *id3v2CreateFrameHeader(const uint8_t id[ID3V2_FRAME_ID_MAX_SIZE],
                                         const bool tagAlter, const bool fileAlter,
                                         const bool readOnly, const bool unsync,
                                         const uint32_t decompressionSize,
                                         const uint8_t encryptionSymbol, const uint8_t groupSymbol);
void id3v2DestroyFrameHeader(Id3v2FrameHeader **toDelete);

Id3v2ContentEntry *id3v2CreateContentEntry(const void *entry, size_t size);

// List/Hash API required functions

void id3v2DeleteContentEntry(void *toBeDeleted);
int id3v2CompareContentEntry(const void *first, const void *second);
char *id3v2PrintContentEntry(const void *toBePrinted);
void *id3v2CopyContentEntry(const void *toBeCopied);

void id3v2DeleteFrame(void *toBeDeleted);
int id3v2CompareFrame(const void *first, const void *second);
char *id3v2PrintFrame(const void *toBePrinted);
void *id3v2CopyFrame(const void *toBeCopied);

/*
    Frame
*/

Id3v2Frame *id3v2CreateFrame(Id3v2FrameHeader *header, List *context, List *entries);
void id3v2DestroyFrame(Id3v2Frame **toDelete);
Id3v2Frame *id3v2CreateEmptyFrame(const char id[ID3V2_FRAME_ID_MAX_SIZE], const uint8_t version,
                                  HashTable *userPairs);

/*
    Frame access
*/

bool id3v2CompareFrameId(const Id3v2Frame *frame, const char id[ID3V2_FRAME_ID_MAX_SIZE]);

ListIter id3v2CreateFrameTraverser(Id3v2Tag *tag);
Id3v2Frame *id3v2FrameTraverse(ListIter *traverser);


ListIter id3v2CreateFrameEntryTraverser(Id3v2Frame *frame);
void *id3v2ReadFrameEntry(ListIter *traverser, size_t *dataSize);
char *id3v2ReadFrameEntryAsChar(ListIter *traverser, size_t *dataSize);
uint8_t id3v2ReadFrameEntryAsU8(ListIter *traverser);
uint16_t id3v2ReadFrameEntryAsU16(ListIter *traverser);
uint32_t id3v2ReadFrameEntryAsU32(ListIter *traverser);

bool id3v2WriteFrameEntry(const Id3v2Frame *frame, const ListIter *entries, const size_t entrySize,
                          const void *entry);

bool id3v2AttachFrameToTag(Id3v2Tag *tag, Id3v2Frame *frame);
Id3v2Frame *id3v2DetachFrameFromTag(Id3v2Tag *tag, Id3v2Frame *frame);


/*
    output
*/

size_t id3v2SerializeEncodedStringEntry(const Id3v2Frame *frame, ListIter *trav,
                                        unsigned char **outStr);
size_t id3v2SerializeLatin1StringEntry(ListIter *trav, unsigned char **outStr);
uint8_t *id3v2FrameHeaderSerialize(Id3v2FrameHeader *header, uint8_t version, uint32_t frameSize,
                                   size_t *outLength);
char *id3v2FrameHeaderToJSON(const Id3v2FrameHeader *header, const uint8_t version);

uint8_t *id3v2FrameSerialize(Id3v2Frame *frame, uint8_t version, size_t *outLength);
char *id3v2FrameToJSON(Id3v2Frame *frame, uint8_t version);


#ifdef __cplusplus
} //extern c end
#endif

#endif