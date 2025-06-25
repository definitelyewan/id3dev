/**
 * @file id3v1Parser.c
 * @author Ewan Jones
 * @brief Function definitions for the parser
 * @version 2.0
 * @date 2023-10-03
 * 
 * @copyright Copyright (c) 2023
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
 * @brief Detect a "TAG" magic number/ID to identify the use of id3v1.
 * @details This function returns true if successful and false otherwise.
 * @param buffer 
 * @return true 
 * @return false 
 */
bool id3v1HasTag(const uint8_t *buffer) {
    return (memcmp(buffer, "TAG",ID3V1_TAG_ID_SIZE) == 0) ? true : false;
}

/**
 * @brief Creates an Id3v1Tag structure.
 *
 * @param title
 * @param artist
 * @param albumTitle
 * @param year
 * @param track
 * @param comment
 * @param genre
 * @return Id3v1Tag*
 */
Id3v1Tag *id3v1CreateTag(uint8_t *title, uint8_t *artist, uint8_t *albumTitle, const int year,
                         const int track, uint8_t *comment, const Genre genre) {

    Id3v1Tag *tag = malloc(sizeof(Id3v1Tag));

    // inits all members
    memset(tag, 0, sizeof(Id3v1Tag));

    // set values
    if (title != NULL) {
        memcpy(tag->title, title, ((strlen((char *) title) >= ID3V1_FIELD_SIZE)
                                       ? ID3V1_FIELD_SIZE
                                       : strlen((char *) title)));
    }

    if (artist != NULL) {
        memcpy(tag->artist, artist, ((strlen((char *) artist) >= ID3V1_FIELD_SIZE)
                                         ? ID3V1_FIELD_SIZE
                                         : strlen((char *) artist)));
    }

    if (albumTitle != NULL) {
        memcpy(tag->albumTitle, albumTitle, ((strlen((char *) albumTitle) >= ID3V1_FIELD_SIZE)
                                                 ? ID3V1_FIELD_SIZE
                                                 : strlen((char *) albumTitle)));
    }

    if (comment != NULL) {
        memcpy(tag->comment, comment, ((strlen((char *) comment) >= ID3V1_FIELD_SIZE)
                                           ? ID3V1_FIELD_SIZE
                                           : strlen((char *) comment)));
    }

    tag->year = year;
    tag->track = track;
    tag->genre = genre;

    return tag;
}

/**
 * @brief Resets all structure members to empty.
 * @details OTHER_GENRE(12) is used as clear due to its unknown designation.
 * @param tag
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
 * @brief Frees an Id3v1Tag structure and sets its address and pointer to null.
 * @param toDelete
 */
void id3v1DestroyTag(Id3v1Tag **toDelete) {

    // error address free
    if (*toDelete) {
        free(*toDelete);
        *toDelete = NULL;
        toDelete = NULL;
    }
}

/**
 * @brief Parses a buffer into an Id3V1Tag structure.
 * @details Reads a buffer of fixed length ID3V1_MAX_SIZE. This function will attempt to return an
 * incomplete Id3v1Tag with one or more members missing if an error is encountered or a buffer of
 * insufficient size is given.
 * @param buffer
 * @return Id3v1Tag*
 */
