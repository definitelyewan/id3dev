#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v2Manipulate.h"
#include "id3Helpers.h"
#include "id3Defines.h"
#include "id3v2Frames.h"
#include "id3v2Header.h"
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

//util
Id3v2Frame *id3v2IterTag(Id3v2Tag *tag){

    if(tag == NULL){
        return NULL;
    }

    if(id3HasNextListIter(tag->iter)){
        return  (Id3v2Frame *)id3NextListIter(tag->iter);
    }

    return NULL;
}

void id3v2ResetIterTag(Id3v2Tag *tag){
    
    if(tag == NULL){
        return;
    }
    
    Id3ListIter *iterN = id3NewListIter(tag->frames);
    id3FreeListIter(tag->iter);
    tag->iter = iterN;
}

Id3List *id3v2SearchForFrames(Id3v2Tag *tag, Id3v2FrameId id){
    
    if(tag == NULL){
        return NULL;
    }
    
    if(tag->frames == NULL || tag->iter == NULL){
        return NULL;
    }

    Id3v2Frame *currFrame = NULL;
    Id3List *idList = id3NewList(id3v2FreeFrame,id3v2CopyFrame);

    while((currFrame = id3v2IterTag(tag)) != NULL){

        if(id3v2GetFrameID(currFrame) == id){
            Id3v2Frame *copy = id3v2CopyFrame((void *)currFrame);
            id3PushList(idList, (void *)copy);
        }
    }
    
    id3v2ResetIterTag(tag);
    return idList;
}

void id3v2SavePicture(const char *fileName, Id3v2Frame *frame){

    if(fileName == NULL){
        return;
    }

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return;
    }

    switch(id3v2GetFrameID(frame)){
        case PIC:
            break;
        case APIC:
            break;
        default:
            return;
    }

    FILE *fp = NULL;
    Id3v2PictureBody *body = (Id3v2PictureBody *)frame->frame;

    if((fp = fopen(fileName,"wb")) == NULL){
        return;
    }

    fwrite(body->pictureData,1,body->picSize,fp);
    fclose(fp);
}

void id3v2SaveEncapsulatedObject(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return;
    }

    switch(id3v2GetFrameID(frame)){
        case GEO:
            break;
        case GEOB:
            break;
        default:
            return;
    }

    FILE *fp = NULL;
    Id3v2GeneralEncapsulatedObjectBody *body = (Id3v2GeneralEncapsulatedObjectBody *)frame->frame;

    char *name = NULL;
    id3buf mime = id3v2GetMIMEType(frame);
    id3buf fileName = id3v2GetObjectFileName(frame);
    int nameLen = id3strlen(fileName, id3v2GetEncoding(frame));
    int mimeLen = strlen((char *)mime);

    int mark = 0;
    name = calloc(sizeof(id3byte), nameLen + mimeLen + 1);
    for(int i = 0; i < nameLen; i++){
        
        if(!(fileName[i] == 0x00 || fileName[i] == 0xFF || fileName[i] == 0xFE)){
            name[mark] = fileName[i];
            mark++;
        }
    }
    name[mark++] = '.';

    for(int i = 0; i < mimeLen; i++){
        name[mark] = mime[i];
        mark++;
    }
    name[mark] = '\0';

    if((fp = fopen(name,"wb")) == NULL){
        return;
    }

    fwrite(body->encapsulatedObject,1,body->encapsulatedObjectLen,fp);
    fclose(fp);
    
   free(mime);
   free(fileName);
   free(name);
}

