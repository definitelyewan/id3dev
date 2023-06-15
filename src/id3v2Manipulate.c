#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v2Manipulate.h"
#include "id3Helpers.h"
#include "id3Defines.h"
#include "id3v2Frames.h"
#include "id3v2Header.h"
#include "id3Reader.h" 

//checks (not in header)

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

//converts (not in header)
//ret[0] = size to be removed
//ret[1] = size to be added
unsigned int *id3v2ChangeCommentEncoding(Id3v2Frame *frame, id3byte encoding){

    unsigned int *ret = calloc(sizeof( unsigned int), 2);
    id3buf curr = NULL;
    id3buf replace = NULL;
    unsigned int currSize = 0;
    unsigned int replaceSize = 0;
    
    //get old description
    curr = ((Id3v2CommentBody *)frame->frame)->description;
    currSize = id3strlen(curr, id3v2GetEncoding(frame));

    //replace description
    replace = id3TextFormatConvert(curr, currSize, encoding);
    replaceSize = id3strlen(replace, encoding);
    ((Id3v2CommentBody *)frame->frame)->description = replace;

    //sizes
    ret[0] = uSafeSum(ret[0],currSize,false);
    ret[1] = uSafeSum(ret[1],replaceSize,false);

    if(curr != NULL){
        free(curr);
    }

    //get old comment
    curr = ((Id3v2CommentBody *)frame->frame)->text;
    currSize = id3strlen(curr, id3v2GetEncoding(frame));

    //replace comment
    replace = id3TextFormatConvert(curr, currSize, encoding);
    replaceSize = id3strlen(replace, encoding);
    ((Id3v2CommentBody *)frame->frame)->text = replace;

    //sizes
    ret[0] = uSafeSum(ret[0],currSize,false);
    ret[1] = uSafeSum(ret[1],replaceSize,false);

    if(curr != NULL){
        free(curr);
    }

    ((Id3v2CommentBody *)frame->frame)->encoding = encoding;
    return ret;
}

unsigned int *id3v2ChangeEncapsulatedObjectEncoding(Id3v2Frame *frame, id3byte encoding){

    unsigned int *ret = calloc(sizeof(int), 2);
    id3buf curr = NULL;
    id3buf replace = NULL;
    unsigned int currSize = 0;
    unsigned int replaceSize = 0;
    
    //get old filename
    curr = ((Id3v2GeneralEncapsulatedObjectBody *)frame->frame)->filename;
    currSize = id3strlen(curr, id3v2GetEncoding(frame));

    //replace filename
    replace = id3TextFormatConvert(curr, currSize, encoding);
    replaceSize = id3strlen(replace, encoding);
    ((Id3v2GeneralEncapsulatedObjectBody *)frame->frame)->filename = replace;

    //sizes
    ret[0] = uSafeSum(ret[0], currSize, false);
    ret[1] = uSafeSum(ret[1], replaceSize, false);

    if(curr != NULL){
        free(curr);
    }

    //get old desc
    curr = ((Id3v2GeneralEncapsulatedObjectBody *)frame->frame)->contentDescription;
    currSize = id3strlen(curr, id3v2GetEncoding(frame));

    //replace desc
    replace = id3TextFormatConvert(curr, currSize, encoding);
    replaceSize = id3strlen(replace, encoding);
    ((Id3v2GeneralEncapsulatedObjectBody *)frame->frame)->contentDescription = replace;

    //sizes
    ret[0] = uSafeSum(ret[0], currSize, false);
    ret[1] = uSafeSum(ret[1], replaceSize, false);

    if(curr != NULL){
        free(curr);
    }

    ((Id3v2CommentBody *)frame->frame)->encoding = encoding;

    return ret;
}

unsigned int *id3v2ChangeInvolvedPeopleEncoding(Id3v2Frame *frame, id3byte encoding){

    unsigned int *ret = calloc(sizeof(int), 2);
    id3buf curr = NULL;
    id3buf replace = NULL;
    Id3ListIter *li = NULL;
    Id3v2InvolvedPerson *iPerson = NULL;
    Id3v2InvolvedPeopleListBody *body = NULL;
    
    body = (Id3v2InvolvedPeopleListBody *)frame->frame;
    li = id3NewListIter(body->involvedPeople);

    //there was no list so there is nothing to change
    if(li == NULL){
        return ret;
    }

    //convert all the text
    while((iPerson = id3NextListIter(li)) != NULL){
        
        //convert job
        if(iPerson->job != NULL){
            curr = iPerson->job;
            ret[0] = uSafeSum(ret[0], iPerson->jobLen, false);

            replace = id3TextFormatConvert(curr, iPerson->jobLen, encoding);

            iPerson->job = replace;
            iPerson->jobLen = id3strlen(replace, encoding);
            ret[1] = uSafeSum(ret[1], iPerson->jobLen, false);

            free(curr);
        }

        //convert person
        if(iPerson->person != NULL){
            curr = iPerson->person;
            ret[0] = uSafeSum(ret[0], iPerson->personLen, false);
            
            replace = id3TextFormatConvert(curr, iPerson->personLen, encoding);

            iPerson->person = replace;
            iPerson->personLen = id3strlen(replace, encoding);
            ret[1] = uSafeSum(ret[1], iPerson->personLen, false);

            free(curr);
        }
    }

    body->encoding = encoding;

    id3FreeListIter(li);
    return ret;
}

unsigned int *id3v2ChangePictureEncoding(Id3v2Frame *frame, id3byte encoding){

    unsigned int *ret = calloc(sizeof(int), 2);
    id3buf curr = NULL;
    id3buf replace = NULL;
    Id3v2PictureBody *body = NULL;

    body = (Id3v2PictureBody *)frame->frame;

    if(body->description != NULL){

        curr = body->description;
        ret[0] = uSafeSum(ret[0], id3strlen(curr, body->encoding), false);
        
        replace = id3TextFormatConvert(curr, ret[0], encoding);
        ret[1] = uSafeSum(ret[1], id3strlen(replace, encoding), false);
        body->description = replace;
        
        free(curr);
    }

    body->encoding = encoding;

    return ret;
}

unsigned int *id3v2ChangeSynchronizedLyricsEncoding(Id3v2Frame *frame, id3byte encoding){
    
    unsigned int *ret = calloc(sizeof(int), 2);
    id3buf curr = NULL;
    id3buf replace = NULL;
    Id3v2SynchronizedLyricsBody *body = NULL;
    Id3v2StampedLyric *lyric = NULL;
    Id3ListIter *li = NULL;

    body = (Id3v2SynchronizedLyricsBody *)frame->frame;
    
    //descriptor change
    if(body->descriptor != NULL){
        curr = body->descriptor;
        ret[0] = id3strlen(curr, body->encoding);

        replace = id3TextFormatConvert(curr, ret[0], encoding);
        ret[1] = id3strlen(replace, encoding);
        body->descriptor = replace;

        free(curr);
    }

    //list change
    li = id3NewListIter(body->lyrics);

    if(li == NULL){
        return ret;
    }

    while((lyric = id3NextListIter(li)) != NULL){
        
        if(lyric->text != NULL){
            curr = lyric->text;
            ret[0] = uSafeSum(ret[0], lyric->lyricLen, false);
            
            replace = id3TextFormatConvert(curr, lyric->lyricLen, encoding);
            lyric->lyricLen = id3strlen(replace, encoding);
            lyric->text = replace;

            ret[1] = uSafeSum(ret[1], lyric->lyricLen, false);

            free(curr);
        }

    }
    id3FreeListIter(li);

    body->encoding = encoding;
    return ret;
}

unsigned int *id3v2ChangeUnsynchronizedLyricsEncoding(Id3v2Frame *frame, id3byte encoding){

    unsigned int *ret = calloc(sizeof(int), 2);
    id3buf curr = NULL;
    id3buf replace = NULL;
    Id3v2UnsynchronizedLyricsBody *body = NULL;

    body = (Id3v2UnsynchronizedLyricsBody *)frame->frame;
    
    //change desc
    if(body->descriptor != NULL){
        curr = body->descriptor;
        ret[0] = uSafeSum(ret[0], id3strlen(curr, body->encoding), false);

        replace = id3TextFormatConvert(curr, ret[0], encoding);
        body->descriptor = replace;
        ret[1] = uSafeSum(ret[1], id3strlen(replace, encoding), false);

        free(curr);
    }

    //change lyrics
    if(body->lyrics != NULL){
        curr = body->lyrics;
        ret[0] = uSafeSum(ret[0], id3strlen(curr, body->encoding), false);

        replace = id3TextFormatConvert(curr, id3strlen(curr, body->encoding), encoding);
        body->lyrics = replace;
        ret[1] = uSafeSum(ret[1], id3strlen(replace, encoding), false);

        free(curr);
    }

    body->encoding = encoding;
    return ret;
}

unsigned int *id3v2ChangeURLEncoding(Id3v2Frame *frame, id3byte encoding){

    unsigned int *ret = calloc(sizeof(int), 2);
    id3buf curr = NULL;
    id3buf replace = NULL;
    Id3v2URLBody *body = (Id3v2URLBody *)frame->frame;

    if(body->description != NULL){
        curr = body->description;
        ret[0] = id3strlen(body->description, body->encoding);

        replace = id3TextFormatConvert(curr, ret[0], encoding);
        ret[1] = id3strlen(replace, encoding);
        body->description = replace;

        free(curr);
    }
    
    body->encoding = encoding;
    return ret;
}

unsigned int *id3v2ChangeCommercialEncoding(Id3v2Frame *frame, id3byte encoding){

    unsigned int *ret = calloc(sizeof(int), 2);
    id3buf curr = NULL;
    id3buf replace = NULL;
    Id3v2CommercialBody *body = NULL;

    body = (Id3v2CommercialBody *)frame->frame;

    if(body->nameOfSeller != NULL){
        curr = body->nameOfSeller;
        ret[0] = uSafeSum(ret[0], id3strlen(curr, body->encoding), false);
        
        replace = id3TextFormatConvert(curr, ret[0], encoding);
        ret[1] = uSafeSum(ret[1], id3strlen(replace, encoding), false);

        body->contractURL = replace;
        free(curr);
    }

    if(body->description != NULL){
        curr = body->description;
        ret[0] = uSafeSum(ret[0], id3strlen(curr, body->encoding), false);

        replace = id3TextFormatConvert(curr, id3strlen(curr, body->encoding), encoding);
        ret[1] = uSafeSum(ret[1], id3strlen(replace, encoding), false);
        
        body->description = replace;
        free(curr);
    }


    body->encoding = encoding;
    return ret;
}

