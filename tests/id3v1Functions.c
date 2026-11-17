/**
 * @file id3v1Functions.c
 * @author Ewan Jones
 * @brief unit tests for id3v1
 * @version 0.1
 * @date 2024-03-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <stdio.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "id3v1/id3v1.h"
#include "id3v1/id3v1Parser.h"

void Id3v1TagPrintf(Id3v1Tag *tag) {
    printf("title: %s\n", (char *) tag->title);
    printf("artist: %s\n", (char *) tag->artist);
    printf("album: %s\n", (char *) tag->albumTitle);
    printf("year: %d\n", tag->year);
    printf("comment: %s\n", (char *) tag->comment);
    printf("track: %d\n", tag->track);
    printf("genre: %d\n", tag->genre);
}

static void id3v1HasTag_foundTag(void **state) {
    (void) state; /* unused */

    FILE *fp = fopen("assets/empty.mp3", "rb");
    unsigned char bytes[ID3V1_MAX_SIZE];
    (void) fseek(fp, -128, SEEK_END);
    (void) fread(bytes, 1, ID3V1_MAX_SIZE, fp);
    assert_true(id3v1HasTag((uint8_t *)bytes));
    fclose(fp);
}

static void id3v1HasTag_foundTag2(void **state) {
    (void) state; /* unused */

    FILE *fp = fopen("assets/OnGP.mp3", "rb");
    unsigned char bytes[ID3V1_MAX_SIZE];
    (void) fseek(fp, -128, SEEK_END);
    (void) fread(bytes, 1, ID3V1_MAX_SIZE, fp);
    assert_true(id3v1HasTag((uint8_t *)bytes));
    (void) fclose(fp);
}

static void id3v1HasTag_noTag(void **state) {
    (void) state; /* unused */

    FILE *fp = fopen("assets/null.mp3", "rb");
    unsigned char bytes[ID3V1_MAX_SIZE];
    (void) fseek(fp, -128, SEEK_END);
    (void) fread(bytes, 1, ID3V1_MAX_SIZE, fp);
    assert_false(id3v1HasTag((uint8_t *)bytes));
    (void) fclose(fp);
}

static void id3v1NewTag_validTag(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag = id3v1CreateTag((uint8_t *) "title",
                                   (uint8_t *) "artist",
                                   (uint8_t *) "album",
                                   2020,
                                   9,
                                   (uint8_t *) "comment",
                                   JAZZ_GENRE);

    assert_memory_equal(tag->title, "title", strlen("title"));
    assert_memory_equal(tag->artist, "artist", strlen("artist"));
    assert_memory_equal(tag->albumTitle, "album", strlen("album"));
    assert_int_equal(tag->year, 2020);
    assert_int_equal(tag->track, 9);
    assert_memory_equal(tag->comment, "comment", strlen("comment"));
    assert_int_equal(tag->genre, JAZZ_GENRE);

    free(tag);
}

static void id3v1NewTag_validTag2(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag = id3v1CreateTag((uint8_t *) "123456789012345678901234567890x",
                                   (uint8_t *) "123456789012345678901234567890x",
                                   (uint8_t *) "123456789012345678901234567890x",
                                   INT_MAX,
                                   127,
                                   (uint8_t *) "123456789012345678901234567890x",
                                   JAZZ_GENRE);

    assert_memory_equal(tag->title, "123456789012345678901234567890", strlen("123456789012345678901234567890"));
    assert_memory_equal(tag->artist, "123456789012345678901234567890", strlen("123456789012345678901234567890"));
    assert_memory_equal(tag->albumTitle, "123456789012345678901234567890", strlen("123456789012345678901234567890"));
    assert_int_equal(tag->year, INT_MAX);
    assert_int_equal(tag->track, 127);
    assert_memory_equal(tag->comment, "123456789012345678901234567890", strlen("123456789012345678901234567890"));
    assert_int_equal(tag->genre, JAZZ_GENRE);

    free(tag);
}

static void id3v1ClearTag_free(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag = id3v1CreateTag((uint8_t *) "title",
                                   (uint8_t *) "artist",
                                   (uint8_t *) "album",
                                   2020,
                                   9,
                                   (uint8_t *) "comment",
                                   JAZZ_GENRE);

    id3v1ClearTag(tag);

    assert_non_null(tag);
    assert_int_equal(tag->track, 0);
    assert_int_equal(tag->year, 0);
    assert_int_equal(tag->genre, 12);

    free(tag);
}

static void id3v1DestroyTag_free(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag = id3v1CreateTag((uint8_t *) "title",
                                   (uint8_t *) "artist",
                                   (uint8_t *) "album",
                                   2020,
                                   9,
                                   (uint8_t *) "comment",
                                   JAZZ_GENRE);

    id3v1DestroyTag(&tag);

    assert_null(tag);
}

static void id3v1TagFromBuffer_validBufferVersion11(void **state) {
    (void) state; /* unused */

    uint8_t buffer[] = {
        'T', 'A', 'G',
        /*title*/ 'n', 'e', 'w', ' ', 't', 'i', 't', 'l', 'e', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0,
        /*artist*/ 'n', 'e', 'w', ' ', 'a', 'r', 't', 'i', 's', 't', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0,
        /*album*/ 'n', 'e', 'w', ' ', 'a', 'l', 'b', 'u', 'm', ' ', 't', 'i', 't', 'l', 'e', 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        /*year*/ '1', '9', '9', '0',
        /*comment*/ 'n', 'e', 'w', ' ', 'c', 'o', 'm', 'm', 'e', 'n', 't', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0,
        /*track 1.1*/ 10,
        /*genre*/ 70
    };

    Id3v1Tag *tag = id3v1TagFromBuffer(buffer);

    assert_string_equal((char *)tag->title, "new title");
    assert_string_equal((char *)tag->artist, "new artist");
    assert_string_equal((char *)tag->albumTitle, "new album title");
    assert_int_equal(tag->year, 1990);
    assert_string_equal((char *)tag->comment, "new comment");
    assert_int_equal(tag->track, 10);
    assert_int_equal(tag->genre, 70);

    id3v1DestroyTag(&tag);
}

