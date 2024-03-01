#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v2Frame.h"
#include "id3v2Context.h"
#include "byteInt.h"
#include "byteUnicode.h"

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

    // ce->size = size;
    // ce->entry = entry;

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
 * @brief Comparse two entries and returns the difference
 * 
 * @param first 
 * @param second 
 * @return int 
 */
int id3v2CompareContentEntry(const void *first, const void *second){

    Id3v2ContentEntry *one = (Id3v2ContentEntry *)first;
    Id3v2ContentEntry *two = (Id3v2ContentEntry *)second;

    int diff = 0;
    
    diff = one->size - two->size;
    if(diff != 0){
        return diff;
    }

    diff = memcmp(one->entry,two->entry,one->size);
    if(diff != 0){
        return diff;
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

    diff = memcmp(f->header->id, s->header->id, ID3V2_FRAME_ID_MAX_SIZE);
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

    while((tmp1 = listIteratorNext(&i1)) != NULL || (tmp2 = listIteratorNext(&i2)) != NULL){
        diff = id3v2CompareContentEntry(tmp1, tmp2);
        if(diff != 0){
            return diff;
        }
        
    }

    diff = f->entries->length - s->entries->length;
    if(diff != 0){
        return diff;
    }

    i1 = listCreateIterator(f->contexts);
    i2 = listCreateIterator(s->contexts);

    while((tmp1 = listIteratorNext(&i1)) != NULL || (tmp2 = listIteratorNext(&i2)) != NULL){
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
 * and returns frame content while doing so. If null is returned the end
 * of the list has been reached.
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
 * moves onto the next entry
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

    // escape quotes and backslashes
    escapedStr = malloc(2 * (*dataSize) + 1);
    j = 0;

    for(size_t i = 0; i < *dataSize; ++i){
        if(outString[i] == '"' || outString[i] == '\\'){
            escapedStr[j++] = '\\';
            escapedStr[j++] = outString[i];
        }else{
            escapedStr[j++] = outString[i];
        }
    }
    
    escapedStr[j] = '\0';
    free(outString);

    return escapedStr;
}