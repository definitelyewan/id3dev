/**
 * @file id3v2Frame.c
 * @author Ewan Jones
 * @brief Functions for creating, destroying, and manipulating ID3v2 frames
 * @version 0.1
 * @date 2024-04-11
 * 
 * @copyright Copyright (c) 2024
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

static char *base64Encode(const unsigned char *input, size_t inputLength){
    const char *base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t outputLength = 4 * ((inputLength + 2) / 3);
    char *output = malloc(outputLength + 1);
    if(output == NULL){
        return NULL;
    }

    size_t i = 0;
    size_t j = 0;
    for(i = 0, j = 0; i < inputLength; i += 3, j += 4){
        unsigned char byte1 = input[i];
        unsigned char byte2 = (i + 1 < inputLength) ? input[i + 1] : 0;
        unsigned char byte3 = (i + 2 < inputLength) ? input[i + 2] : 0;

        output[j] = base64Chars[byte1 >> 2];
        output[j + 1] = base64Chars[((byte1 & 0x03) << 4) | (byte2 >> 4)];
        output[j + 2] = (i + 1 < inputLength) ? base64Chars[((byte2 & 0x0F) << 2) | (byte3 >> 6)] : '=';
        output[j + 3] = (i + 2 < inputLength) ? base64Chars[byte3 & 0x3F] : '=';
    }

    output[outputLength] = '\0';
    return output;
}


/**
 * @brief Creates an ID3v2 frame header structure
 * 
 * @param id 
 * @param tagAlter 
 * @param fileAlter 
 * @param readOnly 
 * @param decompressionSize 
 * @param encryptionSymbol 
 * @param groupSymbol 
 * @return Id3v2FrameHeader* 
 */