static void id3v1TagFromBuffer_validBufferVersion1(void **state) {
    (void) state; /* unused */

    uint8_t buffer[] = {
        'T', 'A', 'G',
        /*title*/ 'n', 'e', 'w', ' ', 't', 'i', 't', 'l', 'e', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0,
        /*artist*/ 'n', 'e', 'w', ' ', 'a', 'r', 't', 'i', 's', 't', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0,
        /*album*/ 'n', 'e', 'w', ' ', 'a', 'l', 'b', 'u', 'm', ' ', 't', 'i', 't', 'l', 'e', 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        /*year*/ '1', '9', '9', '0',
        /*comment*/ 'n', 'e', 'w', ' ', 'c', 'o', 'm', 'm', 'e', 'n', 't', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0,
        /*track 1.1*/ 0,
        /*genre*/ 70
    };

    Id3v1Tag *tag = id3v1TagFromBuffer(buffer);

    assert_string_equal((char *)tag->title, "new title");
    assert_string_equal((char *)tag->artist, "new artist");
    assert_string_equal((char *)tag->albumTitle, "new album title");
    assert_int_equal(tag->year, 1990);
    assert_string_equal((char *)tag->comment, "new comment");
    assert_int_equal(tag->track, 0);
    assert_int_equal(tag->genre, 70);

    id3v1DestroyTag(&tag);
}

static void id3v1TagFromBuffer_zeros(void **state) {
    (void) state; /* unused */

    uint8_t buffer[] = {
        0, 0, 0,
        /*title*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*artist*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*album*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*year*/ 0, 0, 0, 0,
        /*comment*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*track 1.1*/ 0,
        /*genre*/ 0
    };

    Id3v1Tag *tag = id3v1TagFromBuffer(buffer);

    assert_null(tag);
    id3v1DestroyTag(&tag);
}

static void id3v1TagFromBuffer_tagOnly(void **state) {
    (void) state; /* unused */

    uint8_t buffer[] = {
        'T', 'A', 'G',
        /*title*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*artist*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*album*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*year*/ 0, 0, 0, 0,
        /*comment*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*track 1.1*/ 0,
        /*genre*/ 0
    };

    Id3v1Tag *tag = id3v1TagFromBuffer(buffer);

    for (int i = 0; i < 30; i++) {
        assert_int_equal(tag->title[i], 0);
        assert_int_equal(tag->artist[i], 0);
        assert_int_equal(tag->albumTitle[i], 0);
        assert_int_equal(tag->comment[i], 0);
    }

    assert_int_equal(tag->year, 0);
    assert_int_equal(tag->track, 0);
    assert_int_equal(tag->genre, 0);
    id3v1DestroyTag(&tag);
}

static void id3v1TagFromBuffer_titleOnly(void **state) {
    (void) state; /* unused */

    uint8_t buffer[] = {
        'T', 'A', 'G',
        /*title*/ '7', ' ', 'R', 'i', 'n', 'g', 's', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0,
        /*artist*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*album*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*year*/ 0, 0, 0, 0,
        /*comment*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*track 1.1*/ 0,
        /*genre*/ 0
    };

    Id3v1Tag *tag = id3v1TagFromBuffer(buffer);

    for (int i = 0; i < 30; i++) {
        assert_int_equal(tag->artist[i], 0);
        assert_int_equal(tag->albumTitle[i], 0);
        assert_int_equal(tag->comment[i], 0);
    }

    assert_string_equal((char *)tag->title, "7 Rings");
    assert_int_equal(tag->year, 0);
    assert_int_equal(tag->track, 0);
    assert_int_equal(tag->genre, 0);
    id3v1DestroyTag(&tag);
}

static void id3v1TagFromBuffer_artistOnly(void **state) {
    (void) state; /* unused */

    uint8_t buffer[] = {
        'T', 'A', 'G',
        /*title*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*artist*/ 'B', 'l', 'a', 'c', 'k', ' ', 'C', 'o', 'u', 'n', 't', 'r', 'y', ',', ' ', 'N', 'e', 'w', ' ', 'R',
        'o', 'a', 'd', 0, 0, 0, 0, 0, 0, 0,
        /*album*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*year*/ 0, 0, 0, 0,
        /*comment*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*track 1.1*/ 0,
        /*genre*/ 0
    };

    Id3v1Tag *tag = id3v1TagFromBuffer(buffer);

    for (int i = 0; i < 30; i++) {
        assert_int_equal(tag->title[i], 0);
        assert_int_equal(tag->albumTitle[i], 0);
        assert_int_equal(tag->comment[i], 0);
    }

    assert_string_equal((char *)tag->artist, "Black Country, New Road");
    assert_int_equal(tag->year, 0);
    assert_int_equal(tag->track, 0);
    assert_int_equal(tag->genre, 0);
    id3v1DestroyTag(&tag);
}

static void id3v1TagFromBuffer_albumOnly(void **state) {
    (void) state; /* unused */

    uint8_t buffer[] = {
        'T', 'A', 'G',
        /*title*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*artist*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*album*/ 'X', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*year*/ 0, 0, 0, 0,
        /*comment*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*track 1.1*/ 0,
        /*genre*/ 0
    };

    Id3v1Tag *tag = id3v1TagFromBuffer(buffer);

    for (int i = 0; i < 30; i++) {
        assert_int_equal(tag->title[i], 0);
        assert_int_equal(tag->artist[i], 0);
        assert_int_equal(tag->comment[i], 0);
    }
    assert_string_equal((char *)tag->albumTitle, "X");
    assert_int_equal(tag->year, 0);
    assert_int_equal(tag->track, 0);
    assert_int_equal(tag->genre, 0);
    id3v1DestroyTag(&tag);
}

static void id3v1TagFromBuffer_yearOnly(void **state) {
    (void) state; /* unused */

    uint8_t buffer[] = {
        'T', 'A', 'G',
        /*title*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*artist*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*album*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*year*/ '1', '9', '6', '3',
        /*comment*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*track 1.1*/ 0,
        /*genre*/ 0
    };

    Id3v1Tag *tag = id3v1TagFromBuffer(buffer);

    for (int i = 0; i < 30; i++) {
        assert_int_equal(tag->title[i], 0);
        assert_int_equal(tag->artist[i], 0);
        assert_int_equal(tag->albumTitle[i], 0);
        assert_int_equal(tag->comment[i], 0);
    }

    assert_int_equal(tag->year, 1963);
    assert_int_equal(tag->track, 0);
    assert_int_equal(tag->genre, 0);
    id3v1DestroyTag(&tag);
}

static void id3v1TagFromBuffer_commentOnly(void **state) {
    (void) state; /* unused */

    uint8_t buffer[] = {
        'T', 'A', 'G',
        /*title*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*artist*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*album*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*year*/ 0, 0, 0, 0,
        /*comment*/ 't', 'h', 'e', ' ', 'l', 'a', 't', 'e', 's', 't', ' ', 'l', 'a', 'n', 'a', ' ', 'd', 'o', 'n', '\'',
        't', ' ', 'f', 'i', 't', '>', ':', '(', 0,
        /*track 1.1*/ 0,
        /*genre*/ 0
    };

    Id3v1Tag *tag = id3v1TagFromBuffer(buffer);

    for (int i = 0; i < 30; i++) {
        assert_int_equal(tag->title[i], 0);
        assert_int_equal(tag->artist[i], 0);
        assert_int_equal(tag->albumTitle[i], 0);
    }

    assert_string_equal((char *)tag->comment, "the latest lana don\'t fit>:(");
    assert_int_equal(tag->year, 0);
    assert_int_equal(tag->track, 0);
    assert_int_equal(tag->genre, 0);
    id3v1DestroyTag(&tag);
}

