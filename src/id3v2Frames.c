#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v2Frames.h"
#include "id3v2Header.h"
#include "id3Helpers.h"
#include "id3Reader.h"
#include "id3Defines.h"

/*
    FRAME FUNCTIONS
*/

void id3v2FreeFrame(void *toDelete){

    if(toDelete == NULL){
        return;
    }

    Id3v2Frame *curr = (Id3v2Frame *)toDelete;

    if(curr->header == NULL){
        return;
    }

    // free frames
    // all text frames
    if(curr->header->id[0] == 'T'){
        id3v2FreeTextFrame(curr);

    // all url frames
    }else if(curr->header->id[0] == 'W'){
        id3v2FreeURLFrame(curr);

    // Involved people list
    }else if(curr->header->idNum == IPL || curr->header->idNum == IPLS){
        id3v2FreeInvolvedPeopleListFrame(curr);

    // music cd identifier frame
    }else if(curr->header->idNum == MCI || curr->header->idNum == MCDI){
        id3v2FreeMusicCDIdentifierFrame(curr);

    // event timing codes frame
    }else if(curr->header->idNum == ETC || curr->header->idNum == ETCO){
        id3v2FreeEventTimeCodesFrame(curr);

    // synced temp codes
    }else if(curr->header->idNum == STC || curr->header->idNum == SYTC){
        id3v2FreeSyncedTempoCodesFrame(curr);

    // unsynced lyrics
    }else if(curr->header->idNum == ULT || curr->header->idNum == USLT){
        id3v2FreeUnsynchronizedLyricsFrame(curr);

    // synced lyric frame
    }else if(curr->header->idNum == SLT || curr->header->idNum == SYLT){
        id3v2FreeSynchronizedLyricsFrame(curr);

    // comment frame
    }else if(curr->header->idNum == COM || curr->header->idNum == COMM){
        id3v2FreeCommentFrame(curr);

    // picture frame
    }else if(curr->header->idNum == PIC || curr->header->idNum == APIC){
        id3v2FreePictureFrame(curr);

    // general encapsulated object
    }else if(curr->header->idNum == GEO || curr->header->idNum == GEOB){
        id3v2FreeGeneralEncapsulatedObjectFrame(curr);

    // play counter
    }else if(curr->header->idNum == CNT || curr->header->idNum == PCNT){
        id3v2FreePlayCounterFrame(curr);

    // popularmeter
    }else if(curr->header->idNum == POP || curr->header->idNum == POPM){
        id3v2FreePopularFrame(curr);

    // encrypted meta frame 2.2 only
    }else if(curr->header->idNum == CRM){
        id3v2FreeEncryptedMetaFrame(curr);

    // audio encryption frame
    }else if(curr->header->idNum == CRA || curr->header->idNum == AENC){
        id3v2FreeAudioEncryptionFrame(curr);

    // unique file identifier frame
    }else if(curr->header->idNum == UFI || curr->header->idNum == UFID){
        id3v2FreeUniqueFileIdentifierFrame(curr);

    // position synchronisation frame
    }else if(curr->header->idNum == POSS){
        id3v2FreePositionSynchronisationFrame(curr);

    // terms of use frame
    }else if(curr->header->idNum == USER){
        id3v2FreeTermsOfUseFrame(curr);

    // ownership frame
    }else if(curr->header->idNum == OWNE){
        id3v2FreeOwnershipFrame(curr);

    // commercial frame
    }else if(curr->header->idNum == COMR){
        id3v2FreeCommercialFrame(curr);

    // encryption method registration
    }else if(curr->header->idNum == ENCR){
        id3v2FreeCommercialFrame(curr);

    // group id regestration
    }else if(curr->header->idNum == GRID){
        id3v2FreeGroupIDRegistrationFrame(curr);

    // private frame
    }else if(curr->header->idNum == PRIV){
        id3v2FreePrivateFrame(curr);
    
    // relative volume adjustment frame
    }else if(curr->header->idNum == RVA || curr->header->idNum == RVAD || curr->header->idNum == RVA2){
        id3v2FreeRelativeVolumeAdjustmentFrame(curr);
    
    // equalization frame
    }else if(curr->header->idNum == EQU || curr->header->idNum == EQUA || curr->header->idNum == EQU2){
        id3v2FreeEqualisationFrame(curr);
    
    // reverb frame
    }else if(curr->header->idNum == REV || curr->header->idNum == RVRB){
        id3v2FreeReverbFrame(curr);
    
    // signature frame 2.4 only
    }else if(curr->header->idNum == SIGN){
        id3v2FreeSignatureFrame(curr);
    
    // seek frame 2.4 only
    }else if(curr->header->idNum == SEEK){
        id3v2FreeSeekFrame(curr);
    }
}

Id3v2Frame *id3v2NewFrame(Id3v2FrameHeader *header, void *bodyContent){

    Id3v2Frame *frame = malloc(sizeof(Id3v2Frame));

    frame->header = header;
    frame->frame = bodyContent;

    return frame;
}

List *id3v2ExtractFrames(unsigned char *buffer, Id3v2Header *header){

    int saveSize = header->size - ID3V2_HEADER_SIZE;
    int extHeaderOffset = 0;
    int titleNameAndSizeLen = 0;

    // if the version is invalid nothing can be parsed
    if((titleNameAndSizeLen = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    // account for an extended header
    if(header->extendedHeader != NULL){
        extHeaderOffset = header->extendedHeader->size + header->extendedHeader->padding;
        saveSize = saveSize - extHeaderOffset;
    }

    Id3Reader *stream = id3NewReader(buffer, saveSize);

    // create list
    List *list = newList(id3v2FreeFrame, id3v2CopyFrame);

    while(saveSize > 0){

        Id3v2Frame *frame = NULL;

        // get a frame to add to the list
        if((frame = id3v2ParseFrame(id3ReaderCursor(stream), header)) != NULL){
            listPush(list, (void *)frame);
            id3ReaderSeek(stream, frame->header->frameSize + frame->header->headerSize, SEEK_CUR);
            saveSize = saveSize - frame->header->frameSize - frame->header->headerSize;

        // recovery
        }else{
            // the idea here is to read just the frame header and skip everything else
            Id3v2FrameHeader *recovery = id3v2ParseFrameHeader(id3ReaderCursor(stream), header);

            if(recovery == NULL){
                break;
            }

            if(recovery->frameSize == 0 || recovery->headerSize == 0){
                id3v2FreeFrameHeader(recovery);
                break;
            }

            id3ReaderSeek(stream, recovery->frameSize + recovery->headerSize, SEEK_CUR);
            saveSize = saveSize - recovery->frameSize - recovery->headerSize;
            id3v2FreeFrameHeader(recovery);
        }
    }

    id3FreeReader(stream);

    return list;
}

Id3v2Frame *id3v2ParseFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    if(header == NULL){
        return NULL;
    }

    Id3v2Frame *newFrame = NULL;
    int idSize = id3v2IdAndSizeOffset(header);

    // text frames
    if(buffer[0] == 'T'){
        newFrame = id3v2ParseTextFrame(buffer, header);

    // URL frames
    }else if(buffer[0] == 'W'){
        newFrame = id3v2ParseURLFrame(buffer, header);

    // involved peoples list
    }else if((memcmp(buffer, "IPL", idSize) == 0) || (memcmp(buffer, "IPLS", idSize) == 0)){
        newFrame = id3v2ParseInvolvedPeopleListFrame(buffer, header);

    // music cd identifier
    }else if((memcmp(buffer, "MCI", idSize) == 0) || (memcmp(buffer, "MCDI", idSize) == 0)){
        newFrame = id3v2ParseMusicCDIdentifierFrame(buffer, header);

    // event time codes
    }else if((memcmp(buffer, "ETC", idSize) == 0) || (memcmp(buffer, "ETCO", idSize) == 0)){
        newFrame = id3v2ParseEventTimeCodesFrame(buffer, header);

    // synced tempo codes
    }else if((memcmp(buffer, "STC", idSize) == 0) || (memcmp(buffer, "SYTC", idSize) == 0)){
        newFrame = id3v2ParseSyncedTempoCodesFrame(buffer, header);

    // unsynced lyrics
    }else if((memcmp(buffer, "ULT", idSize) == 0) || (memcmp(buffer, "USLT", idSize) == 0)){
        newFrame = id3v2ParseUnsynchronizedLyricsFrame(buffer, header);

    // synced lyric frame
    }else if ((memcmp(buffer, "SLT", idSize) == 0) || (memcmp(buffer, "SYLT", idSize) == 0)){
        newFrame = id3v2ParseSynchronizedLyricsFrame(buffer, header);

    // comment frame
    }else if((memcmp(buffer,"COM",idSize) == 0) || (memcmp(buffer,"COMM",idSize) == 0)){
         newFrame = id3v2ParseCommentFrame(buffer, header);

    // relative volume adjustment frame 
    }else if((memcmp(buffer,"RVA",idSize) == 0) || (memcmp(buffer,"RVAD",idSize) == 0) || (memcmp(buffer,"RVA2",idSize) == 0)){
        newFrame = id3v2ParseRelativeVolumeAdjustmentFrame(buffer, header);
    
    // picture frame
    }else if((memcmp(buffer,"PIC",idSize) == 0) || (memcmp(buffer,"APIC",idSize) == 0)){
        newFrame = id3v2ParsePictureFrame(buffer, header);
    
    // equalization frame
    }else if((memcmp(buffer,"EQU",idSize) == 0) || (memcmp(buffer,"EQUA",idSize) == 0) || (memcmp(buffer,"EQU2",idSize) == 0)){
        newFrame = id3v2ParseEqualisationFrame(buffer, header);
    
    // reverb frame
    }else if((memcmp(buffer,"REV",idSize) == 0) || (memcmp(buffer,"RVRB",idSize) == 0)){
        newFrame = id3v2ParseReverbFrame(buffer, header);

    // general encapsulated object
    }else if((memcmp(buffer,"GEO",idSize) == 0) || (memcmp(buffer,"GEOB",idSize) == 0)){
        newFrame = id3v2ParseGeneralEncapsulatedObjectFrame(buffer, header);
    
    // play counter frame
    }else if((memcmp(buffer,"CNT",idSize) == 0) || (memcmp(buffer,"PCNT",idSize) == 0)){
        newFrame = id3v2ParsePlayCounterFrame(buffer, header);
    
    // popularmeter frame
    }else if((memcmp(buffer,"POP",idSize) == 0) || (memcmp(buffer,"POPM",idSize) == 0)){
        newFrame = id3v2ParsePopularFrame(buffer, header);
    
    // encrypted meta frame 2.2 only
    }else if(memcmp(buffer,"CRM",idSize) == 0){
        newFrame = id3v2ParseEncryptedMetaFrame(buffer, header);

    // audio encryption frame
    }else if((memcmp(buffer,"CRA",idSize) == 0) || (memcmp(buffer,"AENC",idSize) == 0)){
        newFrame = id3v2ParseAudioEncryptionFrame(buffer, header);

    // unique file identifier frame
    }else if((memcmp(buffer,"UFI",idSize) == 0) || (memcmp(buffer,"UFID",idSize) == 0)){
        newFrame = id3v2ParseUniqueFileIdentiferFrame(buffer, header);
    
    // position synchronisation frame
    }else if(memcmp(buffer,"POSS",idSize) == 0){
        newFrame = id3v2ParsePositionSynchronisationFrame(buffer, header);
    
    // terms of use frame
    }else if(memcmp(buffer,"USER",idSize) == 0){
        newFrame = id3v2ParseTermsOfUseFrame(buffer, header);

    // ownership frame
    }else if(memcmp(buffer,"OWNE",idSize) == 0){
        newFrame = id3v2ParseOwnershipFrame(buffer, header);

    // commercial frame
    }else if(memcmp(buffer,"COMR",idSize) == 0){
        newFrame = id3v2ParseCommercialFrame(buffer, header);

    // encryption method registartion
    }else if(memcmp(buffer,"ENCR",idSize) == 0){
        newFrame = id3v2ParseEncryptionMethodRegistrationFrame(buffer, header);

    // group id registration frame
    }else if(memcmp(buffer,"GRID",idSize) == 0){
        newFrame = id3v2ParseGroupIDRegistrationFrame(buffer, header);

    // private frame
    }else if(memcmp(buffer,"PRIV",idSize) == 0){
        newFrame = id3v2ParsePrivateFrame(buffer, header);
    
    // signature frame
    }else if(memcmp(buffer,"SIGN",idSize)){
        newFrame = id3v2ParseSignatureFrame(buffer, header);

    // seek frame
    }else if(memcmp(buffer,"SEEK",idSize)){
        newFrame = id3v2ParseSeekFrame(buffer, header);

    }

    return newFrame;
}

void *id3v2CopyFrame(void *toCopy){

    if(toCopy == NULL){
        return NULL;
    }

    Id3v2Frame *curr = toCopy;

    // all text frames
    if(curr->header->id[0] == 'T'){
        return id3v2CopyTextFrame(curr);

    // all url frames
    }else if(curr->header->id[0] == 'W'){
        return id3v2CopyURLFrame(curr);

    // Involved people list
    }else if(curr->header->idNum == IPL || curr->header->idNum == IPLS){
        return id3v2CopyInvolvedPeopleListFrame(curr);

    // music cd identifier frame
    }else if(curr->header->idNum == MCI || curr->header->idNum == MCDI){
        return id3v2CopyMusicCDIdentifierFrame(curr);

    // event timing codes frame
    }else if(curr->header->idNum == ETC || curr->header->idNum == ETCO){
        return id3v2CopyEventTimeCodesFrame(curr);

    // synced temp codes
    }else if(curr->header->idNum == STC || curr->header->idNum == SYTC){
        return id3v2CopySyncedTempoCodesFrame(curr);

    // unsynced lyrics
    }else if(curr->header->idNum == ULT || curr->header->idNum == USLT){
        return id3v2CopyUnsynchronizedLyricsFrame(curr);

    // synced lyric frame
    }else if(curr->header->idNum == SLT || curr->header->idNum == SYLT){
        return id3v2CopySynchronizedLyricsFrame(curr);

    // comment frame
    }else if(curr->header->idNum == COM || curr->header->idNum == COMM){
        return id3v2CopyCommentFrame(curr);

    // picture frame
    }else if(curr->header->idNum == PIC || curr->header->idNum == APIC){
        return id3v2CopyPictureFrame(curr);

    // general encapsulated object
    }else if(curr->header->idNum == GEO || curr->header->idNum == GEOB){
        return id3v2CopyGeneralEncapsulatedObjectFrame(curr);

    // play counter
    }else if(curr->header->idNum == CNT || curr->header->idNum == PCNT){
        return id3v2CopyPlayCounterFrame(curr);

    // popularmeter
    }else if(curr->header->idNum == POP || curr->header->idNum == POPM){
        return id3v2CopyPopularFrame(curr);

    // encrypted meta frame 2.2 only
    }else if(curr->header->idNum == CRM){
        return id3v2CopyEncryptedMetaFrame(curr);

    // audio encryption frame
    }else if(curr->header->idNum == CRA || curr->header->idNum == AENC){
        return id3v2CopyAudioEncryptionFrame(curr);

    // unique file identifier frame
    }else if(curr->header->idNum == UFI || curr->header->idNum == UFID){
        return id3v2CopyUniqueFileIdentifierFrame(curr);

    // position synchronisation frame
    }else if(curr->header->idNum == POSS){
        return id3v2CopyPositionSynchronisationFrame(curr);

    // terms of use frame
    }else if(curr->header->idNum == USER){
        return id3v2CopyTermsOfUseFrame(curr);

    // ownership frame
    }else if(curr->header->idNum == OWNE){
        return id3v2CopyOwnershipFrame(curr);

    // commercial frame
    }else if(curr->header->idNum == COMR){
        return id3v2CopyCommercialFrame(curr);

    // encryption method registration
    }else if(curr->header->idNum == ENCR){
        return id3v2CopyCommercialFrame(curr);

    // group id regestration
    }else if(curr->header->idNum == GRID){
        return id3v2CopyGroupIDRegistrationFrame(curr);

    // private frame
    }else if(curr->header->idNum == PRIV){
        return id3v2CopyPrivateFrame(curr);
    
    // relative volume adjustment frame
    }else if(curr->header->idNum == RVA || curr->header->idNum == RVAD || curr->header->idNum == RVA2){
        return id3v2CopyRelativeVolumeAdjustmentFrame(curr);
    
    // equalization frame
    }else if(curr->header->idNum == EQU || curr->header->idNum == EQUA || curr->header->idNum == EQU2){
        return id3v2CopyEqualisationFrame(curr);
    
    // reverb frame
    }else if(curr->header->idNum == REV || curr->header->idNum == RVRB){
        return id3v2CopyReverbFrame(curr);
    
    // signature frame 2.4 only
    }else if(curr->header->idNum == SIGN){
        return id3v2CopySignatureFrame(curr);
    
    // seek frame 2.4 only
    }else if(curr->header->idNum == SEEK){
        return id3v2CopySeekFrame(curr);
    }

    return NULL;
}

Id3v2Frame *id3v2SearchFrame(Id3v2Tag *tag, Id3v2FrameId id){
    
    if(tag == NULL){
        return NULL;
    }

    if(tag->frames == NULL){
        return NULL;
    }


    Node *n = tag->frames->head;

    while(n != NULL){
        
        Id3v2Frame *sFrame = (Id3v2Frame *) n->data;
        
        if(sFrame->header->idNum == id){
            return sFrame;
        }

        n = n->next;
    }

    return NULL;
}

/*
    Text frame functions
*/

Id3v2Frame *id3v2ParseTextFrame(unsigned char *buffer, Id3v2Header *header){

    if (buffer == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if ((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2TextBody *newTextBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    //error checking
    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    if(newHeader->id[0] != 'T'){
        id3v2FreeFrameHeader(newHeader);
        return NULL;     
    }

    buffer = buffer + newHeader->headerSize;
    newTextBody = id3v2ParseTextBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newTextBody);
}

Id3v2Frame *id3v2CopyTextFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2TextBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyTextBody(frame->frame);

    return id3v2NewFrame(header, body);
}

Id3v2TextBody *id3v2CopyTextBody(Id3v2TextBody *body){

    if(body == NULL){
        return NULL;
    }

    unsigned char *description = NULL;
    unsigned char *value = NULL;
    unsigned char encoding = 0;

    encoding = body->encoding;

    if(body->description != NULL){
        description = calloc(sizeof(unsigned char), id3strlen(body->description, encoding) + id3ReaderAllocationAdd(encoding));
        memcpy(description, body->description, id3strlen(body->description, encoding));
    }

    if(body->value != NULL){
        value = calloc(sizeof(unsigned char), id3strlen(body->value, encoding) + id3ReaderAllocationAdd(encoding));
        memcpy(value, body->value, id3strlen(body->value, encoding));
    }

    return id3v2NewTextBody(encoding, value, description);
}

Id3v2TextBody *id3v2ParseTextBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }

    unsigned char *description = NULL;
    unsigned char *value = NULL;
    unsigned char encoding = 0;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    // set encoding
    encoding = id3ReaderCursor(stream)[0];
    id3ReaderSeek(stream, 1, SEEK_CUR);

    if(frameHeader->idNum == TXX || frameHeader->idNum == TXXX){
        // copy description and skip spacer
        description = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, encoding) + id3ReaderAllocationAdd(encoding));
        id3ReaderRead(stream, description, id3ReaderReadEncodedSize(stream, encoding));
        id3ReaderSeek(stream, id3ReaderAllocationAdd(encoding), SEEK_CUR);
    }

    // copy value
    value = id3ReaderEncodedRemainder(stream, encoding);

    id3FreeReader(stream);
    return id3v2NewTextBody(encoding, value, description);
}

