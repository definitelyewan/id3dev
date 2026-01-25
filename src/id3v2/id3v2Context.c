/**
 * @file id3v2Context.c
 * @author Ewan Jones
 * @brief Function implementation of ID3v2 frame context definitions and parsing configuration
 * @version 0.1
 * @date 2024-04-11 - 2026-01-19
 * 
 * @copyright Copyright (c) 2024 - 2026
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "id3v2/id3v2Context.h"
#include "id3dependencies/ByteStream/include/byteTypes.h"
#include "id3dependencies/ByteStream/include/byteStream.h"
#include "id3dependencies/ByteStream/include/byteInt.h"

/**
 * @brief Computes a DJB2 hash value for a null-terminated string.
 * @details Implements the DJB2 hash algorithm by Daniel J. Bernstein using the formula 
 * hash = hash * 33 + c for each character. Optimized with bit-shifting: (hash << 5) + hash 
 * is equivalent to hash * 33. Used internally for frame identifier and context key hashing 
 * to enable fast lookups in hash tables. Initial hash value is 5381.
 * 
 * @param str - Null-terminated string to hash
 * 
 * @return unsigned long - DJB2 hash value computed from the input string
 */
unsigned long id3v2djb2(const char *str) {
    unsigned long hash = 5381;
    int c = 0;

    while (*str != '\0') {
        c = (int) *str++;
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }


    return hash;
}

/**
 * @brief Creates a content context structure defining entry constraints and metadata.
 * @details Allocates and initializes an Id3v2ContentContext on the heap. Content contexts define 
 * the structure, type, and size constraints for individual content entries within ID3v2 frames. 
 * Each context specifies how an entry should be interpreted (string, binary, numeric, etc.) and 
 * enforces minimum and maximum size bounds. The key field is a DJB2 hash used for 
 * context identification and lookup operations.
 * 
 * @param type - Context type specifying how entry data should be interpreted (e.g., encodedString_context, binary_context, numeric_context)
 * @param key - Hash key for context identification, typically computed via id3v2djb2()
 * @param max - Maximum allowed size in bytes for entries using this context
 * @param min - Minimum required size in bytes for entries using this context
 * 
 * @return Id3v2ContentContext* - Heap allocated content context structure. Caller must free
 */
Id3v2ContentContext *id3v2CreateContentContext(Id3v2ContextType type, size_t key, size_t max, size_t min) {
    Id3v2ContentContext *context = malloc(sizeof(Id3v2ContentContext));

    context->type = type;
    context->key = key;
    context->max = max;
    context->min = min;

    return context;
}

/**
 * @brief Frees all memory allocated for a content context and nullifies the pointer.
 * @details Safely deallocates a content context structure created by id3v2CreateContentContext.
 * Sets the pointer to NULL after freeing to prevent use-after-free errors. If the pointer
 * is already NULL, no action is taken.
 * 
 * @param toDelete - Pointer to content context pointer to be freed and nullified
 */
void id3v2DestroyContentContext(Id3v2ContentContext **toDelete) {
    if (*toDelete) {
        free(*toDelete);
        *toDelete = NULL;
        toDelete = NULL;
    }
}

/**
 *
 * List/Hash API required functions
 *
 */

/**
 * @brief Frees all memory allocated for a content context structure.
 * @details Wrapper around id3v2DestroyContentContext for use as a callback in list operations.
 * Deallocates the content context structure. Can be used as a callback for list deletion operations.
 * 
 * @param toBeDeleted - Pointer to Id3v2ContentContext to be freed
 */
void id3v2DeleteContentContext(void *toBeDeleted) {
    id3v2DestroyContentContext((Id3v2ContentContext **) &toBeDeleted);
}

/**
 * @brief Performs comparison of two content context structures.
 * @details Compares content contexts by sequentially checking type, key, min, and max fields.
 * Uses a non-standard comparison: returns 0 if first field is less than second, otherwise 
 * returns the field difference. Can be used as a callback for list comparison operations.
 * 
 * @param first - First content context to compare
 * @param second - Second content context to compare
 * 
 * @return int - 0 if contexts are equal or first < second in comparison order, positive if first > second
 */
int id3v2CompareContentContext(const void *first, const void *second) {
    const Id3v2ContentContext *c1 = (Id3v2ContentContext *) first;
    const Id3v2ContentContext *c2 = (Id3v2ContentContext *) second;
    int diff = 0;

    diff = (c1->type < c2->type) ? 0 : c1->type - c2->type;
    if (diff != 0) {
        return diff;
    }


    diff = (c1->key < c2->key) ? 0 : (int) (c1->key - c2->key);
    if (diff != 0) {
        return diff;
    }


    diff = (c1->min < c2->min) ? 0 : (int) (c1->min - c2->min);
    if (diff != 0) {
        return diff;
    }

    diff = (c1->max < c2->max) ? 0 : (int) (c1->max - c2->max);
    return diff;
}

/**
 * @brief Generates a string representation of a content context for debugging.
 * @details Creates a formatted string displaying the context's type, key hash, minimum size, 
 * and maximum size constraints. Can be used as a callback function for list printing operations.
 * Caller must free the returned string.
 * 
 * @param toBePrinted - Content context to represent as a string
 * 
 * @return char* - Heap allocated string in format "Type: <type>, Key: <key>, min: <min>, max: <max>\n". Caller must free
 */
char *id3v2PrintContentContext(const void *toBePrinted) {
    const Id3v2ContentContext *c = (Id3v2ContentContext *) toBePrinted;

    size_t memCount = snprintf(NULL, 0, "Type: %d, Key: %zu, min: %zu, max: %zu\n", c->type, c->key, c->min, c->max);

    // 40 chars for the below string
    char *str = malloc(sizeof(long) + sizeof(long) + sizeof(int) + sizeof(Id3v2ContextType) + 40);

    (void) snprintf(str, memCount + 1, "Type: %d, Key: %zu, min: %zu, max: %zu\n", c->type, c->key, c->min, c->max);

    return str;
}

/**
 * @brief Creates a deep copy of a content context structure.
 * @details Duplicates a content context by creating a new structure with independently allocated
 * memory and copying all fields (type, key, min, max). Can be used as a callback function for 
 * list copy operations.
 * 
 * @param toBeCopied - Content context to copy
 * 
 * @return void* - Heap allocated content context containing a copy of all fields. Caller must free
 */
void *id3v2CopyContentContext(const void *toBeCopied) {
    Id3v2ContentContext *copy = (Id3v2ContentContext *) toBeCopied;
    Id3v2ContentContext *ret = malloc(sizeof(Id3v2ContentContext));

    ret->key = copy->key;
    ret->max = copy->max;
    ret->min = copy->min;
    ret->type = copy->type;

    return (void *) ret;
}

/**
 * @brief Frees all memory allocated for a list of content contexts.
 * @details Wrapper around listFree for use as a callback in hash table or list operations
 * where the stored data is itself a List structure. Deallocates the list and all its 
 * contained content context elements.
 * 
 * @param toBeDeleted - Pointer to List structure to be freed
 */
void id3v2DeleteContentContextList(void *toBeDeleted) {
    listFree((List *) toBeDeleted);
}

/**
 * @brief Generates a string representation of a content context list for debugging.
 * @details Wrapper around listToString for use as a callback in hash table printing operations
 * where the stored data is a List structure containing content contexts. Creates a formatted 
 * string displaying all contexts in the list. Caller must free the returned string.
 * 
 * @param toBePrinted - List of content contexts to represent as a string
 * 
 * @return char* - Heap allocated string representation of the list. Caller must free
 */
char *id3v2PrintContentContextList(const void *toBePrinted) {
    return listToString((List *) toBePrinted);
}

/**
 * @brief Creates a deep copy of a content context list.
 * @details Wrapper around listDeepCopy for use as a callback in hash table copy operations
 * where the stored data is a List structure containing content contexts. Creates a new list
 * with independently allocated copies of all content context elements.
 * 
 * @param toBeCopied - List of content contexts to copy
 * 
 * @return void* - Heap allocated list containing deep copies of all content contexts. Caller must free
 */
