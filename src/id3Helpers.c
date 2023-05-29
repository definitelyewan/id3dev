#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stddef.h>
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

char *integerToCharPointer(int value){

    int n = log10(value) + 1;
    char *numberArray = calloc(n, sizeof(char));

    for(int i = n-1; i >= 0; --i, value /= 10){
        numberArray[i] = (value % 10) + '0';
    }

    return numberArray;
}

void addressFree(void **pptr){
    if(pptr && *pptr){
        free(*pptr);
        *pptr = NULL;
        pptr = NULL;
    }
}

//should work with utf16be
unsigned char *utf16ToUtf8(unsigned char* src, size_t srcSize){
    
    if(src == NULL){
        return NULL;
    }

    size_t index = 0;
    bool bomOffset = false;
    unsigned char *dest = NULL;

    //check for bom
    if(srcSize >= UNICODE_BOM_SIZE && src[0] == 0xFE && src[1] == 0xFF){
        bomOffset = true;
    }else if(srcSize >= UNICODE_BOM_SIZE && src[0] == 0xFF && src[1] == 0xFE){
        bomOffset = true;
    }



    //maximum length of the converted string
    size_t maxDestSize = srcSize - (bomOffset ? 2 : 0);
    for(size_t i = bomOffset ? 2 : 0; i < srcSize; i += 2){
        uint16_t codepoint = (src[i] << 8) | src[i + 1];
        if(codepoint >= 0x80 && codepoint < 0x800){
            maxDestSize += 1;
        }else if(codepoint >= 0x800){
            maxDestSize += 2;
        }
    }

    dest = calloc(sizeof(unsigned char),maxDestSize + 2);

    //eat bom pretty hacky tho
    if(bomOffset == true){
        src[UNICODE_BOM_SIZE-1] = 0x00;
    }

    //convert based on 0x0000
    for(size_t i = bomOffset; i < srcSize; i += 2){
        uint16_t codepoint = (src[i] << 8) | src[i + 1];
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

    return dest;
}

unsigned char *utf8ToUtf16(unsigned char* src, size_t srcSize, unsigned int utfv){

    if(src == NULL){
        return NULL;
    }

    size_t index = 0;
    size_t srcIndex = 0;
    size_t destSize = 0;

    //max size required for the destination string
    if(utfv == UTF16){
        destSize += UNICODE_BOM_SIZE;
    }

    while(srcIndex < srcSize){
        unsigned char c = src[srcIndex];
        srcIndex++;

        if((c & 0x80) == 0){
            //1 byte for ascii;
        }else if((c & 0xE0) == 0xC0){
            //2 byte
            ++srcIndex;
        }else if((c & 0xF0) == 0xE0){
            // 3 byte sequence
            srcIndex += 2;
        } else {
            // Invalid UTF-8 sequence
            return NULL;
        }

        destSize += 2; // 2 bytes for each codepoint in UTF-16
    }

    unsigned char *dest = calloc(sizeof(unsigned char),destSize + 2);
    srcIndex = 0;

    //make bom
    if(utfv == UTF16){
        dest[index] = 0xFE;
        index++;
        dest[index] = 0xFF;
        index++;
    }

    while(srcIndex < srcSize){
        unsigned char c = src[srcIndex++];
        uint16_t codepoint;

        if((c & 0x80) == 0){
            //1 byte sequence for ascii
            codepoint = c;
        }else if((c & 0xE0) == 0xC0){
            //2 byte sequence
            codepoint = ((src[srcIndex] & 0x3F) << 6) | (c & 0x1F);
            ++srcIndex;
        }else if((c & 0xF0) == 0xE0){
            //3 byte sequence
            codepoint = ((src[srcIndex] & 0x3F) << 12) | ((src[srcIndex + 1] & 0x3F) << 6) | (c & 0x0F);
            srcIndex += 2;
        }else{
            //invalid utf8 sequence
            free(dest);
            return NULL;
        }

        dest[index] = (codepoint >> 8) & 0xFF;
        index++;
        dest[index] = codepoint & 0xFF;
        index++;
    }

    return dest;
}

bool isISO_8859_1(const unsigned char *str){

    if(str == NULL){
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
    if((str[0] == '\xFE' && str[1] == '\xFF') || (str[0] == '\xFF' && str[1] == '\xFE')){
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
    if((str[0] == '\xFE' && str[1] == '\xFF') || (str[0] == '\xFF' && str[1] == '\xFE')){
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
	
    if(list == NULL){
        return NULL;
    }
    
    if(pos <= 0 || pos > list->size){
        return NULL;
    }

    Id3Node *temp = list->head;
    Id3Node *prev = list->head;
    void *toReturn;
    for(int i = 0; i < pos; i++) {
        if(i == 0 && pos == 1) {
            toReturn = list->head->data;
            list->head = list->head->next;
            free(temp);
            return toReturn;
        }else{
            if(i == pos - 1 && temp) {
                toReturn = temp->data;
                prev->next = temp->next;
                free(temp);
            }else {
                prev = temp;

                if(prev == NULL){
                    break;
                }
                    
                temp = temp->next;
            }
        }
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
