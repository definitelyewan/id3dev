/**
 * @file id3v2Frame.c
 * @author Ewan Jones
 * @brief Function implementation for ID3v2 frame lifecycle, traversal, serialization, and content management
 * 
 * @version 26.01
 * @date 2024-04-11 - 2026-01-17
 * 
 * @copyright Copyright (c) 2024 - 2026
 * 
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "id3v2/id3v2Frame.h"
#include "id3v2/id3v2Context.h"
#include "id3dependencies/ByteStream/include/byteInt.h"
#include "id3dependencies/ByteStream/include/byteUnicode.h"
#include "id3dependencies/ByteStream/include/byteStream.h"

static char *internal_base64Encode(const unsigned char *input, size_t inputLength) {
    static const unsigned char base64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const size_t outputLength = 4 * ((inputLength + 2) / 3);
    unsigned char *output = malloc(outputLength + 1);

    if (output == NULL) {
        return NULL;
    }

    size_t i = 0;
    size_t j = 0;
    for (i = 0, j = 0; i < inputLength; i += 3, j += 4) {
        const unsigned int byte1 = input[i];
        const unsigned int byte2 = (i + 1 < inputLength) ? input[i + 1] : 0;
        const unsigned int byte3 = (i + 2 < inputLength) ? input[i + 2] : 0;

        output[j] = base64Chars[byte1 >> 2];
        output[j + 1] = base64Chars[((byte1 & 0x03) << 4) | (byte2 >> 4)];
        output[j + 2] = (i + 1 < inputLength) ? base64Chars[((byte2 & 0x0F) << 2) | (byte3 >> 6)] : (unsigned char) '=';
        output[j + 3] = (i + 2 < inputLength) ? base64Chars[byte3 & 0x3F] : (unsigned char) '=';
    }

    output[outputLength] = '\0';
    return (char *) output;
}


/**
 * @brief Creates an ID3v2 frame header structure with specified flags and metadata
 * @details Allocates and initializes a frame header on the heap. The header contains a frame
 * identifier and control flags for preservation (tag/file alter, read-only) and format 
 * (compression, encryption, grouping, unsynchronisation). Format flags are only used in 
 * ID3v2.3/2.4; they are ignored in v2.2 but can be set for forward compatibility.
 * 
 * @param id - Frame identifier array of ID3V2_FRAME_ID_MAX_SIZE. Examples: "TIT2", "TPE1", "APIC"
 * @param tagAlter - Tag alter preservation flag meaning the tag should be preserved
 * @param fileAlter - File alter preservation flag
 * @param readOnly - Read-only flag
 * @param unsync - Frame-level unsynchronisation flag (ID3v2.4 only, false sync signals removed)
 * @param decompressionSize - Original uncompressed size in bytes (0 = no compression, ID3v2.3/2.4 only)
 * @param encryptionSymbol - Encryption method symbol (0 = no encryption, ID3v2.3/2.4 only)
 * @param groupSymbol - Group identifier for related frames (0 = no grouping, ID3v2.3/2.4 only)
 * 
 * @return Id3v2FrameHeader* - Heap-allocated Id3v2FrameHeader structure. Caller must free with id3v2DestroyFrameHeader
 */
Id3v2FrameHeader *id3v2CreateFrameHeader(uint8_t id[ID3V2_FRAME_ID_MAX_SIZE], bool tagAlter, bool fileAlter,
                                         bool readOnly, bool unsync, uint32_t decompressionSize,
                                         uint8_t encryptionSymbol, uint8_t groupSymbol) {
    Id3v2FrameHeader *h = malloc(sizeof(Id3v2FrameHeader));

    for (int i = 0; i < ID3V2_FRAME_ID_MAX_SIZE; i++) {
        h->id[i] = id[i];
    }

    h->tagAlterPreservation = tagAlter;
    h->fileAlterPreservation = fileAlter;
    h->readOnly = readOnly;
    h->decompressionSize = decompressionSize;
    h->encryptionSymbol = encryptionSymbol;
    h->groupSymbol = groupSymbol;
    h->unsynchronisation = unsync;

    return h;
}

/**
 * @brief Frees all memory allocated for a frame header and nullifies the pointer
 * @details Safely deallocates a frame header structure created by id3v2CreateFrameHeader.
 * Sets the pointer to NULL after freeing to prevent use-after-free errors. If the pointer
 * is already NULL, no action is taken.
 * 
 * @param toDelete - Pointer to frame header pointer to be freed and nullified
 */
void id3v2DestroyFrameHeader(Id3v2FrameHeader **toDelete) {
    if (*toDelete) {
        free(*toDelete);
        *toDelete = NULL;
        toDelete = NULL;
    }
}

/**
 * @brief Creates a content entry structure with a deep copy of the provided data.
 * @details Allocates an Id3v2ContentEntry on the heap and performs a deep copy of the provided
 * entry data. Content entries store individual field values within ID3v2 frames (e.g., encoding
 * byte, text strings, binary data). If size is 0, creates an empty entry with NULL data.
 * 
 * @param entry - Pointer to data to copy into the content entry. Ignored if size is 0
 * @param size - Size of data in bytes. If 0, creates empty entry with NULL data pointer
 * 
 * @return Id3v2ContentEntry * - Heap-allocated Id3v2ContentEntry containing a deep copy of the data
 */
Id3v2ContentEntry *id3v2CreateContentEntry(void *entry, size_t size) {
    Id3v2ContentEntry *ce = malloc(sizeof(Id3v2ContentEntry));

    if (!size) {
        ce->entry = NULL;
        ce->size = 0;
    } else {
        void *tmp = malloc(size);
        memcpy(tmp, entry, size);

        ce->entry = tmp;
        ce->size = size;
    }

    return ce;
}

/**
 * @brief Compares two content entries byte-by-byte and returns the difference
 * @details Performs lexicographic comparison of entry data up to the smaller of the two sizes.
 * Size differences are ignored for compatibility with various ID3v2 tag writers (Jaikoz, iTunes,
 * Mp3tag, Mp3diag, kid3) which write strings with different padding/termination. This design
 * prioritizes data content over exact size matching.
 * 
 * @param first - First content entry to compare
 * @param second - Second content entry to compare
 * 
 * @return int - 0 if entries are equal up to minimum size, positive if first > second, negative if
 * first < second, or 1 if either parameter is NULL
 */
int id3v2CompareContentEntry(const void *first, const void *second) {
    const Id3v2ContentEntry *one = (Id3v2ContentEntry *) first;
    const Id3v2ContentEntry *two = (Id3v2ContentEntry *) second;

    if (one == NULL || two == NULL) {
        return 1;
    }

    int diff = 0;
    const size_t usableSize = (one->size <= two->size) ? one->size : two->size;
    for (size_t i = 0; i < usableSize; i++) {
        if (((unsigned char *) one->entry)[i] != ((unsigned char *) two->entry)[i]) {
            diff = ((unsigned char *) one->entry)[i] - ((unsigned char *) two->entry)[i];
            return diff;
        }
    }

    return 0;
}

/**
 * @brief Generates a string representation of a content entry for debugging.
 * @details Creates a formatted string displaying the entry's data size and memory address.
 * Can be used as a callback function for list printing operations. 
 * Caller must free the returned string.
 * 
 * @param toBePrinted - Content entry to represent as a string
 * 
 * @return char* - Heap allocated string in format "Size: <size>, data: <address>\n"
 */
char *id3v2PrintContentEntry(const void *toBePrinted) {
    const Id3v2ContentEntry *e = (Id3v2ContentEntry *) toBePrinted;
    const int memCount = snprintf(NULL, 0, "Size: %zu, data: %p\n", e->size, e->entry);

    char *str = calloc(memCount + 1, sizeof(char));

    (void) snprintf(str, memCount, "Size: %zu, data: %p\n", e->size, e->entry);

    return str;
}

/**
 * @brief Creates a deep copy of a content entry
 * @details Duplicates a content entry by creating a new structure with independently allocated
 * data. Can be used as a callback function for list copy operations.
 * 
 * @param toBeCopied - Content entry to copy
 * 
 * @return void* - Heap allocated content entry containing a deep copy of the data
 * with id3v2DeleteContentEntry()
 */
void *id3v2CopyContentEntry(const void *toBeCopied) {
    Id3v2ContentEntry *e = (Id3v2ContentEntry *) toBeCopied;

    return id3v2CreateContentEntry(e->entry, e->size);
}

/**
 * @brief Frees all memory allocated for a content entry structure.
 * @details Safely deallocates an Id3v2ContentEntry. Frees the 
 * internal data buffer if present, then the structure itself. Can 
 * be used as a callback for list deletion operations.
 * 
 * @param toBeDeleted - Pointer to Id3v2ContentEntry to be freed
 */
void id3v2DeleteContentEntry(void *toBeDeleted) {
    Id3v2ContentEntry *e = (Id3v2ContentEntry *) toBeDeleted;

    if (e->entry != NULL) {
        free(e->entry);
    }
    free(e);
}

/**
 * @brief Frees all memory allocated for an ID3v2 frame structure.
 * @details Wrapper around id3v2DestroyFrame for use as a callback in list operations.
 * Deallocates the frame header, contexts, entries, and the frame structure itself.
 * 
 * @param toBeDeleted - Pointer to Id3v2Frame to be freed
 */
void id3v2DeleteFrame(void *toBeDeleted) {
    Id3v2Frame *f = (Id3v2Frame *) toBeDeleted;
    id3v2DestroyFrame(&f);
}

