/**
 * @file id3v2TagIdentity.c
 * @author Ewan Jones
 * @brief Functions related to identifying an id3v2 tag
 * @version 0.1
 * @date 2024-04-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v2/id3v2TagIdentity.h"
#include "id3dependencies/ByteStream/include/byteStream.h"
#include "id3dependencies/ByteStream/include/byteInt.h"

/**
 * @brief Creates an inits a tag header structure.
 * 
 * @param majorVersion 
 * @param minorVersion 
 * @param flags 
 * @param extendedHeader 
 * @return Id3v2TagHeader* 
 */
Id3v2TagHeader *id3v2CreateTagHeader(uint8_t majorVersion, uint8_t minorVersion, uint8_t flags, Id3v2ExtendedTagHeader *extendedHeader){

    Id3v2TagHeader *header = malloc(sizeof(Id3v2TagHeader));

    header->majorVersion = majorVersion;
    header->minorVersion = minorVersion;
    header->flags = flags;
    header->extendedHeader = extendedHeader;

    return header;
}

/**
 * @brief Frees a tag header structure.
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
 * @brief Sets the unsynchronisation indicator found in the header of a tag.
 * @details If this function is successful it will return 1 otherwise, 0.
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
 * @brief Sets the compression indicator in the header of a tag.
 * @details If this function is successful it will return 1 otherwise, 0.
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
 * @brief Sets the extended header indicator in the tag header.
 * @details If this function fails it returns 0 otherwise, it returns 1.
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
            break;
    }

    return 0;
}

/**
 * @brief Sets the experimental indicator in the tag header.
 * @details If this function fails it returns 0 otherwise, 1.
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
            break;
    }

    return 0;
}

/**
 * @brief Sets the footer indicator in the tag header.
 * @details If this function fails it returns 0 otherwise, 1.
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
            break;
    }

    return 0;
}

/**
 * @brief Reads the unsynchronisation indicator from a tag header.
 * @details Returns the indicator value if successful and -1 otherwise.
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
 * @brief Reads the compression indicator found in a version 2 tag header.
 * @details Returns the indicator value if successful and -1 otherwise.
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
 * @brief Reads the extended header indicator found in the tag header.
 * @details Returns the indicator value on success and -1 otherwise.
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
            break;
    }

    return -1;
}

/**
 * @brief Reads the experimental indicator found in the tag header.
 * @details Returns the indicator value on success and -1 otherwise.
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
            break;
    }

    return -1;
}

/**
 * @brief Reads the footer indicator found in the tag header.
 * @details Returns the indicator value on success and -1 otherwise.
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
 * @brief Creates an extended header structure.
 * @details You should never need to use this structure, it will be handled by a header.
 * @param padding
 * @param crc
 * @param update
 * @param tagRestrictions
 * @param restrictions
 * @return Id3v2ExtendedTagHeader*
 */
Id3v2ExtendedTagHeader *id3v2CreateExtendedTagHeader(uint32_t padding, uint32_t crc, bool update, bool tagRestrictions, uint8_t restrictions){

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
 * @brief Frees an extended header structure.
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
 * @brief Writes bits to an extended headers restrictions.
 * @details Returns 1 on success and 0 otherwise.
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

    if(header->majorVersion != ID3V2_TAG_VERSION_4){
        return false;
    }

    if(!header->extendedHeader){
        Id3v2ExtendedTagHeader *extendedHeader = id3v2CreateExtendedTagHeader(0,0,0,0,0);
        header->extendedHeader = extendedHeader;
    }

    // I don't care it will get set every time
    header->extendedHeader->tagRestrictions = true;

    header->extendedHeader->restrictions = setBit(header->extendedHeader->restrictions, 6, readBit(bits, 0));
    header->extendedHeader->restrictions = setBit(header->extendedHeader->restrictions, 7, readBit(bits, 1));

    return true;
}

