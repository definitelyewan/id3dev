#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include "id3Helpers.h"

int getBits8(unsigned char *bytes, int byteNum){
    
    int byteAsInt = 0x00;
    for(int i = 0; i < byteNum; i++){
        byteAsInt = byteAsInt << 8;
        byteAsInt = byteAsInt | (unsigned char) bytes[i];
    }

    return byteAsInt;	  
}

unsigned int syncint_decode(int value){
    unsigned int a, b, c, d, result = 0x00;
    a = value & 0xFF;
    b = (value >> 8) & 0xFF;
    c = (value >> 16) & 0xFF;
    d = (value >> 24) & 0xFF;

    result = result | a;
    result = result | (b << 7);
    result = result | (c << 14);
    result = result | (d << 21);

    return result;
}

void addressFree(void **pptr){
    if(pptr && *pptr){
        free(*pptr);
        *pptr = NULL;
        pptr = NULL;
    }
}

char *integerToCharPointer(int value){

    int n = log10(value) + 1;
    char *numberArray = calloc(n, sizeof(char));

    for(int i = n-1; i >= 0; --i, value /= 10){
        numberArray[i] = (value % 10) + '0';
    }

    return numberArray;
}

//should be fast and protect against over/under flow
//sub = false is +
//sub = true is -
unsigned int uSafeSum(unsigned int a, unsigned int b, bool sub){
    return (sub) ? //mode selected true = subtraction 
    (b > a) ? 0: a - b: // b - a gives underflow
    ((UINT_MAX - a) < b) ? UINT_MAX: a + b; // gives overflow
}

//should work with utf16be
unsigned char *utf16ToUtf8(unsigned char *src, size_t srcSize){

    if(src == NULL){
        return NULL;
    }

    size_t index = 0;
    size_t newSrcSize = 0;
    bool bomOffset = false;
    unsigned char *dest = NULL;
    unsigned char *newSrc = NULL;

    //check for bom
    if(srcSize >= UNICODE_BOM_SIZE && src[0] == 0xFE && src[1] == 0xFF){
        bomOffset = true;
    }else if(srcSize >= UNICODE_BOM_SIZE && src[0] == 0xFF && src[1] == 0xFE){
        bomOffset = true;
    }

    if(bomOffset == true){
        
        if(src[UNICODE_BOM_SIZE] != 0x00){
            newSrc = calloc(sizeof(unsigned char), srcSize + 1);
            
            /*
                this is problematic and i cant find a way around it

                the problem with the stament newSrc[0] = 0x00; is that 
                a only one endians works or ensures it? so what happens
                if there are two 0x00s back to back well... you probably
                get no string.

                this probably wont occur due to the if condition above
                but it would not be good if it did.  
            */
            newSrc[0] = 0x00;
            
            memcpy(newSrc + 1, src + UNICODE_BOM_SIZE, srcSize - 1);
            newSrcSize = srcSize - UNICODE_BOM_SIZE;
        }else{
            newSrc = calloc(sizeof(unsigned char), srcSize - UNICODE_BOM_SIZE + 1);
            memcpy(newSrc, src + UNICODE_BOM_SIZE, srcSize - UNICODE_BOM_SIZE);
            newSrcSize = srcSize - UNICODE_BOM_SIZE;
        }
        

    }else{
        newSrc = calloc(sizeof(unsigned char), srcSize + 1);
        memcpy(newSrc, src, srcSize);
        newSrcSize = srcSize;
    }

    //maximum length of the converted string
    size_t maxDestSize = newSrcSize;
    for(size_t i = 0; i < newSrcSize; i += 2){
        uint16_t codepoint = (newSrc[i] << 8) | newSrc[i + 1];
        if(codepoint >= 0x80 && codepoint < 0x800){
            maxDestSize += 1;
        }else if(codepoint >= 0x800){
            maxDestSize += 2;
        }
    }

    dest = calloc(sizeof(unsigned char),maxDestSize + 2);

    //convert based on 0x0000
    for(size_t i = 0; i < newSrcSize; i += 2){
        uint16_t codepoint = (newSrc[i] << 8) | newSrc[i + 1];
        if(codepoint < 0x80){
            dest[index] = (unsigned char)codepoint;
            index++;
        }else if(codepoint < 0x800){
            dest[index] = (unsigned char)((codepoint >> 6) | 0xC0);
            index++;
            dest[index] = (unsigned char)((codepoint & 0x3F) | 0x80);
            index++;
        }else{
            dest[index] = (unsigned char)((codepoint >> 12) | 0xE0);
            index++;
            dest[index] = (unsigned char)(((codepoint >> 6) & 0x3F) | 0x80);
            index++;
            dest[index] = (unsigned char)((codepoint & 0x3F) | 0x80);
            index++;
        }
    }

    free(newSrc);
    return dest;
}

