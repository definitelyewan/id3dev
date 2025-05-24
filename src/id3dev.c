/**
 * @file id3dev.c
 * @author Ewan Jones
 * @brief Main functions for interacting with ID3 metadata
 * @version 0.1
 * @date 2024-04-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3dev.h"
#include "id3v2/id3v2.h"
#include "id3v1/id3v1.h"
#include "id3v1/id3v1Parser.h"
#include "id3v2/id3v2Frame.h"

/**
 * @brief default standard for reading ID3 tags from a data structure representation.
 * 
 */
static uint8_t id3PreferredStandard = ID3V2_TAG_VERSION_3;

/**
 * @brief Creates a new ID3 structure with the given ID3v2 and ID3v1 tags.
 * 
 * @param id3v2 
 * @param id3v1 
 * @return ID3* 
 */
ID3 *id3Create(Id3v2Tag *id3v2, Id3v1Tag *id3v1){

    ID3 *metadata = malloc(sizeof(ID3));

    metadata->id3v2 = id3v2;
    metadata->id3v1 = id3v1;

    return metadata;

}

/**
 * @brief Destroys an ID3 structure and frees its memory.
 * 
 * @param toDelete 
 */
void id3Destroy(ID3 **toDelete){

    if(*toDelete){
        id3v2DestroyTag(&((*toDelete)->id3v2));
        id3v1DestroyTag(&((*toDelete)->id3v1));
        free(*toDelete);
        *toDelete = NULL;
        toDelete = NULL;
    }

}

/**
 * @brief Sets the preferred standard for reading ID3 tags from a data structure representation. If no standard is
 * set the default will be set to ID3v2.3 as this is the widest spread version with the most features. If
 * successful this function returns true otherwise, it will return false.
 * 
 * @param standard 
 * @return true 
 * @return false 
 */
bool id3SetPreferredStandard(uint8_t standard){

    switch(standard){
        case ID3V1_TAG_VERSION:
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            id3PreferredStandard = standard;
            break;
        
        default:
            return false;
    }

    return true;
}

/**
 * @brief Returns the current preferred standard
 * 
 * @return uint8_t 
 */
uint8_t id3GetPreferredStandard(void){
    return id3PreferredStandard;
}


/**
 * @brief Reads both ID3v1 and ID3v2 tags located in a provided file. If this function fails an
 * ID3 structure will be returned but one or both versions will be set to NULL.
 * 
 * @param filePath 
 * @return ID3* 
 */
ID3 *id3FromFile(const char *filePath){
    return id3Create(id3v2TagFromFile(filePath), id3v1TagFromFile(filePath));
}

/**
 * @brief Returns a copy of the given ID3 structure. If NUll is passed NULL is returned.
 * 
 * @param toCopy 
 * @return ID3* 
 */
ID3 *id3Copy(ID3 *toCopy){

    if(toCopy == NULL){
        return NULL;
    }

    return id3Create(id3v2CopyTag(toCopy->id3v2), id3v1CopyTag(toCopy->id3v1));
}

/**
 * @brief Compares two ID3 structures. If they are the same this function will return true otherwise it will return false.
 * 
 * @param metadata1 
 * @param metadata2 
 * @return true 
 * @return false 
 */
bool id3Compare(ID3 *metadata1, ID3 *metadata2){

    if(metadata1 == NULL || metadata2 == NULL){
        return false;
    }

    bool v1 = false;
    bool v2 = false;

    v1 = id3v1CompareTag(metadata1->id3v1, metadata2->id3v1);
    v2 = id3v2CompareTag(metadata1->id3v2, metadata2->id3v2);

    if(v1 && v2){
        return true;
    }

    if(v1 == false && (v2 == true && metadata1->id3v1 == NULL && metadata2->id3v1 == NULL)){
        return true;

    }

    if(v2 == false && (v1 == true && metadata1->id3v2 == NULL && metadata2->id3v2 == NULL)){
        return true;

    }

    return false;
}


