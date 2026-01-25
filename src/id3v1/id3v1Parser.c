/**
 * @file id3v1Parser.c
 * @author Ewan Jones
 * @brief Implementation of ID3v1 tag parsing and memory management functions
 * @version 26.01
 * @date 2023-10-03 - 2026-01-11
 * 
 * @copyright Copyright (c) 2023 - 2026
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v1/id3v1Types.h"
#include "id3v1/id3v1Parser.h"
#include "id3dependencies/ByteStream/include/byteStream.h"
#include "id3dependencies/ByteStream/include/byteInt.h"

/**
 * @brief Checks if a buffer contains a valid ID3v1 tag identifier.
 * @details Compares the first ID3V1_TAG_ID_SIZE bytes of the buffer against the "TAG" identifier.
 * @param buffer - The buffer to check. Must contain at least ID3V1_TAG_ID_SIZE bytes.
 * @return bool - true if the buffer starts with "TAG", false otherwise.
 */
bool id3v1HasTag(const uint8_t *buffer) {
    return (memcmp(buffer, "TAG",ID3V1_TAG_ID_SIZE) == 0) ? true : false;
}

/**
 * @brief Creates and allocates a new Id3v1Tag structure.
 * @details Allocates memory, initializes all fields to zero, then copies provided values. String fields are truncated to ID3V1_FIELD_SIZE if needed. Caller must free with id3v1DestroyTag.
 * @param title - Song title.
 * @param artist - Artist name.
 * @param albumTitle - Album title.
 * @param year - Release year
 * @param track - Track number
 * @param comment - Comment text.
 * @param genre - Genre enum value
 * @return Id3v1Tag* - Heap allocated tag structure.
 */
Id3v1Tag *id3v1CreateTag(uint8_t *title, uint8_t *artist, uint8_t *albumTitle, int year, int track, uint8_t *comment,
                         Genre genre) {
    Id3v1Tag *tag = malloc(sizeof(Id3v1Tag));

    //inits all members
    memset(tag, 0, sizeof(Id3v1Tag));

    //set values
    if (title != NULL) {
        memcpy(tag->title, title,
               ((strlen((char *) title) >= ID3V1_FIELD_SIZE) ? ID3V1_FIELD_SIZE : strlen((char *) title)));
    }

    if (artist != NULL) {
        memcpy(tag->artist, artist,
               ((strlen((char *) artist) >= ID3V1_FIELD_SIZE) ? ID3V1_FIELD_SIZE : strlen((char *) artist)));
    }

    if (albumTitle != NULL) {
        memcpy(tag->albumTitle, albumTitle, ((strlen((char *) albumTitle) >= ID3V1_FIELD_SIZE)
                                                 ? ID3V1_FIELD_SIZE
                                                 : strlen((char *) albumTitle)));
    }

    if (comment != NULL) {
        memcpy(tag->comment, comment,
               ((strlen((char *) comment) >= ID3V1_FIELD_SIZE) ? ID3V1_FIELD_SIZE : strlen((char *) comment)));
    }

    tag->year = year;
    tag->track = track;
    tag->genre = genre;

    return tag;
}

/**
 * @brief Resets all tag fields to their default empty state.
 * @details Zeros out all string fields and sets numeric fields to 0. Genre is set to OTHER_GENRE as the default unknown value.
 * @param tag - The tag to clear.
 */
void id3v1ClearTag(Id3v1Tag *tag) {
    if (tag == NULL) {
        return;
    }

    memset(tag->albumTitle, 0, ID3V1_FIELD_SIZE);
    memset(tag->artist, 0, ID3V1_FIELD_SIZE);
    memset(tag->comment, 0, ID3V1_FIELD_SIZE);
    memset(tag->title, 0, ID3V1_FIELD_SIZE);

    tag->genre = OTHER_GENRE;
    tag->track = 0;
    tag->year = 0;
}

/**
 * @brief Frees an Id3v1Tag and nullifies the pointer.
 * @details Deallocates the tag memory and sets the pointer to NULL to prevent dangling pointer issues.
 * @param toDelete - Pointer to the tag pointer to free. Safe to call with NULL.
 */
void id3v1DestroyTag(Id3v1Tag **toDelete) {
    //error address free
    if (*toDelete) {
        free(*toDelete);
        *toDelete = NULL;
        toDelete = NULL;
    }
}

/**
 * @brief Parses an ID3V1_MAX_SIZE buffer into an Id3v1Tag structure.
 * @details Performs sequential field extraction with extensive error checking. Returns partial tags with default values if parsing fails partway through.
 * Detects ID3v1.1 track numbers via null byte at position 28.
 * @param buffer - Buffer containing ID3V1_MAX_SIZE (128) bytes to parse.
 * @return Id3v1Tag* - Newly allocated tag on success, partial tag with defaults on read errors, NULL if "TAG" identifier missing. Caller must free with 
 * id3v1DestroyTag.
 */
