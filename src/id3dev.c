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