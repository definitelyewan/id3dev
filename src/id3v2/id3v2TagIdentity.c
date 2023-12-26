#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "byteInt.h"
#include "id3v2TagIdentity.h"

/**
 * @brief creates an inits a tag header structure
 * 
 * @param majorVersion 
 * @param minorVersion 
 * @param flags 
 * @param extendedHeader 
 * @return Id3v2TagHeader* 
 */
Id3v2TagHeader *id3v2NewTagHeader(uint8_t majorVersion, uint8_t minorVersion, uint8_t flags, Id3v2ExtendedTagHeader *extendedHeader){

    Id3v2TagHeader *header = malloc(sizeof(Id3v2TagHeader));

    header->majorVersion = majorVersion;
    header->minorVersion = minorVersion;
    header->flags = flags;
    header->extendedHeader = extendedHeader;

    return header;
}

/**
 * @brief frees a tag header structure
 * 
 * @param toDelete 
 */
void id3v2DestroyTagHeader(Id3v2TagHeader **toDelete){

    if(*toDelete){
        id3v2DestroyExtendedTagHeader(&((*toDelete)->extendedHeader));
        free(*toDelete);
        *toDelete = NULL;
        toDelete = NULL;
    }

}

/**
 * @brief sets the unsynchronisation indicator found in the header of a tag
 * @details if this function is successful it will return 1 otherwise, 0
 * @param header 
 * @param bit 
 * @return int 
 */
bool id3v2WriteUnsynchronisationIndicator(Id3v2TagHeader *header, bool bit){

    if(!header){
        return 0;
    }

    //set bit 8
    header->flags = setBit(header->flags, 7, bit);
    return 1;
}

/**
 * @brief sets the compression indicator in the header of a tag
 * @details if this function is successful it will return 1 otherwise, 0
 * @param header 
 * @param bit 
 * @return int 
 */
bool id3v2WriteCompressionIndicator(Id3v2TagHeader *header, bool bit){

    if(!header){
        return 0;
    }

    if(header->majorVersion == ID3V2_TAG_VERSION_2){
        header->flags = setBit(header->flags, 6, bit);
        return 1;
    }

    return 0;
}

/**
 * @brief sets the extended header indicator in the tag header
 * @details if this function fails it returns 0 otherwise, it returns 1
 * @param header 
 * @param bit 
 * @return int 
 */
bool id3v2WriteExtendedHeaderIndicator(Id3v2TagHeader *header, bool bit){
    
    if(!header){
        return 0;
    }

    //set bit 7 or do nothing
    switch(header->majorVersion){
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            header->flags = setBit(header->flags, 6, bit);
            return 1;
        default:
            return 0;
    }

    return 0;
}

/**
 * @brief sets the experimental indicator in the tag header
 * @details if this function fails it returns 0 otherwise, 1
 * @param header 
 * @param bit 
 * @return int 
 */
bool id3v2WriteExperimentalIndicator(Id3v2TagHeader *header, bool bit){

    if(!header){
        return 0;
    }

    //set bit 6 or do nothing
    switch(header->majorVersion){
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            header->flags = setBit(header->flags, 5, bit);
            return 1;
        default:
            return 0;
    }

    return 0;
}

/**
 * @brief sets the footer indicator in the tag header
 * @details if this function fails it returns 0 otherwise, 1
 * @param header 
 * @param bit 
 * @return int 
 */
bool id3v2WriteFooterIndicator(Id3v2TagHeader *header, bool bit){
    
    if(!header){
        return 0;
    }

    switch(header->majorVersion){
        case ID3V2_TAG_VERSION_4:
            header->flags = setBit(header->flags, 4, bit);
            return 1;
        default:
            return 0;
    }

    return 0;
}

/**
 * @brief reads the unsynchronisation indicator from a tag header
 * @details returns the indicator value if successful and -1 otherwise
 * @param header 
 * @return int 
 */
int id3v2ReadUnsynchronisationIndicator(Id3v2TagHeader *header){
    
    if(!header){
        return -1;
    }

    return readBit(header->flags, 7);
}

/**
 * @brief reads the compression indicator found in a version 2 tag header
 * @details returns the indicator value if successfull and -1 otherwise
 * @param header 
 * @return int 
 */