unsigned int *id3v2ChangeOwnershipEncoding(Id3v2Frame *frame, id3byte encoding){

    unsigned int *ret = calloc(sizeof(int), 2);
    id3buf curr = NULL;
    id3buf replace = NULL;
    Id3v2OwnershipBody *body = NULL;

    body = (Id3v2OwnershipBody *)frame->frame;

    if(body->seller != NULL){
        curr = body->seller;
        ret[0] = uSafeSum(ret[0], id3strlen(curr, body->encoding), false);
        
        replace = id3TextFormatConvert(curr, ret[0], encoding);
        ret[1] = uSafeSum(ret[1], id3strlen(replace, encoding), false);

        body->seller = replace;
        free(curr);
    }

    body->encoding = encoding;
    return ret;
}

unsigned int *id3v2ChangeTextEncoding(Id3v2Frame *frame, id3byte encoding){

    unsigned int *ret = calloc(sizeof(int), 2);
    id3buf curr = NULL;
    id3buf replace = NULL;
    Id3v2TextBody *body = NULL;

    body = (Id3v2TextBody *)frame->frame;

    if(body->description != NULL){
        curr = body->description;
        ret[0] = uSafeSum(ret[0], id3strlen(curr, body->encoding), false);

        replace = id3TextFormatConvert(curr, ret[0], encoding);
        ret[1] = uSafeSum(ret[1], id3strlen(replace, encoding), false);

        body->description = replace;
        free(curr);
    }

    if(body->value != NULL){
        curr = body->value;
        ret[0] = uSafeSum(ret[0], id3strlen(curr, body->encoding), false);

        replace = id3TextFormatConvert(curr, id3strlen(curr, body->encoding), encoding);
        ret[1] = uSafeSum(ret[1], id3strlen(replace, encoding), false);
        body->value = replace;

        free(curr);
    }

    body->encoding = encoding;
    return ret;
}

unsigned int *id3v2ChangeTermsOfUseEncoding(Id3v2Frame *frame, id3byte encoding){

    unsigned int *ret = calloc(sizeof(int), 2);
    id3buf curr = NULL;
    id3buf replace = NULL;
    Id3v2TermsOfUseBody *body = NULL;

    body = (Id3v2TermsOfUseBody *)frame->frame;

    if(body->text != NULL){
        curr = body->text;
        ret[0] = uSafeSum(ret[0], id3strlen(curr, body->encoding), false);

        replace = id3TextFormatConvert(curr, ret[0], encoding);
        ret[1] = uSafeSum(ret[1], id3strlen(replace, encoding), false);
        body->text = replace;

        free(curr);
    }

    body->encoding = encoding;
    return ret;
}

//quick reads by dual ids (not in header)

id3buf id3v2DualIDGetV(Id3v2FrameId id1, Id3v2FrameId id2, id3byte desiredEncoding, Id3v2Tag *tag, id3buf (*definedFunction)(Id3v2Frame *toRead)){

    if(tag == NULL){
        return NULL;
    }

    if(tag->header == NULL || tag->frames == NULL || tag->iter == NULL){
        return NULL;
    }
    
    Id3List *l = NULL;
    id3buf tmp = NULL;
    id3buf ret = NULL;
    Id3v2Frame *f = NULL;

    switch(tag->header->versionMajor){
        case ID3V22:
            l = id3v2SearchForFrames(tag, id1);
            break;
        case ID3V23:
        case ID3V24:
            l = id3v2SearchForFrames(tag, id2);
            break;
        default:
            return NULL;
    }

    if((f = id3RemoveList(l, 0)) != NULL){
        tmp = definedFunction(f);
        if(tmp != NULL){
            ret = id3TextFormatConvert(tmp, id3strlen(tmp, id3v2GetEncoding(f)), desiredEncoding);
            free(tmp);

        }
        id3v2FreeFrame(f);
    }

    id3DestroyList(l);

    return ret;
}

void id3v2DualIDSetV(Id3v2FrameId id1, Id3v2FrameId id2, Id3v2Tag *tag, id3buf value, unsigned int valueLength, void (*definedFunction)(id3buf value, unsigned int valueLength, Id3v2Frame *toWrite)){

    if(tag == NULL){
        return;
    }

    if(tag->header == NULL || tag->frames == NULL || tag->iter == NULL){
        return;
    }
    
    Id3v2Frame *f = NULL;
    Id3ListIter *li = NULL;

    li = id3NewListIter(tag->frames);

    while((f = id3NextListIter(li)) != NULL){

        if(id3v2GetFrameID(f) == id1 || id3v2GetFrameID(f) == id2){
            definedFunction(value, valueLength, f);
            break;
        }
    }

    id3FreeListIter(li);
}

//quick reads n writes for common attrs

void id3v2SetTitle(id3buf value, unsigned int valueLength, Id3v2Tag *tag){    
    id3v2DualIDSetV(TT2, TIT2, tag, value, valueLength, id3v2SetTextValue);  
}

void id3v2SetArtist(id3buf value, unsigned int valueLength, Id3v2Tag *tag){
    id3v2DualIDSetV(TP1, TPE1, tag, value, valueLength, id3v2SetTextValue);
}

void id3v2SetAlbum(id3buf value, unsigned int valueLength, Id3v2Tag *tag){
    id3v2DualIDSetV(TAL, TALB, tag, value, valueLength, id3v2SetTextValue);
}

void id3v2SetAlbumArtist(id3buf value, unsigned int valueLength, Id3v2Tag *tag){
    id3v2DualIDSetV(TP2, TPE2, tag, value, valueLength, id3v2SetTextValue);
}

void id3v2SetComposer(id3buf value, unsigned int valueLength, Id3v2Tag *tag){
    id3v2DualIDSetV(TCM, TCOM, tag, value, valueLength, id3v2SetTextValue);
}

void id3v2SetYear(id3buf value, unsigned int valueLength, Id3v2Tag *tag){
    id3v2DualIDSetV(TYE, TYER, tag, value, valueLength, id3v2SetTextValue);
}

void id3v2SetComment(id3buf value, unsigned int valueLength, Id3v2Tag *tag){
    id3v2DualIDSetV(COM, COMM, tag, value, valueLength, id3v2SetCommentValue);
}

void id3v2SetGenre(id3buf value, unsigned int valueLength, Id3v2Tag *tag){
    id3v2DualIDSetV(TCO, TCON, tag, value, valueLength, id3v2SetTextValue);
}

void id3v2SetTrack(id3buf value, unsigned int valueLength, Id3v2Tag *tag){
    id3v2DualIDSetV(TRK, TRCK, tag, value, valueLength, id3v2SetTextValue);
}

void id3v2SetDisc(id3buf value, unsigned int valueLength, Id3v2Tag *tag){
    id3v2DualIDSetV(TPA, TPOS, tag, value, valueLength, id3v2SetTextValue);
}

void id3v2SetLyrics(id3buf value, unsigned int valueLength, Id3v2Tag *tag){
    id3v2DualIDSetV(ULT, USLT, tag, value, valueLength, id3v2SetUnsynchronizedLyrics);
}

void id3v2SetPicture(id3buf value, unsigned int valueLength, Id3v2Tag *tag){

    if(tag == NULL){
        return;
    }

    if(tag->header == NULL || tag->frames == NULL || tag->iter == NULL){
        return;
    }
    
    Id3v2Frame *f = NULL;
    Id3ListIter *li = NULL;

    li = id3NewListIter(tag->frames);

    while((f = id3NextListIter(li)) != NULL){

        if(id3v2GetFrameID(f) == PIC || id3v2GetFrameID(f) == APIC){
            id3v2SetPictureValue(value,valueLength, f);
            break;
        }
    }

    id3FreeListIter(li);

}

id3buf id3v2GetTitle(id3byte desiredEncoding, Id3v2Tag *tag){
    return id3v2DualIDGetV(TT2, TIT2, desiredEncoding, tag, id3v2GetTextValue);
}

id3buf id3v2GetArtist(id3byte desiredEncoding, Id3v2Tag *tag){
    return id3v2DualIDGetV(TP1, TPE1, desiredEncoding, tag, id3v2GetTextValue);
}

id3buf id3v2GetAlbum(id3byte desiredEncoding, Id3v2Tag *tag){
    return id3v2DualIDGetV(TAL, TALB, desiredEncoding, tag, id3v2GetTextValue);
}

id3buf id3v2GetAlbumArtist(id3byte desiredEncoding, Id3v2Tag *tag){
    return id3v2DualIDGetV(TP2, TPE2, desiredEncoding, tag, id3v2GetTextValue);
}

id3buf id3v2GetComposer(id3byte desiredEncoding, Id3v2Tag *tag){
    return id3v2DualIDGetV(TCM, TCOM, desiredEncoding, tag, id3v2GetTextValue);
}

id3buf id3v2GetYear(id3byte desiredEncoding, Id3v2Tag *tag){
    return id3v2DualIDGetV(TYE, TYER, desiredEncoding, tag, id3v2GetTextValue);
}

id3buf id3v2GetComment(id3byte desiredEncoding, Id3v2Tag *tag){
    return id3v2DualIDGetV(COM, COMM, desiredEncoding, tag, id3v2GetCommentValue);
}

id3buf id3v2GetGenre(id3byte desiredEncoding, Id3v2Tag *tag){
    return id3v2DualIDGetV(TCO, TCON, desiredEncoding, tag, id3v2GetTextValue);
}

id3buf id3v2GetTrack(id3byte desiredEncoding, Id3v2Tag *tag){
    return id3v2DualIDGetV(TRK, TRCK, desiredEncoding, tag, id3v2GetTextValue);
}

id3buf id3v2GetDisc(id3byte desiredEncoding, Id3v2Tag *tag){
    return id3v2DualIDGetV(TPA, TPOS, desiredEncoding, tag, id3v2GetTextValue);
}

id3buf id3v2GetLyrics(id3byte desiredEncoding, Id3v2Tag *tag){
    return id3v2DualIDGetV(ULT, USLT, desiredEncoding, tag, id3v2GetUnsynchronizedLyrics);
}

