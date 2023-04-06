#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v2Header.h"
#include "id3Helpers.h"
#include "id3Reader.h"

/*
    Header functions
*/

Id3v2Header *id3v2ParseHeader(id3buf buffer, unsigned int bufferSize){

    id3byte version[3];
    id3byte flags[2];
    id3byte tmpHeaderSize[ID3V2_HEADER_SIZE_LEN + 1]; 
    bool extFlag = false;

    bool unsynchronisation = false;
    bool experimentalIndicator = false;
    bool footer = false;
    size_t size = 0;
    Id3v2ExtHeader *extendedHeader = NULL;

    Id3Reader *stream = id3NewReader(buffer, bufferSize);
    
    //skip 'ID3' bytes
    id3ReaderSeek(stream, 3, SEEK_CUR);
    
    //read version
    //version can not be 0xFF
    if(id3ReaderCursor(stream)[0] == 0xFF || id3ReaderCursor(stream)[1] == 0xFF){
        id3FreeReader(stream);
        return NULL;
    }
    
    //read version
    id3ReaderRead(stream, version, 2);

    // unsuported version
    if((int)version[0] > 4 || (int)version[0] < 2){
        id3FreeReader(stream);
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

Id3v2Header *id3v2NewHeader(int versionMinor, int versionMajor, bool unsynchronisation, bool experimentalIndicator, bool footer, size_t size, Id3v2ExtHeader *extendedHeader){
    
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

Id3v2Header *id3v2CopyHeader(Id3v2Header *toCopy){
    return (toCopy == NULL) ? NULL : id3v2NewHeader(toCopy->versionMinor, toCopy->versionMajor, toCopy->unsynchronisation, toCopy->experimentalIndicator, toCopy->footer, toCopy->size, id3v2CopyExtendedHeader(toCopy->extendedHeader));
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

Id3v2ExtHeader *id3v2ParseExtendedHeader(id3buf buffer, Id3v2HeaderVersion version){

    if(buffer == NULL){
        return NULL;
    }
    
    int size = 0;
    int padding = 0;
    id3byte update = 0x00;
    id3byte tagSizeRestriction = 0x00;
    id3byte encodingRestriction = 0x00;
    id3byte textSizeRestriction = 0x00;
    id3byte imageEncodingRestriction = 0x00;
    id3byte imageSizeRestriction = 0x00;
    id3buf crc = NULL;
    id3byte tmpPaddingSize[ID3V2_PADDING_SIZE+1];
    bool crcFlag = false;
    bool updateFlag = false;
    bool restrictionsFlag = false;
    
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
            crc = calloc(sizeof(id3byte), ID3V2_CRC_LEN + 1);
            id3ReaderRead(stream, crc, ID3V2_CRC_LEN);
        }

        id3FreeReader(stream);

    }else if(version == ID3V24){
        
        //read the header size
        size = syncint_decode(getBits8(buffer, ID3V2_HEADER_SIZE_LEN));
  
        Id3Reader *stream = id3NewReader(buffer,size);
        //size is known so skip it
        id3ReaderSeek(stream, ID3V2_HEADER_SIZE_LEN, SEEK_CUR);

        //if any other value this invalid flags exist
        if(id3ReaderGetCh(stream) == 1){
            
            //read flags
            id3ReaderSeek(stream, 1, SEEK_CUR);

            updateFlag = ((id3ReaderGetCh(stream) >> 6) & 1) ? true : false;
            crcFlag = ((id3ReaderGetCh(stream) >> 5) & 1) ? true : false;
            restrictionsFlag = ((id3ReaderGetCh(stream) >> 4) & 1) ? true : false;
    
            id3ReaderSeek(stream, 1, SEEK_CUR);

        }

        //read update byte
        if(updateFlag){
            update = id3ReaderGetCh(stream);
            id3ReaderSeek(stream, 1, SEEK_CUR);
        }

        //read crc
        if(crcFlag){
            id3ReaderSeek(stream, 1, SEEK_CUR);
            crc = calloc(sizeof(id3byte), ID3V2_CRC_LEN + 1);
            id3ReaderRead(stream, crc, ID3V2_CRC_LEN+1);
        }

        //read restrictions
        if(restrictionsFlag){
            id3ReaderSeek(stream, 1, SEEK_CUR);

            tagSizeRestriction |= (id3ReaderGetCh(stream) >> 7) & 1;
            tagSizeRestriction |= (id3ReaderGetCh(stream) >> 6) & 1;
            
            encodingRestriction = (id3ReaderGetCh(stream) >> 5) & 1;

            textSizeRestriction |= (id3ReaderGetCh(stream) >> 4) & 1;
            textSizeRestriction |= (id3ReaderGetCh(stream) >> 3) & 1;
            
            imageEncodingRestriction = (id3ReaderGetCh(stream) >> 2) & 1;

            imageSizeRestriction |= (id3ReaderGetCh(stream) >> 1) & 1;
            imageSizeRestriction |= (id3ReaderGetCh(stream) >> 0) & 1;    
            id3ReaderSeek(stream, 1, SEEK_CUR);
        }

        padding = stream->bufferSize - stream->cursor;

        id3FreeReader(stream);
    }


    return id3v2NewExtendedHeader(size, padding, update, crc, tagSizeRestriction, encodingRestriction, textSizeRestriction, imageEncodingRestriction, imageSizeRestriction);
}

Id3v2ExtHeader *id3v2NewExtendedHeader(int size, int padding, id3byte update, id3buf crc, id3byte tagSizeRestriction, id3byte encodingRestriction, id3byte textSizeRestriction, id3byte imageEncodingRestriction, id3byte imageSizeRestriction){

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

Id3v2ExtHeader *id3v2CopyExtendedHeader(Id3v2ExtHeader *toCopy){

    if(toCopy == NULL){
        return NULL;
    }

    int size = 0;
    int padding = 0;
    id3byte update = 0x00;
    id3byte tagSizeRestriction = 0x00;
    id3byte encodingRestriction = 0x00;
    id3byte textSizeRestriction = 0x00;
    id3byte imageEncodingRestriction = 0x00;
    id3byte imageSizeRestriction = 0x00;
    id3buf crc = NULL;

    size = toCopy->size;
    padding = toCopy->padding;
    update = toCopy->update;
    tagSizeRestriction = toCopy->tagSizeRestriction;
    encodingRestriction = toCopy->encodingRestriction;
    textSizeRestriction = toCopy->textSizeRestriction;
    imageEncodingRestriction = toCopy->imageEncodingRestriction;
    imageSizeRestriction = toCopy->imageSizeRestriction;

    if(toCopy->crc != NULL){
        crc = calloc(sizeof(id3byte), toCopy->crcLen + 1);
        memcpy(crc, toCopy->crc, toCopy->crcLen);
    }

    return id3v2NewExtendedHeader(size, padding, update, crc, tagSizeRestriction, encodingRestriction, textSizeRestriction, imageEncodingRestriction, imageSizeRestriction);
}

void id3v2FreeExtHeader(Id3v2ExtHeader *extHeader){

    if(extHeader == NULL){
        return;
    }

    if(extHeader->crc != NULL){
        free(extHeader->crc);
    }

    free(extHeader);
}

bool containsId3v2(id3buf buffer){
    return (memcmp("ID3",buffer,3) == 0) ? true: false;
}
