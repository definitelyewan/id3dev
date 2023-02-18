#include <stdio.h>
#include <stdlib.h>
#include "id3v2.h"

Id3v2Tag *id3v2ParseTagFromFile(const char *filePath){
    
    if(filePath == NULL){
        return NULL;
    }

    unsigned char headerBytes[ID3V2_HEADER_SIZE + 1];
    unsigned char *buffer = NULL;
    int tagSize = 0;
    FILE *fp = NULL;


    if((fp = fopen(filePath,"rb")) == NULL){
        return NULL;
    }

    //read the tag size
    if(fread(headerBytes, 1, sizeof(unsigned char) * ID3V2_HEADER_SIZE, fp) == 0){
        fclose(fp);
        return NULL;
    }

    if(!containsId3v2(headerBytes)){
        fclose(fp);
        return NULL;
    }

    buffer = headerBytes + ID3V2_TAG_SIZE_OFFSET;
    tagSize = syncint_decode(getBits8(buffer,ID3V2_HEADER_SIZE_LEN));
    
    //reset file so an enter buffer can be set
    fseek(fp, 0, SEEK_SET);

    //create a buffer based of the size in the header
    buffer = calloc(sizeof(unsigned char), tagSize + 1);
    if(fread(buffer, 1, sizeof(unsigned char) * tagSize, fp) == 0){
        fclose(fp);
        return NULL;
    }
    fclose(fp);

    return id3v2ParseTagFromBuffer(buffer, tagSize);
}

Id3v2Tag *id3v2ParseTagFromBuffer(unsigned char *buffer, int tagSize){

    unsigned char *frames = NULL;
    Id3v2Header *headerInfo = NULL;
    List *frameList = NULL;


    //read header information
    if((headerInfo = id3v2ParseHeader(buffer, tagSize)) == NULL){
        free(buffer);
        return NULL;
    }

    //unsynchronisation is not supported
    if(headerInfo->unsynchronisation == true){
        free(buffer);
        return id3v2NewTag(headerInfo, NULL);
    }

    //skip to the index where frames start
    frames = buffer;
    if(headerInfo->extendedHeader != NULL){
        frames = buffer + headerInfo->extendedHeader->size;
    }
    frames = frames + ID3V2_HEADER_SIZE;

    //extract frames from the file
    frameList = id3v2ExtractFrames(frames, headerInfo);
    
    free(buffer);

    return id3v2NewTag(headerInfo, frameList);
}

Id3v2Tag *id3v2NewTag(Id3v2Header *header, List *frames){

    Id3v2Tag *tag = malloc(sizeof(Id3v2Tag));

    tag->header = header;
    tag->frames = frames;

    return tag;
}

void id3v2FreeTag(void *toDelete){

    if(toDelete == NULL){
        return;
    }

    Id3v2Tag *tag = (Id3v2Tag *)toDelete;

    destroyList(tag->frames);
    id3v2FreeHeader(tag->header);
    free(tag);
}
