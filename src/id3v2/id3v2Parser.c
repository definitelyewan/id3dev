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

static void copyNBits(const unsigned char *src, unsigned char *dest, const int startBit,
                      const int nBits) {

    if (dest == NULL) {
        return;
    }

    int byteIndex = startBit / CHAR_BIT;
    int bitIndex = startBit % CHAR_BIT;
    int remainingBits = nBits;

    while (remainingBits > 0) {
        const unsigned char mask = 0xFF >> (CHAR_BIT - remainingBits);
        const unsigned char shiftedMask = mask << bitIndex;

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
 * @brief Parses the ID3v2 extended tag header for version 3 or 4 tags.
 *
 * @details This function reads and parses the extended header of an ID3v2.3 or ID3v2.4 tag from the
 * given buffer. It allocates and returns a heap-allocated Id3v2ExtendedTagHeader structure through
 * the output parameter. The function returns the number of bytes read from the buffer to parse the
 * extended header. If the input is invalid, the version is unsupported, or parsing fails, the
 * output pointer is set to NULL and 0 is returned.
 *
 * @param in Pointer to the input buffer containing the tag data.
 * @param inLength Length of the input buffer.
 * @param version The ID3v2 tag version (should be 3 or 4).
 * @param extendedTagHeader Output pointer to the parsed Id3v2ExtendedTagHeader structure
 * (heap-allocated).
 * @return uint32_t - Number of bytes read from the buffer to parse the extended header, or 0 on
 * failure.
 */
uint32_t id3v2ParseExtendedTagHeader(uint8_t *in, const size_t inLength, const uint8_t version,
                                     Id3v2ExtendedTagHeader **extendedTagHeader) {

    if (in == NULL || inLength == 0) {
        *extendedTagHeader = NULL;
        return 0;
    }

    // ID3V2_TAG_VERSION_2 has not extended header in its standard
    if (version == ID3V2_TAG_VERSION_2) {
        *extendedTagHeader = NULL;
        return 0;
    }

    ByteStream *stream = byteStreamCreate(in, inLength);

    if (!stream) {
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
    unsigned char crcBytes[5] = {0, 0, 0, 0, 0};
    bool hasCrc = false;
    bool hasRestrictions = false;
    ByteStream *innerStream = NULL;

    // size return
    uint32_t walk = 0;
    const uint32_t hSize = byteStreamReturnInt(stream);
    size_t offset = 0;

    if (!hSize) {
        *extendedTagHeader = NULL;
        byteStreamDestroy(stream);
        return 0;
    }

    if (hSize > (stream->bufferSize - stream->cursor)) {
        offset = hSize - (stream->bufferSize - stream->cursor);
    }

    innerStream = byteStreamCreate(byteStreamCursor(stream), hSize - offset);

    switch (version) {
    case ID3V2_TAG_VERSION_3:

        // read flags
        flags = (int8_t) byteStreamReadBit(innerStream, 7);
        if (flags == -1) {
            break;
        }

        // skip over flags
        if (!(byteStreamSeek(innerStream, 2, SEEK_CUR))) {
            break;
        }

        // read padding
        padding = (uint32_t) byteStreamReturnInt(innerStream);
        if (!padding) {
            if (flags == 0) {
                break;
            }
        }

        // check to see if a crc is there
        if (flags) {
            crc = (uint32_t) byteStreamReturnInt(innerStream);
            if (!crc) {
                break;
            }
        }

        break;

    case ID3V2_TAG_VERSION_4:

        // number of flag bytes
        if (!(byteStreamSeek(innerStream, 1, SEEK_CUR))) {
            break;
        }

        // read flags
        if (byteStreamReadBit(innerStream, 6)) {
            update = true;
        }

        if (byteStreamReadBit(innerStream, 5)) {
            hasCrc = true;
        }

        if (byteStreamReadBit(innerStream, 4)) {
            hasRestrictions = true;
        }

        if (!(byteStreamSeek(innerStream, 1, SEEK_CUR))) {
            break;
        }

        // read crc
        if (hasCrc) {
            if (!(byteStreamRead(innerStream, (uint8_t *) crcBytes, 5))) {
                break;
            }

            crc = byteSyncintDecode(btoi(crcBytes, 5));

        }

        // read restrictions
        if (hasRestrictions) {

            for (uint8_t k = 8; k > 0; k--) {

                int bit = byteStreamReadBit(innerStream, k - 1);

                if (bit == -1) {
                    break;
                }

                restrictions = setBit(restrictions, k - 1, (bool) bit);
            }

            tagRestrictions = true;

            if (!(byteStreamSeek(innerStream, 1, SEEK_CUR))) {
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
    byteStreamDestroy(stream);
    *extendedTagHeader = id3v2CreateExtendedTagHeader(padding, crc, update, tagRestrictions,
                                                      restrictions);
    walk = innerStream->cursor + 4;

    if (innerStream != NULL) {
        byteStreamDestroy(innerStream);
    }

    return walk;
}

/**
 * @brief Parses an ID3v2 tag header.
 *
 * @details This function reads and parses the tag header for an ID3v2.2, ID3v2.3, or ID3v2.4 tag
 * from the given buffer. It allocates and returns a heap-allocated Id3v2TagHeader structure through
 * the output parameter and the total tag size through a second output parameter. The function
 * returns the number of bytes read from the buffer to parse the tag header. If the input is invalid
 * or parsing fails, the output pointer will be set to null but, a value 0 or greater may be
 * returned.
 *
 * @param in Pointer to the input buffer containing data.
 * @param inLength Length of the input buffer.
 * @param tagHeader Output pointer to the parsed Id3v2TagHeader structure (heap-allocated)
 * @param tagSize Output pointer to a stack allocated u32
 * @return uint32_t - Number of bytes read from the buffer to parse the tag header
 */
uint32_t id3v2ParseTagHeader(uint8_t *in, size_t const inLength, Id3v2TagHeader **tagHeader,
                             uint32_t *tagSize) {

    if (in == NULL || inLength == 0) {
        *tagHeader = NULL;
        return 0;
    }

    ByteStream *stream = byteStreamCreate(in, inLength);

    if (!stream) {
        *tagHeader = NULL;
        return 0;
    }

    // values needed
    uint8_t major = 0;
    uint8_t minor = 0;
    uint8_t flags = 0;

    // values for parsing
    uint32_t hSize = 0;
    uint32_t walk = 0;
    uint8_t id[ID3V2_TAG_ID_SIZE];
    uint32_t idAsInt = 0;

    // check for magic number
    if (!(byteStreamRead(stream, id, ID3V2_TAG_ID_SIZE))) {
        *tagHeader = NULL;
        return 0;
    }

    for (int i = 0; i < ID3V2_TAG_ID_SIZE; ++i) {
        idAsInt = (idAsInt << 8) | id[i];
    }

    if (ID3V2_TAG_ID_MAGIC_NUMBER_H != idAsInt) {
        *tagHeader = NULL;
        return ID3V2_TAG_ID_SIZE;
    }

    while (true) {

        // versions
        if (!(byteStreamRead(stream, &major, 1))) {
            break;
        }

        if (!(byteStreamRead(stream, &minor, 1))) {
            break;
        }

        // flags
        if (!(byteStreamRead(stream, &flags, 1))) {
            break;
        }

        // size
        hSize = byteStreamReturnSyncInt(stream);
        if (!hSize) {
            break;
        }

        break;
    }

    walk = stream->cursor;
    *tagSize = hSize;
    *tagHeader = id3v2CreateTagHeader(major, minor, flags, NULL);
    byteStreamDestroy(stream);
    return walk;
}

/**
 * @brief Parses an ID3v2 frame header.
 *
 * @details This function reads and parses the frame header for a single ID3v2.2, ID3v2.3, or
 * ID3v2.4 frame from a given buffer. It allocates and returns a heap-allocated Id3v2FrameHeader
 * structure, and stack provided frame size though output parameters. The function returns the
 * number of bytes read from the buffer to parse the frame header. If the inputs are invalid or
 * parsing fails the output pointer will be set to null but, a value 0 or greater may be returned.
 *
 *
 * @param in Pointer to the input buffer containing data.
 * @param inLength Length of the input buffer.
 * @param version ID3v2 tag version.
 * @param frameHeader Output pointer to the parsed Id3v2FrameHeader (heap-allocated).
 * @param frameSize Output pointer to a provided u32
 * @return uint32_t - Number of bytes read from the buffer to parse the frame header
 */
uint32_t id3v2ParseFrameHeader(uint8_t *in, const size_t inLength, const uint8_t version,
                               Id3v2FrameHeader **frameHeader, uint32_t *frameSize) {

    if (in == NULL || inLength == 0) {
        *frameHeader = NULL;
        *frameSize = 0;
        return 0;
    }

    ByteStream *stream = byteStreamCreate(in, inLength);

    if (!stream) {
        *frameHeader = NULL;
        return 0;
    }

    // values needed
    uint8_t id[ID3V2_FRAME_ID_MAX_SIZE] = {0, 0, 0, 0};
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
    uint32_t walk = 0;

    switch (version) {
    case ID3V2_TAG_VERSION_2:

        if (!byteStreamRead(stream, id, ID3V2_FRAME_ID_MAX_SIZE - 1)) {
            *frameHeader = NULL;
            *frameSize = 0;
            byteStreamDestroy(stream);
            return 0;
        }

        if (!byteStreamRead(stream, sizeBytes, ID3V2_FRAME_ID_MAX_SIZE - 1)) {
            *frameHeader = NULL;
            *frameSize = 0;
            byteStreamDestroy(stream);
            return ID3V2_FRAME_ID_MAX_SIZE - 1;
        }

        tSize = (uint32_t) btoi(sizeBytes, ID3V2_FRAME_ID_MAX_SIZE - 1);
        if (!tSize) {
            break;
        }

        break;
    case ID3V2_TAG_VERSION_3:
        if (!byteStreamRead(stream, id, ID3V2_FRAME_ID_MAX_SIZE)) {
            *frameHeader = NULL;
            *frameSize = 0;
            byteStreamDestroy(stream);
            return 0;
        }

        tSize = byteStreamReturnU32(stream);
        if (!tSize) {
            *frameHeader = NULL;
            *frameSize = 0;
            byteStreamDestroy(stream);
            return ID3V2_FRAME_ID_MAX_SIZE;
        }

        if (!(byteStreamRead(stream, flagBytes, ID3V2_FRAME_FLAG_SIZE))) {
            *frameHeader = NULL;
            *frameSize = tSize;
            byteStreamDestroy(stream);
            return ID3V2_FRAME_ID_MAX_SIZE * 2;
        }

        if (readBit(flagBytes[0], 7)) {
            tagAlter = true;
        }

        if (readBit(flagBytes[0], 6)) {
            fileAlter = true;
        }

        if (readBit(flagBytes[0], 5)) {
            readOnly = true;
        }

        if (readBit(flagBytes[1], 7)) {
            decompressionSize = (uint32_t) byteStreamReturnInt(stream);
            if (!decompressionSize) {
                break;
            }
        }

        if (readBit(flagBytes[1], 6)) {
            if (!(byteStreamRead(stream, &encryptionSymbol, 1))) {
                break;
            }
        }

        if (readBit(flagBytes[1], 5)) {
            if (!(byteStreamRead(stream, &groupSymbol, 1))) {
                break;
            }
        }

        break;
    case ID3V2_TAG_VERSION_4:

        if (!byteStreamRead(stream, id, ID3V2_FRAME_ID_MAX_SIZE)) {
            *frameHeader = NULL;
            *frameSize = 0;
            byteStreamDestroy(stream);
            return 0;
        }

        tSize = byteStreamReturnSyncInt(stream);
        if (!tSize) {
            *frameHeader = NULL;
            *frameSize = 0;
            byteStreamDestroy(stream);
            return ID3V2_FRAME_ID_MAX_SIZE;
        }

        if (!(byteStreamRead(stream, flagBytes, ID3V2_FRAME_FLAG_SIZE))) {
            *frameHeader = NULL;
            *frameSize = tSize;
            byteStreamDestroy(stream);
            return ID3V2_FRAME_ID_MAX_SIZE * 2;
        }

        if (readBit(flagBytes[0], 6)) {
            tagAlter = true;
        }

        if (readBit(flagBytes[0], 5)) {
            fileAlter = true;
        }

        if (readBit(flagBytes[0], 4)) {
            readOnly = true;
        }

        if (readBit(flagBytes[1], 6)) {
            if (!byteStreamRead(stream, &groupSymbol, 1)) {
                break;
            }
        }

        if (readBit(flagBytes[1], 2)) {
            if (!byteStreamRead(stream, &encryptionSymbol, 1)) {
                break;
            }
        }

        if (readBit(flagBytes[1], 1)) {
            unsync = true;
        }

        if (readBit(flagBytes[1], 3) || encryptionSymbol || readBit(flagBytes[1], 0)) {
            decompressionSize = byteStreamReturnSyncInt(stream);
        }

        break;

    // no support
    default:
        *frameHeader = NULL;
        *frameSize = 0;
        byteStreamDestroy(stream);
        return 0;
    }

    *frameHeader = id3v2CreateFrameHeader(id, tagAlter, fileAlter, readOnly, unsync,
                                          decompressionSize, encryptionSymbol, groupSymbol);
    *frameSize = tSize;
    walk = stream->cursor;
    byteStreamDestroy(stream);
    // printf("[*] frameSize = %zu, walk = %zu stream->cursor = %zu\n", *frameSize, walk, stream->cursor); // debug info i dont wanna rewrite
    return walk;
}

/**
 * @brief Parse an ID3v2 frame.
 *
 * @details This function reads and parses an ID3v2.2, ID3v2.3, and ID3v2.4 frame from a buffer. It
 * returns a heap-allocated Id3v2frame structure via an output parameter and the number of bytes
 * read from the input required to parse the frame. For this function to work correctly a context
 * list is required and the output is souly dependent on this list. If the inputs are invalid,
 * parsing fails, this function will return a frame in an incomplete state meaning one or more
 * structure members may be missing.
 *
 * @param in Pointer to the input buffer containing data.
 * @param inLength Length of the input buffer.
 * @param context ID3v2 context linked list
 * @param version ID3v2 version.
 * @param frame Output pointer to the parsed Id3v2Frame (heap-allocated)
 * @return uint32_t - Number of bytes read from the buffer to prase the frame
 */
uint32_t id3v2ParseFrame(uint8_t *in, const size_t inLength, List *context, uint8_t version,
                         Id3v2Frame **frame) {

    if (in == NULL || inLength == 0) {
        *frame = NULL;
        return 0;
    }

    ByteStream *stream = byteStreamCreate(in, inLength);

    if (!stream || !context) {
        *frame = NULL;
        return 0;
    }

    // values needed
    Id3v2FrameHeader *header = NULL;
    List *entries = NULL;
    *frame = NULL;

    // needed for parsing
    bool exit = false;
    size_t walk = 0;
    size_t currIterations = 0;
    size_t concurrentBitCount = 0;
    uint32_t expectedHeaderSize = 0;
    uint32_t expectedContentSize = 0;
    ByteStream *innerStream = NULL;
    ListIter iter;
    ListIter iterStorage;
    void *contextData = NULL;

    expectedHeaderSize = id3v2ParseFrameHeader(stream->buffer, stream->bufferSize, version, &header,
                                               &expectedContentSize);
    walk += expectedHeaderSize;
    if (!expectedHeaderSize) {
        if (header != NULL) {
            id3v2DestroyFrameHeader(&header);
        }

        byteStreamDestroy(stream);
        return 0;
    }

    if (header == NULL || !expectedContentSize) {
        // just in case both args are not null
        if (header != NULL) {
            id3v2DestroyFrameHeader(&header);
        }

        byteStreamDestroy(stream);
        return expectedHeaderSize;
    }

    byteStreamSeek(stream, expectedHeaderSize, SEEK_CUR);

    innerStream = byteStreamCreate(byteStreamCursor(stream), expectedContentSize);
    entries = listCreate(id3v2PrintContentEntry, id3v2DeleteContentEntry, id3v2CompareContentEntry,
                         id3v2CopyContentEntry);

    // is a frame compressed or encrypted?
    // if so a generic context will be used meaning it's not up to me to decompress or unencrypted + if it's a text frame none
    // of the reads or writes will work until someone reparses the data after its changed.
    if (header->encryptionSymbol > 0 || header->decompressionSize > 0) {

        void *data = NULL;
        size_t dataSize = 0;
        List *gContext = id3v2CreateGenericFrameContext();
        Id3v2ContentContext *cc = (Id3v2ContentContext *) gContext->head->data;

        if (cc->min > cc->max) {
            // no trust
            dataSize = cc->min;
        } else {
            dataSize = cc->max;
        }

        if (dataSize > expectedContentSize) {
            dataSize = expectedContentSize;
        }

        data = malloc(dataSize);

        if (!byteStreamRead(innerStream, (uint8_t *) data, dataSize)) {
            memset(data, 0, dataSize);
        }

        listInsertBack(entries, id3v2CreateContentEntry(data, dataSize));
        free(data);

        walk += innerStream->cursor;
        *frame = id3v2CreateFrame(header, gContext, entries);
        byteStreamDestroy(innerStream);
        byteStreamDestroy(stream);
        return walk;
    }

    iter = listCreateIterator(context);

    while ((contextData = listIteratorNext(&iter)) != NULL) {

        Id3v2ContentContext *cc = (Id3v2ContentContext *) contextData;

        switch (cc->type) {

        // encoded strings
        case encodedString_context: {

            void *data = NULL;
            size_t dataSize = 0;

            size_t posce = 0;
            size_t poscc = 0;
            uint8_t encoding = 0;
            ListIter contentContextIter = listCreateIterator(context);
            ListIter contentEntryIter = listCreateIterator(entries);
            void *tmp = NULL;

            while ((tmp = listIteratorNext(&contentContextIter)) != NULL) {

                if (((Id3v2ContentContext *) tmp)->type == iter_context) {
                    poscc--;
                }

                if (((Id3v2ContentContext *) tmp)->key == id3v2djb2("encoding")) {
                    break;
                }

                poscc++;
            }

            while ((tmp = listIteratorNext(&contentEntryIter)) != NULL) {

                if (poscc == posce) {
                    encoding = ((uint8_t *) ((Id3v2ContentEntry *) tmp)->entry)[0];
                }

                posce++;
            }

            if (encoding == BYTE_ISO_8859_1 || encoding == BYTE_ASCII || encoding == BYTE_UTF8) {
                data = byteStreamReturnUtf8(innerStream, &dataSize);

                if (data == NULL && dataSize == 0) {
                    byteStreamSeek(innerStream, 1, SEEK_CUR);
                    data = calloc(sizeof(unsigned char), 1);
                    memset(data, 0, 1);
                    dataSize = 1;
                }
            } else if (encoding == BYTE_UTF16BE || encoding == BYTE_UTF16LE) {
                data = byteStreamReturnUtf16(innerStream, &dataSize);

                if (data == NULL && dataSize == 0) {
                    byteStreamSeek(innerStream, 2, SEEK_CUR);
                    data = calloc(sizeof(unsigned char), 2);
                    memset(data, 0, 2);
                    dataSize = 2;
                }

            } else {
                /**
                 * If the caller attempts to use an encoding that is unknown it must be
                 * disregarded (In my opinion rn). As a base case Latin1 will be attempted but
                 * the caller is likley to get a malformed frame.
                 *
                 * NOTE: if the frame has an context labled encoding elsewhere in there frame
                 * it will not match the this encodedString_context's entry and will be up to
                 * the caller to deciper if data is malformed
                 */
                data = byteStreamReturnLatin1(innerStream, &dataSize);

                if (data == NULL && dataSize == 0) {
                    byteStreamSeek(innerStream, 1, SEEK_CUR);
                    data = calloc(sizeof(unsigned char), 1);
                    memset(data, 0, 1);
                    dataSize = 1;
                }

            }

            if (dataSize > cc->max && cc->max > 0) {

                void *reallocPtr = realloc(data, cc->max);

                if (reallocPtr == NULL) {
                    free(data);
                    exit = true;
                    break;
                }

                data = reallocPtr;
                dataSize = cc->max;

            } else if (cc->min > dataSize) {

                void *reallocPtr = realloc(data, cc->min);

                if (reallocPtr == NULL) {
                    free(data);
                    exit = true;
                    break;
                }

                data = reallocPtr;
                memset(data + dataSize, 0, cc->min - dataSize);
                dataSize = cc->min;
            }

            listInsertBack(entries, id3v2CreateContentEntry(data, dataSize));
            free(data);

            expectedContentSize = ((expectedContentSize < dataSize)
                                       ? 0
                                       : expectedContentSize - dataSize);
        }
        break;
        // only characters found within the latin1 character set
        case latin1Encoding_context: {

            void *data = NULL;
            size_t dataSize = 0;

            data = byteStreamReturnLatin1(innerStream, &dataSize);

            if (dataSize > cc->max && cc->max > 0) {

                void *reallocPtr = realloc(data, cc->max);

                if (reallocPtr == NULL) {
                    free(data);
                    exit = true;
                    break;
                }

                data = reallocPtr;
                dataSize = cc->max;

            } else if (cc->min > dataSize) {

                void *reallocPtr = realloc(data, cc->min);

                if (reallocPtr == NULL) {
                    free(data);
                    exit = true;
                    break;
                }

                data = reallocPtr;
                memset(data + dataSize, 0, cc->min - dataSize);
                dataSize = cc->min;
            }

            listInsertBack(entries, id3v2CreateContentEntry(data, dataSize));
            free(data);

            expectedContentSize = ((expectedContentSize < dataSize)
                                       ? 0
                                       : expectedContentSize - dataSize);
        }
        break;
        // numbers (handled the same way)
        case binary_context:
        case noEncoding_context:
        case precision_context:
        case numeric_context: {

            void *data = NULL;
            size_t dataSize = 0;

            if (cc->min > cc->max) {
                // no trust
                dataSize = cc->min;
            } else {
                dataSize = cc->max;
            }

            if (dataSize > expectedContentSize) {
                dataSize = expectedContentSize;
            }

            data = malloc(dataSize);

            if (!byteStreamRead(innerStream, (uint8_t *) data, dataSize)) {
                memset(data, 0, dataSize);
            }

            listInsertBack(entries, id3v2CreateContentEntry(data, dataSize));
            free(data);

            expectedContentSize = ((expectedContentSize < dataSize)
                                       ? 0
                                       : expectedContentSize - dataSize);
        }
        break;
        case bit_context: {

            void *data = NULL;
            size_t nBits = 0;
            size_t dataSize = 0;
            Id3v2ContextType isBitContext = 0;

            if (cc->min > cc->max) {
                // no trust
                nBits = cc->min;
            } else {
                nBits = cc->max;
            }

            dataSize = (nBits + (CHAR_BIT - 1)) / CHAR_BIT;

            data = malloc(dataSize);
            memset(data, 0, dataSize);
            copyNBits(byteStreamCursor(innerStream), data, (int) concurrentBitCount, (int) nBits);

            if (iter.current->data != NULL) {
                Id3v2ContentContext *nextContext = (Id3v2ContentContext *) iter.current->data;
                isBitContext = nextContext->type;
            }

            concurrentBitCount += nBits;

            if (isBitContext != bit_context) {

                if (concurrentBitCount / CHAR_BIT) {
                    byteStreamSeek(innerStream, concurrentBitCount / CHAR_BIT, SEEK_CUR);
                    expectedContentSize = ((expectedContentSize < dataSize)
                                               ? 0
                                               : expectedContentSize - dataSize);
                }

                concurrentBitCount = 0;

            }

            listInsertBack(entries, id3v2CreateContentEntry(data, dataSize));
            free(data);
        }
        break;
        case iter_context: {

            if (!currIterations) {

                iterStorage = iter;

                iter = listCreateIterator(context);

                for (size_t i = 0; i < cc->min; i++) {
                    listIteratorNext(&iter);
                }

            }

            if (currIterations != cc->max && currIterations != 0) {

                iter = listCreateIterator(context);

                for (size_t i = 0; i < cc->min; i++) {
                    listIteratorNext(&iter);
                }
            }

            if (currIterations >= cc->max) {
                iter = iterStorage;

                for (size_t i = 0; i < currIterations; i++) {
                    listIteratorNext(&iter);
                }

                currIterations = 0;

            }

            currIterations++;

        }
        break;
        case adjustment_context: {

            void *data = NULL;
            size_t dataSize = 1; // base case for adjustment, assume 1 byte as a size

            size_t posce = 0;
            size_t poscc = 0;

            ListIter contentContextIter = listCreateIterator(context);
            ListIter contentEntryIter = listCreateIterator(entries);
            void *tmp = NULL;

            while ((tmp = listIteratorNext(&contentContextIter)) != NULL) {

                if (((Id3v2ContentContext *) tmp)->type == iter_context) {
                    poscc--;
                }

                if (((Id3v2ContentContext *) tmp)->key == id3v2djb2("adjustment")) {
                    break;
                }

                poscc++;
            }

            while ((tmp = listIteratorNext(&contentEntryIter)) != NULL) {
                if (poscc == posce) {
                    dataSize = btoi((unsigned char *) ((Id3v2ContentEntry *) tmp)->entry,
                                    (int) ((Id3v2ContentEntry *) tmp)->size);
                }

                posce++;
            }

            if (dataSize > expectedContentSize) {
                dataSize = expectedContentSize;
            }

            data = malloc(dataSize);
            memset(data, 0, dataSize); //ensure data exists
            byteStreamRead(innerStream, data, dataSize);
            // no need to check error code do to above memset

            listInsertBack(entries, id3v2CreateContentEntry(data, dataSize));
            free(data);

            expectedContentSize = ((expectedContentSize < dataSize)
                                       ? 0
                                       : expectedContentSize - dataSize);

        }
        break;
        // no support
        case unknown_context:
        default:
            byteStreamSeek(innerStream, 0, SEEK_END);
            break;
        }

        if (expectedContentSize == 0 || byteStreamGetCh(innerStream) == EOF || exit == true) {
            break;
        }

    }

    // enforce frame size from header parsing
    walk += innerStream->bufferSize;

    *frame = id3v2CreateFrame(header, listDeepCopy(context), entries);
    byteStreamDestroy(innerStream);
    byteStreamDestroy(stream);
    return walk;
}

/**
 * @brief Parses an ID3v2 tag from a memory buffer.
 *
 * @details This function locates and parses an ID3v2.2, ID3v2.3, or ID3v2.4 tag from the given
 * binary buffer. It reads the tag header, optional extended header, and all frames, using default
 * and user-supplied context pairings for frame parsing. The function returns a heap-allocated
 * Id3v2Tag structure representing the parsed tag, or NULL on failure.
 *
 * @param in Pointer to the input buffer containing the tag data.
 * @param inLength Size of the input buffer in bytes.
 * @param userPairs Optional hash table of user-supplied context pairings for frame parsing.
 * @return Id3v2Tag* Pointer to the parsed tag structure, or NULL if parsing fails.
 */
Id3v2Tag *id3v2ParseTagFromBuffer(uint8_t *in, const size_t inLength, HashTable *userPairs) {

    if (in == NULL || inLength == 0) {
        return NULL;
    }

    ByteStream *stream = byteStreamCreate(in, inLength);

    if (!stream) {
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
    while (true) {

        if (!byteStreamRead(stream, id, ID3V2_TAG_ID_SIZE)) {
            listFree(frames);
            byteStreamDestroy(stream);
            return NULL;
        }

        if (btoi(id, ID3V2_TAG_ID_SIZE) == ID3V2_TAG_ID_MAGIC_NUMBER_H) {
            stream->cursor = stream->cursor - ID3V2_TAG_ID_SIZE;
            break;
        }
    }

    while (true) {

        read = id3v2ParseTagHeader(byteStreamCursor(stream), stream->bufferSize - stream->cursor,
                                   &header, &tagSize);

        if (read == 0 || header == NULL || tagSize == 0) {
            break;
        }

        if (!byteStreamSeek(stream, read, SEEK_CUR)) {
            break;
        }

        // shorten to exclude none tag data
        byteStreamResize(stream, tagSize + read);

        if (id3v2ReadUnsynchronisationIndicator(header) == 1) {
            size_t readCount = 0;
            tagSize = tagSize / 2; // format is $xx $00 ...
            while (readCount < tagSize) {

                byteStreamSeek(stream, 1, SEEK_CUR);
                if (!byteStreamDeleteCh(stream)) {
                    break;
                }

                readCount++;
            }

            byteStreamSeek(stream, 10, SEEK_SET);
        }

        if ((header->majorVersion == ID3V2_TAG_VERSION_3 || header->majorVersion ==
             ID3V2_TAG_VERSION_4) && id3v2ReadExtendedHeaderIndicator(header) == 1) {
            read = id3v2ParseExtendedTagHeader(byteStreamCursor(stream),
                                               stream->bufferSize - stream->cursor,
                                               header->majorVersion, &ext);
            header->extendedHeader = ext;

            if (read == 0 || ext == NULL) {
                break;
            }

            tagSize = ((tagSize < read) ? 0 : tagSize - read);
            byteStreamSeek(stream, read, SEEK_CUR);
        }

        pairs = id3v2CreateDefaultIdentifierContextPairings(header->majorVersion);

        while (tagSize) {

            Id3v2Frame *frame = NULL;
            List *context = NULL;
            uint8_t frameId[ID3V2_FRAME_ID_MAX_SIZE] = {0};

            if (header->majorVersion == ID3V2_TAG_VERSION_3 || header->majorVersion ==
                ID3V2_TAG_VERSION_4) {

                if (!byteStreamRead(stream, frameId, ID3V2_FRAME_ID_MAX_SIZE)) {
                    exit = true;
                    break;
                }

                stream->cursor = stream->cursor - ID3V2_FRAME_ID_MAX_SIZE;

            } else if (header->majorVersion == ID3V2_TAG_VERSION_2) {

                if (!byteStreamRead(stream, frameId, ID3V2_FRAME_ID_MAX_SIZE - 1)) {
                    exit = true;
                    break;
                }

                stream->cursor = stream->cursor - (ID3V2_FRAME_ID_MAX_SIZE - 1);
            }

            // check default pairings (pass 1/4)
            context = hashTableRetrieve(pairs, (char *) frameId);

            // check user supplied ones (pass 2/4)
            if (context == NULL) {
                context = hashTableRetrieve(userPairs, (char *) frameId);
            }

            // special considerations (pass 3/4)
            if (context == NULL) {
                if (frameId[0] == 'T') {
                    context = hashTableRetrieve(pairs, "T");
                }

                if (frameId[0] == 'W') {
                    context = hashTableRetrieve(pairs, "W");
                }
            }

            // use generic (pass 4/4)
            if (context == NULL) {
                context = hashTableRetrieve(pairs, "?");
            }

            read = id3v2ParseFrame(byteStreamCursor(stream), stream->bufferSize - stream->cursor,
                                   context, header->majorVersion, &frame);

            if (read == 0 || frame == NULL) {
                exit = true;
                break;
            }

            listInsertBack(frames, frame);
            tagSize = ((tagSize < read) ? 0 : tagSize - read);
            byteStreamSeek(stream, read, SEEK_CUR);

        }

        if (pairs != NULL) {
            hashTableFree(pairs);
        }

        // double loop break
        if (exit) {
            break;
        }

        byteStreamDestroy(stream);
        return id3v2CreateTag(header, frames);

    }

    byteStreamDestroy(stream);
    return id3v2CreateTag(header, frames);

}