Id3v2TextBody *id3v2NewTextBody(unsigned char encoding, unsigned char *value, unsigned char *description){

    Id3v2TextBody *newTextBody = malloc(sizeof(Id3v2TextBody));

    newTextBody->encoding = encoding;
    newTextBody->value = value;
    newTextBody->description = description;

    return newTextBody;
}

void id3v2FreeTextFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2TextBody *body = (Id3v2TextBody *)toDelete->frame;

    if(body->description != NULL){
        free(body->description);
    }

    if(body->value != NULL){
        free(body->value);
    }

    free(body);
    free(toDelete);
}

/*
    URL frame functions
*/

Id3v2Frame *id3v2ParseURLFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2URLBody *newURLBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    //error checking
    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    if(newHeader->id[0] != 'W'){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newURLBody = id3v2ParseURLBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newURLBody);
}

Id3v2Frame *id3v2CopyURLFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2URLBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyURLBody(frame->frame);

    return id3v2NewFrame(header, body);
}

Id3v2URLBody *id3v2CopyURLBody(Id3v2URLBody *body){

    if(body == NULL){
        return NULL;
    }

    unsigned char *description = NULL;
    unsigned char *url = NULL;
    unsigned char encoding = ISO_8859_1;

    encoding = body->encoding;

    if(url != NULL){
        url = calloc(sizeof(unsigned char), strlen((char *)body->url) + 1);
        memcpy(url, body->url, strlen((char *)body->url));
    }

    if(body->description != NULL){
        description = calloc(sizeof(unsigned char), id3strlen(body->description, encoding) + id3ReaderAllocationAdd(encoding));
        memcpy(description, body->description, id3strlen(body->description, encoding));
    }

    return id3v2NewURLBody(encoding, url, description);
}

Id3v2URLBody *id3v2ParseURLBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }

    unsigned char *description = NULL;
    unsigned char *url = NULL;
    unsigned char encoding = ISO_8859_1; //will only ever change if wxx(x)

    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    if(frameHeader->idNum == WXX || frameHeader->idNum == WXXX){

        // set encoding
        encoding = id3ReaderCursor(stream)[0];
        id3ReaderSeek(stream, 1, SEEK_CUR);

        // user defined url frame parse description and skip space
        description = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, encoding) + id3ReaderAllocationAdd(encoding));
        id3ReaderRead(stream, description, id3ReaderReadEncodedSize(stream, encoding));
        id3ReaderSeek(stream, id3ReaderAllocationAdd(encoding), SEEK_CUR);
    }

    // copy url
    url = id3ReaderEncodedRemainder(stream, encoding);

    id3FreeReader(stream);
    return id3v2NewURLBody(encoding, url, description);
}

Id3v2URLBody *id3v2NewURLBody(unsigned char encoding, unsigned char *url, unsigned char *description){

    Id3v2URLBody *new2URLBody = malloc(sizeof(Id3v2URLBody));

    new2URLBody->encoding = encoding;
    new2URLBody->description = description;
    new2URLBody->url = url;

    return new2URLBody;
}

void id3v2FreeURLFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2URLBody *body = (Id3v2URLBody *)toDelete->frame;

    if(body->description != NULL){
        free(body->description);
    }

    if(body->url != NULL){
        free(body->url);
    }

    free(body);
    free(toDelete);
}

/*
    Involved persons list frame functions
*/

Id3v2Frame *id3v2ParseInvolvedPeopleListFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2InvolvedPeopleListBody *newInvolvedPeopleListBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    if(newHeader->idNum != IPL && newHeader->idNum != IPLS){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newInvolvedPeopleListBody = id3v2ParseInvolvedPeopleListBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newInvolvedPeopleListBody);
}

Id3v2Frame *id3v2CopyInvolvedPeopleListFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2InvolvedPeopleListBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyInvolvedPeopleListBody(frame->frame);

    return id3v2NewFrame(header, body);
}

Id3v2InvolvedPeopleListBody *id3v2CopyInvolvedPeopleListBody(Id3v2InvolvedPeopleListBody *body){

    if(body == NULL){
        return NULL;
    }

    unsigned char encoding = 0x00;
    unsigned char *text = NULL;


    encoding = body->encoding;

    if(body->peopleListStrings != NULL){
        text = calloc(sizeof(unsigned char), id3strlen(body->peopleListStrings, encoding) + id3ReaderAllocationAdd(encoding));
        memcpy(text, body->peopleListStrings, id3strlen(body->peopleListStrings, encoding));
    }

    return id3v2NewInvolvedPeopleListBody(encoding, text);
}

Id3v2InvolvedPeopleListBody *id3v2ParseInvolvedPeopleListBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }

    unsigned char encoding = 0x00;
    unsigned char *text = NULL;

    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    // copy encoding
    encoding = id3ReaderCursor(stream)[0];
    id3ReaderSeek(stream, 1, SEEK_CUR);

    // copy text
    text = id3ReaderEncodedRemainder(stream, encoding);

    id3FreeReader(stream);
    return id3v2NewInvolvedPeopleListBody(encoding, text);
}

Id3v2InvolvedPeopleListBody *id3v2NewInvolvedPeopleListBody(unsigned char encoding, unsigned char *peopleListStrings){

    Id3v2InvolvedPeopleListBody *involvedPeopleListBody = malloc(sizeof(Id3v2InvolvedPeopleListBody));

    // copy data
    involvedPeopleListBody->encoding = encoding;
    involvedPeopleListBody->peopleListStrings = peopleListStrings;

    return involvedPeopleListBody;
}

void id3v2FreeInvolvedPeopleListFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2InvolvedPeopleListBody *body = (Id3v2InvolvedPeopleListBody *)toDelete->frame;

    if(body->peopleListStrings != NULL){
        free(body->peopleListStrings);
    }

    free(body);
    free(toDelete);
}

/*
    Music CD identifier frame functions
*/

Id3v2Frame *id3v2ParseMusicCDIdentifierFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2MusicCDIdentifierBody *newMusicCDIdentifierBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    if(newHeader->idNum != MCI && newHeader->idNum != MCDI){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newMusicCDIdentifierBody = id3v2ParseMusicCDIdentifierBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newMusicCDIdentifierBody);
}

Id3v2Frame *id3v2CopyMusicCDIdentifierFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2MusicCDIdentifierBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyMusicCDIdentifierBody(frame->frame);

    return id3v2NewFrame(header, body);

}

Id3v2MusicCDIdentifierBody *id3v2CopyMusicCDIdentifierBody(Id3v2MusicCDIdentifierBody *body){

    if(body == NULL){
        return NULL;
    }

    unsigned char *cdtoc = NULL;

    if(body->cdtoc != NULL){
        cdtoc = calloc(sizeof(unsigned char), sizeof(body) - sizeof(Id3v2MusicCDIdentifierBody) + 1);
        memcpy(cdtoc, body->cdtoc, sizeof(body) - sizeof(Id3v2MusicCDIdentifierBody));
    }

    return id3v2NewMusicCDIdentifierBody(cdtoc);
}

Id3v2MusicCDIdentifierBody *id3v2ParseMusicCDIdentifierBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }

    unsigned char *cdtoc = NULL;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    // copy cd toc
    cdtoc = calloc(sizeof(unsigned char), frameHeader->frameSize + 1);
    id3ReaderRead(stream, cdtoc, frameHeader->frameSize);

    return id3v2NewMusicCDIdentifierBody(cdtoc);
}

Id3v2MusicCDIdentifierBody *id3v2NewMusicCDIdentifierBody(unsigned char *cdtoc){

    Id3v2MusicCDIdentifierBody *musicCdIdentifierBody = malloc(sizeof(Id3v2MusicCDIdentifierBody));

    // copy data
    musicCdIdentifierBody->cdtoc = cdtoc;

    return musicCdIdentifierBody;
}

void id3v2FreeMusicCDIdentifierFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2MusicCDIdentifierBody *body = (Id3v2MusicCDIdentifierBody *)toDelete->frame;

    if(body->cdtoc != NULL){
        free(body->cdtoc);
    }

    free(body);
    free(toDelete);
}

/*
    Event time codes frame functions
*/

Id3v2Frame *id3v2ParseEventTimeCodesFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2EventTimeCodesBody *newEventTimeCodesBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    if(newHeader->idNum != ETC && newHeader->idNum != ETCO){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newEventTimeCodesBody = id3v2ParseEventTimeCodesBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newEventTimeCodesBody);
}

Id3v2Frame *id3v2CopyEventTimeCodesFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2EventTimeCodesBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyEventTimeCodesBody(frame->frame);

    return id3v2NewFrame(header, body);
}

Id3v2EventTimeCodesBody *id3v2CopyEventTimeCodesBody(Id3v2EventTimeCodesBody *body){
    return (body == NULL) ? NULL : id3v2NewEventTimeCodesBody(body->timeStampFormat, copyList(body->eventTimeCodes));
}

Id3v2EventTimeCodesBody *id3v2ParseEventTimeCodesBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }
    
    //-1 to skip time format
    int saveSize = frameHeader->frameSize - 1;
    unsigned int timeStampFormat = 0;
    List *events = NULL;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    // copy time stamp format
    timeStampFormat = id3ReaderCursor(stream)[0];
    id3ReaderSeek(stream, 1, SEEK_CUR);

    // get a list of event codes
    events = newList(id3v2FreeEventCode, id3v2CopyEventCodeEvent);

    while(saveSize > 0){

        long stamp = 0;
        unsigned char typeOfEvent = 0x00;
        Id3v2EventTimesCodeEvent *newEvent = NULL;

        // set event code
        typeOfEvent = id3ReaderCursor(stream)[0];
        id3ReaderSeek(stream, 1, SEEK_CUR);

        // copy the time stamp
        stamp = getBits8(id3ReaderCursor(stream), ID3V2_TIME_STAMP_LEN);
        id3ReaderSeek(stream, ID3V2_TIME_STAMP_LEN, SEEK_CUR);

        // add event to the list
        newEvent = id3v2NewEventCodeEvent(typeOfEvent, stamp);
        listPush(events, newEvent);

        saveSize = saveSize - ID3V2_EVENT_CODE_LEN;
    }

    id3FreeReader(stream);
    return id3v2NewEventTimeCodesBody(timeStampFormat, events);
}

Id3v2EventTimeCodesBody *id3v2NewEventTimeCodesBody(unsigned int timeStampFormat, List *events){

    Id3v2EventTimeCodesBody *eventTimeCodesBody = malloc(sizeof(Id3v2EventTimeCodesBody));

    // copy data
    eventTimeCodesBody->timeStampFormat = timeStampFormat;
    eventTimeCodesBody->eventTimeCodes = events;

    return eventTimeCodesBody;
}

Id3v2EventTimesCodeEvent *id3v2NewEventCodeEvent(unsigned char typeOfEvent, long timeStamp){

    Id3v2EventTimesCodeEvent *eventCode = malloc(sizeof(Id3v2EventTimesCodeEvent));

    eventCode->typeOfEvent = typeOfEvent;
    eventCode->timeStamp = timeStamp;

    return eventCode;
}

void id3v2FreeEventTimeCodesFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2EventTimeCodesBody *body = (Id3v2EventTimeCodesBody *)toDelete->frame;

    destroyList(body->eventTimeCodes);
    free(body);
    free(toDelete);
}

void *id3v2CopyEventCodeEvent(void *toCopy){
    
    if(toCopy == NULL){
        return NULL;
    }

    Id3v2EventTimesCodeEvent *event = (Id3v2EventTimesCodeEvent *)toCopy;
    
    return id3v2NewEventCodeEvent(event->typeOfEvent, event->timeStamp);
}

void id3v2FreeEventCode(void *toDelete){

    if(toDelete == NULL){
        return;
    }

    free(toDelete);
}

/*
    Synced tempo codes frame functions
*/

Id3v2Frame *id3v2ParseSyncedTempoCodesFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2SyncedTempoCodesBody *newSyncedTempoCodesBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    if(newHeader->idNum != STC && newHeader->idNum != SYTC){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newSyncedTempoCodesBody = id3v2ParseSyncedTempoCodesBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newSyncedTempoCodesBody);
}

Id3v2Frame *id3v2CopySyncedTempoCodesFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2SyncedTempoCodesBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopySyncedTempoCodesBody(frame->frame);

    return id3v2NewFrame(header, body);

}