/**
 * @brief Performs deep comparison of two ID3v2 frame structures
 * @details Compares all frame components including header fields (ID, flags, symbols), 
 * content entries, and contexts. Can be used as a callback for list comparison operations.
 * 
 * @param first - First frame to compare
 * @param second - Second frame to compare
 * 
 * @return int - 0 if frames are equal, negative if first < second, positive if first > second
 */
int id3v2CompareFrame(const void *first, const void *second) {
    const Id3v2Frame *f = (Id3v2Frame *) first;
    const Id3v2Frame *s = (Id3v2Frame *) second;
    ListIter i1, i2;
    void *tmp1 = NULL;
    void *tmp2 = NULL;
    int diff = 0;

    if (f == NULL) {
        return -1;
    }

    if (s == NULL) {
        return 1;
    }

    diff = memcmp(f->header->id, s->header->id, ID3V2_FRAME_ID_MAX_SIZE);
    if (diff != 0) {
        return diff;
    }

    diff = (int) f->header->decompressionSize - (int) s->header->decompressionSize;
    if (diff != 0) {
        return diff;
    }

    diff = f->header->encryptionSymbol - s->header->encryptionSymbol;
    if (diff != 0) {
        return diff;
    }

    if (f->header->fileAlterPreservation != s->header->fileAlterPreservation) {
        return 1;
    }

    diff = f->header->groupSymbol - s->header->groupSymbol;
    if (diff != 0) {
        return diff;
    }

    if (f->header->readOnly != s->header->readOnly) {
        return 1;
    }

    if (f->header->tagAlterPreservation != s->header->tagAlterPreservation) {
        return 1;
    }

    if (f->header->unsynchronisation != s->header->unsynchronisation) {
        return 1;
    }

    diff = (int) f->entries->length - (int) s->entries->length;
    if (diff != 0) {
        return diff;
    }

    i1 = listCreateIterator(f->entries);
    i2 = listCreateIterator(s->entries);

    while ((tmp1 = listIteratorNext(&i1)) != NULL) {
        tmp2 = listIteratorNext(&i2);

        diff = id3v2CompareContentEntry(tmp1, tmp2);

        if (diff != 0) {
            return diff;
        }
    }

    diff = (int) f->contexts->length - (int) s->contexts->length;
    if (diff != 0) {
        return diff;
    }

    i1 = listCreateIterator(f->contexts);
    i2 = listCreateIterator(s->contexts);

    while ((tmp1 = listIteratorNext(&i1)) != NULL) {
        tmp2 = listIteratorNext(&i2);

        diff = id3v2CompareContentContext(tmp1, tmp2);
        if (diff != 0) {
            return diff;
        }
    }

    return diff;
}

/**
 * @brief Generates a string representation of a frame for debugging
 * @details Creates a formatted string displaying memory addresses of the frame's header,
 * contexts, and entries. Can be used as a callback for list printing operations.
 * Caller must free the returned string.
 * 
 * @param toBePrinted - Frame to represent as a string
 * 
 * @return char* - Heap allocated string in format "header : <addr>, context : <addr>, entries : <addr>"
 */
char *id3v2PrintFrame(const void *toBePrinted) {
    const Id3v2Frame *f = (Id3v2Frame *) toBePrinted;
    char *s = NULL;

    const int memCount = snprintf(NULL, 0, "header : %p, context : %p, entries : %p", f->header, f->contexts,
                                  f->entries);

    s = calloc(memCount + 1, sizeof(char));

    (void) snprintf(s, memCount, "header : %p, context : %p, entries : %p", f->header, f->contexts, f->entries);

    return s;
}

/**
 * @brief Creates a deep copy of an ID3v2 frame structure.
 * @details Duplicates all frame components including header, contexts, and entries with 
 * independently allocated memory. Can be used as a callback for list copy operations.
 * 
 * @param toBeCopied - Frame to copy
 * 
 * @return void* - Heap allocated frame containing deep copies of all components
 */
void *id3v2CopyFrame(const void *toBeCopied) {
    Id3v2Frame *f = (Id3v2Frame *) toBeCopied;

    Id3v2FrameHeader *h = id3v2CreateFrameHeader(f->header->id,
                                                 f->header->tagAlterPreservation,
                                                 f->header->fileAlterPreservation,
                                                 f->header->readOnly,
                                                 f->header->unsynchronisation,
                                                 f->header->decompressionSize,
                                                 f->header->encryptionSymbol,
                                                 f->header->groupSymbol);

    return id3v2CreateFrame(h, listDeepCopy(f->contexts), listDeepCopy(f->entries));
}


/**
 * @brief Creates an ID3v2 frame structure from provided components.
 * @details Allocates a frame on the heap and assembles it from a header, contexts list,
 * and entries list. The frame takes ownership of all provided components.
 * 
 * @param header - Frame header containing ID and flags
 * @param context - List of content contexts defining entry structures
 * @param entries - List of content entries containing frame data
 * 
 * @return Id3v2Frame* - Heap allocated Id3v2Frame structure. Caller must free with id3v2DestroyFrame()
 */
Id3v2Frame *id3v2CreateFrame(Id3v2FrameHeader *header, List *context, List *entries) {
    Id3v2Frame *frame = malloc(sizeof(Id3v2Frame));

    frame->contexts = context;
    frame->entries = entries;
    frame->header = header;

    return frame;
}

/**
 * @brief Frees all memory allocated for an ID3v2 frame structure and nullifies the pointer.
 * @details Completely deallocates a frame by freeing its contexts list, entries list, 
 * frame header, and the frame structure itself. Sets the pointer to NULL after freeing 
 * to prevent use-after-free errors. If the pointer is already NULL, no action is taken.
 * 
 * @param toDelete - Pointer to frame pointer to be freed and nullified
 */
void id3v2DestroyFrame(Id3v2Frame **toDelete) {
    if (*toDelete) {
        listFree((*toDelete)->contexts);
        listFree((*toDelete)->entries);
        id3v2DestroyFrameHeader(&(*toDelete)->header);
        free(*toDelete);
        *toDelete = NULL;
        toDelete = NULL;
    }
}


/**
 * @brief Creates an empty frame structure with zero-initialized entries based on context lookup.
 * @details Resolves frame context through cascading lookups: exact ID match in defaults, text frame
 * fallback, URL frame fallback, and user-defined pairings,
 * and finally a generic fallback. Creates entries initialized to single zero bytes according to the
 * resolved context, skipping iterator contexts. Frame header is created with all flags disabled.
 * 
 * @param id - Frame identifier array of ID3V2_FRAME_ID_MAX_SIZE
 * @param version - ID3v2 version for default context pairing lookup
 * @param userPairs - Optional user-defined ID-to-context mappings
 * 
 * @return Id3v2Frame* - Heap allocated frame with zero-initialized entries, or NULL if id is NULL. 
 * Caller must free with id3v2DestroyFrame()
 */
Id3v2Frame *id3v2CreateEmptyFrame(const char id[ID3V2_FRAME_ID_MAX_SIZE], uint8_t version, HashTable *userPairs) {
    if (id == NULL) {
        return NULL;
    }

    HashTable *pairs = NULL;
    List *context = NULL;
    List *entries = NULL;
    Id3v2ContentContext *cc = NULL;
    Id3v2FrameHeader *header = NULL;
    Id3v2Frame *f = NULL;

    pairs = id3v2CreateDefaultIdentifierContextPairings(version);

    // first pass
    context = hashTableRetrieve(pairs, id);

    // second pass
    if (context == NULL && id[0] == 'T') {
        context = hashTableRetrieve(pairs, "T");
    }

    // third pass
    if (context == NULL && id[0] == 'W') {
        context = hashTableRetrieve(pairs, "W");
    }

    // forth pass
    if (context == NULL) {
        context = hashTableRetrieve(userPairs, id);
    }

    // fifth pass generic
    if (context == NULL) {
        context = hashTableRetrieve(pairs, "?");
    }


    ListIter i = listCreateIterator(context);

    entries = listCreate(id3v2PrintContentEntry, id3v2DeleteContentEntry, id3v2CompareContentEntry,
                         id3v2CopyContentEntry);
    while ((cc = listIteratorNext(&i)) != NULL) {
        if (cc->type != iter_context) {
            Id3v2ContentEntry *ce = id3v2CreateContentEntry("\0", 1);
            listInsertBack(entries, (void *) ce);
        }
    }

    header = id3v2CreateFrameHeader((uint8_t *) id, false, false, false, false, 0, 0, 0);
    f = id3v2CreateFrame(header, listDeepCopy(context), entries);

    hashTableFree(pairs);
    return f;
}

/**
 * @brief Compares a frame's identifier with a provided ID string.
 * @details Performs byte-by-byte comparison of the frame's header ID against the provided 
 * ID array. Returns false if frame or frame header is NULL to ensure safe operation.
 * 
 * @param frame - Frame structure containing the ID to compare
 * @param id - ID string array of ID3V2_FRAME_ID_MAX_SIZE bytes to compare against
 * 
 * @return bool - true if IDs match exactly, false if they differ or if frame/header is NULL
 */
bool id3v2CompareFrameId(const Id3v2Frame *frame, const char id[ID3V2_FRAME_ID_MAX_SIZE]) {
    if (frame == NULL) {
        return false;
    }

    if (frame->header == NULL) {
        return false;
    }

    return memcmp(frame->header->id, id, ID3V2_FRAME_ID_MAX_SIZE) == 0 ? true : false;
}


