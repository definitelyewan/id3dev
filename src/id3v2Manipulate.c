#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v2Manipulate.h"
#include "id3Helpers.h"
#include "id3Defines.h"
#include "id3Reader.h" 

//checks
bool id3v2ManipExtErrorChecks(Id3v2Tag *tag){
    if(tag == NULL){
        return true;
    }

    if(tag->header == NULL){
        return true;
    }

    if(tag->header->extendedHeader == NULL){
        return true;
    }

    return false;
}

bool id3v2ManipHeaderErrorChecks(Id3v2Tag *tag){
    if(tag == NULL){
        return true;
    }

    if(tag->header == NULL){
        return true;
    }

    return false;
}

bool id3v2ManipFlagContentErrorChecks(Id3v2Frame *frame){
    if(frame == NULL){
        return true;
    }

    if(frame->header == NULL){
        return true;
    }

    if(frame->header->flagContent == NULL){
        return true;
    }

    return false;
}

bool id3v2ManipFrameHeaderErrorChecks(Id3v2Frame *frame){
    if(frame == NULL){
        return true;
    }

    if(frame->header == NULL){
        return true;
    }

    return false;
}

bool id3v2ManipFullFrameErrorChecks(Id3v2Frame *frame){

    if(id3v2ManipFrameHeaderErrorChecks(frame) == true){
        return true;
    }

    if(frame->frame == NULL){
        return true;
    }

    return false;
}

//compatability functions a.k.a getters

/*
    extended header
*/

int id3v2GetTagSizeRestriction(Id3v2Tag *tag){
    return (id3v2ManipExtErrorChecks(tag) == true) ? 0 : tag->header->extendedHeader->size;
}
int id3v2GetEncodingRestriction(Id3v2Tag *tag){
    return (id3v2ManipExtErrorChecks(tag) == true) ? 0 : tag->header->extendedHeader->encodingRestriction;
}
int id3v2GetTextSizeRestriction(Id3v2Tag *tag){
    return (id3v2ManipExtErrorChecks(tag) == true) ? 0 : tag->header->extendedHeader->textSizeRestriction;
}
int id3v2GetImageEncodingRestriction(Id3v2Tag *tag){
    return (id3v2ManipExtErrorChecks(tag) == true) ? 0 : tag->header->extendedHeader->imageEncodingRestriction;
}

int id3v2GetImageSizeRestriction(Id3v2Tag *tag){
    return (id3v2ManipExtErrorChecks(tag) == true) ? 0 : tag->header->extendedHeader->imageSizeRestriction;
}

unsigned char *id3v2GetCrc(Id3v2Tag *tag){
    return (id3v2ManipExtErrorChecks(tag) == true) ? NULL : tag->header->extendedHeader->crc;

}

/*
    header
*/

int id3v2GetVersion(Id3v2Tag *tag){

    if(id3v2ManipHeaderErrorChecks(tag) == true){
        return 0;
    }

    //build a string
    char str[ID3V2_VERSION_SIZE_OF_BYTES+1];
    snprintf(str, ID3V2_VERSION_SIZE_OF_BYTES+1, "%d%d",tag->header->versionMajor,tag->header->versionMinor);

    return strtol(str, NULL, 10); //base 10
}

bool id3v2UnsynchronizedIndicator(Id3v2Tag *tag){
    
    if(id3v2ManipHeaderErrorChecks(tag) == true){
        return false;
    }

    return (id3v2ManipHeaderErrorChecks(tag) == true) ? false : tag->header->unsynchronisation;
}

bool id3v2ExperimentalIndicator(Id3v2Tag *tag){
    return (id3v2ManipHeaderErrorChecks(tag) == true) ? false : tag->header->experimentalIndicator;
}

bool id3v2ExtendedIndicator(Id3v2Tag *tag){
    return (id3v2ManipHeaderErrorChecks(tag) == true) ? false : tag->header->extendedHeader;
}

bool id3v2FooterIndicator(Id3v2Tag *tag){
    return (id3v2ManipHeaderErrorChecks(tag) == true) ? false : tag->header->footer;
}

size_t id3v2GetTagSize(Id3v2Tag *tag){
    return (id3v2ManipHeaderErrorChecks(tag) == true) ? 0 : tag->header->size;
}

/*
    frame flag content
*/

bool id3v2FrameAlterPreservationIndicator(Id3v2Frame *frame){
    return (id3v2ManipFlagContentErrorChecks(frame) == true) ? false : frame->header->flagContent->tagAlterPreservation;
}

bool id3v2FrameFileAlterPreservationIndicator(Id3v2Frame *frame){
    return (id3v2ManipFlagContentErrorChecks(frame) == true) ? false : frame->header->flagContent->fileAlterPreservation;
}