Id3v2SyncedTempoCodesBody *id3v2CopySyncedTempoCodesBody(Id3v2SyncedTempoCodesBody *body){

    unsigned char timeStampFormat = 0x00;
    unsigned char *tempoData = NULL;
    unsigned int tempoDataLen = 0;

    timeStampFormat = body->timeStampFormat;
    tempoDataLen = body->tempoDataLen;

    if(tempoData != NULL){
        tempoData = calloc(sizeof(unsigned char), tempoDataLen + 1);
        memcpy(tempoData, body->tempoData, body->tempoDataLen);
    }

    return id3v2NewSyncedTempoCodesBody(timeStampFormat, tempoData, tempoDataLen);
}

Id3v2SyncedTempoCodesBody *id3v2ParseSyncedTempoCodesBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    unsigned char timeStampFormat = 0x00;
    unsigned char *tempoData = NULL;
    unsigned int tempoDataLen = frameHeader->headerSize - 1;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    // copy time stamp format
    timeStampFormat = id3ReaderCursor(stream)[0];
    id3ReaderSeek(stream, 1, SEEK_CUR);

    // copy data
    tempoData = calloc(sizeof(unsigned char), frameHeader->headerSize);
    id3ReaderRead(stream, tempoData, tempoDataLen);

    id3FreeReader(stream);
    return id3v2NewSyncedTempoCodesBody(timeStampFormat, tempoData, tempoDataLen);
}

Id3v2SyncedTempoCodesBody *id3v2NewSyncedTempoCodesBody(unsigned char timeStampFormat, unsigned char *tempoData, unsigned int tempoDataLen){

    Id3v2SyncedTempoCodesBody *syncedTempoCode = malloc(sizeof(Id3v2SyncedTempoCodesBody));

    syncedTempoCode->tempoData = tempoData;
    syncedTempoCode->tempoDataLen = tempoDataLen;
    syncedTempoCode->timeStampFormat = timeStampFormat;

    return syncedTempoCode;
}

void id3v2FreeSyncedTempoCodesFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2SyncedTempoCodesBody *body = (Id3v2SyncedTempoCodesBody *)toDelete->frame;

    if(body->tempoData != NULL){
        free(body->tempoData);
    }

    free(body);
    free(toDelete);
}

/*
    unsynced lyrics frame functions
*/

Id3v2Frame *id3v2ParseUnsynchronizedLyricsFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2UnsynchronizedLyricsBody *newUnsynchronizedLyricsBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    if(newHeader->idNum != ULT && newHeader->idNum != USLT){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newUnsynchronizedLyricsBody = id3v2ParseUnsynchronizedLyricsBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newUnsynchronizedLyricsBody);
}

Id3v2Frame *id3v2CopyUnsynchronizedLyricsFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2UnsynchronizedLyricsBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyUnsynchronizedLyricsBody(frame->frame);

    return id3v2NewFrame(header, body);

}

Id3v2UnsynchronizedLyricsBody *id3v2CopyUnsynchronizedLyricsBody(Id3v2UnsynchronizedLyricsBody *body){

    if(body == NULL){
        return NULL;
    }

    unsigned char encoding = 0x00;
    unsigned char *language = NULL;
    unsigned char *descriptor = NULL;
    unsigned char *lyrics = NULL;

    encoding = body->encoding;

    if(body->descriptor != NULL){
        descriptor = calloc(sizeof(unsigned char), id3strlen(body->descriptor, encoding) + id3ReaderAllocationAdd(encoding));
        memcpy(descriptor, body->descriptor, id3strlen(body->descriptor, encoding));
    }

    if(language != NULL){
        language = calloc(sizeof(unsigned char), ID3V2_LANGUAGE_LEN + 1);
        memcpy(language, body->language, ID3V2_LANGUAGE_LEN);
    }

    if(body->lyrics != NULL){
        lyrics = calloc(sizeof(unsigned char), id3strlen(body->lyrics, encoding) + id3ReaderAllocationAdd(encoding));
        memcpy(lyrics, body->lyrics, id3strlen(body->lyrics, encoding));
    }

    return id3v2NewUnsynchronizedLyricsBody(encoding, language, descriptor, lyrics);
}

Id3v2UnsynchronizedLyricsBody *id3v2ParseUnsynchronizedLyricsBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }

    unsigned char encoding = 0x00;
    unsigned char *language = NULL;
    unsigned char *descriptor = NULL;
    unsigned char *lyrics = NULL;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    // copy encoding
    encoding = id3ReaderCursor(stream)[0];
    id3ReaderSeek(stream, 1, SEEK_CUR);

    // copy language
    language = calloc(sizeof(unsigned char), ID3V2_LANGUAGE_LEN + 1);
    id3ReaderRead(stream, language, ID3V2_LANGUAGE_LEN);

    // copy descriptor
    if(!(id3ReaderCursor(stream)[0] == 0x00)){
        descriptor = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, encoding) + id3ReaderAllocationAdd(encoding));
        id3ReaderRead(stream, descriptor, id3ReaderReadEncodedSize(stream, encoding));
    }

    // skip spacer
    id3ReaderSeek(stream, id3ReaderAllocationAdd(encoding), SEEK_CUR);

    // copy lyrics
    lyrics = id3ReaderEncodedRemainder(stream, encoding);

    id3FreeReader(stream);
    return id3v2NewUnsynchronizedLyricsBody(encoding, language, descriptor, lyrics);
}

Id3v2UnsynchronizedLyricsBody *id3v2NewUnsynchronizedLyricsBody(unsigned char encoding, unsigned char *language, unsigned char *descriptor, unsigned char *lyrics){

    Id3v2UnsynchronizedLyricsBody *unsynchronizedLyricsBody = malloc(sizeof(Id3v2UnsynchronizedLyricsBody));

    // copy data
    unsynchronizedLyricsBody->encoding = encoding;
    unsynchronizedLyricsBody->language = language;
    unsynchronizedLyricsBody->descriptor = descriptor;
    unsynchronizedLyricsBody->lyrics = lyrics;

    return unsynchronizedLyricsBody;
}

void id3v2FreeUnsynchronizedLyricsFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2UnsynchronizedLyricsBody *body = (Id3v2UnsynchronizedLyricsBody *)toDelete->frame;

    if(body->descriptor != NULL){
        free(body->descriptor);
    }

    if(body->language != NULL){
        free(body->language);
    }

    if(body->lyrics != NULL){
        free(body->lyrics);
    }

    free(body);
    free(toDelete);
}

/*
    Synced lyrics frame functions
*/

Id3v2Frame *id3v2ParseSynchronizedLyricsFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2SynchronizedLyricsBody *newSynchronizedLyricsBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    if(newHeader->idNum != SLT && newHeader->idNum != SYLT){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newSynchronizedLyricsBody = id3v2ParseSynchronizedLyricsBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newSynchronizedLyricsBody);
}

Id3v2Frame *id3v2CopySynchronizedLyricsFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2SynchronizedLyricsBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopySynchronizedLyricsBody(frame->frame);

    return id3v2NewFrame(header, body);

}

Id3v2SynchronizedLyricsBody *id3v2CopySynchronizedLyricsBody(Id3v2SynchronizedLyricsBody *body){

    if(body == NULL){
        return NULL;
    }

    unsigned char encoding = 0x00;
    unsigned char *descriptor = NULL;
    unsigned char *language = NULL;
    unsigned int timeStampFormat = 0;
    unsigned int contentType = 0;
    List *lyrics = NULL;


    encoding = body->encoding;
    timeStampFormat = body->timeStampFormat;
    contentType = body->contentType;

    if(body->descriptor != NULL){
        descriptor = calloc(sizeof(unsigned char), id3strlen(body->descriptor, encoding) + id3ReaderAllocationAdd(encoding));
        memcpy(descriptor, body->descriptor, id3strlen(body->descriptor, encoding));
    }

    if(body->language != NULL){
        language = calloc(sizeof(unsigned char), ID3V2_LANGUAGE_LEN + 1);
        memcpy(language, body->language, ID3V2_LANGUAGE_LEN);
    }

    if(body->lyrics->head != NULL){
        lyrics = copyList(body->lyrics);
    }

    return id3v2NewSynchronizedLyricsBody(encoding, language, timeStampFormat, contentType, descriptor, lyrics);
}

Id3v2SynchronizedLyricsBody *id3v2ParseSynchronizedLyricsBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    unsigned char encoding = 0x00;
    unsigned char *descriptor = NULL;
    unsigned char *language = NULL;
    unsigned int timeStampFormat = 0;
    unsigned int contentType = 0;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);
    List *lyrics = NULL;

    // read encoding
    encoding = buffer[0];
    encoding = id3ReaderCursor(stream)[0];
    id3ReaderSeek(stream, 1, SEEK_CUR);

    // read language
    language = calloc(sizeof(unsigned char), ID3V2_LANGUAGE_LEN + 1);
    id3ReaderRead(stream, language, ID3V2_LANGUAGE_LEN);

    // read time format
    timeStampFormat = id3ReaderCursor(stream)[0];
    id3ReaderSeek(stream, 1, SEEK_CUR);

    // read content type
    contentType = id3ReaderCursor(stream)[0];
    id3ReaderSeek(stream, 1, SEEK_CUR);

    // does a description exist?
    if(!(id3ReaderCursor(stream)[0] == 0x00)){
        descriptor = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, encoding) + id3ReaderAllocationAdd(encoding));
        id3ReaderRead(stream, descriptor, id3ReaderReadEncodedSize(stream, encoding));
    }

    // skip spacer
    id3ReaderSeek(stream, id3ReaderAllocationAdd(encoding), SEEK_CUR);

    // body content
    lyrics = newList(id3v2FreeStampedLyric, id3v2CopyStampedLyric);

    while(id3ReaderGetCh(stream) != EOF){

        unsigned char *text = NULL;
        long timeStamp = 0;
        size_t lyricLen = 0;
        Id3v2StampedLyric *lyric = NULL;

        text = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, encoding) + id3ReaderAllocationAdd(encoding));
        id3ReaderRead(stream, text, id3ReaderReadEncodedSize(stream, encoding));
        lyricLen = id3strlen(text, encoding);

        id3ReaderSeek(stream, id3ReaderAllocationAdd(encoding), SEEK_CUR);

        timeStamp = getBits8(id3ReaderCursor(stream), ID3V2_TIME_STAMP_LEN);
        id3ReaderSeek(stream, ID3V2_TIME_STAMP_LEN, SEEK_CUR);

        lyric = id3v2NewStampedLyric(text, timeStamp, lyricLen);
        listPush(lyrics, lyric);
        
    }

    id3FreeReader(stream);
    return id3v2NewSynchronizedLyricsBody(encoding, language, timeStampFormat, contentType, descriptor, lyrics);
}

Id3v2SynchronizedLyricsBody *id3v2NewSynchronizedLyricsBody(unsigned char encoding, unsigned char *language, unsigned int timeStampFormat, unsigned int contentType, unsigned char *descriptor, List *lyrics){

    Id3v2SynchronizedLyricsBody *synchronizedLyricsBody = malloc(sizeof(Id3v2SynchronizedLyricsBody));

    // copy data
    synchronizedLyricsBody->encoding = encoding;
    synchronizedLyricsBody->language = language;
    synchronizedLyricsBody->timeStampFormat = timeStampFormat;
    synchronizedLyricsBody->contentType = contentType;
    synchronizedLyricsBody->descriptor = descriptor;
    synchronizedLyricsBody->lyrics = lyrics;

    return synchronizedLyricsBody;
}

Id3v2StampedLyric *id3v2NewStampedLyric(unsigned char *text, long timeStamp, size_t lyricLen){

    Id3v2StampedLyric *newLyric = malloc(sizeof(Id3v2StampedLyric));

    newLyric->text = text;
    newLyric->timeStamp = timeStamp;
    newLyric->lyricLen = lyricLen;

    return newLyric;
}

void id3v2FreeSynchronizedLyricsFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2SynchronizedLyricsBody *body = (Id3v2SynchronizedLyricsBody *)toDelete->frame;

    if(body->descriptor != NULL){
        free(body->descriptor);
    }

    if(body->language != NULL){
        free(body->language);
    }

    destroyList(body->lyrics);
    free(body);
    free(toDelete);
}

void *id3v2CopyStampedLyric(void *toCopy){

    if(toCopy == NULL){
        return NULL;
    }

    Id3v2StampedLyric *lyric = (Id3v2StampedLyric *)toCopy;
    unsigned char *text = NULL;
    size_t lyricLen = lyric->lyricLen;
    
    if(lyric->text != NULL){
        //+1 always iso?
        text = calloc(sizeof(unsigned char), lyricLen + 1);
        memcpy(text, lyric->text, lyricLen);
    }
    
    return id3v2NewStampedLyric(text, lyric->timeStamp, lyricLen);
}

void id3v2FreeStampedLyric(void *toDelete){

    if(toDelete == NULL){
        return;
    }

    Id3v2StampedLyric *lyric = (Id3v2StampedLyric *)toDelete;

    if(lyric->text != NULL){
        free(lyric->text);
    }

    free(lyric);
}

/*
    comment frame functions
*/

Id3v2Frame *id3v2ParseCommentFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    if(header == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2CommentBody *newCommentBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    if(newHeader->idNum != COM && newHeader->idNum != COMM){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newCommentBody = id3v2ParseCommentBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newCommentBody);
}

Id3v2Frame *id3v2CopyCommentFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2CommentBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyCommentBody(frame->frame);

    return id3v2NewFrame(header, body);

}

Id3v2CommentBody *id3v2CopyCommentBody(Id3v2CommentBody *body){

    if(body == NULL){
        return NULL;
    }

    unsigned char encoding = 0x00;
    unsigned char *language = NULL;
    unsigned char *description = NULL;
    unsigned char *text = NULL;

    encoding = body->encoding;

    if(body->language != NULL){
        language = calloc(sizeof(unsigned char), ID3V2_LANGUAGE_LEN + 1);
        memcpy(language, body->language, ID3V2_LANGUAGE_LEN);
    }

    if(body->description != NULL){
        description = calloc(sizeof(unsigned char), id3strlen(body->description, encoding) + id3ReaderAllocationAdd(encoding));
        memcpy(description, body->description, id3strlen(body->description, encoding));
    }

    if(body->text != NULL){
        text = calloc(sizeof(unsigned char), id3strlen(body->text, encoding) + id3ReaderAllocationAdd(encoding));
        memcpy(text, body->text, id3strlen(body->text, encoding));
    }

    return id3v2NewCommentBody(encoding, language, description, text);
}

Id3v2CommentBody *id3v2ParseCommentBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }

    unsigned char encoding = 0x00;
    unsigned char *language = NULL;
    unsigned char *description = NULL;
    unsigned char *text = NULL;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    // copy encoding
    encoding = id3ReaderCursor(stream)[0];
    id3ReaderSeek(stream, 1, SEEK_CUR);

    // copy language
    language = calloc(sizeof(unsigned char), ID3V2_LANGUAGE_LEN + 1);
    id3ReaderRead(stream, language, ID3V2_LANGUAGE_LEN);

    if(!(id3ReaderCursor(stream)[0] == 0x00)){
        description = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, encoding) + id3ReaderAllocationAdd(encoding));
        id3ReaderRead(stream, description, id3ReaderReadEncodedSize(stream, encoding));
    }

    id3ReaderSeek(stream, id3ReaderAllocationAdd(encoding), SEEK_CUR);

    // copy text
    text = id3ReaderEncodedRemainder(stream, encoding);

    id3FreeReader(stream);
    return id3v2NewCommentBody(encoding, language, description, text);
}

Id3v2CommentBody *id3v2NewCommentBody(unsigned char encoding, unsigned char *language, unsigned char *description, unsigned char *text){

    Id3v2CommentBody *commentBody = malloc(sizeof(Id3v2CommentBody));

    // copy data
    commentBody->description = description;
    commentBody->encoding = encoding;
    commentBody->language = language;
    commentBody->text = text;

    return commentBody;
}

void id3v2FreeCommentFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2CommentBody *body = (Id3v2CommentBody *)toDelete->frame;

    if(body->description != NULL){
        free(body->description);
    }

    if(body->language != NULL){
        free(body->language);
    }

    if(body->text != NULL){
        free(body->text);
    }

    free(body);
    free(toDelete);
}

/*
    Subjective frame functions
*/