/**
 * @brief Converts the stored ID3v1 tag to an ID3v2 tag to be held in the ID3 structure. The preferred standard will
 * be used to infer what version of ID3v2 is to be used but, if the standard is ID3V1 this function will fail. However, 
 * if there is a stored ID3v2 tag it will be removed and replaced. On success this function will return true, otherwise 
 * it will return false. 
 * 
 * @param metadata 
 * @return true 
 * @return false 
 */
bool id3ConvertId3v1ToId3v2(ID3 *metadata){

    if(metadata == NULL){
        return false;
    }

    if(metadata->id3v1 == NULL){
        return false;
    }

    Id3v2Tag *newTag = NULL;
    Id3v2TagHeader *header = NULL;
    List *frames = NULL;
    char *str = NULL;
    int size = 0;
    int input = 0;

    input = id3GetPreferredStandard() > ID3V1_TAG_VERSION ? id3GetPreferredStandard() : ID3V2_TAG_VERSION_3;

    switch(input){
        case ID3V2_TAG_VERSION_2:
            header = id3v2CreateTagHeader(ID3V2_TAG_VERSION_2, 0, 0, NULL);
            break;
        case ID3V2_TAG_VERSION_3:
            header = id3v2CreateTagHeader(ID3V2_TAG_VERSION_3, 0, 0, NULL);
            break;
        case ID3V2_TAG_VERSION_4:
            header = id3v2CreateTagHeader(ID3V2_TAG_VERSION_4, 0, 0, NULL);
            break;
        default:
            return false;
    }


    frames = listCreate(id3v2PrintFrame, id3v2DeleteFrame, id3v2CompareFrame, id3v2CopyFrame);
    newTag = id3v2CreateTag(header, frames);

    if(metadata->id3v1->title[0] != 0x00){
        if(!id3v2WriteTitle((char *)metadata->id3v1->title, newTag)){
            id3v2DestroyTag(&newTag);
            listFree(frames);
            return false;
        }
    }

    

    if(metadata->id3v1->artist[0] != 0x00){
        if(!id3v2WriteArtist((char *)metadata->id3v1->artist, newTag)){
            id3v2DestroyTag(&newTag);
            listFree(frames);
            return false;
        }
    }

    

    if(metadata->id3v1->albumTitle[0] != 0x00){
        if(!id3v2WriteAlbum((char *)metadata->id3v1->albumTitle, newTag)){
            id3v2DestroyTag(&newTag);
            listFree(frames);
            return false;
        }
    }
    
    

    if(metadata->id3v1->year != 0){
        size = snprintf(NULL, 0, "%d", metadata->id3v1->year);
        str = calloc(size + 1, sizeof(char));
        snprintf(str, size + 1, "%d", metadata->id3v1->year);
        if(!id3v2WriteYear((char *)str, newTag)){
            id3v2DestroyTag(&newTag);
            listFree(frames);
            free(str);
            return false;
        }
        free(str);
    }

    

    if(metadata->id3v1->track != 0){
        size = snprintf(NULL, 0, "%d", metadata->id3v1->track);
        str = calloc(size + 1, sizeof(char));
        snprintf(str, size + 1, "%d", metadata->id3v1->track);

        if(!id3v2WriteTrack((char *)str, newTag)){
            id3v2DestroyTag(&newTag);
            listFree(frames);
            free(str);
            return false;
        }
        free(str);
    }

    if(metadata->id3v1->genre < PSYBIENT_GENRE){
        if(!id3v2WriteGenre(id3v1GenreFromTable(metadata->id3v1->genre), newTag)){
            id3v2DestroyTag(&newTag);
            listFree(frames);
            return false;
        }
    }

    

    if(metadata->id3v1->comment[0] != 0x00){
        if(!id3v2WriteComment((char *)metadata->id3v1->comment, newTag)){
            id3v2DestroyTag(&newTag);
            listFree(frames);
            return false;
        }
    }

    if(metadata->id3v2 != NULL){
        id3v2DestroyTag(&(metadata->id3v2));
    }

    metadata->id3v2 = newTag;
    
    return true;
}