bool id3v2FrameReadOnlyIndicator(Id3v2Frame *frame){
    return (id3v2ManipFlagContentErrorChecks(frame) == true) ? false : frame->header->flagContent->readOnly;
}

bool id3v2FrameUnsynchronizationIndicator(Id3v2Frame *frame){
    return (id3v2ManipFlagContentErrorChecks(frame) == true) ? false : frame->header->flagContent->unsynchronization;
}

size_t id3v2GetFrameDataLengthSize(Id3v2Frame *frame){
    return (id3v2ManipFlagContentErrorChecks(frame) == true) ? false : (frame->header->flagContent->dataLengthIndicator == false) ? 0 : frame->header->flagContent->decompressedSize;
}

unsigned char id3v2GetFrameEncryptionMethod(Id3v2Frame *frame){
    return (id3v2ManipFlagContentErrorChecks(frame) == true) ? 0x00 : frame->header->flagContent->encryption;
}

unsigned char id3v2GetFrameGroup(Id3v2Frame *frame){
    return (id3v2ManipFlagContentErrorChecks(frame) == true) ? 0x00 : frame->header->flagContent->grouping;
}

/*
    frame header
*/

char *id3v2GetFrameStrID(Id3v2Frame *frame){
    return (id3v2ManipFrameHeaderErrorChecks(frame) == true) ? NULL : frame->header->id;
}

size_t id3v2GetFrameSize(Id3v2Frame *frame){
    return (id3v2ManipFrameHeaderErrorChecks(frame) == true) ? 0 : frame->header->frameSize;
}

size_t id3v2GetFrameHeaderSize(Id3v2Frame *frame){
    return (id3v2ManipFrameHeaderErrorChecks(frame) == true) ? 0 : frame->header->headerSize;
}

Id3v2FrameId id3v2GetFrameID(Id3v2Frame *frame){
    return (id3v2ManipFrameHeaderErrorChecks(frame) == true) ? HUH : frame->header->idNum;
}


int id3v2GetFrameEncoding(Id3v2Frame *frame){

    if(frame == NULL){
        return -1;
    }

    if(id3v2ManipFrameHeaderErrorChecks(frame) == true){
        return -1;
    }
    
    int id = 0;

    if(frame->header->id[0] == 'T'){
        //there are lots of text frames so this will be the generalization
        id = TXXX;
    }else{
        id = frame->header->idNum;
    }

    //only these frames use an encoding
    switch(id){
        case COM:
            return (int)((Id3v2CommentBody *)frame->frame)->encoding;
        case GEO:
            return (int)((Id3v2GeneralEncapsulatedObjectBody *)frame->frame)->encoding;
        case IPL:
            return (int)((Id3v2InvolvedPeopleListBody *)frame->frame)->encoding;
        case PIC:
            return (int)((Id3v2PictureBody *)frame->frame)->encoding;
        case SLT:
            return (int)((Id3v2SynchronizedLyricsBody *)frame->frame)->encoding;
        case ULT: 
            return (int)((Id3v2UnsynchronizedLyricsBody *)frame->frame)->encoding;
        case WXX:
            return (int)((Id3v2URLBody *)frame->frame)->encoding;  
        case APIC:
            return (int)((Id3v2PictureBody *)frame->frame)->encoding;
        case COMM:
            return (int)((Id3v2CommentBody *)frame->frame)->encoding; 
        case COMR:
            return (int)((Id3v2CommercialBody *)frame->frame)->encoding; 
        case GEOB:
            return (int)((Id3v2GeneralEncapsulatedObjectBody *)frame->frame)->encoding; 
        case IPLS:
            return (int)((Id3v2InvolvedPeopleListBody *)frame->frame)->encoding; 
        case OWNE:
            return (int)((Id3v2OwnershipBody *)frame->frame)->encoding; 
        case SYLT:
            return (int)((Id3v2SynchronizedLyricsBody *)frame->frame)->encoding;
        case TXXX:
            return (int)((Id3v2TextBody *)frame->frame)->encoding;
        case USER:
            return (int)((Id3v2TermsOfUseBody *)frame->frame)->encoding;
        case USLT:
            return (int)((Id3v2UnsynchronizedLyricsBody *)frame->frame)->encoding;
        case WXXX: 
            return (int)((Id3v2URLBody *)frame->frame)->encoding;
        default: 
            break;

    }

    return -1;
}

