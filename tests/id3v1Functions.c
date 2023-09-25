#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "id3v1.h"

void Id3v1TagPrintf(Id3v1Tag *tag){

    printf("title: %s\n",(char *)tag->title);
    printf("artist: %s\n",(char *)tag->artist);
    printf("album: %s\n",(char *)tag->albumTitle);
    printf("year: %d\n",tag->year);
    printf("comment: %s\n",(char *)tag->comment);
    printf("track: %d\n",tag->track);
    printf("genre: %d\n",tag->genre);

}

static void id3v1HasTag_foundTag(void **state){
    (void) state; /* unused */

    FILE *fp = fopen("assets/empty.mp3", "rb");
    unsigned char bytes[ID3V1_MAX_SIZE];
    fseek(fp, -128, SEEK_END);
    fread(bytes, 1, ID3V1_MAX_SIZE, fp);
    assert_true(id3v1HasTag((uint8_t *)bytes));
    fclose(fp);
}

static void id3v1HasTag_foundTag2(void **state){
    (void) state; /* unused */

    FILE *fp = fopen("assets/OnGP.mp3", "rb");
    unsigned char bytes[ID3V1_MAX_SIZE];
    fseek(fp, -128, SEEK_END);
    fread(bytes, 1, ID3V1_MAX_SIZE, fp);
    assert_true(id3v1HasTag((uint8_t *)bytes));
    fclose(fp);
}

static void id3v1HasTag_noTag(void **state){
    (void) state; /* unused */

    FILE *fp = fopen("assets/null.mp3", "rb");
    unsigned char bytes[ID3V1_MAX_SIZE];
    fseek(fp, -128, SEEK_END);
    fread(bytes, 1, ID3V1_MAX_SIZE, fp);
    assert_false(id3v1HasTag((uint8_t *)bytes));
    fclose(fp);
}

