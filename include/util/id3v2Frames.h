#ifndef ID3V2_FRAMES
#define ID3V2_FRAMES

#ifdef __cplusplus
extern "C"{
#endif

#include "id3v2Helpers.h"
#include "id3Types.h"
#include <stdbool.h>

//ID title sizes
#define ID3V22_ID_SIZE 3
#define ID3V23_ID_SIZE 4
#define ID3V24_ID_SIZE 4

//size description for frames
#define ID3V22_SIZE_OF_SIZE_BYTES 3
#define ID3V23_SIZE_OF_SIZE_BYTES 4
#define ID3V24_SIZE_OF_SIZE_BYTES 4

//flag size defines
#define ID3V2_FLAG_SIZE_OF_BYTES 2

//set size lengths
#define ID3V2_PICTURE_FORMATE_LEN 3
#define ID3V2_LANGUAGE_LEN 3
#define ID3V2_TIME_STAMP_LEN 4

//static lengths within frames
#define ID3V2_EVENT_CODE_LEN 5
#define ID3V2_BASE_DATE_FORMAT_LEN 8
#define ID3V24_DATE_FORMAT_LEN 19

//frame functions
List *id3v2ExtractFrames(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2NewFrame(Id3v2FrameHeader *header, void *bodyContent);
Id3v2Frame *id3v2ParseFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2SearchFrame(Id3v2Tag *tag, Id3v2FrameId id);
void id3v2FreeFrame(void *toDelete);

//header frame functions
Id3v2FrameHeader *id3v2ParseFrameHeader(unsigned char *buffer, Id3v2Header *header);
Id3v2FrameHeader *id3v2NewFrameHeader(char *id, unsigned int frameSize, unsigned int headerSize, Id3v2FlagContent *flagContent);
Id3v2FrameHeader *id3v2CopyFrameHeader(Id3v2FrameHeader *header);
void id3v2FreeFrameHeader(Id3v2FrameHeader *toDelete);

//flag content functions
unsigned int id3v2SizeOfFlagContent(Id3v2FlagContent *content);
Id3v2FlagContent *id3v2NewFlagContent(bool tagAlterPreservation, bool fileAlterPreservation, bool readOnly, bool unsynchronisation, bool dataLengthIndicator, unsigned int decompressedSize, unsigned char encryption, unsigned char grouping);
Id3v2FlagContent *id3v2ParseFlagContent(unsigned char *buffer, Id3v2HeaderVersion version);
Id3v2FlagContent *id3v2CopyFlagContent(Id3v2FlagContent *flagcon);
void id3v2FreeFlagContent(Id3v2FlagContent *toDelete);

//text frame functions
Id3v2Frame *id3v2ParseTextFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2TextBody *id3v2ParseTextBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2TextBody *id3v2NewTextBody(unsigned char encoding, unsigned char *value, unsigned char *description);
void id3v2FreeTextFrame(Id3v2Frame *toDelete);

//url frame functions
Id3v2Frame *id3v2ParseURLFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2URLBody *id3v2ParseURLBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2URLBody *id3v2NewURLBody(unsigned char encoding, unsigned char *url, unsigned char *description);
void id3v2FreeURLFrame(Id3v2Frame *toDelete);

//involved persons list frame functions
Id3v2Frame *id3v2ParseInvolvedPeopleListFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2InvolvedPeopleListBody *id3v2ParseInvolvedPeopleListBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2InvolvedPeopleListBody *id3v2NewInvolvedPeopleListBody(unsigned char encoding, unsigned char *peopleListStrings);
void id3v2FreeInvolvedPeopleListFrame(Id3v2Frame *toDelete);

//music cd identifier frame funstions
Id3v2Frame *id3v2ParseMusicCDIdentifierFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2MusicCDIdentifierBody *id3v2ParseMusicCDIdentifierBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2MusicCDIdentifierBody *id3v2NewMusicCDIdentifierBody(unsigned char *cdtoc);
void id3v2FreeMusicCDIdentifierFrame(Id3v2Frame *toDelete);

//event time codes frame functions
Id3v2Frame *id3v2ParseEventTimeCodesFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2EventTimeCodesBody *id3v2ParseEventTimeCodesBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2EventTimeCodesBody *id3v2NewEventTimeCodesBody(unsigned int timeStampFormat, List *events);
Id3v2EventTimesCodeEvent *id3v2NewEventCodeEvent(unsigned char typeOfEvent, long timeStamp);
void id3v2FreeEventTimeCodesFrame(Id3v2Frame *toDelete);
void id3v2FreeEventCode(void *toDelete);

//synced tempo codes functions
Id3v2Frame *id3v2ParseSyncedTempoCodesFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2SyncedTempoCodesBody *id3v2ParseSyncedTempoCodesBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2SyncedTempoCodesBody *id3v2NewSyncedTempoCodesBody(unsigned char timeStampFormat, unsigned char *tempoData, unsigned int tempoDataLen);
void id3v2FreeSyncedTempoCodesFrame(Id3v2Frame *toDelete);

//unsynced lyrics frame functions
Id3v2Frame *id3v2ParseUnsynchronisedLyricsFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2UnsynchronisedLyricsBody *id3v2ParseUnsynchronisedLyricsBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2UnsynchronisedLyricsBody *id3v2NewUnsynchronisedLyricsBody(unsigned char encoding, unsigned char *language, unsigned char *descriptor, unsigned char *lyrics);
void id3v2FreeUnsynchronizedLyricsFrame(Id3v2Frame *toDelete);

//synced lyrics frame functions
Id3v2Frame *id3v2ParseSynchronisedLyricsFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2SynchronisedLyricsBody *id3v2ParseSynchronisedLyricsBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2SynchronisedLyricsBody *id3v2NewSynchronisedLyricsBody(unsigned char encoding, unsigned char *language, unsigned int timeStampFormat, unsigned int contentType, unsigned char *descriptor, List *lyrics);
Id3v2StampedLyric *id3v2NewStampedLyric(unsigned char* text, long timeStamp);
void id3v2FreeSynchronisedLyricsFrame(Id3v2Frame *toDelete);
void id3v2FreeStampedLyric(void *toDelete);

//comment frame functions
Id3v2Frame *id3v2ParseCommentFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyCommentFrame(Id3v2Frame *frame);
Id3v2CommentBody *id3v2CopyCommentBody(Id3v2CommentBody *body);
Id3v2CommentBody *id3v2ParseCommentBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2CommentBody *id3v2NewCommentBody(unsigned char encoding, unsigned char *language, unsigned char *description, unsigned char *text);
void id3v2FreeCommentFrame(Id3v2Frame *toDelete);

//subjective frame functions
Id3v2Frame *id3v2ParseSubjectiveFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopySubjectiveFrame(Id3v2Frame *frame);
Id3v2SubjectiveBody *id3v2CopySubjectiveBody(Id3v2SubjectiveBody *body);
Id3v2SubjectiveBody *id3v2ParseSubjectiveBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2SubjectiveBody *id3v2NewSubjectiveBody(unsigned char *value, int valueSize);
void id3v2FreeSubjectiveFrame(Id3v2Frame *toDelete);

//Relative volume adjustment frame functions
Id3v2Frame *id3v2ParseRelativeVolumeAdjustmentFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyRelativeVolumeAdjustmentFrame(Id3v2Frame *frame);
Id3v2RelativeVolumeAdjustmentBody *id3v2CopyRelativeVolumeAdjustmentBody(Id3v2SubjectiveBody *body);
Id3v2RelativeVolumeAdjustmentBody *id3v2ParseRelativeVolumeAdjustmentBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2RelativeVolumeAdjustmentBody *id3v2NewRelativeVolumeAdjustmentBody(unsigned char *value, int valueSize);
void id3v2FreeRelativeVolumeAdjustmentFrame(Id3v2Frame *toDelete);

//Equalisation frame functions
Id3v2Frame *id3v2ParseEqualisationFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyEqualisationFrame(Id3v2Frame *frame);
Id3v2EqualisationBody *id3v2CopyEqualisationBody(Id3v2EqualisationBody *body);
Id3v2EqualisationBody *id3v2ParseEqualisationBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2EqualisationBody *id3v2NewEqualisationBody(unsigned char *value, int valueSize);
void id3v2FreeEqualisationFrame(Id3v2Frame *toDelete);

//Reverb frame functions
Id3v2Frame *id3v2ParseReverbFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyReverbFrame(Id3v2Frame *frame);
Id3v2ReverbBody *id3v2CopyReverbBody(Id3v2ReverbBody *body);
Id3v2ReverbBody *id3v2ParseReverbBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2ReverbBody *id3v2NewReverbBody(unsigned char *value, int valueSize);
void id3v2FreeReverbFrame(Id3v2Frame *toDelete);

//picture frame functions
Id3v2Frame *id3v2ParsePictureFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyPictureFrame(Id3v2Frame *frame);
Id3v2PictureBody *id3v2CopyPictureBody(Id3v2PictureBody *body);
Id3v2PictureBody *id3v2ParsePictureBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2PictureBody *id3v2NewPictureBody(unsigned char encoding, unsigned char *format, unsigned char pictureType, unsigned char *description, unsigned char *pictureData, int picSize);
void id3v2FreePictureFrame(Id3v2Frame *toDelete);

//general encapsulated object frame functions
Id3v2Frame *id3v2ParseGeneralEncapsulatedObjectFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyGeneralEncapsulatedObjectFrame(Id3v2Frame *frame);
Id3v2GeneralEncapsulatedObjectBody *id3v2CopyGeneralEncapsulatedObjectBody(Id3v2GeneralEncapsulatedObjectBody *body);
Id3v2GeneralEncapsulatedObjectBody *id3v2NewGeneralEncapsulatedObjectBody(unsigned char encoding, unsigned char *mimeType, unsigned char *filename, unsigned char *contentDescription, unsigned char *encapsulatedObject, unsigned int encapsulatedObjectLen);
Id3v2GeneralEncapsulatedObjectBody *id3v2ParseGeneralEncapsulatedObjectBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
void id3v2FreeGeneralEncapsulatedObjectFrame(Id3v2Frame *toDelete);

//play counter frame functions
Id3v2Frame *id3v2ParsePlayCounterFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyPlayCounterFrame(Id3v2Frame *frame);
Id3v2PlayCounterBody *id3v2CopyPlayCounterBody(Id3v2PlayCounterBody *body);
Id3v2PlayCounterBody *id3v2ParsePlayCounterBody(unsigned char *buffer, Id3v2FrameHeader *fhrameHeader);
Id3v2PlayCounterBody *id3v2NewPlayCounterBody(unsigned char *counter);
void id3v2FreePlayCounterFrame(Id3v2Frame *toDelete);

//popularmeter frame functions
Id3v2Frame *id3v2ParsePopularFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyPopularFrame(Id3v2Frame *frame);
Id3v2PopularBody *id3v2CopyPopularBody(Id3v2PopularBody *body);
Id3v2PopularBody *id3v2NewPopularBody(unsigned char *email, unsigned int rating, long counter);
Id3v2PopularBody *id3v2ParsePopularBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
void id3v2FreePopularFrame(Id3v2Frame *toDelete);

//encrypted meta frame functions 2.2 only
Id3v2Frame *id3v2ParseEncryptedMetaFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyEncryptedMetaFrame(Id3v2Frame *frame);
Id3v2EncryptedMetaBody *id3v2CopyEncryptedMetaBody(Id3v2EncryptedMetaBody *body);
Id3v2EncryptedMetaBody *id3v2NewEncryptedMetaBody(unsigned char *ownerIdentifier, unsigned char *content, unsigned char *encryptedDatablock, unsigned int encryptedDatablockLen);
Id3v2EncryptedMetaBody *id3v2ParseEncryptedMetaBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
void id3v2FreeEncryptedMetaFrame(Id3v2Frame *toDelete);

//audio encryption frame functions
Id3v2Frame *id3v2ParseAudioEncryptionFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyAudioEncryptionFrame(Id3v2Frame *frame);
Id3v2AudioEncryptionBody *id3v2CopyAudioEncryptionBody(Id3v2AudioEncryptionBody *body);
Id3v2AudioEncryptionBody *id3v2ParseAudioEncryptionBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2AudioEncryptionBody *id3v2NewAudioEncryptionBody(unsigned char *ownerIdentifier, void *previewStart, unsigned int previewLength, unsigned char *encryptionInfo, unsigned int encryptionInfoLen);
void id3v2FreeAudioEncryptionFrame(Id3v2Frame *toDelete);

//unique file identifier frame functions
Id3v2Frame *id3v2ParseUniqueFileIdentiferFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyUniqueFileIdentifierFrame(Id3v2Frame *frame);
Id3v2UniqueFileIdentifierBody *id3v2CopyUniqueFileIdentifierBody(Id3v2UniqueFileIdentifierBody *body);
Id3v2UniqueFileIdentifierBody *id3v2ParseUniqueFileIdentiferBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2UniqueFileIdentifierBody *id3v2NewUniqueFileIdentifierBody(unsigned char *ownerIdentifier, unsigned char *identifier);
void id3v2FreeUniqueFileIdentifierFrame(Id3v2Frame *toDelete);

//position synchronisation frame ^2.3
Id3v2Frame *id3v2ParsePositionSynchronisationFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyPositionSynchronisationFrame(Id3v2Frame *frame);
Id3v2PositionSynchronisationBody *id3v2CopyPositionSynchronisationBody(Id3v2PositionSynchronisationBody *body);
Id3v2PositionSynchronisationBody *id3v2ParsePositionSynchronisationBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2PositionSynchronisationBody *id3v2NewPositionSynchronisationBody(unsigned char timeStampFormat, long pos);
void id3v2FreePositionSynchronisationFrame(Id3v2Frame *toDelete);

//terms of service frame functions ^2.3
Id3v2Frame *id3v2ParseTermsOfUseFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyTermsOfUseFrame(Id3v2Frame *frame);
Id3v2TermsOfUseBody *id3v2CopyTermsOfUseBody(Id3v2TermsOfUseBody *body);
Id3v2TermsOfUseBody *id3v2ParseTermsOfUseBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2TermsOfUseBody *id3v2NewTermsOfUseBody(unsigned char encoding, unsigned char *language, unsigned char *text);
void id3v2FreeTermsOfUseFrame(Id3v2Frame *toDelete);

//ownership frame functions ^2.3
Id3v2Frame *id3v2ParseOwnershipFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyOwnershipFrame(Id3v2Frame *frame);
Id3v2OwnershipBody *id3v2CopyOwnershipBody(Id3v2OwnershipBody *body);
Id3v2OwnershipBody *id3v2ParseOwnershipBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2OwnershipBody *id3v2NewOwnershipBody(unsigned char encoding, unsigned char *pricePayed, unsigned char *dateOfPunch, unsigned char *seller);
void id3v2FreeOwnershipFrame(Id3v2Frame *toDelete);

//commercial frame functions ^2.3
Id3v2Frame *id3v2ParseCommercialFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyCommercialFrame(Id3v2Frame *frame);
Id3v2CommercialBody *id3v2CopyCommercialBody(Id3v2CommercialBody *body);
Id3v2CommercialBody *id3v2ParseCommercialBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2CommercialBody *id3v2NewCommercialBody(unsigned char encoding, unsigned char *priceString, unsigned char *validUntil, unsigned char *contractURL, unsigned char receivedAs, unsigned char *nameOfSeller, unsigned char *description, unsigned char *mimeType, unsigned char *sellerLogo, unsigned int sellerLogoLen);
void id3v2FreeCommercialFrame(Id3v2Frame *toDelete);

//encryption method registration frame functions ^2.3
Id3v2Frame *id3v2ParseEncryptionMethodRegistrationFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyEncryptionMethodRegistrationFrame(Id3v2Frame *frame);
Id3v2EncryptionMethodRegistrationBody *id3v2CopyEncryptionMethodRegistrationBody(Id3v2EncryptionMethodRegistrationBody *body);
Id3v2EncryptionMethodRegistrationBody *id3v2ParseEncryptionMethodRegistrationBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2EncryptionMethodRegistrationBody *id3v2NewEncryptionMethodRegistrationBody(unsigned char *ownerIdentifier, unsigned char methodSymbol, unsigned char *encryptionData, unsigned int encryptionDataLen);
void id3v2FreeEncryptionMethodRegistrationFrame(Id3v2Frame *toDelete);

//group id registration frame functions ^2.3
Id3v2Frame *id3v2ParseGroupIDRegistrationFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyGroupIDDRegistrationFrame(Id3v2Frame *frame);
Id3v2GroupIDRegistrationBody *id3v2CopyGroupIDRegistrationBody(Id3v2GroupIDRegistrationBody *body);
Id3v2GroupIDRegistrationBody *id3v2ParseGroupIDRegistrationBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2GroupIDRegistrationBody *id3v2NewGroupIDRegistrationBody(unsigned char *ownerIdentifier, unsigned char groupSymbol, unsigned char *groupDependentData, unsigned int groupDependentDataLen);
void id3v2FreeGroupIDRegistrationFrame(Id3v2Frame *toDelete);

//private frame functions ^2.3
Id3v2Frame *id3v2ParsePrivateFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopyPrivateFrame(Id3v2Frame *frame);
Id3v2PrivateBody *id3v2CopyPrivateBody(Id3v2PrivateBody *body);
Id3v2PrivateBody *id3v2ParsePrivateBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2PrivateBody *id3v2NewPrivateBody(unsigned char *ownerIdentifier, unsigned char *privateData, unsigned int privateDataLen);
void id3v2FreePrivateFrame(Id3v2Frame *toDelete);

//Signature frame fnctions
Id3v2Frame *id3v2ParseSignatureFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopySignatureFrame(Id3v2Frame *frame);
Id3v2SignatureBody *id3v2CopySignatureBody(Id3v2SignatureBody *body);
Id3v2SignatureBody *id3v2ParseSignatureBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2SignatureBody *id3v2NewSignatureBody(unsigned char groupSymbol, unsigned char *signature);
void id3v2FreeSignatureFrame(Id3v2Frame *toDelete);

//Seek frame functions
Id3v2Frame *id3v2ParseSeekFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2Frame *id3v2CopySeekFrame(Id3v2Frame *frame);
Id3v2SeekBody *id3v2CopySeekBody(Id3v2SeekBody *body);
Id3v2SeekBody *id3v2ParseSeekBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2SeekBody *id3v2NewSeekBody(int minimumOffsetToNextTag);
void id3v2FreeSeekFrame(Id3v2Frame *toDelete);

//other functions
Id3v2FrameId id3v2FrameIdFromStr(char *str);
int id3v2IdAndSizeOffset(Id3v2Header *header);

#ifdef __cplusplus
} //extern c end
#endif

#endif