static void id3v1TagFromBuffer_trackOnly(void **state) {
    (void) state; /* unused */

    uint8_t buffer[] = {
        'T', 'A', 'G',
        /*title*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*artist*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*album*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*year*/ 0, 0, 0, 0,
        /*comment*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*track 1.1*/ 127,
        /*genre*/ 0
    };

    Id3v1Tag *tag = id3v1TagFromBuffer(buffer);

    for (int i = 0; i < 30; i++) {
        assert_int_equal(tag->title[i], 0);
        assert_int_equal(tag->artist[i], 0);
        assert_int_equal(tag->albumTitle[i], 0);
        assert_int_equal(tag->comment[i], 0);
    }

    assert_int_equal(tag->year, 0);
    assert_int_equal(tag->track, 127);
    assert_int_equal(tag->genre, 0);
    id3v1DestroyTag(&tag);
}

static void id3v1TagFromBuffer_genreOnly(void **state) {
    (void) state; /* unused */

    uint8_t buffer[] = {
        'T', 'A', 'G',
        /*title*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*artist*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*album*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*year*/ 0, 0, 0, 0,
        /*comment*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*track 1.1*/ 0,
        /*genre*/ 255
    };

    Id3v1Tag *tag = id3v1TagFromBuffer(buffer);

    for (int i = 0; i < 30; i++) {
        assert_int_equal(tag->title[i], 0);
        assert_int_equal(tag->artist[i], 0);
        assert_int_equal(tag->albumTitle[i], 0);
        assert_int_equal(tag->comment[i], 0);
    }

    assert_int_equal(tag->year, 0);
    assert_int_equal(tag->track, 0);
    assert_int_equal(tag->genre, 255);
    id3v1DestroyTag(&tag);
}

static void id3v1TagFromFile_readNull(void **state) {
    (void) state; /* unused */


    Id3v1Tag *tag = id3v1TagFromFile("assets/null.mp3");

    assert_null(tag);

    id3v1DestroyTag(&tag);
}

static void id3v1TagFromFile_nullPath(void **state) {
    (void) state; /* unused */


    Id3v1Tag *tag = id3v1TagFromFile(NULL);

    assert_null(tag);

    id3v1DestroyTag(&tag);
}

static void id3v1TagFromFile_readValidTag(void **state) {
    (void) state; /* unused */


    Id3v1Tag *tag = id3v1TagFromFile("assets/dannybrown.mp3");

    assert_non_null(tag);
    assert_string_equal((char *)tag->title, "Ain't It Funny");
    assert_string_equal((char *)tag->artist, "Danny Brown");
    assert_string_equal((char *)tag->albumTitle, "Atrocity Exhibition");
    assert_int_equal(tag->year, 2016);
    assert_int_equal(tag->track, 6);
    assert_int_equal(tag->genre, 255);
    id3v1DestroyTag(&tag);
}

static void id3v1WriteTitle_WithNUll(void **state) {
    (void) state; /* unused */

    assert_false(id3v1WriteTitle(NULL, NULL));
}

static void id3v1WriteTitle_WithNullTitle(void **state) {
    (void) state; /* unused */


    Id3v1Tag *tag = id3v1CreateTag((uint8_t *) "this is a title of a song",
                                   NULL,
                                   NULL,
                                   0,
                                   0,
                                   NULL,
                                   0);

    assert_true(id3v1WriteTitle(NULL, tag));

    for (int i = 0; i < ID3V1_FIELD_SIZE; i++) {
        assert_int_equal(tag->title[i], 0);
    }

    id3v1DestroyTag(&tag);
}

static void id3v1WriteTitle_WithBigTitle(void **state) {
    (void) state; /* unused */


    Id3v1Tag *tag = id3v1CreateTag((uint8_t *) "this is a title of a song",
                                   NULL,
                                   NULL,
                                   0,
                                   0,
                                   NULL,
                                   0);

    assert_true(id3v1WriteTitle("this is a string that is over 30 bytes", tag));

    assert_string_equal((char *)tag->title, "this is a string that is over ");


    id3v1DestroyTag(&tag);
}

static void id3v1WriteTitle_WithSmallTitle(void **state) {
    (void) state; /* unused */


    Id3v1Tag *tag = id3v1CreateTag((uint8_t *) "this is a title of a song",
                                   NULL,
                                   NULL,
                                   0,
                                   0,
                                   NULL,
                                   0);

    assert_true(id3v1WriteTitle("", tag));

    for (int i = 0; i < ID3V1_FIELD_SIZE; i++) {
        assert_int_equal(tag->title[i], 0);
    }


    id3v1DestroyTag(&tag);
}

static void id3v1WriteYear_save0(void **state) {
    (void) state; /* unused */


    Id3v1Tag *tag = id3v1CreateTag(NULL,
                                   NULL,
                                   NULL,
                                   2001,
                                   0,
                                   NULL,
                                   0);

    assert_true(id3v1WriteYear(0, tag));

    assert_int_equal(0, tag->year);

    id3v1DestroyTag(&tag);
}

static void id3v1WriteYear_saveBig(void **state) {
    (void) state; /* unused */


    Id3v1Tag *tag = id3v1CreateTag(NULL,
                                   NULL,
                                   NULL,
                                   0,
                                   0,
                                   NULL,
                                   0);

    assert_true(id3v1WriteYear(INT_MAX, tag));

    assert_int_equal(INT_MAX, tag->year);

    id3v1DestroyTag(&tag);
}

static void id3v1CompareTag_noTags(void **state) {
    (void) state; /* unused */

    assert_false(id3v1CompareTag(NULL, NULL));
}

static void id3v1CompareTag_oneTag(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag = id3v1CreateTag(NULL,
                                   NULL,
                                   NULL,
                                   0,
                                   0,
                                   NULL,
                                   0);


    assert_false(id3v1CompareTag(tag, NULL));

    id3v1DestroyTag(&tag);
}

static void id3v1CompareTag_diffGenre(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag1 = id3v1CreateTag(NULL,
                                    NULL,
                                    NULL,
                                    0,
                                    0,
                                    NULL,
                                    NOISE_GENRE);

    Id3v1Tag *tag2 = id3v1CreateTag(NULL,
                                    NULL,
                                    NULL,
                                    0,
                                    0,
                                    NULL,
                                    POP_GENRE);


    assert_false(id3v1CompareTag(tag1, tag2));

    id3v1DestroyTag(&tag1);
    id3v1DestroyTag(&tag2);
}