void *id3v2CopyContentContextList(const void *toBeCopied) {
    return listDeepCopy((List *) toBeCopied);
}


/**
 *
 * Context list generation
 *
 */

/**
 * @brief Creates a context definition list for standard ID3v2 text frames.
 * @details Constructs a list defining the structure of text information frames (e.g., TIT2, TPE1, 
 * TALB). The context specifies a 1-byte encoding field followed by a variable-length encoded text 
 * string (1 to UINT_MAX bytes). This pattern is used by all standard text frames that store single 
 * text values with encoding specification.
 * 
 * @return List* - Heap allocated list containing two content contexts: encoding (1 byte) and text 
 * (1-UINT_MAX bytes). Caller must free with listFree()
 */
List *id3v2CreateTextFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // text
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("text"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for user-defined text frames (TXXX).
 * @details Constructs a list defining the structure of user-defined text information frames. 
 * The context specifies a 1-byte encoding field, followed by a description string (1 to UINT_MAX 
 * bytes) identifying the custom field, and finally the text value (1 to UINT_MAX bytes). This 
 * pattern allows users to define custom text metadata fields not covered by standard ID3v2 frames.
 * 
 * @return List* - Heap allocated list containing three content contexts: encoding (1 byte), 
 * description (1-UINT_MAX bytes), and text value (1-UINT_MAX bytes). Caller must free with listFree()
 */
List *id3v2CreateUserDefinedTextFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // desc
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // desc
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("desc"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // text
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("text"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for standard ID3v2 URL frames.
 * @details Constructs a list defining the structure of URL frames (e.g., WOAR, WOAS, WCOP, WORS). 
 * The context specifies a single Latin-1 encoded URL string (1 to UINT_MAX bytes).
 * 
 * @return List* - Heap allocated list containing one content context: URL string in Latin-1 
 * encoding (1-UINT_MAX bytes). Caller must free with listFree()
 */
List *id3v2CreateURLFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // url
    void *toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("url"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for user-defined URL frames (WXXX).
 * @details Constructs a list defining the structure of user-defined URL frames. The context 
 * specifies a 1-byte encoding field, followed by a description string (1 to UINT_MAX bytes) 
 * identifying the custom URL field, and finally the URL value as a Latin-1 encoded string 
 * (1 to UINT_MAX bytes). This pattern allows users to define custom URL metadata fields not 
 * covered by standard ID3v2 URL frames.
 * 
 * @return List* - Heap allocated list containing three content contexts: encoding (1 byte), 
 * description (1-UINT_MAX bytes), and URL in Latin-1 encoding (1-UINT_MAX bytes). Caller must 
 * free with listFree()
 */
List *id3v2CreateUserDefinedURLFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // desc
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("desc"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // url
    toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("url"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for attached picture frames (APIC/PIC).
 * @details Constructs a version-specific list defining the structure of attached picture frames 
 * used for album artwork and other images. v2.2 uses a 3-byte fixed format string (e.g., "PNG", 
 * "JPG"), while v2.3/v2.4 use a Latin-1 encoded MIME type (e.g., "image/png", "image/jpeg"). 
 * The context includes: 1-byte encoding, format identifier, 1-byte picture type (cover front, 
 * back, icon, etc.), encoded description string, and binary image data.
 * 
 * @param version - ID3v2 version (ID3V2_TAG_VERSION_2, ID3V2_TAG_VERSION_3, or ID3V2_TAG_VERSION_4) 
 * determining format field encoding
 * 
 * @return List* - Heap allocated list containing five content contexts: encoding (1 byte), format 
 * (3 bytes for v2.2 or 1-UINT_MAX bytes for v2.3/v2.4), picture type (1 byte), description 
 * (1-UINT_MAX bytes), and binary image data (1-UINT_MAX bytes). Caller must free with listFree()
 */
List *id3v2CreateAttachedPictureFrameContext(unsigned int version) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // image format
    switch (version) {
        case ID3V2_TAG_VERSION_2:
            // format is $xx xx xx
            toAdd = (void *) id3v2CreateContentContext(noEncoding_context, id3v2djb2("format"), 3, 1);
            break;

        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            // format is latin1
            toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("format"), UINT_MAX, 1);
            break;

        default:
            // something it wrong
            toAdd = (void *) id3v2CreateContentContext(unknown_context, id3v2djb2("format"), UINT_MAX, 1);
            break;
    }

    listInsertBack(l, toAdd);

    // picture type
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("type"), 1, 1);
    listInsertBack(l, toAdd);

    // desc
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("desc"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for Audio Seek Point Index frames (ASPI).
 * @details Constructs a list defining the structure of ASPI frames used for efficient seeking 
 * within audio files. The frame contains binary index data that maps file positions to time 
 * positions, enabling fast navigation without parsing the entire audio stream. The context 
 * specifies a single binary data field.
 * 
 * @return List* - Heap allocated list containing one content context: binary index data 
 * (1-UINT_MAX bytes). Caller must free with listFree()
 */
List *id3v2CreateAudioSeekPointIndexFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // data
    void *toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for Audio Encryption frames (AENC).
 * @details Constructs a list defining the structure of AENC frames used to signal encrypted 
 * audio content. The context specifies: a Latin-1 encoded owner identifier (URL or email of 
 * the encryption method owner), a 2-byte preview start position indicating where unencrypted 
 * preview data begins, a 2-byte preview length, and binary encryption information containing 
 * keys or method-specific data.
 * 
 * @return List* - Heap allocated list containing four content contexts: owner identifier 
 * (1-UINT_MAX bytes), preview start (2 bytes), preview length (2 bytes), and encryption data 
 * (1-UINT_MAX bytes). Caller must free with listFree()
 */
List *id3v2CreateAudioEncryptionFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // identifier
    void *toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("identifier"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // preview start
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("start"), 2, 2);
    listInsertBack(l, toAdd);

    // length
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("length"), 2, 2);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for comment frames (COMM).
 * @details Constructs a list defining the structure of comment frames used for storing user 
 * comments and notes. The context specifies: a 1-byte encoding field, a 3-byte ISO-639-2 
 * language code (e.g., "eng", "fra", "deu"), an encoded short content description string 
 * (1-UINT_MAX bytes), and the actual comment text (1-UINT_MAX bytes). Multiple 
 * COMM frames can coexist with different language/description combinations.
 * 
 * @return List* - Heap allocated list containing four content contexts: encoding (1 byte), 
 * language code (3 bytes), content description (1-UINT_MAX bytes), and comment text 
 * (1-UINT_MAX bytes). Caller must free with listFree()
 */
List *id3v2CreateCommentFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // language
    toAdd = (void *) id3v2CreateContentContext(noEncoding_context, id3v2djb2("language"), 3, 1);
    listInsertBack(l, toAdd);

    // desc
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("desc"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // text
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("text"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for commercial frames (COMR).
 * @details Constructs a list defining the structure of commercial information frames used to 
 * describe commercial transactions. The context specifies: a 1-byte encoding field, a price 
 * string (1 to UINT_MAX bytes) in latin-1 format, an 8-byte date string (YYYYMMDD), a URL 
 * (1 to UINT_MAX bytes) for purchasing, a 1-byte received-as type indicator, a name field 
 * (1 to UINT_MAX bytes) in the specified encoding, a description field (1 to UINT_MAX bytes) 
 * in the specified encoding, a MIME type format string (1 to UINT_MAX bytes) in latin-1, and 
 * finally binary data (1 to UINT_MAX bytes) such as a company logo. This structure enables 
 * embedding commercial transaction details and purchase information within ID3v2 tags.
 * 
 * @return List* - Heap allocated list containing nine content contexts: encoding (1 byte), 
 * price (1-UINT_MAX bytes), date (8 bytes), url (1-UINT_MAX bytes), type (1 byte), name 
 * (1-UINT_MAX bytes), description (1-UINT_MAX bytes), format (1-UINT_MAX bytes), and data 
 * (1-UINT_MAX bytes). Caller must free with listFree()
 */
List *id3v2CreateCommercialFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // price
    toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("price"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // date
    toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("date"), 8, 1);
    listInsertBack(l, toAdd);

    // url
    toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("url"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // received as (type)
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("type"), 1, 1);
    listInsertBack(l, toAdd);

    // name
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("name"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // desc
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("desc"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // format
    toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("format"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for encrypted metadata frames (ENCR).
 * @details Constructs a list defining the structure of encryption method registration frames.
 * The context specifies: an identifier field (1 to UINT_MAX bytes) in latin-1 format containing 
 * the owner identifier (typically an email or URL), a content descriptor field (1 to UINT_MAX 
 * bytes) in latin-1 format providing additional encryption method information, and binary data 
 * (1 to UINT_MAX bytes) containing method-specific encryption parameters. This structure enables 
 * registration and documentation of encryption methods used within the ID3v2 tag.
 * 
 * @return List* - Heap allocated list containing three content contexts: identifier 
 * (1-UINT_MAX bytes), content (1-UINT_MAX bytes), and data (1-UINT_MAX bytes). Caller must 
 * free with listFree()
 */
List *id3v2CreateEncryptedMetaFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // identifier
    void *toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("identifier"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // content
    toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("content"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for encryption method registration (ENCR) or group identification (GRID) frames.
 * @details Constructs a list defining the structure of registration frames used for encryption methods 
 * or group identifiers. The context specifies: an identifier field (1 to UINT_MAX bytes) in latin-1 
 * format containing the owner identifier (typically an email or URL), a 1-byte symbol field serving 
 * as the method or group identifier byte, and binary data (1 to UINT_MAX bytes) containing 
 * method-specific or group-specific information. For ENCR frames, this registers encryption methods 
 * with their parameters. For GRID frames, this registers group identifiers for organizational purposes.
 * 
 * @return List* - Heap allocated list containing three content contexts: identifier (1-UINT_MAX bytes), 
 * symbol (1 byte), and data (1-UINT_MAX bytes). Caller must free with listFree()
 */
List *id3v2CreateRegistrationFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // identifier
    void *toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("identifier"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // symbol
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("symbol"), 1, 1);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for music CD identifier frames (MCDI).
 * @details Constructs a list defining the structure of Music CD Identifier frames containing 
 * CD Table of Contents (TOC) data. The context specifies a single binary data field of exactly 
 * 804 bytes, representing the complete CD TOC as defined by the Red Book CD-DA standard. This 
 * fixed size accommodates up to 100 tracks with their position information, enabling accurate 
 * identification and lookup of audio CDs in online databases such as CDDB/FreeDB.
 * 
 * @return List* - Heap allocated list containing one content context: data (804 bytes). 
 * Caller must free with listFree()
 */
List *id3v2CreateMusicCDIdentifierFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // data
    void *toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), 804, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for play counter frames (PCNT).
 * @details Constructs a list defining the structure of play counter frames that track the number 
 * of times a file has been played. The context specifies a single binary data field (1 to UINT_MAX 
 * bytes) containing the play count stored as integer. The counter starts at zero and 
 * increments with each playback. Variable length encoding allows the counter to grow as needed, 
 * typically starting with 4 bytes but expandable for very high play counts.
 * 
 * @return List* - Heap allocated list containing one content context: data (1-UINT_MAX bytes). 
 * Caller must free with listFree()
 */
List *id3v2CreatePlayCounterFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // data
    void *toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, sizeof(uint32_t));
    listInsertBack(l, toAdd);


    return l;
}

/**
 * @brief Creates a context definition list for equalization frames (EQU/EQUA/EQU2).
 * @details Constructs a list defining the structure of equalization frames, with format varying 
 * by ID3v2 version. For v2.2/v2.3 (EQU/EQUA): specifies a 1-byte adjustment bits field, a 1-bit 
 * increment/decrement flag, a 15-bit frequency value, volume adjustment data (1 to UINT_MAX bytes, 
 * length dependent on adjustment field), and an iterator context for repeating frequency/volume 
 * pairs. For v2.4 (EQU2): specifies a 1-byte interpolation method symbol, an identifier string 
 * (1 to UINT_MAX bytes) in latin-1 format describing the equalization curve, a 2-byte volume 
 * adjustment value, and an iterator for repeating frequency/volume pairs (minimum 2 bytes). These 
 * structures enable storage of frequency-specific volume adjustments for audio equalization.
 * 
 * @param version The ID3v2 version (ID3V2_TAG_VERSION_2, ID3V2_TAG_VERSION_3, or ID3V2_TAG_VERSION_4)
 * 
 * @return List* - Heap allocated list containing version-specific content contexts. For v2.2/v2.3: 
 * adjustment (1 byte), unary (1 bit), frequency (15 bits), volume (1-UINT_MAX bytes), and iter 
 * (1-UINT_MAX bytes). For v2.4: symbol (1 byte), identifier (1-UINT_MAX bytes), volume (2 bytes), 
 * and iter (2-UINT_MAX bytes). Caller must free with listFree()
 */
List *id3v2CreateEqualizationFrameContext(unsigned int version) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);
    void *toAdd = NULL;

    switch (version) {
        // EQU and EQUA
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:

            // adjustment
            toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("adjustment"), 1, 1);
            listInsertBack(l, toAdd);

            // increment decrement
            toAdd = (void *) id3v2CreateContentContext(bit_context, id3v2djb2("unary"), 1, 1);
            listInsertBack(l, toAdd);

            // frequency
            toAdd = (void *) id3v2CreateContentContext(bit_context, id3v2djb2("frequency"), 15, 15);
            listInsertBack(l, toAdd);

            // volume (adjustment dependant)
            toAdd = (void *) id3v2CreateContentContext(adjustment_context, id3v2djb2("volume"), UINT_MAX, 1);
            listInsertBack(l, toAdd);

            // iter through the last 3 limit is int max but a frames data will 100% run out before this
            toAdd = (void *) id3v2CreateContentContext(iter_context, id3v2djb2("iter"), UINT_MAX, 1);
            listInsertBack(l, toAdd);
            break;

        // EQU2
        case ID3V2_TAG_VERSION_4:

            // symbol
            toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("symbol"), 1, 1);
            listInsertBack(l, toAdd);

            // identifier
            toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("identifier"), UINT_MAX, 1);
            listInsertBack(l, toAdd);

            // volume
            toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("volume"), 2, 2);
            listInsertBack(l, toAdd);

            // iter through the last 2 limit is int max but a frames data will 100% run out before this
            toAdd = (void *) id3v2CreateContentContext(iter_context, id3v2djb2("iter"), UINT_MAX, 2);
            listInsertBack(l, toAdd);
            break;

        default:
            toAdd = (void *) id3v2CreateContentContext(unknown_context, id3v2djb2("unknown"), 1, 1);
            listInsertBack(l, toAdd);
            break;
    }

    return l;
}

/**
 * @brief Creates a context definition list for event timing codes frames (ETCO).
 * @details Constructs a list defining the structure of event timing codes frames that mark 
 * significant moments during audio playback. The context specifies: a 1-byte symbol field 
 * indicating the time stamp format (MPEG frames, milliseconds, etc.), a 1-byte event type 
 * code identifying the event (intro start, outro start, key change, etc.), a 4-byte time stamp 
 * indicating when the event occurs, and an iterator context for repeating type/stamp pairs 
 * (1 to UINT_MAX bytes). This structure enables synchronization of events with audio playback 
 * at precisely defined moments.
 * 
 * @return List* - Heap allocated list containing four content contexts: symbol (1 byte), 
 * type (1 byte), stamp (4 bytes), and iter (1-UINT_MAX bytes). Caller must free with listFree()
 */
List *id3v2CreateEventTimingCodesFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // select format
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("symbol"), 1, 1);
    listInsertBack(l, toAdd);

    // type
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("type"), 1, 1);
    listInsertBack(l, toAdd);

    // stamp
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("stamp"), 4, 4);
    listInsertBack(l, toAdd);

    // iter from the type onward
    toAdd = (void *) id3v2CreateContentContext(iter_context, id3v2djb2("iter"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for general encapsulated object frames (GEOB).
 * @details Constructs a list defining the structure of general encapsulated object frames used 
 * to embed arbitrary files within ID3v2 tags. The context specifies: a 1-byte encoding field 
 * for text strings, a MIME type format string (1 to UINT_MAX bytes) in latin-1 encoding 
 * describing the object type, a filename field (1 to UINT_MAX bytes) in the specified encoding, 
 * a content description field (1 to UINT_MAX bytes) in the specified encoding, and binary data 
 * (1 to UINT_MAX bytes) containing the actual encapsulated object. This structure enables 
 * embedding any file type (documents, images, archives, etc.) directly within the audio file's 
 * metadata.
 * 
 * @return List* - Heap allocated list containing five content contexts: encoding (1 byte), 
 * format (1-UINT_MAX bytes), name (1-UINT_MAX bytes), description (1-UINT_MAX bytes), and 
 * data (1-UINT_MAX bytes). Caller must free with listFree()
 */
List *id3v2CreateGeneralEncapsulatedObjectFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // format
    toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("format"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // file name
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("name"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // desc
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("desc"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for involved people list frames (IPLS) in ID3v2.2 and ID3v2.3.
 * @details Constructs a list defining the structure of involved people list frames that document 
 * individuals involved in the recording and their roles. The context specifies: a 1-byte encoding 
 * field for text strings, a name field (1 to UINT_MAX bytes) in the specified encoding describing 
 * the involvement or role (e.g., "producer", "engineer", "mixer"), a text field (1 to UINT_MAX 
 * bytes) in the specified encoding containing the person's name, and an iterator context for 
 * repeating name/text pairs (1 to UINT_MAX bytes). This structure enables documentation of multiple 
 * contributors and their specific roles in the production process.
 * 
 * @return List* - Heap allocated list containing four content contexts: encoding (1 byte), name 
 * (1-UINT_MAX bytes), text (1-UINT_MAX bytes), and iter (1-UINT_MAX bytes). Caller must free 
 * with listFree()
 */
List *id3v2CreateInvolvedPeopleListFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // name
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("name"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // text
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("text"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // iter from name onward
    toAdd = (void *) id3v2CreateContentContext(iter_context, id3v2djb2("iter"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for linked information frames (LINK).
 * @details Constructs a list defining the structure of linked information frames used to reference 
 * additional data stored externally or in other locations within the tag. The context specifies: 
 * a URL field (1 to UINT_MAX bytes) in latin-1 encoding pointing to the location of the linked 
 * information, and a data field (1 to UINT_MAX bytes) containing frame identifier codes and 
 * additional identification data for the linked content. This structure enables referencing external 
 * resources or creating connections between frames without duplicating large amounts of data within 
 * the tag itself.
 * 
 * @return List* - Heap allocated list containing two content contexts: url (1-UINT_MAX bytes) 
 * and data (1-UINT_MAX bytes). Caller must free with listFree()
 */
List *id3v2CreateLinkedInformationFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // url
    void *toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("url"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(noEncoding_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for MPEG location lookup table frames (MLLT).
 * @details Constructs a list defining the structure of MPEG location lookup table frames that 
 * provide seek point references for efficient navigation within MPEG audio streams. The context 
 * specifies a single binary data field (1 to UINT_MAX bytes) containing reference point 
 * information including MPEG frames between references, bytes between references, milliseconds 
 * between references, and deviation arrays. Note: This implementation treats the entire frame 
 * as binary data without parsing individual subfields, as MLLT frames are rarely used in practice.
 * 
 * @return List* - Heap allocated list containing one content context: data (1-UINT_MAX bytes). 
 * Caller must free with listFree()
 */
List *id3v2CreateMPEGLocationLookupTableFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // data
    void *toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for ownership frames (OWNE).
 * @details Constructs a list defining the structure of ownership frames that document the legal 
 * ownership of the audio file. The context specifies: a 1-byte encoding field for text strings, 
 * a price string (1 to UINT_MAX bytes) in latin-1 format indicating the purchase price with 
 * currency code, an 8-byte date string (YYYYMMDD format) in latin-1 encoding indicating the date 
 * of purchase, and a seller name field (1 to UINT_MAX bytes) in the specified encoding containing 
 * the name of the person or organization from whom the file was purchased. This structure provides 
 * a record of legitimate ownership and purchase information.
 * 
 * @return List* - Heap allocated list containing four content contexts: encoding (1 byte), price 
 * (1-UINT_MAX bytes), date (8 bytes), and name (1-UINT_MAX bytes). Caller must free with listFree()
 */
List *id3v2CreateOwnershipFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // data
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // price
    toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("price"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // date
    toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("date"), 8, 8);
    listInsertBack(l, toAdd);

    // date
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("name"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for popularimeter frames (POPM).
 * @details Constructs a list defining the structure of popularimeter frames that store user ratings 
 * and play count information. The context specifies: an identifier field (1 to UINT_MAX bytes) in 
 * latin-1 encoding containing the email address of the user or application providing the rating, 
 * a 1-byte rating symbol value (typically 0-255, where higher values indicate better ratings), and 
 * a counter data field (1 to UINT_MAX bytes) containing the play count as a big-endian integer. 
 * This structure enables multiple users or applications to store independent ratings and play counts 
 * by using unique email identifiers.
 * 
 * @return List* - Heap allocated list containing three content contexts: identifier (1-UINT_MAX bytes), 
 * symbol (1 byte), and data (1-UINT_MAX bytes). Caller must free with listFree()
 */
List *id3v2CreatePopularimeterFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // email
    void *toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("identifier"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // symbol / rating
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("symbol"), 1, 1);
    listInsertBack(l, toAdd);

    // counter
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for position synchronisation frames (POSS).
 * @details Constructs a list defining the structure of position synchronisation frames used to 
 * mark the current playback position when audio has been edited or reorganized. The context 
 * specifies: a 1-byte format field indicating the time stamp format (MPEG frames, milliseconds, 
 * etc.), and a 4-byte time stamp value indicating the precise playback position within the audio. 
 * This structure enables applications to maintain synchronized playback positions across edited 
 * versions of the same audio file or after structural modifications.
 * 
 * @return List* - Heap allocated list containing two content contexts: format (1 byte) and 
 * stamp (4 bytes). Caller must free with listFree()
 */
List *id3v2CreatePositionSynchronisationFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // format
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("format"), 1, 1);
    listInsertBack(l, toAdd);

    // stamp
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("stamp"), 4, 4);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for private frames (PRIV).
 * @details Constructs a list defining the structure of private frames used to store application-specific 
 * proprietary data. The context specifies: an identifier field (1 to UINT_MAX bytes) in latin-1 encoding 
 * containing the owner identifier (typically an email address or URL) that uniquely identifies the 
 * application or organization, and a binary data field (1 to UINT_MAX bytes) containing the private data 
 * in any format chosen by the owner. Using unique identifiers prevents conflicts between different 
 * applications storing private data within the same tag, enabling custom metadata extensions without 
 * standardization.
 * 
 * @return List* - Heap allocated list containing two content contexts: identifier (1-UINT_MAX bytes) 
 * and data (1-UINT_MAX bytes). Caller must free with listFree()
 */
List *id3v2CreatePrivateFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // email
    void *toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("identifier"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for recommended buffer size frames (RBUF).
 * @details Constructs a list defining the structure of recommended buffer size frames that provide 
 * guidance for optimal streaming buffer sizes. The context specifies: a 3-byte buffer size field 
 * indicating the recommended minimum buffer size in bytes for streaming the audio file, a 1-byte 
 * flag field where bit 0 indicates whether embedded info is present and bit 1 indicates the next 
 * tag location, and an optional 4-byte offset field (0 to 4 bytes) containing the offset to the 
 * next tag if the flag indicates its presence. This structure helps streaming applications optimize 
 * buffering and locate subsequent tags efficiently.
 * 
 * @return List* - Heap allocated list containing three content contexts: buffer (3 bytes), flag 
 * (1 byte), and offset (0-4 bytes). Caller must free with listFree()
 */
List *id3v2CreateRecommendedBufferSizeFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // buffer size
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("buffer"), 3, 3);
    listInsertBack(l, toAdd);

    // bit
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("flag"), 1, 1);
    listInsertBack(l, toAdd);

    // offset
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("offset"), 4, 0);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for relative volume adjustment frames (RVA/RVAD/RVA2).
 * @details Constructs a simplified list defining the structure of relative volume adjustment frames 
 * used to specify recommended playback volume changes. Currently implemented as a single binary data 
 * field (1 to UINT_MAX bytes) containing the entire frame structure without parsing individual 
 * components. The complete RVA2 structure includes an identification string, channel type bytes, 
 * volume adjustment values (typically 2 bytes), bits representing peak volume, and optional peak 
 * volume data. This simplified implementation allows reading and preserving RVA frames while deferring 
 * detailed parsing to future enhancements or post-processing functions.
 * @warning THIS IS A BUMMY IMPLEMENTATION AND NEEDS TO BE UPDATED IN THE FUTURE
 * @param version The ID3v2 version (currently unused; all versions treated uniformly as binary data)
 * 
 * @return List* - Heap allocated list containing one content context: data (1-UINT_MAX bytes). 
 * Caller must free with listFree()
 */
List *id3v2CreateRelativeVolumeAdjustmentFrameContext(unsigned int version) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // data
    void *toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for reverb frames (RVRB).
 * @details Constructs a list defining the structure of reverb frames that specify recommended reverb 
 * effect parameters for audio playback. The context specifies: 2-byte reverb delay values for left 
 * and right channels in milliseconds, 1-byte bounce values for left and right channels indicating 
 * initial reflection intensity, four 1-byte feedback values defining the cross-channel feedback matrix 
 * (left-to-left, left-to-right, right-to-right, right-to-left), and 1-byte premix level values for 
 * left and right channels controlling the dry/wet mix. These ten fields provide comprehensive control 
 * over stereo reverb characteristics, enabling applications to recreate the intended spatial audio 
 * effects during playback.
 * 
 * @return List* - Heap allocated list containing ten content contexts: left (2 bytes), right (2 bytes), 
 * bounce left (1 byte), bounce right (1 byte), feedback ll (1 byte), feedback lr (1 byte), feedback rr 
 * (1 byte), feedback rl (1 byte), p left (1 byte), and p right (1 byte). Caller must free with listFree()
 */
List *id3v2CreateReverbFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // left
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("left"), 2, 2);
    listInsertBack(l, toAdd);

    // right
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("right"), 2, 2);
    listInsertBack(l, toAdd);

    // bounce left
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("bounce left"), 1, 1);
    listInsertBack(l, toAdd);

    // bounce right
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("bounce right"), 1, 1);
    listInsertBack(l, toAdd);

    // feedback left 2 left
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("feedback ll"), 1, 1);
    listInsertBack(l, toAdd);

    // feedback left 2 right
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("feedback lr"), 1, 1);
    listInsertBack(l, toAdd);

    // feedback right 2 right
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("feedback rr"), 1, 1);
    listInsertBack(l, toAdd);

    // feedback right 2 left
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("feedback rl"), 1, 1);
    listInsertBack(l, toAdd);

    // premix l
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("p left"), 1, 1);
    listInsertBack(l, toAdd);

    // premix r
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("p right"), 1, 1);
    listInsertBack(l, toAdd);
    return l;
}