/**
 * @brief Creates a list iterator for traversing frames in an ID3v2 tag.
 * @details Initializes an iterator for sequential access to frames within a tag structure.
 * Returns an empty iterator with NULL current pointer if the tag or its frames list is NULL,
 * allowing safe iteration in all cases.
 * 
 * @param tag - Tag structure containing the frames list to iterate over
 * 
 * @return ListIter - Iterator positioned at the start of the frames list, or empty iterator if tag/frames is NULL
 */
ListIter id3v2CreateFrameTraverser(Id3v2Tag *tag) {
    ListIter e;
    e.current = NULL;

    if (tag == NULL) {
        return e;
    }

    if (tag->frames == NULL) {
        return e;
    }

    return listCreateIterator(tag->frames);
}

/**
 * @brief Advances the iterator and returns the next frame in the list.
 * @details Wrapper around listIteratorNext that advances the iterator position and returns 
 * the current frame. Returns NULL when the end of the list is reached, indicating iteration 
 * is complete.
 * 
 * @param traverser - List iterator to advance
 * 
 * @return Id3v2Frame* - Pointer to the next frame in the list, or NULL if at the end
 */
Id3v2Frame *id3v2FrameTraverse(ListIter *traverser) {
    return (Id3v2Frame *) listIteratorNext(traverser);
}

/**
 * @brief Creates a list iterator for traversing content entries within a frame.
 * @details Initializes an iterator for sequential access to content entries within a frame structure.
 * Returns an empty iterator with NULL current pointer if the frame or its entries 
 * list is NULL, allowing safe iteration in all cases.
 * 
 * @param frame - Frame structure containing the entries list to iterate over
 * 
 * @return ListIter - Iterator positioned at the start of the entries list, or empty iterator if frame/entries is NULL
 */
ListIter id3v2CreateFrameEntryTraverser(Id3v2Frame *frame) {
    ListIter e;
    e.current = NULL;

    if (frame == NULL) {
        return e;
    }

    if (frame->entries == NULL) {
        return e;
    }

    return listCreateIterator(frame->entries);
}


/**
 * @brief Reads and returns a deep copy of the current entry's data, advancing the iterator.
 * @details Retrieves the content entry at the iterator's current position and creates an 
 * independently allocated copy of its data. Advances the iterator to the next entry. Returns 
 * NULL and sets dataSize to 0 if the traverser is NULL, the current entry is NULL, or the 
 * entry has zero size.
 * 
 * @param traverser - Iterator positioned at the entry to read
 * @param dataSize - Output parameter receiving the size of returned data in bytes, or 0 on failure
 * 
 * @return void* - Heap allocated copy of the entry data. Caller must free. NULL on failure
 */
void *id3v2ReadFrameEntry(ListIter *traverser, size_t *dataSize) {
    if (traverser == NULL) {
        *dataSize = 0;
        return NULL;
    }

    Id3v2ContentEntry *data = (Id3v2ContentEntry *) listIteratorNext(traverser);
    void *ret = NULL;


    if (data == NULL) {
        *dataSize = 0;
        return NULL;
    }

    if (!data->size) {
        *dataSize = 0;
        return NULL;
    }

    ret = malloc(data->size);
    memset(ret, 0, data->size);
    memcpy(ret, data->entry, data->size);

    *dataSize = data->size;
    return ret;
}


/**
 * @brief Reads a frame entry as a UTF-8 encoded string with escaped special characters, advancing the iterator.
 * @details Retrieves the content entry at the iterator's current position, automatically detects its encoding, 
 * and converts it to UTF-8. Strips UTF-8 BOM if present and escapes quotes and backslashes for JSON/C std 
 * compatibility. Advances the iterator to the next entry. Returns NULL and sets dataSize to 0 if the traverser 
 * is NULL, the current entry is NULL, memory allocation fails, or encoding conversion fails.
 * 
 * @param traverser - Iterator positioned at the entry to read and convert
 * @param dataSize - Output parameter receiving the final escaped string length in bytes, or 0 on failure
 * 
 * @return char* - Heap allocated UTF-8 string with escaped quotes and backslashes. Caller must free. NULL on failure
 */
char *id3v2ReadFrameEntryAsChar(ListIter *traverser, size_t *dataSize) {
    unsigned char *tmp = NULL;
    unsigned char *outString = NULL;
    unsigned char encoding = 0;
    char *escapedStr = NULL;
    bool convi = false;
    int utf8BomOffset = 0;
    size_t outLen = 0;
    size_t j = 0;

    tmp = (unsigned char *) id3v2ReadFrameEntry(traverser, dataSize);

    // Failed
    if (!tmp) {
        return NULL;
    }

    // detect utf16
    if (*dataSize > BYTE_BOM_SIZE) {
        if (byteHasBOM(tmp)) {
            if (tmp[0] == 0xff && tmp[1] == 0xfe) {
                encoding = BYTE_UTF16LE;
            } else {
                encoding = BYTE_UTF16BE;
            }
        }
    }

    // add some padding to tmp
    unsigned char *reallocPtr = realloc(tmp, *dataSize + (size_t) (BYTE_PADDING * 2));

    if (reallocPtr == NULL) {
        free(tmp);
        *dataSize = 0;
        return NULL;
    }

    tmp = reallocPtr;

    memset(tmp + *dataSize, 0, (size_t) (BYTE_PADDING * 2));
    outLen = *dataSize + (size_t) (BYTE_PADDING * 2);

    // detect utf8/ascii
    if (byteIsUtf8(tmp)) {
        encoding = BYTE_UTF8;
    }

    // detect latin1
    if (byteIsUtf8(tmp) && encoding == 0) {
        encoding = BYTE_ISO_8859_1;
    }

    // convert to UTF8
    convi = byteConvertTextFormat(tmp, encoding, *dataSize + (size_t) (BYTE_PADDING * 2), &outString, BYTE_UTF8,
                                  &outLen);

    if (!convi && outLen == 0) {
        free(tmp);
        *dataSize = 0;
        return NULL;
    }

    // data is already in utf8
    if (convi && outLen == 0) {
        outString = tmp;
    } else {
        *dataSize = outLen;
        free(tmp);
    }

    // check for UTF8 BOM
    if (*dataSize >= 3 && outString[0] == 0xEF && outString[1] == 0xBB && outString[2] == 0xBF) {
        utf8BomOffset = 3;
    }

    // escape quotes and backslashes
    escapedStr = malloc((2 * (*dataSize - utf8BomOffset)) + 1);
    j = 0;

    for (size_t i = 0; i < *dataSize - utf8BomOffset; i++) {
        if (outString[i + utf8BomOffset] == '"' || outString[i + utf8BomOffset] == '\\') {
            escapedStr[j++] = '\\';
            escapedStr[j++] = (char) outString[i + utf8BomOffset];
        } else {
            escapedStr[j++] = (char) outString[i + utf8BomOffset];
        }
    }

    escapedStr[j] = '\0';
    free(outString);

    // truncate
    size_t nullPos = 0;
    while (escapedStr[nullPos] != '\0') {
        nullPos++;
    }
    *dataSize = nullPos;
    escapedStr[nullPos] = '\0';

    if (*dataSize == 0) {
        *dataSize = 1;
    }

    return escapedStr;
}

/**
 * @brief Reads the first byte of the current entry as an 8-bit unsigned integer, advancing the iterator.
 * @details Retrieves the content entry at the iterator's current position and extracts the first byte. 
 * Advances the iterator to the next entry. Returns 0 if the traverser is NULL, the current entry is NULL, 
 * or the entry has zero size. The remaining bytes in multi-byte entries are discarded.
 * 
 * @param traverser - Iterator positioned at the entry to read
 * 
 * @return uint8_t - First byte of the entry data, or 0 on failure
 */
uint8_t id3v2ReadFrameEntryAsU8(ListIter *traverser) {
    uint8_t *tmp = NULL;
    uint8_t ret = 0;
    size_t dataSize = 0;

    tmp = (uint8_t *) id3v2ReadFrameEntry(traverser, &dataSize);

    // Failed
    if (!tmp) {
        return 0;
    }

    ret = tmp[0];
    free(tmp);

    return ret;
}

/**
 * @brief Reads the current entry as a 16-bit unsigned integer, advancing the iterator.
 * @details Retrieves the content entry at the iterator's current position and interprets 
 * it as a uint16_t. For entries with 2+ bytes, constructs the value from the first two 
 * bytes. For single-byte entries, zero-extends the byte to 16 bits. Advances the iterator 
 * to the next entry. Returns 0 if the traverser is NULL, the current entry is NULL, or the 
 * entry has zero size.
 * 
 * @param traverser - Iterator positioned at the entry to read
 * 
 * @return uint16_t - 16-bit unsigned integer value from entry data, or 0 on failure
 */
uint16_t id3v2ReadFrameEntryAsU16(ListIter *traverser) {
    unsigned char *tmp = NULL;
    uint16_t ret = 0;

    size_t dataSize = 0;

    tmp = (unsigned char *) id3v2ReadFrameEntry(traverser, &dataSize);

    // Failed
    if (!tmp) {
        return 0;
    }

    if (dataSize >= sizeof(uint16_t)) {
        ret = (uint16_t) tmp[1] << 8 | (uint16_t) tmp[0];
    } else if (dataSize == sizeof(uint8_t)) {
        ret = (uint16_t) tmp[0];
    }

    free(tmp);
    return ret;
}

/**
 * @brief Reads the current entry as a 32-bit unsigned integer, advancing the iterator.
 * @details Retrieves the content entry at the iterator's current position and interprets it as a uint32_t. 
 * Entries larger than 4 bytes are clamped to use only the first 4 bytes. Advances the iterator to the 
 * next entry. Returns 0 if the traverser is NULL 
 * or the current entry is NULL.
 * 
 * @param traverser - Iterator positioned at the entry to read
 * 
 * @return uint32_t - 32-bit unsigned integer value from entry data, or 0 on failure
 */