/**
 * @brief Converts the stored ID3v2 tag to an ID3v1 tag to be held in the ID3 structure. However, if there
 * is a stored ID3v1 tag it will be removed and replaced. On success this function will return true,
 * otherwise it will return false. 
 * 
 * @param metadata 
 * @return true 
 * @return false 
 */
bool id3ConvertId3v2ToId3v1(ID3 *metadata){

    if(metadata == NULL){
        return false;
    }

    if(metadata->id3v2 == NULL){
        return false;
    }

    Id3v1Tag *newTag = NULL;
    char *title = NULL;
    char *artist = NULL;
    char *album = NULL;
    char *year = NULL;
    char *track = NULL;
    char *comment = NULL;
    char *genre = NULL;

    newTag = id3v1CreateTag(NULL, NULL, NULL, 0, 0, NULL, OTHER_GENRE);
    title = id3v2ReadTitle(metadata->id3v2);
    artist = id3v2ReadArtist(metadata->id3v2);
    album = id3v2ReadAlbum(metadata->id3v2);
    year = id3v2ReadYear(metadata->id3v2);
    track = id3v2ReadTrack(metadata->id3v2);
    comment = id3v2ReadComment(metadata->id3v2);
    genre = id3v2ReadGenre(metadata->id3v2);

    if(title != NULL){
        id3v1WriteTitle(title, newTag);
        free(title);
    }

    if(artist != NULL){
        id3v1WriteArtist(artist, newTag);
        free(artist);
    }

    if(album != NULL){
        id3v1WriteAlbum(album, newTag);
        free(album);
    }

    if(year != NULL){
        id3v1WriteYear(atoi(year), newTag);
        free(year);
    }

    if(track != NULL){

        int i = 0;
        int offset0 = 0;
        int convi = 0;
        char *dec = NULL;
        char *end = NULL;
        bool flag = false;
        
        while(track[i] != '\0'){
            if(track[i] >= '0' && track[i] <= '9'){
                if(!flag && track[i] == '0'){
                    offset0++;
                }else{
                    flag = true;
                }
            }else{
                break;
            }
            i++;
        }

        dec = calloc(i - offset0 + 1, sizeof(char));
        memcpy(dec, track + offset0, i - offset0);
        convi = strtol(dec, &end, 10);
        id3v1WriteTrack(convi, newTag);

        free(dec);
        free(track);
    }

    if(comment != NULL){
        id3v1WriteComment(comment, newTag);
        free(comment);
    }

    if(genre != NULL){
        id3v1WriteGenre(genre[0], newTag);
        free(genre);
    }

    if(metadata->id3v1 != NULL){
        id3v1DestroyTag(&(metadata->id3v1));
    }

    metadata->id3v1 = newTag;
    return true;
}

// internal -----------------------------------------------------------------
// corrects the standard preference if the preferred standard is not available
static int _getSafePrefStd(ID3 *metadata){

    int input = 0;
    int pref = id3GetPreferredStandard();

    if(pref > ID3V1_TAG_VERSION && metadata->id3v2 == NULL){
        input = ID3V1_TAG_VERSION;
    }else if(pref == ID3V1_TAG_VERSION && metadata->id3v1 == NULL){
        // assumed because of its wide use
        input = ID3V2_TAG_VERSION_3;
    }else{
        input = pref;
    }


    return input;
}

/**
 * @brief Reads the title from the given ID3 structure using the preferred standard. If the title is not found
 * NULL is returned.
 * 
 * @param metadata 
 * @return char* 
 */
