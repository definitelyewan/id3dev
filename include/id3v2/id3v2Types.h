/**
 * @file id3v2Types.h
 * @author Ewan Jones
 * @brief Defintions and types for id3v2 types
 * @version 2.0
 * @date 2023-12-13
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef ID3V2_TYPES
#define ID3V2_TYPES

#ifdef __cplusplus
extern "C" {
#endif

#include "id3v1Types.h"
#include <stdbool.h>
#include <stdint.h>

// data structures

#include "LinkedList.h"
#include "hashTable.h"

//! Size of ID3v2 tag identifier "ID3" or "3DI"
#define ID3V2_TAG_ID_SIZE 3

//! Memory representation of the tag identifer "ID3"
#define ID3V2_TAG_ID_MAGIC_NUMBER_H 0x494433

//! Memory representation of the tag identifer "3DI"
#define ID3V2_TAG_ID_MAGIC_NUMBER_F 0x334449

//! Major version for ID3v2.2
#define ID3V2_TAG_VERSION_2 2

//! Major version for ID3v2.3
#define ID3V2_TAG_VERSION_3 3

//! Major version for ID3v2.4
#define ID3V2_TAG_VERSION_4 4

/**
 * @brief The max size a frame id or size can be in bytes.
 * @details ID3v2.2 uses an ID and size that is only 3 bytes in size where
 * ID3v2.3 & ID3v2.4 use a 4 byte version
 */
#define ID3V2_FRAME_ID_MAX_SIZE 4

/**
 * @brief The size of a frame headers flag section in bytes
 * @details There are no flags in ID3v2.2
 */
#define ID3V2_FRAME_FLAG_SIZE 2

/**
 * @brief The extended header contains information that can provide further
 * insight in the structure of the tag, but is not vital to the correct parsing
 * of the tag information; hence the extended header is optional.
 * @details A programmer should never need to use this structure manually and it
 * will be created, accessed, and deleted automatically
 */
typedef struct _Id3v2ExtendedTagHeader {

  //! Size of the extended header as a 32bit int or a sync safe int depending on
  //! version
  uint32_t padding;

  //! Crc of the audio data
  uint32_t crc;

  //! Marks a tag as an update
  bool update;

  //! Tag restriction mark, if set restrictions are used
  bool tagRestrictions;

  /**
   * @brief Designates restrictions used by this tag in format: %ppqrrstt.
   * pp is a Tag Size Restriction, q is an Encoding restriction, rr is a
   * Text Fields Size Restriction, s is an Image Encoding Restriction, and
   * tt is an Image size restriction.
   */
  uint8_t restrictions;

} Id3v2ExtendedTagHeader;

/**
 * @brief Vital information used to parse and define an ID3 tag
 *
 */
typedef struct _Id3v2TagHeader {

  //! Major version number
  uint8_t majorVersion;

  //! Minor version number (patch number)
  uint8_t minorVersion;

  //! Tag flags %abcd0000
  uint8_t flags;

  //! Extended header
  Id3v2ExtendedTagHeader *extendedHeader;

} Id3v2TagHeader;

/**
 * @brief Vital information used to parse and define a frames content
 *
 */
typedef struct _Id3v2FrameHeader {

  //! The ID used to identify a frame e.g TIT2
  uint8_t id[ID3V2_FRAME_ID_MAX_SIZE];

  //! Marks the frame as unknown if the tag is altered
  bool tagAlterPreservation;

  //! Marks the frame as unkown if the file is altered
  bool fileAlterPreservation;

  //! Marks the frame as read only
  bool readOnly;

  //! Marks a tag as unsynchronised
  bool unsynchronisation;

  //! zlib decompression size
  uint32_t decompressionSize;

  //! Encryption symbol used to define how the frame is encrypted
  uint8_t encryptionSymbol;

  //! Group symbol used to relate a frame to another
  uint8_t groupSymbol;

} Id3v2FrameHeader;

/**
 * @brief Represents possible types that can be used by a context
 *
 */
typedef enum _Id3v2ContextType {

  // error
  unknown_context = -1, // error context what is there is unreadable

  // generic
  noEncoding_context, // data that is to be read in sequence and has no defining
                      // end character e.g '\0'
  binary_context,     // binary data

  // text
  encodedString_context,  // an encoded string that is defined by a previous
                          // context with the enumerated key "encoding"
  latin1Encoding_context, // forces a latin1 encoded string with the ending of
                          // '\0'

  // numbers
  numeric_context,   // integers of different sizes such as 8, 16, 32, or 64
  precision_context, // decimal values such as floats
  bit_context,       // bits such as 1 or 0

  // control
  iter_context, // allows for the iteration of previous frames with a defiend
                // start and stop
  adjustment_context // if a context with the title "adjustment" exists it will
                     // use its parsed value as a max size
} Id3v2ContextType;

/**
 * @brief Context used to determin how an entry in a frame should be handled
 *
 */
typedef struct _Id3v2ContentContext {

  //! The type of value the parser will extract
  Id3v2ContextType type;

  //! Hashed string
  size_t key;

  /**
   * @brief Smallest number of bytes that can represent this block of data.
   * If the bolck is an iter context this will be the min node when the iter
   * will start
   */
  size_t min;

  /**
   * @brief Largest number of bytes that can represent this block of data.
   * If this block is an iter context this will be max times this context
   * will execute.
   */
  size_t max;

} Id3v2ContentContext;

/**
 * @brief An entry in a frame
 * @details There is no context to this structure and the struct with that
 * information is external. This is done so a user could potentially program
 * there own frames into the parser.
 */
typedef struct _Id3v2ContentEntry {

  //! The entry held within this frame
  void *entry;

  //! The size of the entry in this frame
  size_t size;

} Id3v2ContentEntry;

/**
 * @brief A representation of an ID3v2 frame
 *
 */
typedef struct _Id3v2Frame {

  //! A Frame header
  Id3v2FrameHeader *header;

  //! Instructions used to define content entries
  List *contexts;

  //! List of data representing the frame
  List *entries;

} Id3v2Frame;

#ifdef __cplusplus
} // extern c end
#endif

#endif