Id3v2FrameHeader *id3v2CreateFrameHeader(uint8_t id[ID3V2_FRAME_ID_MAX_SIZE], bool tagAlter, bool fileAlter, bool readOnly, bool unsync, uint32_t decompressionSize, uint8_t encryptionSymbol, uint8_t groupSymbol){

    Id3v2FrameHeader *h = malloc(sizeof(Id3v2FrameHeader));

    for(int i = 0; i < ID3V2_FRAME_ID_MAX_SIZE; i++){
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
 * @brief Frees all of the memory allocated for a frame header
 * 
 * @param toDelete 
 */
void id3v2DestroyFrameHeader(Id3v2FrameHeader **toDelete){

    if(*toDelete){
        free(*toDelete);
        *toDelete = NULL;
        toDelete = NULL;
    }
}

/**
 * @brief Allocates memory for a contextEntry and performs a deep copy of the provided entry
 * 
 * @param entry 
 * @param size 
 * @return Id3v2ContentEntry* 
 */
Id3v2ContentEntry *id3v2CreateContentEntry(void *entry, size_t size){

    Id3v2ContentEntry *ce = malloc(sizeof(Id3v2ContentEntry));

    if(!size){
        ce->entry = NULL;
        ce->size = 0;

    }else{

        void *tmp = malloc(size);
        memcpy(tmp, entry, size);

        ce->entry = tmp;
        ce->size = size;
    }

    return ce;
}

/**
 * 
 * List/Hash API functions
 *  
 */


/**
 * @brief Comparse two entries and returns the difference. Note, only the
 * data itself is compared and not its size. The lower of the two sizes
 * will be used to compare the held data.
 * @details This is not ideal but it needs to be done as ive found different
 * writers Jaikoz, itunes, Mp3tag, Mp3diag, and kid3 all write strings 
 * differently. This is a compatability decision when id3v2 is parsed.  
 * @param first 
 * @param second 
 * @return int 
 */
int id3v2CompareContentEntry(const void *first, const void *second){

    Id3v2ContentEntry *one = (Id3v2ContentEntry *)first;
    Id3v2ContentEntry *two = (Id3v2ContentEntry *)second;

    if(one == NULL || two == NULL){
        return 1;
    }
    
    int diff = 0;
    size_t usableSize = (one->size <= two->size) ? one->size : two->size;
    
    for(size_t i = 0; i < usableSize; i++){
        if(((unsigned char *)one->entry)[i] != ((unsigned char *)two->entry)[i]){
            diff = ((unsigned char*)one->entry)[i] - ((unsigned char*)two->entry)[i];
            return diff;
        }
    }

    return 0;
}

/**
 * @brief Generates a string that represents a context
 * 
 * @param toBePrinted 
 * @return char* 
 */
char *id3v2PrintContentEntry(const void *toBePrinted){

    Id3v2ContentEntry *e = (Id3v2ContentEntry *)toBePrinted; 

    char *str = malloc((sizeof(char) * 16) + sizeof(long) + 8);

    sprintf(str, "Size: %ld, data: %p\n",e->size, e->entry);

    return str;
}

/**
 * @brief Performs a deep copy of an entry
 * 
 * @param toBeCopied 
 * @return void* 
 */
void *id3v2CopyContentEntry(const void *toBeCopied){

    Id3v2ContentEntry *e = (Id3v2ContentEntry *)toBeCopied;
    
    return id3v2CreateContentEntry(e->entry, e->size);

}

/**
 * @brief Frees an entry
 * 
 * @param toBeDeleted 
 */
void id3v2DeleteContentEntry(void *toBeDeleted){

    Id3v2ContentEntry *e = (Id3v2ContentEntry *)toBeDeleted;

    if(e->entry != NULL){
        free(e->entry);
    }
    free(e);
}

/**
 * @brief Frees a frame in a linked list
 * 
 * @param toBeDeleted 
 */
void id3v2DeleteFrame(void *toBeDeleted){
    Id3v2Frame *f = (Id3v2Frame *) toBeDeleted;
    id3v2DestroyFrame(&f);
}

/**
 * @brief Compares a frame in a linked list
 * 
 * @param first 
 * @param second 
 * @return int 
 */
int id3v2CompareFrame(const void *first, const void *second){

    Id3v2Frame *f = (Id3v2Frame *)first;
    Id3v2Frame *s = (Id3v2Frame *)second;
    ListIter i1, i2;
    void *tmp1 = NULL;
    void *tmp2 = NULL;
    int diff = 0;

    if(f == NULL){
        return -1;
    }

    if(s == NULL){
        return 1;
    }

    diff = memcmp(f->header->id, s->header->id, ID3V2_FRAME_ID_MAX_SIZE);
    if(diff != 0){
        return diff;
    }

    diff = f->header->decompressionSize - s->header->decompressionSize;
    if(diff != 0){
        return diff;
    }

    diff = f->header->encryptionSymbol - s->header->encryptionSymbol;
    if(diff != 0){
        return diff;
    }

    if(f->header->fileAlterPreservation != s->header->fileAlterPreservation){
        return 1;
    }

    diff = f->header->groupSymbol - s->header->groupSymbol;
    if(diff != 0){
        return diff;
    }

    if(f->header->readOnly != s->header->readOnly){
        return 1;
    }

    if(f->header->tagAlterPreservation != s->header->tagAlterPreservation){
        return 1;
    }

    if(f->header->unsynchronisation != s->header->unsynchronisation){
        return 1;
    }

    diff = f->entries->length - s->entries->length;
    if(diff != 0){
        return diff;
    }

    i1 = listCreateIterator(f->entries);
    i2 = listCreateIterator(s->entries);

    while((tmp1 = listIteratorNext(&i1)) != NULL){
        
        tmp2 = listIteratorNext(&i2);
        
        diff = id3v2CompareContentEntry(tmp1, tmp2);

        if(diff != 0){
            return diff;
        }
        
    }

    diff = f->contexts->length - s->contexts->length;
    if(diff != 0){
        return diff;
    }

    i1 = listCreateIterator(f->contexts);
    i2 = listCreateIterator(s->contexts);

    while((tmp1 = listIteratorNext(&i1)) != NULL){
        
        tmp2 = listIteratorNext(&i2);
        
        diff = id3v2CompareContentContext(tmp1, tmp2);
        if(diff != 0){
            return diff;
        }
        
    }

    return diff;
}

/**
 * @brief Builds a string representation of a frame
 * 
 * @param toBePrinted 
 * @return char* 
 */
char *id3v2PrintFrame(const void *toBePrinted){

    Id3v2Frame *f = (Id3v2Frame *)toBePrinted;    
    char *s = malloc(64); // 8 byte pointers
    memset(s, 0, 64);

    sprintf(s, "header : %p, context : %p, entries : %p", f->header, f->contexts, f->entries);

    return s;
}

/**
 * @brief Deep copys a frame
 * 
 * @param toBeCopied 
 * @return void* 
 */
void *id3v2CopyFrame(const void *toBeCopied){
    
    Id3v2Frame *f = (Id3v2Frame *)toBeCopied;
    
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
 * @brief Creates a frame 
 * 
 * @param header 
 * @param context 
 * @param entries 
 * @return Id3v2Frame* 
 */
Id3v2Frame *id3v2CreateFrame(Id3v2FrameHeader *header, List *context, List *entries){

    Id3v2Frame *frame = malloc(sizeof(Id3v2Frame));

    frame->contexts = context;
    frame->entries = entries;
    frame->header = header;

    return frame;
}

/**
 * @brief Destroys all memory in a frame header
 * 
 * @param toDelete 
 */
void id3v2DestroyFrame(Id3v2Frame **toDelete){
    
    if(*toDelete){
        listFree((*toDelete)->contexts);
        listFree((*toDelete)->entries);
        id3v2DestroyFrameHeader(&(*toDelete)->header);
        free(*toDelete);
        *toDelete = NULL;
        toDelete = NULL;
    }
}


/**
 * @brief Creates a frame with each entry set to 0 with the correct context. default pairings will be
 * scanned first and then userPairs will be scanned. If no pairings are found this function will use a
 * generic pairing. If this function fails it will return NULL.
 * 
 * @param id 
 * @return Id3v2Frame* 
 */
Id3v2Frame *id3v2CreateEmptyFrame(const char id[ID3V2_FRAME_ID_MAX_SIZE], uint8_t version, HashTable *userPairs){

    if(id == NULL){
        return NULL;
    }

    HashTable *pairs = NULL;
    List *context = NULL;
    List *entries = NULL;
    Id3v2ContentContext *cc = NULL;
    Id3v2FrameHeader *header = NULL;
    Id3v2Frame *f = NULL;

    pairs = id3v2CreateDefaultIdentiferContextPairings(version);

    // first pass
    context = hashTableRetrieve(pairs, id);

    // second pass
    if(context == NULL && id[0] == 'T'){
        context = hashTableRetrieve(pairs, "T");
    }

    // third pass
    if(context == NULL && id[0] == 'W'){
        context = hashTableRetrieve(pairs, "W");
    }

    // forth pass
    if(context == NULL){
        context = hashTableRetrieve(userPairs, id);

    }

    // fifth pass generic
    if(context == NULL){
        context = hashTableRetrieve(pairs, "?");
    }


    ListIter i = listCreateIterator(context);
    
    entries = listCreate(id3v2PrintContentEntry, id3v2DeleteContentEntry, id3v2CompareContentEntry, id3v2CopyContentEntry);
    while((cc = listIteratorNext(&i)) != NULL){
        if(cc->type != iter_context){
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
 * @brief Creates a list iter specific to frame content
 * 
 * @param tag 
 * @return ListIter 
 */
ListIter id3v2CreateFrameTraverser(Id3v2Tag *tag){

    ListIter e;
    e.current = NULL;

    if(tag == NULL){
        return e;
    }

    if(tag->frames == NULL){
        return e;
    }

    return listCreateIterator(tag->frames);
}

/**
 * @brief treverses through a list of frames with the use of a list iter
 * and returns a referance to frame content while doing so. If null is 
 * returned, the end of the list has been reached.
 * 
 * @param traverser 
 * @return Id3v2Frame* 
 */
Id3v2Frame *id3v2FrameTraverse(ListIter *traverser){
    return (Id3v2Frame *) listIteratorNext(traverser);
}

/**
 * @brief Creates a list iterator specific to frame entries
 * 
 * @param frame 
 * @return ListIter 
 */
ListIter id3v2CreateFrameEntryTraverser(Id3v2Frame *frame){

    ListIter e;
    e.current = NULL;

    if(frame == NULL){
        return e;
    }

    if(frame->entries == NULL){
        return e;
    }

    return listCreateIterator(frame->entries);
}


/**
 * @brief Returns the data at the traversers current position and 
 * moves onto the next entry.
 * 
 * @param traverser 
 * @param dataSize 
 * @return void* 
 */
void *id3v2ReadFrameEntry(ListIter *traverser, size_t *dataSize){

    if(traverser == NULL){
        *dataSize = 0;
        return NULL;
    }

    Id3v2ContentEntry *data = (Id3v2ContentEntry *) listIteratorNext(traverser);
    void *ret = NULL;


    if(data == NULL){
        *dataSize = 0;
        return NULL;
    }

    if(!data->size){
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
 * @brief Returns a UTF8 representation of the data held at the traversers current
 * position. the size of the returned data is returned though dataSize and a UTF8
 * string is returned to the caller if successful. If this function fails it will
 * return NULL and a dataSize of 0.
 * 
 * @param traverser 
 * @param dataSize 
 * @return char* 
 */
char *id3v2ReadFrameEntryAsChar(ListIter *traverser, size_t *dataSize){
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
    if(!tmp){
        return NULL;
    }

    // detect utf16
    if(*dataSize > BYTE_BOM_SIZE){
        if(byteHasBOM(tmp)){
            if(tmp[0] == 0xff && tmp[1] == 0xfe){
                encoding = BYTE_UTF16LE;
            }else{
                encoding = BYTE_UTF16BE;
            }
        }
    }

    // add some padding to tmp
    tmp = realloc(tmp, *dataSize + (BYTE_PADDING * 2));
    memset(tmp + *dataSize, 0, (BYTE_PADDING * 2));
    outLen = *dataSize + (BYTE_PADDING * 2);

    // detect utf8/ascii
    if(byteIsUtf8(tmp)){
        encoding = BYTE_UTF8;
    }

    // detect latin1
    if(byteIsUtf8(tmp) && encoding == 0){
        encoding = BYTE_ISO_8859_1;
    }

    // convert to UTF8
    convi = byteConvertTextFormat(tmp, encoding, *dataSize + (BYTE_PADDING * 2), &outString, BYTE_UTF8, &outLen);

    if(!convi && outLen == 0){
        free(tmp);
        *dataSize = 0;
        return NULL;
    }

    // data is already in utf8
    if(convi && outLen == 0){
        outString = tmp;
    }else{
        *dataSize = outLen;
        free(tmp);
    }

    // check for UTF8 BOM
    if(*dataSize >= 3 && outString[0] == 0xEF && outString[1] == 0xBB && outString[2] == 0xBF) {
        utf8BomOffset = 3;
    }

    // escape quotes and backslashes
    escapedStr = malloc(2 * (*dataSize - utf8BomOffset) + 1);
    j = 0;

    for(size_t i = 0; i < *dataSize - utf8BomOffset; i++){
        
        if(outString[i + utf8BomOffset] == '"' || outString[i + utf8BomOffset] == '\\'){
            escapedStr[j++] = '\\';
            escapedStr[j++] = outString[i + utf8BomOffset];
        }else{
            escapedStr[j++] = outString[i + utf8BomOffset];
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

    if(escapedStr != NULL && *dataSize == 0){
        *dataSize = 1;
    }

    return escapedStr;
}

/**
 * @brief Returns a 8-bit integer representation of the data held at the traversers 
 * current position. If this function fails it will return 0.
 * 
 * @param traverser 
 * @return uint8_t 
 */
uint8_t id3v2ReadFrameEntryAsU8(ListIter *traverser){

    uint8_t *tmp = NULL;
    uint8_t ret = 0;
    size_t dataSize = 0;

    tmp = (uint8_t *) id3v2ReadFrameEntry(traverser, &dataSize);

    // Failed
    if(!tmp){
        return 0;
    }

    ret = tmp[0];
    free(tmp);

    return ret;
}

/**
 * @brief Returns a 16-bit integer representation of the data held at the traversers 
 * current position. If this function fails it will return 0.
 * 
 * @param traverser 
 * @return uint16_t 
 */
uint16_t id3v2ReadFrameEntryAsU16(ListIter *traverser){

    unsigned char *tmp = NULL;
    uint16_t ret = 0;
    
    size_t dataSize = 0;

    tmp = (unsigned char *) id3v2ReadFrameEntry(traverser, &dataSize);

    // Failed
    if(!tmp){
        return 0;
    }

    if(dataSize >= sizeof(uint16_t)){
        ret = (uint16_t)tmp[1] << 8 | (uint16_t)tmp[0];
    }else if(dataSize == sizeof(uint8_t)){
        ret = (uint16_t)tmp[0];
    }
    
    free(tmp);
    return ret;
}

/**
 * @brief Returns a 32-bit integer representation of the data held at the traversers 
 * current position. If this function fails it will return 0.
 * 
 * @param traverser 
 * @return uint32_t 
 */
uint32_t id3v2ReadFrameEntryAsU32(ListIter *traverser){


    unsigned char *tmp = NULL;
    uint32_t ret = 0;
    size_t dataSize = 0;

    tmp = (unsigned char *) id3v2ReadFrameEntry(traverser, &dataSize);

    if(!tmp){
        return 0;
    }

    // clamp to max size
    if(dataSize > sizeof(uint32_t)){
        dataSize = sizeof(uint32_t);
    }

    switch(dataSize) {
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
 * @brief Writes entrySize bytes of entry at the current position of the traverser.
 * However, the data provided will be truncated as to not violate the entries context.
 * Its recommneded to not change the context to make your data fit but instread create a
 * new pairing with a new frame ID. This is to best ensure compatability with other 
 * ID3v2 readers. You wouldnt want to break your cars metadata reader because it can no
 * longer read a songs title. If this function fails it will return false otherwise, true.
 * 
 * @param frame 
 * @param entries 
 * @param entrySize 
 * @param entry 
 * @return true 
 * @return false 
 */
bool id3v2WriteFrameEntry(Id3v2Frame *frame, ListIter *entries, size_t entrySize, void *entry){

    if(frame == NULL || entries == NULL || entrySize == 0 || entry == NULL){
        return false;
    }

    if(frame->contexts == NULL || frame->entries == NULL || entries->current == NULL){
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
    while((ce = (Id3v2ContentEntry *) listIteratorNext(&entriesIter)) != NULL){

        int comp = frame->entries->compareData((void *) ce, (void *) entries->current->data);

        if(comp == 0){
            break;
        }

        posce++;
    }

    if(ce == NULL){
        return false;
    }

    // loacte the context for the entry
    while((cc = (Id3v2ContentContext *) listIteratorNext(&contextIter)) != NULL){

        if(cc->type == iter_context){
            // in case an iter is the first context
            poscc = (poscc == 0) ? 0 : poscc - 1;
        }

        if(poscc == posce){
            break;
        }

        poscc++;
    }

    if(cc == NULL){
        return false;
    }

    newSize = entrySize;

    if(entrySize > cc->max){
        newSize = cc->max;
    }
    
    if(entrySize < cc->min){
        newSize = cc->min;
    }


    newData = malloc(newSize);
    memset(newData, 0, newSize);
    memcpy(newData, entry, newSize);

    free(((Id3v2ContentEntry *)entries->current->data)->entry);
    ((Id3v2ContentEntry *)entries->current->data)->entry = newData;
    ((Id3v2ContentEntry *)entries->current->data)->size = newSize;

    return true;
}


/**
 * @brief Inserts a frame into the tag. This function returns true on success and false
 * otherwise.
 * 
 * @param tag 
 * @param frame 
 * @return true 
 * @return false 
 */
bool id3v2AttachFrameToTag(Id3v2Tag *tag, Id3v2Frame *frame){

    if(tag == NULL || frame == NULL){
        return false;
    }

    if(tag->frames == NULL || frame->contexts == NULL || frame->entries == NULL || frame->header == NULL){
        return false;
    }

    return listInsertBack(tag->frames, (void *) frame) ? true : false;

}


/**
 * @brief Removes a frame from a tag and returns it to the caller otherwise, NULL.
 * 
 * @param tag 
 * @param frame 
 * @return Id3v2Frame* 
 */
Id3v2Frame *id3v2DetatchFrameFromTag(Id3v2Tag *tag, Id3v2Frame *frame){

    if(tag == NULL || frame == NULL){
        return NULL;
    }

    return listDeleteData(tag->frames, (void *) frame);
}


/**
 * @brief Converts a frame header into its binary representation. If this 
 * function fails it will return NULL otherwise, an uint8_t pointer.
 * 
 * @param header 
 * @param version 
 * @param frameSize 
 * @return ByteStream* 
 */
uint8_t *id3v2FrameHeaderSerialize(Id3v2FrameHeader *header, uint8_t version, uint32_t frameSize, size_t *outl){

    unsigned char *tmp = NULL;
    ByteStream *stream = NULL;
    uint8_t *out = NULL;

    if(header == NULL || version > ID3V2_TAG_VERSION_4){
        *outl = 0;
        return NULL;
    }

    switch(version){
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

            if(header->decompressionSize > 0){
                byteStreamResize(stream, stream->bufferSize + 4);
                tmp = u32tob(header->decompressionSize);
                byteStreamWrite(stream, tmp, 4);
                free(tmp);
            }

            if(header->encryptionSymbol > 0){
                byteStreamResize(stream, stream->bufferSize + 1);
                byteStreamWrite(stream, &header->encryptionSymbol, 1);
            }

            if(header->groupSymbol > 0){
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

            if(header->groupSymbol > 0){
                byteStreamResize(stream, stream->bufferSize + 1);
                byteStreamWrite(stream, &header->groupSymbol, 1);
            }

            if(header->encryptionSymbol > 0){
                byteStreamResize(stream, stream->bufferSize + 1);
                byteStreamWrite(stream, &header->encryptionSymbol, 1);
            }

            if(header->decompressionSize > 0){
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
 * @brief Converts a frame header structure into its representation as JSON.
 * 
 * @param header 
 * @param version 
 * @return char* 
 */
char *id3v2FrameHeaderToJSON(Id3v2FrameHeader *header, uint8_t version){

    char *json = NULL;
    size_t memCount = 3;

    if(header == NULL){
        json = calloc(memCount, sizeof(char));
        memcpy(json, "{}\0", memCount);
        return json;
    }

    switch(version){
        case ID3V2_TAG_VERSION_2:

            memCount += snprintf(NULL, 0,
                                "{\"id\":\"%c%c%c\"}",
                                header->id[0],
                                header->id[1],
                                header->id[2]);

            json = calloc(memCount + 1, sizeof(char)); 

            snprintf(json, memCount,
                    "{\"id\":\"%c%c%c\"}",
                    header->id[0],
                    header->id[1],
                    header->id[2]);

            break;
        case ID3V2_TAG_VERSION_3:

            memCount += snprintf(NULL, 0,
                                "{\"id\":\"%c%c%c%c\",\"tagAlterPreservation\":%s,\"fileAlterPreservation\":%s,\"readOnly\":%s,\"decompressionSize\":%"PRId32",\"encryptionSymbol\":%d,\"groupSymbol\":%d}",
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

            snprintf(json, memCount,
                    "{\"id\":\"%c%c%c%c\",\"tagAlterPreservation\":%s,\"fileAlterPreservation\":%s,\"readOnly\":%s,\"decompressionSize\":%"PRId32",\"encryptionSymbol\":%d,\"groupSymbol\":%d}",
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
                                "{\"id\":\"%c%c%c%c\",\"tagAlterPreservation\":%s,\"fileAlterPreservation\":%s,\"readOnly\":%s,\"unsynchronisation\":%s,\"decompressionSize\":%"PRId32",\"encryptionSymbol\":%d,\"groupSymbol\":%d}",
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

            snprintf(json, memCount,
                    "{\"id\":\"%c%c%c%c\",\"tagAlterPreservation\":%s,\"fileAlterPreservation\":%s,\"readOnly\":%s,\"unsynchronisation\":%s,\"decompressionSize\":%"PRId32",\"encryptionSymbol\":%d,\"groupSymbol\":%d}",
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
 * @brief Converts a frame structure into its binary representation. If this
 * function fails it will return NULL otherwise, an uint8_t structure.
 * 
 * @param frame 
 * @param version 
 * @return ByteStream* 
 */
uint8_t *id3v2FrameSerialize(Id3v2Frame *frame, uint8_t version, size_t *outl){
    
    ByteStream *stream = NULL;
    Id3v2ContentContext *cc = NULL;

    if(frame == NULL || version > ID3V2_TAG_VERSION_4){
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

    while((cc = listIteratorNext(&context)) != NULL){

        switch(cc->type){

            // encoding will always be enforced
            case encodedString_context:{
                
                ListIter contentContextIter = listCreateIterator(frame->contexts);
                ListIter contentEntryIter = listCreateIterator(frame->entries);
                size_t poscc = 0;
                size_t posce = 0;
                size_t utf8Len = 0;
                uint8_t encoding = 0;
                void *iterNext = NULL;
                bool convi = false; 
                

                // hunt down "encoding" key
                while((iterNext = listIteratorNext(&contentContextIter)) != NULL){

                    if(((Id3v2ContentContext *)iterNext)->type == iter_context){
                        poscc--;
                    }

                    if(((Id3v2ContentContext *)iterNext)->key == id3v2djb2("encoding")){
                        break;
                    }

                    poscc++;
                }
                
                // hunt down encoding value
                while((iterNext = listIteratorNext(&contentEntryIter)) != NULL){

                    if(poscc == posce){
                        encoding = ((uint8_t *)((Id3v2ContentEntry *)iterNext)->entry)[0];
                        break;
                    }

                    posce++;
                }

                // enforce encoding as utf8
                tmp = (unsigned char *)id3v2ReadFrameEntryAsChar(&trav, &utf8Len);

                if(tmp == NULL || utf8Len == 0){
                    exit = true;
                    break;
                }

                unsigned char *outStr = NULL;
                size_t outLen = 0;

                // non-empty strings
                if(utf8Len >= 1 && tmp[0] != 0){
                    convi = byteConvertTextFormat(tmp, BYTE_UTF8, utf8Len, &outStr, encoding, &outLen);

                    if(convi == false && outLen == 0){
                        free(tmp);
                        exit = true;
                        break;
                    }


                    // data is already in utf8
                    if(convi && outLen == 0){
                        outStr = tmp;
                        outLen = utf8Len;
                    }else{
                        free(tmp);
                    }

                    // prepend BOM
                    if(encoding == BYTE_UTF16BE || encoding == BYTE_UTF16LE){
                        bytePrependBOM(encoding, &outStr, &outLen);
                    }
                }else{
                    free(tmp);
                }

                // append null spacer if there are more entries in the list
                if(trav.current != NULL){
                    switch(encoding){
                        case BYTE_ISO_8859_1:
                        case BYTE_ASCII:
                        case BYTE_UTF8:
                            outStr = realloc(outStr, outLen + 1);
                            memset(outStr + outLen, 0, 1);
                            outLen++;
                            break;
                        case BYTE_UTF16BE:
                        case BYTE_UTF16LE:
                            outStr = realloc(outStr, outLen + 2);
                            memset(outStr + outLen, 0, 2);
                            outLen += 2;
                            break;
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

            // written the same way with no spacer *coms cut out* i repeat no spacer over
            case numeric_context:
            case noEncoding_context:
            case binary_context:
            case precision_context:
                tmp = id3v2ReadFrameEntry(&trav, &readSize);
                
                if(tmp == NULL){
                    exit = true;
                    break;
                }

                byteStreamResize(stream, stream->bufferSize + readSize);
                byteStreamWrite(stream, tmp, readSize);
                free(tmp);
                contentSize += readSize;
                break;

            // latin1 will be enforced
            case latin1Encoding_context:{
                
                bool convi = false;
                unsigned char *outStr = NULL;
                size_t outLen = 0;
                size_t utf8len = 0;

                tmp = (unsigned char *)id3v2ReadFrameEntryAsChar(&trav, &utf8len);

                if(tmp == NULL){
                    exit = true;
                    break;
                }

                // ensure latin1
                convi = byteConvertTextFormat(tmp, BYTE_UTF8, utf8len, &outStr, BYTE_ISO_8859_1, &outLen);

                if(convi == false && outLen == 0){
                    free(tmp);
                    break;
                }

                // add spacer
                if(trav.current != NULL){
                    outStr = realloc(outStr, outLen + 1);
                    memset(outStr + outLen, 0, 1);
                    outLen++;
                }

                byteStreamResize(stream, stream->bufferSize + outLen);
                byteStreamWrite(stream, outStr, outLen);

                free(tmp);
                free(outStr);
                contentSize += outLen;
                break;
            }

            case iter_context:{

                // create a new iter
                if(currIterations == 0){
                    
                    iterStorage = context;

                    context = listCreateIterator(frame->contexts);
                    
                    for(size_t i = 0; i < cc->min; i++){
                        listIteratorNext(&context);
                    }
                }

                // iter 
                if(currIterations != cc->max && currIterations != 0){
                    context = listCreateIterator(frame->contexts);

                    for(size_t i = 0; i < cc->min; i++){
                        listIteratorNext(&context);
                    }
                }

                // reset
                if(currIterations >= cc->max){

                    context = iterStorage;

                    for(size_t i = 0; i < currIterations; i++){
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
            // wildly long and probably inefficient but it works for now and is the best i can do with my current knowledge
            case bit_context:{

                // there is another bit context next
                if(listIteratorHasNext(context)){
                    if(((Id3v2ContentContext *)context.current->data)->type != bit_context){
                        bitFlag = false;
                    }else{
                        bitFlag = true;
                    }
                }


                // at least 1 or more bit contexts in a row
                if(bitFlag == true){
                    
                    unsigned char *bitBuff = NULL;
                    unsigned char **byteDataArr = NULL;
                    size_t *byteDataSizeArr = NULL;
                    size_t *nbits = NULL;
                    
                    size_t arrSize = 0;
                    size_t totalBits = 0;
                    size_t totalBytes = 0;
                    size_t bitBuffSize = 0;
                    
                    // copy values
                    while(true){
                        
                        tmp = id3v2ReadFrameEntry(&trav, &readSize);

                        if(tmp == NULL){
                            exit = true;
                            break;

                        }

                        if(byteDataSizeArr == NULL){
                            
                            byteDataSizeArr = malloc(sizeof(size_t));
                            byteDataSizeArr[0] = readSize;

                            nbits = malloc(sizeof(size_t));
                            nbits[0] = cc->max;

                            byteDataArr = malloc(sizeof(unsigned char *));
                            byteDataArr[0] = malloc(readSize);

                            for(size_t i = 0; i < readSize; i++){
                                byteDataArr[0][i] = tmp[i];
                            }

                            arrSize++;

                        }else{
                            arrSize++;
                            byteDataSizeArr = realloc(byteDataSizeArr, arrSize * sizeof(size_t));
                            byteDataSizeArr[arrSize - 1] = readSize;

                            nbits = realloc(nbits, arrSize * sizeof(size_t));
                            nbits[arrSize - 1] = cc->max;

                            byteDataArr = realloc(byteDataArr, arrSize * sizeof(unsigned char *));
                            byteDataArr[arrSize - 1] = malloc(readSize);

                            for(size_t i = 0; i < readSize; i++){
                                byteDataArr[arrSize - 1][i] = tmp[i];
                            }

                        }

                        totalBits += cc->max;

                        free(tmp);


                        if(listIteratorHasNext(context)){
                            
                            if(((Id3v2ContentContext *)context.current->data)->type != bit_context){
                                break;
                            
                            // seek to the next context 
                            }else{
                                cc = listIteratorNext(&context);
                            }
                        }else{
                            break;
                        }
                    }

                    totalBytes = ((totalBits / CHAR_BIT) % 2) ? (totalBits / CHAR_BIT) + 1: totalBits / CHAR_BIT; // ? odd : even

                    bitBuff = malloc(totalBytes);
                    memset(bitBuff, 0, totalBytes);
                    bitBuffSize = totalBytes;

                    // reverse the byte data
                    for(size_t i = 0; i < arrSize; i++){
                        size_t dataSize = byteDataSizeArr[i];
                        size_t halfSize = dataSize / 2;
                        for(size_t j = 0; j < halfSize; j++){
                            unsigned char temp = byteDataArr[i][j];
                            byteDataArr[i][j] = byteDataArr[i][dataSize - j - 1];
                            byteDataArr[i][dataSize - j - 1] = temp;
                        }
                    }

                    int step = 0;
                    int offset = 0;
                    int bitIndex = 0;
                    for(size_t i = totalBytes; i > 0; i--){
                        
                        if(step >= arrSize || totalBytes == 0){
                            break;
                        }
                        
                        size_t nBit = nbits[step];
                        size_t nBytes = byteDataSizeArr[step];
                        unsigned char *data = byteDataArr[step];
                        int counter = 0;

                        while(nBit > 0){
                            
                            if(counter == nBytes){
                                break;
                            }

                            int j = 0;
                            for(j = 0; j < CHAR_BIT; j++){
                                if(nBit == 0){
                                    break;
                                }
                                
                                // switch to the next byte
                                if(j + offset >= CHAR_BIT){
                                    offset = 0;
                                    totalBytes--;
                                    bitIndex = 0;
                                }

                                bitBuff[totalBytes - 1] = setBit(bitBuff[totalBytes - 1], bitIndex, readBit(data[counter], j) > 0 ? true : false);
                                bitIndex++;

                                nBit--;
                            }

                            if(j < CHAR_BIT){
                                offset = j;
                            }else{
                                offset = 0;
                            }
                            counter++;
                            
                        }

                        step++;
                    }

                    byteStreamResize(stream, stream->bufferSize + bitBuffSize);
                    byteStreamWrite(stream, bitBuff, bitBuffSize);
                    contentSize += bitBuffSize;

                    for(size_t i = 0; i < arrSize; i++){
                        free(byteDataArr[i]);
                    }

                    free(bitBuff);
                    free(nbits);
                    free(byteDataArr);
                    free(byteDataSizeArr);

                    bitFlag = false;

                // read a single and only bit context
                }else{
                    
                    int totalBytesNeeded = (cc->max / CHAR_BIT) + 1;
                    int nBit = CHAR_BIT - 1;

                    tmp = id3v2ReadFrameEntry(&trav, &readSize);

                    if(tmp == NULL){
                        exit = true;
                        break;
                    }

                    while(totalBytesNeeded > 0){
                        
                        byteStreamResize(stream, stream->bufferSize + 1);

                        while(nBit >= 0){
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

            case adjustment_context:{

                ListIter contentContextIter = listCreateIterator(frame->contexts);
                ListIter contentEntryIter = listCreateIterator(frame->entries);
                size_t poscc = 0;
                size_t posce = 0;
                uint32_t rSize = 0;
                void *iterNext = NULL;
                

                // hunt down "adjustment" key
                while((iterNext = listIteratorNext(&contentContextIter)) != NULL){

                    if(((Id3v2ContentContext *)iterNext)->type == iter_context){
                        poscc--;
                    }

                    if(((Id3v2ContentContext *)iterNext)->key == id3v2djb2("adjustment")){
                        break;
                    }

                    poscc++;
                }
                
                // hunt down adjustment value
                while((iterNext = listIteratorNext(&contentEntryIter)) != NULL){

                    if(poscc == posce){
                        rSize = btou32((uint8_t *)((Id3v2ContentEntry *)iterNext)->entry, (size_t)((Id3v2ContentEntry *)iterNext)->size);
                        break;
                    }

                    posce++;
                }

                tmp = id3v2ReadFrameEntry(&trav, &readSize);

                if(tmp == NULL){
                    exit = true;
                    break;

                }

                byteStreamResize(stream, stream->bufferSize + rSize);
                byteStreamWrite(stream, tmp, rSize);

                contentSize += rSize;
                free(tmp);
                break;
            }
                break;
            case unknown_context:
            default:
                exit = true;
                break;
        }

        if(exit == true){
            break;
        }

    }


    // write in the frame size
    switch(version){

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
 * @brief Converts a frame structure into its JSON representation.
 * 
 * @param frame 
 * @param version 
 * @return char* 
 */
char *id3v2FrameToJSON(Id3v2Frame *frame, uint8_t version){

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

    if(frame == NULL || version > ID3V2_TAG_VERSION_4){
        json = calloc(memCount, sizeof(char));
        memcpy(json, "{}\0", memCount);
        return json;
    }

    while((cc = (Id3v2ContentContext *)listIteratorNext(&context)) != NULL){

        switch(cc->type){

            // treated as base64
            case noEncoding_context:
            case bit_context:
            case binary_context:{
                size_t readSize = 0;
                size_t contentMemCount = 3;
                char *b64 = NULL;

                tmp = id3v2ReadFrameEntry(&trav, &readSize);

                if(tmp == NULL || readSize == 0){
                    exit = true;
                    break;
                }


                b64 = base64Encode(tmp, readSize);

                contentMemCount += snprintf(NULL, 0,
                                           "{\"value\":\"%s\",\"size\":%ld}",
                                           b64,
                                           strlen(b64));
                
                contentJsonSize++;
                if(contentJson == NULL){
                    contentJson = calloc(contentJsonSize, sizeof(char *));
                    contentJson[contentJsonSize - 1] = calloc(contentMemCount + 1, sizeof(char));

                    snprintf(contentJson[contentJsonSize - 1], contentMemCount,
                             "{\"value\":\"%s\",\"size\":%ld}",
                             b64,
                             readSize);                    

                }else{
                    contentJson = realloc(contentJson, (contentJsonSize) * sizeof(char *));
                    contentJson[contentJsonSize - 1] = calloc(contentMemCount + 1, sizeof(char));
                    snprintf(contentJson[contentJsonSize - 1], contentMemCount,
                             "{\"value\":\"%s\",\"size\":%ld}",
                             b64,
                             readSize);   
                }

                free(tmp);
                free(b64);
            }
                break; 


            // will always be treated as utf8 when in json
            case encodedString_context:
            case latin1Encoding_context:{
                size_t readSize = 0;
                size_t contentMemCount = 3;

                tmp = (unsigned char *) id3v2ReadFrameEntryAsChar(&trav, &readSize);

                if(tmp == NULL || readSize == 0){
                    exit = true;
                    break;
                }


                contentMemCount += snprintf(NULL, 0,
                                           "{\"value\":\"%s\",\"size\":%ld}",
                                           tmp,
                                           readSize);

                contentJsonSize++;
                if(contentJson == NULL){
                    contentJson = calloc(contentJsonSize, sizeof(char *));
                    contentJson[contentJsonSize - 1] = calloc(contentMemCount + 1, sizeof(char));

                    snprintf(contentJson[contentJsonSize - 1], contentMemCount,
                             "{\"value\":\"%s\",\"size\":%ld}",
                             tmp,
                             readSize);                    

                }else{
                    contentJson = realloc(contentJson, (contentJsonSize) * sizeof(char *));
                    contentJson[contentJsonSize - 1] = calloc(contentMemCount + 1, sizeof(char));
                    snprintf(contentJson[contentJsonSize - 1], contentMemCount,
                             "{\"value\":\"%s\",\"size\":%ld}",
                             tmp,
                             readSize);   
                }

                free(tmp);

            }
                break;

            case numeric_context:{
                size_t readSize = 0;
                size_t contentMemCount = 3;
                size_t num = 0;

                tmp = id3v2ReadFrameEntry(&trav, &readSize);

                if(tmp == NULL || readSize == 0){
                    exit = true;
                    break;
                }

                num = btost(tmp, readSize);
                free(tmp);

                contentMemCount += snprintf(NULL, 0,
                                           "{\"value\":\"%ld\",\"size\":%ld}",
                                           num,
                                           readSize);

                contentJsonSize++;
                if(contentJson == NULL){
                    contentJson = calloc(contentJsonSize, sizeof(char *));
                    contentJson[contentJsonSize - 1] = calloc(contentMemCount + 1, sizeof(char));

                    snprintf(contentJson[contentJsonSize - 1], contentMemCount,
                             "{\"value\":\"%ld\",\"size\":%ld}",
                             num,
                             readSize);                    

                }else{
                    contentJson = realloc(contentJson, (contentJsonSize) * sizeof(char *));
                    contentJson[contentJsonSize - 1] = calloc(contentMemCount + 1, sizeof(char));
                    snprintf(contentJson[contentJsonSize - 1], contentMemCount,
                             "{\"value\":\"%ld\",\"size\":%ld}",
                             num,
                             readSize);   
                }
            }
                break;
            
            case precision_context:{
                size_t readSize = 0;
                size_t contentMemCount = 0;
                float value = 0;

                tmp = id3v2ReadFrameEntry(&trav, &readSize);

                if(tmp == NULL || readSize == 0){
                    exit = true;
                    break;
                }


                memcpy(&value, tmp, sizeof(value));
                free(tmp);

                contentMemCount = snprintf(NULL, 0,
                                           "{\"value\":\"%f\",\"size\":%ld}",
                                           value,
                                           readSize);

                contentJsonSize++;
                if(contentJson == NULL){
                    contentJson = calloc(contentJsonSize, sizeof(char *));
                    contentJson[contentJsonSize - 1] = calloc(contentMemCount + 1, sizeof(char));

                    snprintf(contentJson[contentJsonSize - 1], contentMemCount,
                             "{\"value\":\"%f\",\"size\":%ld}",
                             value,
                             readSize);                    

                }else{
                    contentJson = realloc(contentJson, (contentJsonSize) * sizeof(char *));
                    contentJson[contentJsonSize - 1] = calloc(contentMemCount + 1, sizeof(char));
                    snprintf(contentJson[contentJsonSize - 1], contentMemCount,
                             "{\"value\":\"%f\",\"size\":%ld}",
                             value,
                             readSize);   
                }

            }
                break;
            
            // produces no json
            case iter_context:{
                // create a new iter
                if(currIterations == 0){
                    
                    iterStorage = context;

                    context = listCreateIterator(frame->contexts);
                    
                    for(size_t i = 0; i < cc->min; i++){
                        listIteratorNext(&context);
                    }
                }

                // iter 
                if(currIterations != cc->max && currIterations != 0){
                    context = listCreateIterator(frame->contexts);

                    for(size_t i = 0; i < cc->min; i++){
                        listIteratorNext(&context);
                    }
                }

                // reset
                if(currIterations >= cc->max){

                    context = iterStorage;

                    for(size_t i = 0; i < currIterations; i++){
                        listIteratorNext(&context);
                    }

                    currIterations = 0;

                }

                currIterations++;

                
                // This will go on forever until a failure condition is met by a previous context
                // i.e latin1_context detects null
                 
            }
                break;

            // base 64 again but concatenated
            case adjustment_context:{
                ListIter contentContextIter = listCreateIterator(frame->contexts);
                ListIter contentEntryIter = listCreateIterator(frame->entries);
                size_t poscc = 0;
                size_t posce = 0;
                size_t readSize = 0;
                size_t contentMemCount = 0;
                void *iterNext = NULL;
                char *b64 = NULL;
                

                // hunt down "adjustment" key
                while((iterNext = listIteratorNext(&contentContextIter)) != NULL){

                    if(((Id3v2ContentContext *)iterNext)->type == iter_context){
                        poscc--;
                    }

                    if(((Id3v2ContentContext *)iterNext)->key == id3v2djb2("adjustment")){
                        break;
                    }

                    poscc++;
                }
                
                // hunt down adjustment value
                while((iterNext = listIteratorNext(&contentEntryIter)) != NULL){

                    if(poscc == posce){
                        readSize = btou32((uint8_t *)((Id3v2ContentEntry *)iterNext)->entry, (size_t)((Id3v2ContentEntry *)iterNext)->size);
                        break;
                    }

                    posce++;
                }


                tmp = id3v2ReadFrameEntry(&trav, &readSize);

                if(tmp == NULL || readSize == 0){
                    exit = true;
                    break;
                }


                b64 = base64Encode(tmp, readSize);
                free(tmp);



                contentMemCount = snprintf(NULL, 0,
                                           "{\"value\":\"%s\",\"size\":%ld}",
                                           b64,
                                           strlen(b64));

                contentJsonSize++;
                if(contentJson == NULL){
                    contentJson = calloc(contentJsonSize, sizeof(char *));
                    contentJson[contentJsonSize - 1] = calloc(contentMemCount + 1, sizeof(char));

                    snprintf(contentJson[contentJsonSize - 1], contentMemCount,
                             "{\"value\":\"%s\",\"size\":%ld}",
                             b64,
                             readSize);                    

                }else{
                    contentJson = realloc(contentJson, (contentJsonSize) * sizeof(char *));
                    contentJson[contentJsonSize - 1] = calloc(contentMemCount + 1, sizeof(char));
                    snprintf(contentJson[contentJsonSize - 1], contentMemCount,
                             "{\"value\":\"%s\",\"size\":%ld}",
                             b64,
                             readSize);   
                }

            }
                break;

            case unknown_context:
            default:
                exit = true;
                break;
        }

        if(exit == true){
            break;
            
        }
    }

    headerJson = id3v2FrameHeaderToJSON(frame->header, version);

    // concatenate all JSON data stored in contentJson into a single string split by ","
    for(size_t i = 0; i < contentJsonSize; i++){
        concatenatedStringLength += strlen(contentJson[i]) + 1;
    }
    

    concatenatedString = calloc(concatenatedStringLength + 1, sizeof(char));
    for(size_t i = 0; i < contentJsonSize; i++){
        strcat(concatenatedString, contentJson[i]);
        if(i < contentJsonSize - 1){
            strcat(concatenatedString, ",");
        }
    }

    memCount += snprintf(NULL, 0,
                        "{\"header\":%s,\"content\":[%s]}",
                        headerJson,
                        concatenatedString);


    json = malloc((memCount + 1) * sizeof(char));
    strcpy(json, "{\"header\":");
    strcat(json, headerJson);
    strcat(json, ",\"content\":[");
    strcat(json, concatenatedString);
    strcat(json, "]}");


    free(headerJson);

    if(concatenatedString != NULL){
        free(concatenatedString);
    }

    if(contentJson != NULL){
        for(size_t i = 0; i < contentJsonSize; i++){
            free(contentJson[i]);

        }
        free(contentJson);
    }

    return json;
}