Id3v2Frame *id3v2ParseSubjectiveFrame(unsigned char *buffer, Id3v2Header *header){
    
    if(buffer == NULL){
        return NULL;
    }

    if(header == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2SubjectiveBody *newSubjectiveBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newSubjectiveBody = id3v2ParseSubjectiveBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newSubjectiveBody);    
}

Id3v2Frame *id3v2CopySubjectiveFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2SubjectiveBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopySubjectiveBody(frame->frame);

    return id3v2NewFrame(header, body);
}

Id3v2SubjectiveBody *id3v2CopySubjectiveBody(Id3v2SubjectiveBody *body){
     
    if(body == NULL){
        return NULL;
    }

    unsigned char *value = NULL;
    int valueSize = 0;

    valueSize = body->valueSize;

    if(body->value != NULL){
        value = calloc(sizeof(unsigned char), body->valueSize + 1);
        memcpy(value, body->value, valueSize);
    }

    return id3v2NewSubjectiveBody(value, valueSize);
}

Id3v2SubjectiveBody *id3v2ParseSubjectiveBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }

    unsigned char *value = NULL;
    int valueSize = 0;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    //just read like ascii
    valueSize = stream->bufferSize;
    value = id3ReaderEncodedRemainder(stream, ISO_8859_1);

    return id3v2NewSubjectiveBody(value, valueSize);
}

Id3v2SubjectiveBody *id3v2NewSubjectiveBody(unsigned char *value, int valueSize){

    Id3v2SubjectiveBody *body = malloc(sizeof(Id3v2SubjectiveBody));
    
    body->value = value;
    body->valueSize = valueSize;
    return body;
}

void id3v2FreeSubjectiveFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2SubjectiveBody *body = (Id3v2SubjectiveBody *)toDelete->frame;

    if(body->value != NULL){
        free(body->value);
    }

    free(body);
    free(toDelete);
}

/*
    Relative volume adjustment frame functions
*/

Id3v2Frame *id3v2ParseRelativeVolumeAdjustmentFrame(unsigned char *buffer, Id3v2Header *header){
    return id3v2ParseSubjectiveFrame(buffer, header);
}

Id3v2Frame *id3v2CopyRelativeVolumeAdjustmentFrame(Id3v2Frame *frame){
    return id3v2CopySubjectiveFrame(frame);
}

Id3v2RelativeVolumeAdjustmentBody *id3v2CopyRelativeVolumeAdjustmentBody(Id3v2SubjectiveBody *body){
    return (Id3v2RelativeVolumeAdjustmentBody *) id3v2CopySubjectiveBody(body);
}

Id3v2RelativeVolumeAdjustmentBody *id3v2ParseRelativeVolumeAdjustmentBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){
    return (Id3v2RelativeVolumeAdjustmentBody *) id3v2ParseSubjectiveBody(buffer, frameHeader);
}

Id3v2RelativeVolumeAdjustmentBody *id3v2NewRelativeVolumeAdjustmentBody(unsigned char *value, int valueSize){
    return (Id3v2RelativeVolumeAdjustmentBody *) id3v2NewSubjectiveBody(value, valueSize);
}

void id3v2FreeRelativeVolumeAdjustmentFrame(Id3v2Frame *toDelete){
    id3v2FreeSubjectiveFrame(toDelete);
}

/*
    Equalization frame functions
*/

Id3v2Frame *id3v2ParseEqualisationFrame(unsigned char *buffer, Id3v2Header *header){
    return id3v2ParseSubjectiveFrame(buffer, header);
}

Id3v2Frame *id3v2CopyEqualisationFrame(Id3v2Frame *frame){
    return id3v2CopySubjectiveFrame(frame);
}

Id3v2EqualisationBody *id3v2CopyEqualisationBody(Id3v2EqualisationBody *body){
    return (Id3v2EqualisationBody *) id3v2CopySubjectiveBody(body);
}

Id3v2EqualisationBody *id3v2ParseEqualisationBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){
    return (Id3v2EqualisationBody *) id3v2ParseSubjectiveBody(buffer, frameHeader);
}

Id3v2EqualisationBody *id3v2NewEqualisationBody(unsigned char *value, int valueSize){
    return (Id3v2EqualisationBody *) id3v2NewSubjectiveBody(value, valueSize);
}

void id3v2FreeEqualisationFrame(Id3v2Frame *toDelete){
    id3v2FreeSubjectiveFrame(toDelete);
}

/*
    Reverb frame functions
*/

Id3v2Frame *id3v2ParseReverbFrame(unsigned char *buffer, Id3v2Header *header){
    return id3v2ParseSubjectiveFrame(buffer, header);
}

Id3v2Frame *id3v2CopyReverbFrame(Id3v2Frame *frame){
    return id3v2CopySubjectiveFrame(frame);
}

Id3v2ReverbBody *id3v2CopyReverbBody(Id3v2ReverbBody *body){
    return (Id3v2ReverbBody *) id3v2CopySubjectiveBody(body);
}

Id3v2ReverbBody *id3v2ParseReverbBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){
    return (Id3v2ReverbBody *) id3v2ParseSubjectiveBody(buffer, frameHeader);
}

Id3v2ReverbBody *id3v2NewReverbBody(unsigned char *value, int valueSize){
    return (Id3v2ReverbBody *) id3v2NewSubjectiveBody(value, valueSize);
}

void id3v2FreeReverbFrame(Id3v2Frame *toDelete){
    id3v2FreeSubjectiveFrame(toDelete);
}

/*
    Picture frame functions
*/

Id3v2Frame *id3v2ParsePictureFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    if(header == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2PictureBody *newPictureBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    switch(newHeader->idNum){
        case PIC:
            break;
        case APIC:
            break;
        default:
            id3v2FreeFrameHeader(newHeader);
            return NULL;            
    }

    if(newHeader->idNum != PIC && newHeader->idNum != APIC){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newPictureBody = id3v2ParsePictureBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newPictureBody);
}

Id3v2Frame *id3v2CopyPictureFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2PictureBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyPictureBody(frame->frame);

    return id3v2NewFrame(header, body);
}

Id3v2PictureBody *id3v2CopyPictureBody(Id3v2PictureBody *body){

    if(body == NULL){
        return NULL;
    }

    unsigned char encoding = 0;
    unsigned char *format = NULL;
    unsigned char pictureType = 0x00;
    unsigned char *description = NULL;
    unsigned char *pictureData = NULL;
    int picSize = 0;

    encoding = body->encoding;
    pictureType = body->pictureType;
    picSize = body->picSize;

    if(body->format != NULL){
        format = calloc(sizeof(unsigned char), strlen((char *)body->format) + 1);
        memcpy(format, body->format, strlen((char *)body->format));
    }

    if(body->description != NULL){
        description = calloc(sizeof(unsigned char), id3strlen(body->description, encoding) + id3ReaderAllocationAdd(encoding));
        memcpy(description, body->description, id3strlen(body->description, encoding));
    }

    if(body->pictureData != NULL){
        pictureData = calloc(sizeof(unsigned char), body->picSize + 1);
        memcpy(pictureData, body->pictureData, body->picSize);
    }

    return id3v2NewPictureBody(encoding, format, pictureType, description, pictureData, picSize);
}

Id3v2PictureBody *id3v2ParsePictureBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){
    
    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }

    unsigned char encoding = 0;
    unsigned char *format = NULL;
    unsigned char pictureType = 0x00;
    unsigned char *description = NULL;
    unsigned char *pictureData = NULL;
    int picSize = 0;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    // copy encoding
    encoding = id3ReaderCursor(stream)[0];
    id3ReaderSeek(stream, 1, SEEK_CUR);
    // copy format
    // 2.2
    if(frameHeader->idNum == PIC){
        format = calloc(sizeof(unsigned char), ID3V2_PICTURE_FORMAT_LEN + 1);
        id3ReaderRead(stream, format, ID3V2_PICTURE_FORMAT_LEN);
    
    }
    // 2.3 or 4
    if(frameHeader->idNum == APIC){
        //encoding is fixed at ISO_8859_1
        format = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, ISO_8859_1) + 1);
        id3ReaderRead(stream, format, id3ReaderReadEncodedSize(stream, ISO_8859_1));
        
        //skip spacer
        id3ReaderSeek(stream, 1, SEEK_CUR);

    }

    //copy picture type
    pictureType = id3ReaderCursor(stream)[0];
    id3ReaderSeek(stream, 1, SEEK_CUR);


    //copy description
    if(id3ReaderCursor(stream)[0] != 0x00){
        description = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, encoding) + id3ReaderAllocationAdd(encoding));
        id3ReaderRead(stream, description, id3ReaderReadEncodedSize(stream, encoding));
    }

    //skip spacer
    id3ReaderSeek(stream, id3ReaderAllocationAdd(encoding), SEEK_CUR);

    //read image
    picSize = stream->bufferSize - stream->cursor;
    pictureData = id3ReaderEncodedRemainder(stream, ISO_8859_1);

    id3FreeReader(stream);

    return id3v2NewPictureBody(encoding, format, pictureType, description, pictureData, picSize);
}

Id3v2PictureBody *id3v2NewPictureBody(unsigned char encoding, unsigned char *format, unsigned char pictureType, unsigned char *description, unsigned char *pictureData, int picSize){

    Id3v2PictureBody *pictureBody = malloc(sizeof(Id3v2PictureBody));

    // copy data
    pictureBody->encoding = encoding;
    pictureBody->format = format;
    pictureBody->pictureType = pictureType;
    pictureBody->description = description;
    pictureBody->pictureData = pictureData;
    pictureBody->picSize = picSize;

    return pictureBody;
}

void id3v2FreePictureFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2PictureBody *body = (Id3v2PictureBody *)toDelete->frame;

    if(body->description != NULL){
        free(body->description);
    }

    if(body->format != NULL){
        free(body->format);
    }

    if(body->pictureData != NULL){
        free(body->pictureData);
    }

    free(body);
    free(toDelete);
}

/*
    General encapsulated object frame functions
*/

Id3v2Frame *id3v2ParseGeneralEncapsulatedObjectFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    if(header == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2GeneralEncapsulatedObjectBody *newGeneralEncapsulatedObjectBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    switch(newHeader->idNum){
        case GEO:
            break;
        case GEOB:
            break;
        default:
            id3v2FreeFrameHeader(newHeader);
            return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newGeneralEncapsulatedObjectBody = id3v2ParseGeneralEncapsulatedObjectBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newGeneralEncapsulatedObjectBody);
}

Id3v2Frame *id3v2CopyGeneralEncapsulatedObjectFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2GeneralEncapsulatedObjectBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyGeneralEncapsulatedObjectBody(frame->frame);

    return id3v2NewFrame(header, body);
}

Id3v2GeneralEncapsulatedObjectBody *id3v2CopyGeneralEncapsulatedObjectBody(Id3v2GeneralEncapsulatedObjectBody *body){

    if(body == NULL){
        return NULL;
    }

    unsigned char encoding = 0x00;
    unsigned char *mimeType = NULL;
    unsigned char *filename = NULL;
    unsigned char *contentDescription = NULL;
    unsigned char *encapsulatedObject = NULL;
    unsigned int encapsulatedObjectLen = 0;

    encoding = body->encoding;
    encapsulatedObjectLen = encapsulatedObjectLen;


    if(body->mimeType != NULL){
        mimeType = calloc(sizeof(unsigned char), strlen((char *)body->mimeType) + 1);
        memcpy(mimeType, body->mimeType, strlen((char *)body->mimeType));
    }

    if(body->filename != NULL){
        filename = calloc(sizeof(unsigned char), id3strlen(body->filename, encoding) + id3ReaderAllocationAdd(encoding));
        memcpy(filename, body->filename, id3strlen(body->filename, encoding));
    }

    if(body->contentDescription != NULL){
        contentDescription = calloc(sizeof(unsigned char), id3strlen(body->contentDescription, encoding) + id3ReaderAllocationAdd(encoding));
        memcpy(contentDescription, body->contentDescription, id3strlen(body->contentDescription, encoding));
    }

    if(body->encapsulatedObject != NULL){
        encapsulatedObject = calloc(sizeof(unsigned char), body->encapsulatedObjectLen + 1);
        memcpy(encapsulatedObject, body->encapsulatedObject, body->encapsulatedObjectLen);
    }

    return id3v2NewGeneralEncapsulatedObjectBody(encoding, mimeType, filename, contentDescription, encapsulatedObject, encapsulatedObjectLen);
}

Id3v2GeneralEncapsulatedObjectBody *id3v2NewGeneralEncapsulatedObjectBody(unsigned char encoding, unsigned char *mimeType, unsigned char *filename, unsigned char *contentDescription, unsigned char *encapsulatedObject, unsigned int encapsulatedObjectLen){

    Id3v2GeneralEncapsulatedObjectBody *generalEncapsulatedObjectBody = malloc(sizeof(Id3v2GeneralEncapsulatedObjectBody));

    // copy data
    generalEncapsulatedObjectBody->encoding = encoding;
    generalEncapsulatedObjectBody->mimeType = mimeType;
    generalEncapsulatedObjectBody->filename = filename;
    generalEncapsulatedObjectBody->contentDescription = contentDescription;
    generalEncapsulatedObjectBody->encapsulatedObject = encapsulatedObject;
    generalEncapsulatedObjectBody->encapsulatedObjectLen = encapsulatedObjectLen;

    return generalEncapsulatedObjectBody;
}

Id3v2GeneralEncapsulatedObjectBody *id3v2ParseGeneralEncapsulatedObjectBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    unsigned char encoding = 0x00;
    unsigned char *mimeType = NULL;
    unsigned char *filename = NULL;
    unsigned char *contentDescription = NULL;
    unsigned char *encapsulatedObject = NULL;
    unsigned int encapsulatedObjectLen = 0;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    // copy encoding
    encoding = id3ReaderCursor(stream)[0];
    id3ReaderSeek(stream, 1, SEEK_CUR);

    // does mime type exist
    mimeType = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, ISO_8859_1) + 1);
    id3ReaderRead(stream, mimeType, id3ReaderReadEncodedSize(stream, ISO_8859_1));
    id3ReaderSeek(stream,1, SEEK_CUR);

    //get filename
    if(id3ReaderCursor(stream)[0] != 0x00){
        filename = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, encoding) + id3ReaderAllocationAdd(encoding));
        id3ReaderRead(stream, filename, id3ReaderReadEncodedSize(stream, encoding));

    }
    id3ReaderSeek(stream,id3ReaderAllocationAdd(encoding), SEEK_CUR);

    //get content desc
    if(id3ReaderCursor(stream)[0] != 0x00){
        contentDescription = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, encoding) + id3ReaderAllocationAdd(encoding));
        id3ReaderRead(stream, contentDescription, id3ReaderReadEncodedSize(stream, encoding));
    }
    id3ReaderSeek(stream, id3ReaderAllocationAdd(encoding), SEEK_CUR);
    

    // copy encapsulated object
    encapsulatedObjectLen = stream->bufferSize - stream->cursor;
    encapsulatedObject = calloc(sizeof(unsigned char), encapsulatedObjectLen + 1);
    id3ReaderRead(stream, encapsulatedObject, encapsulatedObjectLen);

    id3FreeReader(stream);
    return id3v2NewGeneralEncapsulatedObjectBody(encoding, mimeType, filename, contentDescription, encapsulatedObject, encapsulatedObjectLen);
}

void id3v2FreeGeneralEncapsulatedObjectFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2GeneralEncapsulatedObjectBody *body = (Id3v2GeneralEncapsulatedObjectBody *)toDelete->frame;

    if(body->mimeType != NULL){
        free(body->mimeType);
    }

    if(body->filename != NULL){
        free(body->filename);
    }

    if(body->contentDescription){
        free(body->contentDescription);
    }

    if(body->encapsulatedObject != NULL){
        free(body->encapsulatedObject);
    }

    free(body);
    free(toDelete);
}

/*
    play counter frame functions
*/

Id3v2Frame *id3v2ParsePlayCounterFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    if(header == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2PlayCounterBody *newPlayCounterBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    switch(newHeader->idNum){
        case CNT:
            break;
        case PCNT:
            break;
        default:
            id3v2FreeFrameHeader(newHeader);
            return NULL;            
    }

    buffer = buffer + newHeader->headerSize;
    newPlayCounterBody = id3v2ParsePlayCounterBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newPlayCounterBody);
}