unsigned char *id3v2GetFrameDescription(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    if(frame->frame == NULL){
        return NULL;
    }

    if(frame->header == NULL){
        return NULL;
    }

    unsigned char *ptr = NULL;
    unsigned char *ret = NULL;
    int encoding = id3v2GetFrameEncoding(frame);
    
    //only these frames use a description of descriptor
    switch(frame->header->idNum){
        case TXX:
            ptr = ((Id3v2TextBody *)frame->frame)->description;
            break;
        case WXX:
            ptr = ((Id3v2URLBody *)frame->frame)->description;
            break;
        case ULT:
            ptr = ((Id3v2UnsynchronizedLyricsBody *)frame->frame)->descriptor;
            break;
        case SLT:
            ptr = ((Id3v2SynchronizedLyricsBody *)frame->frame)->descriptor;
            break;
        case COM:
            ptr = ((Id3v2CommentBody *)frame->frame)->description;
            break;
        case PIC:
            ptr = ((Id3v2PictureBody *)frame->frame)->description;
            break;
        case GEO:
            ptr = ((Id3v2GeneralEncapsulatedObjectBody *)frame->frame)->contentDescription;
            break;
        case TXXX:
            ptr = ((Id3v2TextBody *)frame->frame)->description;
            break;
        case WXXX:
            ptr = ((Id3v2URLBody *)frame->frame)->description;
            break;
        case USLT:
            ptr = ((Id3v2UnsynchronizedLyricsBody *)frame->frame)->descriptor;
            break;
        case SYLT:
            ptr = ((Id3v2SynchronizedLyricsBody *)frame->frame)->descriptor;
            break;
        case COMM:
            ptr = ((Id3v2CommentBody *)frame->frame)->description;
            break;
        case APIC:
            ptr = ((Id3v2PictureBody *)frame->frame)->description;
            break;
        case GEOB:
            ptr = ((Id3v2GeneralEncapsulatedObjectBody *)frame->frame)->contentDescription;
            break;
        case WCOM:
            ptr = ((Id3v2CommercialBody *)frame->frame)->description;
            break;
        default:
            ptr = NULL;
            break;
    }

    if(ptr == NULL){
        return NULL;
    }

    ret = calloc(sizeof(char), id3strlen(ptr, encoding) + id3ReaderAllocationAdd(encoding));
    memcpy(ret, ptr, id3strlen(ptr, encoding));

    return ret;
}

unsigned char *id3v2GetTextFrameValue(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    if(frame->frame == NULL){
        return NULL;
    }

    Id3v2TextBody *body = (Id3v2TextBody *)frame->frame;
    
    if(body->value == NULL){
        return NULL;
    }
    
    unsigned char *ret = calloc(sizeof(unsigned char), id3strlen(body->value, body->encoding) + id3ReaderAllocationAdd(body->encoding));
    memcpy(ret, body->value, id3strlen(body->value, body->encoding));

    return ret;
}   

unsigned char *id3v2GetURLFrameValue(Id3v2Frame *frame){
    
    if(frame == NULL){
        return NULL;
    }

    if(frame->frame == NULL){
        return NULL;
    }

    Id3v2URLBody *body = (Id3v2URLBody *)frame->frame;
    
    if(body->url == NULL){
        return NULL;
    }

    unsigned char *ret = calloc(sizeof(unsigned char), id3strlen(body->url, ISO_8859_1) + 1);
    memcpy(ret, body->url, id3strlen(body->url, ISO_8859_1));

    return ret;
}

unsigned char *id3v2GetInvolvedPeopleListFrameValue(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    if(frame->frame == NULL){
        return NULL;
    }

    switch(id3v2GetFrameID(frame)){
        case IPL:
            break;
        case IPLS:
            break;
        default:
            return NULL;
    }  

    Id3v2InvolvedPeopleListBody *body = (Id3v2InvolvedPeopleListBody *)frame->frame; 
    int encoding = body->encoding;
    unsigned char *ret = calloc(sizeof(unsigned char), id3strlen(body->peopleListStrings, encoding) + id3ReaderAllocationAdd(encoding));
    memcpy(ret, body->peopleListStrings, id3strlen(body->peopleListStrings, encoding));

    return ret;
}

unsigned char *id3v2GetCDIDFrameValue(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    switch(id3v2GetFrameID(frame)){
        case MCI:
            break;
        case MCDI:
            break;
        default:
            return NULL;
    }   

    Id3v2MusicCDIdentifierBody *body = (Id3v2MusicCDIdentifierBody *)frame->frame; 
    unsigned char *ret = calloc(sizeof(unsigned char), id3strlen(body->cdtoc, ISO_8859_1) + 1);
    memcpy(ret, body->cdtoc, id3strlen(body->cdtoc, ISO_8859_1));

    return ret;   
}

