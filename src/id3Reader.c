#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3Reader.h"

Id3Reader *id3NewReader(unsigned char *buffer,  size_t bufferSize){

    if(buffer == NULL){
        return NULL;
    }
    
    if(bufferSize <= 0){
        return NULL;
    }


    Id3Reader *reader = malloc(sizeof(Id3Reader));

    reader->buffer = calloc(sizeof(id3byte), bufferSize);
    reader->bufferSize = bufferSize;
    reader->cursor = 0;

    memcpy(reader->buffer, buffer, bufferSize);
    return reader;
}

void id3FreeReader(Id3Reader *toDelete){

    if(toDelete == NULL){
        return;
    }

    if(toDelete->buffer != NULL){
        free(toDelete->buffer);
    }

    free(toDelete);
}

unsigned int id3ReaderAllocationAdd(id3byte encoding){
    
    switch(encoding){
        case ISO_8859_1:
            return 1;
        case UTF16:
            return 2;
        case UTF16BE:
            return 2;
        case UTF8:
            return 2;
    }

    return 0;
}

void id3ReaderRead(Id3Reader *reader, id3buf dest, unsigned int size){
    
    if(reader == NULL){
        dest = NULL;
        return;
    }

    if(size <= 0){
        dest = NULL;
        return;
    }
    if(size > reader->bufferSize){
        dest = NULL;
        return;
    }

    memcpy(dest, id3ReaderCursor(reader), size);
    reader->cursor = reader->cursor + size;
}

id3buf id3ReaderEncodedRemainder(Id3Reader *reader, id3byte encoding){
    
    if(reader == NULL){
        return NULL;
    }

    if(reader->cursor > reader->bufferSize){
        return NULL;
    }

    int size = reader->bufferSize - reader->cursor;
    
    if(size == 0){
        return NULL;
    }    

    id3buf remainder = calloc(sizeof(id3byte), size + id3ReaderAllocationAdd(encoding));
    id3ReaderRead(reader, remainder, size);
    return remainder;
}

size_t id3ReaderReadEncodedSize(Id3Reader *reader, id3byte encoding){

    size_t readSize = 0;

    if(reader == NULL){
        return readSize;
    }

    switch(encoding){
        
        case ISO_8859_1:
            readSize = strlen((char *)id3ReaderCursor(reader));
            return readSize;
        
        case UTF16:
            if(!hasBOM(id3ReaderCursor(reader))){
                return readSize;
            }

            readSize = strlenUTF16(id3ReaderCursor(reader));
            return readSize;

        case UTF16BE:
            readSize = strlenUTF16BE(id3ReaderCursor(reader));
            return readSize;
        case UTF8:
            readSize = strlenUTF8(id3ReaderCursor(reader));
            return readSize;
        default:
            //just look for a single 0 and eat it
            for(readSize = 1; id3ReaderCursor(reader)[readSize-1] != 0x00; readSize++);
            return readSize;
    }

    return readSize;
}

void id3ReaderSeek(Id3Reader *reader, size_t dest, const int seekOption){

    if(reader == NULL){
        return;
    }
    
    switch(seekOption){
        
        //reset to the beginning
        case SEEK_SET:
            reader->cursor = dest;
            break;

        //use dest as new cursor    
        case SEEK_CUR:
            reader->cursor = reader->cursor + dest;
            break;
        
        //set cursor to end of buffer
        case SEEK_END:
            reader->cursor = reader->bufferSize - dest;
            break;

        default:
            break;
    }
}

id3buf id3ReaderCursor(Id3Reader *reader){
    if(reader == NULL){
        return NULL;
    }
   
    if(reader->cursor >= reader->bufferSize){
        return NULL;
        
    }
    //printf("buffer size = %ld, calc = %ld\n",reader->bufferSize ,reader->bufferSize - reader->cursor);
    return reader->buffer + reader->cursor; 
}

int id3ReaderGetCh(Id3Reader *reader){
    
    if(reader == NULL){
        return EOF;
    }

    return id3ReaderCursor(reader) == NULL ? EOF: (int) id3ReaderCursor(reader)[0];
}


bool hasBOM(id3buf buffer){

    if(buffer == NULL){
        return false;
    }

    return (memcmp("\xFF\xFE", buffer, UNICODE_BOM_SIZE) == 0 || 
            memcmp("\xFE\xFF", buffer, UNICODE_BOM_SIZE) == 0) ? true : false;
}

size_t strlenUTF16(id3buf buffer){

    size_t len = 0;

    if(buffer == NULL){
        return len;
    }

    //not utf16
    if(!hasBOM(buffer)){
        return len;
    }
    
    while(true){
        //0x00 0x00 means the end of utf
        if(buffer[len] == 0x00 && buffer[len + 1] == 0x00){
            break;
        }

        len = len + 2;
    }
    
    return len;
}

size_t strlenUTF16BE(id3buf buffer){

    size_t len = 0;

    if(buffer == NULL){
        return len;
    }

    if(hasBOM(buffer)){
        return len;
    }
    
    while(true){
        //0x00 0x00 means the end of utf
        if(buffer[len] == 0x00 && buffer[len + 1] == 0x00){
            break;
        }

        len = len + 2;
    }

    return len;
}

size_t strlenUTF8(id3buf buffer){
    //utf8 is variable length and will have no null bytes until the end
    return strlen((char *)buffer);
}

size_t id3strlen(id3buf buffer, id3byte encoding){

    if(buffer == NULL){
        return 0;
    }
    size_t len = 0;
    switch(encoding){
        case ISO_8859_1:
            len = strlen((char *)buffer);
            break;
        case UTF16:
            len = strlenUTF16(buffer) + 1;
            break;
        case UTF16BE:
            len = strlenUTF16BE(buffer) + 1;
            break;
        case UTF8:
            len = strlenUTF8(buffer);
            break;
        default:
            return len;
    }

    return len;
}


void id3ReaderPrintf(Id3Reader *reader){

    if(reader == NULL){
        printf("[]\n");
        return;
    }

    if(reader->cursor == reader->bufferSize){
        printf("[]\n");
        return;
    }

    int size = reader->bufferSize - reader->cursor;
    
    if(size == 0){
        printf("[]\n");
        return;
    }

    printf("[");
    for(int i = 0; i < size; i++){
        printf("[%x]",id3ReaderCursor(reader)[i]);
    }
    printf("]\n");
}

bool id3ValidEncoding(id3byte encoding){
    if(!(encoding == ISO_8859_1 || encoding == UTF16 || encoding == UTF16BE || encoding == UTF8)){
        return false;
    }

    return true;
}
