/**
 * @file id3v2Parser.c
 * @author Ewan Jones
 * @brief Implementations for binary parsing of ID3v2 tags, headers, frames, and complete tag structures from byte buffers
 * @version 26.01
 * @date 2024-02-01 - 2026-01-13
 * 
 * @copyright Copyright (c) 2024 - 2026
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

static void internal_copyNBits(const unsigned char *src, unsigned char *dest, int startBit, int nBits) {
    int byteIndex = startBit / CHAR_BIT;
    int bitIndex = startBit % CHAR_BIT;
    int remainingBits = nBits;

    if (dest == NULL) {
        return;
    }

    while (remainingBits > 0) {
        unsigned char mask = 0xFF >> (CHAR_BIT - remainingBits);
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
 * @brief Parses an ID3v2 extended tag header from a byte buffer.
 * @details Extracts and decodes extended header fields from binary data according to the 
 * ID3v2 specification. Returns the number of bytes consumed during parsing and creates 
 * a heap-allocated extended header structure. Supports version-specific parsing:
 * 
 * - ID3v2.3: Reads size (4 bytes), CRC flag (bit 7), 2 reserved bytes, padding size (4 bytes), 
 * and optional CRC-32 (4 bytes if flag set).
 * 
 * - ID3v2.4: Reads extended size (4 bytes), flag bytes count (1 byte), flags byte (update at 
 * bit 6, CRC at bit 5, restrictions at bit 4), optional syncsafe CRC-32 (5 bytes), and 
 * optional restrictions byte (8 bits for tag/text/image limitations).
 * 
 * ID3v2.2 and unsupported versions return 0 with NULL extendedTagHeader. On success, caller 
 * must free the returned structure with id3v2DestroyExtendedTagHeader.
 * 
 * @param in - Pointer to byte buffer containing extended header data.
 * @param inl - Size of input buffer in bytes.
 * @param version - ID3v2 version (ID3V2_TAG_VERSION_3 or ID3V2_TAG_VERSION_4).
 * @param extendedTagHeader - Output parameter receiving pointer to heap-allocated extended header structure, or NULL on failure.
 * @return uint32_t - Number of bytes read from buffer on success, 0 on failure or unsupported version.
 */