void id3v2AddEventToEventFrame(Id3v2Frame *eventCodeFrame, id3byte typeOfEvent, long timeStamp){

    if(eventCodeFrame == NULL){
        return;
    }
    //printf("%d\n",typeOfEvent);
    Id3v2EventTimesCodeEvent *event = NULL;
    Id3v2EventTimeCodesBody *body = NULL;
    Id3List *list = NULL;

    if(eventCodeFrame->frame == NULL){
        return;
    }

    body = (Id3v2EventTimeCodesBody *)eventCodeFrame->frame;
    event = id3v2NewEventCodeEvent(typeOfEvent, timeStamp);
    
    
    if(body->eventTimeCodes == NULL){
        list = id3NewList(id3v2FreeEventCode,id3v2CopyEventCodeEvent);
        body->eventTimeCodes = list;
    }

    eventCodeFrame->header->frameSize = eventCodeFrame->header->frameSize + 1 + 4;
    
    id3PushList(body->eventTimeCodes, (void *)event);
    id3v2ResetEventTimeCodeIter(eventCodeFrame);
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

id3buf id3v2GetCrc(Id3v2Tag *tag){
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

bool id3v2GetUnsynchronizedIndicator(Id3v2Tag *tag){
    
    if(id3v2ManipHeaderErrorChecks(tag) == true){
        return false;
    }

    return (id3v2ManipHeaderErrorChecks(tag) == true) ? false : tag->header->unsynchronisation;
}

bool id3v2GetExperimentalIndicator(Id3v2Tag *tag){
    return (id3v2ManipHeaderErrorChecks(tag) == true) ? false : tag->header->experimentalIndicator;
}

bool id3v2GetExtendedIndicator(Id3v2Tag *tag){
    return (id3v2ManipHeaderErrorChecks(tag) == true) ? false : (tag->header->extendedHeader == NULL) ? false: true;
}

bool id3v2GetFooterIndicator(Id3v2Tag *tag){
    return (id3v2ManipHeaderErrorChecks(tag) == true) ? false : tag->header->footer;
}

size_t id3v2GetTagSize(Id3v2Tag *tag){
    return (id3v2ManipHeaderErrorChecks(tag) == true) ? 0 : tag->header->size;
}

/*
    frame flag content
*/

bool id3v2GetFrameAlterPreservationIndicator(Id3v2Frame *frame){
    return (id3v2ManipFlagContentErrorChecks(frame) == true) ? false : frame->header->flagContent->tagAlterPreservation;
}

bool id3v2GetFrameFileAlterPreservationIndicator(Id3v2Frame *frame){
    return (id3v2ManipFlagContentErrorChecks(frame) == true) ? false : frame->header->flagContent->fileAlterPreservation;
}

bool id3v2GetFrameReadOnlyIndicator(Id3v2Frame *frame){
    return (id3v2ManipFlagContentErrorChecks(frame) == true) ? false : frame->header->flagContent->readOnly;
}

bool id3v2GetFrameUnsynchronizationIndicator(Id3v2Frame *frame){
    return (id3v2ManipFlagContentErrorChecks(frame) == true) ? false : frame->header->flagContent->unsynchronization;
}

size_t id3v2GetFrameDataLengthSize(Id3v2Frame *frame){
    return (id3v2ManipFlagContentErrorChecks(frame) == true) ? false : (frame->header->flagContent->dataLengthIndicator == false) ? 0 : frame->header->flagContent->decompressedSize;
}

id3byte id3v2GetFrameEncryptionMethod(Id3v2Frame *frame){
    return (id3v2ManipFlagContentErrorChecks(frame) == true) ? 0x00 : frame->header->flagContent->encryption;
}

id3byte id3v2GetFrameGroup(Id3v2Frame *frame){
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

/*
    frame values
*/

int id3v2GetEncoding(Id3v2Frame *frame){

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

id3buf id3v2GetDescription(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    if(frame->frame == NULL){
        return NULL;
    }

    if(frame->header == NULL){
        return NULL;
    }

    id3buf ptr = NULL;
    id3buf ret = NULL;
    int encoding = id3v2GetEncoding(frame);
    
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
        case COMR:
            ptr = ((Id3v2CommercialBody *)frame->frame)->description;
            break;
        case CRM:{
            ptr = ((Id3v2EncryptedMetaBody *)frame->frame)->content;
            if(ptr == NULL){
                return NULL;
            }
            ret = calloc(sizeof(id3byte), strlen((char *)ptr) + 1);
            memcpy(ret, ptr, strlen((char *)ptr));
            return ret;
        }
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

id3buf id3v2GetTextValue(Id3v2Frame *frame){

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
    
    id3buf ret = calloc(sizeof(id3byte), id3strlen(body->value, body->encoding) + id3ReaderAllocationAdd(body->encoding));
    memcpy(ret, body->value, id3strlen(body->value, body->encoding));

    return ret;
}   

id3buf id3v2GetURLValue(Id3v2Frame *frame){
    
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

    id3buf ret = calloc(sizeof(id3byte), id3strlen(body->url, ISO_8859_1) + 1);
    memcpy(ret, body->url, id3strlen(body->url, ISO_8859_1));

    return ret;
}

id3buf id3v2GetInvolvedPeopleListValue(Id3v2Frame *frame){

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
    id3buf ret = calloc(sizeof(id3byte), id3strlen(body->peopleListStrings, encoding) + id3ReaderAllocationAdd(encoding));
    memcpy(ret, body->peopleListStrings, id3strlen(body->peopleListStrings, encoding));

    return ret;
}

id3buf id3v2GetCDIDValue(Id3v2Frame *frame){

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
    id3buf ret = calloc(sizeof(id3byte), id3strlen(body->cdtoc, ISO_8859_1) + 1);
    memcpy(ret, body->cdtoc, id3strlen(body->cdtoc, ISO_8859_1));

    return ret;   
}

int id3v2GetTimeStampFormat(Id3v2Frame *frame){

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

id3byte id3v2GetEventTimeCodeType(Id3v2Frame *frame){

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

    if(id3HasNextListIter(body->eventsTimeCodesIter)){
        Id3v2EventTimesCodeEvent *event = (Id3v2EventTimesCodeEvent *)id3NextListIter(body->eventsTimeCodesIter);

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

    if(id3HasNextListIter(body->eventsTimeCodesIter)){
        Id3v2EventTimesCodeEvent *event = (Id3v2EventTimesCodeEvent *)id3NextListIter(body->eventsTimeCodesIter);

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

    id3FreeListIter(body->eventsTimeCodesIter);

    Id3ListIter *li = id3NewListIter(body->eventTimeCodes);

    body->eventsTimeCodesIter = li;
}

id3buf id3v2GetSyncedTempoCodesFrameValue(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    Id3v2SyncedTempoCodesBody *body = (Id3v2SyncedTempoCodesBody *)frame->frame;
    id3buf data = malloc((sizeof(id3byte)*body->tempoDataLen)+1);
    memcpy(data, body->tempoData, body->tempoDataLen);
    
    return data;
}

id3buf id3v2GetLanguage(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }
    
    id3buf lang = NULL;
    id3buf ret = NULL;
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

    if(lang == NULL){
        return NULL;
    }

    ret = calloc(sizeof(id3byte), ID3V2_LANGUAGE_LEN+1);
    memcpy(ret, lang, ID3V2_LANGUAGE_LEN);

    return ret;
}

id3buf id3v2GetUnsynchronizedLyrics(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    id3buf ptr = NULL;
    id3buf ret = NULL;
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

    encoding = id3v2GetEncoding(frame);
    ret = calloc(sizeof(id3byte), id3strlen(ptr, encoding) + id3ReaderAllocationAdd(encoding));
    memcpy(ret, ptr, id3strlen(ptr, encoding));

    return ret;
}

int id3v2GetSynchronizedLyricsContentType(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return -1;
    }

    switch(id3v2GetFrameID(frame)){
        case SLT:
            return ((Id3v2SynchronizedLyricsBody *)frame->frame)->contentType;
        case SYLT:
            return ((Id3v2SynchronizedLyricsBody *)frame->frame)->contentType;
        default:
            return -1;
    }
}

id3buf id3v2GetSynchronizedLyricsValue(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    switch(id3v2GetFrameID(frame)){
        case SLT:
            break;
        case SYLT:
            break;
        default:
            return NULL;
    }  

    Id3v2SynchronizedLyricsBody *body = (Id3v2SynchronizedLyricsBody *)frame->frame;
    int encoding = id3v2GetEncoding(frame);
    id3buf text = NULL;

    if(id3HasNextListIter(body->lyricsIter)){
        Id3v2StampedLyric *stamp = (Id3v2StampedLyric *)id3NextListIter(body->lyricsIter);

        if(stamp->lyricLen == 0){
            return NULL;
        }

        text = calloc(sizeof(id3byte), stamp->lyricLen + id3ReaderAllocationAdd(encoding));
        memcpy(text, stamp->text, stamp->lyricLen);
    
        return text;
    }

    return NULL;

}

long id3v2GetSynchronizedLyricsTimeStamp(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return -1;
    }

    switch(id3v2GetFrameID(frame)){
        case SLT:
            break;
        case SYLT:
            break;
        default:
            return -1;
    }  

    Id3v2SynchronizedLyricsBody *body = (Id3v2SynchronizedLyricsBody *)frame->frame;

    if(id3HasNextListIter(body->lyricsIter)){
        Id3v2StampedLyric *stamp = (Id3v2StampedLyric *)id3NextListIter(body->lyricsIter);
        return stamp->timeStamp;
    }

    return -1;
}

void id3v2ResetSynchronizedLyricsIter(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return;
    }

    switch(id3v2GetFrameID(frame)){
        case SLT:
            break;
        case SYLT:
            break;
        default:
            return;
    }  

    Id3v2SynchronizedLyricsBody *body = (Id3v2SynchronizedLyricsBody *)frame->frame;

    id3FreeListIter(body->lyricsIter);

    Id3ListIter *li = id3NewListIter(body->lyrics);

    body->lyricsIter = li;
}

id3buf id3v2GetCommentValue(Id3v2Frame *frame){


    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    switch(id3v2GetFrameID(frame)){
        case COM:
            break;
        case COMM:
            break;
        default:
            return NULL;
    }

    int encoding = id3v2GetEncoding(frame);
    id3buf text = NULL;
    Id3v2CommentBody *body = (Id3v2CommentBody *)frame->frame;

    text = calloc(sizeof(id3byte), id3strlen(body->text, encoding) + id3ReaderAllocationAdd(encoding));
    memcpy(text, body->text, id3strlen(body->text, encoding));

    return text;
}


id3buf id3v2GetSubjectiveValue(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    switch(id3v2GetFrameID(frame)){
        case REV:
            break;
        case RVA:
            break;
        case EQU:
            break;
        case EQUA:
            break;
        case RVAD:
            break;
        case RVRB:
            break;
        case RVA2:
            break;
        case EQU2:
            break;
        default:
            return NULL;
    }


    Id3v2SubjectiveBody *body = (Id3v2SubjectiveBody *)frame->frame;
    id3buf value = NULL;

    if(body->valueSize <= 0){
        return NULL;
    }

    value = calloc(sizeof(id3byte), body->valueSize + 1);
    memcpy(value, body->value, body->valueSize);

    return value;
}

id3buf id3v2GetRelativeVolumeAdjustmentValue(Id3v2Frame *frame){
    return id3v2GetSubjectiveValue(frame); 
}

id3buf id3v2GetEqualisationValue(Id3v2Frame *frame){
    return id3v2GetSubjectiveValue(frame); 
}

id3buf id3v2GetReverbValue(Id3v2Frame *frame){
    return id3v2GetSubjectiveValue(frame); 
}

id3buf id3v2GetMIMEType(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    id3buf buildMIME = NULL;
    id3buf tmp = NULL;
    id3buf mime = NULL;

    switch(id3v2GetFrameID(frame)){
        case PIC:{
            
            Id3v2PictureBody *body = (Id3v2PictureBody *)frame->frame;
            int mimeLen = 0;

            if(body->format == NULL){
                return NULL;
            }

            mimeLen = strlen((char *)body->format);
            buildMIME = calloc(sizeof(id3byte), ID3V2_MIME_IMG_LEN + mimeLen + 1);
            
            memcpy(buildMIME,"image/",ID3V2_MIME_IMG_LEN);
            tmp = buildMIME + ID3V2_MIME_IMG_LEN;
            memcpy(tmp, body->format, mimeLen);

            return buildMIME;
        }
        case APIC:
            mime = ((Id3v2PictureBody *)frame->frame)->format; 
            break;
        case GEO:
            mime = ((Id3v2GeneralEncapsulatedObjectBody *)frame->frame)->mimeType;
            break;
        case GEOB:
            mime = ((Id3v2GeneralEncapsulatedObjectBody *)frame->frame)->mimeType;
            break;
        case COMR:
            mime = ((Id3v2CommercialBody *)frame->frame)->mimeType;
            break;
        default:
            return NULL;
    }

    if(mime == NULL){
        return NULL;
    }

    buildMIME = calloc(sizeof(id3byte), strlen((char *)mime) + 1);
    memcpy(buildMIME, mime, strlen((char *)mime) + 1);

    return buildMIME;
}

int id3v2GetPictureType(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return -1;
    }


    Id3v2PictureBody *body = (Id3v2PictureBody *)frame->frame;

    switch(id3v2GetFrameID(frame)){
        case PIC:
            return (int)body->pictureType;
        case APIC:
            return (int)body->pictureType;
        default:
            return -1;
    }

    return -1;
}

id3buf id3v2GetPictureValue(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }
    
    Id3v2PictureBody *body = (Id3v2PictureBody *)frame->frame;
    id3buf value = NULL;
    
    switch(id3v2GetFrameID(frame)){
        case PIC:
            break;
        case APIC:
            break;
        default:
            return NULL;
    }

    if(body->picSize <= 0){
        return NULL;
    }
    
    value = calloc(sizeof(id3byte), body->picSize + 1);
    memcpy(value, body->pictureData, body->picSize);
    
    return value;
}

id3buf id3v2GetObjectFileName(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    int encoding = id3v2GetEncoding(frame);
    id3buf name = NULL;

    switch(id3v2GetFrameID(frame)){
        case GEO:
            break;
        case GEOB:
            break;
        default:
            return NULL;
    }

    Id3v2GeneralEncapsulatedObjectBody *body = (Id3v2GeneralEncapsulatedObjectBody *)frame->frame;

    if(body->filename == NULL){
        return NULL;
    }

    name = calloc(sizeof(id3byte), id3strlen(body->filename, encoding) + id3ReaderAllocationAdd(encoding));
    memcpy(name, body->filename, id3strlen(body->filename, encoding));

    return name;
}

id3buf id3v2GetGeneralEncapsulatedObjectValue(Id3v2Frame *frame){
  
    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    id3buf value = NULL;

    switch(id3v2GetFrameID(frame)){
        case GEO:
            break;
        case GEOB:
            break;
        default:
            return NULL;
    }

    Id3v2GeneralEncapsulatedObjectBody *body = (Id3v2GeneralEncapsulatedObjectBody *)frame->frame;

    if(body->encapsulatedObject == NULL || body->encapsulatedObjectLen <= 0){
        return NULL;
    }

    value = malloc(sizeof(id3byte)*(body->encapsulatedObjectLen + 1));
    memcpy(value, body->encapsulatedObject, body->encapsulatedObjectLen);

    return value;
}

int id3v2GetPlayCount(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return -1;
    }


    switch(id3v2GetFrameID(frame)){
        case CNT:
            return (int)((Id3v2PlayCounterBody *)frame->frame)->counter;
        case PCNT:
            return (int)((Id3v2PlayCounterBody *)frame->frame)->counter;
        case POP:
            return (int)((Id3v2PopularBody *)frame->frame)->counter;
        case POPM:
            return (int)((Id3v2PopularBody *)frame->frame)->counter;
        default:
            return -1;
    }
}

id3buf id3v2GetEmail(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }


    switch(id3v2GetFrameID(frame)){
        case POP:
            break;
        case POPM:
            break;
        default:
            return NULL;
    }

    Id3v2PopularBody *body = (Id3v2PopularBody *)frame->frame;
    id3buf email = NULL;

    if(body->email == NULL){
        return NULL;
    }

    email = calloc(sizeof(id3byte), strlen((char *)body->email) + 1);
    memcpy(email, body->email, strlen((char *)body->email));
    
    return email;
}