unsigned char *utf8ToUtf16(unsigned char* src, size_t srcSize, unsigned int utfv){
    
    if(src == NULL){
        return NULL;
    }

    size_t index = 0;
    size_t destSize = 0;
    unsigned char * bytes = src;

    //max size required for the destination string
    if(utfv == UTF16){
        destSize += UNICODE_BOM_SIZE;
    }

    while(*bytes){
        
        //ASCII
        //use bytes[0] <= 0x7F to allow ASCII control characters
        if((bytes[0] == 0x09 || bytes[0] == 0x0A || bytes[0] == 0x0D || (0x20 <= bytes[0] && bytes[0] <= 0x7E))){
            bytes += 1;
            destSize += 2;
            continue;
        }

        //non-overlong 2-byte
        if(((0xC2 <= bytes[0] && bytes[0] <= 0xDF) && (0x80 <= bytes[1] && bytes[1] <= 0xBF))){
            bytes += 2;
            destSize += 4;
            continue;
        }

        //excluding overlongs
        //straight 3-byte
        //excluding surrogates
        if((bytes[0] == 0xE0 && (0xA0 <= bytes[1] && bytes[1] <= 0xBF) && (0x80 <= bytes[2] && bytes[2] <= 0xBF)) ||
            (((0xE1 <= bytes[0] && bytes[0] <= 0xEC) || bytes[0] == 0xEE || bytes[0] == 0xEF) && (0x80 <= bytes[1] && bytes[1] <= 0xBF) && (0x80 <= bytes[2] && bytes[2] <= 0xBF)) ||
            (bytes[0] == 0xED && (0x80 <= bytes[1] && bytes[1] <= 0x9F) && (0x80 <= bytes[2] && bytes[2] <= 0xBF))){
            bytes += 3;
            destSize += 6;
            continue;
        }
        //planes 1-3
        //planes 4-15
        //plane 16
        if((bytes[0] == 0xF0 && (0x90 <= bytes[1] && bytes[1] <= 0xBF) && (0x80 <= bytes[2] && bytes[2] <= 0xBF) && (0x80 <= bytes[3] && bytes[3] <= 0xBF)) ||
            ((0xF1 <= bytes[0] && bytes[0] <= 0xF3) && (0x80 <= bytes[1] && bytes[1] <= 0xBF) && (0x80 <= bytes[2] && bytes[2] <= 0xBF) && (0x80 <= bytes[3] && bytes[3] <= 0xBF)) ||
            (bytes[0] == 0xF4 && (0x80 <= bytes[1] && bytes[1] <= 0x8F) && (0x80 <= bytes[2] && bytes[2] <= 0xBF) && (0x80 <= bytes[3] && bytes[3] <= 0xBF))){
            bytes += 4;
            destSize += 8;
            continue;
        }

        return NULL;
    }
    
    unsigned char *dest = calloc(sizeof(unsigned char),destSize + 2);

    //make bom
    if(utfv == UTF16){
        dest[index] = 0xFF;
        index++;
        dest[index] = 0xFE;
        index++;
    }

    bytes = src;
    while(*bytes){
        // Read the next UTF-16 code unit
        uint16_t c = *bytes;
        uint16_t codepoint;
        bytes++;

        // ASCII
        if ((c <= 0x7F)) {
            codepoint = c;
            dest[index] = (codepoint >> 8) & 0xFF;
            index++;
            dest[index] = codepoint & 0xFF;
            index++;
            continue;
        }

        // Non-overlong 2-byte
        if ((0xC2 <= c && c <= 0xDF) && (0x80 <= *bytes && *bytes <= 0xBF)) {
            codepoint = ((c & 0x1F) << 6) | (*bytes & 0x3F);
            bytes++;
            dest[index] = (codepoint >> 8) & 0xFF;
            index++;
            dest[index] = codepoint & 0xFF;
            index++;
            continue;
        }

        // Straight 3-byte (excluding surrogates)
        if (((0xE0 <= c && c <= 0xEC) || c == 0xEE || c == 0xEF) &&
            (0x80 <= bytes[0] && bytes[0] <= 0xBF) &&
            (0x80 <= bytes[1] && bytes[1] <= 0xBF)) {
            codepoint = ((c & 0x0F) << 12) | ((*bytes & 0x3F) << 6) | (bytes[1] & 0x3F);
            bytes += 2;
            dest[index] = (codepoint >> 8) & 0xFF;
            index++;
            dest[index] = codepoint & 0xFF;
            index++;
            continue;
        }

        // Planes 1-3, Planes 4-15, Plane 16
        if ((0xF0 <= c && c <= 0xF3) &&
            (0x80 <= bytes[0] && bytes[0] <= 0xBF) &&
            (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF)) {
            codepoint = ((c & 0x07) << 18) | ((*bytes & 0x3F) << 12) | ((bytes[1] & 0x3F) << 6) | (bytes[2] & 0x3F);
            bytes += 3;
            dest[index] = (codepoint >> 16) & 0xFF;
            index++;
            dest[index] = (codepoint >> 8) & 0xFF;
            index++;
            dest[index] = codepoint & 0xFF;
            index++;
            continue;
        }
    }

    return dest;
}

