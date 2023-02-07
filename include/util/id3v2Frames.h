#ifndef ID3V2_FRAMES
#define ID3V2_FRAMES

#ifdef __cplusplus
extern "C"{
#endif

#include "id3v2Helpers.h"
#include "id3v2Header.h"
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


#define ID3V2_MPEG_FORMAT 1
#define ID3V2_MILLISECOND_FORMAT 2

#define ID3V2_EVENT_CODE_LEN 5

#define ID3V2_DATE_FORMAT_LEN 8

typedef enum _Id3v2FrameId{
    BUF, //Recommended buffer size
    CNT, //Play counter
    COM, //Comments
    CRA, //Audio encryption
    CRM, //Encrypted meta frame
    ETC, //Event timing codes
    EQU, //Equalization
    GEO, //General encapsulated object
    IPL, //Involved people list
    LNK, //Linked information
    MCI, //Music CD Identifier
    MLL, //MPEG location lookup table
    PIC, //Attached picture
    POP, //Popularimeter
    REV, //Reverb
    RVA, //Relative volume adjustment
    SLT, //Synchronized lyric/text
    STC, //Synced tempo codes
    TAL, //Album/Movie/Show title
    TBP, //BPM (Beats Per Minute)
    TCM, //Composer
    TCO, //Content type
    TCR, //Copyright message
    TDA, //Date
    TDY, //Playlist delay
    TEN, //Encoded by
    TFT, //File type
    TIM, //Time
    TKE, //Initial key
    TLA, //Language(s)
    TLE, //Length
    TMT, //Media type
    TOA, //Original artist(s)/performer(s)
    TOF, //Original filename
    TOL, //Original Lyricist(s)/text writer(s)
    TOR, //Original release year
    TOT, //Original album/Movie/Show title
    TP1, //Lead artist(s)/Lead performer(s)/Soloist(s)/Performing group
    TP2, //Band/Orchestra/Accompaniment
    TP3, //Conductor/Performer refinement
    TP4, //Interpreted, remixed, or otherwise modified by
    TPA, //Part of a set
    TPB, //Publisher
    TRC, //ISRC (International Standard Recording Code)
    TRD, //Recording dates
    TRK, //Track number/Position in set
    TSI, //Size
    TSS, //Software/hardware and settings used for encoding
    TT1, //Content group description
    TT2, //Title/Songname/Content description
    TT3, //Subtitle/Description refinement
    TXT, //Lyricist/text writer
    TXX, //User defined text information frame
    TYE, //Year
    UFI, //Unique file identifier
    ULT, //Unsychronized lyric/text transcription
    WAF, //Official audio file webpage
    WAR, //Official artist/performer webpage
    WAS, //Official audio source webpage
    WCM, //Commercial information
    WCP, //Copyright/Legal information
    WPB, //Publishers official webpage
    WXX, //User defined URL link frame
    HUH, //got no idea man huh?
    AENC, //Audio encryption
    APIC, //Attached picture
    COMM, //Comments
    COMR, //Commercial frame
    ENCR, //Encryption method registration
    EQUA, //Equalization
    ETCO, //Event timing codes
    GEOB, //General encapsulated object
    GRID, //Group identification registration
    IPLS, //Involved people list
    LINK, //Linked information
    MCDI, //Music CD identifier
    MLLT, //MPEG location lookup table
    OWNE, //Ownership frame
    PRIV, //Private frame
    PCNT, //Play counter
    POPM, //Popularimeter
    POSS, //Position synchronisation frame
    RBUF, //Recommended buffer size
    RVAD, //Relative volume adjustment
    RVRB, //Reverb
    SYLT, //Synchronized lyric/text
    SYTC, //Synchronized tempo codes
    TALB, //Album/Movie/Show title
    TBPM, //BPM (beats per minute)
    TCOM, //Composer
    TCON, //Content type
    TCOP, //Copyright message
    TDAT, //Date
    TDLY, //Playlist delay
    TENC, //Encoded by
    TEXT, //Lyricist/Text writer
    TFLT, //File type
    TIME, //Time
    TIT1, //Content group description
    TIT2, //Title/songname/content description
    TIT3, //Subtitle/Description refinement
    TKEY, //Initial key
    TLAN, //Language(s)
    TLEN, //Length
    TMED, //Media type
    TOAL, //Original album/movie/show title
    TOFN, //Original filename
    TOLY, //Original lyricist(s)/text writer(s)
    TOPE, //Original artist(s)/performer(s)
    TORY, //Original release year
    TOWN, //File owner/licensee
    TPE1, //Lead performer(s)/Soloist(s)
    TPE2, //Band/orchestra/accompaniment
    TPE3, //Conductor/performer refinement
    TPE4, //Interpreted, remixed, or otherwise modified by
    TPOS, //Part of a set
    TPUB, //Publisher
    TRCK, //Track number/Position in set
    TRDA, //Recording dates
    TRSN, //Internet radio station name
    TRSO, //Internet radio station owner
    TSIZ, //Size
    TSRC, //ISRC (international standard recording code)
    TSSE, //Software/Hardware and settings used for encoding
    TYER, //Year
    TXXX, //User defined text information frame
    UFID, //Unique file identifier
    USER, //Terms of use
    USLT, //Unsychronized lyric/text transcription
    WCOM, //Commercial information
    WCOP, //Copyright/Legal information
    WOAF, //Official audio file webpage
    WOAR, //Official artist/performer webpage
    WOAS, //Official audio source webpage
    WORS, //Official internet radio station homepage
    WPAY, //Payment
    WPUB, //Publishers official webpage
    WXXX, //User defined URL link frame

}Id3v2FrameId;

typedef struct _Id3v2FlagContent{
    bool tagAlterPreservation;
    bool fileAlterPreservation;
    bool readOnly;
    unsigned int decompressedSize;
    unsigned char encryption;
    unsigned char grouping;

}Id3v2FlagContent;

typedef struct _Id3v2FrameHeader{
    char *id;
    unsigned int frameSize;
    unsigned int headerSize;
    enum _Id3v2FrameId idNum;
    Id3v2FlagContent *flagContent;

}Id3v2FrameHeader;

//for all text frames including user generated ones(TXX)
typedef struct _Id3v2TextBody{
    unsigned char encoding;
    unsigned char *description;
    unsigned char *value;

}Id3v2TextBody;

//for all url frames including user generated ones (WXX)
typedef struct _Id3v2URLBody{
    unsigned char encoding;
    unsigned char *description;
    unsigned char *url;

}Id3v2URLBody;

typedef struct _Id3v2SubjectiveBody{
    unsigned char *value;

}Id3v2SubjectiveBody;

typedef struct _Id3v2SubjectiveBody Id3v2RelativeVolumeAdjustmentBody;
typedef struct _Id3v2SubjectiveBody Id3v2EqualisationBody;
typedef struct _Id3v2SubjectiveBody Id3v2ReverbBody;


typedef struct _Id3v2PictureBody{
    unsigned char encoding;
    unsigned char *format;
    unsigned char pictureType;
    unsigned char *description;
    unsigned char *pictureData;
    int picSize;

}Id3v2PictureBody;

typedef struct _Id3v2PopularBody{
    unsigned char *email;
    unsigned int rating;
    unsigned char *counter;

}Id3v2PopularBody;

typedef struct _Id3v2PlayCounterBody{
    unsigned char *counter;

}Id3v2PlayCounterBody;

typedef struct _Id3v2CommentBody{
    unsigned char encoding;
    unsigned char *language;
    unsigned char *description;
    unsigned char *text;

}Id3v2CommentBody;

typedef struct _Id3v2EventTimeCodesEvent{
    unsigned char typeOfEvent;
    long timeStamp;

}Id3v2EventTimesCodeEvent;

typedef struct _Id3v2EventTimeCodesBody{
    unsigned int timeStampFormat;
    List *eventTimeCodes;

}Id3v2EventTimeCodesBody;

typedef struct _Id3v2InvolvedPeopleListBody{
    unsigned char encoding;
    unsigned char *peopleListStrings;

}Id3v2InvolvedPeopleListBody;

typedef struct _Id3v2MusicCDIdentifierBody{
    unsigned char *cdtoc;
}Id3v2MusicCDIdentifierBody;

typedef struct _Id3v2SynchronisedLyricsBody{
    unsigned char encoding;
    unsigned char *language;
    unsigned int timeStampFormat;
    unsigned int contentType;
    unsigned char *descriptor;
    List *lyrics;
    
}Id3v2SynchronisedLyricsBody;

typedef struct _Id3v2StampedLyric{
    unsigned char *text;
    long timeStamp;

}Id3v2StampedLyric;

typedef struct _Id3v2UniqueFileIdentifierBody{
    unsigned char *ownerIdentifier;
    unsigned char *identifier;

}Id3v2UniqueFileIdentifierBody;

typedef struct _Id3v2UnsynchronisedLyricsBody{
    unsigned char encoding;
    unsigned char *language;
    unsigned char *descriptor;
    unsigned char *lyrics;

}Id3v2UnsynchronisedLyricsBody;

typedef struct _Id3v2AudioEncryptionBody{
    unsigned char *ownerIdentifier;
    void *previewStart;
    unsigned int previewLength;
    unsigned char *encryptionInfo;
    unsigned int encryptionInfoLen;

}Id3v2AudioEncryptionBody;

typedef struct _Id3v2EncryptedMetaBody{
    unsigned char *ownerIdentifier;
    unsigned char *content;
    unsigned char *encryptedDatablock;
    unsigned int encryptedDatablockLen;

}Id3v2EncryptedMetaBody;

typedef struct _Id3v2SyncedTempoCodesBody{
    unsigned char timeStampFormat;
    unsigned char *tempoData;
    unsigned int tempoDataLen;

}Id3v2SyncedTempoCodesBody;

typedef struct _Id3v2GeneralEncapsulatedObjectBody{
    unsigned char encoding;
    unsigned char *mimeType;
    unsigned char *filename;
    unsigned char *contentDescription;
    unsigned char *encapsulatedObject;
    unsigned int encapsulatedObjectLen;

}Id3v2GeneralEncapsulatedObjectBody;

typedef struct _Id3v2PositionSynchronisationBody{
    unsigned char timeStampFormat;
    long pos;

}Id3v2PositionSynchronisationBody;

typedef struct _Id3v2TermsOfUseBody{
    unsigned char encoding;
    unsigned char *language;
    unsigned char *text;

}Id3v2TermsOfUseBody;

typedef struct _Id3v2OwnershipBody{
    unsigned char encoding;
    unsigned char *pricePayed;
    unsigned char *dateOfPunch;
    unsigned char *seller;

}Id3v2OwnershipBody;

typedef struct _Id3v2CommercialBody{
    unsigned char encoding;
    unsigned char *priceString;
    unsigned char *validUntil;
    unsigned char *contractURL;
    unsigned char receivedAs;
    unsigned char *nameOfSeller;
    unsigned char *description;
    unsigned char *mimeType;
    unsigned char *sellerLogo;

}Id3v2CommercialBody;

typedef struct _Id3v2EncryptionMethodRegistrationBody{
    unsigned char *ownerIdentifier;
    unsigned char methodSymbol;
    unsigned char *encryptionData;
    unsigned int encryptionDataLen;

}Id3v2EncryptionMethodRegistrationBody;


typedef struct _Id3v2GroupIDRegistrationBody{
    unsigned char *ownerIdentifier;
    unsigned char groupSymbol;
    unsigned char *groupDependentData;
    unsigned int groupDependentDataLen;

}Id3v2GroupIDRegistrationBody;

typedef struct _Id3v2PrivateBody{
    unsigned char *ownerIdentifier;
    unsigned char *privateData;
    unsigned int privateDataLen;

}Id3v2PrivateBody;

//can be any frame
typedef struct _Id3v2Frame{
    Id3v2FrameHeader *header;
    void *frame;

}Id3v2Frame;

//frame functions
List *id3v2ExtractFrames(const char *filePath, Id3v2Header *header);
Id3v2Frame *id3v2NewFrame(Id3v2FrameHeader *header, void *bodyContent);
Id3v2Frame *id3v2ParseFrame(unsigned char *buffer, Id3v2Header *header);
void id3v2FreeFrame(void *toDelete);

//header frame functions
Id3v2FrameHeader *id3v2ParseFrameHeader(unsigned char *buffer, Id3v2Header *header);
Id3v2FrameHeader *id3v2NewFrameHeader(char *id, unsigned int frameSize, unsigned int headerSize, Id3v2FlagContent *flagContent);
void id3v2FreeFrameHeader(Id3v2FrameHeader *toDelete);

//flag content functions
unsigned int id3v2SizeOfFlagContent(Id3v2FlagContent *content);
Id3v2FlagContent *id3v2NewFlagContent(bool tagAlterPreservation, bool fileAlterPreservation, bool readOnly, unsigned int decompressedSize, unsigned char encryption, unsigned char grouping);
Id3v2FlagContent *id3v2ParseFlagContent(unsigned char *buffer);
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
Id3v2CommentBody *id3v2ParseCommentBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2CommentBody *id3v2NewCommentBody(unsigned char encoding, unsigned char *language, unsigned char *description, unsigned char *text);
void id3v2FreeCommentFrame(Id3v2Frame *toDelete);

//picture frame functions
Id3v2Frame *id3v2ParsePictureFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2PictureBody *id3v2ParsePictureBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2PictureBody *id3v2NewPictureBody(unsigned char encoding, unsigned char *format, unsigned char pictureType, unsigned char *description, unsigned char *pictureData, int picSize);
void id3v2FreePictureFrame(Id3v2Frame *toDelete);

//general encapsulated object frame functions
Id3v2Frame *id3v2ParseGeneralEncapsulatedObjectFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2GeneralEncapsulatedObjectBody *id3v2NewGeneralEncapsulatedObjectBody(unsigned char encoding, unsigned char *mimeType, unsigned char *filename, unsigned char *contentDescription, unsigned char *encapsulatedObject, unsigned int encapsulatedObjectLen);
Id3v2GeneralEncapsulatedObjectBody *id3v2ParseGeneralEncapsulatedObjectBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
void id3v2FreeGeneralEncapsulatedObjectFrame(Id3v2Frame *toDelete);

//play counter frame functions
Id3v2Frame *id3v2ParsePlayCounterFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2PlayCounterBody *id3v2ParsePlayCounterBody(unsigned char *buffer, Id3v2FrameHeader *fhrameHeader);
Id3v2PlayCounterBody *id3v2NewPlayCounterBody(unsigned char *counter);
void id3v2FreePlayCounterFrame(Id3v2Frame *toDelete);

//popularmeter frame functions
Id3v2Frame *id3v2ParsePopularFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2PopularBody *id3v2NewPopularBody(unsigned char *email, unsigned int rating, unsigned char *counter);
Id3v2PopularBody *id3v2ParsePopularBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
void id3v2FreePopularFrame(Id3v2Frame *toDelete);

//encrypted meta frame functions 2.2 only
Id3v2Frame *id3v2ParseEncryptedMetaFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2EncryptedMetaBody *id3v2NewEncryptedMetaBody(unsigned char *ownerIdentifier, unsigned char *content, unsigned char *encryptedDatablock, unsigned int encryptedDatablockLen);
Id3v2EncryptedMetaBody *id3v2ParseEncryptedMetaBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
void id3v2FreeEncryptedMetaFrame(Id3v2Frame *toDelete);

//audio encryption frame functions
Id3v2Frame *id3v2ParseAudioEncryptionFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2AudioEncryptionBody *id3v2ParseAudioEncryptionBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2AudioEncryptionBody *id3v2NewAudioEncryptionBody(unsigned char *ownerIdentifier, void *previewStart, unsigned int previewLength, unsigned char *encryptionInfo, unsigned int encryptionInfoLen);
void id3v2FreeAudioEncryptionFrame(Id3v2Frame *toDelete);

//unique file identifier frame functions
Id3v2Frame *id3v2ParseUniqueFileIdentiferFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2UniqueFileIdentifierBody *id3v2ParseUniqueFileIdentiferBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2UniqueFileIdentifierBody *id3v2NewUniqueFileIdentifierBody(unsigned char *ownerIdentifier, unsigned char *identifier);
void id3v2FreeUniqueFileIdentifierFrame(Id3v2Frame *toDelete);

//position synchronisation frame ^2.3
Id3v2Frame *id3v2ParsePositionSynchronisationFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2PositionSynchronisationBody *id3v2ParsePositionSynchronisationBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2PositionSynchronisationBody *id3v2NewPositionSynchronisationBody(unsigned char timeStampFormat, long pos);
void id3v2FreePositionSynchronisationFrame(Id3v2Frame *toDelete);

//terms of service frame functions ^2.3
Id3v2Frame *id3v2ParseTermsOfUseFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2TermsOfUseBody *id3v2ParseTermsOfUseBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2TermsOfUseBody *id3v2NewTermsOfUseBody(unsigned char encoding, unsigned char *language, unsigned char *text);
void id3v2FreeTermsOfUseFrame(Id3v2Frame *toDelete);

//ownership frame functions ^2.3
Id3v2Frame *id3v2ParseOwnershipFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2OwnershipBody *id3v2ParseOwnershipBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2OwnershipBody *id3v2NewOwnershipBody(unsigned char encoding, unsigned char *pricePayed, unsigned char *dateOfPunch, unsigned char *seller);
void id3v2FreeOwnershipFrame(Id3v2Frame *toDelete);

//commercial frame functions ^2.3
Id3v2Frame *id3v2ParseCommercialFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2CommercialBody *id3v2ParseCommercialBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2CommercialBody *id3v2NewCommercialBody(unsigned char encoding, unsigned char *priceString, unsigned char *validUntil, unsigned char *contractURL, unsigned char receivedAs, unsigned char *nameOfSeller, unsigned char *description, unsigned char *mimeType, unsigned char *sellerLogo);
void id3v2FreeCommercialFrame(Id3v2Frame *toDelete);

//encryption method registration frame functions ^2.3
Id3v2Frame *id3v2ParseEncryptionMethodRegistrationFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2EncryptionMethodRegistrationBody *id3v2ParseEncryptionMethodRegistrationBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2EncryptionMethodRegistrationBody *id3v2NewEncryptionMethodRegistrationBody(unsigned char *ownerIdentifier, unsigned char methodSymbol, unsigned char *encryptionData, unsigned int encryptionDataLen);
void id3v2FreeEncryptionMethodRegistrationFrame(Id3v2Frame *toDelete);

//group id registration frame functions ^2.3
Id3v2Frame *id3v2ParseGroupIDRegistrationFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2GroupIDRegistrationBody *id3v2ParseGroupIDRegistrationBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2GroupIDRegistrationBody *id3v2NewGroupIDRegistrationBody(unsigned char *ownerIdentifier, unsigned char groupSymbol, unsigned char *groupDependentData, unsigned int groupDependentDataLen);
void id3v2FreeGroupIDRegistrationFrame(Id3v2Frame *toDelete);

//private frame functions ^2.3
Id3v2Frame *id3v2ParsePrivateFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2PrivateBody *id3v2ParsePrivateBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2PrivateBody *id3v2NewPrivateBody(unsigned char *ownerIdentifier, unsigned char *privateData, unsigned int privateDataLen);
void id3v2FreePrivateFrame(Id3v2Frame *toDelete);

//subjective frame functions
Id3v2Frame *id3v2ParseSubjectiveFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2SubjectiveBody *id3v2ParseSubjectiveBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2SubjectiveBody *id3v2NewSubjectiveBody(unsigned char *value);
void id3v2FreeSubjectiveFrame(Id3v2Frame *toDelete);

//Relative volume adjustment frame functions
Id3v2Frame *id3v2ParseRelativeVolumeAdjustmentFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2RelativeVolumeAdjustmentBody *id3v2ParseRelativeVolumeAdjustmentBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2RelativeVolumeAdjustmentBody *id3v2NewRelativeVolumeAdjustmentBody(unsigned char *value);
void id3v2FreeRelativeVolumeAdjustmentFrame(Id3v2Frame *toDelete);

//Equalisation frame functions
Id3v2Frame *id3v2ParseEqualisationFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2EqualisationBody *id3v2ParseEqualisationBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2EqualisationBody *id3v2NewEqualisationBody(unsigned char *value);
void id3v2FreeEqualisationFrame(Id3v2Frame *toDelete);

//Reverb frame functions
Id3v2Frame *id3v2ParseReverbFrame(unsigned char *buffer, Id3v2Header *header);
Id3v2ReverbBody *id3v2ParseReverbBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader);
Id3v2ReverbBody *id3v2NewReverbBody(unsigned char *value);
void id3v2FreeReverbFrame(Id3v2Frame *toDelete);

//other functions
Id3v2FrameId id3v2FrameIdFromStr(char *str);
int id3v2IdAndSizeOffset(Id3v2Header *header);

#ifdef __cplusplus
} //extern c end
#endif

#endif