int id3v2GetRating(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return -1;
    }


    switch(id3v2GetFrameID(frame)){
        case POP:
            break;
        case POPM:
            break;
        default:
            return -1;
    }

    Id3v2PopularBody *body = (Id3v2PopularBody *)frame->frame;
    
    return (int)body->rating;
}

id3buf id3v2GetOwnerIdentifier(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    id3buf ptr = NULL;
    id3buf ret = NULL;

    switch(id3v2GetFrameID(frame)){
        case UFI:
            ptr = ((Id3v2UniqueFileIdentifierBody *)frame->frame)->ownerIdentifier;
            break;
        case UFID:
            ptr = ((Id3v2UniqueFileIdentifierBody *)frame->frame)->ownerIdentifier;
            break;
        case CRM:
            ptr = ((Id3v2EncryptedMetaBody *)frame->frame)->ownerIdentifier;
            break;
        case AENC:
            ptr = ((Id3v2AudioEncryptionBody *)frame->frame)->ownerIdentifier;
            break;
        case ENCR:
            ptr = ((Id3v2EncryptionMethodRegistrationBody *)frame->frame)->ownerIdentifier;
            break;
        case GRID:
            ptr = ((Id3v2GroupIDRegistrationBody *)frame->frame)->ownerIdentifier;
            break;
        case PRIV:
            ptr = ((Id3v2PrivateBody *)frame->frame)->ownerIdentifier;
            break;
        default:
            return NULL;
    }

    if(ptr == NULL){
        return NULL;
    }

    ret = calloc(sizeof(id3byte), strlen((char *)ptr) + 1);
    memcpy(ret, ptr, strlen((char *)ptr));
    return ret;
}