Id3v2Frame *id3v2CopyPlayCounterFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2PlayCounterBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyPlayCounterBody(frame->frame);

    return id3v2NewFrame(header, body);
}

Id3v2PlayCounterBody *id3v2CopyPlayCounterBody(Id3v2PlayCounterBody *body){

    if(body == NULL){
        return NULL;
    }

    unsigned char *counter = NULL;

    if(body->counter != NULL){
        counter = calloc(sizeof(unsigned char), strlen((char *)body->counter) + 1);
        memcpy(counter, body->counter, strlen((char *)body->counter));
    }

    return id3v2NewPlayCounterBody(counter);
}

Id3v2PlayCounterBody *id3v2ParsePlayCounterBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    unsigned char *counter = NULL;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    // copy counter
    counter = id3ReaderEncodedRemainder(stream, ISO_8859_1);
    
    id3FreeReader(stream);
    return id3v2NewPlayCounterBody(counter);
}

Id3v2PlayCounterBody *id3v2NewPlayCounterBody(unsigned char *counter){

    Id3v2PlayCounterBody *body = malloc(sizeof(Id3v2PlayCounterBody));

    body->counter = counter;

    return body;
}

void id3v2FreePlayCounterFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2PlayCounterBody *body = (Id3v2PlayCounterBody *)toDelete->frame;

    if(body->counter != NULL){
        free(body->counter);
    }

    free(body);
    free(toDelete);
}

/*
    popularmeter frame functions
*/

Id3v2Frame *id3v2ParsePopularFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    if(header == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2PopularBody *newPopularBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    switch(newHeader->idNum){
        case POP:
            break;
        case POPM:
            break;
        default:
            id3v2FreeFrameHeader(newHeader);
            return NULL;  
    }

    buffer = buffer + newHeader->headerSize;
    newPopularBody = id3v2ParsePopularBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newPopularBody);
}

Id3v2Frame *id3v2CopyPopularFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2PopularBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyPopularBody(frame->frame);

    return id3v2NewFrame(header, body);
}

Id3v2PopularBody *id3v2CopyPopularBody(Id3v2PopularBody *body){

    if(body == NULL){
        return NULL;
    }

    unsigned char *email = NULL;
    long counter = 0;
    unsigned int rating = 0;

    rating = body->rating;
    counter = body->counter;

    if(body->email != NULL){
        email = calloc(sizeof(unsigned char), strlen((char *)body->email) + 1);
        memcpy(email, body->email, strlen((char *)body->email));
    }

    return id3v2NewPopularBody(email, rating, counter);
}

Id3v2PopularBody *id3v2NewPopularBody(unsigned char *email, unsigned int rating, long counter){

    Id3v2PopularBody *popularBody = malloc(sizeof(Id3v2PopularBody));

    // copy data
    popularBody->email = email;
    popularBody->rating = rating;
    popularBody->counter = counter;

    return popularBody;
}

Id3v2PopularBody *id3v2ParsePopularBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }

    unsigned char *email = NULL;
    long counter = 0;
    unsigned int rating = 0;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    // check for an email
    if(id3ReaderCursor(stream)[0] != 0x00){
        email = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, ISO_8859_1) + 1);
        id3ReaderRead(stream, email, id3ReaderReadEncodedSize(stream, ISO_8859_1));
    }
    id3ReaderSeek(stream, 1, SEEK_CUR);
    
    //copy rating
    rating = id3ReaderCursor(stream)[0];
    id3ReaderSeek(stream, 1, SEEK_CUR);
    
    //counter copy
    counter = getBits8(id3ReaderCursor(stream), stream->bufferSize - stream->cursor);

    id3FreeReader(stream);
    return id3v2NewPopularBody(email, rating, counter);
}

void id3v2FreePopularFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2PopularBody *body = (Id3v2PopularBody *)toDelete->frame;

    if(body->email != NULL){
        free(body->email);
    }

    free(body);
    free(toDelete);
}

/*
    encrypted meta frame
*/

Id3v2Frame *id3v2ParseEncryptedMetaFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    if(header == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2EncryptedMetaBody *newEncryptedMetaBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    if(newHeader->idNum != CRM){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newEncryptedMetaBody = id3v2ParseEncryptedMetaBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newEncryptedMetaBody);
}

Id3v2Frame *id3v2CopyEncryptedMetaFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2EncryptedMetaBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyEncryptedMetaBody(frame->frame);

    return id3v2NewFrame(header, body);
}

Id3v2EncryptedMetaBody *id3v2CopyEncryptedMetaBody(Id3v2EncryptedMetaBody *body){

    if(body == NULL){
        return NULL;
    }

    unsigned char *ownerIdentifier = NULL;
    unsigned char *content = NULL;
    unsigned char *encryptedDatablock = NULL;
    unsigned int encryptedDatablockLen = 0;

    encryptedDatablockLen = body->encryptedDatablockLen;

    if(body->ownerIdentifier != NULL){
        ownerIdentifier = calloc(sizeof(unsigned char), strlen((char *)body->ownerIdentifier) + 1);
        memcpy(ownerIdentifier, body->ownerIdentifier, strlen((char *)body->ownerIdentifier));
    }

    if(body->content != NULL){
        content = calloc(sizeof(unsigned char), strlen((char *)body->content) + 1);
        memcpy(content, body->content, strlen((char *)body->content));
    }

    if(body->encryptedDatablock != NULL){
        encryptedDatablock = calloc(sizeof(unsigned char), body->encryptedDatablockLen + 1);
        memcpy(encryptedDatablock, body->encryptedDatablock, body->encryptedDatablockLen);    
    }

    return id3v2NewEncryptedMetaBody(ownerIdentifier, content, encryptedDatablock, encryptedDatablockLen);
    
}

Id3v2EncryptedMetaBody *id3v2NewEncryptedMetaBody(unsigned char *ownerIdentifier, unsigned char *content, unsigned char *encryptedDatablock, unsigned int encryptedDatablockLen){

    Id3v2EncryptedMetaBody *encryptedMetaBody = malloc(sizeof(Id3v2EncryptedMetaBody));

    // copy data
    encryptedMetaBody->ownerIdentifier = ownerIdentifier;
    encryptedMetaBody->content = content;
    encryptedMetaBody->encryptedDatablock = encryptedDatablock;
    encryptedMetaBody->encryptedDatablockLen = encryptedDatablockLen;

    return encryptedMetaBody;
}

Id3v2EncryptedMetaBody *id3v2ParseEncryptedMetaBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }

    unsigned char *ownerIdentifier = NULL;
    unsigned char *content = NULL;
    unsigned char *encryptedDatablock = NULL;
    unsigned int encryptedDatablockLen = 0;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);
    
    // get owner identifier
    if(id3ReaderCursor(stream)[0] != 0x00){
        ownerIdentifier = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, ISO_8859_1) + 1);
        id3ReaderRead(stream, ownerIdentifier, id3ReaderReadEncodedSize(stream, ISO_8859_1));
    }
    id3ReaderSeek(stream, 1, SEEK_CUR);
    
    // get content/explination
    if(id3ReaderCursor(stream)[0] != 0x00){
        content = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, ISO_8859_1) + 1);
        id3ReaderRead(stream, content, id3ReaderReadEncodedSize(stream, ISO_8859_1));
    }
    id3ReaderSeek(stream, 1, SEEK_CUR);

    // copy encryption data block
    encryptedDatablockLen = stream->bufferSize - stream->cursor;
    encryptedDatablock = id3ReaderEncodedRemainder(stream, ISO_8859_1);
    
    id3FreeReader(stream);
    return id3v2NewEncryptedMetaBody(ownerIdentifier, content, encryptedDatablock, encryptedDatablockLen);
}

void id3v2FreeEncryptedMetaFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2EncryptedMetaBody *body = (Id3v2EncryptedMetaBody *)toDelete->frame;

    if(body->content != NULL){
        free(body->content);
    }

    if(body->encryptedDatablock != NULL){
        free(body->encryptedDatablock);
    }

    if(body->ownerIdentifier != NULL){
        free(body->ownerIdentifier);
    }

    free(body);
    free(toDelete);
}

/*
    audio encryption frame functions
*/

Id3v2Frame *id3v2ParseAudioEncryptionFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    if(header == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2AudioEncryptionBody *newAudioEncryptionBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    switch(newHeader->idNum){
        case CRA:
            break;
        case AENC:
            break;
        default:
            id3v2FreeFrameHeader(newHeader);
            return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newAudioEncryptionBody = id3v2ParseAudioEncryptionBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newAudioEncryptionBody);
}

Id3v2Frame *id3v2CopyAudioEncryptionFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2AudioEncryptionBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyAudioEncryptionBody(frame->frame);

    return id3v2NewFrame(header, body);
}

Id3v2AudioEncryptionBody *id3v2CopyAudioEncryptionBody(Id3v2AudioEncryptionBody *body){

    if(body == NULL){
        return NULL;
    }

    unsigned char *ownerIdentifier = NULL;
    void *previewStart = NULL;
    unsigned int previewLength = 0;
    unsigned char *encryptionInfo = NULL;
    unsigned int encryptionInfoLen = 0;

    previewStart = body->previewStart;
    previewLength = body->previewLength;
    encryptionInfoLen = body->encryptionInfoLen;

    if(body->ownerIdentifier != NULL){
        ownerIdentifier = calloc(sizeof(unsigned char), strlen((char *)body->ownerIdentifier) + 1);
        memcpy(ownerIdentifier, body->ownerIdentifier, strlen((char *)body->ownerIdentifier));
    }

    if(body->encryptionInfo != NULL){
        encryptionInfo = calloc(sizeof(unsigned char), body->encryptionInfoLen + 1);
        memcpy(encryptionInfo, body->encryptionInfo, body->encryptionInfoLen);
    }

    return id3v2NewAudioEncryptionBody(ownerIdentifier, previewStart, previewLength, encryptionInfo, encryptionInfoLen);
}

Id3v2AudioEncryptionBody *id3v2ParseAudioEncryptionBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }

    unsigned char *ownerIdentifier = NULL;
    void *previewStart = NULL;
    unsigned int previewLength = 0;
    unsigned char *encryptionInfo = NULL;
    unsigned int encryptionInfoLen = 0;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    //copy owner identifier
    if(id3ReaderCursor(stream)[0] != 0x00){
        ownerIdentifier = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream,ISO_8859_1) + 1);
        id3ReaderRead(stream, ownerIdentifier, id3ReaderReadEncodedSize(stream,ISO_8859_1));
    }
    id3ReaderSeek(stream, 1, SEEK_CUR);

    // copy preview start pointer
    memcpy(&previewStart, id3ReaderCursor(stream), 2);
    id3ReaderSeek(stream, 2, SEEK_CUR);

    // copy preview length
    previewLength = getBits8(id3ReaderCursor(stream), 2);
    id3ReaderSeek(stream, 2, SEEK_CUR);
    

    // copy encryption info
    encryptionInfoLen = stream->bufferSize - stream->cursor;
    encryptionInfo = id3ReaderEncodedRemainder(stream, ISO_8859_1);

    id3FreeReader(stream);
    return id3v2NewAudioEncryptionBody(ownerIdentifier, previewStart, previewLength, encryptionInfo, encryptionInfoLen);
}

Id3v2AudioEncryptionBody *id3v2NewAudioEncryptionBody(unsigned char *ownerIdentifier, void *previewStart, unsigned int previewLength, unsigned char *encryptionInfo, unsigned int encryptionInfoLen){

    Id3v2AudioEncryptionBody *audioEncryptionBody = malloc(sizeof(Id3v2AudioEncryptionBody));

    // copy data
    audioEncryptionBody->ownerIdentifier = ownerIdentifier;
    audioEncryptionBody->previewStart = previewStart;
    audioEncryptionBody->previewLength = previewLength;
    audioEncryptionBody->encryptionInfo = encryptionInfo;
    audioEncryptionBody->encryptionInfoLen = encryptionInfoLen;

    return audioEncryptionBody;
}

void id3v2FreeAudioEncryptionFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2AudioEncryptionBody *body = (Id3v2AudioEncryptionBody *)toDelete->frame;

    if(body->encryptionInfo != NULL){
        free(body->encryptionInfo);
    }

    if(body->ownerIdentifier != NULL){
        free(body->ownerIdentifier);
    }

    free(body);
    free(toDelete);
}

/*
    Unique file identifier frame functions
*/

Id3v2Frame *id3v2ParseUniqueFileIdentiferFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    if(header == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2UniqueFileIdentifierBody *newUniqueFileIdentiferBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    switch(newHeader->idNum){
        case UFI:
            break;
        case UFID:
            break;
        default:
            id3v2FreeFrameHeader(newHeader);
            return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newUniqueFileIdentiferBody = id3v2ParseUniqueFileIdentiferBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newUniqueFileIdentiferBody);
}

Id3v2Frame *id3v2CopyUniqueFileIdentifierFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2UniqueFileIdentifierBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyUniqueFileIdentifierBody(frame->frame);

    return id3v2NewFrame(header, body);
}

Id3v2UniqueFileIdentifierBody *id3v2CopyUniqueFileIdentifierBody(Id3v2UniqueFileIdentifierBody *body){

    if(body == NULL){
        return NULL;
    }

    unsigned char *ownerIdentifier = NULL;
    unsigned char *identifier = NULL;

    if(body->ownerIdentifier != NULL){
        ownerIdentifier = calloc(sizeof(unsigned char), strlen((char *)body->ownerIdentifier) + 1);
        memcpy(ownerIdentifier, body->ownerIdentifier, strlen((char *)body->ownerIdentifier));
    }

    if(body->identifier != NULL){
        identifier = calloc(sizeof(unsigned char), strlen((char *)body->identifier) + 1);
        memcpy(identifier, body->identifier, strlen((char *)body->identifier));
    }

    return id3v2NewUniqueFileIdentifierBody(ownerIdentifier, identifier);
}

Id3v2UniqueFileIdentifierBody *id3v2ParseUniqueFileIdentiferBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){
    
    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }

    unsigned char *ownerIdentifier = NULL;
    unsigned char *identifier = NULL;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    // does owner id exist
    if(id3ReaderCursor(stream)[0] != 0x00){
        ownerIdentifier = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, ISO_8859_1) + 1);
        id3ReaderRead(stream, ownerIdentifier, id3ReaderReadEncodedSize(stream, ISO_8859_1));
    }
    id3ReaderSeek(stream, 1, SEEK_CUR);

    // copy identifier
    identifier = id3ReaderEncodedRemainder(stream, ISO_8859_1);

    id3FreeReader(stream);
    return id3v2NewUniqueFileIdentifierBody(ownerIdentifier, identifier);
}

Id3v2UniqueFileIdentifierBody *id3v2NewUniqueFileIdentifierBody(unsigned char *ownerIdentifier, unsigned char *identifier){

    Id3v2UniqueFileIdentifierBody *uniqueFileIdentifierBody = malloc(sizeof(Id3v2UniqueFileIdentifierBody));

    // copy data
    uniqueFileIdentifierBody->identifier = identifier;
    uniqueFileIdentifierBody->ownerIdentifier = ownerIdentifier;

    return uniqueFileIdentifierBody;
}

void id3v2FreeUniqueFileIdentifierFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2UniqueFileIdentifierBody *body = (Id3v2UniqueFileIdentifierBody *)toDelete->frame;

    if(body->identifier != NULL){
        free(body->identifier);
    }

    if(body->ownerIdentifier != NULL){
        free(body->ownerIdentifier);
    }

    free(body);
    free(toDelete);
}

/*
    Position synchronisation frame
*/

Id3v2Frame *id3v2ParsePositionSynchronisationFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    if(header == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2PositionSynchronisationBody *newPositionSynchronisationBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    if(newHeader->idNum != POSS){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newPositionSynchronisationBody = id3v2ParsePositionSynchronisationBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newPositionSynchronisationBody);
}

Id3v2Frame *id3v2CopyPositionSynchronisationFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2PositionSynchronisationBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyPositionSynchronisationBody(frame->frame);

    return id3v2NewFrame(header, body);   
}