static void id3v1NewTag_validTag(void **state){
    (void) state; /* unused */

    Id3v1Tag *tag = id3v1NewTag((uint8_t *)"title",
                                (uint8_t *)"artist",
                                (uint8_t *)"album",
                                2020,
                                9,
                                (uint8_t *)"comment",
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

static void id3v1NewTag_validTag2(void **state){
    (void) state; /* unused */

    Id3v1Tag *tag = id3v1NewTag((uint8_t *)"123456789012345678901234567890x",
                                (uint8_t *)"123456789012345678901234567890x",
                                (uint8_t *)"123456789012345678901234567890x",
                                INT_MAX,
                                127,
                                (uint8_t *)"123456789012345678901234567890x",
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

static void id3v1ClearTag_free(void **state){
    (void) state; /* unused */

    Id3v1Tag *tag = id3v1NewTag((uint8_t *)"title",
                                (uint8_t *)"artist",
                                (uint8_t *)"album",
                                2020,
                                9,
                                (uint8_t *)"comment",
                                JAZZ_GENRE);
    
    id3v1ClearTag(tag);

    assert_non_null(tag);
    assert_int_equal(tag->track, 0);
    assert_int_equal(tag->year, 0);
    assert_int_equal(tag->genre, 12);

    free(tag);
}

static void id3v1DestroyTag_free(void **state){
    (void) state; /* unused */

    Id3v1Tag *tag = id3v1NewTag((uint8_t *)"title",
                                (uint8_t *)"artist",
                                (uint8_t *)"album",
                                2020,
                                9,
                                (uint8_t *)"comment",
                                JAZZ_GENRE);
    
    id3v1DestroyTag(&tag);

    assert_null(tag);

}

static void id3v1TagFromBuffer_validBufferVersion11(void **state){
    (void) state; /* unused */

    uint8_t buffer[] = {'T','A','G',
    /*title*/           'n','e','w',' ','t','i','t','l','e',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*artist*/          'n','e','w',' ','a','r','t','i','s','t',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*album*/           'n','e','w',' ','a','l','b','u','m',' ','t','i','t','l','e',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*year*/            '1','9','9','0',
    /*comment*/         'n','e','w',' ','c','o','m','m','e','n','t',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*track 1.1*/       10,
    /*genre*/           70};

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

static void id3v1TagFromBuffer_validBufferVersion1(void **state){
    (void) state; /* unused */

    uint8_t buffer[] = {'T','A','G',
    /*title*/           'n','e','w',' ','t','i','t','l','e',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*artist*/          'n','e','w',' ','a','r','t','i','s','t',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*album*/           'n','e','w',' ','a','l','b','u','m',' ','t','i','t','l','e',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*year*/            '1','9','9','0',
    /*comment*/         'n','e','w',' ','c','o','m','m','e','n','t',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*track 1.1*/        0,
    /*genre*/           70};

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

static void id3v1TagFromBuffer_zeros(void **state){
    (void) state; /* unused */

    uint8_t buffer[] = {0,0,0,
    /*title*/           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*artist*/          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*album*/           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*year*/            0,0,0,0,
    /*comment*/         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*track 1.1*/       0,
    /*genre*/           0};

    Id3v1Tag *tag = id3v1TagFromBuffer(buffer);

    assert_null(tag);
    id3v1DestroyTag(&tag);
}

static void id3v1TagFromBuffer_tagOnly(void **state){
    (void) state; /* unused */

    uint8_t buffer[] = {'T','A','G',
    /*title*/           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*artist*/          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*album*/           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*year*/            0,0,0,0,
    /*comment*/         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*track 1.1*/       0,
    /*genre*/           0};

    Id3v1Tag *tag = id3v1TagFromBuffer(buffer);

    for(int i = 0; i < 30; i++){
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

static void id3v1TagFromBuffer_titleOnly(void **state){
    (void) state; /* unused */

    uint8_t buffer[] = {'T','A','G',
    /*title*/           '7',' ','R','i','n','g','s',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*artist*/          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*album*/           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*year*/            0,0,0,0,
    /*comment*/         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*track 1.1*/       0,
    /*genre*/           0};

    Id3v1Tag *tag = id3v1TagFromBuffer(buffer);

    for(int i = 0; i < 30; i++){
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

static void id3v1TagFromBuffer_artistOnly(void **state){
    (void) state; /* unused */

    uint8_t buffer[] = {'T','A','G',
    /*title*/           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*artist*/          'B','l','a','c','k',' ','C','o','u','n','t','r','y',',',' ','N','e','w',' ','R','o','a','d',0,0,0,0,0,0,0,
    /*album*/           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*year*/            0,0,0,0,
    /*comment*/         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*track 1.1*/       0,
    /*genre*/           0};

    Id3v1Tag *tag = id3v1TagFromBuffer(buffer);

    for(int i = 0; i < 30; i++){
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

static void id3v1TagFromBuffer_albumOnly(void **state){
    (void) state; /* unused */

    uint8_t buffer[] = {'T','A','G',
    /*title*/           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*artist*/          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*album*/           'X',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*year*/            0,0,0,0,
    /*comment*/         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*track 1.1*/       0,
    /*genre*/           0};

    Id3v1Tag *tag = id3v1TagFromBuffer(buffer);

    for(int i = 0; i < 30; i++){
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

static void id3v1TagFromBuffer_yearOnly(void **state){
    (void) state; /* unused */

    uint8_t buffer[] = {'T','A','G',
    /*title*/           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*artist*/          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*album*/           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*year*/            '1','9','6','3',
    /*comment*/         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*track 1.1*/       0,
    /*genre*/           0};

    Id3v1Tag *tag = id3v1TagFromBuffer(buffer);

    for(int i = 0; i < 30; i++){
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

static void id3v1TagFromBuffer_commentOnly(void **state){
    (void) state; /* unused */

    uint8_t buffer[] = {'T','A','G',
    /*title*/           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*artist*/          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*album*/           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*year*/            0,0,0,0,
    /*comment*/         't','h','e',' ','l','a','t','e','s','t',' ','l','a','n','a',' ','d','o','n','\'','t',' ','f','i','t','>',':','(',0,
    /*track 1.1*/       0,
    /*genre*/           0};

    Id3v1Tag *tag = id3v1TagFromBuffer(buffer);

    for(int i = 0; i < 30; i++){
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

static void id3v1TagFromBuffer_trackOnly(void **state){
    (void) state; /* unused */

    uint8_t buffer[] = {'T','A','G',
    /*title*/           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*artist*/          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*album*/           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*year*/            0,0,0,0,
    /*comment*/         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*track 1.1*/       127,
    /*genre*/           0};

    Id3v1Tag *tag = id3v1TagFromBuffer(buffer);

    for(int i = 0; i < 30; i++){
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

static void id3v1TagFromBuffer_genreOnly(void **state){
    (void) state; /* unused */

    uint8_t buffer[] = {'T','A','G',
    /*title*/           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*artist*/          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*album*/           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*year*/            0,0,0,0,
    /*comment*/         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    /*track 1.1*/       0,
    /*genre*/           255};

    Id3v1Tag *tag = id3v1TagFromBuffer(buffer);

    for(int i = 0; i < 30; i++){
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

static void id3v1TagFromFile_readNull(void **state){
    (void) state; /* unused */


    Id3v1Tag *tag = id3v1TagFromFile("assets/null.mp3");

    assert_null(tag);

    id3v1DestroyTag(&tag);
}

static void id3v1TagFromFile_nullPath(void **state){
    (void) state; /* unused */


    Id3v1Tag *tag = id3v1TagFromFile(NULL);

    assert_null(tag);

    id3v1DestroyTag(&tag);
}

static void id3v1TagFromFile_readValidTag(void **state){
    (void) state; /* unused */


    Id3v1Tag *tag = id3v1TagFromFile("assets/dannybrown.mp3");

    assert_non_null(tag);
    assert_string_equal(tag->title, "Ain't It Funny");
    assert_string_equal(tag->artist, "Danny Brown");
    assert_string_equal(tag->albumTitle, "Atrocity Exhibition");
    assert_int_equal(tag->year, 2016);
    assert_int_equal(tag->track, 6);
    assert_int_equal(tag->genre, 255);
    id3v1DestroyTag(&tag);
}

static void id3v1WriteTitle_WithNUll(void **state){
    (void) state; /* unused */

    assert_false(id3v1WriteTitle(NULL, NULL));
}

static void id3v1WriteTitle_WithNullTitle(void **state){
    (void) state; /* unused */


    Id3v1Tag *tag = id3v1NewTag((uint8_t *)"this is a title of a song",
                                NULL,
                                NULL,
                                0,
                                0,
                                NULL,
                                0);

    assert_true(id3v1WriteTitle(NULL, tag));

    for(int i = 0; i < ID3V1_FIELD_SIZE; i++){
        assert_int_equal(tag->title[i], 0);
    }

    id3v1DestroyTag(&tag);
}

static void id3v1WriteTitle_WithBigTitle(void **state){
    (void) state; /* unused */


    Id3v1Tag *tag = id3v1NewTag((uint8_t *)"this is a title of a song",
                                NULL,
                                NULL,
                                0,
                                0,
                                NULL,
                                0);

    assert_true(id3v1WriteTitle("this is a string that is over 30 bytes", tag));

    assert_string_equal(tag->title, "this is a string that is over ");


    id3v1DestroyTag(&tag);
}

static void id3v1WriteTitle_WithSmallTitle(void **state){
    (void) state; /* unused */


    Id3v1Tag *tag = id3v1NewTag((uint8_t *)"this is a title of a song",
                                NULL,
                                NULL,
                                0,
                                0,
                                NULL,
                                0);

    assert_true(id3v1WriteTitle("", tag));

    for(int i = 0; i < ID3V1_FIELD_SIZE; i++){
        assert_int_equal(tag->title[i], 0);
    }


    id3v1DestroyTag(&tag);
}

static void id3v1WriteYear_WithSmallTitle(void **state){
    (void) state; /* unused */


    Id3v1Tag *tag = id3v1NewTag((uint8_t *)"this is a title of a song",
                                NULL,
                                NULL,
                                0,
                                0,
                                NULL,
                                0);

    assert_true(id3v1WriteTitle("", tag));

    for(int i = 0; i < ID3V1_FIELD_SIZE; i++){
        assert_int_equal(tag->title[i], 0);
    }


    id3v1DestroyTag(&tag);
}


int main(){
    
    const struct CMUnitTest tests[] = {
        //id3v1HasTag tests
        cmocka_unit_test(id3v1HasTag_foundTag),
        cmocka_unit_test(id3v1HasTag_foundTag2),
        cmocka_unit_test(id3v1HasTag_noTag),

        //id3v1NewTag tests
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
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}