id3buf id3v2GetEncryptedMetaValue(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    if(id3v2GetFrameID(frame) != CRM){
        return NULL;
    }

    id3buf ptr = ((Id3v2EncryptedMetaBody *)frame->frame)->encryptedDatablock;
    id3buf ret = NULL;

    if(ptr == NULL){
        return NULL;
    }

    ret = calloc(sizeof(id3byte), ((Id3v2EncryptedMetaBody *)frame->frame)->encryptedDatablockLen + 1);
    memcpy(ret, ptr, ((Id3v2EncryptedMetaBody *)frame->frame)->encryptedDatablockLen);
    return ret;
}

id3buf id3v2GetPreviewStart(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    if(id3v2GetFrameID(frame) != AENC){
        return NULL;
    }

    return ((Id3v2AudioEncryptionBody *)frame->frame)->previewStart;
}

int id3v2GetPreviewLength(Id3v2Frame *frame){
    
    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return -1;
    }

    if(id3v2GetFrameID(frame) != AENC){
        return -1;
    }

    return ((Id3v2AudioEncryptionBody *)frame->frame)->previewLength;
}

id3buf id3v2GetAudioEncryptionValue(Id3v2Frame *frame){
    
    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    if(id3v2GetFrameID(frame) != AENC){
        return NULL;
    }

    id3buf value = NULL;

    if(((Id3v2AudioEncryptionBody *)frame->frame)->encryptionInfo == NULL){
        return NULL;
    }

    value = malloc(sizeof(id3byte) * (((Id3v2AudioEncryptionBody *)frame->frame)->encryptionInfoLen + 1));
    memcpy(value, ((Id3v2AudioEncryptionBody *)frame->frame)->encryptionInfo, ((Id3v2AudioEncryptionBody *)frame->frame)->encryptionInfoLen);

    return value;

}