Id3v1Tag *id3v1TagFromBuffer(uint8_t *buffer) {
    //lots of error checking because I cannot 100% know what im given

    int trackno = 0;
    int nYear = 0;
    uint8_t holdTitle[ID3V1_FIELD_SIZE + 1] = {0};
    uint8_t holdArtist[ID3V1_FIELD_SIZE + 1] = {0};
    uint8_t holdAlbum[ID3V1_FIELD_SIZE + 1] = {0};
    uint8_t holdComment[ID3V1_FIELD_SIZE + 1] = {0};
    uint8_t year[ID3V1_YEAR_SIZE + 1]; //must be +1. without it holdTitle gets overwritten somehow
    Genre genre = OTHER_GENRE;
    ByteStream *stream = NULL;

    stream = byteStreamCreate((unsigned char *) buffer, ID3V1_MAX_SIZE);

    //check for tag
    if (!id3v1HasTag(byteStreamCursor(stream))) {
        byteStreamDestroy(stream);
        return NULL;
    }

    if (!byteStreamSeek(stream, ID3V1_TAG_ID_SIZE, SEEK_SET)) {
        byteStreamDestroy(stream);
        return id3v1CreateTag(NULL, NULL, NULL, 0, 0, NULL, OTHER_GENRE);
    }
    //get song title and set index for next tag
    memset(holdTitle, 0, ID3V1_FIELD_SIZE);
    if (!byteStreamRead(stream, holdTitle, ID3V1_FIELD_SIZE)) {
        byteStreamDestroy(stream);
        return id3v1CreateTag(NULL, NULL, NULL, 0, 0, NULL, OTHER_GENRE);
    }

    //get artist and set index for next tag
    memset(holdArtist, 0, ID3V1_FIELD_SIZE);
    if (!byteStreamRead(stream, holdArtist, ID3V1_FIELD_SIZE)) {
        byteStreamDestroy(stream);
        return id3v1CreateTag(holdTitle, NULL, NULL, 0, 0, NULL, OTHER_GENRE);
    }
    //get album title and set index for next tag
    memset(holdAlbum, 0, ID3V1_FIELD_SIZE);
    if (!byteStreamRead(stream, holdAlbum, ID3V1_FIELD_SIZE)) {
        byteStreamDestroy(stream);
        return id3v1CreateTag(holdTitle, holdArtist, NULL, 0, 0, NULL, OTHER_GENRE);
    }
    //get year and set index for next tag
    memset(year, 0, ID3V1_YEAR_SIZE);
    if (!byteStreamRead(stream, year, ID3V1_YEAR_SIZE)) {
        byteStreamDestroy(stream);
        return id3v1CreateTag(holdTitle, holdArtist, holdAlbum, 0, 0, NULL, OTHER_GENRE);
    } else {
        if (memcmp(year, "\x00\x00\x00\x00\x00",ID3V1_YEAR_SIZE) != 0) {
            int i = 0;
            for (i = 0; year[i] == 0 && i < ID3V1_YEAR_SIZE; i++) {
            }
            year[ID3V1_YEAR_SIZE] = '\0';
            nYear = (int) strtol((char *) (year + i), NULL, 10);
        }
    }

    //check for a track number, ID3V1.1 has the 28th bit nulled so that the 29th can be a track number
    if (!byteStreamSeek(stream, ID3V1_FIELD_SIZE - 2, SEEK_CUR)) {
        byteStreamDestroy(stream);
        return id3v1CreateTag(holdTitle, holdArtist, holdAlbum, nYear, 0, NULL, OTHER_GENRE);
    }
    if (!byteStreamCursor(stream)[0] && byteStreamCursor(stream)[1]) {
        trackno = 1;
    }

    //no need to check bytes are confirmed to exist
    byteStreamSeek(stream, -(ID3V1_FIELD_SIZE - 2), SEEK_CUR);

    //get comment and set index for next tag
    memset(holdComment, 0, ID3V1_FIELD_SIZE);
    if (!byteStreamRead(stream, holdComment, ID3V1_FIELD_SIZE - trackno)) {
        byteStreamDestroy(stream);
        return id3v1CreateTag(holdTitle, holdArtist, holdAlbum, nYear, 0, NULL, 0);
    }


    //read and set track number + move index
    if (trackno) {
        trackno = byteStreamGetCh(stream);
        if (trackno == EOF) {
            byteStreamDestroy(stream);
            return id3v1CreateTag(holdTitle, holdArtist, holdAlbum, nYear, 0, holdAlbum, 0);
        }

        if (!byteStreamSeek(stream, 1, SEEK_CUR)) {
            byteStreamDestroy(stream);
            return id3v1CreateTag(holdTitle, holdArtist, holdAlbum, nYear, trackno, holdComment, 0);
        }
    } else {
        trackno = 0;
    }

    //read genre or last byte
    genre = byteStreamGetCh(stream);
    if (genre == EOF) {
        byteStreamDestroy(stream);
        return id3v1CreateTag(holdTitle, holdArtist, holdAlbum, nYear, trackno, holdComment, 0);
    }

    byteStreamDestroy(stream);
    return id3v1CreateTag(holdTitle, holdArtist, holdAlbum, nYear, trackno, holdComment, genre);
}
