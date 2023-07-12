#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v2Frames.h"
#include "id3v2Header.h"
#include "id3v2Manipulate.h"
#include "id3v2Write.h"
#include "id3Reader.h"
#include "id3Helpers.h"

/*
    size functions
*/

unsigned int id3v2CalculateTagSize(Id3v2Tag *tag){

    if(tag == NULL){
        return 0;
    }
    
    unsigned int tagSize = 0;
    unsigned int fSize = 0;
    Id3ListIter *li = NULL;
    Id3v2Frame *frame = NULL;

    //calculate header size
    if(tag->header != NULL){
        //spec says -header size so im unsure if i want this
        //tagSize = ID3V2_HEADER_SIZE;

        if(tag->header->extendedHeader != NULL){
            tagSize = uSafeSum(tagSize, tag->header->extendedHeader->size, false);
        }

        if(tag->header->footer == true){
            tagSize = uSafeSum(tagSize, 10, false);
        }
    }

    li = id3NewListIter(tag->frames);

    if(li != NULL){

        while((frame = id3NextListIter(li)) != NULL){
            fSize = uSafeSum(frame->header->frameSize, frame->header->headerSize, false);
            tagSize = uSafeSum(tagSize, fSize, false);
        }

        id3FreeListIter(li);
    }

    return tagSize;
}

//not in header
//utf16 and utf16be have a 0 byte at the end to know when to stop reading
//this tells me if a given str has that 0 byte
unsigned int modStrLen(id3byte encoding){
    return (encoding == UTF16 || encoding == UTF16BE) ? 1: 0;
}

/*
    inner object writes
*/

void id3v2WritePicture(const char *fileName, Id3v2Frame *frame){

    if(fileName == NULL){
        return;
    }

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    //cannot write data if its not writable
    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    switch(frame->header->idNum){
        case PIC:
            break;
        case APIC:
            break;
        default:
            return;
    }

    FILE *fp = NULL;
    Id3v2PictureBody *body = (Id3v2PictureBody *)frame->frame;

    if((fp = fopen(fileName,"wb")) == NULL){
        return;
    }

    fwrite(body->pictureData,1,body->picSize,fp);
    fclose(fp);
}

void id3v2WriteGeneralEncapsulatedObject(Id3v2Frame *frame){

    if(frame == NULL){
        return;
    }

    if(frame->frame == NULL || frame->header == NULL){
        return;
    }

    //cannot write data if its not writable
    if(!isFrameWritable(frame->header->flagContent)){
        return;
    }

    switch(frame->header->idNum){
        case GEO:
            break;
        case GEOB:
            break;
        default:
            return;
    }


    id3buf buildName = NULL;
    id3buf mime = NULL;
    id3buf fileName = NULL;
    id3buf convertedFileName = NULL;
    id3buf data = NULL;
    unsigned int mimeLen = 0;
    unsigned int fileNameLen = 0;
    unsigned int dataLen = 0;
    FILE *fp = NULL;

    mime = id3v2ReadMIMEType(frame);
    fileName = id3v2ReadObjectFileName(frame);
    data = id3v2ReadGeneralEncapsulatedObjectValue(frame);
    dataLen = id3v2ReadFrameBinaryObjectSize(frame);
    
    //build a file name from in the frame
    if(fileName != NULL){
        convertedFileName = id3TextFormatConvert(fileName, id3strlen(fileName, id3v2ReadEncoding(frame)), UTF8);
        free(fileName);
    }

    if(convertedFileName != NULL){

        mimeLen = id3strlen(mime, ISO_8859_1);
        fileNameLen = id3strlen(convertedFileName, UTF8);

        buildName = calloc(sizeof(unsigned char), mimeLen + fileNameLen + 2);// +1 for .
        strncpy((char *)buildName, (char *)convertedFileName, fileNameLen);
        buildName[fileNameLen] = '.';
        strncat(((char *)buildName) + fileNameLen, (char *)mime, mimeLen);

        fp = fopen((char *)buildName, "wb");
        fwrite(data, sizeof(unsigned char), dataLen, fp);
        fclose(fp);
        
        free(convertedFileName);
    }


    if(mime != NULL){
        free(mime);
    }

    if(data != NULL){
        free(data);
    }
    
    if(buildName != NULL){
        free(buildName);
    }
}