id3buf id3v2GetUniqueFileIdentifierValue(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    switch(id3v2GetFrameID(frame)){
        case UFI:
            break;
        case UFID:
            break;
        default:
            return NULL;
    }

    Id3v2UniqueFileIdentifierBody *body = (Id3v2UniqueFileIdentifierBody *)frame->frame;

    if(body->identifier == NULL){
        return NULL;
    }

    id3buf value = calloc(sizeof(id3byte), strlen((char *)body->identifier) + 1);
    memcpy(value, body->identifier, strlen((char *)body->identifier));
    
    return value;
}

long id3v2GetPositionSynchronisationValue(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return -1;
    }

    if(id3v2GetFrameID(frame) != POSS){
        return -1;
    }

    return ((Id3v2PositionSynchronisationBody *)frame->frame)->pos;
}

id3buf id3v2GetTermsOfUseValue(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    if(id3v2GetFrameID(frame) != USER){
        return NULL;
    }

    Id3v2TermsOfUseBody *body = (Id3v2TermsOfUseBody *)frame->frame;
    id3buf value = NULL;
    int encoding = id3v2GetEncoding(frame);
    if(body->text == NULL){
        return NULL;
    }

    value = calloc(sizeof(id3byte), id3strlen(body->text, encoding) + id3ReaderAllocationAdd(encoding));
    memcpy(value, body->text, id3strlen(body->text, encoding)); 

    return value;
}