bool isISO_8859_1(const unsigned char *str){

    if(str == NULL){
        return false;
    }

    if(str[0] == '\0'){
        return false;
    }

    size_t i = 0;

    while(str[i] != '\0'){
        if(str[i] >= 0xff){
            return false;
        }
        i++;
    }
    return true;
}

bool isUTF16(const unsigned char *str, size_t length){

    if(str == NULL){
        return false;
    }

    if(length < 2){
        return false;
    }

    //if theres a bom its utf16
    if((str[0] == 0xFE && str[1] == 0xFF) || (str[0] == 0xFF && str[1] == 0xFE)){
        return true;
    }

    return false;
}

bool isUTF16BE(const unsigned char* str, size_t length){
    
    if(str == NULL){
        return false;
    }

    if(length < 2){
        return false;
    }

    //if theres a bom its not utf16be
    if((str[0] == 0xFE && str[1] == 0xFF) || (str[0] == 0xFF && str[1] == 0xFE)){
        return false;
    }


    //check for valid UTF-16BE encoding
    for(size_t i = 0; i < length - 1; i += 2){
        uint16_t codeUnit = (uint16_t)((str[i] << 8) | str[i + 1]);

        //check for invalid high surrogate
        if(codeUnit >= 0xD800 && codeUnit <= 0xDBFF){
            if(i + 2 >= length){
                return false;  //incomplete pair
            }
                
            uint16_t nextCodeUnit = (uint16_t)((str[i + 2] << 8) | str[i + 3]);
            if(nextCodeUnit < 0xDC00 || nextCodeUnit > 0xDFFF){
                return false;  //invalid low surrogate
            }
            
            i += 2;  //skip the next code unit since it is part of the surrogate pair
        }
    }
    return true;
}

bool isUTF8(const unsigned char* str){
    
    if(str == NULL){
        return false;
    }
    
    size_t i = 0;
    while(str[i] != '\0'){
        if((str[i] & 0x80) != 0){
            if((str[i] & 0xE0) == 0xC0) {
                //2 byte utf8 character?
                if((str[i + 1] & 0xC0) != 0x80){
                    return false;  //invalid utf8
                }
                    
                i += 2;
            }else if((str[i] & 0xF0) == 0xE0){
                //3 byte utf8 character?
                if((str[i + 1] & 0xC0) != 0x80 || (str[i + 2] & 0xC0) != 0x80){
                    return false;  //invalid utf8 sequence
                }
                    
                i += 3;
            }else if((str[i] & 0xF8) == 0xF0){
                //4 byte utf8 character?
                if((str[i + 1] & 0xC0) != 0x80 || (str[i + 2] & 0xC0) != 0x80 || (str[i + 3] & 0xC0) != 0x80){
                    return false;  //invalid utf8 sequence
                }
                    
                i += 4;
            }else{
                return false; //just not utf8
            }
        }else{
            i++;
        }
    }
    return true;
}