/*
    writers
*/

void id3v2WriteTagToFile(const char *fileName, Id3v2Tag *tag){
    
}


/*
    converts
*/

id3buf id3v2TagToBuffer(unsigned int *len, Id3v2Tag *tag){

    id3buf header = NULL;
    id3buf frame = NULL;
    id3buf raw = NULL;
    unsigned int tagSize = 0;
    unsigned int sz = 0;
    unsigned int step = 0;
    Id3ListIter *li = NULL;
    Id3v2Frame *f = NULL;

    tagSize = id3v2CalculateTagSize(tag);
    if(tagSize == 0){
        *len = 0;
        return NULL;
    }

    raw = calloc(sizeof(id3byte), tagSize + ID3V2_HEADER_SIZE);

    //header
    header = id3v2HeaderToBuffer(&sz, tagSize, tag->header);
    if(header != NULL && sz > 0){
        memcpy(raw, header, sz);
        step += sz;
        free(header);
    
    //if there is no header the tag is unreadable
    }else{
        free(raw);
        *len = 0;
        return NULL;
    }

    li = id3NewListIter(tag->frames);

    while((f = id3NextListIter(li)) != NULL){
        
        frame = id3v2FrameToBuffer(&sz, tag->header->versionMajor, f);

        if(frame != NULL && sz > 0){
            memcpy(raw + step, frame, sz);
            free(frame);
            step += sz;
        }
    }
    id3FreeListIter(li);
    *len = tagSize + ID3V2_HEADER_SIZE;

    return raw;
}

