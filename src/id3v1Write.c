#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v1Write.h"
#include "id3Helpers.h"
#include "id3v1.h"


bool containsId3v1(unsigned char *buffer){
    return (memcmp(buffer,"TAG",ID3V1_ID_LEN) == 0) ? true: false;
}

void id3v1WriteTag(const char *filePath, Id3v1Tag *tag){

    if(filePath == NULL){
        return;
    }

    if(tag == NULL){
        return;
    }

    id3byte isTag[ID3V1_ID_LEN+1];
    id3byte bytes[ID3V1_MAX_BYTES+1];
    int index = 0;
    FILE *fp = NULL;
    
    //create a buffer to write
    for(int i = 0; i < ID3V1_MAX_BYTES; i++){
        bytes[i] = 0x00;
    }

    memcpy(bytes,"TAG",ID3V1_ID_LEN);
    index = index + ID3V1_ID_LEN;

    if(tag->title != NULL){
        memcpy(bytes + index, tag->title, strlen((char *)tag->title));
    }
    index = index + ID3V1_TAG_LEN;

    if(tag->artist != NULL){
        memcpy(bytes + index, tag->artist, strlen((char *)tag->artist));
    }
    index = index + ID3V1_TAG_LEN;

    if(tag->albumTitle != NULL){
        memcpy(bytes + index, tag->albumTitle, strlen((char *)tag->albumTitle));
    }
    index = index + ID3V1_TAG_LEN;

    if(tag->year != 0){
        char *num = integerToCharPointer(tag->year);
        memcpy(bytes + index, num, ID3V1_YEAR_LEN);
        free(num);
    }
    index = index + ID3V1_YEAR_LEN;

    if(tag->comment != NULL){
        memcpy(bytes + index, tag->comment, strlen((char *)tag->comment));
    }

    if(tag->trackNumber > 0 && tag->trackNumber <= 255){
        index = index + ID3V1_TAG_LEN - 1;
        memcpy(bytes+index, &tag->trackNumber, 1);
        index = index + 1;
    }else{
        index = index + ID3V1_TAG_LEN;
    }

    bytes[ID3V1_MAX_BYTES-1] = tag->genre;

    if((fp = fopen(filePath, "r+b")) == NULL){
        //create a new file and write the bytes to it    
        if((fp = fopen(filePath, "wb")) == NULL){
            return;
        }
        
        if((fwrite(bytes, sizeof(id3byte),ID3V1_MAX_BYTES, fp)) == 0){
            fclose(fp);
            return;
        }

    }else{
        
        //file size
        fseek(fp,0, SEEK_END);
        index = ftell(fp);
    
        if((fseek(fp, index - ID3V1_MAX_BYTES, SEEK_SET)) != 0){
            fclose(fp);
            return;
        }
        
        //check to see if the file has ID3 metadata
        if(fread(isTag, sizeof(id3byte), ID3V1_ID_LEN, fp) == 0){
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
            if(fseek(fp, index - ID3V1_MAX_BYTES, SEEK_SET) != 0){
                fclose(fp);
                return;
            }
        }

        //write bytes to the file at the right position
        if((fwrite(bytes, sizeof(id3byte), ID3V1_MAX_BYTES, fp)) == 0){
            fclose(fp);
            return;
        }

    }

    fclose(fp);
}


//if any value is null this fails :(
char *id3v1ToJSON(const Id3v1Tag *tag){

    char *jsonStr = NULL;

    int memCount = 0;
    if(tag == NULL){
        jsonStr = calloc(sizeof(char), 3);
        strncpy(jsonStr,"{}",3);
        return jsonStr;
    }

    memCount = 2/*{}*/ + 1/*\0*/ + 28/*""*8 - the ints*/ + 8/*:*8*/ + 1/*,*7*/;
    memCount = memCount + strlen("title");
    memCount = memCount + strlen("artist");
    memCount = memCount + strlen("album");
    memCount = memCount + strlen("year");
    memCount = memCount + strlen("track");
    memCount = memCount + strlen("comment");
    memCount = memCount + strlen("genreNumber");
    memCount = memCount + strlen("genre");
    memCount = memCount + strlen((char *)tag->title);
    memCount = memCount + strlen((char *)tag->artist);
    memCount = memCount + strlen((char *)tag->albumTitle);
    memCount = memCount + strlen((char *)tag->comment);
    memCount = memCount + strlen(genreFromTable(tag->genre));
    memCount = memCount + (sizeof(int) * 3); /*year, track & genre*/
    
    jsonStr = calloc(sizeof(char), memCount);

    sprintf(jsonStr,"{\"title\":\"%s\",\"artist\":\"%s\",\"album\":%s,\"year\":%d,\"track\":%d,\"comment\":%s,\"genreNumber\":%d,\"genre\":%s}",
    (char *) tag->title, 
    (char *) tag->artist,
    (char *) tag->albumTitle, 
    tag->year,
    tag->trackNumber, 
    (char *) tag->comment,
    tag->genre, 
    genreFromTable(tag->genre));
    return jsonStr;

}