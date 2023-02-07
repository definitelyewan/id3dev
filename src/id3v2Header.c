#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v2Header.h"
#include "id3v2Helpers.h"
#include "id3Reader.h"

/*
    Header functions
*/

Id3v2Header *id3v2ParseHeader(unsigned char *buffer, unsigned int bufferSize){

    unsigned char version[3];
    unsigned char flags[2];
    unsigned char tmpHeaderSize[ID3V2_HEADER_SIZE_LEN + 1]; 
    bool extFlag = false;

    bool unsynchronisation = false;
    bool experimentalIndicator = false;
    bool footer = false;
    int size = 0;
    Id3v2ExtHeader *extendedHeader = NULL;

    Id3Reader *stream = id3NewReader(buffer, bufferSize);
    
    //skip 'ID3' bytes
    id3ReaderSeek(stream, 3, SEEK_CUR);
    
    //read version
    //version can not be 0xFF
    if(id3ReaderCursor(stream)[0] == 0xFF || id3ReaderCursor(stream)[1] == 0xFF){
        return NULL;
    }
    
    //read version
    id3ReaderRead(stream, version, 2);

    // unsuported version
    if((int)version[0] > 4 || (int)version[0] < 2){
        return NULL;
    }

    //flags are bits so extract them
    id3ReaderRead(stream, flags, 1);
    unsynchronisation = ((flags[0] >> 7) & 1) ? true: false;
    
    //extended header check
    if((flags[0] >> 6) & 1){
        extFlag = true;
    }
    //experimental check
    experimentalIndicator = ((flags[0] >> 5) & 1) ? true: false; 
    
    //footer check
    footer = ((flags[0] >> 4) & 1) ? true: false;
    
    //frame size
    id3ReaderRead(stream, tmpHeaderSize, ID3V2_HEADER_SIZE_LEN);
    size = syncint_decode(getBits8(tmpHeaderSize,ID3V2_HEADER_SIZE_LEN));
    
    //header extension
    if(extFlag == true){
        
        extendedHeader = id3v2ParseExtendedHeader(id3ReaderCursor(stream), (int)version[0]);
    }
    
    //clean up
    id3FreeReader(stream);

    return id3v2NewHeader((int)version[1], (int)version[0], unsynchronisation, experimentalIndicator, footer, size, extendedHeader);
}

Id3v2Header *id3v2NewHeader(int versionMinor, int versionMajor, bool unsynchronisation, bool experimentalIndicator, bool footer, int size, Id3v2ExtHeader *extendedHeader){
    
    Id3v2Header *header = malloc(sizeof(Id3v2Header));

    header->versionMajor = versionMajor;
    header->versionMinor = versionMinor;
    header->unsynchronisation = unsynchronisation;
    header->experimentalIndicator = experimentalIndicator;
    header->footer = footer;
    header->size = size;
    header->extendedHeader = extendedHeader;

    return header;
}

void id3v2FreeHeader(Id3v2Header *header){

    if(header == NULL){
        return;
    }

    if(header->extendedHeader != NULL){
        id3v2FreeExtHeader(header->extendedHeader);
    }

    free(header);
}

/*
    Extended header functions
*/