id3buf id3v2HeaderToBuffer(unsigned int *len, unsigned int tagLength, Id3v2Header *header){

    if(header == NULL){
        *len = 0;
        return NULL;
    }

    id3buf raw = NULL;
    id3buf extHeader = NULL;
    char *tmp = NULL;
    unsigned int headerSize = 0;
    unsigned int extLen = 0;
    Id3Reader *stream = NULL;

    switch(header->versionMajor){
        case ID3V22:
            headerSize = 3 + ID3V2_VERSION_SIZE_OF_BYTES + 1 + ID3V2_HEADER_SIZE_LEN;
            stream = id3NewReader(NULL, headerSize);
            
            //id
            id3ReaderWrite(stream, (id3buf)"ID3", 3);
            
            //version
            id3ReaderWrite(stream, (id3buf)&(header->versionMajor),1);
            id3ReaderWrite(stream, (id3buf)&(header->versionMinor),1);

            //flags
            if(header->unsynchronisation == true){
                id3ReaderCursor(stream)[0] ^= 1 << 7;
            }
            id3ReaderSeek(stream, 1, SEEK_CUR);

            //size
            tmp = itob((int)syncintEncode(tagLength));
            id3ReaderSeek(stream, 3 + ID3V2_VERSION_SIZE_OF_BYTES + 1, SEEK_SET);
            id3ReaderWrite(stream, (id3buf)tmp, ID3V2_HEADER_SIZE_LEN);
            free(tmp);
            
            *len = headerSize;
            break;
        case ID3V23:
            headerSize = 3 + ID3V2_VERSION_SIZE_OF_BYTES + 1 + ID3V2_HEADER_SIZE_LEN + ((header->extendedHeader != NULL) ? header->extendedHeader->size: 0);
            stream = id3NewReader(NULL, headerSize);

            //id
            id3ReaderWrite(stream, (id3buf)"ID3", 3);
            
            //version
            id3ReaderWrite(stream, (id3buf)&(header->versionMajor),1);
            id3ReaderWrite(stream, (id3buf)&(header->versionMinor),1);

            //flags
            //%abc00000
            if(header->unsynchronisation == true){
                id3ReaderCursor(stream)[0] ^= 1 << 7;
            }

            if(header->experimentalIndicator == true){
                id3ReaderCursor(stream)[0] ^= 1 << 5;
            }

            if(header->extendedHeader != NULL){
                id3ReaderCursor(stream)[0] ^= 1 << 6;
                id3ReaderSeek(stream, 1 + ID3V2_HEADER_SIZE_LEN, SEEK_CUR);

                //copy ext header
                extHeader = id3v2ExtendedHeaderToBuffer(&extLen, ID3V23, header->extendedHeader);
                if(extHeader != NULL || extLen == 0){
                    id3ReaderWrite(stream, extHeader, extLen);
                    free(extHeader);
                }
            }
            
            //size
            tmp = itob((int)syncintEncode(tagLength));
            id3ReaderSeek(stream, 3 + ID3V2_VERSION_SIZE_OF_BYTES + 1, SEEK_SET);
            id3ReaderWrite(stream, (id3buf)tmp, ID3V2_HEADER_SIZE_LEN);
            free(tmp);

            break;
        case ID3V24:
            headerSize = 3 + ID3V2_VERSION_SIZE_OF_BYTES + 1 + ID3V2_HEADER_SIZE_LEN + ((header->extendedHeader != NULL) ? header->extendedHeader->size: 0);
            stream = id3NewReader(NULL, headerSize);

            //id
            id3ReaderWrite(stream, (id3buf)"ID3", 3);
            
            //version
            id3ReaderWrite(stream, (id3buf)&(header->versionMajor),1);
            id3ReaderWrite(stream, (id3buf)&(header->versionMinor),1);

            //flags
            //%abcd0000
            if(header->unsynchronisation == true){
                id3ReaderCursor(stream)[0] ^= 1 << 7;
            }

            if(header->experimentalIndicator == true){
                id3ReaderCursor(stream)[0] ^= 1 << 5;
            }

            if(header->experimentalIndicator == true){
                id3ReaderCursor(stream)[0] ^= 1 << 4;
            }

            if(header->extendedHeader != NULL){
                id3ReaderCursor(stream)[0] ^= 1 << 6;
                id3ReaderSeek(stream, 1, SEEK_CUR);

                extHeader = id3v2ExtendedHeaderToBuffer(&extLen, ID3V24, header->extendedHeader);

                if(extHeader != NULL && extLen > 0){
                    id3ReaderWrite(stream, extHeader, extLen);
                    free(extHeader);
                }
            }

            //size
            tmp = itob((int)syncintEncode(tagLength));
            id3ReaderSeek(stream, 3 + ID3V2_VERSION_SIZE_OF_BYTES + 1, SEEK_SET);
            id3ReaderWrite(stream, (id3buf)tmp, ID3V2_HEADER_SIZE_LEN);
            free(tmp);

            break;
        default:
            *len = 0;
            return NULL;
    }

    raw = calloc(sizeof(id3byte), headerSize);
    memcpy(raw, stream->buffer, headerSize);
    *len = headerSize;
    if(stream != NULL){
        id3FreeReader(stream);
    }

    return raw;
}