id3buf id3v2GetPrice(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    id3buf ptr = NULL;
    id3buf ret = NULL;

    switch(id3v2GetFrameID(frame)){
        case OWNE:
            ptr = ((Id3v2OwnershipBody *)frame->frame)->pricePayed;
            break;
        case COMR:
            ptr = ((Id3v2CommercialBody *)frame->frame)->priceString;
            break;
        default:
            return NULL;
    }

    if(ptr == NULL){
        return NULL;
    }
    
    ret = calloc(sizeof(id3byte), strlen((char *)ptr) + 1);
    memcpy(ret, ptr, strlen((char *)ptr));

    return ret;
}

id3buf id3v2GetPunchDate(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    id3buf ptr = NULL;
    id3buf ret = NULL;

    if(id3v2GetFrameID(frame) != OWNE){
        return NULL;
    }


    ptr = ((Id3v2OwnershipBody *)frame->frame)->dateOfPunch;

    if(ptr == NULL){
        return NULL;
    }
    
    ret = calloc(sizeof(id3byte), strlen((char *)ptr) + 1);
    memcpy(ret, ptr, strlen((char *)ptr));

    return ret;
}

id3buf id3v2GetSeller(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    id3buf ptr = NULL;
    id3buf ret = NULL;
    int encoding = 0;

    switch(id3v2GetFrameID(frame)){
        case OWNE:
            ptr = ((Id3v2OwnershipBody *)frame->frame)->seller;
            break;
        case COMR:
            ptr = ((Id3v2CommercialBody *)frame->frame)->nameOfSeller;
            break;
        default:
            return NULL;
    }
    encoding = id3v2GetEncoding(frame);

    if(ptr == NULL){
        return NULL;
    }
    
    ret = calloc(sizeof(id3byte), id3strlen(ptr,encoding) + id3ReaderAllocationAdd(encoding));
    memcpy(ret, ptr, id3strlen(ptr,encoding));

    return ret;
}

