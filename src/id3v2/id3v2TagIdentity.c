/**
 * @file id3v2TagIdentity.c
 * @author Ewan Jones
 * @brief Function implementations of ID3v2 tag header and tag structure creation, manipulation, serialization, and JSON conversion
 * @version 26.01
 * @date 2024-04-11 - 2026-01-13
 * 
 * @copyright Copyright (c) 2024 - 2026
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v2/id3v2TagIdentity.h"
#include "id3dependencies/ByteStream/include/byteStream.h"
#include "id3dependencies/ByteStream/include/byteInt.h"

/**
 * @brief Creates and allocates an ID3v2 tag header structure.
 * @details Allocates memory and initializes header fields with provided version, flags, and 
 * optional extended header. Caller must free with id3v2DestroyTagHeader.
 * @param majorVersion - Major version number (2, 3, or 4)
 * @param minorVersion - Minor version/revision number
 * @param flags - Tag header flags byte (%abcd0000)
 * @param extendedHeader - Pointer to extended header structure.
 * @return Id3v2TagHeader* - Heap-allocated tag header structure.
 */
Id3v2TagHeader *id3v2CreateTagHeader(uint8_t majorVersion, uint8_t minorVersion, uint8_t flags,
                                     Id3v2ExtendedTagHeader *extendedHeader) {
    Id3v2TagHeader *header = malloc(sizeof(Id3v2TagHeader));

    header->majorVersion = majorVersion;
    header->minorVersion = minorVersion;
    header->flags = flags;
    header->extendedHeader = extendedHeader;

    return header;
}

/**
 * @brief Frees an ID3v2 tag header and nullifies the pointer.
 * @details Recursively destroys extended header if present, deallocates header memory, 
 * and sets pointer to NULL to prevent dangling pointer issues.
 * @param toDelete - Pointer to the tag header pointer to free.
 */
void id3v2DestroyTagHeader(Id3v2TagHeader **toDelete) {
    if (*toDelete) {
        id3v2DestroyExtendedTagHeader(&((*toDelete)->extendedHeader));
        free(*toDelete);
        *toDelete = NULL;
        toDelete = NULL;
    }
}

/**
 * @brief Sets or clears the unsynchronisation flag in the tag header.
 * @details Modifies bit 7 of the header flags byte. Unsynchronisation prevents false sync patterns in tag data.
 * @param header - The tag header to modify.
 * @param bit - true to set unsynchronisation flag, false to clear it.
 * @return bool - true on success, false on failure.
 */
bool id3v2WriteUnsynchronisationIndicator(Id3v2TagHeader *header, bool bit) {
    if (!header) {
        return 0;
    }

    //set bit 8
    header->flags = setBit(header->flags, 7, bit);
    return 1;
}

/**
 * @brief Sets or clears the compression flag in ID3v2.2 tag headers.
 * @details Modifies bit 6 of the header flags byte. Only applicable to ID3v2.2 tags; 
 * returns failure for other versions as compression moved to frame-level in v2.3+.
 * @param header - The tag header to modify.
 * @param bit - true to set compression flag, false to clear it.
 * @return bool - true on success, false on failure.
 */
bool id3v2WriteCompressionIndicator(Id3v2TagHeader *header, bool bit) {
    if (!header) {
        return 0;
    }

    if (header->majorVersion == ID3V2_TAG_VERSION_2) {
        header->flags = setBit(header->flags, 6, bit);
        return 1;
    }

    return 0;
}

/**
 * @brief Sets or clears the extended header flag in ID3v2.3/ID3v2.4 tag headers.
 * @details Modifies bit 6 of the header flags byte. Only applicable to ID3v2.3 and ID3v2.4 tags; 
 * returns failure for other versions as ID3v2.2 does not support extended headers.
 * @param header - The tag header to modify.
 * @param bit - true to set extended header flag, false to clear it.
 * @return bool - true on success, false on failure
 */
