#ifndef ID3V2_FRAMES
#define ID3V2_FRAMES

#ifdef __cplusplus
extern "C"{
#endif

#include "id3Types.h"
#include <stdbool.h>

//frame functions
Id3List *id3v2ExtractFrames(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2NewFrame(Id3v2FrameHeader *header, void *bodyContent);
Id3v2Frame *id3v2ParseFrame(id3buf buffer, Id3v2Header *header);
void *id3v2CopyFrame(void *toCopy);
void id3v2FreeFrame(void *toDelete);

//header frame functions
Id3v2FrameHeader *id3v2ParseFrameHeader(id3buf buffer, Id3v2Header *header);
Id3v2FrameHeader *id3v2NewFrameHeader(char *id, unsigned int frameSize, unsigned int headerSize, Id3v2FlagContent *flagContent);
Id3v2FrameHeader *id3v2CopyFrameHeader(Id3v2FrameHeader *header);
void id3v2FreeFrameHeader(Id3v2FrameHeader *toDelete);

//flag content functions
unsigned int id3v2SizeOfFlagContent(Id3v2FlagContent *content);
Id3v2FlagContent *id3v2NewFlagContent(bool tagAlterPreservation, bool fileAlterPreservation, bool readOnly, bool unsynchronisation, bool dataLengthIndicator, unsigned int decompressedSize, id3byte encryption, id3byte grouping);
Id3v2FlagContent *id3v2ParseFlagContent(id3buf buffer, Id3v2HeaderVersion version);
Id3v2FlagContent *id3v2CopyFlagContent(Id3v2FlagContent *flagcon);
void id3v2FreeFlagContent(Id3v2FlagContent *toDelete);

//text frame functions
Id3v2Frame *id3v2CreateTextFrame(Id3v2FrameId id, id3byte encoding, id3buf value, id3buf description);
Id3v2Frame *id3v2ParseTextFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyTextFrame(Id3v2Frame *frame);
Id3v2TextBody *id3v2CopyTextBody(Id3v2TextBody *body);
Id3v2TextBody *id3v2ParseTextBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2TextBody *id3v2NewTextBody(id3byte encoding, id3buf value, id3buf description);
void id3v2FreeTextFrame(Id3v2Frame *toDelete);

//url frame functions
Id3v2Frame *id3v2CreateURLFrame(Id3v2FrameId id, id3byte encoding, id3buf url, id3buf description);
Id3v2Frame *id3v2ParseURLFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyURLFrame(Id3v2Frame *frame);
Id3v2URLBody *id3v2CopyURLBody(Id3v2URLBody *body);
Id3v2URLBody *id3v2ParseURLBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2URLBody *id3v2NewURLBody(id3byte encoding, id3buf url, id3buf description);
void id3v2FreeURLFrame(Id3v2Frame *toDelete);

//involved persons list frame functions
Id3v2Frame *id3v2CreateInvolvedPeopleListFrame(Id3v2FrameId id, id3byte encoding, id3buf peopleListStrings);
Id3v2Frame *id3v2ParseInvolvedPeopleListFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyInvolvedPeopleListFrame(Id3v2Frame *frame);
Id3v2InvolvedPeopleListBody *id3v2CopyInvolvedPeopleListBody(Id3v2InvolvedPeopleListBody *body);
Id3v2InvolvedPeopleListBody *id3v2ParseInvolvedPeopleListBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2InvolvedPeopleListBody *id3v2NewInvolvedPeopleListBody(id3byte encoding, id3buf peopleListStrings);
void id3v2FreeInvolvedPeopleListFrame(Id3v2Frame *toDelete);

//music cd identifier frame funstions
Id3v2Frame *id3v2CreateMusicCDIdentifierFrame(Id3v2FrameId id, id3buf cdtoc);
Id3v2Frame *id3v2ParseMusicCDIdentifierFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyMusicCDIdentifierFrame(Id3v2Frame *frame);
Id3v2MusicCDIdentifierBody *id3v2CopyMusicCDIdentifierBody(Id3v2MusicCDIdentifierBody *body);
Id3v2MusicCDIdentifierBody *id3v2ParseMusicCDIdentifierBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2MusicCDIdentifierBody *id3v2NewMusicCDIdentifierBody(id3buf cdtoc);
void id3v2FreeMusicCDIdentifierFrame(Id3v2Frame *toDelete);

//event time codes frame functions
Id3v2Frame *id3v2CreateEventCodesFrame(Id3v2FrameId id, unsigned int timeStampFormat);
Id3v2Frame *id3v2ParseEventTimeCodesFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyEventTimeCodesFrame(Id3v2Frame *frame);
Id3v2EventTimeCodesBody *id3v2CopyEventTimeCodesBody(Id3v2EventTimeCodesBody *body);
Id3v2EventTimeCodesBody *id3v2ParseEventTimeCodesBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2EventTimeCodesBody *id3v2NewEventTimeCodesBody(unsigned int timeStampFormat, Id3List *events);
Id3v2EventTimesCodeEvent *id3v2NewEventCodeEvent(id3byte typeOfEvent, long timeStamp);
void id3v2FreeEventTimeCodesFrame(Id3v2Frame *toDelete);
void *id3v2CopyEventCodeEvent(void *toCopy);
void id3v2FreeEventCode(void *toDelete);

//synced tempo codes functions
Id3v2Frame *id3v2CreateSyncedTempoCodesFrame(Id3v2FrameId id, id3byte timeStampFormat, id3buf tempoData, unsigned int tempoDataLen);
Id3v2Frame *id3v2ParseSyncedTempoCodesFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopySyncedTempoCodesFrame(Id3v2Frame *frame);
Id3v2SyncedTempoCodesBody *id3v2CopySyncedTempoCodesBody(Id3v2SyncedTempoCodesBody *body);
Id3v2SyncedTempoCodesBody *id3v2ParseSyncedTempoCodesBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2SyncedTempoCodesBody *id3v2NewSyncedTempoCodesBody(id3byte timeStampFormat, id3buf tempoData, unsigned int tempoDataLen);
void id3v2FreeSyncedTempoCodesFrame(Id3v2Frame *toDelete);

//unsynced lyrics frame functions
Id3v2Frame *id3v2CreateUnsynchronizedLyricsFrame(Id3v2FrameId id, id3byte encoding, id3buf language, id3buf descriptor, id3buf lyrics);
Id3v2Frame *id3v2ParseUnsynchronizedLyricsFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyUnsynchronizedLyricsFrame(Id3v2Frame *frame);
Id3v2UnsynchronizedLyricsBody *id3v2CopyUnsynchronizedLyricsBody(Id3v2UnsynchronizedLyricsBody *body);
Id3v2UnsynchronizedLyricsBody *id3v2ParseUnsynchronizedLyricsBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2UnsynchronizedLyricsBody *id3v2NewUnsynchronizedLyricsBody(id3byte encoding, id3buf language, id3buf descriptor, id3buf lyrics);
void id3v2FreeUnsynchronizedLyricsFrame(Id3v2Frame *toDelete);

//synced lyrics frame functions
Id3v2Frame *id3v2CreateSynchronizedLyricsFrame(Id3v2FrameId id, id3byte encoding, id3buf language, unsigned int timeStampFormat, unsigned int contentType, id3buf descriptor);
Id3v2Frame *id3v2ParseSynchronizedLyricsFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopySynchronizedLyricsFrame(Id3v2Frame *frame);
Id3v2SynchronizedLyricsBody *id3v2CopySynchronizedLyricsBody(Id3v2SynchronizedLyricsBody *body);
Id3v2SynchronizedLyricsBody *id3v2ParseSynchronizedLyricsBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2SynchronizedLyricsBody *id3v2NewSynchronizedLyricsBody(id3byte encoding, id3buf language, unsigned int timeStampFormat, unsigned int contentType, id3buf descriptor, Id3List *lyrics);
Id3v2StampedLyric *id3v2NewStampedLyric(id3buf text, long timeStamp, size_t lyricLen);
void id3v2FreeSynchronizedLyricsFrame(Id3v2Frame *toDelete);
void *id3v2CopyStampedLyric(void *toCopy);
void id3v2FreeStampedLyric(void *toDelete);

//comment frame functions
Id3v2Frame *id3v2ParseCommentFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyCommentFrame(Id3v2Frame *frame);
Id3v2CommentBody *id3v2CopyCommentBody(Id3v2CommentBody *body);
Id3v2CommentBody *id3v2ParseCommentBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2CommentBody *id3v2NewCommentBody(id3byte encoding, id3buf language, id3buf description, id3buf text);
void id3v2FreeCommentFrame(Id3v2Frame *toDelete);

//subjective frame functions
Id3v2Frame *id3v2ParseSubjectiveFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopySubjectiveFrame(Id3v2Frame *frame);
Id3v2SubjectiveBody *id3v2CopySubjectiveBody(Id3v2SubjectiveBody *body);
Id3v2SubjectiveBody *id3v2ParseSubjectiveBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2SubjectiveBody *id3v2NewSubjectiveBody(id3buf value, int valueSize);
void id3v2FreeSubjectiveFrame(Id3v2Frame *toDelete);

//Relative volume adjustment frame functions
Id3v2Frame *id3v2ParseRelativeVolumeAdjustmentFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyRelativeVolumeAdjustmentFrame(Id3v2Frame *frame);
Id3v2RelativeVolumeAdjustmentBody *id3v2CopyRelativeVolumeAdjustmentBody(Id3v2SubjectiveBody *body);
Id3v2RelativeVolumeAdjustmentBody *id3v2ParseRelativeVolumeAdjustmentBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2RelativeVolumeAdjustmentBody *id3v2NewRelativeVolumeAdjustmentBody(id3buf value, int valueSize);
void id3v2FreeRelativeVolumeAdjustmentFrame(Id3v2Frame *toDelete);

//Equalisation frame functions
Id3v2Frame *id3v2ParseEqualisationFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyEqualisationFrame(Id3v2Frame *frame);
Id3v2EqualisationBody *id3v2CopyEqualisationBody(Id3v2EqualisationBody *body);
Id3v2EqualisationBody *id3v2ParseEqualisationBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2EqualisationBody *id3v2NewEqualisationBody(id3buf value, int valueSize);
void id3v2FreeEqualisationFrame(Id3v2Frame *toDelete);

//Reverb frame functions
Id3v2Frame *id3v2ParseReverbFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyReverbFrame(Id3v2Frame *frame);
Id3v2ReverbBody *id3v2CopyReverbBody(Id3v2ReverbBody *body);
Id3v2ReverbBody *id3v2ParseReverbBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2ReverbBody *id3v2NewReverbBody(id3buf value, int valueSize);
void id3v2FreeReverbFrame(Id3v2Frame *toDelete);

//picture frame functions
Id3v2Frame *id3v2ParsePictureFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyPictureFrame(Id3v2Frame *frame);
Id3v2PictureBody *id3v2CopyPictureBody(Id3v2PictureBody *body);
Id3v2PictureBody *id3v2ParsePictureBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2PictureBody *id3v2NewPictureBody(id3byte encoding, id3buf format, id3byte pictureType, id3buf description, id3buf pictureData, int picSize);
void id3v2FreePictureFrame(Id3v2Frame *toDelete);

//general encapsulated object frame functions
Id3v2Frame *id3v2ParseGeneralEncapsulatedObjectFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyGeneralEncapsulatedObjectFrame(Id3v2Frame *frame);
Id3v2GeneralEncapsulatedObjectBody *id3v2CopyGeneralEncapsulatedObjectBody(Id3v2GeneralEncapsulatedObjectBody *body);
Id3v2GeneralEncapsulatedObjectBody *id3v2NewGeneralEncapsulatedObjectBody(id3byte encoding, id3buf mimeType, id3buf filename, id3buf contentDescription, id3buf encapsulatedObject, unsigned int encapsulatedObjectLen);
Id3v2GeneralEncapsulatedObjectBody *id3v2ParseGeneralEncapsulatedObjectBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
void id3v2FreeGeneralEncapsulatedObjectFrame(Id3v2Frame *toDelete);

//play counter frame functions
Id3v2Frame *id3v2ParsePlayCounterFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyPlayCounterFrame(Id3v2Frame *frame);
Id3v2PlayCounterBody *id3v2CopyPlayCounterBody(Id3v2PlayCounterBody *body);
Id3v2PlayCounterBody *id3v2ParsePlayCounterBody(id3buf buffer, Id3v2FrameHeader *fhrameHeader);
Id3v2PlayCounterBody *id3v2NewPlayCounterBody(long counter);
void id3v2FreePlayCounterFrame(Id3v2Frame *toDelete);

//popularmeter frame functions
Id3v2Frame *id3v2ParsePopularFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyPopularFrame(Id3v2Frame *frame);
Id3v2PopularBody *id3v2CopyPopularBody(Id3v2PopularBody *body);
Id3v2PopularBody *id3v2NewPopularBody(id3buf email, unsigned int rating, long counter);
Id3v2PopularBody *id3v2ParsePopularBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
void id3v2FreePopularFrame(Id3v2Frame *toDelete);

//encrypted meta frame functions 2.2 only
Id3v2Frame *id3v2ParseEncryptedMetaFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyEncryptedMetaFrame(Id3v2Frame *frame);
Id3v2EncryptedMetaBody *id3v2CopyEncryptedMetaBody(Id3v2EncryptedMetaBody *body);
Id3v2EncryptedMetaBody *id3v2NewEncryptedMetaBody(id3buf ownerIdentifier, id3buf content, id3buf encryptedDatablock, unsigned int encryptedDatablockLen);
Id3v2EncryptedMetaBody *id3v2ParseEncryptedMetaBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
void id3v2FreeEncryptedMetaFrame(Id3v2Frame *toDelete);

//audio encryption frame functions
Id3v2Frame *id3v2ParseAudioEncryptionFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyAudioEncryptionFrame(Id3v2Frame *frame);
Id3v2AudioEncryptionBody *id3v2CopyAudioEncryptionBody(Id3v2AudioEncryptionBody *body);
Id3v2AudioEncryptionBody *id3v2ParseAudioEncryptionBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2AudioEncryptionBody *id3v2NewAudioEncryptionBody(id3buf ownerIdentifier, void *previewStart, unsigned int previewLength, id3buf encryptionInfo, unsigned int encryptionInfoLen);
void id3v2FreeAudioEncryptionFrame(Id3v2Frame *toDelete);

//unique file identifier frame functions
Id3v2Frame *id3v2ParseUniqueFileIdentiferFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyUniqueFileIdentifierFrame(Id3v2Frame *frame);
Id3v2UniqueFileIdentifierBody *id3v2CopyUniqueFileIdentifierBody(Id3v2UniqueFileIdentifierBody *body);
Id3v2UniqueFileIdentifierBody *id3v2ParseUniqueFileIdentiferBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2UniqueFileIdentifierBody *id3v2NewUniqueFileIdentifierBody(id3buf ownerIdentifier, id3buf identifier);
void id3v2FreeUniqueFileIdentifierFrame(Id3v2Frame *toDelete);

//position synchronisation frame ^2.3
Id3v2Frame *id3v2ParsePositionSynchronisationFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyPositionSynchronisationFrame(Id3v2Frame *frame);
Id3v2PositionSynchronisationBody *id3v2CopyPositionSynchronisationBody(Id3v2PositionSynchronisationBody *body);
Id3v2PositionSynchronisationBody *id3v2ParsePositionSynchronisationBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2PositionSynchronisationBody *id3v2NewPositionSynchronisationBody(id3byte timeStampFormat, long pos);
void id3v2FreePositionSynchronisationFrame(Id3v2Frame *toDelete);

//terms of service frame functions ^2.3
Id3v2Frame *id3v2ParseTermsOfUseFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyTermsOfUseFrame(Id3v2Frame *frame);
Id3v2TermsOfUseBody *id3v2CopyTermsOfUseBody(Id3v2TermsOfUseBody *body);
Id3v2TermsOfUseBody *id3v2ParseTermsOfUseBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2TermsOfUseBody *id3v2NewTermsOfUseBody(id3byte encoding, id3buf language, id3buf text);
void id3v2FreeTermsOfUseFrame(Id3v2Frame *toDelete);

//ownership frame functions ^2.3
Id3v2Frame *id3v2ParseOwnershipFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyOwnershipFrame(Id3v2Frame *frame);
Id3v2OwnershipBody *id3v2CopyOwnershipBody(Id3v2OwnershipBody *body);
Id3v2OwnershipBody *id3v2ParseOwnershipBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2OwnershipBody *id3v2NewOwnershipBody(id3byte encoding, id3buf pricePayed, id3buf dateOfPunch, id3buf seller);
void id3v2FreeOwnershipFrame(Id3v2Frame *toDelete);

//commercial frame functions ^2.3
Id3v2Frame *id3v2ParseCommercialFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyCommercialFrame(Id3v2Frame *frame);
Id3v2CommercialBody *id3v2CopyCommercialBody(Id3v2CommercialBody *body);
Id3v2CommercialBody *id3v2ParseCommercialBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2CommercialBody *id3v2NewCommercialBody(id3byte encoding, id3buf priceString, id3buf validUntil, id3buf contractURL, id3byte receivedAs, id3buf nameOfSeller, id3buf description, id3buf mimeType, id3buf sellerLogo, unsigned int sellerLogoLen);
void id3v2FreeCommercialFrame(Id3v2Frame *toDelete);

//encryption method registration frame functions ^2.3
Id3v2Frame *id3v2ParseEncryptionMethodRegistrationFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyEncryptionMethodRegistrationFrame(Id3v2Frame *frame);
Id3v2EncryptionMethodRegistrationBody *id3v2CopyEncryptionMethodRegistrationBody(Id3v2EncryptionMethodRegistrationBody *body);
Id3v2EncryptionMethodRegistrationBody *id3v2ParseEncryptionMethodRegistrationBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2EncryptionMethodRegistrationBody *id3v2NewEncryptionMethodRegistrationBody(id3buf ownerIdentifier, id3byte methodSymbol, id3buf encryptionData, unsigned int encryptionDataLen);
void id3v2FreeEncryptionMethodRegistrationFrame(Id3v2Frame *toDelete);

//group id registration frame functions ^2.3
Id3v2Frame *id3v2ParseGroupIDRegistrationFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyGroupIDRegistrationFrame(Id3v2Frame *frame);
Id3v2GroupIDRegistrationBody *id3v2CopyGroupIDRegistrationBody(Id3v2GroupIDRegistrationBody *body);
Id3v2GroupIDRegistrationBody *id3v2ParseGroupIDRegistrationBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2GroupIDRegistrationBody *id3v2NewGroupIDRegistrationBody(id3buf ownerIdentifier, id3byte groupSymbol, id3buf groupDependentData, unsigned int groupDependentDataLen);
void id3v2FreeGroupIDRegistrationFrame(Id3v2Frame *toDelete);

//private frame functions ^2.3
Id3v2Frame *id3v2ParsePrivateFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyPrivateFrame(Id3v2Frame *frame);
Id3v2PrivateBody *id3v2CopyPrivateBody(Id3v2PrivateBody *body);
Id3v2PrivateBody *id3v2ParsePrivateBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2PrivateBody *id3v2NewPrivateBody(id3buf ownerIdentifier, id3buf privateData, unsigned int privateDataLen);
void id3v2FreePrivateFrame(Id3v2Frame *toDelete);

//Signature frame fnctions
Id3v2Frame *id3v2ParseSignatureFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopySignatureFrame(Id3v2Frame *frame);
Id3v2SignatureBody *id3v2CopySignatureBody(Id3v2SignatureBody *body);
Id3v2SignatureBody *id3v2ParseSignatureBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2SignatureBody *id3v2NewSignatureBody(id3byte groupSymbol, id3buf signature, unsigned int sigLen);
void id3v2FreeSignatureFrame(Id3v2Frame *toDelete);

//Seek frame functions
Id3v2Frame *id3v2ParseSeekFrame(id3buf buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopySeekFrame(Id3v2Frame *frame);
Id3v2SeekBody *id3v2CopySeekBody(Id3v2SeekBody *body);
Id3v2SeekBody *id3v2ParseSeekBody(id3buf buffer, Id3v2FrameHeader *frameHeader);
Id3v2SeekBody *id3v2NewSeekBody(int minimumOffsetToNextTag);
void id3v2FreeSeekFrame(Id3v2Frame *toDelete);

//other functions
Id3v2FrameId id3v2FrameIdFromStr(char *str);
char *id3v2FrameIdStrFromId(Id3v2FrameId id);
int id3v2IdAndSizeOffset(Id3v2Header *header);

#ifdef __cplusplus
} //extern c end
#endif

#endif