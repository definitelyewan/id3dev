#ifndef ID3_TYPES
#define ID3_TYPES

#ifdef __cplusplus
extern "C"{
#endif

#include <stdbool.h>
#include "id3Defines.h"

/*
    Id3v2 helpers
*/

//structs
typedef struct _Id3List{
    int size;
    void (*deleteData)(void* toDelete);
    void *(*copyData)(void* toCopy);
    struct _Id3Node *head;

}Id3List;

typedef struct _Id3ListIter{
    Id3List *l;
    struct _Id3Node *curr;

}Id3ListIter;

typedef struct _Id3Node{
    void *data;
    struct _Id3Node *next;

}Id3Node;

/*
    reader types
*/

typedef struct _Id3Reader{
    id3buf buffer;
    size_t bufferSize;
    size_t cursor;

}Id3Reader; 

/*
    Id3v1 types
*/

//enums

//table is from bellow
//https://en.wikipedia.org/wiki/List_of_ID3v1_Genres
typedef enum _Genre{
    BLUES_GENRE,              
    CLASSIC_ROCK_GENRE,      
    COUNTRY_GENRE,          
    DANCE_GENRE,          
    DISCO_GENRE,           
    FUNK_GENRE,            
    GRUNGE_GENRE,           
    HIP_HOP_GENRE,         
    JAZZ_GENRE,           
    METAL_GENRE,      
    NEW_AGE_GENRE,          
    OLDIES_GENRE,      
    OTHER_GENRE,       
    POP_GENRE,           
    RHYTHM_AND_BLUES_GENRE,  
    RAP_GENRE,              
    REGGAE_GENRE,        
    ROCK_GENRE,            
    TECHNO_GENRE,         
    INDUSTRIAL_GENRE,       
    ALTERNATIVE_GENRE,        
    SKA_GENRE,               
    DEATH_METAL_GENRE,        
    PRANKS_GENRE,
    SOUNDTRACK_GENRE,
    EURO_TECHNO_GENRE,
    AMBIENT_GENRE,
    TRIP_HOP_GENRE,
    VOCAL_GENRE,
    JAZZ_AND_FUNK_GENRE,
    FUSION_GENRE,
    TRANCE_GENRE,
    CLASSICAL_GENRE,
    INSTRUMENTAL_GENRE,
    ACID_GENRE,
    HOUSE_GENRE,
    GAME_GENRE,
    SOUND_CLIP_GENRE,
    GOSPEL_GENRE,
    NOISE_GENRE,
    ALTERNATIVE_ROCK_GENRE,
    BASS_GENRE,
    SOUL_GENRE,
    PUNK_GENRE,
    SPACE_GENRE,
    MEDITATIVE_GENRE,
    INSTRUMENTAL_POP_GENRE,
    INSTRUMENTAL_ROCK_GENRE,
    ETHNIC_GENRE,
    GOTHIC_GENRE,
    DARKWAVE_GENRE,
    TECHNO_INDUSTRIAL_GENRE,
    ELECTRONIC_GENRE,
    POP_FOLK_GENRE,
    EURODANCE_GENRE,
    DREAM_GENRE,
    SOUTHERN_ROCK_GENRE,
    COMEDY_GENRE,
    CULT_GENRE,
    GANGSTA_GENRE,
    TOP_40_GENRE,
    CHRISTIAN_RAP_GENRE,
    POP_FUNK_GENRE,
    JUNGLE_MUSIC_GENRE,
    NATIVE_US_GENRE,
    CABARET_GENRE,
    NEW_WAVE_GENRE,
    PSYCHEDELIC_GENRE,
    RAVE_GENRE,
    SHOWTUNES_GENRE,
    TRAILER_GENRE,
    LO_FI_GENRE,
    TRIBAL_GENRE,
    ACID_PUNK_GENRE,
    ACID_JAZZ_GENRE,
    POLKA_GENRE,
    RETRO_GENRE,
    MUSICAL_GENRE,
    ROCKNROLL_GENRE,
    HARD_ROCK_GENRE,
    FOLK_GENRE,
    FOLK_ROCK_GENRE,
    NATIONAL_FOLK_GENRE,
    SWING_GENRE,
    FAST_FUSION_GENRE,
    BEBOP_GENRE,
    LATIN_GENRE,
    REVIVAL_GENRE,
    CELTIC_GENRE,
    BLUEGRASS_GENRE,
    AVANTGARDE_GENRE,
    GOTHIC_ROCK_GENRE,
    PROGRESSIVE_ROCK_GENRE,
    PSYCHEDELIC_ROCK_GENRE,
    SYMPHONIC_ROCK_GENRE,
    SLOW_ROCK_GENRE,
    BIG_BAND_GENRE,
    CHORUS_GENRE,
    EASY_LISTENING_GENRE,
    ACOUSTIC_GENRE,
    HUMOUR_GENRE,
    SPEECH_GENRE,
    CHANSON_GENRE,
    OPERA_GENRE,
    CHAMBER_MUSIC_GENRE,
    SONATA_GENRE,
    SYMPHONY_GENRE,
    BOOTY_BASS_GENRE,
    PRIMUS_GENRE,
    PORN_GROOVE_GENRE,
    SATIRE_GENRE,
    SLOW_JAM_GENRE,
    CLUB_GENRE,
    TANGO_GENRE,
    SAMBA_GENRE,
    FOLKLORE_GENRE,
    BALLAD_GENRE,
    POWER_BALLAD_GENRE,
    RHYTHMIC_SOUL_GENRE,
    FREESTYLE_GENRE,
    DUET_GENRE,
    PUNK_ROCK_GENRE,
    DRUM_SOLO_GENRE,
    A_CAPPELLA_GENRE,
    EURO_HOUSE_GENRE,
    DANCE_HALL_GENRE,
    GOA_MUSIC_GENRE,
    DRUM_AND_BASS_GENRE,
    CLUB_HOUSE_GENRE,
    HARDCORE_TECHNO_GENRE,
    TERROR_GENRE,
    INDIE_GENRE,
    BRITPOP_GENRE,
    NEGERPUNK_GENRE,
    POLSK_PUNK_GENRE,
    BEAT_GENRE,
    CHRISTIAN_GANGSTA_RAP_GENRE,
    HEAVY_METAL_GENRE,
    BLACK_METAL_GENRE,
    CROSSOVER_GENRE,
    CONTEMPORARY_CHRISTIAN_GENRE,
    CHRISTIAN_ROCK_GENRE,
    MERENGUE_GENRE,
    SALSA_GENRE,
    THRASH_METAL_GENRE,
    ANIME_GENRE,
    JPOP_GENRE,
    SYNTHPOP_GENRE,
    ABSTRACT_GENRE,
    ART_ROCK_GENRE,
    BAROQUE_GENRE,
    BHANGRA_GENRE,
    BIG_BEAT_GENRE,
    BREAKBEAT_GENRE,
    CHILLOUT_GENRE,
    DOWNTEMPO_GENRE,
    DUB_GENRE,
    EBM_GENRE,
    ECLECTIC_GENRE,
    ELECTRO_GENRE,
    ELECTROCLASH_GENRE,
    EMO_GENRE,
    EXPERIMENTAL_GENRE,
    GARAGE_GENRE,
    GLOBAL_GENRE,
    IDM_GENRE,
    ILLBIENT_GENRE,
    INDUSTRO_GOTH_GENRE,
    JAM_BAND_GENRE,
    KRAUTROCK_GENRE,
    LEFTFIELD_GENRE,
    LOUNGE_GENRE,
    MATH_ROCK_GENRE,
    NEW_ROMANTIC_GENRE,
    NU_BREAKZ_GENRE,
    POST_PUNK_GENRE,
    POST_ROCK_GENRE,
    PSYTRANCE_GENRE,
    SHOEGAZE_GENRE,
    SPACE_ROCK_GENRE,
    TROP_ROCK_GENRE,
    WORLD_MUSIC_GENRE,
    NEOCLASSICAL_GENRE,
    AUDIOBOOK_GENRE,
    AUDIO_THEATRE_GENRE,
    NEUE_DEUTSCHE_WELLE_GENRE,
    PODCAST_GENRE,
    INDIE_ROCK_GENRE,
    G_FUNK_GENRE,
    DUBSTEP_GENRE,
    GARAGE_ROCK_GENRE,
    PSYBIENT_GENRE

}Genre;

//structs
typedef struct _Id3v1Tag{
    id3buf title;
    id3buf artist;
    id3buf albumTitle;
    int year;
    int trackNumber;
    id3buf comment;
    Genre genre;

}Id3v1Tag;

/*
    Id3v2 hedader types
*/

//enums
typedef enum _Id3v2Headerversion{
    ID3V22 = 2,
    ID3V23 = 3,
    ID3V24 = 4,
    ID3V2INVLAIDVERSION = 0

}Id3v2HeaderVersion;

//structs
typedef struct _Id3v2ExtHeader{
    int size;
    int padding;

    //update flag values
    id3byte update;

    //crc flag values
    id3buf crc;
    unsigned int crcLen;
    
    //tag restrictions values
    id3byte tagSizeRestriction;
    id3byte encodingRestriction;
    id3byte textSizeRestriction;
    id3byte imageEncodingRestriction;
    id3byte imageSizeRestriction;

}Id3v2ExtHeader;


typedef struct _Id3v2header{
    
    //version
    int versionMajor;
    int versionMinor;
    
    //flags
    bool unsynchronisation;
    bool experimentalIndicator;
    bool footer;
    Id3v2ExtHeader *extendedHeader;

}Id3v2Header;


/*
    Id3v2 types
*/

typedef struct _Id3v2Tag{
    Id3v2Header *header;
    Id3List *frames;
    Id3ListIter *iter;

}Id3v2Tag;

/*
    Id3v2 frame types
*/

typedef enum _Id3v2FrameId{
    HUH = -1,  // got no idea man huh?
    BUF,  // Recommended buffer size
    CNT,  // Play counter
    COM,  // Comments
    CRA,  // Audio encryption
    CRM,  // Encrypted meta frame
    ETC,  // Event timing codes
    EQU,  // Equalization
    GEO,  // General encapsulated object
    IPL,  // Involved people list
    LNK,  // Linked information
    MCI,  // Music CD Identifier
    MLL,  // MPEG location lookup table
    PIC,  // Attached picture
    POP,  // Popularimeter
    REV,  // Reverb
    RVA,  // Relative volume adjustment
    SLT,  // Synchronized lyric/text
    STC,  // Synced tempo codes
    TAL,  // Album/Movie/Show title
    TBP,  // BPM (Beats Per Minute)
    TCM,  // Composer
    TCO,  // Content type
    TCR,  // Copyright message
    TDA,  // Date
    TDY,  // Playlist delay
    TEN,  // Encoded by
    TFT,  // File type
    TIM,  // Time
    TKE,  // Initial key
    TLA,  // Language(s)
    TLE,  // Length
    TMT,  // Media type
    TOA,  // Original artist(s)/performer(s)
    TOF,  // Original filename
    TOL,  // Original Lyricist(s)/text writer(s)
    TOR,  // Original release year
    TOT,  // Original album/Movie/Show title
    TP1,  // Lead artist(s)/Lead performer(s)/Soloist(s)/Performing group
    TP2,  // Band/Orchestra/Accompaniment
    TP3,  // Conductor/Performer refinement
    TP4,  // Interpreted, remixed, or otherwise modified by
    TPA,  // Part of a set
    TPB,  // Publisher
    TRC,  // ISRC (International Standard Recording Code)
    TRD,  // Recording dates
    TRK,  // Track number/Position in set
    TSI,  // Size
    TSS,  // Software/hardware and settings used for encoding
    TT1,  // Content group description
    TT2,  // Title/Songname/Content description
    TT3,  // Subtitle/Description refinement
    TXT,  // Lyricist/text writer
    TXX,  // User defined text information frame
    TYE,  // Year
    UFI,  // Unique file identifier
    ULT,  // Unsychronized lyric/text transcription
    WAF,  // Official audio file webpage
    WAR,  // Official artist/performer webpage
    WAS,  // Official audio source webpage
    WCM,  // Commercial information
    WCP,  // Copyright/Legal information
    WPB,  // Publishers official webpage
    WXX,  // User defined URL link frame
    AENC, // Audio encryption
    APIC, // Attached picture
    COMM, // Comments
    COMR, // Commercial frame
    ENCR, // Encryption method registration
    EQUA, // Equalization
    ETCO, // Event timing codes
    GEOB, // General encapsulated object
    GRID, // Group identification registration
    IPLS, // Involved people list
    LINK, // Linked information
    MCDI, // Music CD identifier
    MLLT, // MPEG location lookup table
    OWNE, // Ownership frame
    PRIV, // Private frame
    PCNT, // Play counter
    POPM, // Popularimeter
    POSS, // Position synchronisation frame
    RBUF, // Recommended buffer size
    RVAD, // Relative volume adjustment
    RVRB, // Reverb
    SYLT, // Synchronized lyric/text
    SYTC, // Synchronized tempo codes
    TALB, // Album/Movie/Show title
    TBPM, // BPM (beats per minute)
    TCOM, // Composer
    TCON, // Content type
    TCOP, // Copyright message
    TDAT, // Date
    TDLY, // Playlist delay
    TENC, // Encoded by
    TEXT, // Lyricist/Text writer
    TFLT, // File type
    TIME, // Time
    TIT1, // Content group description
    TIT2, // Title/songname/content description
    TIT3, // Subtitle/Description refinement
    TKEY, // Initial key
    TLAN, // Language(s)
    TLEN, // Length
    TMED, // Media type
    TOAL, // Original album/movie/show title
    TOFN, // Original filename
    TOLY, // Original lyricist(s)/text writer(s)
    TOPE, // Original artist(s)/performer(s)
    TORY, // Original release year
    TOWN, // File owner/licensee
    TPE1, // Lead performer(s)/Soloist(s)
    TPE2, // Band/orchestra/accompaniment
    TPE3, // Conductor/performer refinement
    TPE4, // Interpreted, remixed, or otherwise modified by
    TPOS, // Part of a set
    TPUB, // Publisher
    TRCK, // Track number/Position in set
    TRDA, // Recording dates
    TRSN, // Internet radio station name
    TRSO, // Internet radio station owner
    TSIZ, // Size
    TSRC, // ISRC (international standard recording code)
    TSSE, // Software/Hardware and settings used for encoding
    TIPL, // ID3v2.4 Involved people list
    TYER, // Year
    TXXX, // User defined text information frame
    UFID, // Unique file identifier
    USER, // Terms of use
    USLT, // Unsychronized lyric/text transcription
    WCOM, // Commercial information
    WCOP, // Copyright/Legal information
    WOAF, // Official audio file webpage
    WOAR, // Official artist/performer webpage
    WOAS, // Official audio source webpage
    WORS, // Official internet radio station homepage
    WPAY, // Payment
    WPUB, // Publishers official webpage
    WXXX, // User defined URL link frame
    RVA2, // relative volume adjustmen frame for 2.4
    EQU2, // Equalisation (2)
    ASPI, // Audio seek point index [F:4.30]
    SEEK, // Seek frame [F:4.29]
    SIGN, // Signature frame [F:4.28]

}Id3v2FrameId;

//structs 
typedef struct _Id3v2FlagContent{
    bool tagAlterPreservation;
    bool fileAlterPreservation;
    bool readOnly;
    bool unsynchronization;
    bool dataLengthIndicator;
    unsigned int decompressedSize;
    id3byte encryption;
    id3byte grouping;

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
    id3byte encoding;
    id3buf description;
    id3buf value;

}Id3v2TextBody;

//for all url frames including user generated ones (WXX)
typedef struct _Id3v2URLBody{
    id3byte encoding;
    id3buf description;
    id3buf url;

}Id3v2URLBody;

typedef struct _Id3v2InvolvedPerson{
    id3buf person;
    id3buf job;
    size_t personLen;
    size_t jobLen;

}Id3v2InvolvedPerson;

typedef struct _Id3v2InvolvedPeopleListBody{
    id3byte encoding;
    Id3List *involvedPeople;
    Id3ListIter *involvedPeopleIter;

}Id3v2InvolvedPeopleListBody;

typedef struct _Id3v2MusicCDIdentifierBody{
    id3buf cdtoc;

}Id3v2MusicCDIdentifierBody;

typedef struct _Id3v2EventTimeCodesEvent{
    id3byte typeOfEvent;
    int timeStamp;

}Id3v2EventTimesCodeEvent;

typedef struct _Id3v2EventTimeCodesBody{
    unsigned int timeStampFormat;
    Id3List *eventTimeCodes;
    Id3ListIter *eventsTimeCodesIter;

}Id3v2EventTimeCodesBody;

typedef struct _Id3v2SyncedTempoCodesBody{
    id3byte timeStampFormat;
    id3buf tempoData;
    unsigned int tempoDataLen;

}Id3v2SyncedTempoCodesBody;

typedef struct _Id3v2UnsynchronizedLyricsBody{
    id3byte encoding;
    id3buf language;
    id3buf descriptor;
    id3buf lyrics;

}Id3v2UnsynchronizedLyricsBody;

typedef struct _Id3v2SynchronizedLyricsBody{
    id3byte encoding;
    id3buf language;
    unsigned int timeStampFormat;
    unsigned int contentType;
    id3buf descriptor;
    Id3List *lyrics;
    Id3ListIter *lyricsIter;
    
}Id3v2SynchronizedLyricsBody;

typedef struct _Id3v2StampedLyric{
    id3buf text;
    int timeStamp;
    size_t lyricLen;

}Id3v2StampedLyric;

typedef struct _Id3v2CommentBody{
    id3byte encoding;
    id3buf language;
    id3buf description;
    id3buf text;

}Id3v2CommentBody;

typedef struct _Id3v2SubjectiveBody{
    id3buf value;
    int valueSize;

}Id3v2SubjectiveBody;

typedef struct _Id3v2SubjectiveBody Id3v2RelativeVolumeAdjustmentBody;
typedef struct _Id3v2SubjectiveBody Id3v2EqualisationBody;
typedef struct _Id3v2SubjectiveBody Id3v2ReverbBody;

typedef struct _Id3v2PictureBody{
    id3byte encoding;
    id3buf format;
    id3byte pictureType;
    id3buf description;
    id3buf pictureData;
    unsigned int picSize;

}Id3v2PictureBody;

typedef struct _Id3v2GeneralEncapsulatedObjectBody{
    id3byte encoding;
    id3buf mimeType;
    id3buf filename;
    id3buf contentDescription;
    id3buf encapsulatedObject;
    unsigned int encapsulatedObjectLen;

}Id3v2GeneralEncapsulatedObjectBody;

typedef struct _Id3v2PlayCounterBody{
    unsigned long counter;

}Id3v2PlayCounterBody;

typedef struct _Id3v2PopularBody{
    id3buf email;
    unsigned int rating;
    unsigned long counter;

}Id3v2PopularBody;

typedef struct _Id3v2EncryptedMetaBody{
    id3buf ownerIdentifier;
    id3buf content;
    id3buf encryptedDatablock;
    unsigned int encryptedDatablockLen;

}Id3v2EncryptedMetaBody;

typedef struct _Id3v2AudioEncryptionBody{
    id3buf ownerIdentifier;
    void *previewStart;
    unsigned int previewLength;
    id3buf encryptionInfo;
    unsigned int encryptionInfoLen;

}Id3v2AudioEncryptionBody;

typedef struct _Id3v2UniqueFileIdentifierBody{
    id3buf ownerIdentifier;
    id3buf identifier;

}Id3v2UniqueFileIdentifierBody;

typedef struct _Id3v2PositionSynchronisationBody{
    id3byte timeStampFormat;
    unsigned long pos;

}Id3v2PositionSynchronisationBody;

typedef struct _Id3v2TermsOfUseBody{
    id3byte encoding;
    id3buf language;
    id3buf text;

}Id3v2TermsOfUseBody;

typedef struct _Id3v2OwnershipBody{
    id3byte encoding;
    id3buf pricePayed;
    id3buf dateOfPunch;
    id3buf seller;

}Id3v2OwnershipBody;

typedef struct _Id3v2CommercialBody{
    id3byte encoding;
    id3buf priceString;
    id3buf validUntil;
    id3buf contractURL;
    unsigned int receivedAs;
    id3buf nameOfSeller;
    id3buf description;
    id3buf mimeType;
    id3buf sellerLogo;
    unsigned int sellerLogoLen;

}Id3v2CommercialBody;

typedef struct _Id3v2EncryptionMethodRegistrationBody{
    id3buf ownerIdentifier;
    id3byte methodSymbol;
    id3buf encryptionData;
    unsigned int encryptionDataLen;

}Id3v2EncryptionMethodRegistrationBody;

typedef struct _Id3v2GroupIDRegistrationBody{
    id3buf ownerIdentifier;
    id3byte groupSymbol;
    id3buf groupDependentData;
    unsigned int groupDependentDataLen;

}Id3v2GroupIDRegistrationBody;

typedef struct _Id3v2PrivateBody{
    id3buf ownerIdentifier;
    id3buf privateData;
    unsigned int privateDataLen;

}Id3v2PrivateBody;

typedef struct _Id3v2SignatureBody{
    id3byte groupSymbol;
    id3buf signature;
    unsigned int sigLen;

}Id3v2SignatureBody;

typedef struct _Id3v2SeekBody{
    int minimumOffsetToNextTag;
    
}Id3v2SeekBody;

//can be any frame
typedef struct _Id3v2Frame{
    Id3v2FrameHeader *header;
    void *frame;

}Id3v2Frame;

/*
    Metadata types
*/

//structures
typedef struct _Id3Metadata{
    Id3v2Tag *version2;
    Id3v1Tag *version1;
}Id3Metadata;

#ifdef __cplusplus
} //extern c end
#endif

#endif