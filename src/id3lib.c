#include <stdio.h>
#include <stdlib.h>
#include "id3.h"

Id3Metadata *id3NewMetadataFromFile(const char *filePath){

    if(filePath == NULL){
        return NULL;
    }

    unsigned char *buffer = NULL;
    size_t fileSize = 0;
    Id3Metadata *metadata = NULL;
    FILE *fp = NULL;
    
    if((fp = fopen(filePath, "rb")) == NULL){
        return NULL;
    }

    //read file size
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    //read the entire file
    buffer = calloc(sizeof(unsigned char), fileSize + 1);
    if((fread(buffer, sizeof(unsigned char), fileSize, fp)) == 0){
        fclose(fp);
        return NULL;
    }
    fclose(fp);

    //parse tags
    metadata = id3NewMetadataFromBuffer(buffer, fileSize);
    free(buffer);

    return metadata;
}

Id3Metadata *id3NewMetadataFromBuffer(unsigned char *buffer, int size){

    if(buffer == NULL){
        return NULL;
    }

    Id3Metadata *metadata = malloc(sizeof(Id3Metadata));
    List *version1 = newList(id3v1FreeTag);
    List *version2 = newList(id3v2FreeTag);
    Id3Reader *stream = id3NewReader(buffer, size);

    //id3v1 tag search
    //as per spec this is the only place this tag version can be
    id3ReaderSeek(stream, ID3V1_MAX_BYTES, SEEK_END);

    if(containsId3v1(id3ReaderCursor(stream))){
        listPush(version1, (void *)id3v1TagFromBuffer(id3ReaderCursor(stream)));
    }

    //there can be many id3v2 tags at any point in the file
    //check every byte
    id3ReaderSeek(stream, 0, SEEK_SET);
    while(id3ReaderGetCh(stream) != EOF){
        if(containsId3v2(id3ReaderCursor(stream))){
            
            Id3v2Tag *tag = id3v2ParseTagFromBuffer(id3ReaderCursor(stream),stream->bufferSize - stream->cursor);   
            
            if(tag != NULL){
                listPush(version2, (void *)tag);
                id3ReaderSeek(stream, stream->bufferSize - stream->cursor, SEEK_CUR);
            }else{
                id3ReaderSeek(stream, 1, SEEK_CUR);
            }

            
        }else{
            id3ReaderSeek(stream, 1, SEEK_CUR);
        }
        
    
    }

    free(stream);
    //id3FreeReader(stream);

    metadata->version1 = version1;
    metadata->version2 = version2;

    return metadata;
}

bool hasId3v1(Id3Metadata *metadata){

    if(metadata == NULL){
        return false;
    }

    if(metadata->version1 == NULL){
        return false;
    }

    if(metadata->version1->head != NULL){
        return true;
    }

    return false;
}

bool hasId3v2(Id3Metadata *metadata){

    if(metadata == NULL){
        return false;
    }

    if(metadata->version2 == NULL){
        return false;
    }

    if(metadata->version2->head != NULL){
        return true;
    }

    return false;
}

void id3FreeMetadata(Id3Metadata *toDelete){
    
    if(toDelete == NULL){
        return;
    }

    if(toDelete->version1 != NULL){
        destroyList(toDelete->version1);
    }

    if(toDelete->version2 != NULL){
        destroyList(toDelete->version2);
    }

    free(toDelete);
}
