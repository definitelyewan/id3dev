/**
 * @file id3dev.c
 * @author Ewan Jones
 * @brief Function implementations for reading, writing, converting, and comparing ID3v1 and ID3v2 metadata tags.
 * @version 26.01
 * @date 2024-04-12 - 2026-01-25
 * 
 * @copyright Copyright (c) 2024 - 2026
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
 * @brief Creates a new ID3 metadata structure containing ID3v2 and ID3v1 tags.
 * @details Allocates and initializes an ID3 structure with the provided tag pointers. Both tags can be NULL to represent missing tags.
 * The structure takes ownership of the provided tag pointers. Returns NULL if memory allocation fails.
 * @param id3v2 - Pointer to an ID3v2 tag structure, or NULL if not present.
 * @param id3v1 - Pointer to an ID3v1 tag structure, or NULL if not present.
 * @return ID3* - Pointer to allocated ID3 structure on success, NULL on allocation failure. Caller must free with id3Destroy().
 */
ID3 *id3Create(Id3v2Tag *id3v2, Id3v1Tag *id3v1) {
    ID3 *metadata = malloc(sizeof(ID3));
    if (metadata == NULL) {
        return NULL;
    }

    metadata->id3v2 = id3v2;
    metadata->id3v1 = id3v1;

    return metadata;
}

/**
 * @brief Destroys an ID3 metadata structure and frees all associated memory.
 * @details Calls destroy functions for both ID3v2 and ID3v1 tags (if present), frees the ID3 structure itself, and sets the pointer to NULL.
 * Safe to call with NULL pointer. After calling, the original pointer is invalidated.
 * @param toDelete - Pointer to pointer to ID3 structure to destroy. Set to NULL after destruction.
 */
void id3Destroy(ID3 **toDelete) {
    if (*toDelete) {
        id3v2DestroyTag(&((*toDelete)->id3v2));
        id3v1DestroyTag(&((*toDelete)->id3v1));
        free(*toDelete);
        *toDelete = NULL;
        toDelete = NULL;
    }
}

/**
 * @brief Sets the preferred ID3 standard for reading metadata from tag structures.
 * @details Configures which ID3 version to prioritize when reading metadata from structures containing both ID3v1 and ID3v2 tags.
 * Default is ID3v2.3 (widest adoption with most features). Accepts ID3v1, ID3v2.2, ID3v2.3, or ID3v2.4 version constants.
 * Returns false for invalid version values without changing the current preference.
 * @param standard - ID3 version constant (ID3V1_TAG_VERSION, ID3V2_TAG_VERSION_2, ID3V2_TAG_VERSION_3, or ID3V2_TAG_VERSION_4).
 * @return bool - true if standard set successfully, false if invalid standard value provided.
 */
