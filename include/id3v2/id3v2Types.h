/**
 * @file id3v2Types.h
 * @author Ewan Jones
 * @brief Definitions for ID3v2.x tag structure, constants, and genre enumeration
 * @version 26.01
 * @date 2023-10-02 - 2026-01-11
 *
 * @copyright Copyright (c) 2023 - 2026
 *
 */

#ifndef ID3V2_TYPES
#define ID3V2_TYPES

#ifdef __cplusplus
extern "C" {
#endif

#include "id3v1/id3v1Types.h"
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>

// data structures

#include "id3dependencies/LinkedListLib/include/LinkedList.h"
#include "id3dependencies/HashTableLib/include/hashTable.h"
#include "id3dependencies/ByteStream/include/byteDefines.h"

//! Size in bytes of the ID3v2 tag identifier "ID3" or "3DI" (3 bytes)
#define ID3V2_TAG_ID_SIZE 3

//! Hexadecimal magic number for ID3v2 header tag identifier "ID3" (0x494433)
#define ID3V2_TAG_ID_MAGIC_NUMBER_H 0x494433

//! Hexadecimal magic number for ID3v2 footer tag identifier "3DI" (0x334449)
#define ID3V2_TAG_ID_MAGIC_NUMBER_F 0x334449

//! ID3v2.2 major version number (2)
#define ID3V2_TAG_VERSION_2 2

//! ID3v2.3 major version number (3)
#define ID3V2_TAG_VERSION_3 3

//! ID3v2.4 major version number (4)
#define ID3V2_TAG_VERSION_4 4

//! Latin-1 (ISO-8859-1) character encoding identifier
#define ID3V2_ENCODING_ISO_8859_1 BYTE_ISO_8859_1

//! UTF-16 Little Endian character encoding identifier
#define ID3V2_ENCODING_UTF16LE BYTE_UTF16LE

//! UTF-16 Big Endian character encoding identifier
#define ID3V2_ENCODING_UTF16BE BYTE_UTF16BE

//! UTF-8 character encoding identifier
#define ID3V2_ENCODING_UTF8 BYTE_UTF8

//! ASCII character encoding identifier
#define ID3V2_ENCODING_ASCII BYTE_ASCII

//! Unknown or unsupported character encoding identifier
#define ID3V2_ENCODING_OTHER BYTE_OTHER_ENCODING

/**
 * @brief Maximum size in bytes for a frame ID field (4 bytes).
 * @details ID3v2.2 uses 3-byte frame IDs and sizes, while ID3v2.3 and ID3v2.4 use 4-byte frame IDs and sizes.
 */ 
#define ID3V2_FRAME_ID_MAX_SIZE 4

/**
 * @brief Size in bytes of a frame header's flag section (2 bytes).
 * @details Frame flags are present in ID3v2.3 and ID3v2.4. ID3v2.2 has no frame flags.
 */
#define ID3V2_FRAME_FLAG_SIZE 2

/**
 * @brief Optional ID3v2 extended header containing supplementary tag metadata.
 * @details Provides additional information about tag structure including CRC validation, 
 * update flags, and content restrictions. Not required for basic tag parsing. Present in 
 * ID3v2.3 and ID3v2.4 when extended header flag is set.
 */
typedef struct _Id3v2ExtendedTagHeader {
    //! Size of the extended header as either a 32-bit integer or syncsafe integer depending on version
    uint32_t padding;

    //! CRC-32 checksum of the tag's audio data for integrity verification
    uint32_t crc;

    //! Indicates this tag is an update to a previous tag (ID3v2.4 only)
    bool update;

    //! Indicates whether tag restrictions are applied
    bool tagRestrictions;

    /**
     * @brief Bitfield defining tag restrictions in format %ppqrrstt (ID3v2.4 only).
     * @details pp = Tag size restrictions, q = Text encoding restrictions, rr = Text field size restrictions, 
     * s = Image encoding restrictions, tt = Image size restrictions.
     */
    uint8_t restrictions;
} Id3v2ExtendedTagHeader;

/**
 * @brief ID3v2 tag header containing version and parsing information.
 * @details Contains version identifiers, flag bits, and optional extended header. 
 * This is the first structure parsed from an ID3v2 tag and determines how the 
 * remainder of the tag is interpreted.
 */
typedef struct _Id3v2TagHeader {
    //! Major version number (2, 3, or 4 for ID3v2.2, ID3v2.3, ID3v2.4)
    uint8_t majorVersion;

    //! Minor version/revision number
    uint8_t minorVersion;

    /**
     * @brief Bitfield in format %abcd0000 defining extra format and feature options of a tag.
     * @details a = unsynchronisation, b = extended header, c = experimental, d = footer present in v2.4
     */
    uint8_t flags;

    //! Pointer to optional extended header structure. NULL if extended header flag not set.
    Id3v2ExtendedTagHeader *extendedHeader;
} Id3v2TagHeader;

/**
 * @brief ID3v2 frame header containing identification and processing flags.
 * @details Specifies frame ID, preservation rules, and optional compression/encryption parameters. Determines how frame content should be parsed and whether the frame can be modified or discarded.
 */
typedef struct _Id3v2FrameHeader {
    //! Frame identifier (e.g., "TIT2" for title, "TALB" for album). 3 bytes in v2.2, 4 bytes in v2.3/v2.4.
    uint8_t id[ID3V2_FRAME_ID_MAX_SIZE];

    //! Marks the frame as unknown if the tag is altered
    bool tagAlterPreservation;

    //! Marks the frame as unknown if the file is altered
    bool fileAlterPreservation;

    //! Marks the frame as read only. If true, frame content is read-only and should not be modified
    bool readOnly;

    //! Marks a tag as unsynchronised
    bool unsynchronisation;

    //! Decompressed size in bytes if frame uses zlib compression. 0 if uncompressed.
    uint32_t decompressionSize;

    //! Encryption method identifier. 0 if frame is not encrypted.
    uint8_t encryptionSymbol;

    //! Group identifier to associate related frames. 0 if frame is not grouped.
    uint8_t groupSymbol;
} Id3v2FrameHeader;

/**
 * @brief Context types for parsing and writing ID3v2 frame content fields.
 * @details Defines how each field within a frame should be interpreted during parsing. Context types determine data interpretation, boundary detection, and reading behavior for frame content entries.
 */
typedef enum _Id3v2ContextType {
    /**
     * @brief Error/invalid context state (-1).
     * @details Halts frame parsing when encountered. Indicates unrecognized or malformed frame structure.
     */
    unknown_context = -1,

    /**
     * @brief Raw character data with no encoding or null terminator.
     * @details Fixed-length character sequence without encoding specification.
     */
    noEncoding_context,

    /**
     * @brief Binary data block with no terminator.
     * @details Reads until context upper bound or frame end. Used for image data, binary payloads, etc.
     */
    binary_context,

    /**
     * @brief Text string with encoding determined by prior context with the key 'encoding'.
     * @details Supports Latin-1, UTF-8, and UTF-16 encodings. Requires preceding encoding byte context.
     */
    encodedString_context,

    /**
     * @brief Latin-1 (ISO-8859-1) null-terminated string.
     * @details Fixed encoding string terminated by '\0'. No encoding byte required.
     */
    latin1Encoding_context,

    /**
     * @brief Integer values of 8, 16, 32, or 64 bits.
     * @details Size determined by context min/max bounds.
     */
    numeric_context,

    /**
     * @brief Floating-point values (float or double).
     * @details Precision type determined by context size specification.
     */
    precision_context,

    /**
     * @brief Bit-level field (1-8 bits).
     * @details Identifies 1 to n bits. With this context the current byte being read
     * will not be incremented until 8 sequential bits are read. for example, this
     * means if the following context is a binary_context it will read the same byte
     * twice. If this context is followed by more then one bit_context it will continue
     * reading from the position in which the proceeding context left off. Do note
     * that max and min within the context structure now represent max and min bits
     * instead of bytes.
     */
    bit_context,

    /**
     * @brief Iterator for repeating context sequences.
     * @details Executes contexts from index 'min' for 'max' iterations. Min/max semantics differ: min=start index, max=iteration count.
     */
    iter_context,

    /**
     * @brief Dynamic upper bound adjustment based on prior context with key 'adjustment'.
     * @details Allows runtime modification of context max value using data from previous context labeled 'adjustment'.
     */
    adjustment_context
} Id3v2ContextType;

/**
 * @brief Parsing instructions for a single field within an ID3v2 frame.
 * @details Specifies how to extract and interpret a data field from frame content. Multiple contexts 
 * form a sequence that defines the complete frame structure. Min/max semantics vary by type: for most 
 * types they represent byte bounds, for iter_context min=start index and max=iteration count, for 
 * bit_context they represent bit counts.
 */
typedef struct _Id3v2ContentContext {
    //! Context type determining parsing behavior (string, binary, numeric, etc.)
    Id3v2ContextType type;

    //! Hashed identifier string for this field. Used to reference this context from other contexts (e.g., 'encoding', 'adjustment').
    size_t key;

    /**
     * @brief Minimum size in bytes (or bits for bit_context, or start index for iter_context).
     * @details Defines lower bound for variable-length fields or starting position for iterations.
     */
    size_t min;

    /**
     * @brief Maximum size in bytes (or bits for bit_context, or iteration count for iter_context).
     * @details Defines upper bound for variable-length fields or number of repetitions for iterations.
     */
    size_t max;
} Id3v2ContentContext;

/**
 * @brief Parsed data field from an ID3v2 frame.
 * @details Generic container for a single extracted field value. Interpretation requires corresponding 
 * Id3v2ContentContext from the frame's context list. Separation of data from context metadata enables 
 * custom frame definitions.
 */
typedef struct _Id3v2ContentEntry {
    //! Pointer to extracted field data (string, binary, numeric, etc.). Type determined by corresponding context.
    void *entry;

    //! Size in bytes of the data pointed to by entry
    size_t size;
} Id3v2ContentEntry;

/**
 * @brief Complete ID3v2 frame structure with header, parsing contexts, and data.
 * @details Combines frame identification (header), parsing instructions (contexts), and extracted 
 * data (entries). Contexts and entries lists correspond positionally - each context defines how to 
 * interpret its matching entry.
 */
typedef struct _Id3v2Frame {
    //! Frame header containing ID, flags, and processing parameters
    Id3v2FrameHeader *header;

    //! Linked list of Id3v2ContentContext parsing instructions defining frame field structure
    List *contexts;

    //! Linked list of Id3v2ContentEntry parsed data fields corresponding to contexts
    List *entries;
} Id3v2Frame;

/**
 * @brief Complete ID3v2 tag structure containing header and metadata frames.
 * @details Root structure representing an entire ID3v2 tag parsed from file. Header determines 
 * version and parsing behavior. Frames list contains all metadata (title, artist, artwork, etc.). 
 * Footer, when present, is not stored separately as it mirrors the header with reversed identifier "3DI".
 */
typedef struct _Id3v2Tag {
    //! Tag header with version, flags, and optional extended header
    Id3v2TagHeader *header;

    //! Linked list of Id3v2Frame structures containing all tag metadata
    List *frames;
} Id3v2Tag;

#ifdef __cplusplus
} // extern c end
#endif

#endif
