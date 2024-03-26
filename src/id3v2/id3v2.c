#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "byteStream.h"
#include "id3v2.h"
#include "id3v2Frame.h"
#include "id3v2Parser.h"
#include "id3v2TagIdentity.h"

/**
 * @brief Generates a Id3v2Tag structure from a file. If
 * this function fails to generate a tag, it will return NULL.
 * 
 * @param filename 
 * @return Id3v2Tag* 
 */
Id3v2Tag *id3v2TagFromFile(const char *filename){

    ByteStream *stream = NULL;
    Id3v2Tag *tag = NULL;

    if(filename == NULL){
        return NULL;
    }

    if((stream = byteStreamFromFile(filename)) == NULL){
        return NULL;
    }

    tag = id3v2ParseTagFromStream(stream, NULL);
    byteStreamDestroy(stream);

    return tag;
}

/**
 * @brief Deep copies an Id3v2Tag structure.
 * 
 * @param toCopy 
 * @return Id3v2Tag* 
 */
Id3v2Tag *id3v2CopyTag(Id3v2Tag *toCopy){

    Id3v2TagHeader *header = NULL;

    if(toCopy == NULL){
        return NULL;
    }

    if(toCopy->header == NULL){
        return NULL;
    }
    
    header = id3v2CreateTagHeader(toCopy->header->majorVersion, toCopy->header->minorVersion, toCopy->header->flags, NULL);
    header->extendedHeader = (toCopy->header->extendedHeader != NULL) ? id3v2CreateExtendedTagHeader(toCopy->header->extendedHeader->padding, toCopy->header->extendedHeader->crc, toCopy->header->extendedHeader->update, toCopy->header->extendedHeader->tagRestrictions, toCopy->header->extendedHeader->restrictions) : NULL;

    return id3v2CreateTag(header, listDeepCopy(toCopy->frames));
}

/**
 * @brief Compares two tags and returns true if they are the same, false otherwise.
 * 
 * @param tag1 
 * @param tag2 
 * @return true 
 * @return false 
 */
bool id3v2CompareTag(Id3v2Tag *tag1, Id3v2Tag *tag2){

    if(tag1 == NULL || tag2 == NULL){
        return false;
    }

    if(tag1->header == NULL || tag2->header == NULL){
        return false;

    }

    // check header

    if(tag1->header->majorVersion != tag2->header->majorVersion){
        return false;
    }

    if(tag1->header->minorVersion != tag2->header->minorVersion){
        return false;
    }

    if(tag1->header->flags != tag2->header->flags){
        return false;
    }

    if(tag1->header->extendedHeader != tag2->header->extendedHeader){
        return false;

    }

    // check extended header

    if(tag1->header->extendedHeader != NULL && tag2->header->extendedHeader != NULL){
        if(tag1->header->extendedHeader->padding != tag2->header->extendedHeader->padding){
            return false;
        }

        if(tag1->header->extendedHeader->crc != tag2->header->extendedHeader->crc){
            return false;
        }

        if(tag1->header->extendedHeader->update != tag2->header->extendedHeader->update){
            return false;
        }

        if(tag1->header->extendedHeader->tagRestrictions != tag2->header->extendedHeader->tagRestrictions){
            return false;
        }

        if(tag1->header->extendedHeader->restrictions != tag2->header->extendedHeader->restrictions){
            return false;
        }

    }

    // check frames

    ListIter frames1 = id3v2CreateFrameTraverser(tag1);
    ListIter frames2 = id3v2CreateFrameTraverser(tag2);

    Id3v2Frame *f1 = NULL;
    Id3v2Frame *f2 = NULL;


    while((f1 = id3v2FrameTraverse(&frames1)) != NULL){
        f2 = id3v2FrameTraverse(&frames2);

        if(f2 == NULL){
            return false;
        }

        if(id3v2CompareFrame(f1, f2) != 0){
            return false;
        }
    }

    if((f2 = id3v2FrameTraverse(&frames2)) != NULL){
        return false;
    }
    
    return true;
}

/**
 * @brief Returne a copy of the first occurance of a frame with the given id. If
 * no frame is found, this function will return NULL.
 * 
 * @param id 
 * @param tag 
 * @return Id3v2Frame* 
 */
Id3v2Frame *id3v2ReadFrameByID(const char id[ID3V2_FRAME_ID_MAX_SIZE], Id3v2Tag *tag){

    if(id == NULL || tag == NULL){
        return NULL;
    }

    int i = 0;
    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;
    

    // sanitize id
    for(i = 0; i < ID3V2_FRAME_ID_MAX_SIZE; i++){
        if(id[i] == '\0'){
            break;
        }
    }

    // find frame
    while((f = id3v2FrameTraverse(&frames)) != NULL){
        if(memcmp(id, f->header->id, i) == 0){
            return id3v2CopyFrame(f);
        }
    }

    return NULL;
}

int id3v2RemoveFrameByID(const char *id, Id3v2Tag *tag){
    
    printf("[*] Attempting to remove frame with id [%s]\n", id);

    Id3v2Frame *test = NULL;
    Id3v2Frame *remove = NULL;

    test = id3v2ReadFrameByID(id, tag);
    
    printf("[*] Tested [%s] and got a frame stored at %p [%s,%s]\n", id, test,id,test->header->id);

    remove = id3v2DetatchFrameFromTag(tag, test);

    printf("[*] %p was returned with test %p as input\n", remove, test);

    id3v2DestroyFrame(&test);

    if(remove != NULL){
        printf("[*] Frame with id [%s] was removed\n", id);
        id3v2DestroyFrame(&remove);
        return 1;
    }
    printf("[*] Failed\n");
    return 0;
}