Id3List *id3NewList(void (*deleteFunction)(void* toDelete), void *(*copyFunction)(void* toCopy)){

    Id3List *list = malloc(sizeof(Id3List));

    list->head = NULL;
    list->size = 0;
    list->deleteData = deleteFunction;
    list->copyData = copyFunction;

    return list;
}

Id3Node *id3NewNode(void *data){

    Id3Node *node = malloc(sizeof(Id3Node));

    node->data = data;
    node->next = NULL;

    return node;
}

void id3PushList(Id3List *list, void *toAdd){

    if (list == NULL || toAdd == NULL) {
        return;
    }
    
    Id3Node *node = id3NewNode(toAdd);
    
    if (list->size == 0) {
        list->head = node;
    } else {
        Id3Node *current = list->head;
        
        while (current->next != NULL) {
            current = current->next;
        }
        
        current->next = node;
    }

    list->size = list->size + 1;

/*
    if(list == NULL || toAdd == NULL){
        return;
    }
    Id3Node *node = id3NewNode(toAdd);
    
    if(list->size == 0){
        list->head = node;
    }else{
        Id3Node *tmp = list->head;
        node->next = tmp;
        list->head = node;
    }

    list->size = list->size + 1;
*/
}

void id3FreeList(Id3List *list){
    
    if(list == NULL){
        return;
    }

    if(list->deleteData == NULL){
        return;
    }

    Id3Node *curr = list->head;
    for(int i = 0; i < list->size; i++){
        if(curr == NULL){
            break;
        }

        Id3Node *tmp = curr;
        curr = tmp->next;
        list->deleteData(tmp->data);
        free(tmp);

    }

    list->size = 0;
}

void id3DestroyList(Id3List *list){
    
    if(list == NULL){
        return;
    }

    if(list->deleteData == NULL){
        return;
    }

    id3FreeList(list);
    free(list);
}

void *id3RemoveList(Id3List *list, int pos){
    
    if(list == NULL || pos < 0 || pos > list->size) {
        return NULL;
    }
    
    Id3Node* tmp = list->head;
    Id3Node* prev = NULL;
    int currentPosition = 0;

    if(list->size == 1 && tmp != NULL){
        void *data = tmp->data;
        free(tmp);
        (list->size)--;
        return data;
    }

    while(tmp != NULL && currentPosition < pos){
        prev = tmp;
        tmp = tmp->next;
        currentPosition++;
    }

    if(tmp != NULL){
        
        //unlink the node
        if(prev != NULL){
            prev->next = tmp->next;
        }else{
            list->head = tmp->next;
        }
        void* data = tmp->data;
        free(tmp);

        (list->size)--;

        return data;
    }

    return NULL;
}

Id3List *id3CopyList(Id3List *list){

    if(list == NULL){
        return NULL;
    }

    if(list->copyData == NULL){
        return NULL;
    }

    Id3List *ret = id3NewList(list->deleteData, list->copyData);
    Id3Node *n = list->head;

    while(n != NULL){
        id3PushList(ret, ret->copyData(n->data));
        n = n->next;
    }

    return ret;
}

Id3ListIter *id3NewListIter(Id3List *list){

    if(list == NULL){
        return NULL;
    }

    Id3ListIter *li = malloc(sizeof(Id3ListIter));

    li->l = list;
    li->curr = list->head;

    return li;
}
Id3ListIter *id3CopyListIter(Id3ListIter *li){

    if(li == NULL){
        return NULL;
    }

    Id3ListIter *liCopy = malloc(sizeof(Id3ListIter)); 

    liCopy->l = li->l;
    liCopy->curr = li->curr;

    return liCopy;

}
void *id3NextListIter(Id3ListIter *li){
    
    if(li == NULL){
        return NULL;
    }
    
    void *data = NULL;

    if(li->curr){
        data = li->curr->data;
        li->curr = li->curr->next;
    }

    return data;
}


bool id3HasNextListIter(Id3ListIter *li){
    return (li->curr != NULL) ? true : false;

}
void id3FreeListIter(Id3ListIter *li){
    if(li != NULL){
        free(li);
    }
}