char *id3ReadTitle(ID3 *metadata){

    if(metadata == NULL){
        return NULL;
    }

    if(metadata->id3v2 == NULL && metadata->id3v1 == NULL){
        return NULL;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V1_TAG_VERSION:
            return id3v1ReadTitle(metadata->id3v1);
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadTitle(metadata->id3v2);
        default:
            return NULL;
    }

    // dummy
    return NULL;
}

/**
 * @brief Reads the artist from the given ID3 structure using the preferred standard. If the artist is not found
 * NULL is returned.
 * 
 * @param metadata 
 * @return char* 
 */
char *id3ReadArtist(ID3 *metadata){
    
    if(metadata == NULL){
        return NULL;
    }

    if(metadata->id3v2 == NULL && metadata->id3v1 == NULL){
        return NULL;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V1_TAG_VERSION:
            return id3v1ReadArtist(metadata->id3v1);
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadArtist(metadata->id3v2);
        default:
            return NULL;
    }

    // dummy
    return NULL;
}

/**
 * @brief Reads the album artist from the given ID3 structure using the preferred standard. If the album artist is not found
 * NULL is returned.
 * @details This function is only available in ID3v2 tags and will always return NULL for ID3v1.
 * @param metadata 
 * @return char* 
 */
char *id3ReadAlbumArtist(ID3 *metadata){

    if(metadata == NULL){
        return NULL;
    }

    if(metadata->id3v2 == NULL && metadata->id3v1 == NULL){
        return NULL;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadAlbumArtist(metadata->id3v2);
        case ID3V1_TAG_VERSION:
        default:
            return NULL;
    }

    // dummy
    return NULL;
}

/**
 * @brief Reads the album from the given ID3 structure using the preferred standard. If the album is not found
 * NULL is returned.
 * 
 * @param metadata 
 * @return char* 
 */
char *id3ReadAlbum(ID3 *metadata){

    if(metadata == NULL){
        return NULL;
    }

    if(metadata->id3v2 == NULL && metadata->id3v1 == NULL){
        return NULL;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V1_TAG_VERSION:
            return id3v1ReadAlbum(metadata->id3v1);
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadAlbum(metadata->id3v2);
        default:
            return NULL;
    }

    // dummy
    return NULL;
}

/**
 * @brief Reads the year from the given ID3 structure using the preferred standard. If the year is not found
 * NULL is returned.
 * 
 * @param metadata 
 * @return char* 
 */
char *id3ReadYear(ID3 *metadata){

    if(metadata == NULL){
        return NULL;
    }

    if(metadata->id3v2 == NULL && metadata->id3v1 == NULL){
        return NULL;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V1_TAG_VERSION:{
            char *year = NULL;
            int size = 0;
            
            size = snprintf(NULL, 0, "%d", metadata->id3v1->year);
            year = calloc(size + 1, sizeof(char));
            snprintf(year, size + 1, "%d", metadata->id3v1->year);
            
            return year;
        }
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadYear(metadata->id3v2);
        default:
            return NULL;
    }

    // dummy
    return NULL;
}

/**
 * @brief Reads the genre from the given ID3 structure using the preferred standard. If the genre is not found
 * NULL is returned.
 * 
 * @param metadata 
 * @return char* 
 */
char *id3ReadGenre(ID3 *metadata){

    if(metadata == NULL){
        return NULL;
    }

    if(metadata->id3v2 == NULL && metadata->id3v1 == NULL){
        return NULL;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V1_TAG_VERSION:{
            char *genre = NULL;
            int size = 0;

            size = strlen(id3v1GenreFromTable(metadata->id3v1->genre));
            genre = calloc(size + 1, sizeof(char));
            memcpy(genre, id3v1GenreFromTable(metadata->id3v1->genre), size);
            return genre;
        }
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadGenre(metadata->id3v2);
        default:
            return NULL;
    }

    // dummy
    return NULL;
}

/**
 * @brief Reads the track from the given ID3 structure using the preferred standard. If the track is not found
 * NULL is returned.
 * 
 * @param metadata 
 * @return char* 
 */
