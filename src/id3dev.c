/**
 * @file id3dev.c
 * @author Ewan Jones
 * @brief Main functions for interacting with ID3 metadata
 * @version 0.1
 * @date 2024-04-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3dev.h"
#include "id3v2/id3v2.h"
#include "id3v1/id3v1.h"
#include "id3v1/id3v1Parser.h"
#include "id3v2/id3v2Frame.h"

/**
 * @brief default standard for reading ID3 tags from a data structure representation.
 * 
 */
static uint8_t id3PreferedStandard = ID3V2_TAG_VERSION_3;

/**
 * @brief Creates a new ID3 structure with the given ID3v2 and ID3v1 tags.
 * 
 * @param id3v2 
 * @param id3v1 
 * @return ID3* 
 */
ID3 *id3Create(Id3v2Tag *id3v2, Id3v1Tag *id3v1){

    ID3 *metadata = malloc(sizeof(ID3));

    metadata->id3v2 = id3v2;
    metadata->id3v1 = id3v1;

    return metadata;

}

/**
 * @brief Destroys an ID3 structure and frees its memory.
 * 
 * @param toDelete 
 */
void id3Destroy(ID3 **toDelete){

    if(*toDelete){
        id3v2DestroyTag(&((*toDelete)->id3v2));
        id3v1DestroyTag(&((*toDelete)->id3v1));
        free(*toDelete);
        *toDelete = NULL;
        toDelete = NULL;
    }

}

/**
 * @brief Sets the prefered standard for reading ID3 tags from a data structure representation. If no standard is 
 * set the default will be set to ID3v2.3 as this is the most wide spread version with the most features. If
 * sucessful this function returns true otherwise, it will return false.
 * 
 * @param standard 
 * @return true 
 * @return false 
 */
bool id3SetPreferedStandard(uint8_t standard){

    switch(standard){
        case ID3V1_TAG_VERSION:
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            id3PreferedStandard = standard;
            break;
        
        default:
            return false;
    }

    return true;
}

/**
 * @brief Returns the current prefered standard
 * 
 * @return uint8_t 
 */
uint8_t id3GetPreferedStandard(void){
    return id3PreferedStandard;
}


/**
 * @brief Reads both ID3v1 and ID3v2 tags located in a provided file. If this function fails an
 * ID3 strcuture will be returned but one or both versions will be set to NULL.
 * 
 * @param filePath 
 * @return ID3* 
 */
ID3 *id3FromFile(const char *filePath){
    return id3Create(id3v2TagFromFile(filePath), id3v1TagFromFile(filePath));
}

/**
 * @brief Returns a copy of the given ID3 structure. If NUll is passed NULL is returned.
 * 
 * @param toCopy 
 * @return ID3* 
 */
ID3 *id3Copy(ID3 *toCopy){

    if(toCopy == NULL){
        return NULL;
    }

    return id3Create(id3v2CopyTag(toCopy->id3v2), id3v1CopyTag(toCopy->id3v1));
}

/**
 * @brief Compares two ID3 structures. If they are the same this function will return true otherwise it will return false.
 * 
 * @param metadata1 
 * @param metadata2 
 * @return true 
 * @return false 
 */
bool id3Compare(ID3 *metadata1, ID3 *metadata2){

    if(metadata1 == NULL || metadata2 == NULL){
        return false;
    }

    bool v1 = false;
    bool v2 = false;

    v1 = id3v1CompareTag(metadata1->id3v1, metadata2->id3v1);
    v2 = id3v2CompareTag(metadata1->id3v2, metadata2->id3v2);

    if(v1 && v2){
        return true;
    }

    if(v1 == false && (v2 == true && metadata1->id3v1 == NULL && metadata2->id3v1 == NULL)){
        return true;

    }

    if(v2 == false && (v1 == true && metadata1->id3v2 == NULL && metadata2->id3v2 == NULL)){
        return true;

    }

    return false;
}


/**
 * @brief Converts the stored ID3v1 tag to an ID3v2 tag to be held in the ID3 structre. However, if there
 * there is a stored ID3v2 tag it will be removed and replaced. On success this function will return true,
 * otherwise it will return false. 
 * 
 * @param metadata 
 * @return true 
 * @return false 
 */
