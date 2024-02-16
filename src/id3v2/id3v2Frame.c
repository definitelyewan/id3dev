#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v2Frame.h"
#include "id3v2Context.h"
#include "byteInt.h"

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

void id3v2DeleteFrame(void *toBeDeleted){
    Id3v2Frame *f = (Id3v2Frame *) toBeDeleted;
    id3v2DestroyFrame(&f);
}

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


char *id3v2PrintFrame(const void *toBePrinted){

    Id3v2Frame *f = (Id3v2Frame *)toBePrinted;    
    char *s = malloc(64); // 8 byte pointers
    memset(s, 0, 64);

    sprintf(s, "header : %p, context : %p, entries : %p", f->header, f->contexts, f->entries);

    return s;
}

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