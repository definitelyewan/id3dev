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
#include <limits.h>
#include "id3v2/id3v2Context.h"
#include "id3v2/id3v2Frame.h"
#include "id3v2/id3v2Parser.h"
#include "id3v2/id3v2TagIdentity.h"
#include "id3dependencies/ByteStream/include/byteStream.h"
#include "id3dependencies/ByteStream/include/byteInt.h"
#include "id3dependencies/ByteStream/include/byteUnicode.h"

static void copyNBits(unsigned char* src, unsigned char* dest, int startBit, int nBits) {
    int byteIndex = startBit /  CHAR_BIT;
    int bitIndex = startBit %  CHAR_BIT;
    int remainingBits = nBits;

    while(remainingBits >  0){
        unsigned char mask =  0xFF >> (CHAR_BIT - remainingBits);
        unsigned char shiftedMask = mask << bitIndex;

        // clear the destination bits
        dest[byteIndex] &= ~shiftedMask;

        // copy the bits from source to destination
        dest[byteIndex] |= (src[byteIndex] & shiftedMask) >> bitIndex;

        // update the counters
        remainingBits -= (CHAR_BIT - bitIndex);
        byteIndex++;
        bitIndex = 0;
    }
}

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
                if(flags == 0){
                    break;
                }   
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

                tagRestrictions = true;
                
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
 * means it likely failed to retrive anything useful.
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
    uint8_t id[ID3V2_FRAME_ID_MAX_SIZE] = {0,0,0,0};
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

            if(!(tSize = byteStreamReturnU32(stream))){
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
            }

            if(readBit(flagBytes[1], 6)){
                if(!(byteStreamRead(stream, &encryptionSymbol, 1))){
                    break;
                }
            }

            if(readBit(flagBytes[1], 5)){
                if(!(byteStreamRead(stream, &groupSymbol, 1))){
                    break;
                }
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
            }

            if(readBit(flagBytes[1], 2)){
                if(!byteStreamRead(stream, &encryptionSymbol, 1)){
                    break;
                }
            }

            if(readBit(flagBytes[1], 1)){
                unsync = true;
            }

            if(readBit(flagBytes[1], 3) || encryptionSymbol || readBit(flagBytes[1], 0)){
                decompressionSize = byteStreamReturnSyncInt(stream);
            }

            break;
        
        // no support
        default:
            *frameHeader = NULL;
            *frameSize = 0;
            return 0;
    }

    *frameHeader = id3v2CreateFrameHeader(id, tagAlter, fileAlter, readOnly, unsync, decompressionSize, encryptionSymbol, groupSymbol);
    *frameSize = tSize;
    walk = stream->cursor - resetIndex;
    stream->cursor = resetIndex;
    //printf("[*] frameSize = %zu, walk = %zu stream->cursor = %zu flagOffset = %d\n", *frameSize, walk, stream->cursor, flagOffset);
    return walk;
}

/**
 * @brief Parses an ID3 version 2, 3, or 4 frame. This function returns a couple of different values back to the caller
 * including the number of bytes needed to parse the data and a referance to a heap allocated frame structure. This is
 * the meat of the parser and requires 'hints' in the form of a context list to prase a frame successfully. There is
 * no error state for this function but a returned size of 0 or an incomplete frame such as a missing entry list may
 * indicate one.
 * 
 * @param stream 
 * @param context 
 * @param version 
 * @param frame 
 * @return uint32_t 
 */