id3buf id3v2ExtendedHeaderToBuffer(unsigned int *len, Id3v2HeaderVersion version, Id3v2ExtHeader *ext){

    if(ext == NULL || version == ID3V22){
        *len = 0;
        return NULL;
    }

    if(ext->size <= 0){
        *len = 0;
        return NULL;
    }

    id3buf raw = NULL;
    char *tmp = NULL;
    unsigned int numExtFlags = 0;
    Id3Reader *stream = NULL;

    stream = id3NewReader(NULL, ext->size);

    switch(version){
        case ID3V23:
            
            //set size
            tmp = itob(ext->size);
            id3ReaderWrite(stream, (id3buf)tmp, ID3V23_SIZE_OF_SIZE_BYTES);
            free(tmp);

            //skip flags for now
            id3ReaderSeek(stream, ID3V2_FLAG_SIZE_OF_BYTES, SEEK_CUR);

            //size of padding
            tmp = itob(ext->padding);
            id3ReaderWrite(stream, (id3buf)tmp, ID3V23_SIZE_OF_SIZE_BYTES);
            free(tmp);

            //set crc
            if(ext->crc != NULL){
                id3ReaderWrite(stream, ext->crc, ext->crcLen);
                id3ReaderSeek(stream, ID3V23_SIZE_OF_SIZE_BYTES, SEEK_SET);
                id3ReaderCursor(stream)[0] ^= 1 << 7;
            }

            break;
        case ID3V24:

            //size of ext header
            tmp = itob(syncintEncode(ext->size));
            id3ReaderWrite(stream, (id3buf)tmp, ID3V23_SIZE_OF_SIZE_BYTES);
            free(tmp);

            //skip number of bytes
            id3ReaderSeek(stream, 1, SEEK_CUR);
            
            //set update flas
            if(ext->update > 0){
                id3ReaderCursor(stream)[0] ^= 1 << 6;
            }

            //set tag flag
            if(ext->tagSizeRestriction > 0 || ext->encodingRestriction == 1 || ext->textSizeRestriction > 0 || ext->imageEncodingRestriction == 1 || ext->imageSizeRestriction > 0){
                id3ReaderCursor(stream)[0] ^= 1 << 4;
                numExtFlags += 1;
            }

            //set crc
            if(ext->crc != NULL){
                id3ReaderCursor(stream)[0] ^= 1 << 5;

                //write crc
                id3ReaderSeek(stream, 1, SEEK_CUR);
                id3ReaderWrite(stream, ext->crc, ext->crcLen);
                numExtFlags += ext->crcLen;
            }
            
            
            //tag size restriction
            if(ext->tagSizeRestriction > 0){
                //only writable bits
                if(ext->tagSizeRestriction == 1){
                    id3ReaderCursor(stream)[0] ^= 1 << 6;
                }else if(ext->tagSizeRestriction == 2){
                    id3ReaderCursor(stream)[0] ^= 1 << 7;
                }else if(ext->tagSizeRestriction == 3){
                    id3ReaderCursor(stream)[0] ^= 1 << 6;
                    id3ReaderCursor(stream)[0] ^= 1 << 7;
                }
            }

            //tag encoding restriction
            if(ext->encodingRestriction == 1){
                id3ReaderCursor(stream)[0] ^= 1 << 5;
            }

            //text field size restriction
            if(ext->textSizeRestriction > 0){
                //only writable bits
                if(ext->textSizeRestriction == 1){
                    id3ReaderCursor(stream)[0] ^= 1 << 3;
                }else if(ext->textSizeRestriction == 2){
                    id3ReaderCursor(stream)[0] ^= 1 << 4;
                }else if(ext->textSizeRestriction == 3){
                    id3ReaderCursor(stream)[0] ^= 1 << 3;
                    id3ReaderCursor(stream)[0] ^= 1 << 4;
                }
            }

            //tag encoding restriction
            if(ext->imageEncodingRestriction == 1){
                id3ReaderCursor(stream)[0] ^= 1 << 2;
            }

            //text field size restriction
            if(ext->imageSizeRestriction > 0){
                //only writable bits
                if(ext->imageSizeRestriction == 1){
                    id3ReaderCursor(stream)[0] ^= 1 << 0;
                }else if(ext->imageSizeRestriction == 2){
                    id3ReaderCursor(stream)[0] ^= 1 << 1;
                }else if(ext->imageSizeRestriction == 3){
                    id3ReaderCursor(stream)[0] ^= 1 << 1;
                    id3ReaderCursor(stream)[0] ^= 1 << 0;
                }
            }
            
            //write number of bytes
            id3ReaderSeek(stream, ID3V23_SIZE_OF_SIZE_BYTES, SEEK_SET);
            id3ReaderWrite(stream, (id3buf)&numExtFlags, 1);
            
            break;
        default:
            *len = 0;
            id3FreeReader(stream);
            return NULL;
    }

    raw = calloc(sizeof(id3byte), ext->size);
    memcpy(raw, stream->buffer, ext->size);
    *len = ext->size;
    if(stream != NULL){
        id3FreeReader(stream);
    }

    return raw;

}