char *id3ReadTrack(ID3 *metadata){

    if(metadata == NULL){
        return NULL;
    }

    if(metadata->id3v2 == NULL && metadata->id3v1 == NULL){
        return NULL;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V1_TAG_VERSION:{
            char *track = NULL;
            int size = 0;

            size = snprintf(NULL, 0, "%d", metadata->id3v1->track);
            track = calloc(size + 1, sizeof(char));
            snprintf(track, size + 1, "%d", metadata->id3v1->track);
            return track;
        }
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadTrack(metadata->id3v2);
        default:
            return NULL;
    }

    // dummy
    return NULL;

}

/**
 * @brief Reads the composer from the given ID3 structure using the preferred standard. If the composer is not found
 * NULL is returned.
 * @details This function is only available in ID3v2 tags and will always return NULL for ID3v1.
 * @param metadata 
 * @return char* 
 */
char *id3ReadComposer(ID3 *metadata){
    
    if(metadata == NULL){
        return NULL;
    }

    if(metadata->id3v2 == NULL && metadata->id3v1 == NULL){
        return NULL;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadComposer(metadata->id3v2);
        case ID3V1_TAG_VERSION:
        default:
            return NULL;
    }

    // dummy
    return NULL;
}

/**
 * @brief Reads the disc from the given ID3 structure using the preferred standard. If the disc is not found
 * NULL is returned.
 * @details This function is only available in ID3v2 tags and will always return NULL for ID3v1.
 * @param metadata 
 * @return char* 
 */
char *id3ReadDisc(ID3 *metadata){
    
    if(metadata == NULL){
        return NULL;
    }

    if(metadata->id3v2 == NULL && metadata->id3v1 == NULL){
        return NULL;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadDisc(metadata->id3v2);
        case ID3V1_TAG_VERSION:
        default:
            return NULL;
    }

    // dummy
    return NULL;
}

/**
 * @brief Reads the lyrics from the given ID3 structure using the preferred standard. If the lyrics are not found
 * NULL is returned.
 * @details This function is only available in ID3v2 tags and will always return NULL for ID3v1.
 * @param metadata 
 * @return char* 
 */
char *id3ReadLyrics(ID3 *metadata){
        
    if(metadata == NULL){
        return NULL;
    }

    if(metadata->id3v2 == NULL && metadata->id3v1 == NULL){
        return NULL;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadLyrics(metadata->id3v2);
        case ID3V1_TAG_VERSION:
        default:
            return NULL;
    }

    // dummy
    return NULL;
}

/**
 * @brief Reads the comment from the given ID3 structure using the preferred standard. If the comment is not found
 * NULL is returned.
 * 
 * @param metadata 
 * @return char* 
 */
char *id3ReadComment(ID3 *metadata){

    if(metadata == NULL){
        return NULL;
    }

    if(metadata->id3v2 == NULL && metadata->id3v1 == NULL){
        return NULL;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V1_TAG_VERSION:
            return id3v1ReadComment(metadata->id3v1);
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadComment(metadata->id3v2);
        default:
            return NULL;
    }

    // dummy
    return NULL;
}

/**
 * @brief Reads the picture from the given ID3 structure using the preferred standard. If the picture is not found
 * NULL is returned.
 * @details This function is only available in ID3v2 tags and will always return NULL for ID3v1.
 * @param type 
 * @param metadata 
 * @param dataSize 
 * @return uint8_t* 
 */
uint8_t *id3ReadPicture(uint8_t type, ID3 *metadata, size_t *dataSize){

    if(metadata == NULL){
        *dataSize = 0;
        return NULL;
    }

    if(metadata->id3v2 == NULL && metadata->id3v1 == NULL){
        *dataSize = 0;
        return NULL;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V1_TAG_VERSION:
            *dataSize = 0;
            return NULL;
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadPicture(type, metadata->id3v2, dataSize);
        default:
            *dataSize = 0;
            return NULL;
    }

    // dummy
    *dataSize = 0;
    return NULL;

}