uint32_t id3v2ParseFrame(ByteStream *stream, List *context, uint8_t version, Id3v2Frame **frame){
    
    if(!stream || !context){
        *frame = NULL;
        return 0;
    }

    // values needed
    Id3v2FrameHeader *header = NULL;
    List *entries = NULL;
    *frame = NULL;

    // needed for parsing
    size_t resetIndex = 0;
    size_t walk = 0;
    size_t currIterations = 0;
    size_t concurrentBitCount = 0;
    uint32_t expectedHeaderSize = 0;
    uint32_t expectedContentSize = 0;
    ByteStream *innerSream = NULL;
    ListIter iter;
    ListIter iterStorage;
    void *contextData = NULL;

    resetIndex = stream->cursor;

    expectedHeaderSize = id3v2ParseFrameHeader(stream, version, &header, &expectedContentSize);
    walk += expectedHeaderSize;
    if(!expectedHeaderSize){
        if(header != NULL){
            id3v2DestroyFrameHeader(&header);
        }
        stream->cursor = resetIndex;
        return 0;
    }

    if(header == NULL || !expectedContentSize){
        // just in case both args are not null
        if(header != NULL){
            id3v2DestroyFrameHeader(&header);
        }
        stream->cursor = resetIndex;
        return expectedHeaderSize;
    }

    byteStreamSeek(stream, expectedHeaderSize, SEEK_CUR);

    innerSream = byteStreamCreate(byteStreamCursor(stream), expectedContentSize);
    stream->cursor = resetIndex;

    entries = listCreate(id3v2PrintContentEntry, id3v2DeleteContentEntry, id3v2CompareContentEntry, id3v2CopyContentEntry);


    // is a frame compressed or encryped?
    // if so a generic context will be used meaning its not up to me to decompress or unecrypt + if its a text frame none
    // of the reads or writes will work until someone reparses the data after its changed.
    if(header->encryptionSymbol > 0 || header->decompressionSize > 0){
        
        void * data = NULL;
        size_t dataSize = 0;
        List *gContext = id3v2CreateGenericFrameContext();
        Id3v2ContentContext *cc = (Id3v2ContentContext *) gContext->head->data;

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

        data = malloc(dataSize);

        if(!byteStreamRead(innerSream, (uint8_t *)data, dataSize)){
            memset(data, 0, dataSize);
        }

        listInsertBack(entries, id3v2CreateContentEntry(data, dataSize));
        free(data);

        walk += innerSream->cursor;
        *frame = id3v2CreateFrame(header, gContext, entries);
        byteStreamDestroy(innerSream);
        return walk;
    }

    iter = listCreateIterator(context);

    while((contextData = listIteratorNext(&iter)) != NULL){
        
        Id3v2ContentContext *cc = (Id3v2ContentContext *) contextData;

        switch(cc->type){
            
            // encoded strings
            case encodedString_context:{
                
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
                        poscc--;
                    }

                    if(((Id3v2ContentContext *)tmp)->key == id3v2djb2("encoding")){
                        break;
                    }

                    poscc++;
                }

                while((tmp = listIteratorNext(&contentEntryIter)) != NULL){

                    if(poscc == posce){
                        encoding = ((uint8_t *)((Id3v2ContentEntry *)tmp)->entry)[0];
                    }

                    posce++;
                }

                switch(encoding){
                    case BYTE_ISO_8859_1:
                    case BYTE_ASCII:
                    case BYTE_UTF8:
                        data = byteStreamReturnUtf8(innerSream, &dataSize);

                        if(data == NULL && dataSize == 0){
                            byteStreamSeek(innerSream, 1, SEEK_CUR);
                            data = calloc(sizeof(unsigned char), 1);
                            memset(data, 0, 1);
                            dataSize = 1;
                        }
                        break;
                    case BYTE_UTF16BE:
                    case BYTE_UTF16LE:
                        data = byteStreamReturnUtf16(innerSream, &dataSize);

                        if(data == NULL && dataSize == 0){
                            byteStreamSeek(innerSream, 2, SEEK_CUR);
                            data = calloc(sizeof(unsigned char), 2);
                            memset(data, 0, 2);
                            dataSize = 2;
                        }
                        break;
                    default:
                        break;

                }

                if(dataSize > cc->max){
                    dataSize = cc->max;
                    data = realloc(data, dataSize);

                }else if(cc->min > dataSize){
                    data = realloc(data, cc->min);
                    memset(data + dataSize, 0, cc->min - dataSize);
                    dataSize = cc->min;
                }

                listInsertBack(entries, id3v2CreateContentEntry(data, dataSize));
                free(data);

                expectedContentSize = ((expectedContentSize < dataSize) ? 0 : expectedContentSize - dataSize);
            }
                break;
            // only characters found within the latain1 character set      
            case latin1Encoding_context:{

                void *data = NULL;
                size_t dataSize = 0;

                data = byteStreamReturnLatin1(innerSream, &dataSize);
                
                if(dataSize > cc->max){
                    dataSize = cc->max;
                    data = realloc(data, dataSize);

                }else if(cc->min > dataSize){
                    data = realloc(data, cc->min);
                    memset(data + dataSize, 0, cc->min - dataSize);
                    dataSize = cc->min;
                }

                listInsertBack(entries, id3v2CreateContentEntry(data, dataSize));
                free(data);

                expectedContentSize = ((expectedContentSize < dataSize) ? 0 : expectedContentSize - dataSize);
            }
                break;
            // numbers (handled the same way)
            case binary_context:
            case noEncoding_context:
            case precision_context:
            case numeric_context:{
                
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

                data = malloc(dataSize);

                if(!byteStreamRead(innerSream, (uint8_t *)data, dataSize)){
                    memset(data, 0, dataSize);
                }

                listInsertBack(entries, id3v2CreateContentEntry(data, dataSize));
                free(data);

                expectedContentSize = ((expectedContentSize < dataSize) ? 0 : expectedContentSize - dataSize);
            }
                break;
            case bit_context:{

                void *data = NULL;
                size_t nBits = 0;
                size_t dataSize = 0;
                Id3v2ContextType isBitContext = 0;

                if(cc->min == cc->max){
                    nBits = cc->min;
                }else if(cc->min > cc->max){// no trust
                    nBits = cc->min;
                }else{
                    nBits = cc->max;
                }

                dataSize = (nBits + (CHAR_BIT - 1)) / CHAR_BIT;
                
                data = malloc(dataSize);
                memset(data, 0, dataSize);
                copyNBits(byteStreamCursor(innerSream), data, concurrentBitCount, nBits);
                
                if(iter.current->data != NULL){
                    Id3v2ContentContext *nextContext = (Id3v2ContentContext *) iter.current->data;
                    isBitContext = nextContext->type;
                }
                
                concurrentBitCount += nBits;
                
                if(isBitContext != bit_context){

                    if(concurrentBitCount / CHAR_BIT){
                        byteStreamSeek(innerSream, concurrentBitCount / CHAR_BIT, SEEK_CUR);
                        expectedContentSize = ((expectedContentSize < dataSize) ? 0 : expectedContentSize - dataSize);
                    }

                    concurrentBitCount = 0;
                    
                }

                listInsertBack(entries, id3v2CreateContentEntry(data, dataSize));
                free(data);
            }
                break;
            case iter_context:{

                if(!currIterations){
                    
                    iterStorage = iter;

                    iter = listCreateIterator(context);

                    for(size_t i = 0; i < cc->min; i++){
                        listIteratorNext(&iter);
                    }

                }

                if(currIterations != cc->max && currIterations != 0){
                    
                    iter = listCreateIterator(context);

                    for(size_t i = 0; i < cc->min; i++){
                        listIteratorNext(&iter);
                    }
                }


                if(currIterations >= cc->max){
                    iter = iterStorage;

                    for(size_t i = 0; i < currIterations; i++){
                        listIteratorNext(&iter);
                    }

                    currIterations = 0;

                }

                currIterations++;
            
            }
                break;
            case adjustment_context:{

                void *data = NULL;
                size_t dataSize = 0;

                size_t posce = 0;
                size_t poscc = 0;

                ListIter contentContextIter = listCreateIterator(context); 
                ListIter contentEntryIter = listCreateIterator(entries);
                void *tmp = NULL;

                while((tmp = listIteratorNext(&contentContextIter)) != NULL){

                    if(((Id3v2ContentContext *)tmp)->type == iter_context){
                        poscc--;
                    }

                    if(((Id3v2ContentContext *)tmp)->key == id3v2djb2("adjustment")){
                        break;
                    }

                    poscc++;
                }

                while((tmp = listIteratorNext(&contentEntryIter)) != NULL){
                    if(poscc == posce){
                        dataSize = btoi((unsigned char *)((Id3v2ContentEntry *)tmp)->entry, ((Id3v2ContentEntry *)tmp)->size);
                    }

                    posce++;
                }

                if(dataSize > expectedContentSize){
                    dataSize = expectedContentSize;
                }

                data = malloc(dataSize);
                memset(data, 0, dataSize); //ensure data exists
                byteStreamRead(innerSream, data, dataSize); // no need to check error code do to above memset

                listInsertBack(entries, id3v2CreateContentEntry(data, dataSize));
                free(data);

                expectedContentSize = ((expectedContentSize < dataSize) ? 0 : expectedContentSize - dataSize);


            }
                break;
            // no support
            case unknown_context:
            default:
                byteStreamSeek(innerSream, 0, SEEK_END);
                break;
        }

        if(expectedContentSize == 0 || byteStreamGetCh(innerSream) == EOF){
            break;
        }

    }


    // enforce frame size from header parsing
    walk += innerSream->bufferSize;
    
    *frame = id3v2CreateFrame(header, listDeepCopy(context), entries);
    byteStreamDestroy(innerSream);
    return walk;
}