id3buf id3v2GetValidDate(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    id3buf ptr = NULL;
    id3buf ret = NULL;

    if(id3v2GetFrameID(frame) != COMR){
        return NULL;
    }

    ptr = ((Id3v2CommercialBody *)frame->frame)->validUntil;

    ret = calloc(sizeof(id3byte), strlen((char *)ptr) + 1);
    memcpy(ret, ptr, strlen((char *)ptr));

    return ret;
}

id3buf id3v2GetContractURL(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    id3buf ptr = NULL;
    id3buf ret = NULL;

    if(id3v2GetFrameID(frame) != COMR){
        return NULL;
    }

    ptr = ((Id3v2CommercialBody *)frame->frame)->contractURL;

    if(ptr == NULL){
        return NULL;
    }

    ret = calloc(sizeof(id3byte), strlen((char *)ptr) + 1);
    memcpy(ret, ptr, strlen((char *)ptr));

    return ret;
}

int id3v2GetCommecialDeliveryMethod(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return -1;
    }

    if(id3v2GetFrameID(frame) != COMR){
        return -1;
    }

    return (int)((Id3v2CommercialBody *)frame->frame)->receivedAs;
}


id3buf id3v2GetCommercialSellerLogo(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    if(id3v2GetFrameID(frame) != COMR){
        return NULL;
    }

    Id3v2CommercialBody *body = (Id3v2CommercialBody *)frame->frame;

    if(body->sellerLogo == NULL){
        return NULL;
    }

    id3buf value = calloc(sizeof(id3byte), body->sellerLogoLen + 1);
    memcpy(value, body->sellerLogo, body->sellerLogoLen);

    return value;
}

