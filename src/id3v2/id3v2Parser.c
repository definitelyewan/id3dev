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
#include "id3v2Frame.h"
#include "id3v2Parser.h"
#include "id3v2TagIdentity.h"
#include "byteStream.h"
#include "byteInt.h"
#include "byteUnicode.h"

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

        if(hSize > (stream->bufferSize - stream->cursor)){
            offset = hSize - (stream->bufferSize - stream->cursor);
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

    walk = stream->cursor - resetIndex;
    stream->cursor = resetIndex;
    *tagSize = hSize;
    *tagHeader = id3v2CreateTagHeader(major, minor, flags, NULL);
    return walk;
}
/**
 * @brief Parses an ID3 version 2, 3 or 4 frame header. This function returns a couple values back to the caller, first
 * it returns the number of bytes it read to to create a frame header structure. The structure itself is returned as a
 * heap allocated structure which will need to be provided by the caller along with the size reported by the metadata.
 * There are no error states but a frameSize of 0 and a NULL frameHeader will likely mean nothing useful was gathered. 
 * 
 * @param stream 
 * @param version 
 * @param frameHeader 
 * @param frameSize 
 * @return uint32_t 
 */
uint32_t id3v2ParseFrameHeader(ByteStream *stream, uint8_t version, Id3v2FrameHeader **frameHeader, uint32_t *frameSize){

    if(!stream){
        *frameHeader = NULL;
        return 0;
    }

    // values needed
    uint8_t id[ID3V2_FRAME_ID_MAX_SIZE];
    uint32_t tSize = 0;
    bool tagAlter = false;
    bool fileAlter = false;
    bool readOnly = false;
    bool unsync = false;
    uint32_t decompressionSize = 0;
    uint8_t encryptionSymbol = 0;
    uint8_t groupSymbol = 0;

    // values for parsing
    uint8_t sizeBytes[ID3V2_FRAME_ID_MAX_SIZE]; 
    uint8_t flagBytes[ID3V2_FRAME_FLAG_SIZE];
    uint8_t flagOffset = 0;
    size_t resetIndex = 0;
    uint32_t walk = 0;
    


    resetIndex = stream->cursor;

    switch(version){
        case ID3V2_TAG_VERSION_2:

            if(!byteStreamRead(stream, id, ID3V2_FRAME_ID_MAX_SIZE - 1)){
                *frameHeader = NULL;
                *frameSize = 0;
                return 0;
            }

            if(!byteStreamRead(stream, sizeBytes, ID3V2_FRAME_ID_MAX_SIZE - 1)){
                *frameHeader = NULL;
                *frameSize = 0;
                return ID3V2_FRAME_ID_MAX_SIZE - 1;
            }

            if(!(tSize = btoi(sizeBytes, ID3V2_FRAME_ID_MAX_SIZE - 1))){
                break;
            }

            break;
        case ID3V2_TAG_VERSION_3:
            if(!byteStreamRead(stream, id, ID3V2_FRAME_ID_MAX_SIZE)){
                *frameHeader = NULL;
                *frameSize = 0;
                return 0;
            }

            if(!(tSize = byteStreamReturnInt(stream))){
                *frameHeader = NULL;
                *frameSize = 0;
                return ID3V2_FRAME_ID_MAX_SIZE;
            }

            if(!(byteStreamRead(stream, flagBytes, ID3V2_FRAME_FLAG_SIZE))){
                *frameHeader = NULL;
                *frameSize = tSize;
                return ID3V2_FRAME_ID_MAX_SIZE * 2;
            }

            if(readBit(flagBytes[0], 7)){
                tagAlter = true;
            }

            if(readBit(flagBytes[0], 6)){
                fileAlter = true;
            }

            if(readBit(flagBytes[0], 5)){
                readOnly = true;
            }

            if(readBit(flagBytes[1], 7)){
                if(!(decompressionSize = byteStreamReturnInt(stream))){
                    break;
                }

                flagOffset += sizeof(uint32_t);
            }

            if(readBit(flagBytes[1], 6)){
                if(!(byteStreamRead(stream, &encryptionSymbol, 1))){
                    break;
                }

                flagOffset++;
            }

            if(readBit(flagBytes[1], 5)){
                if(!(byteStreamRead(stream, &groupSymbol, 1))){
                    break;
                }

                flagOffset++;
            }

            break;
        case ID3V2_TAG_VERSION_4:

            if(!byteStreamRead(stream, id, ID3V2_FRAME_ID_MAX_SIZE)){
                *frameHeader = NULL;
                *frameSize = 0;
                return 0;
            }

            if(!(tSize = byteStreamReturnSyncInt(stream))){
                *frameHeader = NULL;
                *frameSize = 0;
                return ID3V2_FRAME_ID_MAX_SIZE;
            }

            if(!(byteStreamRead(stream, flagBytes, ID3V2_FRAME_FLAG_SIZE))){
                *frameHeader = NULL;
                *frameSize = tSize;
                return ID3V2_FRAME_ID_MAX_SIZE * 2;
            }

            if(readBit(flagBytes[0], 6)){
                tagAlter = true;
            }

            if(readBit(flagBytes[0], 5)){
                fileAlter = true;
            }

            if(readBit(flagBytes[0], 4)){
                readOnly = true;
            }

            if(readBit(flagBytes[1], 6)){
                if(!byteStreamRead(stream, &groupSymbol, 1)){
                    break;
                }

                flagOffset++;
            }

            if(readBit(flagBytes[1], 2)){
                if(!byteStreamRead(stream, &encryptionSymbol, 1)){
                    break;
                }

                flagOffset++;
            }

            if(readBit(flagBytes[1], 1)){
                unsync = true;
            }

            if(readBit(flagBytes[1], 3) || encryptionSymbol || readBit(flagBytes[1], 0)){
                decompressionSize = byteStreamReturnSyncInt(stream);

                flagOffset += sizeof(uint32_t);
            }

            break;
        
        // no support
        default:
            *frameHeader = NULL;
            *frameSize = 0;
            return 0;
    }

    *frameHeader = id3v2CreateFrameHeader(id, tagAlter, fileAlter, readOnly, unsync, decompressionSize, encryptionSymbol, groupSymbol);
    *frameSize = tSize - flagOffset;
    walk = stream->cursor - resetIndex;
    stream->cursor = resetIndex;
    return walk;
}