/**
 * @brief Writes a title to the given ID3 structure using the preferred standard. If the title is not written
 * this function will return false otherwise it will return true.
 * 
 * @param title 
 * @param metadata 
 * @return int 
 */
int id3WriteTitle(const char *title, ID3 *metadata){

    if(metadata == NULL || title == NULL){
        return false;
    }

    if(metadata->id3v1 == NULL && metadata->id3v2 == NULL){
        return false;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V1_TAG_VERSION:
            return id3v1WriteTitle(title, metadata->id3v1);
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteTitle(title, metadata->id3v2);
        default:
            return false;
    }

    // dummy
    return false;
}

/**
 * @brief Writes an artist to the given ID3 structure using the preferred standard. If the artist is not written
 * this function will return false otherwise it will return true.
 * 
 * @param artist 
 * @param metadata 
 * @return int 
 */
int id3WriteArtist(const char *artist, ID3 *metadata){

    if(metadata == NULL || artist == NULL){
        return false;
    }

    if(metadata->id3v1 == NULL && metadata->id3v2 == NULL){
        return false;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V1_TAG_VERSION:
            return id3v1WriteArtist(artist, metadata->id3v1);
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteArtist(artist, metadata->id3v2);
        default:
            return false;
    }

    // dummy
    return false;
}

/**
 * @brief Writes an album artist to the given ID3 structure using the preferred standard. If the album artist is not written
 * this function will return false otherwise it will return true.
 * @details This function is only available in ID3v2 tags and will always return false for ID3v1.
 * @param albumArtist 
 * @param metadata 
 * @return int 
 */
int id3WriteAlbumArtist(const char *albumArtist, ID3 *metadata){

    if(metadata == NULL || albumArtist == NULL){
        return false;
    }

    if(metadata->id3v1 == NULL && metadata->id3v2 == NULL){
        return false;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteAlbumArtist(albumArtist, metadata->id3v2);
        case ID3V1_TAG_VERSION:
        default:
            return false;
    }

    // dummy
    return false;
}

/**
 * @brief Writes an album to the given ID3 structure using the preferred standard. If the album is not written
 * this function will return false otherwise it will return true.
 * 
 * @param album 
 * @param metadata 
 * @return int 
 */
int id3WriteAlbum(const char *album, ID3 *metadata){
    
    if(metadata == NULL || album == NULL){
        return false;
    }

    if(metadata->id3v1 == NULL && metadata->id3v2 == NULL){
        return false;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V1_TAG_VERSION:
            return id3v1WriteAlbum(album, metadata->id3v1);
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteAlbum(album, metadata->id3v2);
        default:
            return false;
    }

    // dummy
    return false;
}

/**
 * @brief Writes a year to the given ID3 structure using the preferred standard. If the year is not written
 * this function will return false otherwise it will return true.
 * 
 * @param year 
 * @param metadata 
 * @return int 
 */
int id3WriteYear(const char *year, ID3 *metadata){

    if(metadata == NULL || year == NULL){
        return false;
    }

    if(metadata->id3v1 == NULL && metadata->id3v2 == NULL){
        return false;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V1_TAG_VERSION:{
            int convi = 0;
            char *end = NULL;
            convi = strtol(year, &end, 10);
            return id3v1WriteYear(convi, metadata->id3v1);
        }
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteYear(year, metadata->id3v2);
        default:
            return false;
    }

    // dummy
    return false;
}

/**
 * @brief Writes a genre to the given ID3 structure using the preferred standard. If the genre is not written
 * this function will return false otherwise it will return true. If the preferred standard is ID3V1 the genre
 * will consist of a single byte from 0 to 192 (check id3v1Types.h for the genre table).
 * 
 * @param genre 
 * @param metadata 
 * @return int 
 */