uint32_t id3v2ReadFrameEntryAsU32(ListIter *traverser) {
    unsigned char *tmp = NULL;
    uint32_t ret = 0;
    size_t dataSize = 0;

    tmp = (unsigned char *) id3v2ReadFrameEntry(traverser, &dataSize);

    if (!tmp) {
        return 0;
    }

    // clamp to max size
    if (dataSize > sizeof(uint32_t)) {
        dataSize = sizeof(uint32_t);
    }

    switch (dataSize) {
        case 1:
            ret = tmp[0];
            break;
        case 2:
            ret = tmp[1] | (tmp[0] << 8);
            break;
        case 3:
            ret = tmp[2] | (tmp[1] << 8) | (tmp[0] << 16);
            break;
        case 4:
            ret = tmp[3] | (tmp[2] << 8) | (tmp[1] << 16) | (tmp[0] << 24);
            break;
        default:
            break;
    }


    free(tmp);
    return ret;
}

/**
 * @brief Writes data to the entry at the iterator's current position, clamping size to context constraints.
 * @details Locates the entry referenced by the iterator within the frame's entries list, finds its corresponding 
 * context definition, and replaces the entry's data with a deep copy of the provided data. The written size is 
 * clamped to the context's min/max bounds to maintain frame structure integrity. The iterator position is not 
 * advanced. Returns false if any parameter is NULL, entrySize is 0, the iterator's current position is invalid, 
 * the entry cannot be located, or the corresponding context cannot be found.
 * 
 * @param frame - Frame containing the entry to modify
 * @param entries - Iterator positioned at the entry to write. Iterator position is not advanced
 * @param entrySize - Desired size of data to write in bytes. Will be clamped to context min/max
 * @param entry - Source data to copy into the entry
 * 
 * @return bool - true on successful write, false on failure
 */
bool id3v2WriteFrameEntry(Id3v2Frame *frame, ListIter *entries, size_t entrySize, const void *entry) {
    if (frame == NULL || entries == NULL || entrySize == 0 || entry == NULL) {
        return false;
    }

    if (frame->contexts == NULL || frame->entries == NULL || entries->current == NULL) {
        return false;
    }


    ListIter contextIter = listCreateIterator(frame->contexts);
    ListIter entriesIter = listCreateIterator(frame->entries);
    Id3v2ContentEntry *ce = NULL;
    Id3v2ContentContext *cc = NULL;
    size_t posce = 0;
    size_t poscc = 0;
    size_t newSize = 0;
    void *newData = 0;

    // locate the entries position in the frame
    while ((ce = (Id3v2ContentEntry *) listIteratorNext(&entriesIter)) != NULL) {
        const int comp = frame->entries->compareData((void *) ce, entries->current->data);

        if (comp == 0) {
            break;
        }

        posce++;
    }

    if (ce == NULL) {
        return false;
    }

    // locate the context for the entry
    while ((cc = (Id3v2ContentContext *) listIteratorNext(&contextIter)) != NULL) {
        if (cc->type == iter_context) {
            // in case an iter is the first context
            poscc = (poscc == 0) ? 0 : poscc - 1;
        }

        if (poscc == posce) {
            break;
        }

        poscc++;
    }

    if (cc == NULL) {
        return false;
    }

    newSize = entrySize;

    if (entrySize > cc->max) {
        newSize = cc->max;
    }

    if (entrySize < cc->min) {
        newSize = cc->min;
    }


    newData = malloc(newSize);
    memset(newData, 0, newSize);
    memcpy(newData, entry, newSize);

    free(((Id3v2ContentEntry *) entries->current->data)->entry);
    ((Id3v2ContentEntry *) entries->current->data)->entry = newData;
    ((Id3v2ContentEntry *) entries->current->data)->size = newSize;

    return true;
}


/**
 * @brief Inserts a frame at the end of a tag's frames list.
 * @details Appends the provided frame to the back of the tag's frames list. Validates that the tag 
 * structure and frame structure are complete before insertion. Does not create a copy of the frame; 
 * the tag takes ownership of the frame pointer. Returns false if the tag is NULL, the frame is NULL, 
 * the tag's frames list is NULL, or any of the frame's required components (contexts, entries, header) 
 * are NULL.
 * 
 * @param tag - Tag structure to receive the frame
 * @param frame - Frame to insert. Tag takes ownership of this pointer
 * 
 * @return bool - true on successful insertion, false on failure
 */
bool id3v2AttachFrameToTag(Id3v2Tag *tag, Id3v2Frame *frame) {
    if (tag == NULL || frame == NULL) {
        return false;
    }

    if (tag->frames == NULL || frame->contexts == NULL || frame->entries == NULL || frame->header == NULL) {
        return false;
    }

    return listInsertBack(tag->frames, (void *) frame) ? true : false;
}


/**
 * @brief Removes a frame from a tag's frames list and returns it to the caller.
 * @details Searches the tag's frames list for a matching frame, removes it from the list, and 
 * returns the frame pointer. Does not deallocate the frame; ownership transfers to the caller 
 * who must eventually free it with id3v2DestroyFrame(). Returns NULL if the tag is NULL, the 
 * frame is NULL, or the frame is not found in the tag's frames list.
 * 
 * @param tag - Tag structure containing the frame to remove
 * @param frame - Frame to locate and remove from the tag
 * 
 * @return Id3v2Frame* - Pointer to the removed frame or NULL on failure
 */
Id3v2Frame *id3v2DetachFrameFromTag(Id3v2Tag *tag, Id3v2Frame *frame) {
    if (tag == NULL || frame == NULL) {
        return NULL;
    }

    return listDeleteData(tag->frames, (void *) frame);
}


/**
 * @brief Serializes a frame header to binary format according to the specified ID3v2 version.
 * @details Converts a frame header structure into its binary representation following version-specific 
 * formats. v2.2 produces a 6-byte header. v2.3 and v2.4 produce 10+ byte headers with optional extended 
 * data fields. v2.4 uses syncsafe encoding for the frame size, extended data is conditionally appended 
 * when their values are non-zero. Returns NULL and sets outl to 0 if the header is NULL or version 
 * is invalid (greater than ID3V2_TAG_VERSION_4).
 * 
 * @param header - Frame header structure to serialize
 * @param version - ID3v2 version (ID3V2_TAG_VERSION_2, ID3V2_TAG_VERSION_3, or ID3V2_TAG_VERSION_4)
 * @param frameSize - Size of frame content in bytes (excludes header). Encoded as syncsafe in v2.4
 * @param outl - Output parameter receiving the serialized header size in bytes, or 0 on failure
 * 
 * @return uint8_t* - Heap allocated binary header data. Caller must free. NULL on failure
 */