/**
 * @brief Writes the value of bit to the correct position in the restrictions.
 * @details If this function fails it will return 0 otherwise, 1.
 * @param header
 * @param bit
 * @return true
 * @return false
 */
bool id3v2WriteTextEncodingRestriction(Id3v2TagHeader *header, bool bit){

    if(!header){
        return false;
    }

    if(header->majorVersion != ID3V2_TAG_VERSION_4){
        return false;
    }

    if(!header->extendedHeader){
        Id3v2ExtendedTagHeader *extendedHeader = id3v2CreateExtendedTagHeader(0,0,0,0,0);
        header->extendedHeader = extendedHeader;
    }

    //will set as true every time
    header->extendedHeader->tagRestrictions = true;

    header->extendedHeader->restrictions = setBit(header->extendedHeader->restrictions, 5, bit);

    return true;//avoid warning
}

/**
 * @brief Sets the text fields size restriction within an extended headers bits.
 * @details This function returns 0 if it fails and 1 otherwise.
 * @param header
 * @param bits
 * @return true
 * @return false
 */
bool id3v2WriteTextFieldsSizeRestriction(Id3v2TagHeader *header, uint8_t bits){
    //0b00, 0b01, 0b10, 0b11 making the max option 0x03
    if(!header || bits > 0x03){
        return false;
    }

    if(header->majorVersion != ID3V2_TAG_VERSION_4){
        return false;
    }

    if(!header->extendedHeader){
        Id3v2ExtendedTagHeader *extendedHeader = id3v2CreateExtendedTagHeader(0,0,0,0,0);
        header->extendedHeader = extendedHeader;
    }

    // I don't care it will get set every time
    header->extendedHeader->tagRestrictions = true;

    header->extendedHeader->restrictions = setBit(header->extendedHeader->restrictions, 4, readBit(bits, 1));
    header->extendedHeader->restrictions = setBit(header->extendedHeader->restrictions, 3, readBit(bits, 0));

    return true;
}

/**
 * @brief Sets the image encoding restriction to the value of bit within an extended headers restrictions.
 * @details This function returns 0 if it fails and 1 otherwise.
 * @param header
 * @param bit
 * @return true
 * @return false
 */
bool id3v2WriteImageEncodingRestriction(Id3v2TagHeader *header, bool bit){

    if(!header){
        return false;
    }

    if(header->majorVersion != ID3V2_TAG_VERSION_4){
        return false;
    }

    if(!header->extendedHeader){
        Id3v2ExtendedTagHeader *extendedHeader = id3v2CreateExtendedTagHeader(0,0,0,0,0);
        header->extendedHeader = extendedHeader;
    }

    //will set as true every time
    header->extendedHeader->tagRestrictions = true;

    header->extendedHeader->restrictions = setBit(header->extendedHeader->restrictions, 2, bit);

    return true;//avoid warning
}

/**
 * @brief Sets the imagesize restriction to the value of bits within an extended tag headers restrictions.
 * @details This function returns 0 if it fails and 1 otherwise.
 * @param header
 * @param bits
 * @return true
 * @return false
 */
bool id3v2WriteImageSizeRestriction(Id3v2TagHeader *header, uint8_t bits){
    //0b00, 0b01, 0b10, 0b11 making the max option 0x03
    if(!header || bits > 0x03){
        return false;
    }

    if(header->majorVersion != ID3V2_TAG_VERSION_4){
        return false;
    }

    if(!header->extendedHeader){
        Id3v2ExtendedTagHeader *extendedHeader = id3v2CreateExtendedTagHeader(0,0,0,0,0);
        header->extendedHeader = extendedHeader;
    }

    // I don't care it will get set every time
    header->extendedHeader->tagRestrictions = true;

    header->extendedHeader->restrictions = setBit(header->extendedHeader->restrictions, 1, readBit(bits, 1));
    header->extendedHeader->restrictions = setBit(header->extendedHeader->restrictions, 0, readBit(bits, 0));

    return true;
}

