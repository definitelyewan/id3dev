/**
 * @file id3v2Context.h
 * @author Ewan Jones
 * @brief provides the ability to generate a context and a list of contexts that represent the ruleset of a frame
 * @version 0.1
 * @date 2024-01-19
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef ID3V2_CONTEXT
#define ID3V2_CONTEXT

#ifdef __cplusplus
extern "C"{
#endif

#include "id3v2Types.h"

unsigned long id3v2djb2(char *str);

//mem

Id3v2ContentContext *id3v2CreateContentContext(Id3v2ContextType type, size_t key, size_t max, size_t min);

void id3v2DestroyContentContext(Id3v2ContentContext **toDelete);


// List/Hash API required functions

void id3v2DeleteContentContext(void *toBeDeleted);

int id3v2CompareContentContext(const void *first, const void *second);

char *id3v2PrintContentContext(const void *toBePrinted);

void *id3v2CopyContentContext(const void *toBeCopied);


void id3v2DeleteContentContextList(void *toBeDeleted);

char *id3v2PrintContentContextList(const void *toBePrinted);

void *id3v2CopyContentContextList(const void *toBeCopied);

// context generators

List *id3v2CreateTextFrameContext(void);

List *id3v2CreateUserDefinedTextFrameContext(void);

List *id3v2CreateURLFrameContext(void);

List *id3v2CreateUserDefinedURLFrameContext(void);

List *id3v2CreateAttachedPictureFrameContext(unsigned int version);

List *id3v2CreateAudioEncryptionFrameContext(void);

List *id3v2CreateAudioSeekPointIndexFrameContext(void);

List *id3v2CreateCommentFrameContext(void);

List *id3v2CreateCommercialFrameContext(void);

List *id3v2CreateEncryptedMetaFrameContext(void);

List *id3v2CreateRegistrationFrameContext(void); //ENCR GRID
List *id3v2CreateMusicCDIdentifierFrameContext(void);

List *id3v2CreatePlayCounterFrameContext(void);

List *id3v2CreateEqualizationFrameContext(unsigned int version);

List *id3v2CreateEventTimingCodesFrameContext(void);

List *id3v2CreateGeneralEncapsulatedObjectFrameContext(void);

List *id3v2CreateInvolvedPeopleListFrameContext(void);

List *id3v2CreateLinkedInformationFrameContext(void);

List *id3v2CreateMPEGLocationLookupTableFrameContext(void);

List *id3v2CreateOwnershipFrameContext(void);

List *id3v2CreatePopularimeterFrameContext(void);

List *id3v2CreatePositionSynchronisationFrameContext(void);

List *id3v2CreatePrivateFrameContext(void);

List *id3v2CreateRecommendedBufferSizeFrameContext(void);

List *id3v2CreateRelativeVolumeAdjustmentFrameContext(unsigned int version);

List *id3v2CreateReverbFrameContext(void);

List *id3v2CreateSeekFrameContext(void);

List *id3v2CreateSignatureFrameContext(void);

List *id3v2CreateSynchronisedLyricFrameContext(void);

List *id3v2CreateSynchronisedTempoCodesFrameContext(void);

List *id3v2CreateUniqueFileIdentifierFrameContext(void);

List *id3v2CreateTermsOfUseFrameContext(void);

List *id3v2CreateUnsynchronisedLyricFrameContext(void);

List *id3v2CreateGenericFrameContext(void);

// pairings

HashTable *id3v2CreateDefaultIdentifierContextPairings(unsigned int version);

bool id3v2InsertIdentifierContextPair(HashTable *identifierContextPairs, char key[ID3V2_FRAME_ID_MAX_SIZE],
                                      List *context);


// convi

uint8_t *id3v2ContextSerialize(Id3v2ContentContext *cc, size_t *outl);

char *id3v2ContextToJSON(Id3v2ContentContext *cc);

#ifdef __cplusplus
} //extern c end
#endif

#endif
