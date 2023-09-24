#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v1.h"
#include "byteStream.h"
#include "byteInt.h"


Id3v1Tag *id3v1TagFromFile(const char *filePath){

    if(filePath == NULL){
        return NULL;
    }

    FILE *fp = NULL;
    uint8_t id3Bytes[ID3V1_MAX_SIZE];
    
    //make sure the file can really be read
    if((fp = fopen(filePath, "rb")) == NULL){
        return NULL;
    }

    //seek to the start of metadata
    if((fseek(fp, -ID3V1_MAX_SIZE, SEEK_END)) != 0){
        fclose(fp);
        return NULL;
    }

    if((fread(id3Bytes, ID3V1_MAX_SIZE, 1, fp)) != 1){
        fclose(fp);
        return NULL;
    }

    fclose(fp);    
    return id3v1TagFromBuffer(id3Bytes);
}

Id3v1Tag *id3v1CopyTag(Id3v1Tag *toCopy){
    
    if(toCopy == NULL){
        return NULL;
    }

    return id3v1NewTag(toCopy->title, toCopy->artist, toCopy->albumTitle, toCopy->year, toCopy->track, toCopy->comment, toCopy->genre);
}

int _id3v1CharsToStructUint8(char *src, uint8_t *dest){

    memset(dest, 0, ID3V1_FIELD_SIZE);
    
    if(src != NULL){
        int len = strlen(src);
        int wLen = ((len > ID3V1_FIELD_SIZE) ? ID3V1_FIELD_SIZE : len);

        memcpy(dest, (uint8_t *)src, wLen);    
    }
    
    return 1;

}

int id3v1WriteTitle(char *title, Id3v1Tag *tag){
    return (tag == NULL) ? 0: _id3v1CharsToStructUint8(title, tag->title);
}

int id3v1WriteArtist(char *artist, Id3v1Tag *tag){
    return (tag == NULL) ? 0: _id3v1CharsToStructUint8(artist, tag->artist);
}

int id3v1WriteAlbum(char *album, Id3v1Tag *tag){
    return (tag == NULL) ? 0: _id3v1CharsToStructUint8(album, tag->albumTitle);
}

int id3v1WriteYear(int year, Id3v1Tag *tag){
    
    if(tag == NULL){
        return 0;
    }

    tag->year = year;
    return 1;
}

int id3v1WriteComment(char *comment, Id3v1Tag *tag){
    return (tag == NULL) ? 0: _id3v1CharsToStructUint8(comment, tag->comment);
}

int id3v1WriteGenre(Genre genre, Id3v1Tag *tag){
    
    if(tag == NULL){
        return 0;
    }

    tag->genre = genre;
    return 1;
}

int id3v1WriteTrack(int track, Id3v1Tag *tag){
    
    if(tag == NULL){
        return 0;
    }

    tag->track = track;
    return 1;

}

bool id3v1CompareTag(Id3v1Tag *tag1, Id3v1Tag *tag2);

char *id3v1ReadTitle(Id3v1Tag *tag);
char *id3v1ReadArtist(Id3v1Tag *tag);
char *id3v1ReadAlbum(Id3v1Tag *tag);
int id3v1ReadYear(Id3v1Tag *tag);
char *id3v1ReadComment(Id3v1Tag *tag);
Genre id3v1ReadGenre(Id3v1Tag *tag);
int id3v1ReadTrack(Id3v1Tag *tag);

char *id3v1ToJSON(const Id3v1Tag *tag);

void id3v1WriteTagToFile(const char *filePath, Id3v1Tag *tag){
/*
    if(filePath == NULL){
        return;
    }

    if(tag == NULL){
        return;
    }

    FILE *fp = NULL;
    ByteStream *stream = byteStreamCreate(NULL, ID3V1_MAX_SIZE);

    byteStreamWrite(stream, (unsigned char *)"TAG", ID3V1_TAG_ID_SIZE);
    byteStreamWrite(stream, (unsigned char *)tag->title, ID3V1_FIELD_SIZE);
    byteStreamWrite(stream, (unsigned char *)tag->artist, ID3V1_FIELD_SIZE);
    byteStreamWrite(stream, (unsigned char *)tag->albumTitle, ID3V1_FIELD_SIZE);
    byteStreamWrite(stream, (unsigned char *)tag->artist, ID3V1_FIELD_SIZE);
    byteStreamWrite(stream, (unsigned char *)itob(tag->year), ID3V1_YEAR_SIZE);
    byteStreamWrite(stream, (unsigned char *)tag->comment, ID3V1_FIELD_SIZE);
    byteStreamWrite(stream, (unsigned char *)itob(tag->track), 1);
    byteStreamWrite(stream, (unsigned char *)itob((int)tag->genre), 1);


    if((fp = fopen(filePath, "r+b")) == NULL){
        //create a new file and write the bytes to it    
        if((fp = fopen(filePath, "wb")) == NULL){
            byteStreamDestroy(stream);
            return;
        }
        
        if((fwrite(byteStreamCursor(stream), 1, ID3V1_MAX_SIZE, fp)) == 0){
            byteStreamDestroy(stream);
            fclose(fp);
            return;
        }

    }else{
        //file size
        fseek(fp,0, SEEK_END);
        int index = ftell(fp);
        uint8_t isTag[ID3V1_TAG_ID_SIZE];

        if((fseek(fp, index - ID3V1_MAX_SIZE, SEEK_SET)) != 0){
            fclose(fp);
            return;
        }
        
        //check to see if the file has ID3 metadata
        if(fread(isTag, 1, ID3V1_TAG_ID_SIZE, fp) == 0){
            fclose(fp);
            return;
        }

        //coupld not find 'TAG'
        if(!containsId3v1(isTag)){
            if(fseek(fp, 0, SEEK_END) != 0){
                fclose(fp);
                return;
            }

        }else{
            if(fseek(fp, index - ID3V1_MAX_SIZE, SEEK_SET) != 0){
                fclose(fp);
                return;
            }
        }

        //write bytes to the file at the right position
        if((fwrite(byteStreamCursor(stream), 1, ID3V1_MAX_SIZE, fp)) == 0){
            fclose(fp);
            return;
        }
    }

    byteStreamDestroy(stream);
*/
}