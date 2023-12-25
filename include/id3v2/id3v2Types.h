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
extern "C"{
#endif

#include <stdbool.h>
#include <stdint.h>

//! size of ID3v2 tag identifier "ID3" or "3DI"
#define ID3V2_TAG_ID_SIZE 3

//! major version for ID3v2.2
#define ID3V2_TAG_VERSION_2 2

//! major version for ID3v2.3
#define ID3V2_TAG_VERSION_3 3

//! major version for ID3v2.4
#define ID3V2_TAG_VERSION_4 4

/**
 * @brief The extended header contains information that can provide further insight in the 
 * structure of the tag, but is not vital to the correct parsing of the tag information; 
 * hence the extended header is optional.
 * @details a programmer should never need to use this structure manually and it will be
 * created, accessed, and deleted automatically
 * 
 */
typedef struct _Id3v2ExtendedTagHeader{
    
    //!size of the extended header as a 32bit int or a sync safe int depending on version
    uint32_t padding;

    //!crc of the audio data
    uint32_t crc;

    //!marks a tag as an update
    bool update;

    //!tag restriction mark, if set restrictions are used
    bool tagRestrictions;

    /**
     * @brief designates restrictions used by this tag in format: %ppqrrstt.
     * pp is a Tag Size Restriction, q is an Encoding restriction, rr is a
     * Text Fields Size Restriction, s is an Image Encoding Restriction, and
     * tt is an Image size restriction.  
     */
    uint8_t restrictions;


}Id3v2ExtendedTagHeader;

/**
 * @brief vital information used to parse and define an ID3 tag
 * 
 */
typedef struct _Id3v2TagHeader{

    //!major version number
    uint8_t majorVersion;

    //!minor version number (patch number)
    uint8_t minorVersion;

    //!frame flags %abcd0000
    uint8_t flags;

    //extended header
    Id3v2ExtendedTagHeader *extendedHeader;

}Id3v2TagHeader;

/**
 * @brief An ID3V2.x tag
 * 
 */
typedef struct _Id3v2Tag{

    Id3v2TagHeader *tagHeader;

}Id3v2Tag;


#ifdef __cplusplus
} //extern c end
#endif

#endif