static void id3v1CompareTag_diffComment(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag1 = id3v1CreateTag(NULL,
                                    NULL,
                                    NULL,
                                    0,
                                    0,
                                    (uint8_t *) "this is the worst",
                                    0);

    Id3v1Tag *tag2 = id3v1CreateTag(NULL,
                                    NULL,
                                    NULL,
                                    0,
                                    0,
                                    (uint8_t *) "this is the best",
                                    0);


    assert_false(id3v1CompareTag(tag1, tag2));

    id3v1DestroyTag(&tag1);
    id3v1DestroyTag(&tag2);
}

static void id3v1CompareTag_diffTrack(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag1 = id3v1CreateTag(NULL,
                                    NULL,
                                    NULL,
                                    0,
                                    12,
                                    NULL,
                                    0);

    Id3v1Tag *tag2 = id3v1CreateTag(NULL,
                                    NULL,
                                    NULL,
                                    0,
                                    30,
                                    NULL,
                                    0);


    assert_false(id3v1CompareTag(tag1, tag2));

    id3v1DestroyTag(&tag1);
    id3v1DestroyTag(&tag2);
}

static void id3v1CompareTag_diffYear(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag1 = id3v1CreateTag(NULL,
                                    NULL,
                                    NULL,
                                    1800,
                                    0,
                                    NULL,
                                    0);

    Id3v1Tag *tag2 = id3v1CreateTag(NULL,
                                    NULL,
                                    NULL,
                                    2023,
                                    0,
                                    NULL,
                                    0);


    assert_false(id3v1CompareTag(tag1, tag2));

    id3v1DestroyTag(&tag1);
    id3v1DestroyTag(&tag2);
}

static void id3v1CompareTag_diffAlbum(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag1 = id3v1CreateTag(NULL,
                                    NULL,
                                    (uint8_t *) "the money store",
                                    0,
                                    0,
                                    NULL,
                                    0);

    Id3v1Tag *tag2 = id3v1CreateTag(NULL,
                                    NULL,
                                    (uint8_t *) "speak now",
                                    0,
                                    0,
                                    NULL,
                                    0);


    assert_false(id3v1CompareTag(tag1, tag2));

    id3v1DestroyTag(&tag1);
    id3v1DestroyTag(&tag2);
}

static void id3v1CompareTag_diffArtist(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag1 = id3v1CreateTag(NULL,
                                    (uint8_t *) "alvvays",
                                    NULL,
                                    0,
                                    0,
                                    NULL,
                                    0);

    Id3v1Tag *tag2 = id3v1CreateTag(NULL,
                                    (uint8_t *) "Lana del ray",
                                    NULL,
                                    0,
                                    0,
                                    NULL,
                                    0);


    assert_false(id3v1CompareTag(tag1, tag2));

    id3v1DestroyTag(&tag1);
    id3v1DestroyTag(&tag2);
}

static void id3v1CompareTag_diffTitle(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag1 = id3v1CreateTag((uint8_t *) "1999",
                                    NULL,
                                    NULL,
                                    0,
                                    0,
                                    NULL,
                                    0);

    Id3v1Tag *tag2 = id3v1CreateTag((uint8_t *) "thank u, next",
                                    NULL,
                                    NULL,
                                    0,
                                    0,
                                    NULL,
                                    0);


    assert_false(id3v1CompareTag(tag1, tag2));

    id3v1DestroyTag(&tag1);
    id3v1DestroyTag(&tag2);
}

static void id3v1CompareTag_same(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag1 = id3v1CreateTag((uint8_t *) "1999",
                                    (uint8_t *) "charli xcx",
                                    (uint8_t *) "charli",
                                    4,
                                    2019,
                                    NULL,
                                    POP_GENRE);

    Id3v1Tag *tag2 = id3v1CreateTag((uint8_t *) "1999",
                                    (uint8_t *) "charli xcx",
                                    (uint8_t *) "charli",
                                    4,
                                    2019,
                                    NULL,
                                    POP_GENRE);

    assert_true(id3v1CompareTag(tag1, tag2));

    id3v1DestroyTag(&tag1);
    id3v1DestroyTag(&tag2);
}

static void id3v1GenreFromTable_checkNoNull(void **state) {
    (void) state; /* unused */

    for (int i = 0; i < 255; i++) {
        assert_non_null(id3v1GenreFromTable(i));
    }
}

static void id3v1GenreFromTable_checkForHipHopGenre(void **state) {
    (void) state; /* unused */
    assert_string_equal(id3v1GenreFromTable(HIP_HOP_GENRE), "Hip-Hop");
}

// every genre except OTHER_GENRE has its own name, so no enum value falls through to "Other"
static void id3v1GenreFromTable_everyGenreNamed(void **state) {
    (void) state; /* unused */

    for (int i = BLUES_GENRE; i <= PSYBIENT_GENRE; i++) {
        if (i == OTHER_GENRE) {
            continue;
        }
        assert_string_not_equal(id3v1GenreFromTable(i), "Other");
    }

    assert_string_equal(id3v1GenreFromTable(CHRISTMAS_GENRE), "Christmas");
    assert_string_equal(id3v1GenreFromTable(BHANGRA_GENRE), "Bhangra");

    // 255, the ID3v1 value for no genre, has its own name too
    assert_string_equal(id3v1GenreFromTable(GENRE_UNKNOWN), "Unknown");
    assert_string_equal(id3v1GenreFromTable(OTHER_GENRE), "Other");
}

static void id3v1ToJSON_fullTag(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag = id3v1CreateTag((uint8_t *) "1999",
                                   (uint8_t *) "charli xcx",
                                   (uint8_t *) "charli",
                                   4,
                                   2019,
                                   (uint8_t *) "pretty good song",
                                   POP_GENRE);


    char *json = id3v1ToJSON(tag);


    assert_non_null(json);

    assert_string_equal(json,
                        "{\"title\":\"1999\",\"artist\":\"charli xcx\",\"album\":\"charli\",\"year\":4,\"track\":2019,\"comment\":\"pretty good song\",\"genreNumber\":13,\"genre\":\"Pop\"}");


    free(json);
    id3v1DestroyTag(&tag);
}

static void id3v1ToJSON_noGenre(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag = id3v1CreateTag((uint8_t *) "1999",
                                   (uint8_t *) "charli xcx",
                                   (uint8_t *) "charli",
                                   4,
                                   2019,
                                   (uint8_t *) "pretty good song",
                                   12);


    char *json = id3v1ToJSON(tag);


    assert_non_null(json);
    assert_string_equal(json,
                        "{\"title\":\"1999\",\"artist\":\"charli xcx\",\"album\":\"charli\",\"year\":4,\"track\":2019,\"comment\":\"pretty good song\",\"genreNumber\":12,\"genre\":\"Other\"}");


    free(json);
    id3v1DestroyTag(&tag);
}