int id3v2ReadCompressionIndicator(Id3v2TagHeader *header){

    if(!header){
        return -1;
    }

    if(header->majorVersion != ID3V2_TAG_VERSION_2){
        return -1;
    }

    return readBit(header->flags, 6);
}

/**
 * @brief reads the extended header indicator found in the tag header
 * @details returns the indicator value on success and -1 otherwise
 * @param header 
 * @return int 
 */
int id3v2ReadExtendedHeaderIndicator(Id3v2TagHeader *header){

    if(!header){
        return -1;
    }

    switch(header->majorVersion){
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return readBit(header->flags, 6);
        default:
            return -1;
    }

    return -1;
}

/**
 * @brief reads the experimental indicator found in the tag header
 * @details returns the indicator value on success and -1 otherwise
 * @param header 
 * @return int 
 */
int id3v2ReadExperimentalIndicator(Id3v2TagHeader *header){

    if(!header){
        return -1;
    }

    switch(header->majorVersion){
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return readBit(header->flags, 5);
        default:
            return -1;
    }

    return -1;
}

/**
 * @brief reads the footer indicator found in the tag header
 * @details returns the indicator value on success and -1 otherwise
 * @param header 
 * @return int 
 */
int id3v2ReadFooterIndicator(Id3v2TagHeader *header){

    if(!header){
        return -1;
    }

    if(header->majorVersion != ID3V2_TAG_VERSION_4){
        return -1;
    }

    return readBit(header->flags, 4);
}

/**
 * @brief creates an extended header strcuture
 * @details you should never need to use this structure, it will be handled by a header
 * @param padding 
 * @param crc 
 * @param update 
 * @param tagRestrictions 
 * @param restrictions 
 * @return Id3v2ExtendedTagHeader* 
 */
Id3v2ExtendedTagHeader *id3v2NewExtendedTagHeader(uint32_t padding, uint32_t crc, bool update, bool tagRestrictions, uint8_t restrictions){

    Id3v2ExtendedTagHeader *extendedHeader = malloc(sizeof(Id3v2ExtendedTagHeader));

    memset(extendedHeader, 0, sizeof(Id3v2ExtendedTagHeader));

    extendedHeader->padding = padding;
    extendedHeader->crc = crc;
    extendedHeader->update = update;
    extendedHeader->tagRestrictions = tagRestrictions;
    extendedHeader->restrictions = restrictions;

    return extendedHeader;
}

/**
 * @brief frees an extended header structure
 * @param toDelete 
 */
void id3v2DestroyExtendedTagHeader(Id3v2ExtendedTagHeader **toDelete){
    //error address free
    if(*toDelete){
        free(*toDelete);
        *toDelete = NULL;
        toDelete = NULL;
    }
}

/**
 * @brief writes bits to an extended headers restrictions
 * @details returns 1 on success and 0 otherwise
 * @param header 
 * @param bits 
 * @return true 
 * @return false 
 */
bool id3v2WriteTagSizeRestriction(Id3v2TagHeader *header, uint8_t bits){
    //0b00, 0b01, 0b10, 0b11 making the max option 0x03
    if(!header || bits > 0x03){
        return false;
    }

    if(header->majorVersion != ID3V2_TAG_VERSION_4 && readBit(header->flags, 6) != 1){
        return false;
    }

    if(!header->extendedHeader){
        Id3v2ExtendedTagHeader *extendedHeader = id3v2NewExtendedTagHeader(0,0,0,0,0);
        header->extendedHeader = extendedHeader;
    }

    //i dont care it will get set every time
    header->extendedHeader->tagRestrictions = true;

    header->extendedHeader->restrictions = setBit(header->extendedHeader->restrictions, 6, readBit(bits, 0));
    header->extendedHeader->restrictions = setBit(header->extendedHeader->restrictions, 7, readBit(bits, 1));

    return true;
}

bool id3v2WriteTextEncodingRestriction(Id3v2TagHeader *header, bool bit){

    if(!header){
        return false;
    }

    if(header->majorVersion != ID3V2_TAG_VERSION_4){
        return false;
    }

    if(!header->extendedHeader){
        Id3v2ExtendedTagHeader *extendedHeader = id3v2NewExtendedTagHeader(0,0,0,0,0);
        header->extendedHeader = extendedHeader;
    }

    header->extendedHeader->tagRestrictions = true;

    header->extendedHeader->restrictions = setBit(header->extendedHeader->restrictions, 5, bit);

    return 0;//avoid warning
}