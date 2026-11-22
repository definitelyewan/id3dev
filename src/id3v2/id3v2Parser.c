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
#include "src/id3Internal.h"
#include "id3dependencies/ByteStream/include/byteStream.h"
#include "id3dependencies/ByteStream/include/byteInt.h"
#include "id3dependencies/ByteStream/include/byteUnicode.h"

/**
 * @brief Parses an ID3v2 extended tag header from a byte buffer.
 * @details Decodes the extended header that follows the tag header and checks its structure against the
 * standard. Anything that doesn't match is treated as junk: 0 is returned and extendedTagHeader is set to NULL.
 *
 * - ID3v2.3: size (4, big-endian), flags (2), padding size (4), and a CRC-32 (4) when flag 0x8000 is set. The
 * size excludes its own 4 bytes, so it must be 6 without a CRC or 10 with one. Only the CRC flag may be set, and
 * the padding must fit in the bytes left after the extended header.
 *
 * - ID3v2.4: syncsafe size (4) covering the whole extended header, the number of flag bytes (always 1), the
 * flags byte (%0bcd0000), then for each set flag in order a length byte and its data: update $00, CRC $05 with
 * a 5 byte syncsafe CRC-32, restrictions $01 with 1 byte. The size must equal the bytes those fields take.
 *
 * Every version must also fit within inl. ID3v2.2 has no extended header and returns 0. On success, caller
 * must free the returned structure with id3v2DestroyExtendedTagHeader.
 *
 * @param in - Pointer to byte buffer containing extended header data.
 * @param inl - Bytes left in the tag from in onwards.
 * @param version - ID3v2 version (ID3V2_TAG_VERSION_3 or ID3V2_TAG_VERSION_4).
 * @param extendedTagHeader - Output parameter receiving pointer to heap-allocated extended header structure, or NULL on failure.
 * @return uint32_t - Number of bytes the extended header takes including its size field, 0 on failure or unsupported version.
 */