/**
 * @brief Creates a context definition list for seek frames (SEEK).
 * @details Constructs a list defining the structure of seek frames that indicate the location of 
 * additional ID3v2 tags within the audio file. The context specifies a single 4-byte offset field 
 * containing the minimum number of bytes from the end of the current tag to the beginning of the 
 * next tag. This structure enables efficient tag discovery by allowing applications to skip directly 
 * to subsequent tags without scanning the entire file, particularly useful for files containing 
 * multiple ID3v2 tags at different locations.
 * 
 * @return List* - Heap allocated list containing one content context: offset (4 bytes). Caller 
 * must free with listFree()
 */
List *id3v2CreateSeekFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // offset
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("offset"), 4, 4);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for signature frames (SIGN).
 * @details Constructs a list defining the structure of signature frames used to provide digital 
 * signatures for tag authentication and integrity verification. The context specifies: a 1-byte 
 * symbol field identifying the group or frame type being signed, and a binary data field 
 * (1 to UINT_MAX bytes) containing the actual digital signature. The signature is calculated 
 * over specific tag data to ensure authenticity and detect unauthorized modifications. This 
 * structure enables verification that tag content has not been altered since the signature was 
 * applied by the legitimate owner or authority.
 * 
 * @return List* - Heap allocated list containing two content contexts: symbol (1 byte) and 
 * data (1-UINT_MAX bytes). Caller must free with listFree()
 */