//build all frames as a buffer

id3buf id3v2FrameToBuffer(unsigned int *len, Id3v2HeaderVersion version, Id3v2Frame *frame){

    if(frame == NULL){
        *len = 0;
        return NULL;
    }

    if(frame->header == NULL){
        *len = 0;
        return NULL;
    }

    id3buf raw = NULL;

    //avoids involved persons 
    if(frame->header->id[0] == 'T' && frame->header->idNum != TIPL){
        raw = id3v2TextFrameToBuffer(len, version, frame);
    }else if(frame->header->id[0] == 'W'){
        raw = id3v2TextFrameToBuffer(len, version, frame);
    }else if(frame->header->idNum == IPL || frame->header->idNum == IPLS || frame->header->idNum == TIPL){
        raw = id3v2InvolvedPeopleListToBuffer(len, version, frame);
    }else if(frame->header->idNum == MCI || frame->header->idNum == MCDI){
        raw = id3v2MusicCDIdentifierToBuffer(len, version, frame);
    }

    return raw;
}

//frame header building

id3buf id3v2FrameFlagContentToBuffer(unsigned int *len, Id3v2HeaderVersion version, Id3v2FlagContent *flags){

    if(flags == NULL){
        *len = 0;
        return NULL;
    }
    
    if(!(version == ID3V23 || version == ID3V24)){
        *len = 0;
        return NULL;
    }
    
    id3buf raw = NULL;
    id3byte f2 = 0;
    unsigned int rawSize = 2; //min 2 bytes
    Id3Reader *stream = NULL;

    //calculate size
    rawSize += (flags->decompressedSize > 0) ? ZLIB_COMPRESSION_SIZE : 0;
    rawSize += (flags->encryption > 0) ? 1: 0;
    rawSize += (flags->grouping > 0) ? 1: 0;

    stream = id3NewReader(NULL, rawSize);

    //%abc00000 %ijk00000
    if(version == ID3V23){
        
        //set first byte flag bits
        if(flags->tagAlterPreservation == true){
            id3ReaderCursor(stream)[0] ^= 1 << 7;
        }

        if(flags->fileAlterPreservation == true){
            id3ReaderCursor(stream)[0] ^= 1 << 6;
        }

        if(flags->readOnly == true){
            id3ReaderCursor(stream)[0] ^= 1 << 5;
        }

        //set second byte flag bits
        id3ReaderSeek(stream, 2, SEEK_CUR); //skip second flag as f2 will be added later
        
        if(flags->decompressedSize > 0){
            char *tmp = itob(flags->decompressedSize);
            f2 ^= 1 << 7;
            id3ReaderWrite(stream, (id3buf)tmp, ZLIB_COMPRESSION_SIZE);
            free(tmp);
        }

        if(flags->encryption > 0){
            f2 ^= 1 << 6;
            id3ReaderWrite(stream, &(flags->encryption), 1);
        }

        if(flags->grouping > 0){
            f2 ^= 1 << 5;
            id3ReaderWrite(stream, &(flags->grouping), 1);
        }

        id3ReaderWriteAtPosition(stream, &f2, 1, 1);
    }

    //%0abc0000 %0h00kmnp
    if(version == ID3V24){
        //set first byte flag bits
        if(flags->tagAlterPreservation == true){
            id3ReaderCursor(stream)[0] ^= 1 << 6;
        }

        if(flags->fileAlterPreservation == true){
            id3ReaderCursor(stream)[0] ^= 1 << 5;
        }

        if(flags->readOnly == true){
            id3ReaderCursor(stream)[0] ^= 1 << 4;
        }

        //set second byte flag bits
        id3ReaderSeek(stream, 2, SEEK_CUR);
        if(flags->grouping > 0){
            id3ReaderWrite(stream, &(flags->grouping), 1);
            f2 ^= 1 << 6;
        }
        
        if(flags->decompressedSize > 0){
            char *tmp = itob(flags->decompressedSize);
            id3ReaderWrite(stream, (id3buf)tmp, ZLIB_COMPRESSION_SIZE);
            free(tmp);
            f2 ^= 1 << 3;
        }

        if(flags->encryption > 0){
            id3ReaderWrite(stream, &(flags->encryption), 1);
            f2 ^= 1 << 2;
        }

        if(flags->unsynchronization == true){
            f2 ^= 1 << 1;
        }
        
        if(flags->dataLengthIndicator == true){
            f2 ^= 1 << 0;
        }

        //write the second flag byte
        id3ReaderWriteAtPosition(stream, &f2, 1, 1);
    }

    //finish up
    raw = calloc(sizeof(id3byte), rawSize);
    memcpy(raw, stream->buffer, rawSize);
    id3FreeReader(stream);
    *len = rawSize;
    return raw;
}