static void id3v1ToJSON_noYear(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag = id3v1CreateTag((uint8_t *) "1999",
                                   (uint8_t *) "charli xcx",
                                   (uint8_t *) "charli",
                                   4,
                                   0,
                                   (uint8_t *) "pretty good song",
                                   POP_GENRE);


    char *json = id3v1ToJSON(tag);


    assert_non_null(json);

    assert_string_equal(json,
                        "{\"title\":\"1999\",\"artist\":\"charli xcx\",\"album\":\"charli\",\"year\":4,\"track\":0,\"comment\":\"pretty good song\",\"genreNumber\":13,\"genre\":\"Pop\"}");


    free(json);
    id3v1DestroyTag(&tag);
}

static void id3v1ToJSON_noTitle(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag = id3v1CreateTag(NULL,
                                   (uint8_t *) "charli xcx",
                                   (uint8_t *) "charli",
                                   4,
                                   2019,
                                   (uint8_t *) "pretty good song",
                                   POP_GENRE);


    char *json = id3v1ToJSON(tag);


    assert_non_null(json);

    assert_string_equal(json,
                        "{\"title\":\"\",\"artist\":\"charli xcx\",\"album\":\"charli\",\"year\":4,\"track\":2019,\"comment\":\"pretty good song\",\"genreNumber\":13,\"genre\":\"Pop\"}");


    free(json);
    id3v1DestroyTag(&tag);
}

static void id3v1WriteTagToFile_noInputs(void **state) {
    (void) state; /* unused */

    assert_false(id3v1WriteTagToFile(NULL, NULL));
}

static void id3v1WriteTagToFile_CreateFile(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag = id3v1CreateTag((uint8_t *) "1999",
                                   (uint8_t *) "charli xcx",
                                   (uint8_t *) "charli",
                                   2019,
                                   4,
                                   (uint8_t *) "pretty good song",
                                   POP_GENRE);
    Id3v1Tag *tag2 = NULL;

    assert_true(id3v1WriteTagToFile("test.mp3", tag));
    tag2 = id3v1TagFromFile("test.mp3");
    assert_non_null(tag2);

    assert_string_equal((char *)tag2->title, "1999");
    assert_string_equal((char *)tag2->artist, "charli xcx");
    assert_string_equal((char *)tag2->albumTitle, "charli");
    assert_int_equal(tag2->year, 2019);
    assert_string_equal((char *)tag2->comment, "pretty good song");
    assert_int_equal(tag2->track, 4);
    assert_int_equal(tag2->genre, POP_GENRE);
    id3v1DestroyTag(&tag);
    id3v1DestroyTag(&tag2);

    (void) remove("test.mp3");
}

// years with fewer than 4 digits (including 0, no year) must not read past the converted digits
static void id3v1WriteTagToFile_shortYears(void **state) {
    (void) state; /* unused */

    const int years[] = {0, 7, 999, 2024, 12345};
    const int expected[] = {0, 7, 999, 2024, 1234};

    for (size_t i = 0; i < sizeof(years) / sizeof(years[0]); i++) {
        Id3v1Tag *tag = id3v1CreateTag((uint8_t *) "title", NULL, NULL, years[i], 0, NULL, OTHER_GENRE);
        assert_non_null(tag);

        assert_true(id3v1WriteTagToFile("year.mp3", tag));
        Id3v1Tag *read = id3v1TagFromFile("year.mp3");
        assert_non_null(read);
        assert_int_equal(read->year, expected[i]);
        assert_string_equal((char *) read->title, "title");

        id3v1DestroyTag(&tag);
        id3v1DestroyTag(&read);
        (void) remove("year.mp3");
    }
}

static void id3v1WriteTagToFile_editExistingFile(void **state) {
    (void) state; /* unused */

    Id3v1Tag *preTag = id3v1CreateTag((uint8_t *) "1999",
                                      (uint8_t *) "charli xcx",
                                      (uint8_t *) "charli",
                                      2019,
                                      4,
                                      (uint8_t *) "pretty good song",
                                      POP_GENRE);


    Id3v1Tag *tag2 = id3v1CreateTag((uint8_t *) "Headlines",
                                    (uint8_t *) "Drake",
                                    (uint8_t *) "Take Care",
                                    2011,
                                    3,
                                    NULL,
                                    RAP_GENRE);
    Id3v1Tag *readTag = NULL;

    assert_true(id3v1WriteTagToFile("test.mp3", preTag));
    assert_true(id3v1WriteTagToFile("test.mp3", tag2));

    readTag = id3v1TagFromFile("test.mp3");
    assert_non_null(readTag);

    assert_string_equal((char *)readTag->title, "Headlines");
    assert_string_equal((char *)readTag->artist, "Drake");
    assert_string_equal((char *)readTag->albumTitle, "Take Care");
    assert_int_equal(readTag->year, 2011);
    for (int i = 0; i < ID3V1_FIELD_SIZE; i++) {
        assert_int_equal(readTag->comment[i], 0);
    }
    assert_int_equal(readTag->track, 3);
    assert_int_equal(readTag->genre, RAP_GENRE);

    id3v1DestroyTag(&preTag);
    id3v1DestroyTag(&tag2);
    id3v1DestroyTag(&readTag);

    (void) remove("test.mp3");
}

static void id3v1WriteTagToFile_appendFile(void **state) {
    (void) state; /* unused */

    FILE *fp = fopen("test.mp3", "w");

    (void) fwrite("do not overwrite me please", 1, 27, fp);
    (void) fclose(fp);


    Id3v1Tag *tag2 = id3v1CreateTag((uint8_t *) "Headlines",
                                    (uint8_t *) "Drake",
                                    (uint8_t *) "Take Care",
                                    2011,
                                    3,
                                    NULL,
                                    RAP_GENRE);
    Id3v1Tag *readTag = NULL;

    assert_true(id3v1WriteTagToFile("test.mp3", tag2));

    readTag = id3v1TagFromFile("test.mp3");
    assert_non_null(readTag);

    assert_string_equal((char *)readTag->title, "Headlines");
    assert_string_equal((char *)readTag->artist, "Drake");
    assert_string_equal((char *)readTag->albumTitle, "Take Care");
    assert_int_equal(readTag->year, 2011);
    for (int i = 0; i < ID3V1_FIELD_SIZE; i++) {
        assert_int_equal(readTag->comment[i], 0);
    }
    assert_int_equal(readTag->track, 3);
    assert_int_equal(readTag->genre, RAP_GENRE);

    id3v1DestroyTag(&tag2);
    id3v1DestroyTag(&readTag);

    (void) remove("test.mp3");
}

