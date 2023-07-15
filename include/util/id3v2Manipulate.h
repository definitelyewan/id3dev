#ifndef ID3V2_MANIPULATE
#define ID3V2_MANIPULATE

#ifdef __cplusplus
extern "C"{
#endif

#include "id3Types.h"


//tag header should not be touched probably a bad idea to edit that


//frame header functions


//flag functions

//frame content functions

//functions to change values within a id3v2 structure


//set/edit functions

//quick reads n writes for common attrs
void id3v2SetTitle(id3buf value, unsigned int valueLength, Id3v2Tag *tag);
void id3v2SetArtist(id3buf value, unsigned int valueLength, Id3v2Tag *tag);
void id3v2SetAlbum(id3buf value, unsigned int valueLength, Id3v2Tag *tag);
void id3v2SetAlbumArtist(id3buf value, unsigned int valueLength, Id3v2Tag *tag);
void id3v2SetComposer(id3buf value, unsigned int valueLength, Id3v2Tag *tag);
void id3v2SetYear(id3buf value, unsigned int valueLength, Id3v2Tag *tag);
void id3v2SetComment(id3buf value, unsigned int valueLength, Id3v2Tag *tag);
void id3v2SetGenre(id3buf value, unsigned int valueLength, Id3v2Tag *tag);
void id3v2SetTrack(id3buf value, unsigned int valueLength, Id3v2Tag *tag);
void id3v2SetDisc(id3buf value, unsigned int valueLength, Id3v2Tag *tag);
void id3v2SetLyrics(id3buf value, unsigned int valueLength, Id3v2Tag *tag);
void id3v2SetPicture(id3buf value, unsigned int valueLength, Id3v2Tag *tag);

id3buf id3v2ReadTitle(id3byte desiredEncoding, Id3v2Tag *tag);
id3buf id3v2ReadArtist(id3byte desiredEncoding, Id3v2Tag *tag);
id3buf id3v2ReadAlbum(id3byte desiredEncoding, Id3v2Tag *tag);
id3buf id3v2ReadAlbumArtist(id3byte desiredEncoding, Id3v2Tag *tag);
id3buf id3v2ReadComposer(id3byte desiredEncoding, Id3v2Tag *tag);
id3buf id3v2ReadYear(id3byte desiredEncoding, Id3v2Tag *tag);
id3buf id3v2ReadComment(id3byte desiredEncoding, Id3v2Tag *tag);
id3buf id3v2ReadGenre(id3byte desiredEncoding, Id3v2Tag *tag);
id3buf id3v2ReadTrack(id3byte desiredEncoding, Id3v2Tag *tag);
id3buf id3v2ReadDisc(id3byte desiredEncoding, Id3v2Tag *tag);
id3buf id3v2ReadLyrics(id3byte desiredEncoding, Id3v2Tag *tag);
id3buf id3v2ReadPicture(id3byte pictureType, unsigned int *pictureSize, Id3v2Tag *tag);

//util
Id3v2Frame *id3v2IterTag(Id3v2Tag *tag);
void id3v2ResetIterTag(Id3v2Tag *tag);
Id3List *id3v2SearchForFrames(Id3v2Tag *tag, Id3v2FrameId id);
void id3v2AddEventToEventFrame(Id3v2Frame *eventCodeFrame, id3byte typeOfEvent, int timeStamp);
void id3v2AddLyricToSynchronizedLyricsFrame(Id3v2Frame *lyricFrame, id3buf lyric, int timeStamp);
void id3v2AddPersonToInvolvedPersonListFrame(Id3v2Frame *personListFrame, id3buf person, id3buf job);

//read tag header attributes

int id3v2ReadTagSizeRestriction(Id3v2Tag *tag);
int id3v2ReadEncodingRestriction(Id3v2Tag *tag);
int id3v2ReadTextSizeRestriction(Id3v2Tag *tag);
int id3v2ReadImageEncodingRestriction(Id3v2Tag *tag);
int id3v2ReadImageSizeRestriction(Id3v2Tag *tag);
id3buf id3v2ReadCrc(Id3v2Tag *tag);

int id3v2ReadVersion(Id3v2Tag *tag);
bool id3v2ReadUnsynchronizedIndicator(Id3v2Tag *tag);
bool id3v2ReadExperimentalIndicator(Id3v2Tag *tag);
bool id3v2ReadExtendedIndicator(Id3v2Tag *tag);
bool id3v2ReadFooterIndicator(Id3v2Tag *tag);

//read and write frame attributes

//frame flags
void id3v2SetFrameAlterPreservationIndicator(bool indicator, Id3v2Frame *frame);
bool id3v2ReadFrameAlterPreservationIndicator(Id3v2Frame *frame);
void id3v2SetFrameFileAlterPreservationIndicator(bool indicator, Id3v2Frame *frame);
bool id3v2ReadFrameFileAlterPreservationIndicator(Id3v2Frame *frame);
void id3v2SetFrameReadOnlyIndicator(bool indicator, Id3v2Frame *frame);
bool id3v2ReadFrameReadOnlyIndicator(Id3v2Frame *frame);
void id3v2SetFrameUnsynchronizationIndicator(bool indicator, Id3v2Frame *frame);
bool id3v2ReadFrameUnsynchronizationIndicator(Id3v2Frame *frame);

/*
    WRONG WORNG WRONG WRONG WRONG decompressions size and data length indeicator are different
*/
void id3v2SetFrameCompressionSize(size_t size, Id3v2Frame *frame);
size_t id3v2ReadFrameCompressionSize(Id3v2Frame *frame);
void id3v2SetFrameEncryptionMethod(id3byte symbol, Id3v2Frame *frame);
id3byte id3v2ReadFrameEncryptionMethod(Id3v2Frame *frame);
void id3v2SetFrameGroup(id3byte symbol, Id3v2Frame *frame);
id3byte id3v2ReadFrameGroup(Id3v2Frame *frame);
void id3v2SetDataLengthIndicator(bool indicator, Id3v2Frame *frame);
bool id3v2ReadFrameDataLengthIndicator(Id3v2Frame *frame);

//frame header
char *id3v2ReadFrameStrID(Id3v2Frame *frame);
size_t id3v2ReadFrameSize(Id3v2Frame *frame);
size_t id3v2ReadFrameHeaderSize(Id3v2Frame *frame);
Id3v2FrameId id3v2ReadFrameID(Id3v2Frame *frame);

//frame encoding
void id3v2SetEncoding(id3byte encoding, Id3v2Frame *frame);
int id3v2ReadEncoding(Id3v2Frame *frame);

//frames with descriptions
void id3v2SetDescription(id3buf description, size_t descriptionLength, Id3v2Frame *frame);
id3buf id3v2ReadDescription(Id3v2Frame *frame);

//text frame
void id3v2SetTextValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame);
id3buf id3v2ReadTextValue(Id3v2Frame *frame);