id3buf id3v2FrameHeaderToBuffer(unsigned int *len, Id3v2HeaderVersion version, Id3v2FrameHeader *frameHeader){

    if(frameHeader == NULL){
        *len = 0;
        return NULL;
    }

    //at the moment only these will be supported 
    if(!(version == ID3V22 || version == ID3V23 || version == ID3V24)){
        *len = 0;
        return NULL;
    }

    id3buf raw = NULL;
    id3buf flags = NULL;
    char *tmp = NULL;
    unsigned int flagLen = 0;
    Id3Reader *stream = NULL;

    stream = id3NewReader(NULL, frameHeader->headerSize);

    if(version == ID3V22){
        
        //id
        id3ReaderWrite(stream, (id3buf)frameHeader->id, ID3V22_SIZE_OF_SIZE_BYTES);
        
        //frame size
        tmp = itob(frameHeader->frameSize);

        //tmp + 1 removes the first byte to match spec, only 3 bytes are used here
        id3ReaderWrite(stream, (id3buf)(tmp + 1), ID3V22_SIZE_OF_SIZE_BYTES);
        free(tmp);
    }

    //same size and will be built the same way
    if(version == ID3V23 || version == ID3V24){

        //id
        id3ReaderWrite(stream, (id3buf)frameHeader->id, ID3V23_SIZE_OF_SIZE_BYTES);

        //frame size
        if(version == ID3V24){
            tmp = itob((int) syncintEncode(frameHeader->frameSize)); 
        }else{
            tmp = itob((int) frameHeader->frameSize);
        }

        id3ReaderWrite(stream, (id3buf)tmp, ID3V23_SIZE_OF_SIZE_BYTES);
        free(tmp);

        //flags
        flags = id3v2FrameFlagContentToBuffer(&flagLen, version, frameHeader->flagContent);
        if(flags != NULL && flagLen > 0){
            id3ReaderWrite(stream, flags, flagLen);
            free(flags);
        }
    }

    raw = calloc(sizeof(id3byte), frameHeader->headerSize);
    memcpy(raw, stream->buffer, frameHeader->headerSize);
    *len = frameHeader->headerSize;
    if(stream != NULL){
        id3FreeReader(stream);
    }

    return raw;
}

//frames

