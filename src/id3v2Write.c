#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v2Frames.h"
#include "id3v2Manipulate.h"
#include "id3v2Write.h"
#include "id3Reader.h"

void id3v2WritePicture(const char *fileName, Id3v2Frame *frame){

    if(fileName == NULL){
        return;
    }

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    //cannot write data if its not writable
    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    switch(frame->header->idNum){
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

void id3v2WriteGeneralEncapsulatedObject(Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    //cannot write data if its not writable
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


    id3buf buildName = NULL;
    id3buf mime = NULL;
    id3buf fileName = NULL;
    id3buf convertedFileName = NULL;
    id3buf data = NULL;
    unsigned int mimeLen = 0;
    unsigned int fileNameLen = 0;
    unsigned int dataLen = 0;
    FILE *fp = NULL;

    mime = id3v2ReadMIMEType(frame);
    fileName = id3v2ReadObjectFileName(frame);
    data = id3v2ReadGeneralEncapsulatedObjectValue(frame);
    dataLen = id3v2ReadFrameBinaryObjectSize(frame);
    
    //build a file name from in the frame
    if(fileName != NULL){
        convertedFileName = id3TextFormatConvert(fileName, id3strlen(fileName, id3v2ReadEncoding(frame)), UTF8);
        free(fileName);
    }

    if(convertedFileName != NULL){

        mimeLen = id3strlen(mime, ISO_8859_1);
        fileNameLen = id3strlen(convertedFileName, UTF8);

        buildName = calloc(sizeof(unsigned char), mimeLen + fileNameLen + 2);// +1 for .
        strncpy((char *)buildName, (char *)convertedFileName, fileNameLen);
        buildName[fileNameLen] = '.';
        strncat(((char *)buildName) + fileNameLen, (char *)mime, mimeLen);

        fp = fopen((char *)buildName, "wb");
        fwrite(data, sizeof(unsigned char), dataLen, fp);
        fclose(fp);
        
        free(convertedFileName);
    }


    if(mime != NULL){
        free(mime);
    }

    if(data != NULL){
        free(data);
    }
    
    if(buildName != NULL){
        free(buildName);
    }
}