uint32_t id3v2ParseExtendedTagHeader(uint8_t *in, size_t inl, uint8_t version,
                                     Id3v2ExtendedTagHeader **extendedTagHeader) {
    uint32_t padding = 0;
    uint32_t crc = 0;
    bool update = false;
    bool tagRestrictions = false;
    uint8_t restrictions = 0;
    size_t total = 0;

    *extendedTagHeader = NULL;

    if (in == NULL || inl < 4) {
        return 0;
    }

    switch (version) {
        case ID3V2_TAG_VERSION_3: {
            const uint32_t size = ((uint32_t) in[0] << 24) | ((uint32_t) in[1] << 16) | ((uint32_t) in[2] << 8) | in[3];
            uint16_t flags = 0;
            bool hasCrc = false;

            if (size != 6 && size != 10) {
                return 0;
            }

            total = 4 + (size_t) size;
            if (total > inl) {
                return 0;
            }

            flags = (uint16_t) ((in[4] << 8) | in[5]);
            hasCrc = (flags & 0x8000) != 0;

            // only the CRC flag is defined and the size must match it
            if ((flags & 0x7FFF) != 0 || size != (hasCrc ? 10u : 6u)) {
                return 0;
            }

            padding = ((uint32_t) in[6] << 24) | ((uint32_t) in[7] << 16) | ((uint32_t) in[8] << 8) | in[9];
            if (padding > inl - total) {
                return 0;
            }

            if (hasCrc) {
                crc = ((uint32_t) in[10] << 24) | ((uint32_t) in[11] << 16) | ((uint32_t) in[12] << 8) | in[13];
            }

            break;
        }
        case ID3V2_TAG_VERSION_4: {
            uint32_t size = 0;
            size_t pos = 6;
            uint8_t flags = 0;

            for (int i = 0; i < 4; i++) {
                if (in[i] & 0x80) {
                    return 0;
                }
                size = (size << 7) | in[i];
            }

            if (size < 6 || size > inl) {
                return 0;
            }

            total = size;

            // one flag byte, %0bcd0000
            if (in[4] != 1) {
                return 0;
            }

            flags = in[5];
            if (flags & 0x8F) {
                return 0;
            }

            // each set flag carries a length byte then its data, in flag order
            if (flags & 0x40) {
                if (pos + 1 > total || in[pos] != 0x00) {
                    return 0;
                }
                update = true;
                pos += 1;
            }

            if (flags & 0x20) {
                if (pos + 6 > total || in[pos] != 0x05) {
                    return 0;
                }

                // 35 bits of syncsafe data hold a 32 bit CRC, so the first byte uses only its low 4 bits
                if (in[pos + 1] > 0x0F) {
                    return 0;
                }

                for (size_t i = pos + 1; i < pos + 6; i++) {
                    if (in[i] & 0x80) {
                        return 0;
                    }
                    crc = (crc << 7) | in[i];
                }

                pos += 6;
            }

            if (flags & 0x10) {
                if (pos + 2 > total || in[pos] != 0x01) {
                    return 0;
                }
                tagRestrictions = true;
                restrictions = in[pos + 1];
                pos += 2;
            }

            if (pos != total) {
                return 0;
            }

            break;
        }
        // no support
        default:
            return 0;
    }

    *extendedTagHeader = id3v2CreateExtendedTagHeader(padding, crc, update, tagRestrictions, restrictions);

    return (*extendedTagHeader == NULL) ? 0 : (uint32_t) total;
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
    uint8_t id[ID3V2_TAG_ID_SIZE];
    uint32_t idAsInt = 0;

    // check for magic number
    if (!byteStreamRead(stream, id, ID3V2_TAG_ID_SIZE)) {
        *tagHeader = NULL;
        return 0;
    }

    for (int i = 0; i < ID3V2_TAG_ID_SIZE; ++i) {
        idAsInt = (idAsInt << (uint8_t) 8) | id[i];
    }

    if (ID3V2_TAG_ID_MAGIC_NUMBER_H != idAsInt) {
        *tagHeader = NULL;
        return ID3V2_TAG_ID_SIZE;
    }


    while (true) {
        // versions
        if (!byteStreamRead(stream, &major, 1)) {
            break;
        }

        if (!byteStreamRead(stream, &minor, 1)) {
            break;
        }

        // flags
        if (!byteStreamRead(stream, &flags, 1)) {
            break;
        }

        // size is a syncsafe integer, so the top bit of every byte must be clear. Decoding masks those
        // bits off, so check the raw bytes first
        if (stream->bufferSize - stream->cursor < 4) {
            break;
        }

        if (((byteStreamCursor(stream)[0] | byteStreamCursor(stream)[1] | byteStreamCursor(stream)[2] |
              byteStreamCursor(stream)[3]) & 0x80) != 0) {
            *tagHeader = NULL;
            *tagSize = 0;
            byteStreamDestroy(stream);
            return 0;
        }

        hSize = byteStreamReturnSyncInt(stream);
        if (!hSize) {
            break;
        }

        break;
    }

    uint32_t walk = stream->cursor;
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
 * encryption (+1 byte), unsynchronisation, compression, data length indicator (+4 byte syncsafe
 * size, only when its own flag is set).
 *
 * The frameSize output represents the frame content size, excluding the header and any extra
 * header bytes added by format flags (both specs count those inside the size field). On success,
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
    bool compressed = false;
    bool dataLengthIndicator = false;
    uint32_t decompressionSize = 0;
    uint8_t encryptionSymbol = 0;
    uint8_t groupSymbol = 0;

    // values for parsing
    uint8_t sizeBytes[ID3V2_FRAME_ID_MAX_SIZE];
    uint8_t flagBytes[ID3V2_FRAME_FLAG_SIZE];
    uint32_t walk = 0;
    uint32_t baseHeaderSize = 0;
    uint32_t extraHeaderSize = 0;

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

            if (!byteStreamRead(stream, flagBytes, ID3V2_FRAME_FLAG_SIZE)) {
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

            // i - compression, the decompressed size is present only with it
            if (readBit(flagBytes[1], 7)) {
                compressed = true;
                decompressionSize = byteStreamReturnInt(stream);
                if (!decompressionSize) {
                    break;
                }
            }

            if (readBit(flagBytes[1], 6)) {
                if (!byteStreamRead(stream, &encryptionSymbol, 1)) {
                    break;
                }
            }

            if (readBit(flagBytes[1], 5)) {
                if (!byteStreamRead(stream, &groupSymbol, 1)) {
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

            if (!byteStreamRead(stream, flagBytes, ID3V2_FRAME_FLAG_SIZE)) {
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

            // k - compression adds no bytes of its own, the spec requires p alongside it
            if (readBit(flagBytes[1], 3)) {
                compressed = true;
            }

            if (readBit(flagBytes[1], 1)) {
                unsync = true;
            }

            // p - the data length indicator is present only when this flag is set,
            // compression and encryption do not imply it
            if (readBit(flagBytes[1], 0)) {
                dataLengthIndicator = true;
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

    walk = stream->cursor;
    byteStreamDestroy(stream);

    // the size field counts the extra header bytes (group, encryption, decompressed size or
    // data length indicator), so take them off to leave only the frame content
    baseHeaderSize = (version == ID3V2_TAG_VERSION_2) ? (ID3V2_FRAME_ID_MAX_SIZE - 1) * 2
                                                      : ID3V2_FRAME_BASE_HEADER_SIZE;
    extraHeaderSize = (walk > baseHeaderSize) ? walk - baseHeaderSize : 0;

    // extra bytes that do not fit inside the frame mean the header is not legitimate
    if (extraHeaderSize > tSize) {
        *frameHeader = NULL;
        *frameSize = 0;
        return walk;
    }

    *frameHeader = id3v2CreateFrameHeader(id, tagAlter, fileAlter, readOnly, unsync, compressed,
                                          dataLengthIndicator, decompressionSize, encryptionSymbol, groupSymbol);
    *frameSize = tSize - extraHeaderSize;
    // printf("[*] frameSize = %zu, walk = %zu stream->cursor = %zu\n", *frameSize, walk, stream->cursor); // debug info i dont wanna rewrite
    return walk;
}

/**
 * @brief Parses a frame entry of a fixed size using a provided context as parameters for interpretation.
 * @param streamSource - stream containing the data source to read from
 * @param cc - context parameters for the frame entry
 * @param upperBound - maximum allowed size for the resulting data
 * @param data - output parameter containing the parsed blob content
 * @return size_t
 */
static size_t internal_id3v2ParseFrameBlobContent(ByteStream *streamSource, const Id3v2ContentContext *cc,
                                                  const uint32_t upperBound, uint8_t **data) {

     size_t dataSize = 0;

     if (cc->min >= cc->max) {
         // no trust
         dataSize = cc->min;
     } else {
         dataSize = cc->max;
     }

     if (dataSize > upperBound) {
         dataSize = upperBound;
     }

     *data = calloc(sizeof(uint8_t), dataSize);

     if (!byteStreamRead(streamSource, *data, dataSize)) {
         memset(*data, 0, dataSize);
     }

     return dataSize;
}

/**
 * @brief Parses a latin1 encoded frame entry of a fixed size using a provided context as parameters for interpretation.
 * @param streamSource - stream containing the data source to read from
 * @param cc - context parameters for the frame entry
 * @param upperBound - the maximum allowed size for the resulting parsed data
 * @param data - output parameter containing the parsed latin1 encoded content
 * @return
 */
static size_t internal_id3v2ParseFrameLatin1Content(ByteStream * streamSource, const Id3v2ContentContext *cc,
                                                    const uint32_t upperBound, uint8_t **data) {

    size_t dataSize = 0;
    *data = byteStreamReturnLatin1(streamSource, &dataSize);
    // catch when the parsed size is greater than the contexts specified max
    if (dataSize > cc->max) {
        dataSize = cc->max;
        uint8_t *reallocPtr = realloc(*data, dataSize);
        if (reallocPtr == NULL) {
            free(*data);
            return 0;
        }
        *data = reallocPtr;

    // for when the parsed size is less than the minimum allowed by context
    } else if (cc->min > dataSize) {
        uint8_t *reallocPtr = realloc(*data, cc->min);
        if (reallocPtr == NULL) {
            free(*data);
            return 0;
        }
        *data = reallocPtr;
        memset(*data + dataSize, 0, cc->min - dataSize);
        dataSize = cc->min;

    // for when the parsed data is higher than the maximum upper bound
    } else if (dataSize > upperBound) {
        dataSize = upperBound + 1;
        uint8_t *reallocPtr = realloc(*data, dataSize);
        if (reallocPtr == NULL) {
            free(*data);
            return 0;
        }
        *data = reallocPtr;
        (*data)[dataSize - 1] = 0;
    }

    return dataSize;
}

/**
 * @brief Parses an encoded string field from an ID3v2 frame's inner byte stream.
 * @details Looks up the encoding byte from previously parsed content entries, then reads
 * a null-terminated string in the appropriate encoding (UTF-8/Latin-1 or UTF-16).
 * Clamps the result to [cc->min, cc->max] bytes, zero-padding if too short or truncating
 * if too long.
 *
 * @param innerStream     - Stream positioned at the start of the string field.
 * @param context         - The full context list for this frame (used to locate "encoding").
 * @param entries         - Content entries parsed so far (used to read the encoding byte value).
 * @param cc              - The context descriptor for this field (carries min/max size bounds).
 * @param data            - Output parameter receiving a heap-allocated buffer with the string bytes.
 *                          Caller must free. Set to NULL on failure.
 * @return size_t         - Number of bytes consumed on success, 0 on failure. Note that a
 *                          genuinely empty string will also return 0 — callers should check
 *                          that *data is non-NULL to distinguish the two cases.
 */
static size_t internal_id3v2ParseFrameEncodedStringContent(ByteStream *innerStream, List *context, List *entries,
                                                            Id3v2ContentContext *cc, size_t upperBound, uint8_t **data) {
    *data = NULL;
    size_t dataSize = 0;

    // Find the position of the "encoding" key in the context list,
    // skipping iter_context entries as they don't map to real content entries.
    size_t poscc = 0;
    ListIter contentContextIter = listCreateIterator(context);
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

    // Walk the already-parsed entries to find the encoding byte at poscc.
    uint8_t encoding = 0;
    size_t posce = 0;
    ListIter contentEntryIter = listCreateIterator(entries);
    while ((tmp = listIteratorNext(&contentEntryIter)) != NULL) {
        if (poscc == posce) {
            encoding = ((uint8_t *) ((Id3v2ContentEntry *) tmp)->entry)[0];
        }
        posce++;
    }

    // Read string bytes according to encoding.
    switch (encoding) {
        case ID3V2_ENCODING_ISO_8859_1:
        case ID3V2_ENCODING_ASCII:
        case ID3V2_ENCODING_UTF8:
            *data = byteStreamReturnUtf8(innerStream, &dataSize);
            if (*data == NULL && dataSize == 0) {
                byteStreamSeek(innerStream, 1, SEEK_CUR);
                *data = calloc(sizeof(unsigned char), 1);
                if (*data == NULL) { return 0; }
                dataSize = 1;
            }
            break;
        case ID3V2_ENCODING_UTF16BE:
        case ID3V2_ENCODING_UTF16LE:
            *data = byteStreamReturnUtf16(innerStream, &dataSize);
            if (*data == NULL && dataSize == 0) {
                byteStreamSeek(innerStream, 2, SEEK_CUR);
                *data = calloc(sizeof(unsigned char), 2);
                if (*data == NULL) { return 0; }
                dataSize = 2;
            }
            break;
        default:
            break;
    }

    // Clamp to [cc->min, cc->max].
    if (dataSize > cc->max) {
        uint8_t *reallocPtr = realloc(*data, cc->max);
        if (reallocPtr == NULL) {
            free(*data);
            *data = NULL;
            return 0;
        }
        dataSize = cc->max;
        *data = reallocPtr;
    } else if (cc->min > dataSize) {
        uint8_t *reallocPtr = realloc(*data, cc->min);
        if (reallocPtr == NULL) {
            free(*data);
            *data = NULL;
            return 0;
        }
        memset(reallocPtr + dataSize, 0, cc->min - dataSize);
        *data = reallocPtr;
        dataSize = cc->min;
    } else if (dataSize > upperBound) {
        dataSize = upperBound + 1;
        uint8_t *reallocPtr = realloc(*data, dataSize);
        if (reallocPtr == NULL) {
            free(*data);
            return 0;
        }
        *data = reallocPtr;
        (*data)[dataSize - 1] = 0;
    }

    return dataSize;
}

/**
 * @brief Parses an adjustment_context field from the inner stream.
 * @details The field is adjustmentBits bits wide and, as the standard requires, is stored in whole
 * bytes padded at the most significant end, so ceil(adjustmentBits / 8) bytes are read. A width of 0
 * yields no bytes (e.g. RVA2 "bits representing peak" of $00). The read is clamped to upperBound.
 *
 * @param innerStream - Stream to read from.
 * @param adjustmentBits - Width of the field in bits, taken from the most recent field keyed "adjustment".
 * @param upperBound - Remaining content bytes in the frame.
 * @param data - Out. Heap-allocated field bytes, or NULL when 0 bytes are read. Caller must free.
 * @return size_t - Number of bytes read.
 */
static size_t internal_id3v2ParseFrameAdjustmentContent(ByteStream *innerStream, size_t adjustmentBits,
                                                        size_t upperBound, uint8_t **data) {
    size_t dataSize = (adjustmentBits / CHAR_BIT) + ((adjustmentBits % CHAR_BIT) ? 1 : 0);

    *data = NULL;
    dataSize = (dataSize > upperBound) ? upperBound : dataSize;

    if (dataSize == 0) {
        return 0;
    }

    *data = calloc(dataSize, 1);
    if (*data == NULL) {
        return 0;
    }

    byteStreamRead(innerStream, *data, dataSize); // no need to check error code due to above calloc
    return dataSize;
}

static void internal_copyNBits(const unsigned char *src, unsigned char *dest,
                                int startBit, int nBits) {
    if (!src || nBits <= 0) return;

    for (int i = 0; i < nBits; i++) {
        int srcBitPos = startBit + i;
        int srcByte   = srcBitPos / CHAR_BIT;
        int srcOff    = (CHAR_BIT - 1) - (srcBitPos % CHAR_BIT); // MSB-first from src

        // pack so result is little-endian readable: last bits go in dest[0]
        int destBitPos = (nBits - 1) - i;
        int destByte   = destBitPos / CHAR_BIT;
        int destOff    = destBitPos % CHAR_BIT;                   // LSB-first within byte

        unsigned char bit = (src[srcByte] >> srcOff) & 0x01;
        dest[destByte] |= (bit << destOff);
    }
}

/**
 * @brief Parses a bit_context field from the inner stream.
 * @details Extracts nBits from the current stream position using concurrentBitCount as the bit offset. If the next
 * context in the iterator is not a bit_context, the stream cursor is flushed forward and concurrentBitCount is reset.
 * The extracted bits are right-justified in the returned buffer.
 *
 * @param innerStream - Stream to read from. Cursor only advances on flush.
 * @param cc - Current context entry, cc->min/max determines nBits.
 * @param iter - Live iterator; peeked (not advanced) to detect run end.
 * @param concurrentBitCount - In/out. Accumulated bit offset across consecutive bit_context fields.
 * @param expectedContentSize - In/out. Remaining content bytes, reduced on flush.
 * @param dataOut - Out. Heap-allocated buffer of extracted bits. Caller must free.
 * @return size_t - Size in bytes of the allocated buffer, or 0 on failure.
 */
static size_t internal_id3v2ParseFrameBitContent(ByteStream *innerStream, const Id3v2ContentContext *cc,
                                                const ListIter *iter, size_t *concurrentBitCount,
                                                uint32_t *expectedContentSize, uint8_t **dataOut) {

    if (!innerStream || !cc || !iter || !concurrentBitCount || !expectedContentSize || !dataOut) {
        return 0;
    }

    *dataOut = NULL;

    size_t nBits    = (cc->min >= cc->max) ? cc->min : cc->max;
    size_t dataSize = (nBits + (CHAR_BIT - 1)) / CHAR_BIT;

    uint8_t *data = calloc(dataSize, 1);
    if (!data) {
        return 0;
    }

    // use concurrentBitCount to find both which byte and which bit to start from
    size_t startByte = *concurrentBitCount / CHAR_BIT;
    size_t startBit  = *concurrentBitCount % CHAR_BIT;

    internal_copyNBits(byteStreamCursor(innerStream) + startByte, data, (int)startBit, (int)nBits);
    // reverse the byte order
    for (size_t i = 0; i < dataSize / 2; i++) {
        uint8_t tmp = data[i];
        data[i] = data[dataSize - 1 - i];
        data[dataSize - 1 - i] = tmp;
    }

    *concurrentBitCount += nBits;

    // peek at next context
    Id3v2ContextType nextType = unknown_context;
    if (iter->current && iter->current->data) {
        nextType = ((Id3v2ContentContext *)iter->current->data)->type;
    }

    // only advance stream when the bit run ends
    if (nextType != bit_context) {
        size_t flushBytes = *concurrentBitCount / CHAR_BIT;
        if (flushBytes) {
            byteStreamSeek(innerStream, (long)flushBytes, SEEK_CUR);
            *expectedContentSize = (*expectedContentSize < flushBytes) ? 0 : *expectedContentSize - flushBytes;
        }
        *concurrentBitCount = 0;
    }

    *dataOut = data;
    return dataSize;
}

/**
 * @brief Parses an ID3v2 frame (header + content) from a byte buffer using context-driven interpretation.
 * @details Extracts a complete frame by first parsing the header, then interpreting frame content 
 * according to a context list that provides "hints" about data types and structure. The context-driven 
 * approach allows flexible parsing of the diverse frame types in the ID3v2 specification.
 * 
 * **Encrypted/Compressed Frame Handling:**
 * Compressed or encrypted frames are parsed as raw binary using a generic 
 * context. The caller must decrypt/decompress and reparse to access structured content.
 * 
 * **Context Types Supported:**
 * - encodedString_context: Null-terminated strings with encoding determined by prior "encoding" byte 
 * (ISO-8859-1, UTF-8, UTF-16BE/LE)
 * - latin1Encoding_context: Null-terminated Latin-1 (ISO-8859-1) strings only
 * - binary_context/noEncoding_context/precision_context/numeric_context: Fixed-size raw binary/numeric data
 * - bit_context: Bit-level extraction, MSB-first across consecutive bit_contexts
 * - iter_context: Loop control marker for repeated field groups
 * - adjustment_context: Field whose width in bits comes from the most recently parsed "adjustment" field,
 * stored in ceil(bits / 8) bytes
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
    size_t adjustmentBits = 0;
    const size_t adjustmentKey = id3v2djb2("adjustment");
    size_t iterStartCursor = 0;
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

    // v2.4 frame sizes count the stored (unsynchronised) bytes, so advance by that
    // but parse the content from the resynchronised bytes
    const uint32_t storedContentSize = expectedContentSize;
    const bool frameUnsync = (version == ID3V2_TAG_VERSION_4 && header->unsynchronisation);

    if (frameUnsync && innerStream != NULL) {
        innerStream->bufferSize = id3v2RemoveUnsynchronisation(innerStream->buffer, innerStream->bufferSize);
        expectedContentSize = (uint32_t) innerStream->bufferSize;
    }


    // is a frame compressed or encrypted?
    // if so a generic context will be used meaning it's not up to me to decompress or unencrypted + if it's a text frame none
    // of the reads or writes will work until someone reparses the data after its changed.
    if (header->encryptionSymbol > 0 || header->compressed) {
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

        walk += frameUnsync ? storedContentSize : innerStream->cursor;
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

                dataSize = internal_id3v2ParseFrameEncodedStringContent(innerStream, context, entries, cc,
                    expectedContentSize, &data);

                if (data == NULL) {
                    listFree(entries);
                    byteStreamDestroy(innerStream);
                    byteStreamDestroy(stream);
                    id3v2DestroyFrameHeader(&header);
                    *frame = NULL;
                    return 0;
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

                dataSize = internal_id3v2ParseFrameLatin1Content(innerStream, cc, expectedContentSize, &data);

                // parsing or memmory error
                if (data == NULL && dataSize == 0) {
                    free(data);
                    listFree(entries);
                    byteStreamDestroy(innerStream);
                    byteStreamDestroy(stream);
                    id3v2DestroyFrameHeader(&header);
                    *frame = NULL;
                    return 0;
                }

                listInsertBack(entries, (void *) id3v2CreateContentEntry(data, dataSize));
                free(data);

                expectedContentSize = ((expectedContentSize < dataSize) ? 0 : expectedContentSize - dataSize);
                break;
            }
            // numbers (handled the same way)
            case binary_context:
            case noEncoding_context:
            case precision_context:
            case numeric_context: {
                uint8_t *data = NULL;
                size_t dataSize = 0;
                dataSize = internal_id3v2ParseFrameBlobContent(innerStream, cc, expectedContentSize, &data);

                // the most recent "adjustment" field sets the width of later adjustment_contexts
                if (cc->key == adjustmentKey) {
                    adjustmentBits = internal_id3v2AdjustmentBits(data, dataSize);
                }

                listInsertBack(entries, id3v2CreateContentEntry(data, dataSize));
                free(data);

                expectedContentSize = ((expectedContentSize < dataSize) ? 0 : expectedContentSize - dataSize);
            }
            break;
            case bit_context: {
                uint8_t *data   = NULL;
                size_t dataSize = internal_id3v2ParseFrameBitContent(
                                      innerStream, cc, &iter,
                                      &concurrentBitCount,
                                      &expectedContentSize,
                                      &data);
                if (!data) {
                    listFree(entries);
                    byteStreamDestroy(innerStream);
                    byteStreamDestroy(stream);
                    id3v2DestroyFrameHeader(&header);
                    *frame = NULL;
                    return 0;
                }

                listInsertBack(entries, id3v2CreateContentEntry(data, dataSize));
                free(data);
            }
            break;
            case iter_context: {
                // an iteration that read nothing would repeat forever (e.g. only 0 bit wide fields),
                // so leave the loop and carry on with whatever follows the iter_context
                if (currIterations != 0 && innerStream->cursor == iterStartCursor) {
                    iter = iterStorage;
                    currIterations = 0;
                    break;
                }

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
                iterStartCursor = innerStream->cursor;
            }
            break;
            case adjustment_context: {

                size_t dataSize = 0;
                uint8_t *data = NULL;
                dataSize = internal_id3v2ParseFrameAdjustmentContent(innerStream, adjustmentBits,
                                                                    expectedContentSize, &data);
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
            const Id3v2ContentContext *next = (iter.current != NULL) ? (Id3v2ContentContext *) iter.current->data : NULL;

            // a field declared 0 bits wide is present but empty (e.g. the last RVA2 channel with no peak),
            // so emit its entry before stopping. Fields wider than 0 bits with no data left are omitted.
            if (next != NULL && next->type == adjustment_context && adjustmentBits == 0) {
                continue;
            }

            break;
        }
    }


    // enforce frame size from header parsing
    walk += frameUnsync ? storedContentSize : innerStream->bufferSize;

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
    List *frames = listCreate(id3v2PrintFrame, id3v2DeleteFrame, id3v2CompareFrame, id3v2CopyFrame);

    // locate the start of the tag, checking every byte offset
    while (true) {
        // byteStreamRead allows short reads, so check a whole identifier is left
        if (stream->bufferSize - stream->cursor < ID3V2_TAG_ID_SIZE) {
            listFree(frames);
            byteStreamDestroy(stream);
            return NULL;
        }

        memcpy(id, byteStreamCursor(stream), ID3V2_TAG_ID_SIZE);

        if (btoi(id, ID3V2_TAG_ID_SIZE) == ID3V2_TAG_ID_MAGIC_NUMBER_H) {
            break;
        }

        stream->cursor++;
    }

    while (true) {
        read = id3v2ParseTagHeader(byteStreamCursor(stream), stream->bufferSize - stream->cursor, &header, &tagSize);

        // "ID3" followed by a header that is not legitimate is not a tag, e.g. the bytes appeared in audio
        if (read == 0 || header == NULL || !id3v2ValidateTagHeader(header)) {
            id3v2DestroyTagHeader(&header);
            listFree(frames);
            byteStreamDestroy(stream);
            return NULL;
        }

        if (tagSize == 0) {
            break;
        }

        if (!byteStreamSeek(stream, read, SEEK_CUR)) {
            break;
        }

        // shorten to exclude none tag data
        byteStreamResize(stream, stream->cursor + tagSize);

        // v2.4 unsynchronisation is applied per frame and frame sizes describe the stored bytes,
        // so only v2.2/v2.3 undo it over the whole tag body
        if (id3v2ReadUnsynchronisationIndicator(header) == 1 &&
            (header->majorVersion == ID3V2_TAG_VERSION_2 || header->majorVersion == ID3V2_TAG_VERSION_3)) {
            size_t bodySize = stream->bufferSize - stream->cursor;
            size_t decodedSize = id3v2RemoveUnsynchronisation(byteStreamCursor(stream), bodySize);

            stream->bufferSize = stream->cursor + decodedSize;
            tagSize = (uint32_t) decodedSize;
        }

        if ((header->majorVersion == ID3V2_TAG_VERSION_3 || header->majorVersion == ID3V2_TAG_VERSION_4) &&
            id3v2ReadExtendedHeaderIndicator(header) == 1) {
            read = id3v2ParseExtendedTagHeader(byteStreamCursor(stream), stream->bufferSize - stream->cursor,
                                               header->majorVersion, &ext);
            header->extendedHeader = ext;

            // the header promised an extended header, so a missing or illegitimate one means junk
            if (read == 0 || ext == NULL || !id3v2ValidateExtendedTagHeader(ext, header->majorVersion)) {
                id3v2DestroyTagHeader(&header);
                listFree(frames);
                byteStreamDestroy(stream);
                return NULL;
            }

            tagSize = ((tagSize < read) ? 0 : tagSize - read);
            byteStreamSeek(stream, read, SEEK_CUR);
        }

        HashTable *pairs = id3v2CreateDefaultIdentifierContextPairings(header->majorVersion);

        while (tagSize) {
            Id3v2Frame *frame = NULL;
            uint8_t frameId[ID3V2_FRAME_ID_MAX_SIZE + 1] = {0};

            if (header->majorVersion == ID3V2_TAG_VERSION_3 || header->majorVersion == ID3V2_TAG_VERSION_4) {
                if (!byteStreamRead(stream, frameId, ID3V2_FRAME_ID_MAX_SIZE)) {
                    exit = true;
                    break;
                }
                frameId[ID3V2_FRAME_ID_MAX_SIZE] = '\0';

                stream->cursor = stream->cursor - ID3V2_FRAME_ID_MAX_SIZE;
            } else if (header->majorVersion == ID3V2_TAG_VERSION_2) {
                if (!byteStreamRead(stream, frameId, ID3V2_FRAME_ID_MAX_SIZE - 1)) {
                    exit = true;
                    break;
                }
                frameId[ID3V2_FRAME_ID_MAX_SIZE - 1] = '\0';

                stream->cursor = stream->cursor - (ID3V2_FRAME_ID_MAX_SIZE - 1);
            }

            // check default pairings (pass 1/4)
            List *context = hashTableRetrieve(pairs, (char *) frameId);

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