Id3v2PositionSynchronisationBody *id3v2CopyPositionSynchronisationBody(Id3v2PositionSynchronisationBody *body){
    return (body == NULL) ? NULL : id3v2NewPositionSynchronisationBody(body->timeStampFormat, body->pos);
}

Id3v2PositionSynchronisationBody *id3v2ParsePositionSynchronisationBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }

    unsigned char timeStampFormat = 0x00;
    long pos = 0;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);
    timeStampFormat = id3ReaderCursor(stream)[0];
    id3ReaderSeek(stream, 1, SEEK_CUR);

    // gets a time stamp
    pos = getBits8(id3ReaderCursor(stream), stream->bufferSize - stream->cursor);

    id3FreeReader(stream);
    return id3v2NewPositionSynchronisationBody(timeStampFormat, pos);
}

Id3v2PositionSynchronisationBody *id3v2NewPositionSynchronisationBody(unsigned char timeStampFormat, long pos){

    Id3v2PositionSynchronisationBody *body = malloc(sizeof(Id3v2PositionSynchronisationBody));

    body->pos = pos;
    body->timeStampFormat = timeStampFormat;

    return body;
}

void id3v2FreePositionSynchronisationFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->frame != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame != NULL){
        free(toDelete->frame);
    }

    free(toDelete);
}

/*
    terms of service frame functions
*/

Id3v2Frame *id3v2ParseTermsOfUseFrame(unsigned char *buffer, Id3v2Header *header){
    
    if(buffer == NULL){
        return NULL;
    }

    if(header == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2TermsOfUseBody *newTermsOfUseBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    if(newHeader->idNum != USER){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newTermsOfUseBody = id3v2ParseTermsOfUseBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newTermsOfUseBody);
}

Id3v2Frame *id3v2CopyTermsOfUseFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2TermsOfUseBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyTermsOfUseBody(frame->frame);

    return id3v2NewFrame(header, body);   

}

Id3v2TermsOfUseBody *id3v2CopyTermsOfUseBody(Id3v2TermsOfUseBody *body){

    if(body == NULL){
        return NULL;
    }

    unsigned char encoding = 0x00;
    unsigned char *language = NULL;
    unsigned char *text = NULL;

    encoding = body->encoding;

    if(body->language != NULL){
        language = calloc(sizeof(unsigned char), ID3V2_LANGUAGE_LEN + 1);
        memcpy(language, body->language, ID3V2_LANGUAGE_LEN);
    }

    if(body->text != NULL){
        text = calloc(sizeof(unsigned char), id3strlen(body->text, encoding));
        memcpy(text, body->text, id3strlen(body->text, encoding));
    }

    return id3v2NewTermsOfUseBody(encoding, language, text);
}

Id3v2TermsOfUseBody *id3v2ParseTermsOfUseBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }

    unsigned char encoding = 0x00;
    unsigned char *language = NULL;
    unsigned char *text = NULL;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    // get encoding
    encoding = id3ReaderCursor(stream)[0];
    id3ReaderSeek(stream, 1, SEEK_CUR);

    language = calloc(sizeof(unsigned char), ID3V2_LANGUAGE_LEN + 1);
    id3ReaderRead(stream, language, ID3V2_LANGUAGE_LEN);

    // check to see if text is there
    text = id3ReaderEncodedRemainder(stream, encoding);
    
    id3FreeReader(stream);
    return id3v2NewTermsOfUseBody(encoding, language, text);
}

Id3v2TermsOfUseBody *id3v2NewTermsOfUseBody(unsigned char encoding, unsigned char *language, unsigned char *text){

    Id3v2TermsOfUseBody *body = malloc(sizeof(Id3v2TermsOfUseBody));

    body->encoding = encoding;
    body->language = language;
    body->text = text;

    return body;
}

void id3v2FreeTermsOfUseFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2TermsOfUseBody *body = (Id3v2TermsOfUseBody *)toDelete->frame;

    if(body->language != NULL){
        free(body->language);
    }

    if(body->text != NULL){
        free(body->text);
    }

    free(toDelete->frame);
    free(toDelete);
}

/*
    ownership functions
*/

Id3v2Frame *id3v2ParseOwnershipFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    if(header == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2OwnershipBody *newOwnershipBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    if(newHeader->idNum != OWNE){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newOwnershipBody = id3v2ParseOwnershipBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newOwnershipBody);
}

Id3v2Frame *id3v2CopyOwnershipFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2OwnershipBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyOwnershipBody(frame->frame);

    return id3v2NewFrame(header, body);   

}

Id3v2OwnershipBody *id3v2CopyOwnershipBody(Id3v2OwnershipBody *body){

    if(body == NULL){
        return NULL;
    }

    unsigned char encoding = 0x00;
    unsigned char *pricePayed = NULL;
    unsigned char *dateOfPunch = NULL;
    unsigned char *seller = NULL;

    encoding = body->encoding;

    if(body->pricePayed != NULL){
        pricePayed = calloc(sizeof(unsigned char), strlen((char *)body->pricePayed) + 1);
        memcpy(pricePayed, body->pricePayed, strlen((char *)body->pricePayed));
    }

    if(body->dateOfPunch != NULL){
        dateOfPunch = calloc(sizeof(unsigned char), ID3V2_BASE_DATE_FORMAT_LEN + 1);
        memcpy(dateOfPunch, body->dateOfPunch, ID3V2_BASE_DATE_FORMAT_LEN);
    }

    if(body->seller != NULL){
        seller = calloc(sizeof(unsigned char), id3strlen(body->seller, encoding) + id3ReaderAllocationAdd(encoding));
        memcpy(seller, body->seller, id3strlen(body->seller, encoding));
    }

    return id3v2NewOwnershipBody(encoding, pricePayed, dateOfPunch, seller);
}

Id3v2OwnershipBody *id3v2ParseOwnershipBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }

    unsigned char encoding = 0x00;
    unsigned char *pricePayed = NULL;
    unsigned char *dateOfPunch = NULL;
    unsigned char *seller = NULL;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    // copy encoding
    encoding = id3ReaderCursor(stream)[0];
    id3ReaderSeek(stream, 1, SEEK_CUR);

    // get amount payed for the file
    if(id3ReaderCursor(stream)[0] != 0x00){
        pricePayed = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream,ISO_8859_1) + 1);
        id3ReaderRead(stream, pricePayed, id3ReaderReadEncodedSize(stream, ISO_8859_1));
    }
    id3ReaderSeek(stream, 1, SEEK_CUR);

    // get punched for the file
    dateOfPunch = calloc(sizeof(unsigned char), ID3V2_BASE_DATE_FORMAT_LEN + 1);
    id3ReaderRead(stream, dateOfPunch, ID3V2_BASE_DATE_FORMAT_LEN);

    // check to see if there is a seller
    seller = id3ReaderEncodedRemainder(stream, encoding);

    id3FreeReader(stream);
    return id3v2NewOwnershipBody(encoding, pricePayed, dateOfPunch, seller);
}

Id3v2OwnershipBody *id3v2NewOwnershipBody(unsigned char encoding, unsigned char *pricePayed, unsigned char *dateOfPunch, unsigned char *seller){

    Id3v2OwnershipBody *body = malloc(sizeof(Id3v2OwnershipBody));

    body->encoding = encoding;
    body->pricePayed = pricePayed;
    body->dateOfPunch = dateOfPunch;
    body->seller = seller;

    return body;
}

void id3v2FreeOwnershipFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2OwnershipBody *body = (Id3v2OwnershipBody *)toDelete->frame;

    if(body->pricePayed != NULL){
        free(body->pricePayed);
    }

    if(body->dateOfPunch != NULL){
        free(body->dateOfPunch);
    }

    if(body->seller != NULL){
        free(body->seller);
    }

    free(toDelete->frame);
    free(toDelete);
}

/*
    commercial frame
*/

Id3v2Frame *id3v2ParseCommercialFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    if(header == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2CommercialBody *newCommercialBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    if(newHeader->idNum != COMR){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newCommercialBody = id3v2ParseCommercialBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newCommercialBody);
}

Id3v2Frame *id3v2CopyCommercialFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2CommercialBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyCommercialBody(frame->frame);

    return id3v2NewFrame(header, body);
}

Id3v2CommercialBody *id3v2CopyCommercialBody(Id3v2CommercialBody *body){

    if(body == NULL){
        return NULL;
    }

    unsigned char encoding = 0x00;
    unsigned char *priceString = NULL;
    unsigned char *validUntil = NULL;
    unsigned char *contractURL = NULL;
    unsigned char receivedAs = 0x00;
    unsigned char *nameOfSeller = NULL; //encoded
    unsigned char *description = NULL; //encoded
    unsigned char *mimeType = NULL;
    unsigned char *sellerLogo = NULL;
    unsigned int sellerLogoLen = 0;

    encoding = body->encoding;
    receivedAs = body->receivedAs;
    sellerLogoLen = body->sellerLogoLen;

    if(body->priceString != NULL){
        priceString = calloc(sizeof(unsigned char), strlen((char *)body->priceString) + 1);
        memcpy(priceString, body->priceString, strlen((char *)body->priceString));
    }

    if(body->validUntil != NULL){
        validUntil = calloc(sizeof(unsigned char), ID3V2_BASE_DATE_FORMAT_LEN + 1);
        memcpy(validUntil, body->validUntil, ID3V2_BASE_DATE_FORMAT_LEN);
    }

    if(body->contractURL != NULL){
        contractURL = calloc(sizeof(unsigned char), strlen((char *)body->contractURL) + 1);
        memcpy(contractURL, body->contractURL, strlen((char *)body->contractURL));
    }

    if(body->nameOfSeller != NULL){
        nameOfSeller = calloc(sizeof(unsigned char), id3strlen(body->nameOfSeller, encoding) + id3ReaderAllocationAdd(encoding));
        memcpy(nameOfSeller, body->nameOfSeller, id3strlen(body->nameOfSeller, encoding));
    }

    if(body->description != NULL){
        description = calloc(sizeof(unsigned char), id3strlen(body->description, encoding) + id3ReaderAllocationAdd(encoding));
        memcpy(description, body->description, id3strlen(body->description, encoding));
    }

    if(body->mimeType != NULL){
        mimeType = calloc(sizeof(unsigned char), strlen((char *)body->mimeType) + 1);
        memcpy(mimeType, body->mimeType, strlen((char *)body->mimeType));        
    }

    if(body->sellerLogo != NULL){
        sellerLogo = calloc(sizeof(unsigned char), body->sellerLogoLen + 1);
        memcpy(sellerLogo, body->sellerLogo, body->sellerLogoLen);
    }

    return id3v2NewCommercialBody(encoding, priceString, validUntil, contractURL, receivedAs, nameOfSeller, description, mimeType, sellerLogo, sellerLogoLen);
    
}

Id3v2CommercialBody *id3v2ParseCommercialBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }

    unsigned char encoding = 0x00;
    unsigned char *priceString = NULL;
    unsigned char *validUntil = NULL;
    unsigned char *contractURL = NULL;
    unsigned char receivedAs = 0x00;
    unsigned char *nameOfSeller = NULL;
    unsigned char *description = NULL;
    unsigned char *mimeType = NULL;
    unsigned char *sellerLogo = NULL;
    unsigned int sellerLogoLen = 0;

    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    encoding = id3ReaderCursor(stream)[0];
    id3ReaderSeek(stream, 1, SEEK_CUR);

    // get price string for the file
    if(id3ReaderCursor(stream)[0] != 0x00){
        priceString = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, ISO_8859_1) + 1);
        id3ReaderRead(stream, priceString, id3ReaderReadEncodedSize(stream, ISO_8859_1));
    }
    id3ReaderSeek(stream, 1, SEEK_CUR);
    
    // get valid
    validUntil = calloc(sizeof(unsigned char), ID3V2_BASE_DATE_FORMAT_LEN + 1);
    id3ReaderRead(stream, validUntil, ID3V2_BASE_DATE_FORMAT_LEN);

    // get contract URL for the file
    if(id3ReaderCursor(stream)[0] != 0x00){
        contractURL = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, ISO_8859_1) + 1);
        id3ReaderRead(stream, contractURL, id3ReaderReadEncodedSize(stream, ISO_8859_1));
    }
    id3ReaderSeek(stream, 1, SEEK_CUR);

    // what kind is it?
    receivedAs = id3ReaderCursor(stream)[0];
    id3ReaderSeek(stream, 1, SEEK_CUR);

    // read the name of the seller
    if(id3ReaderCursor(stream)[0] != 0x00){
        nameOfSeller = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, encoding) + id3ReaderAllocationAdd(encoding));
        id3ReaderRead(stream, nameOfSeller, id3ReaderReadEncodedSize(stream, encoding));
    }
    id3ReaderSeek(stream, id3ReaderAllocationAdd(encoding), SEEK_CUR);

    // copy description
    if(id3ReaderCursor(stream)[0] != 0x00){
        description = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, encoding) + id3ReaderAllocationAdd(encoding));
        id3ReaderRead(stream, description,  id3ReaderReadEncodedSize(stream, encoding));
    }
    id3ReaderSeek(stream, id3ReaderAllocationAdd(encoding), SEEK_CUR);

    //read file type
    if(id3ReaderCursor(stream)[0] != 0x00){
        mimeType = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, ISO_8859_1) + 1);
        id3ReaderRead(stream, description,  id3ReaderReadEncodedSize(stream, ISO_8859_1));
    }
    id3ReaderSeek(stream, 1, SEEK_CUR);

    //get picture
    sellerLogoLen = stream->bufferSize - stream->cursor;
    sellerLogo = id3ReaderEncodedRemainder(stream, ISO_8859_1);

    id3FreeReader(stream);
    return id3v2NewCommercialBody(encoding, priceString, validUntil, contractURL, receivedAs, nameOfSeller, description, mimeType, sellerLogo, sellerLogoLen);
}

Id3v2CommercialBody *id3v2NewCommercialBody(unsigned char encoding, unsigned char *priceString, unsigned char *validUntil, unsigned char *contractURL, unsigned char receivedAs, unsigned char *nameOfSeller, unsigned char *description, unsigned char *mimeType, unsigned char *sellerLogo, unsigned int sellerLogoLen){

    Id3v2CommercialBody *body = malloc(sizeof(Id3v2CommercialBody));

    body->encoding = encoding;
    body->priceString = priceString;
    body->validUntil = validUntil;
    body->contractURL = contractURL;
    body->receivedAs = receivedAs;
    body->nameOfSeller = nameOfSeller;
    body->description = description;
    body->mimeType = mimeType;
    body->sellerLogo = sellerLogo;
    body->sellerLogoLen = sellerLogoLen;

    return body;
}

void id3v2FreeCommercialFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2CommercialBody *body = (Id3v2CommercialBody *)toDelete->frame;

    if(body->priceString != NULL){
        free(body->priceString);
    }

    if(body->validUntil != NULL){
        free(body->validUntil);
    }

    if(body->contractURL != NULL){
        free(body->contractURL);
    }

    if(body->nameOfSeller != NULL){
        free(body->nameOfSeller);
    }

    if(body->description != NULL){
        free(body->description);
    }

    if(body->mimeType != NULL){
        free(body->mimeType);
    }

    if(body->sellerLogo != NULL){
        free(body->sellerLogo);
    }

    free(toDelete->frame);
    free(toDelete);
}

/*
    encryption method regitration frame
*/

Id3v2Frame *id3v2ParseEncryptionMethodRegistrationFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    if(header == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2EncryptionMethodRegistrationBody *newEncryptionMethodRegistrationBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    if(newHeader->idNum != ENCR){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newEncryptionMethodRegistrationBody = id3v2ParseEncryptionMethodRegistrationBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newEncryptionMethodRegistrationBody);
}

Id3v2Frame *id3v2CopyEncryptionMethodRegistrationFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2EncryptionMethodRegistrationBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyEncryptionMethodRegistrationBody(frame->frame);

    return id3v2NewFrame(header, body);
}