/**
 * @brief Reads the tag size restriction from the restriction byte held within the extended header.
 * @details If this function fails it will return -1 otherwise it will return the value of the tag size restriction.
 * @param header
 * @return int
 */
int id3v2ReadTagSizeRestriction(Id3v2TagHeader *header){

    if(!header){
        return -1;
    }

    if(!header->extendedHeader){
        return -1;
    }

    //build an int from bits
    int lsb = 0;
    int msb = 0;

    msb = readBit(header->extendedHeader->restrictions, 7);
    lsb = readBit(header->extendedHeader->restrictions, 6);

    //combine into an int
    return (msb << 1) | lsb;
}

/**
 * @brief Reads the text encoding restriction from the restriction byte held by the extended header.
 * @details If this function fails it will return -1 otherwise, it will return the value of the indicator.
 * @param header
 * @return int
 */
int id3v2ReadTextEncodingRestriction(Id3v2TagHeader *header){

    if(!header){
        return -1;
    }

    if(!header->extendedHeader){
        return -1;
    }

    return readBit(header->extendedHeader->restrictions, 5);
}

/**
 * @brief Reads the text field size restriction from the restriction byte held by the extended header.
 * @details If this function fails it will return -1 otherwise, it will return the value of the indicator.
 * @param header
 * @return int
 */
int id3v2ReadTextFieldsSizeRestriction(Id3v2TagHeader *header){

    if(!header){
        return -1;
    }

    if(!header->extendedHeader){
        return -1;
    }

    //build an int from bits
    int lsb = 0;
    int msb = 0;

    msb = readBit(header->extendedHeader->restrictions, 4);
    lsb = readBit(header->extendedHeader->restrictions, 3);

    //combine into an int
    return (msb << 1) | lsb;
}

/**
 * @brief Reads the Image Encoding restriction from the restriction byte held by the extended header.
 * @details If this function fails it will return -1 otherwise, it will return the value of the indicator.
 * @param header
 * @return int
 */
int id3v2ReadImageEncodingRestriction(Id3v2TagHeader *header){
    if(!header){
        return -1;
    }

    if(!header->extendedHeader){
        return -1;
    }

    return readBit(header->extendedHeader->restrictions, 2);
}

/**
 * @brief Reads the Image Size restriction from the restriction byte held by the extended header.
 * @details If this function fails it will return -1 otherwise, it will return the value of the indicator.
 * @param header
 * @return int
 */
int id3v2ReadImageSizeRestriction(Id3v2TagHeader *header){
    if(!header){
        return -1;
    }

    if(!header->extendedHeader){
        return -1;
    }

    //build an int from bits
    int lsb = 0;
    int msb = 0;

    lsb = readBit(header->extendedHeader->restrictions, 0);
    msb = readBit(header->extendedHeader->restrictions, 1);

    return (msb << 1) | lsb;
}

/**
 * @brief Clears all restrictions along with the flag in the tag header.
 * @details if this function fails it returns 0 and 1 otherwise.
 * @param header
 * @return true
 * @return false
 */
bool id3v2ClearTagRestrictions(Id3v2TagHeader *header){
    if(!header){
        return 0;
    }

    if(!header->extendedHeader){
        return 0;
    }

    header->extendedHeader->tagRestrictions = false;
    header->extendedHeader->restrictions = 0;

    return 1;
}

/**
 * @brief Creates a tag
 *
 * @param header
 * @param frames
 * @return Id3v2Tag*
 */
Id3v2Tag *id3v2CreateTag(Id3v2TagHeader *header, List *frames){

    Id3v2Tag *tag = malloc(sizeof(Id3v2Tag));

    tag->frames = frames;
    tag->header = header;

    return tag;
}

/**
 * @brief Destroys a tag and its content
 *
 * @param toDelete
 */