Id3v1Tag *id3v1TagFromBuffer(uint8_t *buffer) {

    // lots of error checking because I cannot 100% know what im given

    int trackNumber = 0;
    int nYear = 0;
    uint8_t holdTitle[ID3V1_FIELD_SIZE + 1] = {0};
    uint8_t holdArtist[ID3V1_FIELD_SIZE + 1] = {0};
    uint8_t holdAlbum[ID3V1_FIELD_SIZE + 1] = {0};
    uint8_t holdComment[ID3V1_FIELD_SIZE + 1] = {0};
    uint8_t year[ID3V1_YEAR_SIZE + 1]; // must be +1. without it holdTitle gets overwritten somehow
    ByteStream *stream = NULL;

    stream = byteStreamCreate((unsigned char *) buffer, ID3V1_MAX_SIZE);

    // check for tag
    if (!id3v1HasTag(byteStreamCursor(stream))) {
        byteStreamDestroy(stream);
        return NULL;
    }

    if (!byteStreamSeek(stream, ID3V1_TAG_ID_SIZE, SEEK_SET)) {
        byteStreamDestroy(stream);
        return id3v1CreateTag(NULL, NULL, NULL, 0, 0, NULL, OTHER_GENRE);
    }

    // get song title and set index for next tag
    memset(holdTitle, 0, ID3V1_FIELD_SIZE);
    if (!byteStreamRead(stream, holdTitle, ID3V1_FIELD_SIZE)) {
        byteStreamDestroy(stream);
        return id3v1CreateTag(NULL, NULL, NULL, 0, 0, NULL, OTHER_GENRE);
    }

    // get artist and set index for next tag
    memset(holdArtist, 0, ID3V1_FIELD_SIZE);
    if (!byteStreamRead(stream, holdArtist, ID3V1_FIELD_SIZE)) {
        byteStreamDestroy(stream);
        return id3v1CreateTag(holdTitle, NULL, NULL, 0, 0, NULL, OTHER_GENRE);
    }

    // get album title and set index for next tag
    memset(holdAlbum, 0, ID3V1_FIELD_SIZE);
    if (!byteStreamRead(stream, holdAlbum, ID3V1_FIELD_SIZE)) {
        byteStreamDestroy(stream);
        return id3v1CreateTag(holdTitle, holdArtist, NULL, 0, 0, NULL, OTHER_GENRE);
    }

    // get year and set index for next tag
    memset(year, 0, ID3V1_YEAR_SIZE);
    if (!byteStreamRead(stream, year, ID3V1_YEAR_SIZE)) {
        byteStreamDestroy(stream);
        return id3v1CreateTag(holdTitle, holdArtist, holdAlbum, 0, 0, NULL, OTHER_GENRE);
    } else {
        if (memcmp(year, "\x00\x00\x00\x00\x00",ID3V1_YEAR_SIZE) != 0) {

            int i = 0;
            for (i = 0; year[i] == 0 && i < ID3V1_YEAR_SIZE; i++) {
            };
            year[ID3V1_YEAR_SIZE] = '\0';
            nYear = (int) strtol((char *) year + i, NULL, 10);
        }
    }

    // check for a track number, ID3V1.1 has the 28th byte nulled so that the 29th can be a track number
    if (!byteStreamSeek(stream, ID3V1_FIELD_SIZE - 2, SEEK_CUR)) {
        byteStreamDestroy(stream);
        return id3v1CreateTag(holdTitle, holdArtist, holdAlbum, nYear, 0, NULL, OTHER_GENRE);
    }

    if (!byteStreamCursor(stream)[0] && byteStreamCursor(stream)[1]) {
        trackNumber = 1;
    }

    // no need to check bytes are confirmed to exist
    byteStreamSeek(stream, -(ID3V1_FIELD_SIZE - 2), SEEK_CUR);

    // get comment and set index for next tag
    memset(holdComment, 0, ID3V1_FIELD_SIZE);
    if (!byteStreamRead(stream, holdComment, ID3V1_FIELD_SIZE - trackNumber)) {
        byteStreamDestroy(stream);
        return id3v1CreateTag(holdTitle, holdArtist, holdAlbum, nYear, 0, NULL, 0);
    }

    // read and set track number + move index
    if (trackNumber) {
        trackNumber = byteStreamGetCh(stream);

        if (trackNumber == EOF) {
            byteStreamDestroy(stream);
            return id3v1CreateTag(holdTitle, holdArtist, holdAlbum, nYear, 0, holdAlbum, 0);
        }

        if (!byteStreamSeek(stream, 1, SEEK_CUR)) {
            byteStreamDestroy(stream);
            return id3v1CreateTag(holdTitle, holdArtist, holdAlbum, nYear, trackNumber, holdComment,
                                  0);
        }

    } else {
        trackNumber = 0;
    }

    // read genre or last byte
    const Genre genre = byteStreamGetCh(stream);
    if (genre == EOF) {
        byteStreamDestroy(stream);
        return id3v1CreateTag(holdTitle, holdArtist, holdAlbum, nYear, trackNumber, holdComment, 0);
    }

    byteStreamDestroy(stream);
    return id3v1CreateTag(holdTitle, holdArtist, holdAlbum, nYear, trackNumber, holdComment, genre);
}