Id3v2EncryptionMethodRegistrationBody *id3v2CopyEncryptionMethodRegistrationBody(Id3v2EncryptionMethodRegistrationBody *body){

    if(body == NULL){
        return NULL;
    }

    unsigned char *ownerIdentifier = NULL;
    unsigned char methodSymbol = body->methodSymbol;
    unsigned char *encryptionData = NULL;
    unsigned int encryptionDataLen = body->encryptionDataLen;

    if(body->ownerIdentifier != NULL){
        ownerIdentifier = calloc(sizeof(unsigned char), strlen((char *)body->ownerIdentifier) + 1);
        memcpy(ownerIdentifier, body->ownerIdentifier, strlen((char *)body->ownerIdentifier));
    }
    
    encryptionData = calloc(sizeof(unsigned char), encryptionDataLen + 1);
    memcpy(encryptionData, body->encryptionData, encryptionDataLen);

    return id3v2NewEncryptionMethodRegistrationBody(ownerIdentifier, methodSymbol, encryptionData, encryptionDataLen);
}

Id3v2EncryptionMethodRegistrationBody *id3v2ParseEncryptionMethodRegistrationBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }

    unsigned char *ownerIdentifier = NULL;
    unsigned char methodSymbol = 0x00;
    unsigned char *encryptionData = NULL;
    unsigned int encryptionDataLen = 0;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    // get owner id for the file
    if(id3ReaderCursor(stream)[0] != 0x00){
        ownerIdentifier = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, ISO_8859_1) + 1);
        id3ReaderRead(stream, ownerIdentifier, id3ReaderReadEncodedSize(stream, ISO_8859_1));
    }
    id3ReaderSeek(stream, 1, SEEK_CUR);

    // get method symbol
    methodSymbol = id3ReaderCursor(stream)[0];
    id3ReaderSeek(stream, 1, SEEK_CUR);

    // get len of data
    encryptionDataLen = stream->bufferSize - stream->cursor;

    // encryption data
    if(encryptionDataLen != 0){
        encryptionData = id3ReaderEncodedRemainder(stream, ISO_8859_1);
    }

    return id3v2NewEncryptionMethodRegistrationBody(ownerIdentifier, methodSymbol, encryptionData, encryptionDataLen);
}

Id3v2EncryptionMethodRegistrationBody *id3v2NewEncryptionMethodRegistrationBody(unsigned char *ownerIdentifier, unsigned char methodSymbol, unsigned char *encryptionData, unsigned int encryptionDataLen){

    Id3v2EncryptionMethodRegistrationBody *body = malloc(sizeof(Id3v2EncryptionMethodRegistrationBody));

    body->ownerIdentifier = ownerIdentifier;
    body->methodSymbol = methodSymbol;
    body->encryptionData = encryptionData;
    body->encryptionDataLen = encryptionDataLen;

    return body;
}

void id3v2FreeEncryptionMethodRegistrationFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2EncryptionMethodRegistrationBody *body = (Id3v2EncryptionMethodRegistrationBody *)toDelete->frame;

    if(body->ownerIdentifier != NULL){
        free(body->ownerIdentifier);
    }

    if(body->encryptionData != NULL){
        free(body->encryptionData);
    }

    free(toDelete->frame);
    free(toDelete);
}

/*
    group id registration frame
*/

Id3v2Frame *id3v2ParseGroupIDRegistrationFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    if(header == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2GroupIDRegistrationBody *newGroupIDRegistrationBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    if(newHeader->idNum != GRID){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newGroupIDRegistrationBody = (Id3v2GroupIDRegistrationBody *)id3v2ParseEncryptionMethodRegistrationBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newGroupIDRegistrationBody);
}

Id3v2Frame *id3v2CopyGroupIDRegistrationFrame(Id3v2Frame *frame){
    
    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2GroupIDRegistrationBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyGroupIDRegistrationBody(frame->frame);

    return id3v2NewFrame(header, body);
}

Id3v2GroupIDRegistrationBody *id3v2CopyGroupIDRegistrationBody(Id3v2GroupIDRegistrationBody *body){
    return (Id3v2GroupIDRegistrationBody *)id3v2CopyEncryptionMethodRegistrationBody((Id3v2EncryptionMethodRegistrationBody *)body);
}

Id3v2GroupIDRegistrationBody *id3v2ParseGroupIDRegistrationBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){
    return (Id3v2GroupIDRegistrationBody *)id3v2ParseEncryptionMethodRegistrationBody(buffer, frameHeader);
}

Id3v2GroupIDRegistrationBody *id3v2NewGroupIDRegistrationBody(unsigned char *ownerIdentifier, unsigned char groupSymbol, unsigned char *groupDependentData, unsigned int groupDependentDataLen){
    return (Id3v2GroupIDRegistrationBody *)id3v2NewEncryptionMethodRegistrationBody(ownerIdentifier, groupSymbol, groupDependentData, groupDependentDataLen);
}

void id3v2FreeGroupIDRegistrationFrame(Id3v2Frame *toDelete){
    id3v2FreeEncryptionMethodRegistrationFrame(toDelete);
}

/*
    private frame
*/

Id3v2Frame *id3v2ParsePrivateFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    if(header == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2PrivateBody *newPrivateBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    if(newHeader->idNum != PRIV){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    newPrivateBody = id3v2ParsePrivateBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newPrivateBody);
}

Id3v2Frame *id3v2CopyPrivateFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2PrivateBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopyPrivateBody(frame->frame);

    return id3v2NewFrame(header, (void *)body);
}

Id3v2PrivateBody *id3v2CopyPrivateBody(Id3v2PrivateBody *body){

    if(body == NULL){
        return NULL;
    }

    unsigned char *ownerIdentifier = NULL;
    unsigned char *privateData = NULL;
    unsigned int privateDataLen = body->privateDataLen;

    if(body->ownerIdentifier != NULL){
        ownerIdentifier = calloc(sizeof(unsigned char), strlen((char *)body->ownerIdentifier) + 1);
        memcpy(ownerIdentifier, body->ownerIdentifier, strlen((char *)body->ownerIdentifier));
    }
    if(body->privateData != NULL){
        privateData = calloc(sizeof(unsigned char), privateDataLen + 1);
        memcpy(privateData, body->privateData, privateDataLen);
    }

    
    return id3v2NewPrivateBody(ownerIdentifier, privateData, privateDataLen);    

}

Id3v2PrivateBody *id3v2ParsePrivateBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }

    unsigned char *ownerIdentifier = NULL;
    unsigned char *privateData = NULL;
    unsigned int privateDataLen = 0;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    //read owner identifier
    if(id3ReaderCursor(stream)[0] != 0x00){
        ownerIdentifier = calloc(sizeof(unsigned char), id3ReaderReadEncodedSize(stream, ISO_8859_1) + 1);
        id3ReaderRead(stream, ownerIdentifier, id3ReaderReadEncodedSize(stream, ISO_8859_1));
    }
    id3ReaderSeek(stream, 1, SEEK_CUR);



    privateDataLen = stream->bufferSize - stream->cursor;

    if(privateDataLen > 0){
        privateData = id3ReaderEncodedRemainder(stream, ISO_8859_1);
    }

    return id3v2NewPrivateBody(ownerIdentifier, privateData, privateDataLen);
}

Id3v2PrivateBody *id3v2NewPrivateBody(unsigned char *ownerIdentifier, unsigned char *privateData, unsigned int privateDataLen){

    Id3v2PrivateBody *body = malloc(sizeof(Id3v2PrivateBody));

    body->ownerIdentifier = ownerIdentifier;
    body->privateData = privateData;
    body->privateDataLen = privateDataLen;

    return body;
}

void id3v2FreePrivateFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2PrivateBody *body = (Id3v2PrivateBody *)toDelete->frame;

    if(body->ownerIdentifier != NULL){
        free(body->ownerIdentifier);
    }

    if(body->privateData != NULL){
        free(body->privateData);
    }

    free(toDelete->frame);
    free(toDelete);
}

/*
    signature frame functions
*/

Id3v2Frame *id3v2ParseSignatureFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    if(header == NULL){
        return NULL;
    }

    int versionOffset = 0;
    
    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2SignatureBody *newSignatureBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0 ){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    if(newHeader->idNum != SIGN){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    
    newSignatureBody = id3v2ParseSignatureBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newSignatureBody);
}

Id3v2Frame *id3v2CopySignatureFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2SignatureBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopySignatureBody(frame->frame);

    return id3v2NewFrame(header, (void *)body);

}

Id3v2SignatureBody *id3v2CopySignatureBody(Id3v2SignatureBody *body){

    if(body == NULL){
        return NULL;
    }

    unsigned char groupSymbol = body->groupSymbol;
    unsigned char *signature = calloc(sizeof(unsigned char), strlen((char *)body->signature) + 1);

    memcpy(signature, body->signature, strlen((char *)body->signature));

    return id3v2NewSignatureBody(groupSymbol, signature);    

}

Id3v2SignatureBody *id3v2ParseSignatureBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }

    unsigned char groupSymbol = 0x00;
    unsigned char *signature = NULL; 
    unsigned int signatureDataLen = 0;
    Id3Reader *stream = id3NewReader(buffer, frameHeader->frameSize);

    groupSymbol = id3ReaderGetCh(stream);
    id3ReaderSeek(stream, 1, SEEK_CUR);

    signatureDataLen = stream->bufferSize - stream->cursor;

    if(signatureDataLen > 0){
        signature = id3ReaderEncodedRemainder(stream, ISO_8859_1);
    }

    id3FreeReader(stream);
    return id3v2NewSignatureBody(groupSymbol, signature);

}

Id3v2SignatureBody *id3v2NewSignatureBody(unsigned char groupSymbol, unsigned char *signature){

    Id3v2SignatureBody *body = malloc(sizeof(Id3v2SignatureBody));

    body->groupSymbol = groupSymbol;
    body->signature = signature;

    return body;
}

void id3v2FreeSignatureFrame(Id3v2Frame *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    Id3v2SignatureBody *body = (Id3v2SignatureBody *)toDelete->frame;

    if(body->signature != NULL){
        free(body->signature);
    }

    free(toDelete->frame);
    free(toDelete);

}

/*
    seek frame functions
*/

Id3v2Frame *id3v2ParseSeekFrame(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    if(header == NULL){
        return NULL;
    }

    int versionOffset = 0;

    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }

    Id3v2FrameHeader *newHeader = NULL;
    Id3v2SeekBody *newSeekBody = NULL;

    newHeader = id3v2ParseFrameHeader(buffer, header);

    if(newHeader == NULL){
        return NULL;
    }

    if(newHeader->frameSize == 0){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    if(newHeader->idNum != SEEK){
        id3v2FreeFrameHeader(newHeader);
        return NULL;
    }

    buffer = buffer + newHeader->headerSize;
    
    newSeekBody = id3v2ParseSeekBody(buffer, newHeader);

    return id3v2NewFrame(newHeader, newSeekBody);

}

Id3v2Frame *id3v2CopySeekFrame(Id3v2Frame *frame){

    if(frame == NULL){
        return NULL;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2SeekBody *body = NULL;

    header = id3v2CopyFrameHeader(frame->header);
    body = id3v2CopySeekBody(frame->frame);

    return id3v2NewFrame(header, (void *)body);
}

Id3v2SeekBody *id3v2CopySeekBody(Id3v2SeekBody *body){
    return (body == NULL) ? NULL : id3v2NewSeekBody(body->minimumOffsetToNextTag);
}

Id3v2SeekBody *id3v2ParseSeekBody(unsigned char *buffer, Id3v2FrameHeader *frameHeader){

    if(buffer == NULL){
        return NULL;
    }

    if(frameHeader == NULL){
        return NULL;
    }

    int minimumOffsetToNextTag = 0;

    if(frameHeader->frameSize > 0){
        minimumOffsetToNextTag = getBits8(buffer, frameHeader->frameSize);
    }

    return id3v2NewSeekBody(minimumOffsetToNextTag);
}

Id3v2SeekBody *id3v2NewSeekBody(int minimumOffsetToNextTag){

    Id3v2SeekBody *body = malloc(sizeof(Id3v2SeekBody));

    body->minimumOffsetToNextTag = minimumOffsetToNextTag;

    return body;
}

void id3v2FreeSeekFrame(Id3v2Frame *toDelete){
    
    if(toDelete == NULL){
        return;
    }

    if(toDelete->header != NULL){
        id3v2FreeFrameHeader(toDelete->header);
    }

    if(toDelete->frame == NULL){
        free(toDelete);
        return;
    }

    free(toDelete->frame);
    free(toDelete);
}

/*
    FLAG FUNCTIONS
*/

Id3v2FlagContent *id3v2NewFlagContent(bool tagAlterPreservation, bool fileAlterPreservation, bool readOnly, bool unsynchronisation, bool dataLengthIndicator, unsigned int decompressedSize, unsigned char encryption, unsigned char grouping){

    Id3v2FlagContent *newContent = malloc(sizeof(Id3v2FlagContent));

    newContent->tagAlterPreservation = tagAlterPreservation;
    newContent->fileAlterPreservation = fileAlterPreservation;
    newContent->readOnly = readOnly;
    newContent->decompressedSize = decompressedSize;
    newContent->encryption = encryption;
    newContent->grouping = grouping;
    newContent->unsynchronisation = unsynchronisation;
    newContent->dataLengthIndicator = dataLengthIndicator;

    return newContent;
}

Id3v2FlagContent *id3v2ParseFlagContent(unsigned char *buffer, Id3v2HeaderVersion version){

    if(buffer == NULL){
        return NULL;
    }

    if(version == ID3V2INVLAIDVERSION){
        return NULL;
    }

    bool tagAlterPreservation = false;
    bool fileAlterPreservation = false;
    bool readOnly = false;
    bool unsynchronisation = false;
    bool dataLengthIndicator = false;
    unsigned int decompressedSize = 0;
    unsigned char encryption = 0x00;
    unsigned char grouping = 0x00;


    // read flags
    if(version != ID3V24){
        tagAlterPreservation = ((buffer[0] >> 7) & 1) ? true : false;
        fileAlterPreservation = ((buffer[0] >> 6) & 1) ? true : false;
        readOnly = ((buffer[0] >> 5) & 1) ? true : false;
        buffer = buffer + 1;

        decompressedSize = ((buffer[0] >> 7) & 1) ? true : false;
        encryption = ((buffer[0] >> 6) & 1) ? true : false;
        grouping = ((buffer[0] >> 5) & 1) ? true : false;
        buffer = buffer + 1;
    
        // read the extra bits after the flags
        if(decompressedSize == true){
            decompressedSize = getBits8((unsigned char *)buffer, 4);
            buffer = buffer + 4;

        }

        if(encryption == true){
            encryption = buffer[0];
            buffer = buffer + 1;
        }

        if(grouping == true){
            grouping = buffer[0];
            buffer = buffer + 1;
        }

    }else{
        // format %0abc0000 %0h00kmnp
        tagAlterPreservation = ((buffer[0] >> 6) & 1) ? true : false;
        fileAlterPreservation = ((buffer[0] >> 5) & 1) ? true : false;
        readOnly = ((buffer[0] >> 5) & 4) ? true : false;
        buffer = buffer + 1;

        grouping = ((buffer[0] >> 6) & 1) ? true : false;
        decompressedSize = ((buffer[0] >> 3) & 1) ? true : false;
        encryption = ((buffer[0] >> 2) & 1) ? true : false;
        unsynchronisation = ((buffer[0] >> 1) & 1) ? true : false;
        dataLengthIndicator = ((buffer[0] >> 0) & 1) ? true : false;
        buffer = buffer + 1;

        if(grouping == true){
            grouping = buffer[0];
            buffer = buffer + 1;
        }

        if(decompressedSize == true){
            decompressedSize = getBits8((unsigned char *)buffer, 4);
            buffer = buffer + 4;

        }

        if(encryption == true){
            encryption = buffer[0];
            buffer = buffer + 1;
        }

    }

    return id3v2NewFlagContent(tagAlterPreservation, fileAlterPreservation, readOnly, unsynchronisation, dataLengthIndicator, decompressedSize, encryption, grouping);
}

Id3v2FlagContent *id3v2CopyFlagContent(Id3v2FlagContent *flagContent){

    if(flagContent == NULL){
        return NULL;
    }

    bool tagAlterPreservation = flagContent->tagAlterPreservation;
    bool fileAlterPreservation = flagContent->fileAlterPreservation;
    bool readOnly = flagContent->readOnly;
    bool unsynchronisation = flagContent->unsynchronisation;
    bool dataLengthIndicator = flagContent->dataLengthIndicator;
    unsigned int decompressedSize = flagContent->decompressedSize;
    unsigned char encryption = flagContent->encryption;
    unsigned char grouping = flagContent->grouping;

    return id3v2NewFlagContent(tagAlterPreservation, fileAlterPreservation, readOnly, unsynchronisation, dataLengthIndicator, decompressedSize, encryption, grouping);
}

void id3v2FreeFlagContent(Id3v2FlagContent *toDelete){

    if(toDelete != NULL){
        free(toDelete);
    }
}

unsigned int id3v2SizeOfFlagContent(Id3v2FlagContent *content){

    int size = 0;

    if(content == NULL){
        return 0;
    }

    if(content->decompressedSize != 0){
        size = size + 4;
    }

    if(content->encryption != 0x00){
        size = size + 1;
    }

    if(content->grouping != 0x00){
        size = size + 1;
    }

    // flags are 2 bytes to begin with
    return size + ID3V2_FLAG_SIZE_OF_BYTES;
}

/*
    FRAME HEADER FUNCTIONS
*/

Id3v2FrameHeader *id3v2ParseFrameHeader(unsigned char *buffer, Id3v2Header *header){

    if(buffer == NULL){
        return NULL;
    }

    char *id = NULL;
    unsigned int frameSize = 0;
    unsigned int headerSize = 0;
    unsigned char *frameSizeOffset = NULL;
    Id3v2FlagContent *flagContent = NULL;
    
    int versionOffset = 0;

    // calculate an offset for id and size
    if((versionOffset = id3v2IdAndSizeOffset(header)) == 0){
        return NULL;
    }
    headerSize = versionOffset * 2;

    // read id
    id = calloc(sizeof(unsigned int), versionOffset + 1);
    memcpy(id, buffer, versionOffset);
    buffer = buffer + versionOffset;

    // read size
    frameSize = (header->versionMajor == ID3V24) ? syncint_decode(getBits8(buffer, versionOffset)): getBits8(buffer, versionOffset);
    frameSizeOffset = buffer;
    buffer = buffer + versionOffset;

    // parse flags for 2.3 and 2.4
    if(header->versionMajor != ID3V22){
        flagContent = id3v2ParseFlagContent(buffer, header->versionMajor);
        headerSize = headerSize + id3v2SizeOfFlagContent(flagContent);
        buffer = buffer + id3v2SizeOfFlagContent(flagContent);

        // in id3v2.4 a syncsafe int or a 4 byte int can be used
        if(flagContent->dataLengthIndicator == true){
            frameSize = getBits8(frameSizeOffset, versionOffset);
        }
    }

    return id3v2NewFrameHeader(id, frameSize, headerSize, flagContent);
}

Id3v2FrameHeader *id3v2NewFrameHeader(char *id, unsigned int frameSize, unsigned int headerSize, Id3v2FlagContent *flagContent){

    Id3v2FrameHeader *newHeader = malloc(sizeof(Id3v2FrameHeader));

    Id3v2FrameId idNum = id3v2FrameIdFromStr(id);

    // copy data
    newHeader->id = id;
    newHeader->frameSize = frameSize;
    newHeader->headerSize = headerSize;
    newHeader->idNum = idNum;
    newHeader->flagContent = flagContent;

    return newHeader;
}

Id3v2FrameHeader *id3v2CopyFrameHeader(Id3v2FrameHeader *header){

    if(header == NULL){
        return NULL;
    }

    int idLen = strlen(header->id);
    char *id = calloc(sizeof(char), idLen + 1);
    unsigned int frameSize = header->frameSize;
    unsigned int headerSize = header->headerSize;
    Id3v2FlagContent *flagContent = id3v2CopyFlagContent(header->flagContent);

    memcpy(id, header->id, idLen);

    return id3v2NewFrameHeader(id, frameSize, headerSize, flagContent);
}

void id3v2FreeFrameHeader(Id3v2FrameHeader *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->id != NULL){
        free(toDelete->id);
    }

    id3v2FreeFlagContent(toDelete->flagContent);

    free(toDelete);
}