//url frame
void id3v2SetURLValue(id3buf value, size_t valueLength, Id3v2Frame *frame);
id3buf id3v2ReadURLValue(Id3v2Frame *frame);

//ipls frame
id3buf id3v2ReadInvolvedPeopleListPerson(Id3v2Frame *frame);
id3buf id3v2ReadInvolvedPeopleListJob(Id3v2Frame *frame);
void id3v2RemoveInvolvedPerson(id3buf personToRemove, unsigned int personToRemoveLength, Id3v2Frame *frame);

//music cd id frame
void id3v2SetCDIDValue(char *cdtoc, Id3v2Frame *frame);
id3buf id3v2ReadCDIDValue(Id3v2Frame *frame);

//frames with time stamp formats
void id3v2SetTimeStampFormat(id3byte format, Id3v2Frame *frame);
int id3v2ReadTimeStampFormat(Id3v2Frame *frame);

//event time codes frame
id3byte id3v2ReadEventTimeCodeType(Id3v2Frame *frame);
int id3v2ReadEventTimeCodeTimeStamp(Id3v2Frame *frame);
void id3v2RemoveEventTimeCode(int stamp, Id3v2Frame *frame);

//synced tempo codes gonna need to pars this yourself
void id3v2SetSyncedTempoCodesFrameValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame);
id3buf id3v2ReadSyncedTempoCodesFrameValue(Id3v2Frame *frame);

//lang functions
void id3v2SetLanguage(id3buf lang, Id3v2Frame *frame);
id3buf id3v2ReadLanguage(Id3v2Frame *frame);

//unsynced lyrics
void id3v2SetUnsynchronizedLyrics(id3buf lyrics, unsigned int lyricLength, Id3v2Frame *frame);
id3buf id3v2ReadUnsynchronizedLyrics(Id3v2Frame *frame);

//synced lyrics
int id3v2ReadSynchronizedLyricsContentType(Id3v2Frame *frame);
id3buf id3v2ReadSynchronizedLyricsValue(Id3v2Frame *frame);
int id3v2ReadSynchronizedLyricsTimeStamp(Id3v2Frame *frame);
void id3v2RemoveSynchronizedLyric(int timeStamp, Id3v2Frame *frame);

//comment values
void id3v2SetCommentValue(id3buf comment, unsigned int commentLength, Id3v2Frame *frame);
id3buf id3v2ReadCommentValue(Id3v2Frame *frame);

//subjective frames stuff
void id3v2SetSubjectiveValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame);
id3buf id3v2ReadSubjectiveValue(Id3v2Frame *frame);
void id3v2SetRelativeVolumeAdjustmentValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame);
id3buf id3v2ReadRelativeVolumeAdjustmentValue(Id3v2Frame *frame);
void id3v2SetEqualisationValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame);
id3buf id3v2ReadEqualisationValue(Id3v2Frame *frame);
void id3v2SetReverbValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame);
id3buf id3v2ReadReverbValue(Id3v2Frame *frame);

//for frames that have file types
void id3v2SetMIMEType(id3buf format, Id3v2Frame *frame);
id3buf id3v2ReadMIMEType(Id3v2Frame *frame);