bool id3SetPreferredStandard(uint8_t standard) {
    switch (standard) {
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
 * @brief Returns the currently configured preferred ID3 standard.
 * @details Retrieves the ID3 version constant that is currently set for reading metadata from tag structures.
 * Default value is ID3v2.3 unless changed via id3SetPreferredStandard().
 * @return uint8_t - ID3 version constant (ID3V1_TAG_VERSION, ID3V2_TAG_VERSION_2, ID3V2_TAG_VERSION_3, or ID3V2_TAG_VERSION_4).
 */
uint8_t id3GetPreferredStandard(void) {
    return id3PreferredStandard;
}


/**
 * @brief Reads both ID3v1 and ID3v2 tags from a file into an ID3 metadata structure.
 * @details Attempts to read both ID3v2 (from file start) and ID3v1 (from file end) tags from the specified file.
 * Always returns an ID3 structure, but individual tag pointers (id3v1, id3v2) will be NULL if not found or if read errors occur.
 * The returned structure must be freed with id3Destroy().
 * @param filePath - Null-terminated string containing the path to the file to read.
 * @return ID3* - Pointer to allocated ID3 structure containing the read tags (tags may be NULL if not found). Caller must free with id3Destroy().
 */
ID3 *id3FromFile(const char *filePath) {
    return id3Create(id3v2TagFromFile(filePath), id3v1TagFromFile(filePath));
}

/**
 * @brief Creates a deep copy of an ID3 metadata structure.
 * @details Allocates a new ID3 structure and copies both ID3v1 and ID3v2 tags (if present).
 * Returns NULL if the input is NULL. The returned copy is independent and must be freed with id3Destroy().
 * @param toCopy - ID3 structure to copy, or NULL.
 * @return ID3* - Pointer to allocated copy of the ID3 structure, or NULL if input was NULL. Caller must free with id3Destroy().
 */
ID3 *id3Copy(const ID3 *toCopy) {
    if (toCopy == NULL) {
        return NULL;
    }

    return id3Create(id3v2CopyTag(toCopy->id3v2), id3v1CopyTag(toCopy->id3v1));
}

/**
 * @brief Compares two ID3 metadata structures for equality.
 * @details Compares both ID3v1 and ID3v2 tags within the structures. Two structures are considered equal if both their ID3v1 tags match (or both are NULL)
 * AND both their ID3v2 tags match (or both are NULL). Returns false if either parameter is NULL or if the tags differ.
 * @param metadata1 - First ID3 structure to compare.
 * @param metadata2 - Second ID3 structure to compare.
 * @return bool - true if structures are equal, false if different or either parameter is NULL.
 */
bool id3Compare(const ID3 *metadata1, const ID3 *metadata2) {
    if (metadata1 == NULL || metadata2 == NULL) {
        return false;
    }

    bool v1 = false;
    bool v2 = false;

    v1 = id3v1CompareTag(metadata1->id3v1, metadata2->id3v1);
    v2 = id3v2CompareTag(metadata1->id3v2, metadata2->id3v2);

    if (v1 && v2) {
        return true;
    }

    if (v1 == false && (v2 == true && metadata1->id3v1 == NULL && metadata2->id3v1 == NULL)) {
        return true;
    }

    if (v2 == false && (v1 == true && metadata1->id3v2 == NULL && metadata2->id3v2 == NULL)) {
        return true;
    }

    return false;
}


/**
 * @brief Converts the ID3v1 tag to an ID3v2 tag within the metadata structure.
 * @details Creates a new ID3v2 tag from the existing ID3v1 tag, copying all non-empty fields (title, artist, album, year, track, genre, comment).
 * Uses the preferred standard to determine the ID3v2 version (2.2, 2.3, or 2.4), defaulting to ID3v2.3 if preferred standard is ID3v1.
 * Numeric fields (year, track) are converted to strings. Genre is looked up from the ID3v1 genre table.
 * If an ID3v2 tag already exists, it is destroyed and replaced. Returns false on validation failures (null metadata, null ID3v1 tag, or write errors),
 * cleaning up all allocated memory. Does not modify the original ID3v1 tag.
 * @param metadata - ID3 structure containing the ID3v1 tag to convert (id3v1 must not be NULL).
 * @return bool - true if conversion successful and ID3v2 tag created, false on failure.
 */
bool id3ConvertId3v1ToId3v2(ID3 *metadata) {
    if (metadata == NULL) {
        return false;
    }

    if (metadata->id3v1 == NULL) {
        return false;
    }

    Id3v2Tag *newTag = NULL;
    Id3v2TagHeader *header = NULL;
    List *frames = NULL;
    char *str = NULL;
    int size = 0;
    int input = 0;

    input = id3GetPreferredStandard() > ID3V1_TAG_VERSION ? id3GetPreferredStandard() : ID3V2_TAG_VERSION_3;

    switch (input) {
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

    if (metadata->id3v1->title[0] != 0x00) {
        if (!id3v2WriteTitle((char *) metadata->id3v1->title, newTag)) {
            id3v2DestroyTag(&newTag);
            listFree(frames);
            return false;
        }
    }


    if (metadata->id3v1->artist[0] != 0x00) {
        if (!id3v2WriteArtist((char *) metadata->id3v1->artist, newTag)) {
            id3v2DestroyTag(&newTag);
            listFree(frames);
            return false;
        }
    }


    if (metadata->id3v1->albumTitle[0] != 0x00) {
        if (!id3v2WriteAlbum((char *) metadata->id3v1->albumTitle, newTag)) {
            id3v2DestroyTag(&newTag);
            listFree(frames);
            return false;
        }
    }


    if (metadata->id3v1->year != 0) {
        size = snprintf(NULL, 0, "%d", metadata->id3v1->year);
        str = calloc(size + 1, sizeof(char));
        (void) snprintf(str, size + 1, "%d", metadata->id3v1->year);
        if (!id3v2WriteYear(str, newTag)) {
            id3v2DestroyTag(&newTag);
            listFree(frames);
            free(str);
            return false;
        }
        free(str);
    }


    if (metadata->id3v1->track != 0) {
        size = snprintf(NULL, 0, "%d", metadata->id3v1->track);
        str = calloc(size + 1, sizeof(char));
        (void) snprintf(str, size + 1, "%d", metadata->id3v1->track);

        if (!id3v2WriteTrack(str, newTag)) {
            id3v2DestroyTag(&newTag);
            listFree(frames);
            free(str);
            return false;
        }
        free(str);
    }

    if (metadata->id3v1->genre < PSYBIENT_GENRE) {
        if (!id3v2WriteGenre(id3v1GenreFromTable(metadata->id3v1->genre), newTag)) {
            id3v2DestroyTag(&newTag);
            listFree(frames);
            return false;
        }
    }


    if (metadata->id3v1->comment[0] != 0x00) {
        if (!id3v2WriteComment((char *) metadata->id3v1->comment, newTag)) {
            id3v2DestroyTag(&newTag);
            listFree(frames);
            return false;
        }
    }

    if (metadata->id3v2 != NULL) {
        id3v2DestroyTag(&(metadata->id3v2));
    }

    metadata->id3v2 = newTag;

    return true;
}


/**
 * @brief Converts the ID3v2 tag to an ID3v1 tag within the metadata structure.
 * @details Creates a new ID3v1 tag from the existing ID3v2 tag, reading and copying all common fields (title, artist, album, year, track, comment, genre).
 * String fields from ID3v2 are converted to ID3v1 format: year and track are parsed as integers, with track parsing stripping leading zeros and extracting
 * only the numeric portion before any delimiter. Genre uses the first character. Fields truncated to ID3v1 limits (30 bytes for most text fields).
 * If an ID3v1 tag already exists, it is destroyed and replaced. Returns false on validation failures (null metadata or null ID3v2 tag).
 * Does not modify the original ID3v2 tag. Frees all intermediate allocations.
 * @param metadata - ID3 structure containing the ID3v2 tag to convert (id3v2 must not be NULL).
 * @return bool - true if conversion successful and ID3v1 tag created, false on failure.
 */
bool id3ConvertId3v2ToId3v1(ID3 *metadata) {
    if (metadata == NULL) {
        return false;
    }

    if (metadata->id3v2 == NULL) {
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

    if (title != NULL) {
        id3v1WriteTitle(title, newTag);
        free(title);
    }

    if (artist != NULL) {
        id3v1WriteArtist(artist, newTag);
        free(artist);
    }

    if (album != NULL) {
        id3v1WriteAlbum(album, newTag);
        free(album);
    }

    if (year != NULL) {
        id3v1WriteYear((int) strtol(year, NULL, 10), newTag);
        free(year);
    }

    if (track != NULL) {
        int i = 0;
        int offset0 = 0;
        int convi = 0;
        char *dec = NULL;
        char *end = NULL;
        bool flag = false;

        while (track[i] != '\0') {
            if (track[i] >= '0' && track[i] <= '9') {
                if (!flag && track[i] == '0') {
                    offset0++;
                } else {
                    flag = true;
                }
            } else {
                break;
            }
            i++;
        }

        dec = calloc(i - offset0 + 1, sizeof(char));
        memcpy(dec, track + offset0, i - offset0);
        convi = (int) strtol(dec, &end, 10);
        id3v1WriteTrack(convi, newTag);

        free(dec);
        free(track);
    }

    if (comment != NULL) {
        id3v1WriteComment(comment, newTag);
        free(comment);
    }

    if (genre != NULL) {
        id3v1WriteGenre(genre[0], newTag);
        free(genre);
    }

    if (metadata->id3v1 != NULL) {
        id3v1DestroyTag(&(metadata->id3v1));
    }

    metadata->id3v1 = newTag;
    return true;
}

// internal -----------------------------------------------------------------
// corrects the standard preference if the preferred standard is not available
static int internal_getSafePrefStd(const ID3 *metadata) {
    int input = 0;
    int pref = id3GetPreferredStandard();

    if (pref > ID3V1_TAG_VERSION && metadata->id3v2 == NULL) {
        input = ID3V1_TAG_VERSION;
    } else if (pref == ID3V1_TAG_VERSION && metadata->id3v1 == NULL) {
        // assumed because of its wide use
        input = ID3V2_TAG_VERSION_3;
    } else {
        input = pref;
    }


    return input;
}

/**
 * @brief Reads the title from an ID3 metadata structure using the preferred standard.
 * @details Retrieves the title from either the ID3v1 or ID3v2 tag based on the preferred standard setting.
 * If the preferred tag is not available, falls back to the available tag. Returns NULL if both tags are missing,
 * metadata is NULL, or the title field is not found.
 * @param metadata - ID3 structure to read the title from.
 * @return char* - Pointer to allocated null-terminated string containing the title, or NULL if not found. Caller must free the returned string.
 */
char *id3ReadTitle(const ID3 *metadata) {
    if (metadata == NULL) {
        return NULL;
    }

    if (metadata->id3v2 == NULL && metadata->id3v1 == NULL) {
        return NULL;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V1_TAG_VERSION:
            return id3v1ReadTitle(metadata->id3v1);
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadTitle(metadata->id3v2);
        default:
            break;
    }

    return NULL;
}

/**
 * @brief Reads the artist from an ID3 metadata structure using the preferred standard.
 * @details Retrieves the artist from either the ID3v1 or ID3v2 tag based on the preferred standard setting.
 * If the preferred tag is not available, falls back to the available tag. Returns NULL if both tags are missing,
 * metadata is NULL, or the artist field is not found.
 * @param metadata - ID3 structure to read the artist from.
 * @return char* - Pointer to allocated null-terminated string containing the artist, or NULL if not found. Caller must free the returned string.
 */
char *id3ReadArtist(const ID3 *metadata) {
    if (metadata == NULL) {
        return NULL;
    }

    if (metadata->id3v2 == NULL && metadata->id3v1 == NULL) {
        return NULL;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V1_TAG_VERSION:
            return id3v1ReadArtist(metadata->id3v1);
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadArtist(metadata->id3v2);
        default:
            break;
    }

    return NULL;
}

/**
 * @brief Reads the album artist from an ID3 metadata structure using the preferred standard.
 * @details Retrieves the album artist from the ID3v2 tag only, as this field is not available in ID3v1.
 * Returns NULL if metadata is NULL, no ID3v2 tag is present (regardless of preferred standard), or the album artist field is not found.
 * Always returns NULL when only ID3v1 is available.
 * @param metadata - ID3 structure to read the album artist from.
 * @return char* - Pointer to allocated null-terminated string containing the album artist, or NULL if not found or ID3v2 not available. Caller must free the returned string.
 */
char *id3ReadAlbumArtist(const ID3 *metadata) {
    if (metadata == NULL) {
        return NULL;
    }

    if (metadata->id3v2 == NULL && metadata->id3v1 == NULL) {
        return NULL;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadAlbumArtist(metadata->id3v2);
        case ID3V1_TAG_VERSION:
        default:
            break;
    }

    return NULL;
}

/**
 * @brief Reads the album from an ID3 metadata structure using the preferred standard.
 * @details Retrieves the album from either the ID3v1 or ID3v2 tag based on the preferred standard setting.
 * If the preferred tag is not available, falls back to the available tag. Returns NULL if both tags are missing,
 * metadata is NULL, or the album field is not found.
 * @param metadata - ID3 structure to read the album from.
 * @return char* - Pointer to allocated null-terminated string containing the album, or NULL if not found. Caller must free the returned string.
 */
char *id3ReadAlbum(const ID3 *metadata) {
    if (metadata == NULL) {
        return NULL;
    }

    if (metadata->id3v2 == NULL && metadata->id3v1 == NULL) {
        return NULL;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V1_TAG_VERSION:
            return id3v1ReadAlbum(metadata->id3v1);
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadAlbum(metadata->id3v2);
        default:
            break;
    }

    return NULL;
}

/**
 * @brief Reads the year from an ID3 metadata structure using the preferred standard.
 * @details Retrieves the year from either the ID3v1 or ID3v2 tag based on the preferred standard setting.
 * For ID3v1, converts the numeric year field to an allocated string. For ID3v2, returns the year text frame content.
 * If the preferred tag is not available, falls back to the available tag. Returns NULL if both tags are missing,
 * metadata is NULL, or the year field is not found.
 * @param metadata - ID3 structure to read the year from.
 * @return char* - Pointer to allocated null-terminated string containing the year, or NULL if not found. Caller must free the returned string.
 */
char *id3ReadYear(const ID3 *metadata) {
    if (metadata == NULL) {
        return NULL;
    }

    if (metadata->id3v2 == NULL && metadata->id3v1 == NULL) {
        return NULL;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V1_TAG_VERSION: {
            if (metadata->id3v1 == NULL) {
                return NULL;
            }

            char *year = NULL;
            int size = 0;

            size = snprintf(NULL, 0, "%d", metadata->id3v1->year);
            year = calloc(size + 1, sizeof(char));
            (void) snprintf(year, size + 1, "%d", metadata->id3v1->year);

            return year;
        }
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadYear(metadata->id3v2);
        default:
            break;
    }

    return NULL;
}

/**
 * @brief Reads the genre from an ID3 metadata structure using the preferred standard.
 * @details Retrieves the genre from either the ID3v1 or ID3v2 tag based on the preferred standard setting.
 * For ID3v1, looks up the genre byte in the standard genre table and returns an allocated copy of the genre string.
 * For ID3v2, returns the genre text frame content. If the preferred tag is not available, falls back to the available tag.
 * Returns NULL if both tags are missing, metadata is NULL, or the genre field is not found.
 * @param metadata - ID3 structure to read the genre from.
 * @return char* - Pointer to allocated null-terminated string containing the genre, or NULL if not found. Caller must free the returned string.
 */
char *id3ReadGenre(const ID3 *metadata) {
    if (metadata == NULL) {
        return NULL;
    }

    if (metadata->id3v2 == NULL && metadata->id3v1 == NULL) {
        return NULL;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V1_TAG_VERSION: {
            if (metadata->id3v1 == NULL) {
                return NULL;
            }

            char *genre = NULL;
            int size = 0;

            size = (int) strlen(id3v1GenreFromTable(metadata->id3v1->genre));
            genre = calloc(size + 1, sizeof(char));
            memcpy(genre, id3v1GenreFromTable(metadata->id3v1->genre), size);
            return genre;
        }
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadGenre(metadata->id3v2);
        default:
            break;
    }

    return NULL;
}

/**
 * @brief Reads the track number from an ID3 metadata structure using the preferred standard.
 * @details Retrieves the track number from either the ID3v1 or ID3v2 tag based on the preferred standard setting.
 * For ID3v1, converts the numeric track field to an allocated string. For ID3v2, returns the track text frame content (may include format like "track/total").
 * If the preferred tag is not available, falls back to the available tag. Returns NULL if both tags are missing,
 * metadata is NULL, or the track field is not found.
 * @param metadata - ID3 structure to read the track number from.
 * @return char* - Pointer to allocated null-terminated string containing the track number, or NULL if not found. Caller must free the returned string.
 */
char *id3ReadTrack(const ID3 *metadata) {
    if (metadata == NULL) {
        return NULL;
    }

    if (metadata->id3v2 == NULL && metadata->id3v1 == NULL) {
        return NULL;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V1_TAG_VERSION: {
            if (metadata->id3v1 == NULL) {
                return NULL;
            }

            char *track = NULL;
            int size = 0;

            size = snprintf(NULL, 0, "%d", metadata->id3v1->track);
            track = calloc(size + 1, sizeof(char));
            (void) snprintf(track, size + 1, "%d", metadata->id3v1->track);
            return track;
        }
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadTrack(metadata->id3v2);
        default:
            break;
    }

    return NULL;
}

/**
 * @brief Reads the composer from an ID3 metadata structure using the preferred standard.
 * @details Retrieves the composer from the ID3v2 tag only, as this field is not available in ID3v1.
 * Returns NULL if metadata is NULL, no ID3v2 tag is present (regardless of preferred standard), or the composer field is not found.
 * Always returns NULL when only ID3v1 is available.
 * @param metadata - ID3 structure to read the composer from.
 * @return char* - Pointer to allocated null-terminated string containing the composer, or NULL if not found or ID3v2 not available. Caller must free the returned string.
 */
char *id3ReadComposer(const ID3 *metadata) {
    if (metadata == NULL) {
        return NULL;
    }

    if (metadata->id3v2 == NULL && metadata->id3v1 == NULL) {
        return NULL;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadComposer(metadata->id3v2);
        case ID3V1_TAG_VERSION:
        default:
            break;
    }

    return NULL;
}

/**
 * @brief Reads the disc number from an ID3 metadata structure using the preferred standard.
 * @details Retrieves the disc number from the ID3v2 tag only, as this field is not available in ID3v1.
 * Returns NULL if metadata is NULL, no ID3v2 tag is present (regardless of preferred standard), or the disc field is not found.
 * Always returns NULL when only ID3v1 is available.
 * @param metadata - ID3 structure to read the disc number from.
 * @return char* - Pointer to allocated null-terminated string containing the disc number, or NULL if not found or ID3v2 not available. Caller must free the returned string.
 */
char *id3ReadDisc(const ID3 *metadata) {
    if (metadata == NULL) {
        return NULL;
    }

    if (metadata->id3v2 == NULL && metadata->id3v1 == NULL) {
        return NULL;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadDisc(metadata->id3v2);
        case ID3V1_TAG_VERSION:
        default:
            break;
    }

    return NULL;
}

/**
 * @brief Reads the lyrics from an ID3 metadata structure using the preferred standard.
 * @details Retrieves the lyrics from the ID3v2 tag only, as this field is not available in ID3v1.
 * Returns NULL if metadata is NULL, no ID3v2 tag is present (regardless of preferred standard), or the lyrics field is not found.
 * Always returns NULL when only ID3v1 is available.
 * @param metadata - ID3 structure to read the lyrics from.
 * @return char* - Pointer to allocated null-terminated string containing the lyrics, or NULL if not found or ID3v2 not available. Caller must free the returned string.
 */
char *id3ReadLyrics(const ID3 *metadata) {
    if (metadata == NULL) {
        return NULL;
    }

    if (metadata->id3v2 == NULL && metadata->id3v1 == NULL) {
        return NULL;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadLyrics(metadata->id3v2);
        case ID3V1_TAG_VERSION:
        default:
            break;
    }

    return NULL;
}

/**
 * @brief Reads the comment from an ID3 metadata structure using the preferred standard.
 * @details Retrieves the comment from either the ID3v1 or ID3v2 tag based on the preferred standard setting.
 * If the preferred tag is not available, falls back to the available tag. Returns NULL if both tags are missing,
 * metadata is NULL, or the comment field is not found.
 * @param metadata - ID3 structure to read the comment from.
 * @return char* - Pointer to allocated null-terminated string containing the comment, or NULL if not found. Caller must free the returned string.
 */
char *id3ReadComment(const ID3 *metadata) {
    if (metadata == NULL) {
        return NULL;
    }

    if (metadata->id3v2 == NULL && metadata->id3v1 == NULL) {
        return NULL;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V1_TAG_VERSION:
            return id3v1ReadComment(metadata->id3v1);
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadComment(metadata->id3v2);
        default:
            break;
    }

    return NULL;
}

/**
 * @brief Reads a picture of the specified type from an ID3 metadata structure using the preferred standard.
 * @details Retrieves binary picture data from the ID3v2 tag only, as pictures are not available in ID3v1.
 * Searches for a picture frame matching the specified type (e.g., 0x03 for front cover, 0x04 for back cover).
 * Returns NULL and sets dataSize to 0 if metadata is NULL, no ID3v2 tag is present (regardless of preferred standard), or no matching picture is found.
 * Always returns NULL when only ID3v1 is available.
 * @param type - Picture type byte (0x00-0x14) to search for (e.g., 0x03 for front cover).
 * @param metadata - ID3 structure to read the picture from.
 * @param dataSize - Pointer to size_t to receive the size of the returned picture data in bytes (set to 0 on failure).
 * @return uint8_t* - Pointer to allocated binary picture data, or NULL if not found or ID3v2 not available. Caller must free the returned buffer.
 */
uint8_t *id3ReadPicture(uint8_t type, const ID3 *metadata, size_t *dataSize) {
    if (metadata == NULL) {
        *dataSize = 0;
        return NULL;
    }

    if (metadata->id3v2 == NULL && metadata->id3v1 == NULL) {
        *dataSize = 0;
        return NULL;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V1_TAG_VERSION:
            *dataSize = 0;
            return NULL;
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2ReadPicture(type, metadata->id3v2, dataSize);
        default:
            break;
    }

    *dataSize = 0;
    return NULL;
}

/**
 * @brief Writes a title to an ID3 metadata structure using the preferred standard.
 * @details Updates the title field in either the ID3v1 or ID3v2 tag based on the preferred standard setting.
 * If the preferred tag is not available, falls back to the available tag. Returns false on validation failures
 * (null parameters, both tags missing, or write errors) without modifying the metadata.
 * @param title - Null-terminated string containing the title to write.
 * @param metadata - ID3 structure to write the title to (must have at least one tag present).
 * @return int - 1 (true) if title written successfully, 0 (false) on failure.
 */
int id3WriteTitle(const char *title, ID3 *metadata) {
    if (metadata == NULL || title == NULL) {
        return false;
    }

    if (metadata->id3v1 == NULL && metadata->id3v2 == NULL) {
        return false;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V1_TAG_VERSION:
            return id3v1WriteTitle(title, metadata->id3v1);
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteTitle(title, metadata->id3v2);
        default:
            break;
    }

    return false;
}

/**
 * @brief Writes an artist to an ID3 metadata structure using the preferred standard.
 * @details Updates the artist field in either the ID3v1 or ID3v2 tag based on the preferred standard setting.
 * If the preferred tag is not available, falls back to the available tag. Returns false on validation failures
 * (null parameters, both tags missing, or write errors) without modifying the metadata.
 * @param artist - Null-terminated string containing the artist to write.
 * @param metadata - ID3 structure to write the artist to (must have at least one tag present).
 * @return int - 1 (true) if artist written successfully, 0 (false) on failure.
 */
int id3WriteArtist(const char *artist, ID3 *metadata) {
    if (metadata == NULL || artist == NULL) {
        return false;
    }

    if (metadata->id3v1 == NULL && metadata->id3v2 == NULL) {
        return false;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V1_TAG_VERSION:
            return id3v1WriteArtist(artist, metadata->id3v1);
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteArtist(artist, metadata->id3v2);
        default:
            break;
    }

    return false;
}

/**
 * @brief Writes an album artist to an ID3 metadata structure using the preferred standard.
 * @details Updates the album artist field in the ID3v2 tag only, as this field is not available in ID3v1.
 * Returns false on validation failures (null parameters, no ID3v2 tag present regardless of preferred standard, or write errors) without modifying the metadata.
 * Always returns false when only ID3v1 is available.
 * @param albumArtist - Null-terminated string containing the album artist to write.
 * @param metadata - ID3 structure to write the album artist to (must have ID3v2 tag present).
 * @return int - 1 (true) if album artist written successfully, 0 (false) on failure or ID3v2 not available.
 */
int id3WriteAlbumArtist(const char *albumArtist, ID3 *metadata) {
    if (metadata == NULL || albumArtist == NULL) {
        return false;
    }

    if (metadata->id3v1 == NULL && metadata->id3v2 == NULL) {
        return false;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteAlbumArtist(albumArtist, metadata->id3v2);
        case ID3V1_TAG_VERSION:
        default:
            break;
    }

    return false;
}

/**
 * @brief Writes an album to an ID3 metadata structure using the preferred standard.
 * @details Updates the album field in either the ID3v1 or ID3v2 tag based on the preferred standard setting.
 * If the preferred tag is not available, falls back to the available tag. Returns false on validation failures
 * (null parameters, both tags missing, or write errors) without modifying the metadata.
 * @param album - Null-terminated string containing the album to write.
 * @param metadata - ID3 structure to write the album to (must have at least one tag present).
 * @return int - 1 (true) if album written successfully, 0 (false) on failure.
 */
int id3WriteAlbum(const char *album, ID3 *metadata) {
    if (metadata == NULL || album == NULL) {
        return false;
    }

    if (metadata->id3v1 == NULL && metadata->id3v2 == NULL) {
        return false;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V1_TAG_VERSION:
            return id3v1WriteAlbum(album, metadata->id3v1);
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteAlbum(album, metadata->id3v2);
        default:
            break;
    }

    return false;
}

/**
 * @brief Writes a year to an ID3 metadata structure using the preferred standard.
 * @details Updates the year field in either the ID3v1 or ID3v2 tag based on the preferred standard setting.
 * For ID3v1, converts the year string to an integer using strtol. For ID3v2, writes the string directly.
 * If the preferred tag is not available, falls back to the available tag. Returns false on validation failures
 * (null parameters, both tags missing, or write errors) without modifying the metadata.
 * @param year - Null-terminated string containing the year to write (must be parseable as integer for ID3v1).
 * @param metadata - ID3 structure to write the year to (must have at least one tag present).
 * @return int - 1 (true) if year written successfully, 0 (false) on failure.
 */
int id3WriteYear(const char *year, ID3 *metadata) {
    if (metadata == NULL || year == NULL) {
        return false;
    }

    if (metadata->id3v1 == NULL && metadata->id3v2 == NULL) {
        return false;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V1_TAG_VERSION: {
            int convi = 0;
            char *end = NULL;
            convi = (int) strtol(year, &end, 10);
            return id3v1WriteYear(convi, metadata->id3v1);
        }
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteYear(year, metadata->id3v2);
        default:
            break;
    }

    return false;
}

/**
 * @brief Writes a genre to an ID3 metadata structure using the preferred standard.
 * @details Updates the genre field in either the ID3v1 or ID3v2 tag based on the preferred standard setting.
 * For ID3v1, uses the first byte of the genre string as a genre code (0-192 per standard genre table in id3v1Types.h), clamping values above PSYBIENT_GENRE to OTHER_GENRE.
 * For ID3v2, writes the genre string directly. If the preferred tag is not available, falls back to the available tag.
 * Returns false on validation failures (null parameters, both tags missing, or write errors) without modifying the metadata.
 * @param genre - Null-terminated string containing the genre to write (first byte used as genre code for ID3v1: 0-192).
 * @param metadata - ID3 structure to write the genre to (must have at least one tag present).
 * @return int - 1 (true) if genre written successfully, 0 (false) on failure.
 */
int id3WriteGenre(const char *genre, ID3 *metadata) {
    if (metadata == NULL || genre == NULL) {
        return false;
    }

    if (metadata->id3v1 == NULL && metadata->id3v2 == NULL) {
        return false;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V1_TAG_VERSION: {
            uint8_t usableGenre = (uint8_t) genre[0] > PSYBIENT_GENRE ? OTHER_GENRE : (uint8_t) genre[0];
            return id3v1WriteGenre(usableGenre, metadata->id3v1);
        }
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteGenre(genre, metadata->id3v2);
        default:
            break;
    }

    return false;
}

/**
 * @brief Writes a track number to an ID3 metadata structure using the preferred standard.
 * @details Updates the track number field in either the ID3v1 or ID3v2 tag based on the preferred standard setting.
 * For ID3v1, converts the track string to an integer using strtol and clamps the value to 0-255 (UINT8_MAX) range.
 * For ID3v2, writes the track string directly (may include format like "track/total"). If the preferred tag is not available, falls back to the available tag.
 * Returns false on validation failures (null parameters, both tags missing, or write errors) without modifying the metadata.
 * @param track - Null-terminated string containing the track number to write (must be parseable as integer for ID3v1, clamped to 0-255).
 * @param metadata - ID3 structure to write the track number to (must have at least one tag present).
 * @return int - 1 (true) if track written successfully, 0 (false) on failure.
 */
int id3WriteTrack(const char *track, ID3 *metadata) {
    if (metadata == NULL || track == NULL) {
        return false;
    }

    if (metadata->id3v1 == NULL && metadata->id3v2 == NULL) {
        return false;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V1_TAG_VERSION: {
            int convi = 0;
            char *end = NULL;
            convi = (int) strtol(track, &end, 10);

            if (convi > UINT8_MAX) {
                convi = UINT8_MAX;
            } else if (convi < 0) {
                convi = 0;
            }

            return id3v1WriteTrack(convi, metadata->id3v1);
        }
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteTrack(track, metadata->id3v2);
        default:
            break;
    }

    return false;
}

/**
 * @brief Writes a disc number to an ID3 metadata structure using the preferred standard.
 * @details Updates the disc number field in the ID3v2 tag only, as this field is not available in ID3v1.
 * Returns false on validation failures (null parameters, no ID3v2 tag present regardless of preferred standard, or write errors) without modifying the metadata.
 * Always returns false when only ID3v1 is available.
 * @param disc - Null-terminated string containing the disc number to write (e.g., "1", "1/2").
 * @param metadata - ID3 structure to write the disc number to (must have ID3v2 tag present).
 * @return int - 1 (true) if disc number written successfully, 0 (false) on failure or ID3v2 not available.
 */
int id3WriteDisc(const char *disc, ID3 *metadata) {
    if (metadata == NULL || disc == NULL) {
        return false;
    }

    if (metadata->id3v1 == NULL && metadata->id3v2 == NULL) {
        return false;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteDisc(disc, metadata->id3v2);
        case ID3V1_TAG_VERSION:
        default:
            break;
    }

    return false;
}

/**
 * @brief Writes a composer to an ID3 metadata structure using the preferred standard.
 * @details Updates the composer field in the ID3v2 tag only, as this field is not available in ID3v1.
 * Returns false on validation failures (null parameters, no ID3v2 tag present regardless of preferred standard, or write errors) without modifying the metadata.
 * Always returns false when only ID3v1 is available.
 * @param composer - Null-terminated string containing the composer to write.
 * @param metadata - ID3 structure to write the composer to (must have ID3v2 tag present).
 * @return int - 1 (true) if composer written successfully, 0 (false) on failure or ID3v2 not available.
 */
int id3WriteComposer(const char *composer, ID3 *metadata) {
    if (metadata == NULL || composer == NULL) {
        return false;
    }

    if (metadata->id3v1 == NULL && metadata->id3v2 == NULL) {
        return false;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteComposer(composer, metadata->id3v2);
        case ID3V1_TAG_VERSION:
        default:
            break;
    }

    return false;
}

/**
 * @brief Writes lyrics to an ID3 metadata structure using the preferred standard.
 * @details Updates the lyrics field in the ID3v2 tag only, as this field is not available in ID3v1.
 * Returns false on validation failures (null parameters, no ID3v2 tag present regardless of preferred standard, or write errors) without modifying the metadata.
 * Always returns false when only ID3v1 is available.
 * @param lyrics - Null-terminated string containing the lyrics to write.
 * @param metadata - ID3 structure to write the lyrics to (must have ID3v2 tag present).
 * @return int - 1 (true) if lyrics written successfully, 0 (false) on failure or ID3v2 not available.
 */
int id3WriteLyrics(const char *lyrics, ID3 *metadata) {
    if (metadata == NULL || lyrics == NULL) {
        return false;
    }

    if (metadata->id3v1 == NULL && metadata->id3v2 == NULL) {
        return false;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteLyrics(lyrics, metadata->id3v2);
        case ID3V1_TAG_VERSION:
        default:
            break;
    }

    return false;
}

/**
 * @brief Writes a comment to an ID3 metadata structure using the preferred standard.
 * @details Updates the comment field in either the ID3v1 or ID3v2 tag based on the preferred standard setting.
 * If the preferred tag is not available, falls back to the available tag. Returns false on validation failures
 * (null parameters, both tags missing, or write errors) without modifying the metadata.
 * @param comment - Null-terminated string containing the comment to write.
 * @param metadata - ID3 structure to write the comment to (must have at least one tag present).
 * @return int - 1 (true) if comment written successfully, 0 (false) on failure.
 */
int id3WriteComment(const char *comment, ID3 *metadata) {
    if (metadata == NULL || comment == NULL) {
        return false;
    }

    if (metadata->id3v1 == NULL && metadata->id3v2 == NULL) {
        return false;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V1_TAG_VERSION:
            return id3v1WriteComment(comment, metadata->id3v1);
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteComment(comment, metadata->id3v2);
        default:
            break;
    }

    return false;
}

/**
 * @brief Writes a picture to an ID3 metadata structure using the preferred standard.
 * @details Updates the picture field in the ID3v2 tag only, as this field is not available in ID3v1.
 * Returns false on validation failures (null parameters, no ID3v2 tag present regardless of preferred standard, or write errors) without modifying the metadata.
 * Always returns false when only ID3v1 is available.
 * @param image - Pointer to the binary image data to write.
 * @param imageSize - Size of the image data in bytes (must be greater than 0).
 * @param kind - Null-terminated string specifying image format (e.g., "jpeg", "png") used as MIME type suffix (must not be empty).
 * @param type - Picture type value (0x00-0x14, values above 0x14 are clamped to 0x00).
 * @param metadata - ID3 structure to write the picture to (must have ID3v2 tag present).
 * @return int - 1 (true) if picture written successfully, 0 (false) on failure or ID3v2 not available.
 */
int id3WritePicture(uint8_t *image, size_t imageSize, const char *kind, uint8_t type, ID3 *metadata) {
    if (metadata == NULL || image == NULL || kind == NULL) {
        return false;
    }

    if (metadata->id3v1 == NULL && metadata->id3v2 == NULL) {
        return false;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WritePicture(image, imageSize, kind, type, metadata->id3v2);
        case ID3V1_TAG_VERSION:
        default:
            break;
    }

    return false;
}

/**
 * @brief Writes a picture from a file to an ID3 metadata structure using the preferred standard.
 * @details Loads binary image data from the specified file and updates the picture field in the ID3v2 tag only, as this field is not available in ID3v1.
 * Returns false on validation failures (null parameters, no ID3v2 tag present regardless of preferred standard, file read errors, or write errors) without modifying the metadata.
 * Always returns false when only ID3v1 is available.
 * @param filename - Null-terminated string containing the path to the image file to read (must not be empty).
 * @param kind - Null-terminated string specifying image format (e.g., "jpeg", "png") used as MIME type suffix (must not be empty).
 * @param type - Picture type value (0x00-0x14, values above 0x14 are clamped to 0x00).
 * @param metadata - ID3 structure to write the picture to (must have ID3v2 tag present).
 * @return int - 1 (true) if picture read and written successfully, 0 (false) on failure or ID3v2 not available.
 */
int id3WritePictureFromFile(const char *filename, const char *kind, uint8_t type, ID3 *metadata) {
    if (metadata == NULL || filename == NULL || kind == NULL) {
        return false;
    }

    if (metadata->id3v1 == NULL && metadata->id3v2 == NULL) {
        return false;
    }

    switch (internal_getSafePrefStd(metadata)) {
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WritePictureFromFile(filename, kind, type, metadata->id3v2);
        case ID3V1_TAG_VERSION:
        default:
            break;
    }

    return false;
}

/**
 * @brief Converts an ID3 metadata structure to a JSON string.
 * @details Serializes both the ID3v1 and ID3v2 tags (if present) to JSON and combines them into a single JSON object.
 * Returns "{}" if the input metadata is NULL. The returned string is dynamically allocated and must be freed by the caller.
 * @param metadata - ID3 structure to serialize to JSON, or NULL.
 * @return char* - Pointer to allocated null-terminated JSON string, or "{}" if metadata is NULL. Caller must free the returned string.
 *
 * Example output:
 * ```json
 * {
 *   "ID3v1": { ... },
 *   "ID3v2": { ... }
 * }
 * ```
 */
char *id3ToJSON(const ID3 *metadata) {
    char *json = NULL;
    char *id3v1 = NULL;
    char *id3v2 = NULL;
    size_t memCount = 3;

    if (metadata == NULL) {
        json = calloc(memCount, sizeof(char));
        memcpy(json, "{}\0", memCount);
        return json;
    }

    id3v1 = id3v1ToJSON(metadata->id3v1);
    id3v2 = id3v2TagToJSON(metadata->id3v2);

    memCount += snprintf(NULL, 0, "{\"id3v1\":%s,\"id3v2\":%s}", id3v1, id3v2);
    json = calloc(memCount + 1, sizeof(char));
    if (json == NULL) {
        free(id3v1);
        free(id3v2);
        return NULL;
    }
    (void) snprintf(json, memCount + 1, "{\"ID3v1\":%s,\"ID3v2\":%s}", id3v1, id3v2);

    free(id3v1);
    free(id3v2);

    return json;
}

/**
 * @brief Writes both ID3v1 and ID3v2 tags to a file using the given ID3 structure.
 * @details Updates existing tags or creates new ones as needed. Writes both tags if present; if only one tag is present, only that tag is written.
 * Returns true if at least one tag is written successfully and the other is either also written or not present.
 * Returns false on validation failures (null parameters, both tags missing, or write errors for all tags).
 * @param filePath - Null-terminated string containing the path to the file to write.
 * @param metadata - ID3 structure containing the tags to write (must have at least one tag present).
 * @return int - 1 (true) if at least one tag written successfully, 0 (false) on failure.
 */
int id3WriteToFile(const char *filePath, const ID3 *metadata) {
    if (filePath == NULL || metadata == NULL) {
        return false;
    }

    // no point in trying
    if (metadata->id3v1 == NULL && metadata->id3v2 == NULL) {
        return false;
    }


    bool v1 = false;
    bool v2 = false;

    v1 = id3v1WriteTagToFile(filePath, metadata->id3v1);
    v2 = id3v2WriteTagToFile(filePath, metadata->id3v2);


    if (v1 && v2) {
        return true;
    }

    if (v1 == false && (v2 == true && metadata->id3v1 == NULL)) {
        return true;
    }

    if (v2 == false && (v1 == true && metadata->id3v2 == NULL)) {
        return true;
    }

    return false;
}