static void id3v1WriteTagToFile_appendFileBig(void **state) {
    (void) state; /* unused */

    FILE *fp = fopen("test.mp3", "w");

    (void) fwrite(
        "oiejvpeinvpwiuevnpiwernvpiwernvpiweornvpoiwernvpoewinvoipwenvpoewinveiowvneowpnvewionveopwinvreoiwnrvoewmldakcmsdkfnvjkfenwviuerpieojvweirjv49fu980hv4tubvonufikldockc0924-9r934u8r234funeijdckdl",
        1, 194, fp);
    (void) fclose(fp);


    Id3v1Tag *tag2 = id3v1CreateTag((uint8_t *) "Headlines",
                                    (uint8_t *) "Drake",
                                    (uint8_t *) "Take Care",
                                    2011,
                                    3,
                                    NULL,
                                    RAP_GENRE);
    Id3v1Tag *readTag = NULL;

    assert_true(id3v1WriteTagToFile("test.mp3", tag2));

    readTag = id3v1TagFromFile("test.mp3");
    assert_non_null(readTag);

    assert_string_equal((char *)readTag->title, "Headlines");
    assert_string_equal((char *)readTag->artist, "Drake");
    assert_string_equal((char *)readTag->albumTitle, "Take Care");
    assert_int_equal(readTag->year, 2011);
    for (int i = 0; i < ID3V1_FIELD_SIZE; i++) {
        assert_int_equal(readTag->comment[i], 0);
    }
    assert_int_equal(readTag->track, 3);
    assert_int_equal(readTag->genre, RAP_GENRE);

    id3v1DestroyTag(&tag2);
    id3v1DestroyTag(&readTag);

    (void) remove("test.mp3");
}

// reads the last ID3V1_MAX_SIZE bytes of a file
static void internal_readTail(const char *path, uint8_t out[ID3V1_MAX_SIZE]) {
    FILE *fp = fopen(path, "rb");
    assert_non_null(fp);
    assert_int_equal(fseek(fp, -ID3V1_MAX_SIZE, SEEK_END), 0);
    assert_int_equal(fread(out, 1, ID3V1_MAX_SIZE, fp), ID3V1_MAX_SIZE);
    (void) fclose(fp);
}

// ID3v1.1: with a track, a long comment is cut to 28 bytes and byte 125 is 0x00 so the track is kept
static void id3v1WriteTagToFile_longCommentWithTrack(void **state) {
    (void) state; /* unused */

    const char *comment = "cccccccccccccccccccccccccccccccccccccccc"; // 40 bytes
    uint8_t tail[ID3V1_MAX_SIZE];
    Id3v1Tag *tag = id3v1CreateTag((uint8_t *) "title", NULL, NULL, 0, 5, (uint8_t *) comment, OTHER_GENRE);
    assert_non_null(tag);

    assert_true(id3v1WriteTagToFile("v11.mp3", tag));
    internal_readTail("v11.mp3", tail);

    assert_memory_equal(tail + 97, comment, 28);
    assert_int_equal(tail[125], 0x00);
    assert_int_equal(tail[126], 5);
    assert_int_equal(tail[127], OTHER_GENRE);

    Id3v1Tag *read = id3v1TagFromFile("v11.mp3");
    assert_non_null(read);
    assert_int_equal(read->track, 5);
    assert_memory_equal(read->comment, comment, 28);
    assert_int_equal(read->comment[28], 0x00);
    assert_int_equal(read->comment[29], 0x00);

    id3v1DestroyTag(&read);
    id3v1DestroyTag(&tag);
    (void) remove("v11.mp3");
}

// ID3v1.0: with no track the full 30 byte comment is kept
static void id3v1WriteTagToFile_longCommentNoTrack(void **state) {
    (void) state; /* unused */

    const char *comment = "cccccccccccccccccccccccccccccccccccccccc"; // 40 bytes
    uint8_t tail[ID3V1_MAX_SIZE];
    Id3v1Tag *tag = id3v1CreateTag((uint8_t *) "title", NULL, NULL, 0, 0, (uint8_t *) comment, OTHER_GENRE);
    assert_non_null(tag);

    assert_true(id3v1WriteTagToFile("v10.mp3", tag));
    internal_readTail("v10.mp3", tail);

    assert_memory_equal(tail + 97, comment, ID3V1_FIELD_SIZE);

    Id3v1Tag *read = id3v1TagFromFile("v10.mp3");
    assert_non_null(read);
    assert_int_equal(read->track, 0);
    assert_memory_equal(read->comment, comment, ID3V1_FIELD_SIZE);

    id3v1DestroyTag(&read);
    id3v1DestroyTag(&tag);
    (void) remove("v10.mp3");
}

// tracks 1-255 are written as ID3v1.1, anything else as ID3v1.0 with byte 126 left as comment
static void id3v1WriteTagToFile_trackRange(void **state) {
    (void) state; /* unused */

    const int tracks[] = {1, 255, 256, -1};
    const int expected[] = {1, 255, 0, 0};
    uint8_t tail[ID3V1_MAX_SIZE];

    for (size_t i = 0; i < sizeof(tracks) / sizeof(tracks[0]); i++) {
        Id3v1Tag *tag = id3v1CreateTag((uint8_t *) "title", NULL, NULL, 0, tracks[i],
                                       (uint8_t *) "cccccccccccccccccccccccccccccc", OTHER_GENRE);
        assert_non_null(tag);

        assert_true(id3v1WriteTagToFile("track.mp3", tag));
        internal_readTail("track.mp3", tail);

        if (expected[i] != 0) {
            assert_int_equal(tail[125], 0x00);
            assert_int_equal(tail[126], expected[i]);
        } else {
            assert_int_equal(tail[125], 'c');
            assert_int_equal(tail[126], 'c');
        }

        Id3v1Tag *read = id3v1TagFromFile("track.mp3");
        assert_non_null(read);
        assert_int_equal(read->track, expected[i]);

        id3v1DestroyTag(&read);
        id3v1DestroyTag(&tag);
        (void) remove("track.mp3");
    }
}

// ID3v1 text is Latin-1; the API is UTF-8
static void id3v1WriteTitle_utf8ToLatin1(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag = id3v1CreateTag(NULL, NULL, NULL, 0, 0, NULL, OTHER_GENRE);
    assert_non_null(tag);

    assert_true(id3v1WriteTitle("H\xc3\xa9" "ctor", tag)); // Héctor
    assert_memory_equal(tag->title, "H\xe9" "ctor\0", 7);

    char *str = id3v1ReadTitle(tag);
    assert_string_equal(str, "H\xc3\xa9" "ctor");
    free(str);

    id3v1DestroyTag(&tag);
}