int id3v2GetFrameTimeStampFormat(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return 0;
    }

    //only these frames use time stamp format
    switch(frame->header->idNum){
        case ETC:
            return (int)((Id3v2EventTimeCodesBody *)frame->frame)->timeStampFormat;
        case ETCO:
            return (int)((Id3v2EventTimeCodesBody *)frame->frame)->timeStampFormat;
        case STC:
            return (int)((Id3v2SyncedTempoCodesBody *)frame->frame)->timeStampFormat;
        case SYTC:
            return (int)((Id3v2SyncedTempoCodesBody *)frame->frame)->timeStampFormat;
        case SLT:
            return (int)((Id3v2SynchronizedLyricsBody *)frame->frame)->timeStampFormat;
        case SYLT:
            return (int)((Id3v2SynchronizedLyricsBody *)frame->frame)->timeStampFormat;
        case POSS:
            return (int)((Id3v2PositionSynchronisationBody *)frame->frame)->timeStampFormat;
        default:
            return 0;
    }
}

unsigned char id3v2GetEventTimeCodeType(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return 0x00;
    }

    switch(id3v2GetFrameID(frame)){
        case ETC:
            break;
        case ETCO:
            break;
        default:
            return 0x00;
    }  

    Id3v2EventTimeCodesBody *body = (Id3v2EventTimeCodesBody *)frame->frame;

    if(hasNextListIter(body->eventsTimeCodesIter)){
        Id3v2EventTimesCodeEvent *event = (Id3v2EventTimesCodeEvent *)nextListIter(body->eventsTimeCodesIter);

        return event->typeOfEvent;
    }

    return 0x00;
}
long id3v2GetEventTimeCodeTimeStamp(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return -1;
    }

    switch(id3v2GetFrameID(frame)){
        case ETC:
            break;
        case ETCO:
            break;
        default:
            return -1;
    }  

    Id3v2EventTimeCodesBody *body = (Id3v2EventTimeCodesBody *)frame->frame;

    if(hasNextListIter(body->eventsTimeCodesIter)){
        Id3v2EventTimesCodeEvent *event = (Id3v2EventTimesCodeEvent *)nextListIter(body->eventsTimeCodesIter);

        return event->timeStamp;
    }

    return -1;
}


void id3v2ResetEventTimeCodeIter(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return;
    }

    switch(id3v2GetFrameID(frame)){
        case ETC:
            break;
        case ETCO:
            break;
        default:
            return;
    }  

    Id3v2EventTimeCodesBody *body = (Id3v2EventTimeCodesBody *)frame->frame;

    freeListIter(body->eventsTimeCodesIter);

    ListIter *li = newListIter(body->eventTimeCodes);

    body->eventsTimeCodesIter = li;

}



unsigned char *id3v2GetSyncedTempoCodesFrameValue(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    Id3v2SyncedTempoCodesBody *body = (Id3v2SyncedTempoCodesBody *)frame->frame;
    unsigned char *data = malloc((sizeof(unsigned char)*body->tempoDataLen)+1);
    memcpy(data, body->tempoData, body->tempoDataLen);
    
    return data;
}




unsigned char *id3v2GetFrameLanguage(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }
    
    unsigned char *lang = NULL;
    unsigned char *ret = NULL;
    switch(id3v2GetFrameID(frame)){
        case ULT:
            lang = ((Id3v2UnsynchronizedLyricsBody *)frame->frame)->language;
            break;
        case USLT:
            lang = ((Id3v2UnsynchronizedLyricsBody *)frame->frame)->language;
            break;
        case SLT:
            lang = ((Id3v2SynchronizedLyricsBody *)frame->frame)->language;
            break;
        case SYLT:
            lang = ((Id3v2SynchronizedLyricsBody *)frame->frame)->language;
            break;
        case COM:
            lang = ((Id3v2CommentBody *)frame->frame)->language;
            break;
        case COMM:
            lang = ((Id3v2CommentBody *)frame->frame)->language;
            break;
        case USER:
            lang = ((Id3v2TermsOfUseBody *)frame->frame)->language;
            break;
        default:
            return NULL;
    }

    ret = calloc(sizeof(unsigned char), ID3V2_LANGUAGE_LEN+1);
    memcpy(ret, lang, ID3V2_LANGUAGE_LEN);

    return ret;
}

unsigned char *id3v2GetFrameUnsynchronizedLyrics(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    unsigned char *ptr = NULL;
    unsigned char *ret = NULL;
    int encoding = 0;

    switch(id3v2GetFrameID(frame)){
        case ULT:
            ptr = ((Id3v2UnsynchronizedLyricsBody *)frame->frame)->lyrics;
            break;
        case USLT:
            ptr = ((Id3v2UnsynchronizedLyricsBody *)frame->frame)->lyrics;
            break;
        default:
            return NULL;
    }

    encoding = id3v2GetFrameEncoding(frame);
    ret = calloc(sizeof(unsigned char), id3strlen(ptr, encoding) + id3ReaderAllocationAdd(encoding));
    memcpy(ret, ptr, id3strlen(ptr, encoding));

    return ret;
}