#include <stdio.h>
#include <stdlib.h>
#include "id3v2.h"

void id3v2FreeTag(Id3v2 *tag){

    if(tag == NULL){
        return;
    }

    destroyList(tag->frames);
    id3v2FreeHeader(tag->header);
    free(tag);
}

Id3v2 *Id3v2Tag(const char *filePath){
    
    if(filePath == NULL){
        return NULL;
    }
    
    
    unsigned char *headerBytes = NULL;
    unsigned char *tagSizeOffset;
    unsigned char *extOffset;
    int bufferSize = ID3V2_HEADER_SIZE;
    FILE *fp = NULL;
    Id3v2 *tag = NULL;
    Id3v2Header *headerInfo = NULL;
    List *frameList = NULL;

    if(!containsId3v2(filePath)){
        return NULL;
    }

    if((fp = fopen(filePath,"rb")) == NULL){
        return NULL;
    }

    headerBytes = calloc(sizeof(unsigned char), ID3V2_FULL_HEADER_LEN + 1);
    if(fread(headerBytes, 1, sizeof(unsigned char) * ID3V2_FULL_HEADER_LEN, fp) == 0){
        fclose(fp);
        return NULL;
    }

    //calculate buffer size
    extOffset = headerBytes;
    
    if((((extOffset+5)[0] >> 7) & 1) ? true: false){
        extOffset = extOffset + ID3V2_EXTHEADER_SIZE_OFFSET;
        bufferSize = bufferSize + syncint_decode(getBits8(extOffset, ID3V2_HEADER_SIZE_LEN));   
    }

    //get buffer
    fseek(fp, 0, SEEK_SET);

    free(headerBytes);
    headerBytes = calloc(sizeof(unsigned char), bufferSize + 1);
    if(fread(headerBytes, 1, sizeof(unsigned char) * bufferSize, fp) == 0){
        fclose(fp);
        return NULL;
    }

    //parse header
    if((headerInfo = id3v2ParseHeader(headerBytes, bufferSize)) == NULL){
        return NULL;
    }
    tag = malloc(sizeof(Id3v2));

    tag->header = headerInfo;
    free(headerBytes);
    fclose(fp);

    //unsync is not supported but the header can still be read from
    if(tag->header->unsynchronisation == true){
        return tag;
    }

    frameList = id3v2ExtractFrames(filePath, headerInfo);

    tag->frames = frameList;
    
    return tag;
}