static void id3v1WriteArtist_notLatin1(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag = id3v1CreateTag(NULL, NULL, NULL, 0, 0, NULL, OTHER_GENRE);
    assert_non_null(tag);

    // 가€x -> ??x
    assert_true(id3v1WriteArtist("\xea\xb0\x80\xe2\x82\xac" "x", tag));
    assert_memory_equal(tag->artist, "??x\0", 4);

    // Ünïcödé € -> Latin-1 bytes then ?
    assert_true(id3v1WriteArtist("\xc3\x9cn\xc3\xaf" "c\xc3\xb6" "d\xc3\xa9 \xe2\x82\xac", tag));
    assert_memory_equal(tag->artist, "\xdcn\xef" "c\xf6" "d\xe9 ?\0", 10);

    // U+1F600 (4 byte UTF-8) -> ?
    assert_true(id3v1WriteArtist("a\xf0\x9f\x98\x80" "b", tag));
    assert_memory_equal(tag->artist, "a?b\0", 4);

    id3v1DestroyTag(&tag);
}

static void id3v1WriteAlbum_invalidUtf8(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag = id3v1CreateTag(NULL, NULL, NULL, 0, 0, NULL, OTHER_GENRE);
    assert_non_null(tag);

    // raw Latin-1 is not UTF-8
    assert_true(id3v1WriteAlbum("\xe9t\xe9", tag));
    assert_memory_equal(tag->albumTitle, "?t?\0", 4);

    // stray continuation byte
    assert_true(id3v1WriteAlbum("\x80z", tag));
    assert_memory_equal(tag->albumTitle, "?z\0", 3);

    // truncated sequence at the end, then before an ASCII byte
    assert_true(id3v1WriteAlbum("a\xe2\x82", tag));
    assert_memory_equal(tag->albumTitle, "a?\0", 3);
    assert_true(id3v1WriteAlbum("\xe2\x82q", tag));
    assert_memory_equal(tag->albumTitle, "?q\0", 3);

    // overlong NUL and a surrogate
    assert_true(id3v1WriteAlbum("\xc0\x80" "b\xed\xa0\x80", tag));
    assert_memory_equal(tag->albumTitle, "?b?\0", 4);

    id3v1DestroyTag(&tag);
}

static void id3v1WriteComment_truncatesAfterConversion(void **state) {
    (void) state; /* unused */

    char in[81] = {0};
    char expected[61] = {0};
    Id3v1Tag *tag = id3v1CreateTag(NULL, NULL, NULL, 0, 0, NULL, OTHER_GENRE);
    assert_non_null(tag);

    // 40 x é is 80 UTF-8 bytes but 40 Latin-1 bytes, so 30 fit
    for (int i = 0; i < 40; i++) {
        in[i * 2] = (char) 0xc3;
        in[i * 2 + 1] = (char) 0xa9;
    }

    for (int i = 0; i < 30; i++) {
        expected[i * 2] = (char) 0xc3;
        expected[i * 2 + 1] = (char) 0xa9;
    }

    assert_true(id3v1WriteComment(in, tag));

    for (int i = 0; i < ID3V1_FIELD_SIZE; i++) {
        assert_int_equal(tag->comment[i], 0xe9);
    }

    // a full field has no NUL, the reader must still stop at ID3V1_FIELD_SIZE bytes
    char *str = id3v1ReadComment(tag);
    assert_string_equal(str, expected);
    free(str);

    id3v1DestroyTag(&tag);
}

static void id3v1ToJSON_latin1AsUtf8(void **state) {
    (void) state; /* unused */

    // full 30 byte title and Latin-1 bytes, as parsed from a file
    Id3v1Tag *tag = id3v1CreateTag((uint8_t *) "abcdefghijklmnopqrstuvwxyz0123",
                                   (uint8_t *) "H\xe9" "ctor", NULL, 0, 0, NULL, OTHER_GENRE);
    assert_non_null(tag);

    char *json = id3v1ToJSON(tag);
    assert_string_equal(json,
                        "{\"title\":\"abcdefghijklmnopqrstuvwxyz0123\",\"artist\":\"H\xc3\xa9" "ctor\",\"album\":\"\","
                        "\"year\":0,\"track\":0,\"comment\":\"\",\"genreNumber\":12,\"genre\":\"Other\"}");
    free(json);

    id3v1DestroyTag(&tag);
}

// text fields are escaped so the JSON stays valid
static void id3v1ToJSON_escapesText(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag = id3v1CreateTag(NULL, NULL, NULL, 0, 0, NULL, OTHER_GENRE);
    assert_non_null(tag);
    assert_true(id3v1WriteTitle("Say \"hi\"", tag));
    assert_true(id3v1WriteArtist("C:\\new", tag));
    assert_true(id3v1WriteAlbum("tab\there\n", tag));
    assert_true(id3v1WriteComment("\x01", tag));

    char *json = id3v1ToJSON(tag);
    assert_string_equal(json,
                        "{\"title\":\"Say \\\"hi\\\"\",\"artist\":\"C:\\\\new\",\"album\":\"tab\\there\\n\","
                        "\"year\":0,\"track\":0,\"comment\":\"\\u0001\",\"genreNumber\":12,\"genre\":\"Other\"}");
    free(json);

    id3v1DestroyTag(&tag);
}

// full 30 byte fields have no NUL; copying must not read past them (ASan reports an over read otherwise)
static void id3v1CopyTag_fullFields(void **state) {
    (void) state; /* unused */

    Id3v1Tag *tag = id3v1CreateTag(NULL, NULL, NULL, 1999, 7, NULL, POP_GENRE);
    assert_non_null(tag);
    memset(tag->title, 'a', ID3V1_FIELD_SIZE);
    memset(tag->artist, 'b', ID3V1_FIELD_SIZE);
    memset(tag->albumTitle, 'c', ID3V1_FIELD_SIZE);
    memset(tag->comment, 'd', ID3V1_FIELD_SIZE);   // last field of the text block

    Id3v1Tag *copy = id3v1CopyTag(tag);
    assert_non_null(copy);
    assert_true(id3v1CompareTag(tag, copy));
    assert_memory_equal(copy->title, tag->title, ID3V1_FIELD_SIZE);
    assert_memory_equal(copy->artist, tag->artist, ID3V1_FIELD_SIZE);
    assert_memory_equal(copy->albumTitle, tag->albumTitle, ID3V1_FIELD_SIZE);
    assert_memory_equal(copy->comment, tag->comment, ID3V1_FIELD_SIZE);
    assert_int_equal(copy->year, 1999);
    assert_int_equal(copy->track, 7);
    assert_int_equal(copy->genre, POP_GENRE);

    id3v1DestroyTag(&copy);
    id3v1DestroyTag(&tag);
}