/*
    OTHER FUNCTIONS
*/

int id3v2IdAndSizeOffset(Id3v2Header *header){

    int versionOffset = 0;

    if(header->versionMajor == ID3V2INVLAIDVERSION){
        return versionOffset;

    }else if(header->versionMajor == ID3V22){
        versionOffset = ID3V22_ID_SIZE;
    
    }else{
        // 2.3 and 2.4 have the same size and frame ids
        versionOffset = ID3V23_ID_SIZE;
    }

    return versionOffset;
}

Id3v2FrameId id3v2FrameIdFromStr(char *str){

    int offset = strlen(str);

    switch (offset){
    
        // 2.2 frames
        case ID3V22_SIZE_OF_SIZE_BYTES:
            if(strncmp("BUF", str, offset) == 0){
                return BUF;
            }
            if(strncmp("CNT", str, offset) == 0){
                return CNT;
            }
            if(strncmp("COM", str, offset) == 0){
                return COM;
            }
            if(strncmp("CRA", str, offset) == 0){
                return CRA;
            }
            if(strncmp("CRM", str, offset) == 0){
                return CRM;
            }
            if(strncmp("ETC", str, offset) == 0){
                return ETC;
            }
            if(strncmp("EQU", str, offset) == 0){
                return EQU;
            }
            if(strncmp("GEO", str, offset) == 0){
                return GEO;
            }
            if(strncmp("IPL", str, offset) == 0){
                return IPL;
            }
            if(strncmp("LNK", str, offset) == 0){
                return LNK;
            }
            if(strncmp("MCI", str, offset) == 0){
                return MCI;
            }
            if(strncmp("MLL", str, offset) == 0){
                return MLL;
            }
            if(strncmp("PIC", str, offset) == 0){
                return PIC;
            }
            if(strncmp("POP", str, offset) == 0){
                return POP;
            }
            if(strncmp("REV", str, offset) == 0){
                return REV;
            }
            if(strncmp("RVA", str, offset) == 0){
                return RVA;
            }
            if(strncmp("SLT", str, offset) == 0){
                return SLT;
            }
            if(strncmp("STC", str, offset) == 0){
                return STC;
            }
            if(strncmp("TAL", str, offset) == 0){
                return TAL;
            }
            if(strncmp("TBP", str, offset) == 0){
                return TBP;
            }
            if(strncmp("TCM", str, offset) == 0){
                return TCM;
            }
            if(strncmp("TCO", str, offset) == 0){
                return TCO;
            }
            if(strncmp("TCR", str, offset) == 0){
                return TCR;
            }
            if(strncmp("TDA", str, offset) == 0){
                return TDA;
            }
            if(strncmp("TDY", str, offset) == 0){
                return TDY;
            }
            if(strncmp("TEN", str, offset) == 0){
                return TEN;
            }
            if(strncmp("TFT", str, offset) == 0){
                return TFT;
            }
            if(strncmp("TIM", str, offset) == 0){
                return TIM;
            }
            if(strncmp("TKE", str, offset) == 0){
                return TKE;
            }
            if(strncmp("TLA", str, offset) == 0){
                return TLA;
            }
            if(strncmp("TLE", str, offset) == 0){
                return TLE;
            }
            if(strncmp("TMT", str, offset) == 0){
                return TMT;
            }
            if(strncmp("TOA", str, offset) == 0){
                return TOA;
            }
            if(strncmp("TOF", str, offset) == 0){
                return TOF;
            }
            if(strncmp("TOL", str, offset) == 0){
                return TOL;
            }
            if(strncmp("TOR", str, offset) == 0){
                return TOR;
            }
            if(strncmp("TOT", str, offset) == 0){
                return TOT;
            }
            if(strncmp("TP1", str, offset) == 0){
                return TP1;
            }
            if(strncmp("TP2", str, offset) == 0){
                return TP2;
            }
            if(strncmp("TP3", str, offset) == 0){
                return TP3;
            }
            if(strncmp("TP4", str, offset) == 0){
                return TP4;
            }
            if(strncmp("TPA", str, offset) == 0){
                return TPA;
            }
            if(strncmp("TPB", str, offset) == 0){
                return TPB;
            }
            if(strncmp("TRC", str, offset) == 0){
                return TRC;
            }
            if(strncmp("TRD", str, offset) == 0){
                return TRD;
            }
            if(strncmp("TRK", str, offset) == 0){
                return TRK;
            }
            if(strncmp("TSI", str, offset) == 0){
                return TSI;
            }
            if(strncmp("TSS", str, offset) == 0){
                return TSS;
            }
            if(strncmp("TT1", str, offset) == 0){
                return TT1;
            }
            if(strncmp("TT2", str, offset) == 0){
                return TT2;
            }
            if(strncmp("TT3", str, offset) == 0){
                return TT3;
            }
            if(strncmp("TXT", str, offset) == 0){
                return TXT;
            }
            if(strncmp("TXX", str, offset) == 0){
                return TXX;
            }
            if(strncmp("TYE", str, offset) == 0){
                return TYE;
            }
            if(strncmp("UFI", str, offset) == 0){
                return UFI;
            }
            if(strncmp("ULT", str, offset) == 0){
                return ULT;
            }
            if(strncmp("WAF", str, offset) == 0){
                return WAF;
            }
            if(strncmp("WAR", str, offset) == 0){
                return WAR;
            }
            if(strncmp("WAS", str, offset) == 0){
                return WAS;
            }
            if(strncmp("WCM", str, offset) == 0){
                return WCM;
            }
            if(strncmp("WCP", str, offset) == 0){
                return WCP;
            }
            if(strncmp("WPB", str, offset) == 0){
                return WPB;
            }
            if(strncmp("WXX", str, offset) == 0){
                return WXX;
            }
        // 2.3 and 2.4 frames
        case ID3V23_ID_SIZE:
            if(strncmp("AENC", str, offset) == 0){
                return AENC;
            }
            if(strncmp("APIC", str, offset) == 0){
                return APIC;
            }
            if(strncmp("COMM", str, offset) == 0){
                return COMM;
            }
            if(strncmp("COMR", str, offset) == 0){
                return COMR;
            }
            if(strncmp("ENCR", str, offset) == 0){
                return ENCR;
            }
            if(strncmp("EQUA", str, offset) == 0){
                return EQUA;
            }
            if(strncmp("ETCO", str, offset) == 0){
                return ETCO;
            }
            if(strncmp("GEOB", str, offset) == 0){
                return GEOB;
            }
            if(strncmp("GRID", str, offset) == 0){
                return GRID;
            }
            if(strncmp("IPLS", str, offset) == 0){
                return IPLS;
            }
            if(strncmp("LINK", str, offset) == 0){
                return LINK;
            }
            if(strncmp("MCDI", str, offset) == 0){
                return MCDI;
            }
            if(strncmp("MLLT", str, offset) == 0){
                return MLLT;
            }
            if(strncmp("OWNE", str, offset) == 0){
                return OWNE;
            }
            if(strncmp("PRIV", str, offset) == 0){
                return PRIV;
            }
            if(strncmp("PCNT", str, offset) == 0){
                return PCNT;
            }
            if(strncmp("POPM", str, offset) == 0){
                return POPM;
            }
            if(strncmp("POSS", str, offset) == 0){
                return POSS;
            }
            if(strncmp("RBUF", str, offset) == 0){
                return RBUF;
            }
            if(strncmp("RVAD", str, offset) == 0){
                return RVAD;
            }
            if(strncmp("RVRB", str, offset) == 0){
                return RVRB;
            }
            if(strncmp("SYLT", str, offset) == 0){
                return SYLT;
            }
            if(strncmp("SYTC", str, offset) == 0){
                return SYTC;
            }
            if(strncmp("TALB", str, offset) == 0){
                return TALB;
            }
            if(strncmp("TBPM", str, offset) == 0){
                return TBPM;
            }
            if(strncmp("TCOM", str, offset) == 0){
                return TCOM;
            }
            if(strncmp("TCON", str, offset) == 0){
                return TCON;
            }
            if(strncmp("TCOP", str, offset) == 0){
                return TCOP;
            }
            if(strncmp("TDAT", str, offset) == 0){
                return TDAT;
            }
            if(strncmp("TDLY", str, offset) == 0){
                return TDLY;
            }
            if(strncmp("TENC", str, offset) == 0){
                return TENC;
            }
            if(strncmp("TEXT", str, offset) == 0){
                return TEXT;
            }
            if(strncmp("TFLT", str, offset) == 0){
                return TFLT;
            }
            if(strncmp("TIME", str, offset) == 0){
                return TIME;
            }
            if(strncmp("TIT1", str, offset) == 0){
                return TIT1;
            }
            if(strncmp("TIT2", str, offset) == 0){
                return TIT2;
            }
            if(strncmp("TIT3", str, offset) == 0){
                return TIT3;
            }
            if(strncmp("TKEY", str, offset) == 0){
                return TKEY;
            }
            if(strncmp("TLAN", str, offset) == 0){
                return TLAN;
            }
            if(strncmp("TLEN", str, offset) == 0){
                return TLEN;
            }
            if(strncmp("TMED", str, offset) == 0){
                return TMED;
            }
            if(strncmp("TOAL", str, offset) == 0){
                return TOAL;
            }
            if(strncmp("TOFN", str, offset) == 0){
                return TOFN;
            }
            if(strncmp("TOLY", str, offset) == 0){
                return TOLY;
            }
            if(strncmp("TOPE", str, offset) == 0){
                return TOPE;
            }
            if(strncmp("TORY", str, offset) == 0){
                return TORY;
            }
            if(strncmp("TOWN", str, offset) == 0){
                return TOWN;
            }
            if(strncmp("TPE1", str, offset) == 0){
                return TPE1;
            }
            if(strncmp("TPE2", str, offset) == 0){
                return TPE2;
            }
            if(strncmp("TPE3", str, offset) == 0){
                return TPE3;
            }
            if(strncmp("TPE4", str, offset) == 0){
                return TPE4;
            }
            if(strncmp("TPOS", str, offset) == 0){
                return TPOS;
            }
            if(strncmp("TPUB", str, offset) == 0){
                return TPUB;
            }
            if(strncmp("TRCK", str, offset) == 0){
                return TRCK;
            }
            if(strncmp("TRDA", str, offset) == 0){
                return TRDA;
            }
            if(strncmp("TRSN", str, offset) == 0){
                return TRSN;
            }
            if(strncmp("TRSO", str, offset) == 0){
                return TRSO;
            }
            if(strncmp("TSIZ", str, offset) == 0){
                return TSIZ;
            }
            if(strncmp("TSRC", str, offset) == 0){
                return TSRC;
            }
            if(strncmp("TSSE", str, offset) == 0){
                return TSSE;
            }
            if(strncmp("TYER", str, offset) == 0){
                return TYER;
            }
            if(strncmp("TXXX", str, offset) == 0){
                return TXXX;
            }
            if(strncmp("UFID", str, offset) == 0){
                return UFID;
            }
            if(strncmp("USER", str, offset) == 0){
                return USER;
            }
            if(strncmp("USLT", str, offset) == 0){
                return USLT;
            }
            if(strncmp("WCOM", str, offset) == 0){
                return WCOM;
            }
            if(strncmp("WCOP", str, offset) == 0){
                return WCOP;
            }
            if(strncmp("WOAF", str, offset) == 0){
                return WOAF;
            }
            if(strncmp("WOAR", str, offset) == 0){
                return WOAR;
            }
            if(strncmp("WOAS", str, offset) == 0){
                return WOAS;
            }
            if(strncmp("WORS", str, offset) == 0){
                return WORS;
            }
            if(strncmp("WPAY", str, offset) == 0){
                return WPAY;
            }
            if(strncmp("WPUB", str, offset) == 0){
                return WPUB;
            }
            if(strncmp("WXXX", str, offset) == 0){
                return WXXX;
            }
            if(strncmp("SIGN", str, offset) == 0){
                return SIGN;
            }
            if(strncmp("SEEK", str, offset) == 0){
                return SEEK;
            }
        default:
            return HUH; // the idk frame
    }

    return HUH;
}