uint8_t *id3v2FrameHeaderSerialize(Id3v2FrameHeader *header, uint8_t version, uint32_t frameSize, size_t *outl) {
    unsigned char *tmp = NULL;
    ByteStream *stream = NULL;
    uint8_t *out = NULL;

    if (header == NULL || version > ID3V2_TAG_VERSION_4) {
        *outl = 0;
        return NULL;
    }

    switch (version) {
        case ID3V2_TAG_VERSION_2:

            // size of header for this version is always 6 bytes.
            stream = byteStreamCreate(NULL, 6);

            byteStreamWrite(stream, header->id, ID3V2_FRAME_ID_MAX_SIZE - 1);

            tmp = u32tob(frameSize);
            byteStreamWrite(stream, tmp + 1, 3);
            free(tmp);

            break;
        case ID3V2_TAG_VERSION_3:

            // base header is always 10 bytes.
            stream = byteStreamCreate(NULL, 10);

            byteStreamWrite(stream, header->id, ID3V2_FRAME_ID_MAX_SIZE);

            tmp = u32tob(frameSize);
            byteStreamWrite(stream, tmp, 4);
            free(tmp);

            byteStreamWriteBit(stream, header->tagAlterPreservation, 7);
            byteStreamWriteBit(stream, header->fileAlterPreservation, 6);
            byteStreamWriteBit(stream, header->readOnly, 5);
            byteStreamSeek(stream, 1, SEEK_CUR);

            byteStreamWriteBit(stream, (header->decompressionSize > 0) ? true : false, 7);
            byteStreamWriteBit(stream, (header->encryptionSymbol > 0) ? true : false, 6);
            byteStreamWriteBit(stream, (header->groupSymbol > 0) ? true : false, 5);
            byteStreamSeek(stream, 1, SEEK_CUR);

            if (header->decompressionSize > 0) {
                byteStreamResize(stream, stream->bufferSize + 4);
                tmp = u32tob(header->decompressionSize);
                byteStreamWrite(stream, tmp, 4);
                free(tmp);
            }

            if (header->encryptionSymbol > 0) {
                byteStreamResize(stream, stream->bufferSize + 1);
                byteStreamWrite(stream, &header->encryptionSymbol, 1);
            }

            if (header->groupSymbol > 0) {
                byteStreamResize(stream, stream->bufferSize + 1);
                byteStreamWrite(stream, &header->groupSymbol, 1);
            }

            break;
        case ID3V2_TAG_VERSION_4:

            // base header is always 10 bytes.
            stream = byteStreamCreate(NULL, 10);

            byteStreamWrite(stream, header->id, ID3V2_FRAME_ID_MAX_SIZE);

            tmp = u32tob(byteSyncintEncode(frameSize));
            byteStreamWrite(stream, tmp, 4);
            free(tmp);

            byteStreamWriteBit(stream, header->tagAlterPreservation, 6);
            byteStreamWriteBit(stream, header->fileAlterPreservation, 5);
            byteStreamWriteBit(stream, header->readOnly, 4);
            byteStreamSeek(stream, 1, SEEK_CUR);

            byteStreamWriteBit(stream, (header->groupSymbol > 0) ? true : false, 6);
            byteStreamWriteBit(stream, (header->decompressionSize > 0) ? true : false, 3);
            byteStreamWriteBit(stream, (header->encryptionSymbol > 0) ? true : false, 2);
            byteStreamWriteBit(stream, (header->unsynchronisation > 0) ? true : false, 1);
            byteStreamWriteBit(stream, (header->decompressionSize > 0) ? true : false, 0);
            byteStreamSeek(stream, 1, SEEK_CUR);

            if (header->groupSymbol > 0) {
                byteStreamResize(stream, stream->bufferSize + 1);
                byteStreamWrite(stream, &header->groupSymbol, 1);
            }

            if (header->encryptionSymbol > 0) {
                byteStreamResize(stream, stream->bufferSize + 1);
                byteStreamWrite(stream, &header->encryptionSymbol, 1);
            }

            if (header->decompressionSize > 0) {
                byteStreamResize(stream, stream->bufferSize + 4);
                tmp = u32tob(header->decompressionSize);
                byteStreamWrite(stream, tmp, 4);
                free(tmp);
            }

            break;
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
 * @brief Converts a frame header structure to its JSON representation based on ID3v2 version.
 * @details Serializes frame header metadata into a JSON string with version-specific fields. v2.2 includes 
 * only the 3-character frame ID. v2.3 adds preservation flags, compression, encryption, and grouping metadata. 
 * v2.4 additionally includes the unsynchronisation flag. Returns an empty JSON object "{}" if the header is 
 * NULL or the version is unsupported.
 * 
 * Example outputs:
 * - v2.2: {"id":"TIT"}
 * - v2.3: {"id":"TIT2","tagAlterPreservation":false,"fileAlterPreservation":false,"readOnly":false,"decompressionSize":0,"encryptionSymbol":0,"groupSymbol":0}
 * - v2.4: {"id":"TIT2","tagAlterPreservation":false,"fileAlterPreservation":false,"readOnly":false,"unsynchronisation":false,"decompressionSize":0,"encryptionSymbol":0,"groupSymbol":0}
 * 
 * @param header - Frame header structure to convert to JSON
 * @param version - ID3v2 version (ID3V2_TAG_VERSION_2, ID3V2_TAG_VERSION_3, or ID3V2_TAG_VERSION_4)
 * 
 * @return char* - Heap allocated JSON string. Caller must free. Returns "{}" if header is NULL or version is unsupported
 */
char *id3v2FrameHeaderToJSON(const Id3v2FrameHeader *header, uint8_t version) {
    char *json = NULL;
    size_t memCount = 3;

    if (header == NULL) {
        json = calloc(memCount, sizeof(char));
        memcpy(json, "{}\0", memCount);
        return json;
    }

    switch (version) {
        case ID3V2_TAG_VERSION_2:

            memCount += snprintf(NULL, 0,
                                 "{\"id\":\"%c%c%c\"}",
                                 header->id[0],
                                 header->id[1],
                                 header->id[2]);

            json = calloc(memCount + 1, sizeof(char));

            (void) snprintf(json, memCount,
                            "{\"id\":\"%c%c%c\"}",
                            header->id[0],
                            header->id[1],
                            header->id[2]);

            break;
        case ID3V2_TAG_VERSION_3:

            memCount += snprintf(NULL, 0,
                                 "{\"id\":\"%c%c%c%c\",\"tagAlterPreservation\":%s,\"fileAlterPreservation\":%s,\"readOnly\":%s,\"decompressionSize\":%"
                                 PRId32",\"encryptionSymbol\":%d,\"groupSymbol\":%d}",
                                 header->id[0],
                                 header->id[1],
                                 header->id[2],
                                 header->id[3],
                                 ((header->tagAlterPreservation == true) ? "true" : "false"),
                                 ((header->fileAlterPreservation == true) ? "true" : "false"),
                                 ((header->readOnly == true) ? "true" : "false"),
                                 header->decompressionSize,
                                 header->encryptionSymbol,
                                 header->groupSymbol);

            json = calloc(memCount + 1, sizeof(char));

            (void) snprintf(json, memCount,
                            "{\"id\":\"%c%c%c%c\",\"tagAlterPreservation\":%s,\"fileAlterPreservation\":%s,\"readOnly\":%s,\"decompressionSize\":%"
                            PRId32",\"encryptionSymbol\":%d,\"groupSymbol\":%d}",
                            header->id[0],
                            header->id[1],
                            header->id[2],
                            header->id[3],
                            ((header->tagAlterPreservation == true) ? "true" : "false"),
                            ((header->fileAlterPreservation == true) ? "true" : "false"),
                            ((header->readOnly == true) ? "true" : "false"),
                            header->decompressionSize,
                            header->encryptionSymbol,
                            header->groupSymbol);

            break;
        case ID3V2_TAG_VERSION_4:

            memCount += snprintf(NULL, 0,
                                 "{\"id\":\"%c%c%c%c\",\"tagAlterPreservation\":%s,\"fileAlterPreservation\":%s,\"readOnly\":%s,\"unsynchronisation\":%s,\"decompressionSize\":%"
                                 PRId32",\"encryptionSymbol\":%d,\"groupSymbol\":%d}",
                                 header->id[0],
                                 header->id[1],
                                 header->id[2],
                                 header->id[3],
                                 ((header->tagAlterPreservation == true) ? "true" : "false"),
                                 ((header->fileAlterPreservation == true) ? "true" : "false"),
                                 ((header->readOnly == true) ? "true" : "false"),
                                 ((header->unsynchronisation == true) ? "true" : "false"),
                                 header->decompressionSize,
                                 header->encryptionSymbol,
                                 header->groupSymbol);

            json = calloc(memCount + 1, sizeof(char));

            (void) snprintf(json, memCount,
                            "{\"id\":\"%c%c%c%c\",\"tagAlterPreservation\":%s,\"fileAlterPreservation\":%s,\"readOnly\":%s,\"unsynchronisation\":%s,\"decompressionSize\":%"
                            PRId32",\"encryptionSymbol\":%d,\"groupSymbol\":%d}",
                            header->id[0],
                            header->id[1],
                            header->id[2],
                            header->id[3],
                            ((header->tagAlterPreservation == true) ? "true" : "false"),
                            ((header->fileAlterPreservation == true) ? "true" : "false"),
                            ((header->readOnly == true) ? "true" : "false"),
                            ((header->unsynchronisation == true) ? "true" : "false"),
                            header->decompressionSize,
                            header->encryptionSymbol,
                            header->groupSymbol);

            break;

        // no support
        default:
            json = calloc(memCount, sizeof(char));
            memcpy(json, "{}\0", memCount);
            break;
    }

    return json;
}

/**
 * @brief Serializes a complete ID3v2 frame to binary format according to the specified version.
 * @details Converts a frame structure into its binary representation by serializing the header and 
 * processing each content entry according to its context type. Handles encoding conversions (UTF-8, 
 * UTF-16LE/BE, Latin-1), null terminator insertion, bit-packing, and size adjustments. The serialization 
 * process iterates through frame contexts and applies type-specific transformations: encoded strings are 
 * converted to their target encoding with BOM prepending where required, binary/numeric data is written 
 * directly, bit contexts are packed into compact byte representations, and adjustment contexts modify 
 * data sizes dynamically. Returns NULL and sets outl to 0 if the frame is NULL, version is invalid 
 * (greater than ID3V2_TAG_VERSION_4), or memory allocation fails during processing.
 * 
 * @param frame - Frame structure containing header, contexts, and entries to serialize
 * @param version - ID3v2 version (ID3V2_TAG_VERSION_2, ID3V2_TAG_VERSION_3, or ID3V2_TAG_VERSION_4)
 * @param outl - Output parameter receiving the total serialized frame size in bytes (header + content), or 0 on failure
 * 
 * @return uint8_t* - Heap allocated binary frame data ready for writing to file. Caller must free. NULL on failure
 */
uint8_t *id3v2FrameSerialize(Id3v2Frame *frame, uint8_t version, size_t *outl) {
    ByteStream *stream = NULL;
    Id3v2ContentContext *cc = NULL;

    if (frame == NULL || version > ID3V2_TAG_VERSION_4) {
        *outl = 0;
        return NULL;
    }

    ListIter context = listCreateIterator(frame->contexts);
    ListIter trav = id3v2CreateFrameEntryTraverser(frame);
    ListIter iterStorage;
    size_t readSize = 0;
    size_t contentSize = 0;
    size_t currIterations = 0;
    size_t headerSize = 0;
    uint8_t *header = NULL;
    uint8_t *out = NULL;
    unsigned char *tmp = NULL;
    bool exit = false;
    bool bitFlag = false;

    // the frame size will be updated later as it cannot be calculated
    // before processing frame entries
    header = id3v2FrameHeaderSerialize(frame->header, version, 0, &headerSize);
    stream = byteStreamCreate(header, headerSize);
    free(header);

    byteStreamSeek(stream, 0, SEEK_END);

    while ((cc = listIteratorNext(&context)) != NULL) {
        switch (cc->type) {
            // encoding will always be enforced
            case encodedString_context: {
                ListIter contentContextIter = listCreateIterator(frame->contexts);
                ListIter contentEntryIter = listCreateIterator(frame->entries);
                size_t poscc = 0;
                size_t posce = 0;
                size_t utf8Len = 0;
                uint8_t encoding = 0;
                void *iterNext = NULL;


                // hunt down "encoding" key
                while ((iterNext = listIteratorNext(&contentContextIter)) != NULL) {
                    if (((Id3v2ContentContext *) iterNext)->type == iter_context) {
                        poscc--;
                    }

                    if (((Id3v2ContentContext *) iterNext)->key == id3v2djb2("encoding")) {
                        break;
                    }

                    poscc++;
                }

                // hunt down encoding value
                while ((iterNext = listIteratorNext(&contentEntryIter)) != NULL) {
                    if (poscc == posce) {
                        encoding = ((uint8_t *) ((Id3v2ContentEntry *) iterNext)->entry)[0];
                        break;
                    }

                    posce++;
                }

                // enforce encoding as utf8
                tmp = (unsigned char *) id3v2ReadFrameEntryAsChar(&trav, &utf8Len);

                if (tmp == NULL || utf8Len == 0) {
                    exit = true;
                    break;
                }

                unsigned char *outStr = NULL;
                size_t outLen = 0;

                // non-empty strings
                if (utf8Len >= 1 && tmp[0] != 0) {
                    bool convi = false;
                    convi = byteConvertTextFormat(tmp, BYTE_UTF8, utf8Len, &outStr, encoding, &outLen);

                    if (convi == false && outLen == 0) {
                        free(tmp);
                        exit = true;
                        break;
                    }


                    // data is already in utf8
                    if (convi && outLen == 0) {
                        outStr = tmp;
                        outLen = utf8Len;
                    } else {
                        free(tmp);
                    }

                    // prepend BOM
                    if (encoding == BYTE_UTF16BE || encoding == BYTE_UTF16LE) {
                        bytePrependBOM(encoding, &outStr, &outLen);
                    }
                } else {
                    free(tmp);
                }

                // append null spacer if there are more entries in the list
                if (trav.current != NULL) {
                    switch (encoding) {
                        case BYTE_ISO_8859_1:
                        case BYTE_ASCII:
                        case BYTE_UTF8: {
                            unsigned char *reallocPtr = realloc(outStr, outLen + 1);
                            if (reallocPtr == NULL) {
                                free(outStr);
                                byteStreamDestroy(stream);
                                *outl = 0;
                                return NULL;
                            }
                            outStr = reallocPtr;
                            memset(outStr + outLen, 0, 1);
                            outLen++;
                            break;
                        }
                        case BYTE_UTF16BE:
                        case BYTE_UTF16LE: {
                            unsigned char *reallocPtr = realloc(outStr, outLen + 2);
                            if (reallocPtr == NULL) {
                                free(outStr);
                                byteStreamDestroy(stream);
                                *outl = 0;
                                return NULL;
                            }
                            outStr = reallocPtr;
                            memset(outStr + outLen, 0, 2);
                            outLen += 2;
                            break;
                        }
                        default:
                            break;
                    }
                }

                byteStreamResize(stream, stream->bufferSize + outLen);
                byteStreamWrite(stream, outStr, outLen);
                contentSize += outLen;
                free(outStr);

                break;
            }

            // written the same way with no spacer I repeat no spacer over
            case numeric_context:
            case noEncoding_context:
            case binary_context:
            case precision_context:
                tmp = id3v2ReadFrameEntry(&trav, &readSize);

                if (tmp == NULL) {
                    exit = true;
                    break;
                }

                byteStreamResize(stream, stream->bufferSize + readSize);
                byteStreamWrite(stream, tmp, readSize);
                free(tmp);
                contentSize += readSize;
                break;

            // latin1 will be enforced
            case latin1Encoding_context: {
                bool convi = false;
                unsigned char *outStr = NULL;
                size_t outLen = 0;
                size_t utf8len = 0;

                tmp = (unsigned char *) id3v2ReadFrameEntryAsChar(&trav, &utf8len);

                if (tmp == NULL) {
                    exit = true;
                    break;
                }

                // ensure latin1
                convi = byteConvertTextFormat(tmp, BYTE_UTF8, utf8len, &outStr, BYTE_ISO_8859_1, &outLen);

                if (convi == false && outLen == 0) {
                    free(tmp);
                    break;
                }

                // data is already in latin1
                if (convi && outLen == 0) {
                    outStr = tmp;
                    outLen = utf8len;
                } else {
                    free(tmp);
                }

                // add spacer
                if (trav.current != NULL) {
                    unsigned char *reallocPtr = realloc(outStr, outLen + 1);
                    if (reallocPtr == NULL) {
                        free(outStr);
                        byteStreamDestroy(stream);
                        *outl = 0;
                        return NULL;
                    }
                    outStr = reallocPtr;
                    memset(outStr + outLen, 0, 1);
                    outLen++;
                }

                byteStreamResize(stream, stream->bufferSize + outLen);
                byteStreamWrite(stream, outStr, outLen);

                free(outStr);
                contentSize += outLen;
                break;
            }

            case iter_context: {
                // create a new iter
                if (currIterations == 0) {
                    iterStorage = context;

                    context = listCreateIterator(frame->contexts);

                    for (size_t i = 0; i < cc->min; i++) {
                        listIteratorNext(&context);
                    }
                }

                // iter
                if (currIterations != cc->max && currIterations != 0) {
                    context = listCreateIterator(frame->contexts);

                    for (size_t i = 0; i < cc->min; i++) {
                        listIteratorNext(&context);
                    }
                }

                // reset
                if (currIterations >= cc->max) {
                    context = iterStorage;

                    for (size_t i = 0; i < currIterations; i++) {
                        listIteratorNext(&context);
                    }

                    currIterations = 0;
                }

                currIterations++;

                /**
                 * This will go on forever until a failure condition is met by a previous context
                 * i.e latin1_context detects null
                 */
            }
            break;
            // wildly long and probably inefficient, but it works for now and is the best I can do with my current knowledge
            case bit_context: {
                // there is another bit context next
                if (listIteratorHasNext(context)) {
                    if (((Id3v2ContentContext *) context.current->data)->type != bit_context) {
                        bitFlag = false;
                    } else {
                        bitFlag = true;
                    }
                }


                // at least 1 or more bit contexts in a row
                if (bitFlag == true) {
                    unsigned char *bitBuff = NULL;
                    unsigned char **byteDataArr = NULL;
                    size_t *byteDataSizeArr = NULL;
                    size_t *nbits = NULL;

                    size_t arrSize = 0;
                    size_t totalBits = 0;
                    size_t totalBytes = 0;
                    size_t bitBuffSize = 0;

                    // copy values
                    while (true) {
                        tmp = id3v2ReadFrameEntry(&trav, &readSize);

                        if (tmp == NULL) {
                            exit = true;
                            break;
                        }

                        if (byteDataSizeArr == NULL) {
                            byteDataSizeArr = malloc(sizeof(size_t));
                            byteDataSizeArr[0] = readSize;

                            nbits = malloc(sizeof(size_t));
                            nbits[0] = cc->max;

                            byteDataArr = (unsigned char **) malloc(sizeof(unsigned char *));
                            byteDataArr[0] = malloc(readSize);

                            for (size_t i = 0; i < readSize; i++) {
                                byteDataArr[0][i] = tmp[i];
                            }

                            arrSize++;
                        } else {
                            arrSize++;
                            byteDataSizeArr = realloc(byteDataSizeArr, arrSize * sizeof(size_t));
                            byteDataSizeArr[arrSize - 1] = readSize;

                            nbits = realloc(nbits, arrSize * sizeof(size_t));
                            nbits[arrSize - 1] = cc->max;

                            byteDataArr = realloc(byteDataArr, arrSize * sizeof(unsigned char *));
                            byteDataArr[arrSize - 1] = malloc(readSize);

                            for (size_t i = 0; i < readSize; i++) {
                                byteDataArr[arrSize - 1][i] = tmp[i];
                            }
                        }

                        totalBits += cc->max;

                        free(tmp);


                        if (listIteratorHasNext(context)) {
                            if (((Id3v2ContentContext *) context.current->data)->type != bit_context) {
                                break;

                                // seek to the next context
                            } else {
                                cc = listIteratorNext(&context);
                            }
                        } else {
                            break;
                        }
                    }

                    totalBytes = ((totalBits / CHAR_BIT) % 2) ? (totalBits / CHAR_BIT) + 1 : totalBits / CHAR_BIT;
                    // ? odd : even

                    bitBuff = malloc(totalBytes);
                    memset(bitBuff, 0, totalBytes);
                    bitBuffSize = totalBytes;

                    // reverse the byte data
                    for (size_t i = 0; i < arrSize; i++) {
                        size_t dataSize = byteDataSizeArr[i];
                        size_t halfSize = dataSize / 2;
                        for (size_t j = 0; j < halfSize; j++) {
                            unsigned char temp = byteDataArr[i][j];
                            byteDataArr[i][j] = byteDataArr[i][dataSize - j - 1];
                            byteDataArr[i][dataSize - j - 1] = temp;
                        }
                    }

                    int step = 0;
                    int offset = 0;
                    int bitIndex = 0;
                    for (size_t i = totalBytes; i > 0; i--) {
                        if (step >= arrSize || totalBytes == 0) {
                            break;
                        }

                        size_t nBit = nbits[step];
                        size_t nBytes = byteDataSizeArr[step];
                        unsigned char *data = byteDataArr[step];
                        int counter = 0;

                        while (nBit > 0) {
                            if (counter == nBytes) {
                                break;
                            }

                            int j = 0;
                            for (j = 0; j < CHAR_BIT; j++) {
                                if (nBit == 0) {
                                    break;
                                }

                                // switch to the next byte
                                if (j + offset >= CHAR_BIT) {
                                    offset = 0;
                                    totalBytes--;
                                    bitIndex = 0;
                                }

                                bitBuff[totalBytes - 1] = setBit(bitBuff[totalBytes - 1], bitIndex,
                                                                 readBit(data[counter], j) > 0 ? true : false);
                                bitIndex++;

                                nBit--;
                            }

                            if (j < CHAR_BIT) {
                                offset = j;
                            } else {
                                offset = 0;
                            }
                            counter++;
                        }

                        step++;
                    }

                    byteStreamResize(stream, stream->bufferSize + bitBuffSize);
                    byteStreamWrite(stream, bitBuff, bitBuffSize);
                    contentSize += bitBuffSize;

                    for (size_t i = 0; i < arrSize; i++) {
                        free(byteDataArr[i]);
                    }

                    free(bitBuff);
                    free(nbits);
                    free(byteDataArr);
                    free(byteDataSizeArr);

                    bitFlag = false;

                    // read a single and only bit context
                } else {
                    int totalBytesNeeded = (cc->max / CHAR_BIT) + 1;
                    int nBit = CHAR_BIT - 1;

                    tmp = id3v2ReadFrameEntry(&trav, &readSize);

                    if (tmp == NULL) {
                        exit = true;
                        break;
                    }

                    while (totalBytesNeeded > 0) {
                        byteStreamResize(stream, stream->bufferSize + 1);

                        while (nBit >= 0) {
                            byteStreamWriteBit(stream, (readBit(tmp[readSize - 1], nBit) > 0) ? true : false, nBit);
                            nBit--;
                        }

                        byteStreamSeek(stream, 1, SEEK_CUR);
                        totalBytesNeeded--;
                    }

                    contentSize += readSize;
                    free(tmp);
                }

                break;
            }

            case adjustment_context: {
                ListIter contentContextIter = listCreateIterator(frame->contexts);
                ListIter contentEntryIter = listCreateIterator(frame->entries);
                size_t poscc = 0;
                size_t posce = 0;
                uint32_t rSize = 0;
                void *iterNext = NULL;


                // hunt down "adjustment" key
                while ((iterNext = listIteratorNext(&contentContextIter)) != NULL) {
                    if (((Id3v2ContentContext *) iterNext)->type == iter_context) {
                        poscc--;
                    }

                    if (((Id3v2ContentContext *) iterNext)->key == id3v2djb2("adjustment")) {
                        break;
                    }

                    poscc++;
                }

                // hunt down adjustment value
                while ((iterNext = listIteratorNext(&contentEntryIter)) != NULL) {
                    if (poscc == posce) {
                        rSize = btou32((uint8_t *) ((Id3v2ContentEntry *) iterNext)->entry,
                                       (int) ((Id3v2ContentEntry *) iterNext)->size);
                        break;
                    }

                    posce++;
                }

                tmp = id3v2ReadFrameEntry(&trav, &readSize);

                if (tmp == NULL) {
                    exit = true;
                    break;
                }

                byteStreamResize(stream, stream->bufferSize + rSize);
                byteStreamWrite(stream, tmp, rSize);

                contentSize += rSize;
                free(tmp);
                break;
            }
            case unknown_context:
            default:
                exit = true;
                break;
        }

        if (exit == true) {
            break;
        }
    }


    // write in the frame size
    switch (version) {
        case ID3V2_TAG_VERSION_2:
            tmp = u32tob(contentSize);
            byteStreamSeek(stream, ID3V2_FRAME_ID_MAX_SIZE - 1, SEEK_SET);
            byteStreamWrite(stream, tmp + 1, ID3V2_FRAME_ID_MAX_SIZE - 1);
            free(tmp);

            break;
        case ID3V2_TAG_VERSION_3:
            tmp = u32tob(contentSize);
            byteStreamSeek(stream, ID3V2_FRAME_ID_MAX_SIZE, SEEK_SET);
            byteStreamWrite(stream, tmp, ID3V2_FRAME_ID_MAX_SIZE);
            free(tmp);

            break;
        case ID3V2_TAG_VERSION_4:
            tmp = u32tob(byteSyncintEncode(contentSize));
            byteStreamSeek(stream, ID3V2_FRAME_ID_MAX_SIZE, SEEK_SET);
            byteStreamWrite(stream, tmp, ID3V2_FRAME_ID_MAX_SIZE);
            free(tmp);

            break;
        default:
            break;
    }

    byteStreamRewind(stream);
    *outl = stream->bufferSize;
    out = calloc(stream->bufferSize, sizeof(uint8_t));
    byteStreamRead(stream, out, stream->bufferSize);
    byteStreamDestroy(stream);

    return out;
}

/**
 * @brief Converts a complete ID3v2 frame structure to its JSON representation.
 * @details Serializes frame metadata and content entries into a JSON object with header information 
 * and an array of content values. String entries are converted to UTF-8 with escaped special characters, 
 * binary data is base64-encoded, numeric values are represented as strings, and floating-point values 
 * are formatted with decimal precision. Returns an empty JSON object "{}" if the frame is NULL or 
 * version is invalid (greater than ID3V2_TAG_VERSION_4).
 * 
 * Example output for a TIT2 (title) frame in ID3v2.4:
 * ```json
 * {
 *   "header": {
 *     "id":"TIT2",
 *     "tagAlterPreservation":false,
 *     "fileAlterPreservation":false,
 *     "readOnly":false,
 *     "unsynchronisation":false,
 *     "decompressionSize":0,
 *     "encryptionSymbol":0,
 *     "groupSymbol":0
 *   },
 *   "content": [
 *     {"value":"3","size":1},
 *     {"value":"Beetlebum","size":9}
 *   ]
 * }
 * ```
 * 
 * @param frame - Frame structure to convert to JSON
 * @param version - ID3v2 version (ID3V2_TAG_VERSION_2, ID3V2_TAG_VERSION_3, or ID3V2_TAG_VERSION_4)
 * 
 * @return char* - Heap allocated JSON string. Caller must free. Returns "{}" if frame is NULL or version is invalid
 */
char *id3v2FrameToJSON(Id3v2Frame *frame, uint8_t version) {
    char *json = NULL;
    char *headerJson = NULL;
    char **contentJson = NULL;
    char *concatenatedString = NULL;
    size_t contentJsonSize = 0;
    size_t memCount = 3;
    size_t concatenatedStringLength = 0;
    size_t currIterations = 0;

    ListIter trav = id3v2CreateFrameEntryTraverser(frame);
    ListIter context = listCreateIterator(frame->contexts);
    ListIter iterStorage;


    Id3v2ContentContext *cc = NULL;
    bool exit = false;

    unsigned char *tmp = NULL;

    if (frame == NULL || version > ID3V2_TAG_VERSION_4) {
        json = calloc(memCount, sizeof(char));
        memcpy(json, "{}\0", memCount);
        return json;
    }

    while ((cc = (Id3v2ContentContext *) listIteratorNext(&context)) != NULL) {
        switch (cc->type) {
            // treated as base64
            case noEncoding_context:
            case bit_context:
            case binary_context: {
                size_t readSize = 0;
                size_t contentMemCount = 3;
                char *b64 = NULL;

                tmp = id3v2ReadFrameEntry(&trav, &readSize);

                if (tmp == NULL || readSize == 0) {
                    exit = true;
                    break;
                }


                b64 = internal_base64Encode(tmp, readSize);

                contentMemCount += snprintf(NULL, 0,
                                            "{\"value\":\"%s\",\"size\":%zu}",
                                            b64,
                                            strlen(b64));

                contentJsonSize++;
                if (contentJson == NULL) {
                    contentJson = (char **) calloc(contentJsonSize, sizeof(char *));
                    contentJson[contentJsonSize - 1] = calloc(contentMemCount + 1, sizeof(char));

                    (void) snprintf(contentJson[contentJsonSize - 1], contentMemCount,
                                    "{\"value\":\"%s\",\"size\":%zu}",
                                    b64,
                                    readSize);
                } else {
                    char **reallocContentJson = (char **) realloc((void *) contentJson,
                                                                  (contentJsonSize) * sizeof(char *));

                    if (reallocContentJson == NULL) {
                        for (size_t i = 0; i < contentJsonSize - 1; i++) {
                            free(contentJson[i]);
                        }
                        free((void *) contentJson);
                        free(b64);
                        free(tmp);
                        return NULL;
                    }

                    contentJson = reallocContentJson;
                    contentJson[contentJsonSize - 1] = calloc(contentMemCount + 1, sizeof(char));
                    (void) snprintf(contentJson[contentJsonSize - 1], contentMemCount,
                                    "{\"value\":\"%s\",\"size\":%zu}",
                                    b64,
                                    readSize);
                }

                free(tmp);
                free(b64);
            }
            break;


            // will always be treated as utf8 when in json
            case encodedString_context:
            case latin1Encoding_context: {
                size_t readSize = 0;
                size_t contentMemCount = 3;

                tmp = (unsigned char *) id3v2ReadFrameEntryAsChar(&trav, &readSize);

                if (tmp == NULL || readSize == 0) {
                    exit = true;
                    break;
                }


                contentMemCount += snprintf(NULL, 0,
                                            "{\"value\":\"%s\",\"size\":%zu}",
                                            (char *) tmp,
                                            readSize);

                contentJsonSize++;
                if (contentJson == NULL) {
                    contentJson = (char **) calloc(contentJsonSize, sizeof(char *));
                    contentJson[contentJsonSize - 1] = calloc(contentMemCount + 1, sizeof(char));

                    (void) snprintf(contentJson[contentJsonSize - 1], contentMemCount,
                                    "{\"value\":\"%s\",\"size\":%zu}",
                                    (char *) tmp,
                                    readSize);
                } else {
                    char **reallocContentJson = (char **) realloc((void *) contentJson,
                                                                  (contentJsonSize) * sizeof(char *));

                    if (reallocContentJson == NULL) {
                        for (size_t i = 0; i < contentJsonSize - 1; i++) {
                            free(contentJson[i]);
                        }
                        free((void *) contentJson);
                        free(tmp);
                        return NULL;
                    }

                    contentJson = reallocContentJson;
                    contentJson[contentJsonSize - 1] = calloc(contentMemCount + 1, sizeof(char));
                    (void) snprintf(contentJson[contentJsonSize - 1], contentMemCount,
                                    "{\"value\":\"%s\",\"size\":%zu}",
                                    (char *) tmp,
                                    readSize);
                }

                free(tmp);
            }
            break;

            case numeric_context: {
                size_t readSize = 0;
                size_t contentMemCount = 3;
                size_t num = 0;

                tmp = id3v2ReadFrameEntry(&trav, &readSize);

                if (tmp == NULL || readSize == 0) {
                    exit = true;
                    break;
                }

                num = btost(tmp, (int) readSize);
                free(tmp);

                contentMemCount += snprintf(NULL, 0,
                                            "{\"value\":\"%zu\",\"size\":%zu}",
                                            num,
                                            readSize);

                contentJsonSize++;
                if (contentJson == NULL) {
                    contentJson = (char **) calloc(contentJsonSize, sizeof(char *));
                    contentJson[contentJsonSize - 1] = calloc(contentMemCount + 1, sizeof(char));

                    (void) snprintf(contentJson[contentJsonSize - 1], contentMemCount,
                                    "{\"value\":\"%zu\",\"size\":%zu}",
                                    num,
                                    readSize);
                } else {
                    char **reallocContentJson = (char **) realloc((void *) contentJson,
                                                                  (contentJsonSize) * sizeof(char *));

                    if (reallocContentJson == NULL) {
                        for (size_t i = 0; i < contentJsonSize - 1; i++) {
                            free(contentJson[i]);
                        }
                        free((void *) contentJson);
                        return NULL;
                    }

                    contentJson = reallocContentJson;
                    contentJson[contentJsonSize - 1] = calloc(contentMemCount + 1, sizeof(char));
                    (void) snprintf(contentJson[contentJsonSize - 1], contentMemCount,
                                    "{\"value\":\"%zu\",\"size\":%zu}",
                                    num,
                                    readSize);
                }
            }
            break;

            case precision_context: {
                size_t readSize = 0;
                size_t contentMemCount = 0;
                float value = 0;

                tmp = id3v2ReadFrameEntry(&trav, &readSize);

                if (tmp == NULL || readSize == 0) {
                    exit = true;
                    break;
                }


                memcpy(&value, tmp, sizeof(value));
                free(tmp);

                contentMemCount = snprintf(NULL, 0,
                                           "{\"value\":\"%f\",\"size\":%zu}",
                                           value,
                                           readSize);

                contentJsonSize++;
                if (contentJson == NULL) {
                    contentJson = (char **) calloc(contentJsonSize, sizeof(char *));
                    contentJson[contentJsonSize - 1] = calloc(contentMemCount + 1, sizeof(char));

                    (void) snprintf(contentJson[contentJsonSize - 1], contentMemCount,
                                    "{\"value\":\"%f\",\"size\":%zu}",
                                    value,
                                    readSize);
                } else {
                    char **reallocContentJson = (char **) realloc((void *) contentJson,
                                                                  (contentJsonSize) * sizeof(char *));

                    if (reallocContentJson == NULL) {
                        for (size_t i = 0; i < contentJsonSize - 1; i++) {
                            free(contentJson[i]);
                        }
                        free((void *) contentJson);
                        return NULL;
                    }
                    contentJson = reallocContentJson;
                    contentJson[contentJsonSize - 1] = calloc(contentMemCount + 1, sizeof(char));
                    (void) snprintf(contentJson[contentJsonSize - 1], contentMemCount,
                                    "{\"value\":\"%f\",\"size\":%zu}",
                                    value,
                                    readSize);
                }
            }
            break;

            // produces no json
            case iter_context: {
                // create a new iter
                if (currIterations == 0) {
                    iterStorage = context;

                    context = listCreateIterator(frame->contexts);

                    for (size_t i = 0; i < cc->min; i++) {
                        listIteratorNext(&context);
                    }
                }

                // iter
                if (currIterations != cc->max && currIterations != 0) {
                    context = listCreateIterator(frame->contexts);

                    for (size_t i = 0; i < cc->min; i++) {
                        listIteratorNext(&context);
                    }
                }

                // reset
                if (currIterations >= cc->max) {
                    context = iterStorage;

                    for (size_t i = 0; i < currIterations; i++) {
                        listIteratorNext(&context);
                    }

                    currIterations = 0;
                }

                currIterations++;


                // This will go on forever until a failure condition is met by a previous context
                // for example, latin1_context detects null
            }
            break;

            // base 64 again but concatenated
            case adjustment_context: {
                ListIter contentContextIter = listCreateIterator(frame->contexts);
                ListIter contentEntryIter = listCreateIterator(frame->entries);
                size_t poscc = 0;
                size_t posce = 0;
                size_t readSize = 0;
                size_t contentMemCount = 0;
                void *iterNext = NULL;
                char *b64 = NULL;


                // hunt down "adjustment" key
                while ((iterNext = listIteratorNext(&contentContextIter)) != NULL) {
                    if (((Id3v2ContentContext *) iterNext)->type == iter_context) {
                        poscc--;
                    }

                    if (((Id3v2ContentContext *) iterNext)->key == id3v2djb2("adjustment")) {
                        break;
                    }

                    poscc++;
                }

                // hunt down adjustment value
                while ((iterNext = listIteratorNext(&contentEntryIter)) != NULL) {
                    if (poscc == posce) {
                        readSize = btou32((uint8_t *) ((Id3v2ContentEntry *) iterNext)->entry,
                                          (int) ((Id3v2ContentEntry *) iterNext)->size);
                        break;
                    }

                    posce++;
                }


                tmp = id3v2ReadFrameEntry(&trav, &readSize);

                if (tmp == NULL || readSize == 0) {
                    exit = true;
                    break;
                }


                b64 = internal_base64Encode(tmp, readSize);
                free(tmp);


                contentMemCount = snprintf(NULL, 0,
                                           "{\"value\":\"%s\",\"size\":%zu}",
                                           b64,
                                           strlen(b64));

                contentJsonSize++;
                if (contentJson == NULL) {
                    contentJson = (char **) calloc(contentJsonSize, sizeof(char *));
                    contentJson[contentJsonSize - 1] = calloc(contentMemCount + 1, sizeof(char));

                    (void) snprintf(contentJson[contentJsonSize - 1], contentMemCount,
                                    "{\"value\":\"%s\",\"size\":%zu}",
                                    b64,
                                    readSize);
                } else {
                    char **reallocContentJson = (char **) realloc((void *) contentJson,
                                                                  (contentJsonSize) * sizeof(char *));

                    if (reallocContentJson == NULL) {
                        for (size_t i = 0; i < contentJsonSize - 1; i++) {
                            free(contentJson[i]);
                        }
                        free((void *) contentJson);
                        free(b64);
                        return NULL;
                    }
                    contentJson = reallocContentJson;
                    contentJson[contentJsonSize - 1] = calloc(contentMemCount + 1, sizeof(char));
                    (void) snprintf(contentJson[contentJsonSize - 1], contentMemCount,
                                    "{\"value\":\"%s\",\"size\":%zu}",
                                    b64,
                                    readSize);
                }

                free(b64);
            }
            break;

            case unknown_context:
            default:
                exit = true;
                break;
        }

        if (exit == true) {
            break;
        }
    }

    headerJson = id3v2FrameHeaderToJSON(frame->header, version);

    // concatenate all JSON data stored in contentJson into a single string split by ","
    if (contentJson != NULL) {
        for (size_t i = 0; i < contentJsonSize; i++) {
            concatenatedStringLength += strlen(contentJson[i]) + 1;
        }

        concatenatedString = calloc(concatenatedStringLength + 1, sizeof(char));

        size_t offset = 0;
        for (size_t i = 0; i < contentJsonSize; i++) {
            if (i > 0) {
                concatenatedString[offset++] = ',';
            }

            size_t currentLen = strlen(contentJson[i]);
            memcpy(concatenatedString + offset, contentJson[i], currentLen);
            offset += currentLen;
        }
        concatenatedString[offset] = '\0';
    }

    memCount += snprintf(NULL, 0,
                         "{\"header\":%s,\"content\":[%s]}",
                         headerJson,
                         concatenatedString);

    json = calloc(memCount + 1, sizeof(char));
    (void) snprintf(json, memCount,
                    "{\"header\":%s,\"content\":[%s]}",
                    headerJson,
                    concatenatedString);


    free(headerJson);

    if (concatenatedString != NULL) {
        free(concatenatedString);
    }

    if (contentJson != NULL) {
        for (size_t i = 0; i < contentJsonSize; i++) {
            free(contentJson[i]);
        }
        free((void *) contentJson);
    }

    return json;
}
