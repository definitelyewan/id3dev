#include <stdio.h>
#include <stdlib.h>
#include "id3.h"
#include "id3v2.h"
#include "id3v1.h"

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

    bool first = true;
    Id3v1Tag *version1 = NULL;
    Id3v2Tag *version2 = NULL;
    Id3Reader *stream = id3NewReader(buffer, size);

    //id3v1 tag search
    //as per spec this is the only place this tag version can be
    id3ReaderSeek(stream, ID3V1_MAX_BYTES, SEEK_END);

    if(containsId3v1(id3ReaderCursor(stream))){
        version1 = id3v1TagFromBuffer(id3ReaderCursor(stream));
    }

    //there can be many id3v2 tags at any point in the file
    //the only tag we care about is the first one but...
    //if there is a tag marked with u in the flags then we want that one
    //check every byte
    id3ReaderSeek(stream, 0, SEEK_SET);
    while(id3ReaderGetCh(stream) != EOF){
        if(containsId3v2(id3ReaderCursor(stream))){
            
            Id3v2Tag *tag = id3v2ParseTagFromBuffer(id3ReaderCursor(stream),stream->bufferSize - stream->cursor);   
            
            if(tag != NULL){
                
                if(first == true){
                    version2 = tag;
                    first = false;
                }

                if(tag->header->versionMinor == ID3V24 && tag->header->extendedHeader != NULL){
                    if(tag->header->extendedHeader->update){
                        id3v2FreeTag(version2);
                        version2 = tag;
                    }
                }

                id3ReaderSeek(stream, stream->bufferSize - stream->cursor, SEEK_CUR);
            }else{
                id3ReaderSeek(stream, 1, SEEK_CUR);
            }

        }else{
            id3ReaderSeek(stream, 1, SEEK_CUR);
        }
    }

    free(stream);
    //bug! figure out why this causes a memory error at a later time
    //works with no errors if commented out
    //id3FreeReader(stream);

    return id3NewMetadata(version1, version2);
}

Id3Metadata *id3CopyMetadata(Id3Metadata *toCopy){
    return (toCopy == NULL) ? NULL : id3NewMetadata(id3v1CopyTag(toCopy->version1), id3v2CopyTag(toCopy->version2));
}

Id3Metadata *id3NewMetadata(Id3v1Tag *v1, Id3v2Tag *v2){

    Id3Metadata *metadata = malloc(sizeof(Id3Metadata));

    metadata->version1 = v1;
    metadata->version2 = v2;

    return metadata;
}

bool hasId3v1(Id3Metadata *metadata){

    if(metadata == NULL){
        return false;
    }

    if(metadata->version1 == NULL){
        return false;
    }

    if(metadata->version1 != NULL){
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

    if(metadata->version2 != NULL){
        return true;
    }

    return false;
}

void id3FreeMetadata(Id3Metadata *toDelete){
    
    if(toDelete == NULL){
        return;
    }

    if(toDelete->version1 != NULL){
        id3v1FreeTag(toDelete->version1);
    }

    if(toDelete->version2 != NULL){
        id3v2FreeTag(toDelete->version2);
    }

    free(toDelete);
}