id3buf id3v2GetPicture(id3byte pictureType, unsigned int *pictureSize, Id3v2Tag *tag){

    if(tag == NULL){
        return NULL;
    }

    if(tag->header == NULL || tag->frames == NULL || tag->iter == NULL){
        return NULL;
    }
    
    int pos = 1;
    Id3List *l = NULL;
    id3buf ret = NULL;
    Id3v2Frame *f = NULL;

    switch(tag->header->versionMajor){
        case ID3V22:
            l = id3v2SearchForFrames(tag, PIC);
            break;
        case ID3V23:
        case ID3V24:
            l = id3v2SearchForFrames(tag, APIC);
            break;
        default:
            return NULL;
    }
    
    while((f = id3RemoveList(l,pos)) != NULL){
        
        if(ret != NULL){
            free(ret);
        }
        
        ret = id3v2GetPictureValue(f);
        *pictureSize = ((Id3v2PictureBody *)f->frame)->picSize;

        if(id3v2GetPictureType(f) == pictureType){
            break;
            id3v2FreeFrame(f);
        }
        
        pos++;
        id3v2FreeFrame(f);
    }

    id3DestroyList(l);

    return ret;

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
    Id3ListIter *li = id3NewListIter(tag->frames);

    while((currFrame = id3NextListIter(li)) != NULL){

        if(id3v2GetFrameID(currFrame) == id){
            Id3v2Frame *copy = id3v2CopyFrame((void *)currFrame);
            id3PushList(idList, (void *)copy);
        }
    }
    
    id3FreeListIter(li);
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

void id3v2AddEventToEventFrame(Id3v2Frame *eventCodeFrame, id3byte typeOfEvent, int timeStamp){

    if(id3v2ManipFullFrameErrorChecks(eventCodeFrame) == true){
        return;
    }

    Id3v2EventTimesCodeEvent *event = NULL;
    Id3v2EventTimeCodesBody *body = NULL;
    Id3List *list = NULL;

    body = (Id3v2EventTimeCodesBody *)eventCodeFrame->frame;
    event = id3v2NewEventCodeEvent(typeOfEvent, timeStamp);
    
    
    if(body->eventTimeCodes == NULL){
        list = id3NewList(id3v2FreeEventCode,id3v2CopyEventCodeEvent);
        body->eventTimeCodes = list;
    }

    eventCodeFrame->header->frameSize = eventCodeFrame->header->frameSize + 1 + 4;
    
    id3PushList(body->eventTimeCodes, (void *)event);
    id3v2ResetEventTimeCodesIter(eventCodeFrame);
}

void id3v2AddLyricToSynchronizedLyricsFrame(Id3v2Frame *lyricFrame, id3buf lyric, int timeStamp){

    if(id3v2ManipFullFrameErrorChecks(lyricFrame) == true){
        return;
    }
    
    id3buf newLyric = NULL;
    int strSize = 0;
    Id3v2StampedLyric *stampedLyric = NULL;
    Id3v2SynchronizedLyricsBody *body = NULL;
    Id3List *list = NULL;

    //build new string
    body = (Id3v2SynchronizedLyricsBody *)lyricFrame->frame;
    strSize = id3strlen(lyric, body->encoding);
    newLyric = calloc(sizeof(id3byte), strSize + id3ReaderAllocationAdd(body->encoding));
    memcpy(newLyric, lyric, strSize);

    //make a new stamp
    stampedLyric = id3v2NewStampedLyric(newLyric, timeStamp, strSize);

    //add the new stamp
    if(body->lyrics == NULL){
        list = id3NewList(id3v2FreeStampedLyric, id3v2CopyStampedLyric);
        body->lyrics = list;
    }

    lyricFrame->header->frameSize = lyricFrame->header->frameSize + strSize + ID3V2_TIME_STAMP_LEN + 1;//+1 is a padding

    id3PushList(body->lyrics, (void *)stampedLyric);
    id3v2ResetSynchronizedLyricsIter(lyricFrame);
}

void id3v2AddPersonToInvolvedPersonListFrame(Id3v2Frame *personListFrame, id3buf person, id3buf job){

    if(id3v2ManipFullFrameErrorChecks(personListFrame) == true){
        return;
    }

    id3buf newPerson = NULL;
    id3buf newJob = NULL;
    int pLen = 0;
    int jLen = 0;
    Id3v2InvolvedPerson *iPerson = NULL;
    Id3v2InvolvedPeopleListBody *body = NULL;

    body = (Id3v2InvolvedPeopleListBody *)personListFrame->frame;
    
    //create copies
    if(person != NULL){
        pLen = id3strlen(person, body->encoding);
        newPerson = calloc(sizeof(id3byte), pLen + id3ReaderAllocationAdd(body->encoding));
        memcpy(newPerson, person, pLen);

    }

    if(job != NULL){
        jLen = id3strlen(job, body->encoding);
        newJob = calloc(sizeof(id3byte), jLen + id3ReaderAllocationAdd(body->encoding));
        memcpy(newJob, job, jLen);

    }

    iPerson = id3v2NewInvolvedPerson(newPerson, newJob, pLen, jLen);
    personListFrame->header->frameSize = personListFrame->header->frameSize + jLen + pLen + 1; //1 for padding
    id3PushList(body->involvedPeople, iPerson);
    id3v2ResetInvolvedPeopleListIter(personListFrame);
}

//read and write frame attributes

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

void id3v2SetFrameAlterPreservationIndicator(bool indicator, Id3v2Frame *frame){
    
    if(id3v2ManipFrameHeaderErrorChecks(frame) == true){
        return;
    }
    
    //2.2 doesnt have this
    if(frame->header->idNum <= WXX){
        return;
    }
    
    if(frame->header->flagContent != NULL){
        frame->header->flagContent->tagAlterPreservation = indicator;
        return;
    }

    //just in case the parser gave null
    Id3v2FlagContent *flags = id3v2NewFlagContent(indicator,false,false,false,false,0,0,0);
    frame->header->flagContent = flags;
}

bool id3v2GetFrameAlterPreservationIndicator(Id3v2Frame *frame){
    return (id3v2ManipFlagContentErrorChecks(frame) == true) ? false : frame->header->flagContent->tagAlterPreservation;
}

void id3v2SetFrameFileAlterPreservationIndicator(bool indicator, Id3v2Frame *frame){

    if(id3v2ManipFrameHeaderErrorChecks(frame) == true){
        return;
    }
    
    //2.2 doesnt have this
    if(frame->header->idNum <= WXX){
        return;
    }

    if(frame->header->flagContent != NULL){
        frame->header->flagContent->fileAlterPreservation = indicator;
        return;
    }

    //just in case the parser gave null
    Id3v2FlagContent *flags = id3v2NewFlagContent(false,indicator,false,false,false,0,0,0);
    frame->header->flagContent = flags;
}

bool id3v2GetFrameFileAlterPreservationIndicator(Id3v2Frame *frame){
    return (id3v2ManipFlagContentErrorChecks(frame) == true) ? false : frame->header->flagContent->fileAlterPreservation;
}

void id3v2SetFrameReadOnlyIndicator(bool indicator, Id3v2Frame *frame){

    if(id3v2ManipFrameHeaderErrorChecks(frame) == true){
        return;
    }
    
    //2.2 doesnt have this
    if(frame->header->idNum <= WXX){
        return;
    }

    if(frame->header->flagContent != NULL){
        frame->header->flagContent->readOnly = indicator;
        return;
    }

    //just in case the parser gave null
    Id3v2FlagContent *flags = id3v2NewFlagContent(false,false,indicator,false,false,0,0,0);
    frame->header->flagContent = flags;
}

bool id3v2GetFrameReadOnlyIndicator(Id3v2Frame *frame){
    return (id3v2ManipFlagContentErrorChecks(frame) == true) ? false : frame->header->flagContent->readOnly;
}

void id3v2SetFrameUnsynchronizationIndicator(bool indicator, Id3v2Frame *frame){

    if(id3v2ManipFrameHeaderErrorChecks(frame) == true){
        return;
    }
    
    //2.2 doesnt have this
    if(frame->header->idNum <= WXX){
        return;
    }

    if(frame->header->flagContent != NULL){
        frame->header->flagContent->unsynchronization = indicator;
        return;
    }

    //just in case the parser gave null
    Id3v2FlagContent *flags = id3v2NewFlagContent(false,false,false,indicator,false,0,0,0);
    frame->header->flagContent = flags;

}

bool id3v2GetFrameUnsynchronizationIndicator(Id3v2Frame *frame){
    return (id3v2ManipFlagContentErrorChecks(frame) == true) ? false : frame->header->flagContent->unsynchronization;
}

void id3v2SetFrameDataLengthSize(size_t size, Id3v2Frame *frame){

    if(id3v2ManipFrameHeaderErrorChecks(frame) == true){
        return;
    }
    
    //2.2 doesnt have this
    if(frame->header->idNum <= WXX){
        return;
    }


    if(frame->header->flagContent != NULL){
        
        if(size <= 0){
            frame->header->flagContent->dataLengthIndicator = false;
            frame->header->flagContent->decompressedSize = 0;

            //correct size
            if(frame->header->headerSize > ID3V2_HEADER_SIZE){
                frame->header->headerSize = frame->header->headerSize - ZLIB_COMPRESSION_SIZE;
            }
            return;
        }

        //add size only once if called multiple times
        if(frame->header->flagContent->dataLengthIndicator == false){
                    frame->header->headerSize = frame->header->headerSize + ZLIB_COMPRESSION_SIZE; 
        }

        frame->header->flagContent->dataLengthIndicator = true;
        frame->header->flagContent->decompressedSize = size;

        return;
    }

    //just in case the parser gave null
    Id3v2FlagContent *flags = id3v2NewFlagContent(false,false,false,false,true,size,0,0);
    frame->header->headerSize = frame->header->headerSize + ZLIB_COMPRESSION_SIZE; 
    frame->header->flagContent = flags;
}

size_t id3v2GetFrameDataLengthSize(Id3v2Frame *frame){
    return (id3v2ManipFlagContentErrorChecks(frame) == true) ? false : (frame->header->flagContent->dataLengthIndicator == false) ? 0 : frame->header->flagContent->decompressedSize;
}

void id3v2SetFrameEncryptionMethod(id3byte symbol, Id3v2Frame *frame){

    if(id3v2ManipFrameHeaderErrorChecks(frame) == true){
        return;
    }
    
    //2.2 doesnt have this
    if(frame->header->idNum <= WXX){
        return;
    }

    if(frame->header->flagContent != NULL){

        //add to size only once
        if(frame->header->flagContent->encryption == 0x00){
            frame->header->headerSize = frame->header->headerSize + 1; 
        }

        if(symbol == 0x00){
            frame->header->headerSize = frame->header->headerSize - 1;
        }

        frame->header->flagContent->encryption = symbol;
        return;
    }

    //just in case the parser gave null
    Id3v2FlagContent *flags = id3v2NewFlagContent(false,false,false,false,false,0,symbol,0);
    frame->header->headerSize = frame->header->headerSize + 1; 
    frame->header->flagContent = flags;
}

id3byte id3v2GetFrameEncryptionMethod(Id3v2Frame *frame){
    return (id3v2ManipFlagContentErrorChecks(frame) == true) ? 0x00 : frame->header->flagContent->encryption;
}

void id3v2SetFrameGroup(id3byte symbol, Id3v2Frame *frame){

    if(id3v2ManipFrameHeaderErrorChecks(frame) == true){
        return;
    }
    
    //2.2 doesnt have this
    if(frame->header->idNum <= WXX){
        return;
    }

    if(frame->header->flagContent != NULL){
        
        //add to size only once
        if(frame->header->flagContent->grouping == 0x00){
            frame->header->headerSize = frame->header->headerSize + 1; 
        }

        if(symbol == 0x00){
            frame->header->headerSize = frame->header->headerSize - 1;
        }

        frame->header->flagContent->grouping = symbol;
        return;
    }

    //just in case the parser gave null
    Id3v2FlagContent *flags = id3v2NewFlagContent(false,false,false,false,false,0,0,symbol);
    frame->header->headerSize = frame->header->headerSize + 1; 
    frame->header->flagContent = flags;
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

void id3v2SetEncoding(id3byte encoding, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(id3v2ManipFrameHeaderErrorChecks(frame) == true){
        return;
    }
    
    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    int id = 0;
    unsigned int *sizes = NULL;

    if(frame->header->id[0] == 'T'){
        //there are lots of text frames so this will be the generalization
        id = TXXX;
    }else{
        id = frame->header->idNum;
    }

    //only these frames use an encoding
    switch(id){
        case COM:
            sizes = id3v2ChangeCommentEncoding(frame, encoding);
            break;

        case GEO:
            sizes = id3v2ChangeEncapsulatedObjectEncoding(frame, encoding);
            break;

        case IPL:
            sizes = id3v2ChangeInvolvedPeopleEncoding(frame, encoding);
            break;

        case PIC:
            sizes = id3v2ChangePictureEncoding(frame, encoding);
            break;
            
        case SLT:
            sizes = id3v2ChangeSynchronizedLyricsEncoding(frame, encoding);
            break;

        case ULT: 
            sizes = id3v2ChangeUnsynchronizedLyricsEncoding(frame, encoding);
            break;

        case WXX:
            sizes = id3v2ChangeURLEncoding(frame, encoding); 
            break;

        case APIC:
            sizes = id3v2ChangePictureEncoding(frame, encoding);
            break;

        case COMM:
            sizes = id3v2ChangeCommentEncoding(frame, encoding);
            break;

        case COMR:
            sizes = id3v2ChangeCommercialEncoding(frame, encoding); 
            break;

        case GEOB:
            sizes = id3v2ChangeEncapsulatedObjectEncoding(frame, encoding);
            break;

        case IPLS:
            sizes = id3v2ChangeInvolvedPeopleEncoding(frame, encoding); 
            break;

        case OWNE:
            sizes = id3v2ChangeOwnershipEncoding(frame, encoding);
            break;

        case SYLT:
            sizes = id3v2ChangeSynchronizedLyricsEncoding(frame, encoding);
            break;

        case TXXX:
            sizes = id3v2ChangeTextEncoding(frame, encoding);
            break;

        case USER:
            sizes = id3v2ChangeTermsOfUseEncoding(frame, encoding);
            break;

        case USLT:
            sizes = id3v2ChangeUnsynchronizedLyricsEncoding(frame, encoding);
            break;

        case WXXX: 
            sizes = id3v2ChangeURLEncoding(frame, encoding);
            break;

        default:
            return;

    }

    //fix sizes
    frame->header->frameSize = uSafeSum(frame->header->frameSize, sizes[0], true);
    frame->header->frameSize = uSafeSum(frame->header->frameSize, sizes[1], false);

    if(sizes != NULL){
        free(sizes);
    }
}

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

void id3v2SetDescription(id3buf description, size_t descriptionLength, Id3v2Frame *frame){
   
    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL){
        return;
    }

    if(frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    id3buf newDesc = NULL;
    id3buf currDesc = NULL;
    int newDescLen = 0;
    int currDescLen = 0;
    
    newDesc = id3TextFormatConvert(description,descriptionLength,id3v2GetEncoding(frame));
    newDescLen = id3strlen(newDesc,id3v2GetEncoding(frame));
    
    //keep track of the desc to be removed and replace it
    switch(id3v2GetFrameID(frame)){
        case TXX:
            currDesc = ((Id3v2TextBody *)frame->frame)->description;
            currDescLen = id3strlen(currDesc, id3v2GetEncoding(frame));

            ((Id3v2TextBody *)frame->frame)->description = newDesc;
            break;
        case WXX:
            
            currDesc = ((Id3v2URLBody *)frame->frame)->description;
            currDescLen = id3strlen(currDesc, id3v2GetEncoding(frame));

            ((Id3v2URLBody *)frame->frame)->description = newDesc;            
            break;
        case ULT:
            currDesc = ((Id3v2UnsynchronizedLyricsBody *)frame->frame)->descriptor;
            currDescLen = id3strlen(currDesc, id3v2GetEncoding(frame));
            
            ((Id3v2UnsynchronizedLyricsBody *)frame->frame)->descriptor = newDesc;
            break;
        case SLT:
            currDesc = ((Id3v2SynchronizedLyricsBody *)frame->frame)->descriptor;
            currDescLen = id3strlen(currDesc, id3v2GetEncoding(frame));

            ((Id3v2SynchronizedLyricsBody *)frame->frame)->descriptor = newDesc;
            break;
        case COM:
            currDesc = ((Id3v2CommentBody *)frame->frame)->description;
            currDescLen = id3strlen(currDesc, id3v2GetEncoding(frame));

            ((Id3v2CommentBody *)frame->frame)->description = newDesc;
            break;
        case PIC:
            currDesc = ((Id3v2PictureBody *)frame->frame)->description;
            currDescLen = id3strlen(currDesc, id3v2GetEncoding(frame));

            ((Id3v2PictureBody *)frame->frame)->description = newDesc;
            break;
        case GEO:
            currDesc = ((Id3v2GeneralEncapsulatedObjectBody *)frame->frame)->contentDescription;
            currDescLen = id3strlen(currDesc, id3v2GetEncoding(frame));

            ((Id3v2GeneralEncapsulatedObjectBody *)frame->frame)->contentDescription = newDesc;
            break;
        case TXXX:
            currDesc = ((Id3v2TextBody *)frame->frame)->description;
            currDescLen = id3strlen(currDesc, id3v2GetEncoding(frame));

            ((Id3v2TextBody *)frame->frame)->description = newDesc;            
            break;
        case WXXX:
            currDesc = ((Id3v2URLBody *)frame->frame)->description;
            currDescLen = id3strlen(currDesc, id3v2GetEncoding(frame));

            ((Id3v2URLBody *)frame->frame)->description = newDesc;
            break;
        case USLT:
            currDesc = ((Id3v2UnsynchronizedLyricsBody *)frame->frame)->descriptor;
            currDescLen = id3strlen(currDesc, id3v2GetEncoding(frame));
            
            ((Id3v2UnsynchronizedLyricsBody *)frame->frame)->descriptor = newDesc;
            break;
        case SYLT:
            currDesc = ((Id3v2SynchronizedLyricsBody *)frame->frame)->descriptor;
            currDescLen = id3strlen(currDesc, id3v2GetEncoding(frame));
            
            ((Id3v2SynchronizedLyricsBody *)frame->frame)->descriptor = newDesc;
            break;
        case COMM:
            currDesc = ((Id3v2CommentBody *)frame->frame)->description;
            currDescLen = id3strlen(currDesc, id3v2GetEncoding(frame));
            
            ((Id3v2CommentBody *)frame->frame)->description = newDesc;
            break;
        case APIC:
            currDesc = ((Id3v2PictureBody *)frame->frame)->description;
            currDescLen = id3strlen(currDesc, id3v2GetEncoding(frame));

            ((Id3v2PictureBody *)frame->frame)->description = newDesc;
            break;
        case GEOB:
            currDesc = ((Id3v2GeneralEncapsulatedObjectBody *)frame->frame)->contentDescription;
            currDescLen = id3strlen(currDesc, id3v2GetEncoding(frame));
            
            ((Id3v2GeneralEncapsulatedObjectBody *)frame->frame)->contentDescription = newDesc;
            break;
        case COMR:
            currDesc = ((Id3v2CommercialBody *)frame->frame)->description;
            currDescLen = id3strlen(currDesc, id3v2GetEncoding(frame));
            
            ((Id3v2CommercialBody *)frame->frame)->description = newDesc;
            break;

        //should only ever be ascii
        case CRM: 
            currDesc = ((Id3v2EncryptedMetaBody *)frame->frame)->content;
            currDescLen = id3strlen(currDesc, id3v2GetEncoding(frame));
            
            ((Id3v2CommercialBody *)frame->frame)->description = newDesc;            
            break;
        default:
            return;
    }

    //fix sizes
    frame->header->frameSize = uSafeSum(frame->header->frameSize, currDescLen, true);
    frame->header->frameSize = uSafeSum(frame->header->frameSize, newDescLen, false);

    //free old description
    if(currDesc != NULL){
        free(currDesc);
    }
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

void id3v2SetTextValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    //not a text frame
    if(frame->header->id[0] != 'T'){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    id3buf currValue = NULL;
    id3buf newValue = NULL;
    int currValueLen = 0;
    int newValueLen = 0;

    newValue = id3TextFormatConvert(value, valueLength, id3v2GetEncoding(frame));
    newValueLen = id3strlen(newValue,id3v2GetEncoding(frame));

    currValue = ((Id3v2TextBody *)frame->frame)->value;
    currValueLen = id3strlen(currValue, id3v2GetEncoding(frame));

    ((Id3v2TextBody *)frame->frame)->value = newValue;

    //fix sizes
    frame->header->frameSize = uSafeSum(frame->header->frameSize, currValueLen, true);
    frame->header->frameSize = uSafeSum(frame->header->frameSize, newValueLen, false);

    //free old value
    if(currValue != NULL){
        free(currValue);
    }
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

void id3v2SetURLValue(id3buf value, size_t valueLength, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    if(frame->header->id[0] != 'W'){
        return;
    }

    id3buf curr = NULL;
    id3buf replace = NULL;
    Id3v2URLBody *body = (Id3v2URLBody *)frame->frame;

    //get old value
    curr = body->url;
    frame->header->frameSize = uSafeSum(frame->header->frameSize, id3strlen(curr, ISO_8859_1), true);
    
    //put new value
    replace = id3TextFormatConvert(value, valueLength, ISO_8859_1);
    body->url = replace;
    frame->header->frameSize = uSafeSum(frame->header->frameSize, id3strlen(replace, ISO_8859_1), false);
    
    
    if(curr != NULL){
        free(curr);
    }
}

id3buf id3v2GetURLValue(Id3v2Frame *frame){
    
    if(frame == NULL){
        return NULL;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return NULL;
    }

 

    if(frame->header->id[0] != 'W'){
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

id3buf id3v2GetInvolvedPeopleListPerson(Id3v2Frame *frame){

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

    id3buf ret = NULL;
    Id3v2InvolvedPeopleListBody *body = (Id3v2InvolvedPeopleListBody *)frame->frame; 
    Id3v2InvolvedPerson *iPerson = NULL;

    if(body->involvedPeopleIter != NULL){
        if(body->involvedPeopleIter->curr != NULL){
            iPerson = body->involvedPeopleIter->curr->data;
        }
    }

    if(iPerson == NULL){
        return NULL;
    }

    //copy person

    if(iPerson->person != NULL || iPerson->personLen > 0){
        ret = calloc(sizeof(id3byte), iPerson->personLen + 1);
        memcpy(ret, iPerson->person, iPerson->personLen);
    }

    return ret;
}

id3buf id3v2GetInvolvedPeopleListJob(Id3v2Frame *frame){
    
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

    id3buf ret = NULL;
    Id3v2InvolvedPeopleListBody *body = (Id3v2InvolvedPeopleListBody *)frame->frame; 
    Id3v2InvolvedPerson *iPerson = NULL;

    //get struct
    if(body->involvedPeopleIter != NULL){
        if(body->involvedPeopleIter->curr != NULL){
            iPerson = body->involvedPeopleIter->curr->data;
        }
    }

    if(iPerson == NULL){
        return NULL;
    }

    //copy job

    if(iPerson->job != NULL || iPerson->jobLen > 0){
        ret = calloc(sizeof(id3byte), iPerson->jobLen + 1);
        memcpy(ret, iPerson->job, iPerson->jobLen);
    }

    return ret;
}

void id3v2RemoveInvolvedPerson(id3buf personToRemove,  unsigned int personToRemoveLength, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    switch(frame->header->idNum){
        case IPL:
            break;
        case IPLS:
            break;
        default:
            return;
    }

    id3buf personToCheck = NULL;
    unsigned int checkLen = 0;
    int pos = 0;
    Id3v2InvolvedPeopleListBody *body = NULL;
    Id3v2InvolvedPerson *iPerson = NULL;
    Id3ListIter *li = NULL;

    body = (Id3v2InvolvedPeopleListBody *)frame->frame;
    li = id3NewListIter(body->involvedPeople);

    //convert to the frames encoding
    personToCheck = id3TextFormatConvert(personToRemove, personToRemoveLength, body->encoding);
    checkLen = id3strlen(personToCheck, body->encoding);

    while((iPerson = id3NextListIter(li)) != NULL){
        pos++;
        //check for a matching string
        if(iPerson->personLen == checkLen){
            unsigned int i = 0;

            for(i = 0; i < checkLen; i++){
                
                if(iPerson->person[i] != personToCheck[i]){
                    break;
                }
            }

            //remove the link
            if(i == checkLen){
                iPerson = id3RemoveList(body->involvedPeople, pos-1);

                //removed the head
                if(pos == 1){
                    body->involvedPeopleIter->curr = body->involvedPeopleIter->l->head; 
                }

                if(iPerson != NULL){
                    frame->header->frameSize = uSafeSum(frame->header->frameSize, iPerson->jobLen, true);
                    frame->header->frameSize = uSafeSum(frame->header->frameSize, iPerson->personLen, true);
                    id3v2FreeInvolvedPerson(iPerson);
                }  
                break;
            }
        }
    }

    if(personToCheck != NULL){
        free(personToCheck);
    }

    id3FreeListIter(li);
}

void id3v2SetCDIDValue(char *cdtoc, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    switch(frame->header->idNum){
        case MCI:
            break;
        case MCDI:
            break;
        default:
            return;
    }  

    if(cdtoc == NULL){
        return;
    }

    unsigned int sz = 0;
    unsigned int oldSz = 0;
    id3buf copy = NULL;
    Id3v2MusicCDIdentifierBody *body = NULL;

    sz = strlen(cdtoc);

    copy = calloc(sizeof(id3byte), sz + 1);
    memcpy(copy, cdtoc, sz);

    body = (Id3v2MusicCDIdentifierBody *)frame->frame;

    if(body->cdtoc != NULL){
        oldSz = strlen((char *)body->cdtoc);
        free(body->cdtoc);
    }

    body->cdtoc = copy;
    frame->header->frameSize = uSafeSum(frame->header->frameSize, oldSz, true);
    frame->header->frameSize = uSafeSum(frame->header->frameSize, oldSz, false);

}

id3buf id3v2GetCDIDValue(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

 

    switch(frame->header->idNum){
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

void id3v2SetTimeStampFormat(id3byte format, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    //only these frames use time stamp format
    switch(frame->header->idNum){
        case ETC:
            ((Id3v2EventTimeCodesBody *)frame->frame)->timeStampFormat = format;
        case ETCO:
            ((Id3v2EventTimeCodesBody *)frame->frame)->timeStampFormat = format;
        case STC:
            ((Id3v2SyncedTempoCodesBody *)frame->frame)->timeStampFormat = format;
        case SYTC:
            ((Id3v2SyncedTempoCodesBody *)frame->frame)->timeStampFormat = format;
        case SLT:
            ((Id3v2SynchronizedLyricsBody *)frame->frame)->timeStampFormat = format;
        case SYLT:
            ((Id3v2SynchronizedLyricsBody *)frame->frame)->timeStampFormat = format;
        case POSS:
            ((Id3v2PositionSynchronisationBody *)frame->frame)->timeStampFormat = format;
        default:
            return;
    }

}

int id3v2GetTimeStampFormat(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame)){
        return 0;
    }

    //only these frames use time stamp format
    switch(frame->header->idNum){
        case ETC:
            return ((Id3v2EventTimeCodesBody *)frame->frame)->timeStampFormat;
        case ETCO:
            return ((Id3v2EventTimeCodesBody *)frame->frame)->timeStampFormat;
        case STC:
            return ((Id3v2SyncedTempoCodesBody *)frame->frame)->timeStampFormat;
        case SYTC:
            return ((Id3v2SyncedTempoCodesBody *)frame->frame)->timeStampFormat;
        case SLT:
            return ((Id3v2SynchronizedLyricsBody *)frame->frame)->timeStampFormat;
        case SYLT:
            return ((Id3v2SynchronizedLyricsBody *)frame->frame)->timeStampFormat;
        case POSS:
            return ((Id3v2PositionSynchronisationBody *)frame->frame)->timeStampFormat;
        default:
            return 0;
    }
}

id3byte id3v2GetEventTimeCodeType(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return 0x00;
    }

    if(frame->header->flagContent != NULL){
        if(frame->header->flagContent->encryption > 0){
            return 0x00;
        }
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
    Id3v2EventTimesCodeEvent *e = NULL;


    if(body->eventsTimeCodesIter != NULL){
        if(body->eventsTimeCodesIter->curr != NULL){
            if(body->eventsTimeCodesIter->curr->data != NULL){
                e = (Id3v2EventTimesCodeEvent *)body->eventsTimeCodesIter->curr->data;
                return e->typeOfEvent;
            }
        }

    }


    return 0x00;
}

int id3v2GetEventTimeCodeTimeStamp(Id3v2Frame *frame){

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
    Id3v2EventTimesCodeEvent *e = NULL;

    if(body->eventsTimeCodesIter != NULL){
        if(body->eventsTimeCodesIter->curr != NULL){
            if(body->eventsTimeCodesIter->curr->data != NULL){
                e = (Id3v2EventTimesCodeEvent *)body->eventsTimeCodesIter->curr->data;
                return e->timeStamp;
            }
        }

    }

    return -1;
}

void id3v2RemoveEventTimeCode(int stamp, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    switch(frame->header->idNum){
        case ETC:
            break;
        case ETCO:
            break;
        default:
            return;
    }

    if(stamp < 0){
        return;
    }

    int pos = 0;
    Id3v2EventTimeCodesBody *body = NULL;
    Id3v2EventTimesCodeEvent *event = NULL;
    Id3ListIter *li = NULL;

    body = (Id3v2EventTimeCodesBody *)frame->frame;


    //make an iter and match with the given stamp
    if((li = id3NewListIter(body->eventTimeCodes)) == NULL){
        return;
    }

    while((event = id3NextListIter(li)) != NULL){
        pos++;
        if(stamp == event->timeStamp){
            id3v2FreeEventCode(id3RemoveList(body->eventTimeCodes, pos-1));
            
            //removed the head
            if(pos == 1){
                body->eventsTimeCodesIter->curr = body->eventsTimeCodesIter->l->head; 
            }   

            frame->header->frameSize = uSafeSum(frame->header->frameSize, ID3V2_TIME_STAMP_LEN, true);
            
            break;
        }
    }

    id3FreeListIter(li);
}

void id3v2SetSyncedTempoCodesFrameValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame){

    if(frame == NULL){
        return; 
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(value == NULL || valueLength < 0){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    id3buf copy = NULL;
    Id3v2SyncedTempoCodesBody *body = NULL;
    
    body = (Id3v2SyncedTempoCodesBody *)frame->frame;

    //do sizes
    frame->header->frameSize = uSafeSum(frame->header->frameSize, body->tempoDataLen, true);
    frame->header->frameSize = uSafeSum(frame->header->frameSize, valueLength, false);
    body->tempoDataLen = valueLength;
    
    //change value
    if(body->tempoData != NULL){
        free(body->tempoData);
    }

    //copy value
    copy = calloc(sizeof(id3byte), valueLength + 1);
    memcpy(copy, value, valueLength);
    body->tempoData = copy;
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

void id3v2SetLanguage(id3buf lang, Id3v2Frame *frame){

    if(frame == NULL || lang == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    id3buf copy = NULL;
    unsigned int langLen = 0;
    id3buf rmLang = NULL;

    langLen = strlen((char *)lang);

    //language cannot be greater then 3 theres
    //some ISO standard for this
    if(langLen > ID3V2_LANGUAGE_LEN){
        return;
    }

    copy = calloc(sizeof(id3byte), ID3V2_LANGUAGE_LEN);
    memcpy(copy, lang, ID3V2_LANGUAGE_LEN);

    switch(frame->header->idNum){
        case ULT:
            rmLang = ((Id3v2UnsynchronizedLyricsBody *)frame->frame)->language;
            ((Id3v2UnsynchronizedLyricsBody *)frame->frame)->language = copy;
            break;
        case USLT:
            rmLang = ((Id3v2UnsynchronizedLyricsBody *)frame->frame)->language;
            ((Id3v2UnsynchronizedLyricsBody *)frame->frame)->language = copy;
            break;
        case SLT:
            rmLang = ((Id3v2SynchronizedLyricsBody *)frame->frame)->language;
            ((Id3v2SynchronizedLyricsBody *)frame->frame)->language = copy;
            break;
        case SYLT:
            rmLang = ((Id3v2SynchronizedLyricsBody *)frame->frame)->language;
            ((Id3v2SynchronizedLyricsBody *)frame->frame)->language = copy;
            break;
        case COM:
            rmLang = ((Id3v2CommentBody *)frame->frame)->language;
            ((Id3v2CommentBody *)frame->frame)->language = copy;
            break;
        case COMM:
            rmLang = ((Id3v2CommentBody *)frame->frame)->language;
            ((Id3v2CommentBody *)frame->frame)->language = copy;
            break;
        case USER:
            rmLang = ((Id3v2TermsOfUseBody *)frame->frame)->language;
            ((Id3v2TermsOfUseBody *)frame->frame)->language = copy;
            break;
        default:
            free(copy);
            return;
    }

    if(rmLang != NULL){
        free(rmLang);
    }
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

void id3v2SetUnsynchronizedLyrics(id3buf lyrics, unsigned int lyricLength, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    switch(frame->header->idNum){
        case USLT:
            break;
        case ULT:
            break;
        default:
            return;
    }

    id3buf replace = NULL;
    unsigned int cLen = 0;
    unsigned int rLen = 0;
    Id3v2UnsynchronizedLyricsBody *body = NULL;

    body = (Id3v2UnsynchronizedLyricsBody *)frame->frame;

    //deal with old value
    if(body->lyrics != NULL){
        cLen = id3strlen(body->lyrics, body->encoding);
        free(body->lyrics);
    }

    frame->header->frameSize = uSafeSum(frame->header->frameSize, cLen, true);

    //make new value
    replace = id3TextFormatConvert(lyrics, lyricLength, body->encoding);
    rLen = id3strlen(replace, body->encoding);
    body->lyrics = replace;

    frame->header->frameSize = uSafeSum(frame->header->frameSize, rLen, false);
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

    if(body->lyricsIter != NULL){
        if(body->lyricsIter->curr != NULL){
            if(body->lyricsIter->curr->data != NULL){
                Id3v2StampedLyric *stamp = body->lyricsIter->curr->data;
           
                if(stamp->lyricLen == 0 || stamp->text == NULL){
                    return NULL;
                }
    
                text = calloc(sizeof(id3byte), stamp->lyricLen + id3ReaderAllocationAdd(encoding));
                memcpy(text, stamp->text, stamp->lyricLen);
            
                return text;           
            }
        }
    }

    return NULL;

}

int id3v2GetSynchronizedLyricsTimeStamp(Id3v2Frame *frame){

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
    
    if(body->lyricsIter != NULL){
        if(body->lyricsIter->curr != NULL){
            if(body->lyricsIter->curr->data != NULL){
                return ((Id3v2StampedLyric *)body->lyricsIter->curr->data)->timeStamp;
            }
        }
    }

    return -1;
}

void id3v2RemoveSynchronizedLyric(int timeStamp, Id3v2Frame *frame){
    
    if(frame == NULL || timeStamp < 0){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    switch(frame->header->idNum){
        case SYLT:
            break;
        case SLT:
            break;
        default:
            return;
    }

    int pos = 0;
    Id3v2StampedLyric *lyric = NULL;
    Id3v2SynchronizedLyricsBody *body = NULL;
    Id3ListIter *li = NULL;

    body = (Id3v2SynchronizedLyricsBody *)frame->frame;
    li = id3NewListIter(body->lyrics);

    if(li == NULL){
        return;
    }

    //find a match
    while((lyric = id3NextListIter(li)) != NULL){
        pos++;

        if(lyric->timeStamp == timeStamp){

            frame->header->frameSize = uSafeSum(frame->header->frameSize, lyric->lyricLen, true);
            id3v2FreeStampedLyric(id3RemoveList(body->lyrics, pos-1));
            
            //removed head
            if(pos == 1){
                body->lyricsIter->curr = body->lyricsIter->l->head;
            }
        }
    }

    id3FreeListIter(li);  
}

void id3v2SetCommentValue(id3buf comment, unsigned int commentLength, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    switch(frame->header->idNum){
        case COM:
            break;
        case COMM:
            break;
        default:
            return;
    }
    
    id3buf replace = NULL;
    id3buf curr = NULL;
    Id3v2CommentBody *body = NULL;

    body = (Id3v2CommentBody *)frame->frame;
    curr = body->text;
    
    if(curr != NULL){
        frame->header->frameSize = uSafeSum(frame->header->frameSize, id3strlen(curr, body->encoding), true);
        free(curr);
    }

    replace = id3TextFormatConvert(comment, commentLength, body->encoding);
    frame->header->frameSize = uSafeSum(frame->header->frameSize, id3strlen(replace, body->encoding), false);
    body->text = replace;
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

void id3v2SetSubjectiveValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    id3buf replace = NULL;
    Id3v2SubjectiveBody *body = (Id3v2SubjectiveBody *)frame->frame;

    if(body->value != NULL){
        free(body->value);
    }

    frame->header->frameSize = uSafeSum(frame->header->frameSize, body->valueSize, true);

    if(value != NULL && valueLength != 0){
        replace = calloc(sizeof(id3byte), valueLength + 1);
        memcpy(replace, value, valueLength);
    }

    body->value = replace;
    body->valueSize = valueLength;
    frame->header->frameSize = uSafeSum(frame->header->frameSize, valueLength, false);
}

id3buf id3v2GetSubjectiveValue(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
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

void id3v2SetRelativeVolumeAdjustmentValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame){
    id3v2SetSubjectiveValue(value, valueLength, frame);
}

id3buf id3v2GetRelativeVolumeAdjustmentValue(Id3v2Frame *frame){
    return id3v2GetSubjectiveValue(frame); 
}

void id3v2SetEqualisationValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame){
    id3v2SetSubjectiveValue(value, valueLength, frame);
}

id3buf id3v2GetEqualisationValue(Id3v2Frame *frame){
    return id3v2GetSubjectiveValue(frame); 
}

void id3v2SetReverbValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame){
    id3v2SetSubjectiveValue(value, valueLength, frame);
}

id3buf id3v2GetReverbValue(Id3v2Frame *frame){
    return id3v2GetSubjectiveValue(frame); 
}

void id3v2SetMIMEType(id3buf format, Id3v2Frame *frame){
//format is assumed to always be ISO whatever
    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    id3buf mime = NULL;
    id3buf oldMime = NULL;
    id3buf buildMime = NULL;
    unsigned int mimeLen = 0;
    
    if(format != NULL){
        mimeLen = strlen((char *)format);    
        mime = calloc(sizeof(id3byte), mimeLen + 1);
        memcpy(mime, format, mimeLen);
    }

    switch(frame->header->idNum){
        
        //build a mime with image/
        case APIC:
            id3buf tmp = NULL;
            oldMime = ((Id3v2PictureBody *)frame->frame)->format;

            buildMime = calloc(sizeof(id3byte), mimeLen + ID3V2_MIME_IMG_LEN + 1);
            memcpy(buildMime, "image/", ID3V2_MIME_IMG_LEN);
            tmp = buildMime + ID3V2_MIME_IMG_LEN;
            memcpy(tmp, mime, mimeLen);

            free(mime);
            ((Id3v2PictureBody *)frame->frame)->format = buildMime;
            frame->header->frameSize = uSafeSum(frame->header->frameSize, mimeLen + ID3V2_MIME_IMG_LEN, false);
            break;
        case PIC:
            oldMime = ((Id3v2PictureBody *)frame->frame)->format;
            ((Id3v2PictureBody *)frame->frame)->format = mime;
            frame->header->frameSize = uSafeSum(frame->header->frameSize, mimeLen, false);  
            break;
        case GEO:
            oldMime = ((Id3v2GeneralEncapsulatedObjectBody *)frame->frame)->mimeType;
            ((Id3v2GeneralEncapsulatedObjectBody *)frame->frame)->mimeType = mime;
            frame->header->frameSize = uSafeSum(frame->header->frameSize, mimeLen, false);
            break;
        case GEOB:
            oldMime = ((Id3v2GeneralEncapsulatedObjectBody *)frame->frame)->mimeType;
            ((Id3v2GeneralEncapsulatedObjectBody *)frame->frame)->mimeType = mime;
            frame->header->frameSize = uSafeSum(frame->header->frameSize, mimeLen, false);
            break;
        case COMR:
            oldMime = ((Id3v2CommercialBody *)frame->frame)->mimeType;
            ((Id3v2CommercialBody *)frame->frame)->mimeType = mime;
            frame->header->frameSize = uSafeSum(frame->header->frameSize, mimeLen, false);
            break;
        default:
            free(mime);
            return;
    }
    
    if(oldMime != NULL){
        frame->header->frameSize = uSafeSum(frame->header->frameSize, strlen((char *)oldMime), true);
        free(oldMime);
    }
}

id3buf id3v2GetMIMEType(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return NULL;
    }

 

    id3buf buildMIME = NULL;
    id3buf mime = NULL;

    switch(id3v2GetFrameID(frame)){
        case APIC:
            mime = ((Id3v2PictureBody *)frame->frame)->format; 
            break;
        case PIC:
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

void id3v2SetPictureType(id3byte pictureType, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    switch(frame->header->idNum){
        case PIC:
            ((Id3v2PictureBody *)frame->frame)->pictureType = pictureType;
            break;
        case APIC:
            ((Id3v2PictureBody *)frame->frame)->pictureType = pictureType;
            break;
        default:
            return;
    }

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

void id3v2SetPictureValue(id3buf picture, unsigned int pictureLength, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    id3buf currPic = NULL;
    id3buf replace = NULL;
    unsigned int currLen = 0;
    Id3v2PictureBody *body = NULL;


    if(picture != NULL && pictureLength > 0){
        replace = calloc(sizeof(id3byte), pictureLength + 1);
        memcpy(replace, picture, pictureLength);
        frame->header->frameSize = uSafeSum(frame->header->frameSize, pictureLength, false);
    }

    body = (Id3v2PictureBody *)frame->frame;

    switch(frame->header->idNum){
        case PIC:
            currPic = body->pictureData;
            body->pictureData = replace;

            currLen = body->picSize;
            body->picSize = pictureLength;
            break;
        case APIC:
            currPic = body->pictureData;
            body->pictureData = replace;

            currLen = body->picSize;
            body->picSize = pictureLength;
            break;
        default:
            return;
    }

    if(currPic != NULL){
        free(currPic);
        frame->header->frameSize = uSafeSum(frame->header->frameSize, currLen, true);
    }

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

void id3v2SetObjectFileName(id3buf fileName, unsigned int fileNameLength, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    switch(frame->header->idNum){
        case GEO:
            break;
        case GEOB:
            break;
        default:
            return;
    }

    id3buf replace = NULL;
    id3buf curr = NULL;
    unsigned int oldLen = 0;
    Id3v2GeneralEncapsulatedObjectBody *body = NULL;


    body = (Id3v2GeneralEncapsulatedObjectBody *)frame->frame;

    if(fileName != NULL && fileNameLength > 0){
        replace = id3TextFormatConvert(fileName, fileNameLength, body->encoding);
        frame->header->frameSize = uSafeSum(frame->header->frameSize, id3strlen(replace, body->encoding), false);
    }

    curr = body->filename;

    if(curr != NULL){
        oldLen = id3strlen(curr, body->encoding);
        free(curr);
    }

    body->filename = replace;
    frame->header->frameSize = uSafeSum(frame->header->frameSize, oldLen, true);
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

void id3v2SetGeneralEncapsulatedObjectValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    switch(frame->header->idNum){
        case GEO:
            break;
        case GEOB:
            break;
        default:
            return;
    }

    id3buf curr = NULL;
    id3buf replace = NULL;
    Id3v2GeneralEncapsulatedObjectBody *body = NULL;

    body = (Id3v2GeneralEncapsulatedObjectBody *)frame->frame;

    curr = body->encapsulatedObject;
    frame->header->frameSize = uSafeSum(frame->header->frameSize, body->encapsulatedObjectLen, true);

    if(value != NULL && valueLength > 0){
        replace = calloc(sizeof(id3byte), valueLength + 1);
        memcpy(replace, value, valueLength);
    }

    body->encapsulatedObject = replace;
    body->encapsulatedObjectLen = valueLength;
    frame->header->frameSize = uSafeSum(frame->header->frameSize, valueLength, false);

    if(curr != NULL){
        free(curr);
    }


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

void id3v2SetPlayCount(unsigned long playCount, Id3v2Frame *frame){

    if(frame == NULL){
        return; 
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    switch(frame->header->idNum){
        case CNT:
            ((Id3v2PlayCounterBody *)frame->frame)->counter = playCount;
            break;
        case PCNT:
            ((Id3v2PlayCounterBody *)frame->frame)->counter = playCount;
            break;
        case POP:
            ((Id3v2PopularBody *)frame->frame)->counter = playCount;
            break;
        case POPM:
            ((Id3v2PopularBody *)frame->frame)->counter = playCount;
            break;
        default:
            return;
    }
}

unsigned long id3v2GetPlayCount(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return 0;
    }

    switch(id3v2GetFrameID(frame)){
        case CNT:
            return (unsigned long)((Id3v2PlayCounterBody *)frame->frame)->counter;
        case PCNT:
            return (unsigned long)((Id3v2PlayCounterBody *)frame->frame)->counter;
        case POP:
            return (unsigned long)((Id3v2PopularBody *)frame->frame)->counter;
        case POPM:
            return (unsigned long)((Id3v2PopularBody *)frame->frame)->counter;
        default:
            return 0;
    }
}

void id3v2SetEmail(char *email, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    switch(frame->header->idNum){
        case POP:
            break;
        case POPM:
            break;
        default:
            return;
    }

    id3buf replace = NULL;
    id3buf curr = NULL;
    unsigned int sz = 0;
    Id3v2PopularBody *body = (Id3v2PopularBody *)frame->frame;

    curr = body->email;

    if(curr != NULL){
        frame->header->frameSize = uSafeSum(frame->header->frameSize, strlen((char *)curr), true);
        free(curr);
    }

    if(email != NULL){
        sz = strlen(email);
        replace = calloc(sizeof(id3byte), sz + 1);
        memcpy(replace, email, sz);
        body->email = replace;

        frame->header->frameSize = uSafeSum(frame->header->frameSize, sz, false);
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

void id3v2SetRating(unsigned int rating, Id3v2Frame *frame){
    
    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    switch(frame->header->idNum){
        case POP:
            break;
        case POPM:
            break;
        default:
            return;
    }

    ((Id3v2PopularBody *)frame->frame)->rating = rating;    
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

void id3v2SetOwnerIdentifier(char *owner, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    id3buf replace = NULL;
    id3buf curr = NULL;
    unsigned int newLen = 0;
    
    if(owner != NULL){
        newLen = strlen(owner);
        replace = calloc(sizeof(id3byte), newLen + 1);
        memcpy(replace, owner, newLen);

        frame->header->frameSize = uSafeSum(frame->header->frameSize, newLen, false);
    }

    switch(frame->header->idNum){
        case UFI:
            curr = ((Id3v2UniqueFileIdentifierBody *)frame->frame)->ownerIdentifier;
            ((Id3v2UniqueFileIdentifierBody *)frame->frame)->ownerIdentifier = replace;
            break;
        case UFID:
            curr = ((Id3v2UniqueFileIdentifierBody *)frame->frame)->ownerIdentifier;
            ((Id3v2UniqueFileIdentifierBody *)frame->frame)->ownerIdentifier = replace;
            break;
        case CRM:
            curr = ((Id3v2EncryptedMetaBody *)frame->frame)->ownerIdentifier;
            ((Id3v2EncryptedMetaBody *)frame->frame)->ownerIdentifier = replace;
            break;
        case AENC:
            curr = ((Id3v2AudioEncryptionBody *)frame->frame)->ownerIdentifier;
            ((Id3v2AudioEncryptionBody *)frame->frame)->ownerIdentifier = replace;
            break;
        case ENCR:
            curr = ((Id3v2EncryptionMethodRegistrationBody *)frame->frame)->ownerIdentifier;
            ((Id3v2EncryptionMethodRegistrationBody *)frame->frame)->ownerIdentifier = replace;
            break;
        case GRID:
            curr = ((Id3v2GroupIDRegistrationBody *)frame->frame)->ownerIdentifier;
            ((Id3v2GroupIDRegistrationBody *)frame->frame)->ownerIdentifier = replace;
            break;
        case PRIV:
            curr = ((Id3v2PrivateBody *)frame->frame)->ownerIdentifier;
            ((Id3v2PrivateBody *)frame->frame)->ownerIdentifier = replace;
            break;
        default:
            return;
    }
    
    if(curr != NULL){
        frame->header->frameSize = uSafeSum(frame->header->frameSize, strlen((char *)curr), true);
        free(curr);
    }
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

void id3v2SetEncryptedMetaValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    if(frame->header->idNum != CRM){
        return;
    }

    id3buf replace = NULL;
    id3buf curr = NULL;
    Id3v2EncryptedMetaBody *body = (Id3v2EncryptedMetaBody *)frame->frame;

    if(value != NULL && valueLength > 0){
        replace = calloc(sizeof(id3byte), valueLength + 1);
        memcpy(replace, value, valueLength);
        frame->header->headerSize = uSafeSum(frame->header->headerSize, valueLength, false);
    }

    curr = body->encryptedDatablock;

    if(curr != NULL){
        free(curr);
        frame->header->headerSize = uSafeSum(frame->header->headerSize, body->encryptedDatablockLen, true);
    }

    body->encryptedDatablock = replace;
    body->encryptedDatablockLen = valueLength;
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

void id3v2SetPreviewStart(void *start, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    if(frame->header->idNum != AENC){
        return;
    }

    ((Id3v2AudioEncryptionBody *)frame->frame)->previewStart = start;   
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

void id3v2SetPreviewLength(unsigned int previewLength, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    if(frame->header->idNum != AENC){
        return;
    }

    ((Id3v2AudioEncryptionBody *)frame->frame)->previewLength = previewLength;    
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

void id3v2SetAudioEncryptionValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame){

    if(frame == NULL){
        return; 
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    if(frame->header->idNum != AENC){
        return;
    }

    id3buf replace = NULL;
    id3buf curr = NULL;
    Id3v2AudioEncryptionBody *body = (Id3v2AudioEncryptionBody *)frame->frame;
    
    if(value != NULL && valueLength > 0){
        replace = calloc(sizeof(id3byte), valueLength + 1);
        memcpy(replace, value, valueLength);
        frame->header->frameSize = uSafeSum(frame->header->frameSize, valueLength, false);
    }

    curr = body->encryptionInfo;

    if(curr != NULL){
        free(curr);
        frame->header->frameSize = uSafeSum(frame->header->frameSize, body->encryptionInfoLen, true);
    }

    body->encryptionInfo = replace;
    body->encryptionInfoLen = valueLength;
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

void id3v2SetUniqueFileIdentifierValue(char *identifier, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    switch(frame->header->idNum){
        case UFI:
            break;
        case UFID:
            break;
        default:
            return;
    }

    id3buf curr = NULL;
    id3buf replace = NULL;
    unsigned int sz = 0;
    Id3v2UniqueFileIdentifierBody *body = (Id3v2UniqueFileIdentifierBody *)frame->frame;

    if(identifier != NULL){
        sz = strlen(identifier);
        replace = calloc(sizeof(id3byte), sz + 1);
        memcpy(replace, identifier, sz);
        frame->header->frameSize = uSafeSum(frame->header->frameSize, sz, false);
    }

    curr = body->identifier;

    if(curr != NULL){
        frame->header->frameSize = uSafeSum(frame->header->frameSize, strlen((char *)curr), true);
        free(curr);
    }

    body->identifier = replace;
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

void id3v2SetPositionSynchronisationValue(unsigned long position, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->header == NULL || frame->frame == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    if(frame->header->idNum != POSS){
        return;
    }

    ((Id3v2PositionSynchronisationBody *)frame->frame)->pos = position;
}

unsigned long id3v2GetPositionSynchronisationValue(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return 0;
    }

    if(id3v2GetFrameID(frame) != POSS){
        return 0;
    }

    return ((Id3v2PositionSynchronisationBody *)frame->frame)->pos;
}

void id3v2SetTermsOfUseValue(id3buf termsOfuse, unsigned int termsOfuseLength, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    if(frame->header->idNum != USER){
        return;
    }

    id3buf replace = NULL;
    id3buf curr = NULL;
    Id3v2TermsOfUseBody *body = (Id3v2TermsOfUseBody *)frame->frame;

    if(termsOfuse != NULL && termsOfuseLength > 0){
        replace = id3TextFormatConvert(termsOfuse, termsOfuseLength, body->encoding);
        frame->header->frameSize = uSafeSum(frame->header->frameSize, id3strlen(replace,body->encoding), false);
    }

    curr = body->text;

    if(curr != NULL){
        frame->header->frameSize = uSafeSum(frame->header->headerSize, id3strlen(curr, body->encoding), true);
        free(curr);
    }

    body->text = replace;
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

void id3SetPrice(char *price, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    id3buf replace = NULL;
    id3buf curr = NULL;
    unsigned int len = 0;

    if(price != NULL){
        len = strlen(price);
        replace = calloc(sizeof(char), len + 1);
        memcpy(replace, price, len);
    }
    
    switch(frame->header->idNum){
        case OWNE:
            curr = ((Id3v2OwnershipBody *)frame->frame)->pricePayed;
            ((Id3v2OwnershipBody *)frame->frame)->pricePayed = replace;
            break;
        case COMR:
            curr = ((Id3v2CommercialBody *)frame->frame)->priceString;
            ((Id3v2CommercialBody *)frame->frame)->priceString = replace;
            break;
        default:
            if(replace != NULL){
                free(replace);
            }
            return;
    }

    if(curr != NULL){
        frame->header->frameSize = uSafeSum(frame->header->frameSize, strlen((char *)curr), true);
        free(curr);
    }

    frame->header->frameSize = uSafeSum(frame->header->frameSize, len, false);
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

void id3v2SetPunchDate(char *punch, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    if(frame->header->idNum != OWNE){
        return;
    }

    id3buf replace = NULL;
    id3buf curr = NULL;
    Id3v2OwnershipBody *body = (Id3v2OwnershipBody *)frame->frame;

    if(punch != NULL){
        replace = calloc(sizeof(id3byte), strlen(punch) + 1);
        memcpy(replace, punch, strlen(punch));
    }

    if(body->dateOfPunch != NULL){
        curr = body->dateOfPunch;
        frame->header->frameSize = uSafeSum(frame->header->frameSize, strlen((char *)curr), true);
        free(curr);
    }

    body->dateOfPunch = replace;    
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

void id3v2SetSeller(id3buf seller, unsigned int sellerLength, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    //not an ownership frame
    if(frame->header->idNum != OWNE){
        return;
    }

    id3buf currValue = NULL;
    id3buf newValue = NULL;
    int currValueLen = 0;
    int newValueLen = 0;

    newValue = id3TextFormatConvert(seller, sellerLength, id3v2GetEncoding(frame));
    newValueLen = id3strlen(newValue,id3v2GetEncoding(frame));

    currValue = ((Id3v2OwnershipBody *)frame->frame)->seller;
    currValueLen = id3strlen(currValue, id3v2GetEncoding(frame));

    ((Id3v2OwnershipBody *)frame->frame)->seller = newValue;

    //fix sizes
    frame->header->frameSize = uSafeSum(frame->header->frameSize, currValueLen, true);
    frame->header->frameSize = uSafeSum(frame->header->frameSize, newValueLen, false);

    //free old value
    if(currValue != NULL){
        free(currValue);
    }

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

void id3v2SetValidDate(char *validDate, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    if(frame->header->idNum != COMR){
        return;
    }

    id3buf replace = NULL;
    id3buf curr = NULL;
    unsigned int sz = 0;
    Id3v2CommercialBody *body = (Id3v2CommercialBody *)frame->frame;

    curr = body->validUntil;

    if(curr != NULL){
        frame->header->frameSize = uSafeSum(frame->header->frameSize, strlen((char *)curr), true);
        free(curr);
    }

    if(validDate != NULL){
        sz = strlen(validDate);
        replace = calloc(sizeof(id3byte), sz + 1);
        memcpy(replace, validDate, sz);
        body->validUntil = replace;

        frame->header->frameSize = uSafeSum(frame->header->frameSize, sz, false);
    }
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

void id3v2SetContactURL(char *url, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    if(frame->header->idNum != COMR){
        return;
    }

    id3buf replace = NULL;
    id3buf curr = NULL;
    unsigned int sz = 0;
    Id3v2CommercialBody *body = (Id3v2CommercialBody *)frame->frame;

    curr = body->contractURL;

    if(curr != NULL){
        frame->header->frameSize = uSafeSum(frame->header->frameSize, strlen((char *)curr), true);
        free(curr);
    }

    if(url != NULL){
        sz = strlen(url);
        replace = calloc(sizeof(id3byte), sz + 1);
        memcpy(replace, url, sz);
        body->contractURL = replace;

        frame->header->frameSize = uSafeSum(frame->header->frameSize, sz, false);
    }
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

void id3v2SetCommecialDeliveryMethod(id3byte deliveryMethod, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    if(frame->header->idNum != COMR){
        return;
    }

    ((Id3v2CommercialBody *)frame->frame)->receivedAs = deliveryMethod;

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

void id3v2SetCommercialSellerLogo(id3buf logo, unsigned int logoLength, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    if(frame->header->idNum == COMR){
        return;
    }

    id3buf currPic = NULL;
    id3buf replace = NULL;
    unsigned int currLen = 0;
    Id3v2CommercialBody *body = NULL;   

    if(logo != NULL && logoLength > 0){
        replace = calloc(sizeof(id3byte), logoLength + 1);
        memcpy(replace, logo, logoLength);
        frame->header->frameSize = uSafeSum(frame->header->frameSize, logoLength, false);
    }

    body = (Id3v2CommercialBody *)frame->frame;

    currPic = body->sellerLogo;
    body->sellerLogo = replace;
    currLen = body->sellerLogoLen;
    body->sellerLogoLen = logoLength;

    if(currPic != NULL){
        free(currPic);
        frame->header->frameSize = uSafeSum(frame->header->frameSize, currLen, true);
    }
    
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

void id3v2SetSymbol(id3byte symbol, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->header == NULL || frame->frame == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    switch(frame->header->idNum){
        case ENCR:
            ((Id3v2EncryptionMethodRegistrationBody *)frame->frame)->methodSymbol = symbol;
            break;
        case GRID:
            ((Id3v2GroupIDRegistrationBody *)frame->frame)->groupSymbol = symbol;
            break;
        case SIGN:
            ((Id3v2SignatureBody *)frame->frame)->groupSymbol = symbol;
            break;
        default:
            return;
    }
}

id3byte id3v2GetSymbol(Id3v2Frame *frame){

    if(id3v2ManipFullFrameErrorChecks(frame) == true){
        return 0x00;
    }

    if(frame->header->flagContent != NULL){
        if(frame->header->flagContent->encryption > 0){
            return 0x00;
        }
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

void id3v2SetEncryptionRegistrationValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame != NULL || frame->header != NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    if(frame->header->idNum != ENCR){
        return;
    }

    id3buf replace = NULL;
    id3buf curr = NULL;
    Id3v2EncryptionMethodRegistrationBody *body = (Id3v2EncryptionMethodRegistrationBody *)frame->frame; 

    if(value != NULL && valueLength > 0){
        replace = calloc(sizeof(id3byte), valueLength + 1);
        memcpy(replace, value, valueLength);
    }

    curr = body->encryptionData;

    if(curr != NULL && body->encryptionDataLen > 0){
        free(curr);
        frame->header->frameSize = uSafeSum(frame->header->frameSize, body->encryptionDataLen, true);
    }

    body->encryptionData = replace;
    body->encryptionDataLen = valueLength;
    frame->header->frameSize = uSafeSum(frame->header->frameSize, valueLength, false);
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

void id3v2SetGroupIDValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame != NULL || frame->header != NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    if(frame->header->idNum != GRID){
        return;
    }

    id3buf replace = NULL;
    id3buf curr = NULL;
    Id3v2GroupIDRegistrationBody *body = (Id3v2GroupIDRegistrationBody *)frame->frame; 

    if(value != NULL && valueLength > 0){
        replace = calloc(sizeof(id3byte), valueLength + 1);
        memcpy(replace, value, valueLength);
    }

    curr = body->groupDependentData;

    if(curr != NULL && body->groupDependentDataLen > 0){
        free(curr);
        frame->header->frameSize = uSafeSum(frame->header->frameSize, body->groupDependentDataLen, true);
    }

    body->groupDependentData = replace;
    body->groupDependentDataLen = valueLength;
    frame->header->frameSize = uSafeSum(frame->header->frameSize, valueLength, false);
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

void id3v2SetPrivateValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame != NULL || frame->header != NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    if(frame->header->idNum != PRIV){
        return;
    }

    id3buf replace = NULL;
    id3buf curr = NULL;
    Id3v2PrivateBody *body = (Id3v2PrivateBody *)frame->frame; 

    if(value != NULL && valueLength > 0){
        replace = calloc(sizeof(id3byte), valueLength + 1);
        memcpy(replace, value, valueLength);
    }

    curr = body->privateData;

    if(curr != NULL && body->privateDataLen > 0){
        free(curr);
        frame->header->frameSize = uSafeSum(frame->header->frameSize, body->privateDataLen, true);
    }

    body->privateData = replace;
    body->privateDataLen = valueLength;
    frame->header->frameSize = uSafeSum(frame->header->frameSize, valueLength, false);
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

void id3v2SetSignatureValue(id3buf value, unsigned int valueLength, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame != NULL || frame->header != NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    if(frame->header->idNum != SIGN){
        return;
    }

    id3buf replace = NULL;
    id3buf curr = NULL;
    Id3v2SignatureBody *body = (Id3v2SignatureBody *)frame->frame; 

    if(value != NULL && valueLength > 0){
        replace = calloc(sizeof(id3byte), valueLength + 1);
        memcpy(replace, value, valueLength);
    }

    curr = body->signature;

    if(curr != NULL && body->sigLen > 0){
        free(curr);
        frame->header->frameSize = uSafeSum(frame->header->frameSize, body->sigLen, true);
    }

    body->signature = replace;
    body->sigLen = valueLength;
    frame->header->frameSize = uSafeSum(frame->header->frameSize, valueLength, false);
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

void id3v2SetOffsetToNextTag(int offset, Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    if(frame->header->idNum != SEEK){
        return;
    }

    ((Id3v2SeekBody *)frame->frame)->minimumOffsetToNextTag = offset;
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
