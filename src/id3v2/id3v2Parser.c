

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v2Context.h"
#include "id3v2Parser.h"
#include "id3v2TagIdentity.h"
#include "byteStream.h"
#include "byteInt.h"

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
    uint8_t nBytes = 0;
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
            }

            if(!(byteStreamSeek(innerStream, 1, SEEK_CUR))){
                break;
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

Id3v2TagHeader *id3v2ParseTagHeader(ByteStream *stream, uint32_t *tagBodySize){

    if(!stream){
        *tagBodySize = 0;
        return NULL;
    }

    uint8_t magicNumber[ID3V2_TAG_ID_SIZE];
    uint8_t major = 0;
    uint8_t minor = 0;
    uint8_t flags = 0;
    Id3v2ExtendedTagHeader *ext = NULL;

    if(!byteStreamRead(stream, magicNumber, ID3V2_TAG_ID_SIZE)){
        *tagBodySize = 0;
        return NULL;
    }

    if(memcmp(magicNumber, "ID3", ID3V2_TAG_ID_SIZE) != 0){
        *tagBodySize = 0;
        return NULL;
    }

    if(!byteStreamRead(stream, major, 1)){
        *tagBodySize = 0;
        return NULL;
    }

    if(!byteStreamRead(stream, minor, 1)){
        *tagBodySize = 0;
        return NULL;
    }


    switch(major){
        case ID3V2_TAG_VERSION_2:
            break;
        case ID3V2_TAG_VERSION_3:
            break;
        case ID3V2_TAG_VERSION_4:
            break;
        default:
            break;   
    }

    return id3v2CreateTagHeader(major, minor, flags, ext);
}