/**
 * @brief Parses an ID3 version 2, 3, or 4 tag from a stream. This function will load the default frame
 * ID, context pairings and any user supplied ones to extract frames within a tag. If this function fails
 * it will return NULL to the caller otherwise, some form of parsed tag will be returned based on the
 * completness of the stream.
 * @details Unlike other parsing functions this one consumes the stream it uses so sending in a copy is
 * recommended. Additionally, unsynchronisation will take a long time to parse as 0s are stripped before
 * frame parsing begins, which is increadibly expensive to do. A fast system is recommended for this 
 * feature.
 * @param stream 
 * @param userPairs 
 * @return Id3v2Tag* 
 */
Id3v2Tag *id3v2ParseTagFromStream(ByteStream *stream, HashTable *userPairs){

    if(!stream){
        return NULL;
    }

    bool exit = false;
    uint32_t read = 0;
    uint32_t tagSize = 0;
    uint8_t id[ID3V2_TAG_ID_SIZE];
    Id3v2TagHeader *header = NULL;
    Id3v2ExtendedTagHeader *ext = NULL;
    List *frames = NULL;
    HashTable *pairs = NULL;

    frames = listCreate(id3v2PrintFrame, id3v2DeleteFrame, id3v2CompareFrame, id3v2CopyFrame);

    // locate the start of the tag
    while(true){

        if(!byteStreamRead(stream, id, ID3V2_TAG_ID_SIZE)){
            listFree(frames);
            return NULL;
        }

        if(btoi(id, ID3V2_TAG_ID_SIZE) == ID3V2_TAG_ID_MAGIC_NUMBER_H){
            stream->cursor = stream->cursor - ID3V2_TAG_ID_SIZE;
            break;
        }
    }

    while(true){

        read = id3v2ParseTagHeader(stream, &header, &tagSize);

        if(read == 0 || header == NULL || tagSize == 0){
            break;
        }

        if(!byteStreamSeek(stream, read, SEEK_CUR)){
            break;
        }

        // shorten to exclude none tag data
        byteStreamResize(stream, tagSize + read);

        if(id3v2ReadUnsynchronisationIndicator(header) == 1){
            size_t readCount = 0;
            tagSize = tagSize / 2; // format is $xx $00 ...
            while(readCount < tagSize){

                byteStreamSeek(stream, 1, SEEK_CUR);
                if(!byteStreamDeleteCh(stream)){
                    break;
                }

                readCount++;
            }

            byteStreamSeek(stream, 10, SEEK_SET);
        }

        if((header->majorVersion == ID3V2_TAG_VERSION_3 || header->majorVersion == ID3V2_TAG_VERSION_4) && id3v2ReadExtendedHeaderIndicator(header) == 1){
            read = id3v2ParseExtendedTagHeader(stream, header->majorVersion, &ext);
            header->extendedHeader = ext;

            if(read == 0 || ext == NULL){
                break;
            }

            tagSize = ((tagSize < read) ? 0 : tagSize - read);
            byteStreamSeek(stream, read, SEEK_CUR);
        }

        pairs = id3v2CreateDefaultIdentiferContextPairings(header->majorVersion);

        while(tagSize){
            
            Id3v2Frame *frame = NULL;
            List *context = NULL;
            uint8_t frameId[ID3V2_FRAME_ID_MAX_SIZE];

            memset(frameId, 0, ID3V2_FRAME_ID_MAX_SIZE);

            if(header->majorVersion == ID3V2_TAG_VERSION_3 || header->majorVersion == ID3V2_TAG_VERSION_4){

                if(!byteStreamRead(stream, frameId, ID3V2_FRAME_ID_MAX_SIZE)){
                    exit = true;
                    break;
                }

                stream->cursor = stream->cursor - ID3V2_FRAME_ID_MAX_SIZE;

            }else if(header->majorVersion == ID3V2_TAG_VERSION_2){

                if(!byteStreamRead(stream, frameId, ID3V2_FRAME_ID_MAX_SIZE - 1)){
                    exit = true;
                    break;
                }

                stream->cursor = stream->cursor - (ID3V2_FRAME_ID_MAX_SIZE - 1);
            }

            // check default pairings (pass 1/4)
            context = hashTableRetrieve(pairs, (char *)frameId);

            // check user supplied ones (pass 2/4)
            if(context == NULL){
                context = hashTableRetrieve(userPairs, (char *)frameId);
            }

            // special considerations (pass 3/4)
            if(context == NULL){
                if(frameId[0] == 'T'){
                    context = hashTableRetrieve(pairs, "T");
                }
                
                if(frameId[0] == 'W'){
                    context = hashTableRetrieve(pairs, "W");
                }
            }

            // use generic (pass 4/4)
            if(context == NULL){
                context = hashTableRetrieve(pairs, "?");
            }
            
            read = id3v2ParseFrame(stream, context, header->majorVersion, &frame);

            if(read == 0 || frame == NULL){
                exit = true;
                break;
            }

            listInsertBack(frames, frame);
            tagSize = ((tagSize < read) ? 0 : tagSize - read);
            byteStreamSeek(stream, read, SEEK_CUR);

        }

        if(pairs != NULL){
            hashTableFree(pairs);
        }

        // double loop break
        if(exit){
            break;
        }

        return id3v2CreateTag(header, frames);

    }

    return id3v2CreateTag(header, frames);

}