void id3v2DestroyTag(Id3v2Tag **toDelete){

    if(*toDelete){

        id3v2DestroyTagHeader(&(*toDelete)->header);
        listFree((*toDelete)->frames);
        free(*toDelete);
        *toDelete = NULL;
        toDelete = NULL;
    }

}

/**
 * @brief Converts an extended tag header into a stream. if this function fails it will
 * return NULL otherwise, an uint8_t pointer.
 *
 * @param ext
 * @param version
 * @param outl
 * @return uint8_t*
 */
uint8_t *id3v2ExtendedTagHeaderSerialize(Id3v2ExtendedTagHeader *ext, uint8_t version, size_t *outl){

    ByteStream *stream = NULL;
    uint8_t *out = NULL;
    int buildSize = 0;
    unsigned char *tmp = NULL;
    unsigned char crcb[5] = {0,0,0,0,0};

    if(ext == NULL){
        *outl = 0;
        return NULL;
    }

    switch(version){

        case ID3V2_TAG_VERSION_3:

            // size
            buildSize = 10 + ((ext->crc) ? 4 : 0);

            stream = byteStreamCreate(NULL, buildSize);

            tmp = (unsigned char *) itob(buildSize);
            byteStreamWrite(stream, tmp, 4);
            free(tmp);

            // flag
            byteStreamWriteBit(stream, (ext->crc > 0) ? 1 : 0, 7);
            byteStreamSeek(stream, 2, SEEK_CUR);

            // crc
            tmp = u32tob(ext->padding);
            byteStreamWrite(stream, tmp, 4);
            free(tmp);

            if(ext->crc){
                tmp = u32tob(ext->crc);
                byteStreamWrite(stream, tmp, 4);
                free(tmp);
            }

            break;

        case ID3V2_TAG_VERSION_4:

            buildSize = 6 + ((ext->crc) ? 5 : 0)  + ((ext->tagRestrictions) ? 1 : 0);

            stream = byteStreamCreate(NULL, buildSize);


            // ext size
            tmp = u32tob(buildSize);
            byteStreamWrite(stream, tmp, 4);
            free(tmp);

            // flag bytes
            tmp = (unsigned char *) itob(buildSize - 6);
            byteStreamWrite(stream, &tmp[3], 1);
            free(tmp);

            // flags
            byteStreamWriteBit(stream, ext->update, 6);
            byteStreamWriteBit(stream, (ext->crc > 0) ? 1 : 0, 5);
            byteStreamWriteBit(stream, ext->tagRestrictions, 4);
            byteStreamSeek(stream, 1, SEEK_CUR);

            // crc
            if(ext->crc){
                int offset = 0;
                int toWrite = 0;
                tmp = sttob(byteSyncintEncode(ext->crc));

                while(offset < sizeof(size_t) && tmp[offset] == 0){
                    offset++;
                }

                // c4c is 5 bytes if the 0s are there they must be kept
                if(offset > 3){
                    offset = 3;
                }

                toWrite = ((int)(sizeof(size_t) - offset) > 5) ? 5 : (int)(sizeof(size_t) - offset);
                memcpy(crcb, tmp + offset, toWrite);

                byteStreamSeek(stream, 6, SEEK_SET);
                byteStreamWrite(stream, crcb, 5);
                free(tmp);
            }

            if(ext->tagRestrictions){
                byteStreamWrite(stream, &ext->restrictions, 1);
            }

            break;

        // no support or it does not exist
        case ID3V2_TAG_VERSION_2:
        default:
            *outl = 0;
            return NULL;
    }

    byteStreamRewind(stream);
    out = calloc(stream->bufferSize, sizeof(uint8_t));
    *outl = stream->bufferSize;
    byteStreamRead(stream, out, stream->bufferSize);
    byteStreamDestroy(stream);

    return out;
}

/**
 * @brief Converts an extended tag header into its structures representation as JSON.
 *
 * @param ext
 * @param version
 * @return char*
 */