bool id3v2WriteExtendedHeaderIndicator(Id3v2TagHeader *header, bool bit) {
    if (!header) {
        return 0;
    }

    //set bit 7 or do nothing
    switch (header->majorVersion) {
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
 * @brief Sets or clears the experimental indicator flag in ID3v2.3/ID3v2.4 tag headers.
 * @details Modifies bit 5 of the header flags byte. Marks tag as experimental/testing. Only applicable 
 * to ID3v2.3 and ID3v2.4 tags; returns failure for other versions.
 * @param header - The tag header to modify.
 * @param bit - true to set experimental flag, false to clear it.
 * @return bool - true on success, false on failure.
 */
bool id3v2WriteExperimentalIndicator(Id3v2TagHeader *header, bool bit) {
    if (!header) {
        return 0;
    }

    //set bit 6 or do nothing
    switch (header->majorVersion) {
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
 * @brief Sets or clears the footer presence flag in ID3v2.4 tag headers.
 * @details Modifies bit 4 of the header flags byte. Footer is a 10-byte structure at tag 
 * end mirroring the header with reversed "3DI" identifier. Only applicable to ID3v2.4 tags.
 * @param header - The tag header to modify.
 * @param bit - true to set footer flag, false to clear it.
 * @return bool - true on success, false on failure.
 */
bool id3v2WriteFooterIndicator(Id3v2TagHeader *header, bool bit) {
    if (!header) {
        return 0;
    }

    switch (header->majorVersion) {
        case ID3V2_TAG_VERSION_4:
            header->flags = setBit(header->flags, 4, bit);
            return 1;
        default:
            break;
    }

    return 0;
}

/**
 * @brief Reads the unsynchronisation flag from a tag header.
 * @details Extracts bit 7 of the header flags byte. Valid for all ID3v2 versions (2.2, 2.3, 2.4).
 * @param header - The tag header to read from.
 * @return int - 0 or 1 if successful, -1 on failure.
 */
int id3v2ReadUnsynchronisationIndicator(Id3v2TagHeader *header) {
    if (!header) {
        return -1;
    }

    return readBit(header->flags, 7);
}

/**
 * @brief Reads the compression flag from an ID3v2.2 tag header.
 * @details Extracts bit 6 of the header flags byte. Only valid for ID3v2.2 tags.
 * @param header - The tag header to read from.
 * @return int - 0 or 1 if successful, -1 on failure.
 */
int id3v2ReadCompressionIndicator(Id3v2TagHeader *header) {
    if (!header) {
        return -1;
    }

    if (header->majorVersion != ID3V2_TAG_VERSION_2) {
        return -1;
    }

    return readBit(header->flags, 6);
}

/**
 * @brief Reads the extended header flag from an ID3v2.3/ID3v2.4 tag header.
 * @details Extracts bit 6 of the header flags byte. Only valid for ID3v2.3 and ID3v2.4 tags 
 * as ID3v2.2 does not support extended headers.
 * @param header - The tag header to read from.
 * @return int - 0 or 1 if successful, -1 on failure.
 */
int id3v2ReadExtendedHeaderIndicator(Id3v2TagHeader *header) {
    if (!header) {
        return -1;
    }

    switch (header->majorVersion) {
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return readBit(header->flags, 6);
        default:
            break;
    }

    return -1;
}

/**
 * @brief Reads the experimental indicator flag from an ID3v2.3/ID3v2.4 tag header.
 * @details Extracts bit 5 of the header flags byte. The experimental indicator marks 
 * a tag as being in an experimental or testing state. Only valid for ID3v2.3 and ID3v2.4 
 * tags as this flag is not available in ID3v2.2.
 * @param header - The tag header to read from.
 * @return int - 0 or 1 if successful, -1 on failure.
 */
int id3v2ReadExperimentalIndicator(Id3v2TagHeader *header) {
    if (!header) {
        return -1;
    }

    switch (header->majorVersion) {
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return readBit(header->flags, 5);
        default:
            break;
    }

    return -1;
}

/**
 * @brief Reads the footer presence flag from an ID3v2.4 tag header.
 * @details Extracts bit 4 of the header flags byte. Only valid 
 * for ID3v2.4 tags as earlier versions do not support footers.
 * @param header - The tag header to read from.
 * @return int - 0 or 1 if successful, -1 on failure.
 */
int id3v2ReadFooterIndicator(Id3v2TagHeader *header) {
    if (!header) {
        return -1;
    }

    if (header->majorVersion != ID3V2_TAG_VERSION_4) {
        return -1;
    }

    return readBit(header->flags, 4);
}

/**
 * @brief Creates and allocates an ID3v2 extended tag header structure.
 * @details Allocates memory and initializes extended header members with provided padding, 
 * CRC, update flag, and restrictions. Extended headers contain optional metadata used in 
 * ID3v2.3 and ID3v2.4 tags. Caller must free with id3v2DestroyExtendedTagHeader.
 * @param padding - Padding size in bytes (ID3v2.3) or unused (ID3v2.4).
 * @param crc - CRC-32 checksum value for data integrity verification (0 if not used).
 * @param update - Update flag indicating tag is an update of previous tag (ID3v2.4 only).
 * @param tagRestrictions - Whether tag restrictions are present (ID3v2.4 only).
 * @param restrictions - Restrictions byte defining tag size, text encoding, and image limitations.
 * @return Id3v2ExtendedTagHeader* - Heap-allocated extended header structure.
 */
Id3v2ExtendedTagHeader *id3v2CreateExtendedTagHeader(uint32_t padding, uint32_t crc, bool update, bool tagRestrictions,
                                                     uint8_t restrictions) {
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
 * @brief Frees an ID3v2 extended tag header and nullifies the pointer.
 * @details Deallocates extended header memory and sets pointer to NULL to prevent 
 * dangling pointer issues.
 * @param toDelete - Pointer to the extended tag header pointer to free.
 */
void id3v2DestroyExtendedTagHeader(Id3v2ExtendedTagHeader **toDelete) {
    //error address free
    if (*toDelete) {
        free(*toDelete);
        *toDelete = NULL;
        toDelete = NULL;
    }
}

/**
 * @brief Sets the tag size restriction in an ID3v2.4 extended header.
 * @details Modifies bits 6-7 of the restrictions byte to set maximum tag size limitations. 
 * The 2-bit value defines four restriction levels (0x00-0x03). Only applicable to ID3v2.4 tags. 
 * Automatically creates an extended header if not present and enables the tagRestrictions flag.
 * @param header - The tag header to modify.
 * @param bits - Restriction level (0x00-0x03): defines maximum allowed tag size.
 * @return bool - true on success, false on failure.
 */
bool id3v2WriteTagSizeRestriction(Id3v2TagHeader *header, uint8_t bits) {
    //0b00, 0b01, 0b10, 0b11 making the max option 0x03
    if (!header || bits > 0x03) {
        return false;
    }

    if (header->majorVersion != ID3V2_TAG_VERSION_4) {
        return false;
    }

    if (!header->extendedHeader) {
        Id3v2ExtendedTagHeader *extendedHeader = id3v2CreateExtendedTagHeader(0, 0, 0, 0, 0);
        header->extendedHeader = extendedHeader;
    }

    // I don't care it will get set every time
    header->extendedHeader->tagRestrictions = true;

    header->extendedHeader->restrictions = setBit(header->extendedHeader->restrictions, 6, readBit(bits, 0));
    header->extendedHeader->restrictions = setBit(header->extendedHeader->restrictions, 7, readBit(bits, 1));

    return true;
}

/**
 * @brief Sets the text encoding restriction in an ID3v2.4 extended header.
 * @details Modifies bit 5 of the restrictions byte to limit allowed text encodings. 
 * When set, restricts text encoding to ISO-8859-1 or UTF-8 only. Only applicable to 
 * ID3v2.4 tags. Automatically creates an extended header if not present and enables 
 * the tagRestrictions flag.
 * @param header - The tag header to modify.
 * @param bit - true to enable text encoding restriction, false to disable.
 * @return bool - true on success, false on failure.
 */
bool id3v2WriteTextEncodingRestriction(Id3v2TagHeader *header, bool bit) {
    if (!header) {
        return false;
    }

    if (header->majorVersion != ID3V2_TAG_VERSION_4) {
        return false;
    }

    if (!header->extendedHeader) {
        Id3v2ExtendedTagHeader *extendedHeader = id3v2CreateExtendedTagHeader(0, 0, 0, 0, 0);
        header->extendedHeader = extendedHeader;
    }

    //will set as true every time
    header->extendedHeader->tagRestrictions = true;

    header->extendedHeader->restrictions = setBit(header->extendedHeader->restrictions, 5, bit);

    return true;
}

/**
 * @brief Sets the text encoding restriction in an ID3v2.4 extended header.
 * @details Modifies bit 5 of the restrictions byte to limit allowed text encodings. 
 * When set, restricts text encoding to ISO-8859-1 or UTF-8 only. Only applicable to 
 * ID3v2.4 tags. Automatically creates an extended header if not present and enables 
 * the tagRestrictions flag.
 * @param header - The tag header to modify.
 * @param bit - true to enable text encoding restriction, false to disable.
 * @return bool - true on success, false on failure.
 */
bool id3v2WriteTextFieldsSizeRestriction(Id3v2TagHeader *header, uint8_t bits) {
    //0b00, 0b01, 0b10, 0b11 making the max option 0x03
    if (!header || bits > 0x03) {
        return false;
    }

    if (header->majorVersion != ID3V2_TAG_VERSION_4) {
        return false;
    }

    if (!header->extendedHeader) {
        Id3v2ExtendedTagHeader *extendedHeader = id3v2CreateExtendedTagHeader(0, 0, 0, 0, 0);
        header->extendedHeader = extendedHeader;
    }

    // I don't care it will get set every time
    header->extendedHeader->tagRestrictions = true;

    header->extendedHeader->restrictions = setBit(header->extendedHeader->restrictions, 4, readBit(bits, 1));
    header->extendedHeader->restrictions = setBit(header->extendedHeader->restrictions, 3, readBit(bits, 0));

    return true;
}

/**
 * @brief Sets the image encoding restriction in an ID3v2.4 extended header.
 * @details Modifies bit 2 of the restrictions byte to limit allowed image formats. 
 * When set, restricts embedded images to PNG [PNG] or JPEG [JFIF] format only. Only applicable to 
 * ID3v2.4 tags. Automatically creates an extended header if not present and enables 
 * the tagRestrictions flag.
 * @param header - The tag header to modify.
 * @param bit - true to enable image encoding restriction, false to disable.
 * @return bool - true on success, false on failure.
 */
bool id3v2WriteImageEncodingRestriction(Id3v2TagHeader *header, bool bit) {
    if (!header) {
        return false;
    }

    if (header->majorVersion != ID3V2_TAG_VERSION_4) {
        return false;
    }

    if (!header->extendedHeader) {
        Id3v2ExtendedTagHeader *extendedHeader = id3v2CreateExtendedTagHeader(0, 0, 0, 0, 0);
        header->extendedHeader = extendedHeader;
    }

    //will set as true every time
    header->extendedHeader->tagRestrictions = true;

    header->extendedHeader->restrictions = setBit(header->extendedHeader->restrictions, 2, bit);

    return true; //avoid warning
}

/**
 * @brief Sets the image size restriction in an ID3v2.4 extended header.
 * @details Modifies bits 0-1 of the restrictions byte to set maximum image dimension limitations. 
 * The 2-bit value defines four restriction levels (0x00-0x03) for embedded image sizes. Only 
 * applicable to ID3v2.4 tags. Automatically creates an extended header if not present and enables 
 * the tagRestrictions flag.
 * @param header - The tag header to modify.
 * @param bits - Restriction level (0x00-0x03): defines maximum allowed image dimensions.
 * @return bool - true on success, false on failure.
 */
bool id3v2WriteImageSizeRestriction(Id3v2TagHeader *header, uint8_t bits) {
    //0b00, 0b01, 0b10, 0b11 making the max option 0x03
    if (!header || bits > 0x03) {
        return false;
    }

    if (header->majorVersion != ID3V2_TAG_VERSION_4) {
        return false;
    }

    if (!header->extendedHeader) {
        Id3v2ExtendedTagHeader *extendedHeader = id3v2CreateExtendedTagHeader(0, 0, 0, 0, 0);
        header->extendedHeader = extendedHeader;
    }

    // I don't care it will get set every time
    header->extendedHeader->tagRestrictions = true;

    header->extendedHeader->restrictions = setBit(header->extendedHeader->restrictions, 1, readBit(bits, 1));
    header->extendedHeader->restrictions = setBit(header->extendedHeader->restrictions, 0, readBit(bits, 0));

    return true;
}

/**
 * @brief Reads the tag size restriction from an ID3v2.4 extended header.
 * @details Extracts bits 6-7 from the restrictions byte and combines them into a 2-bit 
 * value (0x00-0x03) indicating the maximum tag size restriction level. Requires a valid 
 * extended header with restrictions enabled.
 * @param header - The tag header to read from.
 * @return int - Restriction level (0-3) on success, -1 on failure.
 */
int id3v2ReadTagSizeRestriction(Id3v2TagHeader *header) {
    if (!header) {
        return -1;
    }

    if (!header->extendedHeader) {
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
 * @brief Reads the text encoding restriction from an ID3v2.4 extended header.
 * @details Extracts bit 5 from the restrictions byte. When set (1), text encodings are 
 * restricted to ISO-8859-1 or UTF-8 only. Requires a valid extended header with restrictions enabled.
 * @param header - The tag header to read from.
 * @return int - 0 or 1 on success, -1 on failure.
 */
int id3v2ReadTextEncodingRestriction(Id3v2TagHeader *header) {
    if (!header) {
        return -1;
    }

    if (!header->extendedHeader) {
        return -1;
    }

    return readBit(header->extendedHeader->restrictions, 5);
}

/**
 * @brief Reads the text fields size restriction from an ID3v2.4 extended header.
 * @details Extracts bits 3-4 from the restrictions byte and combines them into a 2-bit 
 * value (0x00-0x03) indicating the maximum text field size restriction level. Requires a 
 * valid extended header with restrictions enabled.
 * @param header - The tag header to read from.
 * @return int - Restriction level (0-3) on success, -1 on failure.
 */
int id3v2ReadTextFieldsSizeRestriction(Id3v2TagHeader *header) {
    if (!header) {
        return -1;
    }

    if (!header->extendedHeader) {
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
 * @brief Reads the image encoding restriction from an ID3v2.4 extended header.
 * @details Extracts bit 2 from the restrictions byte. When set (1), embedded images are 
 * restricted to PNG [PNG] or JPEG [JFIF] format only. Requires a valid extended header 
 * with restrictions enabled.
 * @param header - The tag header to read from.
 * @return int - 0 or 1 on success, -1 on failure.
 */
int id3v2ReadImageEncodingRestriction(Id3v2TagHeader *header) {
    if (!header) {
        return -1;
    }

    if (!header->extendedHeader) {
        return -1;
    }

    return readBit(header->extendedHeader->restrictions, 2);
}

/**
 * @brief Reads the image size restriction from an ID3v2.4 extended header.
 * @details Extracts bits 0-1 from the restrictions byte and combines them into a 2-bit 
 * value (0x00-0x03) indicating the maximum image dimension restriction level. Requires a 
 * valid extended header with restrictions enabled.
 * @param header - The tag header to read from.
 * @return int - Restriction level (0-3) on success, -1 on failure.
 */
int id3v2ReadImageSizeRestriction(Id3v2TagHeader *header) {
    if (!header) {
        return -1;
    }

    if (!header->extendedHeader) {
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
 * @brief Clears all tag restrictions from an ID3v2.4 extended header.
 * @details Resets the tagRestrictions flag to false and zeroes the restrictions byte, 
 * removing all tag size, text encoding, text field size, image encoding, and image size 
 * restrictions. Requires a valid extended header to be present. Only applicable to ID3v2.4 
 * tags as earlier versions do not support restrictions.
 * @param header - The tag header to modify.
 * @return bool - true on success, false on failure.
 */
bool id3v2ClearTagRestrictions(Id3v2TagHeader *header) {
    if (!header) {
        return 0;
    }

    if (!header->extendedHeader) {
        return 0;
    }

    header->extendedHeader->tagRestrictions = false;
    header->extendedHeader->restrictions = 0;

    return 1;
}

/**
 * @brief Creates and allocates an ID3v2 tag structure.
 * @details Allocates memory and initializes a complete ID3v2 tag with the provided 
 * header and frame list while taking ownership. The tag structure combines the tag header 
 * with the list of data frames. Caller must free with id3v2DestroyTag.
 * @param header - Pointer to an initialized tag header structure.
 * @param frames - Pointer to a list containing ID3v2 frame structures.
 * @return Id3v2Tag* - Heap-allocated tag structure.
 */
Id3v2Tag *id3v2CreateTag(Id3v2TagHeader *header, List *frames) {
    Id3v2Tag *tag = malloc(sizeof(Id3v2Tag));

    tag->frames = frames;
    tag->header = header;

    return tag;
}

/**
 * @brief Frees an ID3v2 tag and all associated resources, nullifying the pointer.
 * @details Recursively destroys the tag header (including extended header if present), 
 * frees the frames list and all frame contents, deallocates the tag structure memory, 
 * and sets the pointer to NULL to prevent dangling pointer issues.
 * @param toDelete - Pointer to the tag pointer to free.
 */
void id3v2DestroyTag(Id3v2Tag **toDelete) {
    if (*toDelete) {
        id3v2DestroyTagHeader(&(*toDelete)->header);
        listFree((*toDelete)->frames);
        free(*toDelete);
        *toDelete = NULL;
        toDelete = NULL;
    }
}

/**
 * @brief Serializes an ID3v2 extended tag header into a byte stream.
 * @details Converts an extended header structure into its binary representation according 
 * to the ID3v2 specification. Returns a dynamically allocated byte array that must be freed 
 * by the caller. Handles version-specific formats:
 * 
 * - ID3v2.3: Writes 10+ bytes containing size (4), flags (1+2 reserved), padding size (4), 
 * and optional CRC-32 (4).
 * 
 * - ID3v2.4: Writes 6+ bytes containing extended size (4), flag bytes count (1), flags 
 * (update/CRC/restrictions in 1 byte), optional syncsafe CRC-32 (5), and optional 
 * restrictions byte (1).
 * 
 * ID3v2.2 does not support extended headers and returns NULL.
 * 
 * @param ext - Pointer to the extended header structure to serialize.
 * @param version - ID3v2 version (ID3V2_TAG_VERSION_2, ID3V2_TAG_VERSION_3, or ID3V2_TAG_VERSION_4) determining serialization format.
 * @param outl - Output parameter receiving the size of the returned byte array.
 * @return uint8_t* - Dynamically allocated byte array containing serialized header, or NULL on failure. Caller must free returned memory.
 */
uint8_t *id3v2ExtendedTagHeaderSerialize(Id3v2ExtendedTagHeader *ext, uint8_t version, size_t *outl) {
    ByteStream *stream = NULL;
    uint8_t *out = NULL;
    int buildSize = 0;
    unsigned char *tmp = NULL;
    unsigned char crcb[5] = {0, 0, 0, 0, 0};

    if (ext == NULL) {
        *outl = 0;
        return NULL;
    }

    switch (version) {
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

            if (ext->crc) {
                tmp = u32tob(ext->crc);
                byteStreamWrite(stream, tmp, 4);
                free(tmp);
            }

            break;

        case ID3V2_TAG_VERSION_4:

            buildSize = 6 + ((ext->crc) ? 5 : 0) + ((ext->tagRestrictions) ? 1 : 0);

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
            if (ext->crc) {
                int offset = 0;
                int toWrite = 0;
                tmp = sttob(byteSyncintEncode(ext->crc));

                while (offset < sizeof(size_t) && tmp[offset] == 0) {
                    offset++;
                }

                // c4c is 5 bytes if the 0s are there they must be kept
                if (offset > 3) {
                    offset = 3;
                }

                toWrite = ((int) (sizeof(size_t) - offset) > 5) ? 5 : (int) (sizeof(size_t) - offset);
                memcpy(crcb, tmp + offset, toWrite);

                byteStreamSeek(stream, 6, SEEK_SET);
                byteStreamWrite(stream, crcb, 5);
                free(tmp);
            }

            if (ext->tagRestrictions) {
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
 * @brief Converts an ID3v2 extended tag header structure to JSON string representation.
 * @details Creates a dynamically allocated JSON string containing the extended header's 
 * fields according to the specified ID3v2 version. Returns an empty JSON object "{}" if 
 * the header is NULL or the version is unsupported. The caller must free the returned string.
 * 
 * - ID3v2.3: Returns JSON with padding and crc fields.
 *   Example: {"padding":100,"crc":12345678}
 * 
 * - ID3v2.4: Returns JSON with padding, crc, update flag, tagRestrictions flag, and 
 * restrictions byte value.
 *   Example: {"padding":0,"crc":0,"update":false,"tagRestrictions":true,"restrictions":64}
 * 
 * - ID3v2.2 and unsupported versions: Returns empty JSON object "{}".
 * 
 * @param ext - Pointer to the extended header structure to convert (may be NULL).
 * @param version - ID3v2 version (ID3V2_TAG_VERSION_2, ID3V2_TAG_VERSION_3, or ID3V2_TAG_VERSION_4) determining which fields to include.
 * @return char* - Dynamically allocated JSON string. Caller must free the returned memory.
 */
char *id3v2ExtendedTagHeaderToJSON(const Id3v2ExtendedTagHeader *ext, uint8_t version) {
    char *json = NULL;
    size_t memCount = 3;

    if (ext == NULL) {
        json = calloc(memCount, sizeof(char));
        if (json == NULL) {
            return NULL;
        }
        memcpy(json, "{}\0", memCount);
        return json;
    }

    switch (version) {
        case ID3V2_TAG_VERSION_3:

            memCount += snprintf(NULL, 0,
                                 "{\"padding\":%"PRIu32",\"crc\":%"PRIu32"}",
                                 ext->padding,
                                 ext->crc);

            json = calloc(memCount + 1, sizeof(char)); // NOLINT(clang-analyzer-unix.Malloc)
            if (json == NULL) {
                return NULL;
            }

            (void) snprintf(json, memCount + 1,
                            "{\"padding\":%"PRIu32",\"crc\":%"PRIu32"}",
                            ext->padding,
                            ext->crc);

            break;
        case ID3V2_TAG_VERSION_4:

            memCount += snprintf(NULL, 0,
                                 "{\"padding\":%"PRIu32",\"crc\":%"PRIu32
                                 ",\"update\":%s,\"tagRestrictions\":%s,\"restrictions\":%d}",
                                 ext->padding,
                                 ext->crc,
                                 ext->update ? "true" : "false",
                                 ext->tagRestrictions ? "true" : "false",
                                 ext->restrictions);

            json = calloc(memCount + 1, sizeof(char)); // NOLINT(clang-analyzer-unix.Malloc)
            if (json == NULL) {
                return NULL;
            }

            (void) snprintf(json, memCount + 1,
                            "{\"padding\":%"PRIu32",\"crc\":%"PRIu32
                            ",\"update\":%s,\"tagRestrictions\":%s,\"restrictions\":%d}",
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
 * @brief Serializes an ID3v2 tag header into a byte stream.
 * @details Converts a tag header structure into its binary representation per the ID3v2 
 * specification. Returns a dynamically allocated byte array containing:
 * 
 * "ID3" identifier (3) + major version (1) + minor version (1) + flags (1) + 
 * syncsafe tag size (4) + optional extended header (variable).
 * 
 * Version-specific flags written: v2.2 (unsync, compression), v2.3 (unsync, extended header, 
 * experimental), v2.4 (unsync, extended header, experimental, footer). Returns NULL for 
 * versions > 2.4 or NULL input.
 * 
 * @param header - Pointer to the tag header structure to serialize.
 * @param uintSize - Tag size in bytes (excluding 10-byte header), encoded as syncsafe integer.
 * @param outl - Output parameter receiving total size of returned byte array.
 * @return uint8_t* - Dynamically allocated byte array containing serialized header, or NULL on failure. Caller must free.
 */
uint8_t *id3v2TagHeaderSerialize(Id3v2TagHeader *header, uint32_t uintSize, size_t *outl) {
    ByteStream *stream = NULL;
    uint8_t *out = NULL;
    unsigned char *tmp = NULL;

    if (header == NULL) {
        *outl = 0;
        return NULL;
    }

    // no support
    if (header->majorVersion > ID3V2_TAG_VERSION_4) {
        *outl = 0;
        return NULL;
    }

    stream = byteStreamCreate(NULL, 10);

    byteStreamWrite(stream, (uint8_t *) "ID3", ID3V2_TAG_ID_SIZE);

    byteStreamWrite(stream, &header->majorVersion, 1);
    byteStreamWrite(stream, &header->minorVersion, 1);


    switch ((int) header->majorVersion) {
        case ID3V2_TAG_VERSION_2:

            byteStreamWriteBit(stream, (bool) id3v2ReadUnsynchronisationIndicator(header), 7);
            byteStreamWriteBit(stream, (bool) id3v2ReadCompressionIndicator(header), 6);

            break;
        case ID3V2_TAG_VERSION_3:

            byteStreamWriteBit(stream, (bool) id3v2ReadUnsynchronisationIndicator(header), 7);
            byteStreamWriteBit(stream, (bool) id3v2ReadExtendedHeaderIndicator(header), 6);
            byteStreamWriteBit(stream, (bool) id3v2ReadExperimentalIndicator(header), 5);

            break;
        case ID3V2_TAG_VERSION_4:

            byteStreamWriteBit(stream, (bool) id3v2ReadUnsynchronisationIndicator(header), 7);
            byteStreamWriteBit(stream, (bool) id3v2ReadExtendedHeaderIndicator(header), 6);
            byteStreamWriteBit(stream, (bool) id3v2ReadExperimentalIndicator(header), 5);
            byteStreamWriteBit(stream, (bool) id3v2ReadFooterIndicator(header), 4);

            break;
        // dummy break as header version is already checked to be less than 4
        default:
            break;
    }

    byteStreamSeek(stream, 1, SEEK_CUR);

    tmp = u32tob(byteSyncintEncode(uintSize));
    byteStreamWrite(stream, tmp, sizeof(uint32_t));
    free(tmp);

    if (id3v2ReadExtendedHeaderIndicator(header)) {
        size_t extSize = 0;
        uint8_t *ext = id3v2ExtendedTagHeaderSerialize(header->extendedHeader, header->majorVersion, &extSize);

        if (ext != NULL) {
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

/**
 * @brief Converts an ID3v2 tag header structure to JSON string representation.
 * @details Creates a dynamically allocated JSON string containing the tag header's 
 * version, flags, and extended header fields according to the ID3v2 version. Returns 
 * an empty JSON object "{}" if the header is NULL or the version is unsupported. 
 * The caller must free the returned string.
 * 
 * - ID3v2.2: Returns JSON with major/minor version and flags byte.
 *   Example: {"major":2,"minor":0,"flags":0}
 * 
 * - ID3v2.3: Returns JSON with version, flags, and embedded extended header JSON.
 *   Example: {"major":3,"minor":0,"flags":64,"extended":{"padding":100,"crc":12345678}}
 * 
 * - ID3v2.4: Returns JSON with version, flags, and embedded extended header JSON.
 *   Example: {"major":4,"minor":0,"flags":64,"extended":{"padding":0,"crc":0,"update":false,"tagRestrictions":true,"restrictions":64}}
 * 
 * - Unsupported versions: Returns empty JSON object "{}".
 * 
 * @param header - Pointer to the tag header structure to convert (may be NULL).
 * @return char* - Dynamically allocated JSON string. Caller must free the returned memory.
 */
char *id3v2TagHeaderToJSON(const Id3v2TagHeader *header) {
    char *json = NULL;
    size_t memCount = 3;
    char *extJson = NULL;

    if (header == NULL) {
        json = calloc(memCount, sizeof(char));
        memcpy(json, "{}\0", memCount);
        return json;
    }

    switch (header->majorVersion) {
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