int id3WriteGenre(const char *genre, ID3 *metadata){
    
    if(metadata == NULL || genre == NULL){
        return false;
    }

    if(metadata->id3v1 == NULL && metadata->id3v2 == NULL){
        return false;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V1_TAG_VERSION:{    
            uint8_t usableGenre = (uint8_t) genre[0] > PSYBIENT_GENRE ? OTHER_GENRE : (uint8_t) genre[0];        
            return id3v1WriteGenre(usableGenre, metadata->id3v1);
        }
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteGenre(genre, metadata->id3v2);
        default:
            return false;
    }

    // dummy
    return false;
}

/**
 * @brief Writes a track to the given ID3 structure using the preferred standard. If the track is not written
 * this function will return false otherwise it will return true.
 * 
 * @param track 
 * @param metadata 
 * @return int 
 */
int id3WriteTrack(const char *track, ID3 *metadata){

    if(metadata == NULL || track == NULL){
        return false;
    }

    if(metadata->id3v1 == NULL && metadata->id3v2 == NULL){
        return false;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V1_TAG_VERSION:{
            int convi = 0;
            char *end = NULL;
            convi = strtol(track, &end, 10);

            if(convi > UINT8_MAX){
                convi = UINT8_MAX;
            }else if(convi < 0){
                convi = 0;
            }

            return id3v1WriteTrack(convi, metadata->id3v1);
        }
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteTrack(track, metadata->id3v2);
        default:
            return false;
    }

    // dummy
    return false;
}

/**
 * @brief writes a disc number to the given ID3 structure using the preferred standard. If the disc number is not written
 * this function will return false otherwise it will return true.
 * @details This function is only available in ID3v2 tags and will always return false for ID3v1.
 * @param disc 
 * @param metadata 
 * @return int 
 */
int id3WriteDisc(const char *disc, ID3 *metadata){

    if(metadata == NULL || disc == NULL){
        return false;
    }

    if(metadata->id3v1 == NULL && metadata->id3v2 == NULL){
        return false;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteDisc(disc, metadata->id3v2);
        case ID3V1_TAG_VERSION:
        default:
            return false;
    }

    // dummy
    return false;
}

/**
 * @brief Writes composers to the given ID3 structure using the preferred standard. If a composer are not written
 * this function will return false otherwise it will return true.
 * @details This function is only available in ID3v2 tags and will always return false for ID3v1.
 * @param composer 
 * @param metadata 
 * @return int 
 */
int id3WriteComposer(const char *composer, ID3 *metadata){
    
    if(metadata == NULL || composer == NULL){
        return false;
    }

    if(metadata->id3v1 == NULL && metadata->id3v2 == NULL){
        return false;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteComposer(composer, metadata->id3v2);
        case ID3V1_TAG_VERSION:
        default:
            return false;
    }

    // dummy
    return false;
}

/**
 * @brief Writes lyrics to the given ID3 structure using the preferred standard. If the lyrics are not written
 * this function will return false otherwise it will return true.
 * @details This function is only available in ID3v2 tags and will always return false for ID3v1.
 * @param lyrics 
 * @param metadata 
 * @return int 
 */
int id3WriteLyrics(const char *lyrics, ID3 *metadata){

    if(metadata == NULL || lyrics == NULL){
        return false;
    }

    if(metadata->id3v1 == NULL && metadata->id3v2 == NULL){
        return false;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteLyrics(lyrics, metadata->id3v2);
        case ID3V1_TAG_VERSION:
        default:
            return false;
    }

    // dummy
    return false;
}

/**
 * @brief Writes a comment to the given ID3 structure using the preferred standard. If the comment is not written
 * this function will return false otherwise it will return true.
 * 
 * @param comment 
 * @param metadata 
 * @return int 
 */
int id3WriteComment(const char *comment, ID3 *metadata){

    if(metadata == NULL || comment == NULL){
        return false;
    }

    if(metadata->id3v1 == NULL && metadata->id3v2 == NULL){
        return false;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V1_TAG_VERSION:
            return id3v1WriteComment(comment, metadata->id3v1);
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteComment(comment, metadata->id3v2);
        default:
            return false;
    }

    // dummy
    return false;
}