//pic type
void id3v2SetPictureType(id3byte pictureType, Id3v2Frame *frame);
int id3v2ReadPictureType(Id3v2Frame *frame);

//picture values
void id3v2SetPictureValue(id3buf picture, unsigned int pictureLength, Id3v2Frame *frame);
id3buf id3v2ReadPictureValue(Id3v2Frame *frame);

//filename 
void id3v2SetObjectFileName(id3buf fileName, unsigned int fileNameLength, Id3v2Frame *frame);
id3buf id3v2ReadObjectFileName(Id3v2Frame *frame);

//binary image
void id3v2SetGeneralEncapsulatedObjectValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame);
id3buf id3v2ReadGeneralEncapsulatedObjectValue(Id3v2Frame *frame);

//how many times a songs been played
void id3v2SetPlayCount(size_t playCount, Id3v2Frame *frame);
size_t id3v2ReadPlayCount(Id3v2Frame *frame);

//email for popm
void id3v2SetEmail(char *email, Id3v2Frame *frame);
id3buf id3v2ReadEmail(Id3v2Frame *frame);

//rating for popm
void id3v2SetRating(unsigned int rating, Id3v2Frame *frame); 
int id3v2ReadRating(Id3v2Frame *frame);

//owner identifier stuff
void id3v2SetOwnerIdentifier(char *owner, Id3v2Frame *frame);
id3buf id3v2ReadOwnerIdentifier(Id3v2Frame *frame);

//crm stuff 2.2 only
void id3v2SetEncryptedMetaValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame);
id3buf id3v2ReadEncryptedMetaValue(Id3v2Frame *frame);

//privew starts
void id3v2SetPreviewStart(void *start, Id3v2Frame *frame);
id3buf id3v2ReadPreviewStart(Id3v2Frame *frame);

//preview length
void id3v2SetPreviewLength(unsigned int previewLength, Id3v2Frame *frame);
int id3v2ReadPreviewLength(Id3v2Frame *frame);

//audio value
void id3v2SetAudioEncryptionValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame);
id3buf id3v2ReadAudioEncryptionValue(Id3v2Frame *frame);

//file id
void id3v2SetUniqueFileIdentifierValue(char *identifier, Id3v2Frame *frame);
id3buf id3v2ReadUniqueFileIdentifierValue(Id3v2Frame *frame);

//position
void id3v2SetPositionSynchronisationValue(unsigned long position, Id3v2Frame *frame);
unsigned long id3v2ReadPositionSynchronisationValue(Id3v2Frame *frame);

//eulas n such
void id3v2SetTermsOfUseValue(id3buf termsOfuse, unsigned int termsOfuseLength, Id3v2Frame *frame);
id3buf id3v2ReadTermsOfUseValue(Id3v2Frame *frame);

//price
void id3SetPrice(char *price, Id3v2Frame *frame);
id3buf id3v2ReadPrice(Id3v2Frame *frame);

//date
void id3v2SetPunchDate(char *punch, Id3v2Frame *frame);
id3buf id3v2ReadPunchDate(Id3v2Frame *frame);

//seller name
void id3v2SetSeller(id3buf seller, unsigned int sellerLength, Id3v2Frame *frame);
id3buf id3v2ReadSeller(Id3v2Frame *frame);

//date
void id3v2SetValidDate(char *validDate, Id3v2Frame *frame);
id3buf id3v2ReadValidDate(Id3v2Frame *frame);

//url
void id3v2SetContactURL(char *url, Id3v2Frame *frame);
id3buf id3v2ReadContractURL(Id3v2Frame *frame);

//delivery method
void id3v2SetCommecialDeliveryMethod(id3byte deliveryMethod, Id3v2Frame *frame);
int id3v2ReadCommecialDeliveryMethod(Id3v2Frame *frame);

//seller logo
void id3v2SetCommercialSellerLogo(id3buf logo, unsigned int logoLength, Id3v2Frame *frame);
id3buf id3v2ReadCommercialSellerLogo(Id3v2Frame *frame);

//symbols
void id3v2SetSymbol(id3byte symbol, Id3v2Frame *frame);
id3byte id3v2ReadSymbol(Id3v2Frame *frame);

//encrypt reg
void id3v2SetEncryptionRegistrationValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame);
id3buf id3v2ReadEncryptionRegistrationValue(Id3v2Frame *frame);

//group spec
void id3v2SetGroupIDValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame);
id3buf id3v2ReadGroupIDValue(Id3v2Frame *frame);

//private shhh
void id3v2SetPrivateValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame);
id3buf id3v2ReadPrivateValue(Id3v2Frame *frame);

//sig
void id3v2SetSignatureValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame);
id3buf id3v2ReadSignatureValue(Id3v2Frame *frame);

//offset
void id3v2SetOffsetToNextTag(int offset, Id3v2Frame *frame);
int id3v2ReadOffsetToNextTag(Id3v2Frame *frame);

//any binary object like pictures - get that size
unsigned int id3v2ReadFrameBinaryObjectSize(Id3v2Frame *frame);

#ifdef __cplusplus
} //extern c end
#endif

#endif