char *id3v2ExtendedTagHeaderToJSON(const Id3v2ExtendedTagHeader *ext, uint8_t version){

    char *json = NULL;
    size_t memCount = 3;

    if(ext == NULL){
        json = calloc(memCount, sizeof(char));
        memcpy(json, "{}\0", memCount);
        return json;
    }

    switch(version){
        case ID3V2_TAG_VERSION_3:

            memCount += snprintf(NULL, 0,
                                "{\"padding\":%"PRIu32",\"crc\":%"PRIu32"}",
                                ext->padding,
                                ext->crc);

            json = calloc(memCount + 1, sizeof(char)); // NOLINT(clang-analyzer-unix.Malloc)

            (void) snprintf(json, memCount,
                     "{\"padding\":%"PRIu32",\"crc\":%"PRIu32"}",
                     ext->padding,
                     ext->crc);

            break;
        case ID3V2_TAG_VERSION_4:

            memCount += snprintf(NULL, 0,
                                "{\"padding\":%"PRIu32",\"crc\":%"PRIu32",\"update\":%s,\"tagRestrictions\":%s,\"restrictions\":%d}",
                                ext->padding,
                                ext->crc,
                                ext->update ? "true" : "false",
                                ext->tagRestrictions ? "true" : "false",
                                ext->restrictions);

            json = calloc(memCount + 1, sizeof(char)); // NOLINT(clang-analyzer-unix.Malloc)

            (void) snprintf(json, memCount,
                     "{\"padding\":%"PRIu32",\"crc\":%"PRIu32",\"update\":%s,\"tagRestrictions\":%s,\"restrictions\":%d}",
                     ext->padding,
                     ext->crc,
                     ext->update ? "true" : "false",
                     ext->tagRestrictions ? "true" : "false",
                     ext->restrictions);

            break;

        // no support
        case ID3V2_TAG_VERSION_2:
        default:
            json = malloc(sizeof(char) * memCount); // NOLINT(clang-analyzer-unix.Malloc)
            memcpy(json, "{}\0", memCount);
            break;
    }


    return json;
}

/**
 * @brief Converts a tag header and tag size into an uint8_t pointer. if this function fails it will
 * return NULL otherwise, an uint8_t pointer.
 *
 * @param header
 * @param uintSize
 * @param outl
 * @return uint8_t*
 */
uint8_t *id3v2TagHeaderSerialize(Id3v2TagHeader *header, uint32_t uintSize, size_t *outl){

    ByteStream *stream = NULL;
    uint8_t *out = NULL;
    unsigned char *tmp = NULL;

    if(header == NULL){
        *outl = 0;
        return NULL;
    }

    // no support
    if(header->majorVersion > ID3V2_TAG_VERSION_4){
        *outl = 0;
        return NULL;
    }

    stream = byteStreamCreate(NULL, 10);

    byteStreamWrite(stream, (uint8_t *)"ID3", ID3V2_TAG_ID_SIZE);

    byteStreamWrite(stream, &header->majorVersion, 1);
    byteStreamWrite(stream, &header->minorVersion, 1);


    switch((int) header->majorVersion){

        case ID3V2_TAG_VERSION_2:

            byteStreamWriteBit(stream, (bool)id3v2ReadUnsynchronisationIndicator(header), 7);
            byteStreamWriteBit(stream, (bool)id3v2ReadCompressionIndicator(header), 6);

            break;
        case ID3V2_TAG_VERSION_3:

            byteStreamWriteBit(stream, (bool)id3v2ReadUnsynchronisationIndicator(header), 7);
            byteStreamWriteBit(stream, (bool)id3v2ReadExtendedHeaderIndicator(header), 6);
            byteStreamWriteBit(stream, (bool)id3v2ReadExperimentalIndicator(header), 5);

            break;
        case ID3V2_TAG_VERSION_4:

            byteStreamWriteBit(stream, (bool)id3v2ReadUnsynchronisationIndicator(header), 7);
            byteStreamWriteBit(stream, (bool)id3v2ReadExtendedHeaderIndicator(header), 6);
            byteStreamWriteBit(stream, (bool)id3v2ReadExperimentalIndicator(header), 5);
            byteStreamWriteBit(stream, (bool)id3v2ReadFooterIndicator(header), 4);

            break;
        // dummy break as header version is already checked to be less than 4
        default:
            break;
    }

    byteStreamSeek(stream, 1, SEEK_CUR);

    tmp = u32tob(byteSyncintEncode(uintSize));
    byteStreamWrite(stream, tmp, sizeof(uint32_t));
    free(tmp);

    if(id3v2ReadExtendedHeaderIndicator(header)){
        size_t extSize = 0;
        uint8_t *ext = id3v2ExtendedTagHeaderSerialize(header->extendedHeader, header->majorVersion, &extSize);

        if(ext != NULL){
            byteStreamResize(stream, stream->bufferSize + extSize);
            byteStreamWrite(stream, ext, extSize);
            free(ext);
        }

    }

    byteStreamRewind(stream);
    *outl = stream->bufferSize;
    out = calloc(stream->bufferSize, sizeof(uint8_t));
    byteStreamRead(stream, out, stream->bufferSize);
    byteStreamDestroy(stream);
    return out;
}