Id3v2ExtHeader *id3v2ParseExtendedHeader(unsigned char *buffer, Id3v2HeaderVersion version){

    if(buffer == NULL){
        return NULL;
    }
    
    int size = 0;
    int padding = 0;
    unsigned char update = 0x00;
    unsigned char tagSizeRestriction = 0x00;
    unsigned char encodingRestriction = 0x00;
    unsigned char textSizeRestriction = 0x00;
    unsigned char imageEncodingRestriction = 0x00;
    unsigned char imageSizeRestriction = 0x00;
    unsigned char crc[ID3V2_CRC_LEN+1];
    unsigned char tmpPaddingSize[ID3V2_PADDING_SIZE+1];
    bool crcFlag = false;
    
    //extended headers are treated differently between versions
    if(version == ID3V23){
        //copy size aka first 4 bytes
        size = getBits8(buffer, ID3V2_HEADER_SIZE_LEN);

        Id3Reader *stream = id3NewReader(buffer,size);
        id3ReaderSeek(stream,ID3V2_HEADER_SIZE_LEN + ID3V2_HEADER_SIZE_LEN,SEEK_CUR);
        
        //check if crc exist by checking the flag
        if((id3ReaderCursor(stream)[0] >> 7) & 1){
            crcFlag = true;
        }
        id3ReaderSeek(stream, 2, SEEK_CUR);

        //copy padding
        id3ReaderRead(stream, tmpPaddingSize, ID3V2_PADDING_SIZE);
        padding = getBits8(tmpPaddingSize, ID3V2_PADDING_SIZE);

        //get crc
        if(crcFlag){
            id3ReaderRead(stream, crc, ID3V2_CRC_LEN);
        }

        id3FreeReader(stream);

    }else if(version == ID3V24){
        
        //read the header size
        size = syncint_decode(getBits8(buffer, ID3V2_HEADER_SIZE_LEN));
  
        Id3Reader *stream = id3NewReader(buffer,size);
        //size is known so skip it
        id3ReaderSeek(stream, ID3V2_HEADER_SIZE_LEN, SEEK_CUR);

        //if any other value this invalid
        if(id3ReaderGetCh(stream) == 1){
            
            id3ReaderSeek(stream, 1, SEEK_CUR);
            id3ReaderPrintf(stream);
            if((id3ReaderCursor(stream)[0] >> 6) & 1){
                id3ReaderSeek(stream, 1, SEEK_CUR);
                update = id3ReaderGetCh(stream);
                id3ReaderSeek(stream, -1, SEEK_CUR);
            }
            if((id3ReaderCursor(stream)[0] >> 5) & 1){
            }
            if((id3ReaderCursor(stream)[0] >> 4) & 1){
            }

            id3ReaderSeek(stream, 1, SEEK_CUR);

            


        }

        id3FreeReader(stream);
    }else{
        memset(crc, '\0', ID3V2_CRC_LEN + 1);
    }


    return id3v2NewExtendedHeader(size, padding, update, crc, tagSizeRestriction, encodingRestriction, textSizeRestriction, imageEncodingRestriction, imageSizeRestriction);
}

Id3v2ExtHeader *id3v2NewExtendedHeader(int size, int padding, unsigned char update, unsigned char *crc, unsigned char tagSizeRestriction, unsigned char encodingRestriction, unsigned char textSizeRestriction, unsigned char imageEncodingRestriction, unsigned char imageSizeRestriction){

    Id3v2ExtHeader *extHeader = malloc(sizeof(Id3v2ExtHeader));

    extHeader->crc = crc;
    extHeader->crcLen = ID3V2_CRC_LEN;
    extHeader->encodingRestriction = encodingRestriction;
    extHeader->imageEncodingRestriction = imageEncodingRestriction;
    extHeader->imageSizeRestriction = imageSizeRestriction;
    extHeader->padding = padding;
    extHeader->size = size;
    extHeader->tagSizeRestriction = tagSizeRestriction;
    extHeader->textSizeRestriction = textSizeRestriction;
    extHeader->update = update;

    return extHeader;
}

void id3v2FreeExtHeader(Id3v2ExtHeader *extHeader){

    if(extHeader == NULL){
        return;
    }

    if(extHeader->crc != NULL){
        //free(extHeader->crc);
    }

    free(extHeader);
}




bool containsId3v2(const char *filePath){
    
    FILE *fp = NULL;
    char id3[3];

    if(filePath == NULL || strlen(filePath) == 0){
        return false;
    }

    if((fp = fopen(filePath,"rb")) == NULL){
        return false;
    }

    if((fread(id3, 1, sizeof(char)*3, fp)) == 0){
        fclose(fp);
        return false;
    }

    if(strncmp("ID3",id3,3) == 0){
        fclose(fp);
        return true;
    }

    fclose(fp);
    return false;
}