uint32_t id3v2ParseExtendedTagHeader(uint8_t *in, size_t inl, uint8_t version,
                                     Id3v2ExtendedTagHeader **extendedTagHeader) {
    if (in == NULL || inl == 0) {
        *extendedTagHeader = NULL;
        return 0;
    }

    ByteStream *stream = byteStreamCreate(in, inl);

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
    size_t offset = 0;
    unsigned char crcBytes[5] = {0, 0, 0, 0, 0};
    bool hasCrc = false;
    bool hasRestrictions = false;
    ByteStream *innerStream = NULL;

    // size return
    uint32_t hSize = 0;
    uint32_t walk = 0;

    if (version == ID3V2_TAG_VERSION_3 || version == ID3V2_TAG_VERSION_4) {
        hSize = byteStreamReturnInt(stream);
        if (!hSize) {
            *extendedTagHeader = NULL;
            byteStreamDestroy(stream);
            return 0;
        }

        if (hSize > (stream->bufferSize - stream->cursor)) {
            offset = hSize - (stream->bufferSize - stream->cursor);
        }
    }

    innerStream = byteStreamCreate(byteStreamCursor(stream), hSize - offset);

    switch (version) {
        case ID3V2_TAG_VERSION_3:
            flags = (int8_t) byteStreamReadBit(innerStream, 7);

            // read flags
            if (flags == -1) {
                break;
            }

            // skip over flags
            if (!(byteStreamSeek(innerStream, 2, SEEK_CUR))) {
                break;
            }

            // read padding
            padding = byteStreamReturnInt(innerStream);

            if (!padding) {
                if (flags == 0) {
                    break;
                }
            }

            // check to see if a crc is there
            if (flags) {
                crc = byteStreamReturnInt(innerStream);
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
            byteStreamDestroy(innerStream);
            *extendedTagHeader = NULL;
            return 0;
    }

    // clean up
    byteStreamDestroy(stream);
    *extendedTagHeader = id3v2CreateExtendedTagHeader(padding, crc, update, tagRestrictions, restrictions);
    walk = innerStream->cursor + 4;

    if (innerStream != NULL) {
        byteStreamDestroy(innerStream);
    }

    return walk;
}

 /**
 * @brief Parses an ID3v2 tag header from a byte buffer (excluding extended header).
 * @details Extracts and validates the 10-byte tag header structure from binary data according 
 * to the ID3v2 specification. Verifies the "ID3" identifier and decodes version, flags, and 
 * tag size fields. Returns the number of bytes consumed and creates a heap-allocated tag 
 * header structure. The extended header is not parsed by this function - the returned 
 * structure has extendedHeader set to NULL.
 * 
 * Header format (10 bytes):
 * "ID3" identifier (3 bytes) + major version (1) + minor version (1) + flags (1) + 
 * syncsafe tag size (4).
 * 
 * The tagSize output represents the total size of tag content (frames + extended header if 
 * present) excluding the 10-byte header itself. On success, caller must free the returned 
 * structure with id3v2DestroyTagHeader. Returns 0 with NULL tagHeader on complete failure. 
 * Returns ID3V2_TAG_ID_SIZE (3) with NULL tagHeader if magic number is invalid, indicating 
 * the buffer doesn't contain a valid ID3v2 tag.
 * 
 * @param in - Pointer to byte buffer containing tag header data.
 * @param inl - Size of input buffer in bytes.
 * @param tagHeader - Output parameter receiving pointer to heap-allocated tag header structure, or NULL on failure.
 * @param tagSize - Output parameter receiving tag content size in bytes (excluding 10-byte header), or 0 on failure.
 * @return uint32_t - Number of bytes read on success, or 0 on complete failure.
 */
uint32_t id3v2ParseTagHeader(uint8_t *in, size_t inl, Id3v2TagHeader **tagHeader, uint32_t *tagSize) {
    if (in == NULL || inl == 0) {
        *tagHeader = NULL;
        return 0;
    }

    ByteStream *stream = byteStreamCreate(in, inl);


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
 * @brief Parses an ID3v2 frame header from a byte buffer.
 * @details Extracts and decodes frame header fields from binary data according to the 
 * ID3v2 specification. Returns the number of bytes consumed during parsing and creates 
 * a heap-allocated frame header structure. Supports version-specific parsing:
 * 
 * - ID3v2.2: Reads 3-byte frame ID + 3-byte size with no flag bytes.
 * 
 * - ID3v2.3: Reads 4-byte frame ID + 4-byte size + 2-byte flags. Status flags: 
 * tag alter preservation, file alter preservation, read-only. Format flags: 
 * compression (+4 bytes decompression size), encryption (+1 byte symbol), 
 * grouping (+1 byte identifier).
 * 
 * - ID3v2.4: Reads 4-byte frame ID + 4-byte syncsafe size + 2-byte flags. 
 * Status flags: tag alter, file alter, read-only. Format flags: grouping (+1 byte), 
 * encryption (+1 byte), unsynchronisation, compression/data length (+syncsafe size).
 * 
 * The frameSize output represents the frame content size (excluding header). On success, 
 * caller must free the returned structure with id3v2DestroyFrameHeader. Returns 0 with 
 * NULL on complete failure, or partial byte count with appropriate NULL/non-NULL outputs 
 * on partial parsing success.
 * 
 * @param in - Pointer to byte buffer containing frame header data.
 * @param inl - Size of input buffer in bytes.
 * @param version - ID3v2 version (ID3V2_TAG_VERSION_2, ID3V2_TAG_VERSION_3, or ID3V2_TAG_VERSION_4).
 * @param frameHeader - Output parameter receiving pointer to heap-allocated frame header structure, or NULL on failure.
 * @param frameSize - Output parameter receiving frame content size in bytes (excluding header), or 0 on failure.
 * @return uint32_t - Number of bytes read from buffer on success, 0 on complete failure, or partial byte count on partial success.
 */
uint32_t id3v2ParseFrameHeader(uint8_t *in, size_t inl, uint8_t version, Id3v2FrameHeader **frameHeader,
                               uint32_t *frameSize) {
    if (in == NULL || inl == 0) {
        *frameHeader = NULL;
        *frameSize = 0;
        return 0;
    }

    ByteStream *stream = byteStreamCreate(in, inl);

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

            tSize = btoi(sizeBytes, ID3V2_FRAME_ID_MAX_SIZE - 1);
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
                decompressionSize = byteStreamReturnInt(stream);
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

    *frameHeader = id3v2CreateFrameHeader(id, tagAlter, fileAlter, readOnly, unsync, decompressionSize,
                                          encryptionSymbol, groupSymbol);
    *frameSize = tSize;
    walk = stream->cursor;
    byteStreamDestroy(stream);
    // printf("[*] frameSize = %zu, walk = %zu stream->cursor = %zu\n", *frameSize, walk, stream->cursor); // debug info i dont wanna rewrite
    return walk;
}

/**
 * @brief Parses an ID3v2 frame (header + content) from a byte buffer using context-driven interpretation.
 * @details Extracts a complete frame by first parsing the header, then interpreting frame content 
 * according to a context list that provides "hints" about data types and structure. The context-driven 
 * approach allows flexible parsing of the diverse frame types in the ID3v2 specification.
 * 
 * **Encrypted/Compressed Frame Handling:**
 * Frames with encryptionSymbol or decompressionSize set are parsed as raw binary using a generic 
 * context. The caller must decrypt/decompress and reparse to access structured content.
 * 
 * **Context Types Supported:**
 * - encodedString_context: Null-terminated strings with encoding determined by prior "encoding" byte 
 * (ISO-8859-1, UTF-8, UTF-16BE/LE)
 * - latin1Encoding_context: Null-terminated Latin-1 (ISO-8859-1) strings only
 * - binary_context/noEncoding_context/precision_context/numeric_context: Fixed-size raw binary/numeric data
 * - bit_context: Bit-level extraction (WARNING: incorrect implementation as of jan 13 2026)
 * - iter_context: Loop control marker for repeated field groups
 * - adjustment_context: Variable-length data where size comes from previously parsed "adjustment" field
 * - unknown_context: Skip remaining content
 * 
 * The function creates heap-allocated content entries for each parsed field and assembles them into 
 * a complete frame structure. On success, caller must free with id3v2DestroyFrame.
 * 
 * @param in - Pointer to byte buffer containing complete frame data.
 * @param inl - Size of input buffer in bytes.
 * @param context - List of Id3v2ContentContext structures defining frame structure and data types.
 * @param version - ID3v2 version (ID3V2_TAG_VERSION_2, ID3V2_TAG_VERSION_3, or ID3V2_TAG_VERSION_4).
 * @param frame - Output parameter receiving pointer to heap-allocated frame structure, or NULL on failure.
 * @return uint32_t - Total bytes consumed on success, header size on partial success, or 0 on complete failure.
 */
uint32_t id3v2ParseFrame(uint8_t *in, size_t inl, List *context, uint8_t version, Id3v2Frame **frame) {
    if (in == NULL || inl == 0) {
        *frame = NULL;
        return 0;
    }

    ByteStream *stream = byteStreamCreate(in, inl);

    if (!stream || !context) {
        *frame = NULL;
        return 0;
    }

    // values needed
    Id3v2FrameHeader *header = NULL;
    List *entries = NULL;
    *frame = NULL;

    // needed for parsing
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
        uint8_t *data = NULL;
        size_t dataSize = 0;
        List *gContext = id3v2CreateGenericFrameContext();
        Id3v2ContentContext *cc = (Id3v2ContentContext *) gContext->head->data;

        if (cc->min >= cc->max) {
            // no trust
            dataSize = cc->min;
        } else {
            dataSize = cc->max;
        }

        if (dataSize > expectedContentSize) {
            dataSize = expectedContentSize;
        }

        data = malloc(dataSize);

        if (!byteStreamRead(innerStream, data, dataSize)) {
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
                uint8_t *data = NULL;
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

                switch (encoding) {
                    case BYTE_ISO_8859_1:
                    case BYTE_ASCII:
                    case BYTE_UTF8:
                        data = byteStreamReturnUtf8(innerStream, &dataSize);

                        if (data == NULL && dataSize == 0) {
                            byteStreamSeek(innerStream, 1, SEEK_CUR);
                            data = calloc(sizeof(unsigned char), 1);
                            memset(data, 0, 1);
                            dataSize = 1;
                        }
                        break;
                    case BYTE_UTF16BE:
                    case BYTE_UTF16LE:
                        data = byteStreamReturnUtf16(innerStream, &dataSize);

                        if (data == NULL && dataSize == 0) {
                            byteStreamSeek(innerStream, 2, SEEK_CUR);
                            data = calloc(sizeof(unsigned char), 2);
                            memset(data, 0, 2);
                            dataSize = 2;
                        }
                        break;
                    default:
                        break;
                }

                if (dataSize > cc->max) {
                    uint8_t *reallocPtr = realloc(data, cc->max);
                    if (reallocPtr == NULL) {
                        free(data);
                        listFree(entries);
                        byteStreamDestroy(innerStream);
                        byteStreamDestroy(stream);
                        id3v2DestroyFrameHeader(&header);
                        *frame = NULL;
                        return 0;
                    }
                    dataSize = cc->max;
                    data = reallocPtr;
                } else if (cc->min > dataSize) {
                    uint8_t *reallocPtr = realloc(data, cc->min);
                    if (reallocPtr == NULL) {
                        free(data);
                        listFree(entries);
                        byteStreamDestroy(innerStream);
                        byteStreamDestroy(stream);
                        id3v2DestroyFrameHeader(&header);
                        *frame = NULL;
                        return 0;
                    }
                    data = reallocPtr;
                    memset(data + dataSize, 0, cc->min - dataSize);
                    dataSize = cc->min;
                }

                listInsertBack(entries, id3v2CreateContentEntry(data, dataSize));
                free(data);

                expectedContentSize = ((expectedContentSize < dataSize) ? 0 : expectedContentSize - dataSize);
            }
            break;
            // only characters found within the latin1 character set
            case latin1Encoding_context: {
                uint8_t *data = NULL;
                size_t dataSize = 0;

                data = byteStreamReturnLatin1(innerStream, &dataSize);

                if (dataSize > cc->max) {
                    dataSize = cc->max;
                    uint8_t *reallocPtr = realloc(data, dataSize);
                    if (reallocPtr == NULL) {
                        free(data);
                        listFree(entries);
                        byteStreamDestroy(innerStream);
                        byteStreamDestroy(stream);
                        id3v2DestroyFrameHeader(&header);
                        *frame = NULL;
                        return 0;
                    }
                    data = reallocPtr;
                } else if (cc->min > dataSize) {
                    uint8_t *reallocPtr = realloc(data, cc->min);
                    if (reallocPtr == NULL) {
                        free(data);
                        listFree(entries);
                        byteStreamDestroy(innerStream);
                        byteStreamDestroy(stream);
                        id3v2DestroyFrameHeader(&header);
                        *frame = NULL;
                        return 0;
                    }
                    data = reallocPtr;
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
            case numeric_context: {
                uint8_t *data = NULL;
                size_t dataSize = 0;

                if (cc->min >= cc->max) {
                    // no trust
                    dataSize = cc->min;
                } else {
                    dataSize = cc->max;
                }

                if (dataSize > expectedContentSize) {
                    dataSize = expectedContentSize;
                }

                data = calloc(sizeof(uint8_t), dataSize);

                if (!byteStreamRead(innerStream, data, dataSize)) {
                    memset(data, 0, dataSize);
                }

                listInsertBack(entries, id3v2CreateContentEntry(data, dataSize));
                free(data);

                expectedContentSize = ((expectedContentSize < dataSize) ? 0 : expectedContentSize - dataSize);
            }
            break;
            case bit_context: {
                uint8_t *data = NULL;
                size_t nBits = 0;
                size_t dataSize = 0;
                Id3v2ContextType isBitContext = 0;

                if (cc->min >= cc->max) {
                    // no trust
                    nBits = cc->min;
                } else {
                    nBits = cc->max;
                }

                dataSize = (nBits + (CHAR_BIT - 1)) / CHAR_BIT;

                data = malloc(dataSize);
                memset(data, 0, dataSize);
                internal_copyNBits(byteStreamCursor(innerStream), data, (int) concurrentBitCount, (int) nBits);

                if (iter.current->data != NULL) {
                    Id3v2ContentContext *nextContext = (Id3v2ContentContext *) iter.current->data;
                    isBitContext = nextContext->type;
                }

                concurrentBitCount += nBits;

                if (isBitContext != bit_context) {
                    if (concurrentBitCount / CHAR_BIT) {
                        byteStreamSeek(innerStream, concurrentBitCount / CHAR_BIT, SEEK_CUR);
                        expectedContentSize = ((expectedContentSize < dataSize) ? 0 : expectedContentSize - dataSize);
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
                uint8_t *data = NULL;
                size_t dataSize = 0;

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
                byteStreamRead(innerStream, data, dataSize); // no need to check error code do to above memset

                listInsertBack(entries, id3v2CreateContentEntry(data, dataSize));
                free(data);

                expectedContentSize = ((expectedContentSize < dataSize) ? 0 : expectedContentSize - dataSize);
            }
            break;
            // no support
            case unknown_context:
            default:
                byteStreamSeek(innerStream, 0, SEEK_END);
                break;
        }

        if (expectedContentSize == 0 || byteStreamGetCh(innerStream) == EOF) {
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
 * @brief Parses a complete ID3v2 tag from a byte buffer, including header, optional extended header, and all frames.
 * @details Orchestrates the full tag parsing workflow by locating the "ID3" identifier, parsing headers, 
 * processing unsynchronisation if present, and iteratively parsing all frames using a multi-pass context 
 * resolution strategy. Returns a heap-allocated tag structure containing all successfully parsed components.
 * 
 * **Parsing Process:**
 * 1. Scans buffer to locate "ID3" magic number identifier
 * 2. Parses tag header (version, flags, size)
 * 3. If unsynchronisation flag set: strips $00 bytes following $FF
 * 4. If extended header flag set: parses version-specific extended header
 * 5. Iterates through frame data, parsing each frame using 4-pass context lookup
 * 
 * **Frame Context Resolution (4-Pass System):**
 * - Pass 1: Exact frame ID match in default context mappings (e.g., "TIT2", "APIC")
 * - Pass 2: Exact frame ID match in user-supplied custom mappings (userPairs parameter)
 * - Pass 3: Generic frame type patterns - 'T' prefix (text frames), 'W' prefix (URL frames)
 * - Pass 4: Fallback to generic binary context ('?') for unknown frame types
 * 
 * Parsing continues until all frames are extracted or an unrecoverable error occurs. On partial 
 * failure, returns a tag structure with successfully parsed frames. Returns NULL only on complete 
 * failure. Caller must free returned structure with id3v2DestroyTag.
 * 
 * @param in - Pointer to byte buffer containing ID3v2 tag data (may include non-tag data before/after).
 * @param inl - Size of input buffer in bytes.
 * @param userPairs - Optional hash table mapping frame IDs to custom context lists (NULL for default mappings only).
 * @return Id3v2Tag* - Heap-allocated complete tag structure on success, partial tag on partial failure, or NULL on complete failure.
 */
Id3v2Tag *id3v2ParseTagFromBuffer(uint8_t *in, size_t inl, HashTable *userPairs) {
    if (in == NULL || inl == 0) {
        return NULL;
    }

    ByteStream *stream = byteStreamCreate(in, inl);

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
        read = id3v2ParseTagHeader(byteStreamCursor(stream), stream->bufferSize - stream->cursor, &header, &tagSize);

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

        if ((header->majorVersion == ID3V2_TAG_VERSION_3 || header->majorVersion == ID3V2_TAG_VERSION_4) &&
            id3v2ReadExtendedHeaderIndicator(header) == 1) {
            read = id3v2ParseExtendedTagHeader(byteStreamCursor(stream), stream->bufferSize - stream->cursor,
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

            if (header->majorVersion == ID3V2_TAG_VERSION_3 || header->majorVersion == ID3V2_TAG_VERSION_4) {
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

            read = id3v2ParseFrame(byteStreamCursor(stream), stream->bufferSize - stream->cursor, context,
                                   header->majorVersion, &frame);

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
