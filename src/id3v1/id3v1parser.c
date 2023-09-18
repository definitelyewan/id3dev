#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v1Types.h"
#include "id3v1Parser.h"
#include "byteStream.h"

bool id3v1HasTag(unsigned char *buffer){
    return (memcmp(buffer,"TAG",ID3V1_TAG_ID_SIZE) == 0) ? true: false;
}

Id3v1Tag *id3v1NewTag(uint8_t *title, uint8_t *artist, uint8_t *albumTitle, int year, int track, uint8_t *comment, Genre genre){

    Id3v1Tag *tag = malloc(sizeof(Id3v1Tag));
    
    tag->title = title;
    tag->artist = artist;
    tag->albumTitle = albumTitle;
    tag->year = year;
    tag->track = track;
    tag->comment = comment;
    tag->genre = genre;

    return tag;
}

void id3v1FreeTag(Id3v1Tag *tag){
    
    if(tag == NULL){
        return;
    }

    if(tag->albumTitle != NULL){
        free(tag->albumTitle);
        tag->albumTitle = NULL;
    }

    if(tag->artist != NULL){
        free(tag->artist);
        tag->artist = NULL;
    }

    if(tag->comment != NULL){
        free(tag->comment);
        tag->comment = NULL;
    }

    if(tag->title != NULL){
        free(tag->title);
        tag->title = NULL;
    }

    tag->genre = OTHER_GENRE;
    tag->track = 0;
    tag->year = 0;
}

void id3v1DestroyTag(void *toDelete){
    
    Id3v1Tag *toFree = (Id3v1Tag *)toDelete;

    //error checking until frees 
    if(toFree){
        id3v1FreeTag(toFree);
        
        free(toFree);
        toFree = NULL;
    }
}

Id3v1Tag *id3v1TagFromBuffer(unsigned char *buffer){

    int trackno = 0;
    uint8_t *holdTitle = NULL;
    uint8_t *holdArtist = NULL;
    uint8_t *holdAlbum = NULL;
    uint8_t *holdComment = NULL;
    unsigned char year[ID3V1_YEAR_SIZE];
    Genre genre;
/*
    Id3Reader *stream = id3NewReader(buffer, ID3V1_MAX_BYTES);

    if(strncmp((char *)id3ReaderCursor(stream), "TAG", ID3V1_ID_LEN)){
        id3FreeReader(stream);
        return NULL;
    }
    id3ReaderSeek(stream, ID3V1_ID_LEN, SEEK_CUR);
    
    //get song title and set index for next tag
    holdTitle = calloc(sizeof(id3byte), ID3V1_TAG_LEN + 1);    
    id3ReaderRead(stream, holdTitle, ID3V1_TAG_LEN);
    
    //get artist and set index for next tag
    holdArtist = calloc(sizeof(id3byte), ID3V1_TAG_LEN + 1);
    id3ReaderRead(stream, holdArtist, ID3V1_TAG_LEN);


    //get album title and set index for next tag
    holdAlbum = calloc(sizeof(id3byte), ID3V1_TAG_LEN + 1);
    id3ReaderRead(stream, holdAlbum, ID3V1_TAG_LEN);

    //get year and set index for next tag
    strncpy((char *)year, (char *)id3ReaderCursor(stream), ID3V1_YEAR_LEN);
    year[4] = '\0';
    id3ReaderSeek(stream, ID3V1_YEAR_LEN, SEEK_CUR);

    //check for a track number, ID3V1.1 has the 28th bit nulled so that the 29th can be a track number
    id3ReaderSeek(stream, ID3V1_TAG_LEN - 2, SEEK_CUR);
    if(!id3ReaderCursor(stream)[0] && id3ReaderCursor(stream)[1]){
        trackno = 1;
    }
    id3ReaderSeek(stream, -(ID3V1_TAG_LEN - 2), SEEK_CUR);

    //get comment and set index for next tag
    holdComment = calloc(sizeof(id3byte), ID3V1_TAG_LEN + 1);
    id3ReaderRead(stream, holdComment, ID3V1_TAG_LEN - trackno);

    //read and set track number + move index
    if(trackno){
        trackno = (int)id3ReaderGetCh(stream);
    }else{
        trackno = 0;
    }
    
    id3ReaderSeek(stream, 1, SEEK_CUR);
    genre = id3ReaderGetCh(stream);

    id3FreeReader(stream);
*/
    return id3v1NewTag(holdTitle, holdArtist, holdAlbum, atoi((char *)year), trackno, holdComment, genre);
}