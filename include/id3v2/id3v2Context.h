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

//mem

Id3v2ContentContext *id3v2CreateContentContext(Id3v2ContextType type, size_t key, size_t max, size_t min);
void id3v2DestroyContentContext(Id3v2ContentContext **toDelete);

// List API required functions

void id3v2DeleteContentContext(void *toBeDeleted);
int id3v2CompareContentContext(const void *first, const void *second);
char *id3v2PrintContentContext(const void *toBePrinted);
void *id3v2CopyContentContext(const void *toBeCopied);


List *id3v2CreateTextFrameContext(void);
List *id3v2CreateUserDefinedTextFrameContext(void);
List *id3v2CreateURLFrameContext(void);
List *id3v2CreateUserDefinedURLFrameContext(void);
List *id3v2CreateAttachedPictureFrameContext(unsigned int version);
List *id3v2CreateCommentFrameContext(void);
List *id3v2CreateCommercialFrameContext(void); 
List *id3v2CreateEncryptedMetaFrameContext(void);
List *id3v2CreateRegistrationFrameContext(void); //ENCR GRID
List *id3v2CreateMusicCDIdentifierFrameContext(void);
List *id3v2CreatePlayCounterFrameContext(void);
List *id3v2CreateEqulizationFrameContext(unsigned int version);
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
List *id3v2CreateSeekPointIndexFrameContext(void);
List *id3v2CreateSignatureFrameContext(void);
List *id3v2CreateSynchronisedLyricFrameContext(void);
List *id3v2CreateSynchronisedTempoCodesFrameContext(void);
List *id3v2CreateUniqueFileIdentifierFrameContext(void);

List *id3v2CreateUnsynchronisedLyricFrameContext(void);







#ifdef __cplusplus
} //extern c end
#endif

#endif