id3buf id3v2TextFrameToBuffer(unsigned int *len, Id3v2HeaderVersion version, Id3v2Frame *frame){

    if(frame == NULL){
        *len = 0;
        return NULL;
    }

    if(frame->header == NULL || frame->frame == NULL){
        *len = 0;
        return NULL;
    }

    if(!id3v2IsValidVersion(version)){
        *len = 0;
        return NULL;
    }

    //none of these are version specific
    //i wont enforce encodings because i dont expect the caller to use
    //id3v2SetEncoding() unless they really need to and at that point...
    //well, they would probably want it to be written in that encoding

    id3buf raw = NULL;
    id3buf frameHeader = NULL;
    unsigned int frameHeaderLen = 0;
    unsigned int sz = frame->header->frameSize + frame->header->headerSize;
    Id3Reader *stream = NULL;
    Id3v2TextBody *body = NULL;

    stream = id3NewReader(NULL, sz);
    
    //frame header
    frameHeader = id3v2FrameHeaderToBuffer(&frameHeaderLen, version, frame->header);
    
    if(frameHeader != NULL && frameHeaderLen > 0){
        id3ReaderWrite(stream, frameHeader, frameHeaderLen);
        free(frameHeader);
    }else{
        *len = 0;
        return NULL;
    }

    body = (Id3v2TextBody *)frame->frame;

    //write encoding
    id3ReaderWrite(stream, &(body->encoding), 1);

    //write a description if it exists
    if((frame->header->idNum == TXX || frame->header->idNum == TXXX) && body->description != NULL){
        id3ReaderWrite(stream, body->description, id3strlen(body->description, body->encoding) - modStrLen(body->encoding));
        
        //padding
        if(body->encoding == UTF16 || body->encoding == UTF16BE){
            id3ReaderSeek(stream, 2, SEEK_CUR);
        }else{
            id3ReaderSeek(stream, 1, SEEK_CUR);
        }

    }

    //write text
    id3ReaderWrite(stream, body->value, id3strlen(body->value, body->encoding) - modStrLen(body->encoding));

    //finish up
    raw = calloc(sizeof(id3byte), sz);
    memcpy(raw, stream->buffer, sz);
    *len = sz;
    id3FreeReader(stream);

    return raw;
}

id3buf id3v2URLFrameToBuffer(unsigned int *len, Id3v2HeaderVersion version, Id3v2Frame *frame){

    if(frame == NULL){
        *len = 0;
        return NULL;
    }

    if(frame->header == NULL || frame->frame == NULL){
        *len = 0;
        return NULL;
    }

    if(!id3v2IsValidVersion(version)){
        *len = 0;
        return NULL;
    }

    id3buf raw = NULL;
    id3buf frameHeader = NULL;
    unsigned int frameHeaderLen = 0;
    unsigned int sz = frame->header->frameSize + frame->header->headerSize;
    Id3Reader *stream = NULL;
    Id3v2URLBody *body = NULL;

    stream = id3NewReader(NULL, sz);
    body = (Id3v2URLBody *)frame->frame;

    //frame header
    frameHeader = id3v2FrameHeaderToBuffer(&frameHeaderLen, version, frame->header);
    
    if(frameHeader != NULL && frameHeaderLen > 0){
        id3ReaderWrite(stream, frameHeader, frameHeaderLen);
        free(frameHeader);
    }else{
        *len = 0;
        return NULL;
    }

    //write a description if it exists
    if(body->description != NULL){
        id3ReaderWrite(stream, body->description, id3strlen(body->description, body->encoding) - modStrLen(body->encoding));
        
        //padding
        if(body->encoding == UTF16 || body->encoding == UTF16BE){
            id3ReaderSeek(stream, 2, SEEK_CUR);
        }else{
            id3ReaderSeek(stream, 1, SEEK_CUR);
        }
    }

    //url
    id3ReaderWrite(stream, body->url, id3strlen(body->url, ISO_8859_1));

    //finish up
    raw = calloc(sizeof(id3byte), sz);
    memcpy(raw, stream->buffer, sz);
    *len = sz;
    id3FreeReader(stream);

    return raw;
}