// a standalone ID3V1_FIELD_SIZE byte buffer with no NUL; strlen read past it (ASan heap-buffer-overflow)
static void id3v1CreateTag_fullFieldNoNul(void **state) {
    (void) state; /* unused */

    uint8_t *field = malloc(ID3V1_FIELD_SIZE);
    assert_non_null(field);
    memset(field, 'x', ID3V1_FIELD_SIZE);

    Id3v1Tag *tag = id3v1CreateTag(field, field, field, 0, 0, field, OTHER_GENRE);
    assert_non_null(tag);
    assert_memory_equal(tag->title, field, ID3V1_FIELD_SIZE);
    assert_memory_equal(tag->comment, field, ID3V1_FIELD_SIZE);

    free(field);
    id3v1DestroyTag(&tag);
}

// short strings stop at their NUL, longer ones are cut to ID3V1_FIELD_SIZE bytes
static void id3v1CreateTag_fieldLengths(void **state) {
    (void) state; /* unused */

    // heap allocated and exactly sized so ASan reports any read past the NUL
    uint8_t *shortTitle = malloc(4);
    assert_non_null(shortTitle);
    memcpy(shortTitle, "abc", 4);

    Id3v1Tag *tag = id3v1CreateTag(shortTitle, (uint8_t *) "0123456789012345678901234567890123456789", NULL, 0, 0,
                                   NULL, OTHER_GENRE);
    free(shortTitle);
    assert_non_null(tag);

    assert_memory_equal(tag->title, "abc\0", 4);
    assert_memory_equal(tag->artist, "012345678901234567890123456789", ID3V1_FIELD_SIZE);
    assert_int_equal(tag->albumTitle[0], 0x00);
    assert_int_equal(tag->comment[0], 0x00);

    id3v1DestroyTag(&tag);
}

int main() {
    const struct CMUnitTest tests[] = {
        //id3v1HasTag tests
        cmocka_unit_test(id3v1HasTag_foundTag),
        cmocka_unit_test(id3v1HasTag_foundTag2),
        cmocka_unit_test(id3v1HasTag_noTag),

        //id3v1CreateTag tests
        cmocka_unit_test(id3v1NewTag_validTag),
        cmocka_unit_test(id3v1NewTag_validTag2),

        //id3v1ClearTag tests
        cmocka_unit_test(id3v1ClearTag_free),

        //id3v1DestroyTag tests
        cmocka_unit_test(id3v1DestroyTag_free),

        //id3v1TagFromBuffer tests
        cmocka_unit_test(id3v1TagFromBuffer_validBufferVersion11),
        cmocka_unit_test(id3v1TagFromBuffer_validBufferVersion1),
        cmocka_unit_test(id3v1TagFromBuffer_zeros),
        cmocka_unit_test(id3v1TagFromBuffer_tagOnly),
        cmocka_unit_test(id3v1TagFromBuffer_titleOnly),
        cmocka_unit_test(id3v1TagFromBuffer_artistOnly),
        cmocka_unit_test(id3v1TagFromBuffer_albumOnly),
        cmocka_unit_test(id3v1TagFromBuffer_yearOnly),
        cmocka_unit_test(id3v1TagFromBuffer_commentOnly),
        cmocka_unit_test(id3v1TagFromBuffer_trackOnly),
        cmocka_unit_test(id3v1TagFromBuffer_genreOnly),

        //id3v1TagFromFile tests
        cmocka_unit_test(id3v1TagFromFile_readNull),
        cmocka_unit_test(id3v1TagFromFile_nullPath),
        cmocka_unit_test(id3v1TagFromFile_readValidTag),

        //id3v1WriteTitle tests
        //artist, album title, and comment use the same logic
        cmocka_unit_test(id3v1WriteTitle_WithNUll),
        cmocka_unit_test(id3v1WriteTitle_WithNullTitle),
        cmocka_unit_test(id3v1WriteTitle_WithBigTitle),
        cmocka_unit_test(id3v1WriteTitle_WithSmallTitle),

        //id3v1WriteYear tests
        //same logic for genre and track
        cmocka_unit_test(id3v1WriteYear_save0),
        cmocka_unit_test(id3v1WriteYear_saveBig),

        //id3v1CompareTag tests
        cmocka_unit_test(id3v1CompareTag_noTags),
        cmocka_unit_test(id3v1CompareTag_oneTag),
        cmocka_unit_test(id3v1CompareTag_diffGenre),
        cmocka_unit_test(id3v1CompareTag_diffTrack),
        cmocka_unit_test(id3v1CompareTag_diffYear),
        cmocka_unit_test(id3v1CompareTag_diffAlbum),
        cmocka_unit_test(id3v1CompareTag_diffArtist),
        cmocka_unit_test(id3v1CompareTag_diffComment),
        cmocka_unit_test(id3v1CompareTag_diffTitle),
        cmocka_unit_test(id3v1CompareTag_same),

        //no tests for any read functions
        //these just do tag->member

        //id3v1GenreFromTable tests
        cmocka_unit_test(id3v1GenreFromTable_checkNoNull),
        cmocka_unit_test(id3v1GenreFromTable_checkForHipHopGenre),
        cmocka_unit_test(id3v1GenreFromTable_everyGenreNamed),

        //id3v1ToJSON
        cmocka_unit_test(id3v1ToJSON_fullTag),
        cmocka_unit_test(id3v1ToJSON_noGenre),
        cmocka_unit_test(id3v1ToJSON_noYear),
        cmocka_unit_test(id3v1ToJSON_noTitle),

        //id3v1WriteTagToFile
        cmocka_unit_test(id3v1WriteTagToFile_noInputs),
        cmocka_unit_test(id3v1WriteTagToFile_CreateFile),
        cmocka_unit_test(id3v1WriteTagToFile_shortYears),
        cmocka_unit_test(id3v1WriteTagToFile_editExistingFile),
        cmocka_unit_test(id3v1WriteTagToFile_appendFile),
        cmocka_unit_test(id3v1WriteTagToFile_appendFileBig),
        cmocka_unit_test(id3v1WriteTagToFile_longCommentWithTrack),
        cmocka_unit_test(id3v1WriteTagToFile_longCommentNoTrack),
        cmocka_unit_test(id3v1WriteTagToFile_trackRange),
        cmocka_unit_test(id3v1WriteTitle_utf8ToLatin1),
        cmocka_unit_test(id3v1WriteArtist_notLatin1),
        cmocka_unit_test(id3v1WriteAlbum_invalidUtf8),
        cmocka_unit_test(id3v1WriteComment_truncatesAfterConversion),
        cmocka_unit_test(id3v1ToJSON_latin1AsUtf8),
        cmocka_unit_test(id3v1ToJSON_escapesText),
        cmocka_unit_test(id3v1CopyTag_fullFields),
        cmocka_unit_test(id3v1CreateTag_fullFieldNoNul),
        cmocka_unit_test(id3v1CreateTag_fieldLengths),

    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