/**
 * @brief Writes a picture to the given ID3 structure using the preferred standard. If the picture is not written
 * this function will return false otherwise it will return true.
 * @details This function is only available in ID3v2 tags and will always return false for ID3v1.
 * 
 * @param image 
 * @param imageSize 
 * @param kind 
 * @param type 
 * @param metadata 
 * @return int 
 */
int id3WritePicture(uint8_t *image, size_t imageSize, const char *kind, uint8_t type, ID3 *metadata){
    
    if(metadata == NULL || image == NULL || kind == NULL){
        return false;
    }

    if(metadata->id3v1 == NULL && metadata->id3v2 == NULL){
        return false;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WritePicture(image, imageSize, kind, type, metadata->id3v2);
        case ID3V1_TAG_VERSION:
        default:
            return false;
    }

    // dummy
    return false;
}

/**
 * @brief Writes a picture from a file to the given ID3 structure using the preferred standard. If the picture is not written
 * this function will return false otherwise it will return true.
 * @details This function is only available in ID3v2 tags and will always return false for ID3v1.
 * 
 * @param filename 
 * @param kind 
 * @param type 
 * @param metadata 
 * @return int 
 */
int id3WritePictureFromFile(const char *filename, const char *kind, uint8_t type, ID3 *metadata){
    
    if(metadata == NULL || filename == NULL || kind == NULL){
        return false;
    }

    if(metadata->id3v1 == NULL && metadata->id3v2 == NULL){
        return false;
    }

    switch(_getSafePrefStd(metadata)){
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WritePictureFromFile(filename, kind, type, metadata->id3v2);
        case ID3V1_TAG_VERSION:
        default:
            return false;
    }

    // dummy
    return false;
}

/**
 * @brief Converts the given ID3 structure to a JSON string. If the ID3 structure is NULL empty JSON is returned.
 * 
 * @param metadata 
 * @return char* 
 */
char *id3ToJSON(ID3 *metadata){
    
    char *json = NULL;
    char *id3v1 = NULL;
    char *id3v2 = NULL;
    size_t memCount = 3;

    if(metadata == NULL){
        json = calloc(memCount, sizeof(char));
        memcpy(json, "{}\0", memCount);
        return json;
    }

    id3v1 = id3v1ToJSON(metadata->id3v1);
    id3v2 = id3v2TagToJSON(metadata->id3v2);

    memCount += snprintf(NULL, 0, "{\"id3v1\":%s,\"id3v2\":%s}", id3v1, id3v2);
    json = calloc(memCount, sizeof(char));
    snprintf(json, memCount, "{\"ID3v1\":%s,\"ID3v2\":%s}", id3v1, id3v2);

    free(id3v1);
    free(id3v2);

    return json;
}

/**
 * @brief Writes both ID3v1 and ID3v2 tags to a file using the given ID3 structure. If a tag is found
 * it will be updated, otherwise it will be created.
 * 
 * @param filePath 
 * @param metadata 
 * @return int 
 */
int id3WriteToFile(const char *filePath, ID3 *metadata){

    if(filePath == NULL || metadata == NULL){
        return false;
    }

    // no point in trying
    if(metadata->id3v1 == NULL && metadata->id3v2 == NULL){
        return false;

    }


    bool v1 = false;
    bool v2 = false;

    v1 = id3v1WriteTagToFile(filePath, metadata->id3v1);
    v2 = id3v2WriteTagToFile(filePath, metadata->id3v2);


    if(v1 && v2){
        return true;
    }

    if(v1 == false && (v2 == true && metadata->id3v1 == NULL)){
        return true;

    }

    if(v2 == false && (v1 == true && metadata->id3v2 == NULL)){
        return true;

    }

    return false;
}