uint32_t id3v2ParseFrame(ByteStream *stream, List *context, uint8_t version, Id3v2Frame **frame){


    printf("[*] attempting to parse provided frame\n");

    if(!stream || !context){
        *frame = NULL;
        return 0;
    }

    printf("[*] provided content is valid\n");
    byteStreamPrintf("%x", stream);
    // values needed
    Id3v2FrameHeader *header = NULL;
    List *entries = NULL;
    *frame = NULL;

    // needed for parsing
    size_t resetIndex = 0;
    size_t walk = 0;
    uint32_t expectedHeaderSize = 0;
    uint32_t expectedContentSize = 0;
    ByteStream *innerSream = NULL;
    ListIter iter;
    void *contextData = NULL;

    resetIndex = stream->cursor;

    expectedHeaderSize = id3v2ParseFrameHeader(stream, version, &header, &expectedContentSize);
    walk += expectedHeaderSize;

    if(!expectedHeaderSize){
        stream->cursor = resetIndex;
        return 0;
    }

    if(!header || !expectedContentSize){
        stream->cursor = resetIndex;
        return expectedHeaderSize;
    }
    
    printf("[*] parsed a frame header with a size of %d and a id of %c%c%c%c\n",expectedHeaderSize,header->id[0],header->id[1], header->id[2],header->id[3]);

    byteStreamSeek(stream, expectedHeaderSize, SEEK_CUR);

    printf("[*] skipping header bytes\n");
    byteStreamPrintf("%x", stream);


    innerSream = byteStreamCreate(byteStreamCursor(stream), expectedContentSize);
    stream->cursor = resetIndex;

    printf("[*] created inner stream container with size %d\n", expectedContentSize);
    byteStreamPrintf("%x",innerSream);

    entries = listCreate(id3v2PrintContentEntry, id3v2DeleteContentEntry, id3v2CompareContentEntry, id3v2CopyContentEntry);
    iter = listCreateIterator(context);

    while((contextData = listIteratorNext(&iter)) != NULL){

        Id3v2ContentContext *cc = (Id3v2ContentContext *) contextData;

        switch(cc->type){
            // give up on parsing when the context is undefined
            case unknown_context:
                printf("[*] detected unknown_context");
                byteStreamSeek(innerSream, 0, SEEK_END);
                break;
            case noEncoding_context:
                break;
            case binary_context:
                break;
            
            // encoded strings
            case encodedString_context:{
                printf("[*] detected encodedString_context\n");
                void *data = NULL;
                size_t dataSize = 0;

                size_t posce = 0;
                size_t poscc = 0;
                uint8_t encoding = 0;
                ListIter contentContextIter = listCreateIterator(context); 
                ListIter contentEntryIter = listCreateIterator(entries);
                void *tmp = NULL;

                while((tmp = listIteratorNext(&contentContextIter)) != NULL){

                    if(((Id3v2ContentContext *)tmp)->type == iter_context){
                        printf("[*] detected an iter, skipping and reducing position by 1\n");
                        poscc--;
                    }

                    if(((Id3v2ContentContext *)tmp)->key == id3v2djb2("encoding")){
                        printf("[*] encoding found in contexts at position %ld\n",poscc);
                        break;
                    }

                    poscc++;
                }

                while((tmp = listIteratorNext(&contentEntryIter)) != NULL){

                    if(poscc == posce){
                        encoding = ((uint8_t *)((Id3v2ContentEntry *)tmp)->entry)[0];
                        printf("[*] detected encoding in content entries at %ld position with the value %d\n", posce, encoding);
                    }

                    posce++;
                }
                byteStreamPrintf("%x", innerSream);
                switch(encoding){
                    case BYTE_ISO_8859_1:
                    case BYTE_ASCII:
                    case BYTE_UTF8:
                        data = byteStreamReturnUtf8(innerSream, &dataSize);
                        break;
                    case BYTE_UTF16BE:
                    case BYTE_UTF16LE:
                        data = byteStreamReturnUtf16(innerSream, &dataSize);
                        break;
                    default:
                        break;

                }
                printf("[*] string is %ld bytes long and is at address %p\n",dataSize, data);
                printf("[*] encoded string context = {");
                for(int i = 0; i < dataSize; i++){
                    printf("%x,",((uint8_t *)data)[i]);
                }
                printf("}\n");
                byteStreamPrintf("%x", innerSream);
                listInsertBack(entries, id3v2CreateContentEntry(data, dataSize));
                free(data);
                expectedContentSize = expectedContentSize - dataSize;
            }
                break;      
            case latin1Encoding_context:
                break;
            // numbers
            case numeric_context:{
                printf("[*] detected a numeric_context\n");
                byteStreamPrintf("%x",innerSream);

                void * data = NULL;
                size_t dataSize = 0;

                if(cc->min == cc->max){
                    dataSize = cc->min;
                }else if(cc->min > cc->max){// no trust
                    dataSize = cc->min;
                }else{
                    dataSize = cc->max;
                }

                if(dataSize > expectedContentSize){
                    dataSize = expectedContentSize;
                }
                printf("[*] size of the number is %ld bytes\n", dataSize);
                data = malloc(dataSize);

                if(!byteStreamRead(innerSream, (uint8_t *)data, dataSize)){
                    memset(data, 0, dataSize);
                }

                printf("[*] numeric content = {");
                for(int i = 0; i < dataSize; i++){
                    printf("%x,",((uint8_t *)data)[i]);
                }
                printf("}\n");

                byteStreamPrintf("%x", innerSream);

                listInsertBack(entries, id3v2CreateContentEntry(data, dataSize));
                free(data);
                expectedContentSize = expectedContentSize - dataSize;
            }
                break;
            case precision_context:
                break;
            case bit_context:
                break;
            case iter_context:
                break;
            case adjustment_context:
                break;
            // no support
            default:
                break;
        }

        if(expectedContentSize == 0){
            break;
        }

    }
    printf("[*]exiting\n");
    walk += innerSream->cursor;
    *frame = id3v2CreateFrame(header, listDeepCopy(context), entries);
    byteStreamDestroy(innerSream);
    return walk;
}