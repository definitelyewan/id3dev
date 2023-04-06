#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v1Manipulate.h"
#include "id3Helpers.h"
#include "id3Defines.h"

/*
    setters
*/

void id3v1SetTitle(id3buf title, Id3v1Tag *tag){

    if(tag == NULL){
        return;
    }

    if(title == NULL){
        return;
    }

    size_t len = strlen((char *)title);

    id3buf tmp = calloc(sizeof(id3byte), ID3V1_TAG_LEN + 1);

    if(len > ID3V1_TAG_LEN){
        memcpy(tmp, title, ID3V1_TAG_LEN);

    }else{
        memcpy(tmp, title, len);
    }

    free(tag->title);
    tag->title = tmp;
    
}

void id3v1SetArtist(id3buf artist, Id3v1Tag *tag){
    
    if(tag == NULL){
        return;
    }

    if(artist == NULL){
        return;
    }

    size_t len = strlen((char *)artist);

    id3buf tmp = calloc(sizeof(id3byte), ID3V1_TAG_LEN + 1);

    if(len > ID3V1_TAG_LEN){
        memcpy(tmp, artist, ID3V1_TAG_LEN);

    }else{
        memcpy(tmp, artist, len);
    }

    free(tag->artist);
    tag->artist = tmp;
}

void id3v1SetAlbum(id3buf album, Id3v1Tag *tag){

    if(tag == NULL){
        return;
    }

    if(album == NULL){
        return;
    }

    size_t len = strlen((char *)album);

    id3buf tmp = calloc(sizeof(id3byte), ID3V1_TAG_LEN + 1);

    if(len > ID3V1_TAG_LEN){
        memcpy(tmp, album, ID3V1_TAG_LEN);

    }else{
        memcpy(tmp, album, len);
    }

    free(tag->albumTitle);
    tag->albumTitle = tmp;
}

void id3v1SetYear(int year, Id3v1Tag *tag){

    if(tag == NULL){
        return;
    }

    tag->year = year;
}

void id3v1SetComment(id3buf comment, Id3v1Tag *tag){
    
    if(tag == NULL){
        return;
    }

    if(comment == NULL){
        return;
    }   

    size_t len = strlen((char *)comment);

    id3buf tmp = calloc(sizeof(id3byte), ID3V1_TAG_LEN + 1);

    if(len > ID3V1_TAG_LEN){
        memcpy(tmp, comment, ID3V1_TAG_LEN);

    }else{
        memcpy(tmp, comment, len);
    }

    free(tag->comment);
    tag->comment = tmp;  
}

void id3v1SetGenre(Genre genre, Id3v1Tag *tag){

    if(tag == NULL){
        return;
    }
    
    tag->genre = genre;
}

void id3v1SetTrack(int track, Id3v1Tag *tag){

    if(tag == NULL){
        return;
    }

    tag->trackNumber = track;
}

/*
    edit functions
*/

void id3v1ClearTagInformation(Id3v1Tag *tag){
    
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
    tag->trackNumber = 0;
    tag->year = 0;
}

bool id3v1CompareTag(Id3v1Tag *tag1, Id3v1Tag *tag2){

    if(tag1 == NULL){
        return false;
    }

    if(tag2 == NULL){
        return false;
    }

    if(tag1->genre != tag2->genre){
        return false;
    }

    if(tag1->trackNumber != tag2->trackNumber){
        return false;
    }

    if(tag1->year != tag2->year){
        return false;
    }

    if(strcmp((char *)tag1->albumTitle, (char *)tag2->albumTitle) != 0){
        return false;
    }

    if(strcmp((char *)tag1->artist, (char *)tag2->artist) != 0){
        return false;
    }

    if(strcmp((char *)tag1->comment, (char *)tag2->comment) != 0){
        return false;
    }

    if(strcmp((char *)tag1->title, (char *)tag2->title) != 0){
        return false;
    }

    return true;

}

/*
    compatability getters
*/

id3buf id3v1GetTitle(Id3v1Tag *tag){
    if(tag == NULL){
        return NULL;
    }

    if(tag->title == NULL){
        return NULL;
    }

    id3buf value = calloc(sizeof(id3byte), strlen((char *)tag->title) + 1);
    memcpy(value, tag->title, strlen((char *)tag->title));
    return value;
}

id3buf id3v1GetArtist(Id3v1Tag *tag){
    if(tag == NULL){
        return NULL;
    }

    if(tag->artist == NULL){
        return NULL;
    }

    id3buf value = calloc(sizeof(id3byte), strlen((char *)tag->artist) + 1);
    memcpy(value, tag->artist, strlen((char *)tag->artist));
    return value;
}

id3buf id3v1GetAlbum(Id3v1Tag *tag){
    if(tag == NULL){
        return NULL;
    }

    if(tag->albumTitle == NULL){
        return NULL;
    }

    id3buf value = calloc(sizeof(id3byte), strlen((char *)tag->albumTitle) + 1);
    memcpy(value, tag->albumTitle, strlen((char *)tag->albumTitle));
    return value;
}

int id3v1GetYear(Id3v1Tag *tag){
    return (tag == NULL) ? 0 : tag->year;
}

id3buf id3v1GetComment(Id3v1Tag *tag){
    if(tag == NULL){
        return NULL;
    }

    if(tag->comment == NULL){
        return NULL;
    }

    id3buf value = calloc(sizeof(id3byte), strlen((char *)tag->comment) + 1);
    memcpy(value, tag->comment, strlen((char *)tag->comment));
    return value;
}

Genre id3v1GetGenre(Id3v1Tag *tag){
    return (tag == NULL) ? OTHER_GENRE : tag->genre;
}
int id3v1GetTrack(Id3v1Tag *tag){
    return (tag == NULL) ? 0 : tag->trackNumber;
}