id3buf id3v2InvolvedPeopleListToBuffer(unsigned int *len, Id3v2HeaderVersion version, Id3v2Frame *frame){

    if(frame == NULL){
        *len = 0;
        return NULL;
    }

    if(frame->header == NULL || frame->frame == NULL){
        *len = 0;
        return NULL;
    }

    if(!id3v2IsValidVersion(version)){
        *len = 0;
        return NULL;
    }

    id3buf raw = NULL;
    id3buf frameHeader = NULL;
    unsigned int frameHeaderLen = 0;
    unsigned int sz = frame->header->frameSize + frame->header->headerSize;
    Id3Reader *stream = NULL;
    Id3v2InvolvedPeopleListBody *body = NULL;
    Id3ListIter *iter = NULL;
    Id3v2InvolvedPerson *iPerson = NULL;
    
    stream = id3NewReader(NULL, sz);
    body = (Id3v2InvolvedPeopleListBody *)frame->frame;

    //frame header
    frameHeader = id3v2FrameHeaderToBuffer(&frameHeaderLen, version, frame->header);
    
    if(frameHeader != NULL && frameHeaderLen > 0){
        id3ReaderWrite(stream, frameHeader, frameHeaderLen);
        free(frameHeader);
    }else{
        *len = 0;
        return NULL;
    }

    //write encoding
    id3ReaderWrite(stream, &(body->encoding), 1);

    //write people and jobs
    if((iter = id3NewListIter(body->involvedPeople)) != NULL){
        
        while((iPerson = id3NextListIter(iter)) != NULL){
            
            //write person
            if(iPerson->person != NULL){
                id3ReaderWrite(stream, iPerson->person, iPerson->personLen);
            }
            id3ReaderSeek(stream, ((body->encoding == UTF16 || body->encoding == UTF16BE) ? 1: 2), SEEK_CUR);

            //write job
            if(iPerson->job != NULL){
                id3ReaderWrite(stream, iPerson->job, iPerson->jobLen);
            }

            //this is just to avoid a buffer overflow and match std
            if(id3HasNextListIter(iter)){
                id3ReaderSeek(stream, ((body->encoding == UTF16 || body->encoding == UTF16BE) ? 1: 2), SEEK_CUR);
            }
        }

        id3FreeListIter(iter);
    }

    
    //finish up
    raw = calloc(sizeof(id3byte), sz);
    memcpy(raw, stream->buffer, sz);
    *len = sz;
    id3FreeReader(stream);

    return raw;
}

id3buf id3v2MusicCDIdentifierToBuffer(unsigned int *len, Id3v2HeaderVersion version, Id3v2Frame *frame){

    if(frame == NULL){
        *len = 0;
        return NULL;
    }

    if(frame->header == NULL || frame->frame == NULL){
        *len = 0;
        return NULL;
    }

    if(!id3v2IsValidVersion(version)){
        *len = 0;
        return NULL;
    }

    id3buf raw = NULL;
    id3buf frameHeader = NULL;
    unsigned int frameHeaderLen = 0;
    unsigned int sz = frame->header->frameSize + frame->header->headerSize;
    Id3Reader *stream = NULL;
    Id3v2MusicCDIdentifierBody *body = NULL;
    
    stream = id3NewReader(NULL, sz);
    body = (Id3v2MusicCDIdentifierBody *)frame->frame;

    //frame header
    frameHeader = id3v2FrameHeaderToBuffer(&frameHeaderLen, version, frame->header);
    
    if(frameHeader != NULL && frameHeaderLen > 0){
        id3ReaderWrite(stream, frameHeader, frameHeaderLen);
        free(frameHeader);
    }else{
        *len = 0;
        return NULL;
    }

    id3ReaderWrite(stream, body->cdtoc, ISO_8859_1);

    //finish up
    raw = calloc(sizeof(id3byte), sz);
    memcpy(raw, stream->buffer, sz);
    *len = sz;
    id3FreeReader(stream);

    return raw;

}

