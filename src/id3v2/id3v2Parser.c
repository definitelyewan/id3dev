/**
 * @file id3v2Parser.c
 * @author Ewan Jones
 * @brief Functions related to parsing content
 * @version 0.1
 * @date 2024-02-01
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v2Context.h"
#include "id3v2Parser.h"
#include "id3v2TagIdentity.h"
#include "byteStream.h"
#include "byteInt.h"

/**
 * @brief Parses an ID3v2.3, ID3v2.4, and unsupported versions. This function will return the number of bytes it read
 * to correctly parse an extended header and a heap stored structure through extendedTagHeader. There are no error
 * states for this function but if a size of 0 is returned or extendedTagHeader = NULL it likly failed but this could 
 * also mean an unsupported version was passed. 
 * 
 * @param stream 
 * @param version 
 * @param extendedTagHeader 
 * @return uint32_t 
 */
uint32_t id3v2ParseExtendedTagHeader(ByteStream *stream, uint8_t version, Id3v2ExtendedTagHeader **extendedTagHeader){

    if(!stream){
        *extendedTagHeader = NULL;
        return 0;
    }

    // values needed
    uint32_t padding = 0;
    uint32_t crc = 0;
    bool update = false;
    bool tagRestrictions = false;
    uint8_t restrictions = 0;

    // values for parsing
    int8_t flags = 0;
    size_t offset = 0;
    size_t resetIndex = 0;
    unsigned char crcBytes[5] = {0,0,0,0,0};
    bool hasCrc = false;
    bool hasRestrictions = false;
    ByteStream *innerStream = NULL;
    
    // size return
    uint32_t hSize = 0;
    uint32_t walk = 0;

    if(version == ID3V2_TAG_VERSION_3 || version == ID3V2_TAG_VERSION_4){

        resetIndex = stream->cursor;

        if(!(hSize = byteStreamReturnInt(stream))){
            stream->cursor = resetIndex;
            *extendedTagHeader = NULL;
            return 0;
        }

        if(hSize > stream->bufferSize){
            offset = hSize - stream->bufferSize;
        }
        
        innerStream = byteStreamCreate(byteStreamCursor(stream), hSize - offset);
        stream->cursor = resetIndex;
    }

    switch(version){
        case ID3V2_TAG_VERSION_3:

            // read flags
            if((flags = byteStreamReadBit(innerStream, 7)) == -1){
                break;
            }

            // skip over flags
            if(!(byteStreamSeek(innerStream, 2, SEEK_CUR))){
                break;
            }

            // read padding
            if(!(padding = byteStreamReturnInt(innerStream))){
                break;
            }

            // check to see if a crc is there
            if(flags){
                if(!(crc = byteStreamReturnInt(innerStream))){
                    break;
                }
            }

            break;
        
        case ID3V2_TAG_VERSION_4:  

            // number of flag bytes
            if(!(byteStreamSeek(innerStream, 1, SEEK_CUR))){
                break;
            }

            // read flags
            if(byteStreamReadBit(innerStream, 6)){
                update = true;
            }

            if(byteStreamReadBit(innerStream, 5)){
                hasCrc = true;
            }

            if(byteStreamReadBit(innerStream, 4)){
                hasRestrictions = true;
            }

            if(!(byteStreamSeek(innerStream, 1, SEEK_CUR))){
                break;
            }

            // read crc
            if(hasCrc){
                if(!(byteStreamRead(innerStream, (uint8_t *)crcBytes, 5))){
                    break;
                }

                crc = byteSyncintDecode(btoi(crcBytes, 5));

            }

            // read restrictions
            if(hasRestrictions){
                for(uint8_t k = 8; k > 0; k--){
                    
                    int bit = byteStreamReadBit(innerStream, k - 1);
                    
                    if(bit == -1){
                        break;
                    }

                    restrictions = setBit(restrictions, k - 1, (bool) bit);
                }

                if(restrictions > 0){
                    tagRestrictions = true;
                }
                
                if(!(byteStreamSeek(innerStream, 1, SEEK_CUR))){
                    break;
                }            
            }

            break;

        // no support
        default:
            *extendedTagHeader = NULL;
            return 0;
    }

    // clean up
    *extendedTagHeader = id3v2CreateExtendedTagHeader(padding, crc, update, tagRestrictions, restrictions);
    walk = innerStream->cursor + 4;

    if(innerStream != NULL){
        byteStreamDestroy(innerStream);
    }

    return walk; 
}

/**
 * @brief Parses an ID3 header but, not its extended header. This function will return the number
 * of bytes it read in order to correctly parse a tag header. the size of tag itself and header 
 * are returned by referance. There are no error states but a tagSize of 0 and a NULL header 
 * it likely faild to retrive anything useful.
 * 
 * @param stream 
 * @param tagHeader 
 * @param tagSize 
 * @return uint32_t 
 */
uint32_t id3v2ParseTagHeader(ByteStream *stream, Id3v2TagHeader **tagHeader, uint32_t *tagSize){

    if(!stream){
        *tagHeader = NULL;
        return 0;
    }

    // values needed
    uint8_t major = 0;
    uint8_t minor = 0;
    uint8_t flags = 0;

    // values for parsing
    size_t resetIndex = 0;
    uint32_t hSize = 0;
    uint32_t walk = 0;
    uint8_t id[ID3V2_TAG_ID_SIZE];
    uint32_t idAsInt = 0;

    // check for magic number
    resetIndex = stream->cursor;
    if(!(byteStreamRead(stream, id, ID3V2_TAG_ID_SIZE))){
        stream->cursor = resetIndex;
        *tagHeader = NULL;
        return 0;
    }
        
    for(int i = 0; i < ID3V2_TAG_ID_SIZE; ++i) {
        idAsInt = (idAsInt << 8) | id[i];
    }

    if(ID3V2_TAG_ID_MAGIC_NUMBER_H != idAsInt){
        stream->cursor = resetIndex;
        *tagHeader = NULL;
        return ID3V2_TAG_ID_SIZE;
    }


    while(true){

        // versions
        if(!(byteStreamRead(stream, &major, 1))){
            break;
        }

        if(!(byteStreamRead(stream, &minor, 1))){
            break;
        }

        // flags
        if(!(byteStreamRead(stream, &flags, 1))){
            break;
        }

        // size
        if(!(hSize = byteStreamReturnSyncInt(stream))){
            break;
        }

        break;
    }


    walk = stream->cursor;
    stream->cursor = resetIndex;
    *tagSize = hSize;
    *tagHeader = id3v2CreateTagHeader(major, minor, flags, NULL);
    return walk;
}