char *id3v2TagHeaderToJSON(const Id3v2TagHeader *header){

    char *json = NULL;
    size_t memCount = 3;
    char *extJson = NULL;

    if(header == NULL){
        json = calloc(memCount, sizeof(char));
        memcpy(json, "{}\0", memCount);
        return json;
    }

    switch(header->majorVersion){

        case ID3V2_TAG_VERSION_2:

            memCount += snprintf(NULL, 0,
                                "{\"major\":%d,\"minor\":%d,\"flags\":%d}",
                                header->majorVersion,
                                header->minorVersion,
                                header->flags);

            json = calloc(memCount + 1, sizeof(char));

            (void) snprintf(json, memCount,
                    "{\"major\":%d,\"minor\":%d,\"flags\":%d}",
                    header->majorVersion,
                    header->minorVersion,
                    header->flags);

            break;
        case ID3V2_TAG_VERSION_3:

            extJson = id3v2ExtendedTagHeaderToJSON(header->extendedHeader, ID3V2_TAG_VERSION_3);

            memCount += snprintf(NULL, 0,
                                "{\"major\":%d,\"minor\":%d,\"flags\":%d,\"extended\":%s}",
                                header->majorVersion,
                                header->minorVersion,
                                header->flags,
                                extJson);

            json = calloc(memCount + 1, sizeof(char));


            (void) snprintf(json, memCount,
                    "{\"major\":%d,\"minor\":%d,\"flags\":%d,\"extended\":%s}",
                    header->majorVersion,
                    header->minorVersion,
                    header->flags,
                    extJson);


            free(extJson);
            break;
        case ID3V2_TAG_VERSION_4:
            extJson = id3v2ExtendedTagHeaderToJSON(header->extendedHeader, ID3V2_TAG_VERSION_4);

            memCount += snprintf(NULL, 0,
                                "{\"major\":%d,\"minor\":%d,\"flags\":%d,\"extended\":%s}",
                                header->majorVersion,
                                header->minorVersion,
                                header->flags,
                                extJson);

            json = calloc(memCount + 1, sizeof(char));


            (void) snprintf(json, memCount,
                    "{\"major\":%d,\"minor\":%d,\"flags\":%d,\"extended\":%s}",
                    header->majorVersion,
                    header->minorVersion,
                    header->flags,
                    extJson);


            free(extJson);
            break;

        // no support
        default:
            json = malloc(sizeof(char) * memCount);
            memcpy(json, "{}\0", memCount);
            break;
    }

    return json;
}