bool id3ConvertId3v1ToId3v2(ID3 *metadata){
    printf("[*] Entered id3ConvertId3v1ToId3v2\n");
    if(metadata == NULL){
        return false;
    }

    if(metadata->id3v1 == NULL){
        return false;
    }

    printf("[*] Args are valid\n");

    Id3v2Tag *newTag = NULL;
    Id3v2TagHeader *header = NULL;
    List *frames = NULL;
    char *str = NULL;
    int size = 0;
    int ret = 0;

    switch(id3GetPreferedStandard()){
        case ID3V2_TAG_VERSION_2:
            header = id3v2CreateTagHeader(ID3V2_TAG_VERSION_2, 0, 0, NULL);
            break;
        case ID3V2_TAG_VERSION_3:
            header = id3v2CreateTagHeader(ID3V2_TAG_VERSION_3, 0, 0, NULL);
            break;
        case ID3V2_TAG_VERSION_4:
            header = id3v2CreateTagHeader(ID3V2_TAG_VERSION_4, 0, 0, NULL);
            break;
        default:
            return false;
    }

    printf("[*] Created a header\n");

    frames = listCreate(id3v2PrintFrame, id3v2DeleteFrame, id3v2CompareFrame, id3v2CopyFrame);
    newTag = id3v2CreateTag(header, frames);


    printf("[*] Set up tag\n");

    if(metadata->id3v1->title[0] != 0x00){
        if(!id3v2WriteTitle((char *)metadata->id3v1->title, newTag)){
            id3v2DestroyTag(&newTag);
            listFree(frames);
            return false;
        }
        printf("[*] Wrote Title\n");
    }

    

    if(metadata->id3v1->artist[0] != 0x00){
        if(!id3v2WriteArtist((char *)metadata->id3v1->artist, newTag)){
            id3v2DestroyTag(&newTag);
            listFree(frames);
            return false;
        }
        printf("[*] Wrote Artist\n");
    }

    

    if(metadata->id3v1->albumTitle[0] != 0x00){
        if(!id3v2WriteAlbum((char *)metadata->id3v1->albumTitle, newTag)){
            id3v2DestroyTag(&newTag);
            listFree(frames);
            return false;
        }
        printf("[*] Wrote Album\n");
    }
    
    

    if(metadata->id3v1->year != 0){
        size = snprintf(NULL, 0, "%d", metadata->id3v1->year);
        str = calloc(size + 1, sizeof(char));
        snprintf(str, size + 1, "%d", metadata->id3v1->year);
        if(!id3v2WriteYear((char *)str, newTag)){
            id3v2DestroyTag(&newTag);
            listFree(frames);
            free(str);
            return false;
        }
        free(str);
        printf("[*] Wrote Year\n");
    }

    

    if(metadata->id3v1->track != 0){
        size = snprintf(NULL, 0, "%d", metadata->id3v1->track);
        str = calloc(size + 1, sizeof(char));
        snprintf(str, size + 1, "%d", metadata->id3v1->track);

        printf("[*] Track: %s\n", str);

        if(!id3v2WriteTrack((char *)str, newTag)){
            id3v2DestroyTag(&newTag);
            listFree(frames);
            free(str);
            return false;
        }
        free(str);
        printf("[*] Wrote Track\n");
    }

    
    printf("[*] Genre: %d\n", metadata->id3v1->genre);
    if(metadata->id3v1->genre < PSYBIENT_GENRE){
        if(!id3v2WriteGenre(id3v1GenreFromTable(metadata->id3v1->genre), newTag)){
            id3v2DestroyTag(&newTag);
            listFree(frames);
            return false;
        }
        printf("[*] Wrote Genre\n");
    }

    

    if(metadata->id3v1->comment[0] != 0x00){
        if(!id3v2WriteComment((char *)metadata->id3v1->comment, newTag)){
            id3v2DestroyTag(&newTag);
            listFree(frames);
            return false;
        }
        printf("[*] Wrote Comment\n");
    }

    

    if(metadata->id3v2 != NULL){
        id3v2DestroyTag(&(metadata->id3v2));
    }

    metadata->id3v2 = newTag;
    
    return true;
}