id3byte id3v2GetSymbol(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return 0x00;
    }

    switch(id3v2GetFrameID(frame)){
        case ENCR:
            return ((Id3v2EncryptionMethodRegistrationBody *)frame->frame)->methodSymbol;
        case GRID:
            return ((Id3v2GroupIDRegistrationBody *)frame->frame)->groupSymbol;
        case SIGN:
            return ((Id3v2SignatureBody *)frame->frame)->groupSymbol;
        default:
            return 0x00;
    }
}

id3buf id3v2GetEncryptionRegistrationValue(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    if(id3v2GetFrameID(frame) != ENCR){
        return NULL;
    }

    Id3v2EncryptionMethodRegistrationBody *body = (Id3v2EncryptionMethodRegistrationBody *)frame->frame;
    id3buf value = calloc(sizeof(id3byte), body->encryptionDataLen + 1);
    memcpy(value, body->encryptionData, body->encryptionDataLen);

    return value;
}

id3buf id3v2GetGroupIDValue(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    if(id3v2GetFrameID(frame) != GRID){
        return NULL;
    }

    Id3v2GroupIDRegistrationBody *body = (Id3v2GroupIDRegistrationBody *)frame->frame;
    id3buf value = calloc(sizeof(id3byte), body->groupDependentDataLen + 1);
    memcpy(value, body->groupDependentData, body->groupDependentDataLen);

    return value;
}

id3buf id3v2GetPrivateValue(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    if(id3v2GetFrameID(frame) != PRIV){
        return NULL;
    }

    Id3v2PrivateBody *body = (Id3v2PrivateBody *)frame->frame;
    id3buf value = calloc(sizeof(id3byte), body->privateDataLen + 1);
    memcpy(value, body->privateData, body->privateDataLen);

    return value;
}

id3buf id3v2GetSignatureValue(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

    if(id3v2GetFrameID(frame) != SIGN){
        return NULL;
    }

    Id3v2SignatureBody *body = (Id3v2SignatureBody *)frame->frame;
    id3buf value = calloc(sizeof(id3byte),body->sigLen + 1);
    memcpy(value, body->signature, body->sigLen);

    return value;
}

int id3v2GetOffsetToNextTag(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return 0;
    }

    if(id3v2GetFrameID(frame) != SEEK){
        return 0;
    }

    return ((Id3v2SeekBody *)frame->frame)->minimumOffsetToNextTag;
}