List *id3v2CreateSignatureFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // symbol
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("symbol"), 1, 1);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for synchronised lyrics/text frames (SYLT).
 * @details Constructs a list defining the structure of synchronised lyrics or text that 
 * display lyrics, captions, or other text synchronized with audio playback. The context specifies: 
 * a 1-byte encoding field for text strings, a 3-byte language code in ISO-639-2 format, a 1-byte 
 * time stamp format indicator (MPEG frames, milliseconds, etc.), a 1-byte content type symbol 
 * identifying the text purpose (lyrics, transcription, movement description, etc.), a content 
 * descriptor field (1 to UINT_MAX bytes) in the specified encoding, a text field (1 to UINT_MAX 
 * bytes) containing the synchronized text content, a 4-byte time stamp indicating when to display 
 * the text, and an iterator context for repeating text/stamp pairs (minimum 5 bytes). This structure 
 * enables karaoke-style lyric display and timed caption presentation during playback.
 * 
 * @return List* - Heap allocated list containing eight content contexts: encoding (1 byte), language 
 * (3 bytes), format (1 byte), symbol (1 byte), description (1-UINT_MAX bytes), text (1-UINT_MAX bytes), 
 * stamp (4 bytes), and iter (5-UINT_MAX bytes). Caller must free with listFree()
 */
List *id3v2CreateSynchronisedLyricFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // language
    toAdd = (void *) id3v2CreateContentContext(noEncoding_context, id3v2djb2("language"), 3, 3);
    listInsertBack(l, toAdd);

    // format
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("format"), 1, 1);
    listInsertBack(l, toAdd);

    // symbol
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("symbol"), 1, 1);
    listInsertBack(l, toAdd);

    // desc
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("desc"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // text
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("text"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // stamp
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("stamp"), 4, 4);
    listInsertBack(l, toAdd);

    // iter
    toAdd = (void *) id3v2CreateContentContext(iter_context, id3v2djb2("iter"), UINT_MAX, 5);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for synchronised tempo codes frames (SYTC).
 * @details Constructs a simplified list defining the structure of synchronised tempo codes frames 
 * used to specify tempo/BPM changes throughout the audio track. The context specifies: a 1-byte 
 * time stamp format field indicating the timing unit (MPEG frames, milliseconds, etc.), and a 
 * binary data field (1 to UINT_MAX bytes) containing tempo change events. Currently implemented 
 * as binary data without parsing individual tempo entries, each of which would normally consist 
 * of a tempo value and corresponding time stamp. This simplified implementation allows reading 
 * and preserving SYTC frames while deferring detailed tempo event parsing to future enhancements 
 * or post-processing functions.
 * @warning THIS IS AN INCOMPLETE IMPLEMENTATION AND NEEDS TO BE UPDATED IN THE FUTURE
 * @return List* - Heap allocated list containing two content contexts: format (1 byte) and 
 * data (1-UINT_MAX bytes). Caller must free with listFree()
 */
List *id3v2CreateSynchronisedTempoCodesFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // format
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("format"), 1, 1);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for unique file identifier frames (UFID).
 * @details Constructs a list defining the structure of unique file identifier frames used to 
 * store database-specific identifiers for the audio file. The context specifies: a URL or owner 
 * identifier field (1 to UINT_MAX bytes) in latin-1 encoding identifying the database or system 
 * (such as "http://www.id3.org/dummy/ufid.html" or "http://musicbrainz.org"), and a binary data 
 * field (1 to 64 bytes) containing the unique identifier value assigned by that database. Multiple 
 * UFID frames with different owner identifiers enable the file to be tracked across various music 
 * databases and services simultaneously.
 * 
 * @return List* - Heap allocated list containing two content contexts: url (1-UINT_MAX bytes) 
 * and data (1-64 bytes). Caller must free with listFree()
 */
List *id3v2CreateUniqueFileIdentifierFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // url
    void *toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("url"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), 64, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for terms of use frames (USER).
 * @details Constructs a list defining the structure of terms of use frames that specify legal 
 * usage conditions and licensing information for the audio file. The context specifies: a 1-byte 
 * encoding field for text strings, a 3-byte language code in ISO-639-2 format indicating the 
 * language of the terms, and a text field (1 to UINT_MAX bytes) in the specified encoding 
 * containing the complete terms of use, copyright restrictions, or licensing conditions. This 
 * structure enables content creators and distributors to embed legally binding usage terms 
 * directly within the audio file metadata, ensuring users are informed of restrictions and 
 * requirements.
 * 
 * @return List* - Heap allocated list containing three content contexts: encoding (1 byte), 
 * language (3 bytes), and text (1-UINT_MAX bytes). Caller must free with listFree()
 */
List *id3v2CreateTermsOfUseFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // language
    toAdd = (void *) id3v2CreateContentContext(noEncoding_context, id3v2djb2("language"), 3, 1);
    listInsertBack(l, toAdd);

    // text
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("text"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}


/**
 * @brief Creates a context definition list for unsynchronised lyrics/text frames (USLT).
 * @details Constructs a list defining the structure of unsynchronised lyrics or text comment frames 
 * that contain complete lyrics or text without timing information. The context specifies: a 1-byte 
 * encoding field for text strings, a 3-byte language code in ISO-639-2 format indicating the text 
 * language, a content descriptor field (1 to UINT_MAX bytes) in the specified encoding providing 
 * a brief description of the content, and a text field (1 to UINT_MAX bytes) in the specified 
 * encoding containing the full lyrics or extended text. Unlike synchronised lyrics (SYLT), this 
 * frame stores text without time stamps, making it suitable for displaying complete lyrics statically 
 * or for general text content that doesn't require playback synchronization.
 * 
 * @return List* - Heap allocated list containing four content contexts: encoding (1 byte), language 
 * (3 bytes), description (1-UINT_MAX bytes), and text (1-UINT_MAX bytes). Caller must free with 
 * listFree()
 */
List *id3v2CreateUnsynchronisedLyricFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // language
    toAdd = (void *) id3v2CreateContentContext(noEncoding_context, id3v2djb2("language"), 3, 3);
    listInsertBack(l, toAdd);

    // desc
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("desc"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // text
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("text"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a context definition list for generic frames with unknown or unrecognized structure.
 * @details Constructs a minimal fallback list for handling frames whose structure is not explicitly 
 * defined or recognized by the parser. The context specifies a single binary data field (1 to UINT_MAX 
 * bytes) that captures the entire frame content without interpretation. This generic context enables 
 * the library to preserve and handle unrecognized frame types, experimental frames, or proprietary 
 * frame extensions without data loss, allowing the complete tag to be read, modified, and written 
 * back even when some frame types are unknown.
 * 
 * @return List* - Heap allocated list containing one content context: binary data (1-UINT_MAX bytes) 
 * marked with unknown identifier. Caller must free with listFree()
 */
List *id3v2CreateGenericFrameContext(void) {
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("?"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Creates a default mapping of frame identifiers to their corresponding parse contexts for all ID3v2 versions.
 * @details Constructs and populates a hash table that maps frame ID strings to context definition lists used 
 * by the parser to interpret frame structures. The mapping varies by ID3v2 version: v2.2 uses 3-character frame 
 * IDs (e.g., "TIT", "COM", "PIC"), while v2.3 and v2.4 use 4-character IDs (e.g., "TIT2", "COMM", "APIC"). 
 * The function creates context lists for all standard frame types including text frames (T*), URL frames (W*), 
 * comment frames, picture frames, and specialized frames like equalization, synchronised lyrics, and encryption 
 * registration. Three fallback contexts are provided for unknown or generic frames: "?" handles completely 
 * unrecognized frame types, "T" provides generic text frame parsing, and "W" provides generic URL frame parsing. 
 * Each context list is temporarily assigned to the hash table then freed, as the table makes its own internal copy. 
 * This centralized mapping enables version-specific frame parsing without hardcoding structure definitions throughout 
 * the parser.
 * 
 * @param version The ID3v2 tag version (ID3V2_TAG_VERSION_2, ID3V2_TAG_VERSION_3, or ID3V2_TAG_VERSION_4)
 * 
 * @return HashTable* - Heap allocated hash table mapping frame ID strings to context definition lists. Contains 
 * 60+ standard frame mappings plus fallback contexts. Caller must free with hashTableDestroy()
 */
HashTable *id3v2CreateDefaultIdentifierContextPairings(unsigned int version) {
    size_t minFrameContexts = 66;
    HashTable *table = hashTableCreate(minFrameContexts, id3v2DeleteContentContextList, id3v2PrintContentContextList,
                                       id3v2CopyContentContextList);
    List *l = NULL;

    switch (version) {
        case ID3V2_TAG_VERSION_2:

            hashTableInsert(table, "BUF", (l = id3v2CreateRecommendedBufferSizeFrameContext()));
            listFree(l);

            hashTableInsert(table, "CNT", (l = id3v2CreatePlayCounterFrameContext()));
            listFree(l);

            hashTableInsert(table, "COM", (l = id3v2CreateCommentFrameContext()));
            listFree(l);
            hashTableInsert(table, "CRA", (l = id3v2CreateAudioEncryptionFrameContext()));
            listFree(l);
            hashTableInsert(table, "CRM", (l = id3v2CreateEncryptedMetaFrameContext()));
            listFree(l);

            hashTableInsert(table, "ETC", (l = id3v2CreateEventTimingCodesFrameContext()));
            listFree(l);
            hashTableInsert(table, "EQU", (l = id3v2CreateEqualizationFrameContext(version)));
            listFree(l);

            hashTableInsert(table, "GEO", (l = id3v2CreateGeneralEncapsulatedObjectFrameContext()));
            listFree(l);

            hashTableInsert(table, "IPL", (l = id3v2CreateInvolvedPeopleListFrameContext()));
            listFree(l);

            hashTableInsert(table, "LNK", (l = id3v2CreateLinkedInformationFrameContext()));
            listFree(l);

            hashTableInsert(table, "MCI", (l = id3v2CreateMusicCDIdentifierFrameContext()));
            listFree(l);
            hashTableInsert(table, "MLL", (l = id3v2CreateMPEGLocationLookupTableFrameContext()));
            listFree(l);

            hashTableInsert(table, "PIC", (l = id3v2CreateAttachedPictureFrameContext(version)));
            listFree(l);
            hashTableInsert(table, "POP", (l = id3v2CreatePopularimeterFrameContext()));
            listFree(l);

            hashTableInsert(table, "REV", (l = id3v2CreateReverbFrameContext()));
            listFree(l);
            hashTableInsert(table, "RVA", (l = id3v2CreateRelativeVolumeAdjustmentFrameContext(version)));
            listFree(l);

            hashTableInsert(table, "SLT", (l = id3v2CreateSynchronisedLyricFrameContext()));
            listFree(l);
            hashTableInsert(table, "STC", (l = id3v2CreateSynchronisedTempoCodesFrameContext()));
            listFree(l);

            hashTableInsert(table, "TAL", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TBP", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TCM", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TCO", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TCR", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TDA", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TDY", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TEN", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TFT", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TIM", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TKE", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TLA", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TLE", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TMT", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOA", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOF", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOL", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOR", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOT", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TP1", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TP2", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TP3", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TP4", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPA", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPB", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TRC", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TRD", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TRK", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TSI", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TSS", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TT1", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TT2", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TT3", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TXT", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TXX", (l = id3v2CreateUserDefinedTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TYE", (l = id3v2CreateTextFrameContext()));
            listFree(l);

            hashTableInsert(table, "UFI", (l = id3v2CreateUniqueFileIdentifierFrameContext()));
            listFree(l);
            hashTableInsert(table, "ULT", (l = id3v2CreateUnsynchronisedLyricFrameContext()));
            listFree(l);

            hashTableInsert(table, "WAF", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WAR", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WAS", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WCM", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WCP", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WPB", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WXX", (l = id3v2CreateUserDefinedURLFrameContext()));
            listFree(l);

            break;
        case ID3V2_TAG_VERSION_3:

            hashTableInsert(table, "AENC", (l = id3v2CreateAudioEncryptionFrameContext()));
            listFree(l);
            hashTableInsert(table, "APIC", (l = id3v2CreateAttachedPictureFrameContext(version)));
            listFree(l);

            hashTableInsert(table, "COMM", (l = id3v2CreateCommentFrameContext()));
            listFree(l);
            hashTableInsert(table, "COMR", (l = id3v2CreateCommercialFrameContext()));
            listFree(l);

            hashTableInsert(table, "ENCR", (l = id3v2CreateRegistrationFrameContext()));
            listFree(l);
            hashTableInsert(table, "EQUA", (l = id3v2CreateEqualizationFrameContext(version)));
            listFree(l);
            hashTableInsert(table, "ETCO", (l = id3v2CreateEventTimingCodesFrameContext()));
            listFree(l);

            hashTableInsert(table, "GEOB", (l = id3v2CreateGeneralEncapsulatedObjectFrameContext()));
            listFree(l);
            hashTableInsert(table, "GRID", (l = id3v2CreateRegistrationFrameContext()));
            listFree(l);

            hashTableInsert(table, "IPLS", (l = id3v2CreateInvolvedPeopleListFrameContext()));
            listFree(l);

            hashTableInsert(table, "LINK", (l = id3v2CreateLinkedInformationFrameContext()));
            listFree(l);

            hashTableInsert(table, "MCDI", (l = id3v2CreateMusicCDIdentifierFrameContext()));
            listFree(l);
            hashTableInsert(table, "MLLT", (l = id3v2CreateMPEGLocationLookupTableFrameContext()));
            listFree(l);

            hashTableInsert(table, "OWNE", (l = id3v2CreateOwnershipFrameContext()));
            listFree(l);

            hashTableInsert(table, "PRIV", (l = id3v2CreatePrivateFrameContext()));
            listFree(l);
            hashTableInsert(table, "PCNT", (l = id3v2CreatePlayCounterFrameContext()));
            listFree(l);
            hashTableInsert(table, "POPM", (l = id3v2CreatePopularimeterFrameContext()));
            listFree(l);
            hashTableInsert(table, "POSS", (l = id3v2CreatePositionSynchronisationFrameContext()));
            listFree(l);

            hashTableInsert(table, "RBUF", (l = id3v2CreateRecommendedBufferSizeFrameContext()));
            listFree(l);
            hashTableInsert(table, "RVAD", (l = id3v2CreateRelativeVolumeAdjustmentFrameContext(version)));
            listFree(l);
            hashTableInsert(table, "RVRB", (l = id3v2CreateReverbFrameContext()));
            listFree(l);

            hashTableInsert(table, "SYLT", (l = id3v2CreateSynchronisedLyricFrameContext()));
            listFree(l);
            hashTableInsert(table, "SYTC", (l = id3v2CreateSynchronisedTempoCodesFrameContext()));
            listFree(l);

            hashTableInsert(table, "TALB", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TBPM", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TCOM", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TCON", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TCOP", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TDAT", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TDLY", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TENC", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TEXT", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TFLT", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TIME", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TIT1", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TIT2", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TIT3", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TKEY", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TLAN", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TLEN", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TMED", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOAL", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOFN", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOLY", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOPE", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TORY", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOWN", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPE1", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPE2", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPE3", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPE4", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPOS", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPUB", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TRCK", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TRDA", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TRSN", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TRSO", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TSIZ", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TSRC", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TSSE", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TYER", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TXXX", (l = id3v2CreateUserDefinedTextFrameContext()));
            listFree(l);

            hashTableInsert(table, "UFID", (l = id3v2CreateUniqueFileIdentifierFrameContext()));
            listFree(l);
            hashTableInsert(table, "USER", (l = id3v2CreateTermsOfUseFrameContext()));
            listFree(l);
            hashTableInsert(table, "USLT", (l = id3v2CreateUnsynchronisedLyricFrameContext()));
            listFree(l);

            hashTableInsert(table, "WCOM", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WCOP", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WOAF", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WOAR", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WOAS", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WORS", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WPAY", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WPUB", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WXXX", (l = id3v2CreateUserDefinedURLFrameContext()));
            listFree(l);

            break;
        case ID3V2_TAG_VERSION_4:

            hashTableInsert(table, "AENC", (l = id3v2CreateAudioEncryptionFrameContext()));
            listFree(l);
            hashTableInsert(table, "APIC", (l = id3v2CreateAttachedPictureFrameContext(version)));
            listFree(l);
            hashTableInsert(table, "ASPI", (l = id3v2CreateAudioSeekPointIndexFrameContext()));
            listFree(l);

            hashTableInsert(table, "COMM", (l = id3v2CreateCommentFrameContext()));
            listFree(l);
            hashTableInsert(table, "COMR", (l = id3v2CreateCommercialFrameContext()));
            listFree(l);

            hashTableInsert(table, "ENCR", (l = id3v2CreateRegistrationFrameContext()));
            listFree(l);
            hashTableInsert(table, "EQU2", (l = id3v2CreateEqualizationFrameContext(version)));
            listFree(l);
            hashTableInsert(table, "ETCO", (l = id3v2CreateEventTimingCodesFrameContext()));
            listFree(l);

            hashTableInsert(table, "GEOB", (l = id3v2CreateGeneralEncapsulatedObjectFrameContext()));
            listFree(l);
            hashTableInsert(table, "GRID", (l = id3v2CreateRegistrationFrameContext()));
            listFree(l);

            hashTableInsert(table, "LINK", (l = id3v2CreateLinkedInformationFrameContext()));
            listFree(l);

            hashTableInsert(table, "MCDI", (l = id3v2CreateMusicCDIdentifierFrameContext()));
            listFree(l);
            hashTableInsert(table, "MLLT", (l = id3v2CreateMPEGLocationLookupTableFrameContext()));
            listFree(l);

            hashTableInsert(table, "OWNE", (l = id3v2CreateOwnershipFrameContext()));
            listFree(l);

            hashTableInsert(table, "PRIV", (l = id3v2CreatePrivateFrameContext()));
            listFree(l);
            hashTableInsert(table, "PCNT", (l = id3v2CreatePlayCounterFrameContext()));
            listFree(l);
            hashTableInsert(table, "POPM", (l = id3v2CreatePopularimeterFrameContext()));
            listFree(l);
            hashTableInsert(table, "POSS", (l = id3v2CreatePositionSynchronisationFrameContext()));
            listFree(l);

            hashTableInsert(table, "RBUF", (l = id3v2CreateRecommendedBufferSizeFrameContext()));
            listFree(l);
            hashTableInsert(table, "RVA2", (l = id3v2CreateRelativeVolumeAdjustmentFrameContext(version)));
            listFree(l);
            hashTableInsert(table, "RVRB", (l = id3v2CreateReverbFrameContext()));
            listFree(l);

            hashTableInsert(table, "SEEK", (l = id3v2CreateSeekFrameContext()));
            listFree(l);
            hashTableInsert(table, "SIGN", (l = id3v2CreateSignatureFrameContext()));
            listFree(l);
            hashTableInsert(table, "SYLT", (l = id3v2CreateSynchronisedLyricFrameContext()));
            listFree(l);
            hashTableInsert(table, "SYTC", (l = id3v2CreateSynchronisedTempoCodesFrameContext()));
            listFree(l);

            hashTableInsert(table, "TALB", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TBPM", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TCOM", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TCON", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TCOP", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TDEN", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TDLY", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TDOR", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TDRC", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TDRL", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TDTG", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TENC", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TEXT", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TFLT", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TIPL", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TIT1", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TIT2", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TIT3", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TKEY", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TLAN", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TLEN", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TMCL", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TMED", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TMOO", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOAL", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOFN", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOLY", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOPE", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOWN", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPE1", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPE2", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPE3", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPE4", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPOS", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPRO", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPUB", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TRCK", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TRSN", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TRSO", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TSOA", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TSOP", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TSOT", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TSRC", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TSSE", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TSST", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TXXX", (l = id3v2CreateUserDefinedTextFrameContext()));
            listFree(l);

            hashTableInsert(table, "UFID", (l = id3v2CreateUniqueFileIdentifierFrameContext()));
            listFree(l);
            hashTableInsert(table, "USER", (l = id3v2CreateTermsOfUseFrameContext()));
            listFree(l);
            hashTableInsert(table, "USLT", (l = id3v2CreateUnsynchronisedLyricFrameContext()));
            listFree(l);

            hashTableInsert(table, "WCOM", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WCOP", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WOAF", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WOAR", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WOAS", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WORS", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WPAY", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WPUB", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WXXX", (l = id3v2CreateUserDefinedURLFrameContext()));
            listFree(l);

            break;
        default:
            break;
    }

    hashTableInsert(table, "?", (l = id3v2CreateGenericFrameContext()));
    listFree(l);
    hashTableInsert(table, "T", (l = id3v2CreateTextFrameContext()));
    listFree(l);
    hashTableInsert(table, "W", (l = id3v2CreateURLFrameContext()));
    listFree(l);

    return table;
}

/**
 * @brief Inserts a new frame identifier and context pairing into an existing frame mapping table.
 * @details Adds or updates a mapping between a frame ID string and its corresponding parse context definition 
 * list in the provided hash table. This function enables extension of the default frame mappings with custom 
 * frame types or modification of existing frame parsing behavior. Performs validation to ensure all parameters 
 * are non-NULL before insertion. The hash table makes an internal copy of the context list, so the caller 
 * retains ownership of the original context parameter. Used to register proprietary frame formats or 
 * override default parsing contexts for specific applications.
 * 
 * @param identifierContextPairs The hash table containing frame ID to context mappings
 * @param key The frame identifier string (3 characters for v2.2, 4 characters for v2.3/v2.4)
 * @param context The context definition list describing the frame structure
 * 
 * @return bool - true if the pairing was successfully inserted, false if any parameter is NULL
 */
bool id3v2InsertIdentifierContextPair(HashTable *identifierContextPairs, char key[ID3V2_FRAME_ID_MAX_SIZE],
                                      List *context) {
    if (!identifierContextPairs || !key || !context) {
        return false;
    }

    hashTableInsert(identifierContextPairs, key, context);

    return true;
}

/**
 * @brief Creates a binary representation of a content context structure for serialization.
 * @details Converts an Id3v2ContentContext structure into a compact binary format suitable for 
 * storage or transmission. The serialization layout consists of: a 1-byte type field identifying 
 * the context type, followed by three size_t values (key hash, maximum bytes, minimum bytes) 
 * encoded in binary format. The total serialized size is (3 × sizeof(size_t)) + 1 bytes. Uses 
 * a ByteStream for structured binary writing and the sttob() utility to convert size_t values 
 * to byte arrays in a platform-independent manner. The serialized format enables context structures 
 * to be persisted to disk, transmitted over networks, or embedded in other data structures while 
 * preserving all essential parsing information.
 * 
 * @param cc The content context structure to serialize
 * @param outl Output parameter receiving the length of the serialized binary data in bytes
 * 
 * @return uint8_t* - Heap allocated byte array containing the serialized context. Returns NULL 
 * if cc is NULL (with outl set to 0). Caller must free the returned buffer
 */
uint8_t *id3v2ContextSerialize(Id3v2ContentContext *cc, size_t *outl) {
    ByteStream *stream = NULL;
    size_t s = 0;
    unsigned char *convi = NULL;
    uint8_t *out = NULL;

    if (cc == NULL) {
        *outl = 0;
        return out;
    }

    s += (sizeof(size_t) * 3) + 1;

    stream = byteStreamCreate(NULL, s);

    byteStreamWrite(stream, (uint8_t *) &cc->type, 1);

    convi = sttob(cc->key);
    byteStreamWrite(stream, convi, sizeof(size_t));
    free(convi);

    convi = sttob(cc->max);
    byteStreamWrite(stream, convi, sizeof(size_t));
    free(convi);

    convi = sttob(cc->min);
    byteStreamWrite(stream, convi, sizeof(size_t));
    free(convi);

    byteStreamRewind(stream);

    out = calloc(stream->bufferSize, sizeof(uint8_t));
    *outl = stream->bufferSize;
    byteStreamRead(stream, out, stream->bufferSize);
    byteStreamDestroy(stream);
    return out;
}

/**
 * @brief Converts a content context structure to its JSON representation.
 * @details Serializes an Id3v2ContentContext structure into a compact JSON string containing the 
 * context's type identifier, key hash value, maximum byte constraint, and minimum byte constraint. 
 * The function calculates the exact memory requirement using snprintf with a NULL buffer to determine 
 * string length, then allocates and formats the JSON output. For NULL input, returns an empty JSON 
 * object string "{}". This JSON format enables context structures to be transmitted via web APIs, 
 * stored in configuration files, or logged for debugging purposes while maintaining human readability 
 * and standard JSON parser compatibility.
 * 
 * Example output for a text encoding context:
 * ```json
 * {"type":2,"key":2087083923,"max":255,"min":1}
 * ```
 * 
 * Example output for NULL input:
 * ```json
 * {}
 * ```
 * 
 * @param cc The content context structure to convert, or NULL for empty object
 * 
 * @return char* - Heap allocated null-terminated JSON string. Returns "{}" if cc is NULL. 
 * Caller must free the returned string
 */
char *id3v2ContextToJSON(const Id3v2ContentContext *cc) {
    char *json = NULL;
    size_t memCount = 3;
    if (cc == NULL) {
        json = calloc(memCount, sizeof(char));
        memcpy(json, "{}\0", memCount);
        return json;
    }

    memCount += snprintf(NULL, 0,
                         "{\"type\":%d,\"key\":%zu,\"max\":%zu,\"min\":%zu}",
                         cc->type,
                         cc->key,
                         cc->max,
                         cc->min);

    json = calloc(memCount + 1, sizeof(char));

    (void) snprintf(json, memCount,
                    "{\"type\":%d,\"key\":%zu,\"max\":%zu,\"min\":%zu}",
                    cc->type,
                    cc->key,
                    cc->max,
                    cc->min);

    return json;
}
