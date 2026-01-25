#include <stdio.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include "id3dev.h"
#include "id3v2/id3v2.h"
#include "id3v1/id3v1.h"
#include "id3v1/id3v1Parser.h"

static void id3CreateAndDestroy_allInOne(void **state) {
    (void) state;

    ID3 *metadata = id3Create(id3v2TagFromFile("assets/sorry4dying.mp3"), id3v1TagFromFile("assets/sorry4dying.mp3"));

    assert_non_null(metadata);
    assert_non_null(metadata->id3v2);
    assert_non_null(metadata->id3v1);


    id3Destroy(&metadata);
}

static void id3CreateAndDestroy_allInOneNoV1(void **state) {
    (void) state;

    ID3 *metadata = id3Create(id3v2TagFromFile("assets/sorry4dying.mp3"), NULL);

    assert_non_null(metadata);
    assert_non_null(metadata->id3v2);
    assert_null(metadata->id3v1);

    id3Destroy(&metadata);
}


static void id3SetPreferredStandard_changeVersion(void **state) {
    (void) state;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_4);

    assert_int_equal(id3GetPreferredStandard(), ID3V2_TAG_VERSION_4);
}

static void id3FromFile_badPath(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("example/example/example");

    assert_non_null(metadata);
    assert_null(metadata->id3v2);
    assert_null(metadata->id3v1);

    id3Destroy(&metadata);
}

static void id3FromFile_noV2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    Id3v1Tag *tag = metadata->id3v1;

    assert_non_null(metadata);
    assert_null(metadata->id3v2);
    assert_non_null(metadata->id3v1);

    assert_string_equal((char *)metadata->id3v1->albumTitle, "Blur");
    assert_string_equal((char *)metadata->id3v1->artist, "Blur");
    assert_string_equal((char *)metadata->id3v1->comment, "test");
    assert_int_equal(id3v1ReadGenre(tag), 17);
    assert_string_equal((char *)metadata->id3v1->title, "Beetlebum");
    assert_int_equal(id3v1ReadTrack(tag), 0);
    assert_int_equal(id3v1ReadYear(tag), 1997);

    id3Destroy(&metadata);
}

static void id3Copy_fullTags(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/sorry4dying.mp3");
    ID3 *metadata2 = NULL;

    metadata2 = id3Copy(metadata);

    assert_non_null(metadata2);
    assert_true(id3v1CompareTag(metadata->id3v1, metadata2->id3v1));
    assert_true(id3v2CompareTag(metadata->id3v2, metadata2->id3v2));

    id3Destroy(&metadata2);
    id3Destroy(&metadata);
}

static void id3Copy_noId3v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    ID3 *metadata2 = NULL;

    metadata2 = id3Copy(metadata);

    assert_non_null(metadata2);
    assert_true(id3v1CompareTag(metadata->id3v1, metadata2->id3v1));
    assert_false(id3v2CompareTag(metadata->id3v2, metadata2->id3v2));

    id3Destroy(&metadata2);
    id3Destroy(&metadata);
}

static void id3Compare_sameTags(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/sorry4dying.mp3");
    ID3 *metadata2 = id3FromFile("assets/sorry4dying.mp3");

    assert_true(id3Compare(metadata, metadata2));

    id3Destroy(&metadata2);
    id3Destroy(&metadata);
}

static void id3Compare_diffTags(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/sorry4dying.mp3");
    ID3 *metadata2 = id3FromFile("assets/OnGP.mp3");

    assert_false(id3Compare(metadata, metadata2));

    id3Destroy(&metadata2);
    id3Destroy(&metadata);
}

static void id3Compare_nullId3v1Tag(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/sorry4dying.mp3");
    ID3 *metadata2 = id3FromFile("assets/OnGP.mp3");

    id3v1DestroyTag(&metadata2->id3v1);


    assert_false(id3Compare(metadata, metadata2));

    id3Destroy(&metadata2);
    id3Destroy(&metadata);
}

static void id3Compare_bothNullId3v1Tag(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/sorry4dying.mp3");
    ID3 *metadata2 = id3FromFile("assets/sorry4dying.mp3");

    id3v1DestroyTag(&metadata->id3v1);
    id3v1DestroyTag(&metadata2->id3v1);

    assert_true(id3Compare(metadata, metadata2));

    id3Destroy(&metadata2);
    id3Destroy(&metadata);
}

static void id3Compare_bothNullId3v2Tag(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/sorry4dying.mp3");
    ID3 *metadata2 = id3FromFile("assets/sorry4dying.mp3");

    id3v2DestroyTag(&metadata->id3v2);
    id3v2DestroyTag(&metadata2->id3v2);

    assert_true(id3Compare(metadata, metadata2));

    id3Destroy(&metadata2);
    id3Destroy(&metadata);
}

static void id3ConvertId3v1ToId3v2_nullArg(void **state) {
    (void) state;

    assert_false(id3ConvertId3v1ToId3v2(NULL));
}

static void id3ConvertId3v1ToId3v2_noId3v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    char *str = 0;


    assert_true(id3ConvertId3v1ToId3v2(metadata));
    assert_non_null(metadata->id3v2);

    str = id3v2ReadTitle(metadata->id3v2);
    assert_string_equal(str, "Beetlebum");
    free(str);

    str = id3v2ReadAlbum(metadata->id3v2);
    assert_string_equal(str, "Blur");
    free(str);

    str = id3v2ReadArtist(metadata->id3v2);
    assert_string_equal(str, "Blur");
    free(str);

    str = id3v2ReadComment(metadata->id3v2);
    assert_string_equal(str, "test");
    free(str);

    str = id3v2ReadGenre(metadata->id3v2);
    assert_string_equal(str, "Rock");
    free(str);

    str = id3v2ReadTrack(metadata->id3v2);
    assert_null(str);

    str = id3v2ReadYear(metadata->id3v2);
    assert_string_equal(str, "1997");
    free(str);

    id3Destroy(&metadata);
}

static void id3ConvertId3v1ToId3v2_presentId3v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/sorry4dying.mp3");
    char *str = 0;

    id3v2WriteAlbum("SCRAPYARD", metadata->id3v2);
    id3v2WriteYear("2024", metadata->id3v2);
    id3v2WriteTitle("Texas Blue (feat. Kevin Abstract)", metadata->id3v2);

    assert_true(id3ConvertId3v1ToId3v2(metadata));
    assert_non_null(metadata->id3v2);

    str = id3v2ReadTitle(metadata->id3v2);
    assert_string_equal(str, "sorry4dying");
    free(str);

    str = id3v2ReadAlbum(metadata->id3v2);
    assert_string_equal(str, "I Didn't Mean To Haunt You");
    free(str);

    str = id3v2ReadArtist(metadata->id3v2);
    assert_string_equal(str, "Quadeca");
    free(str);

    str = id3v2ReadComment(metadata->id3v2);
    assert_null(str);
    free(str);

    str = id3v2ReadGenre(metadata->id3v2);
    assert_null(str);
    free(str);

    str = id3v2ReadTrack(metadata->id3v2);
    assert_string_equal(str, "1");
    free(str);

    str = id3v2ReadYear(metadata->id3v2);
    assert_string_equal(str, "2022");
    free(str);

    assert_null(id3v2ReadFrameByID("TXXX", metadata->id3v2));

    id3Destroy(&metadata);
}

static void id3ConvertId3v2ToId3v1_nullArg(void **state) {
    (void) state;
    assert_false(id3ConvertId3v2ToId3v1(NULL));
}

static void id3ConvertId3v2ToId3v1_noId3v1(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/OnGP.mp3");
    char *str = NULL;
    char *v1Str = NULL;


    id3v1DestroyTag(&metadata->id3v1);

    assert_true(id3ConvertId3v2ToId3v1(metadata));

    assert_non_null(metadata->id3v1);


    str = id3v2ReadTitle(metadata->id3v2);
    v1Str = id3v1ReadTitle(metadata->id3v1);
    assert_string_equal(str, v1Str);
    free(str);
    free(v1Str);

    str = id3v2ReadArtist(metadata->id3v2);
    v1Str = id3v1ReadArtist(metadata->id3v1);
    assert_string_equal(str, v1Str);
    free(str);
    free(v1Str);

    // utf8 not supported so cant call id3v2ReadAlbum
    v1Str = id3v1ReadAlbum(metadata->id3v1);
    assert_memory_equal("The Powers That Butf8ÛȾℲ", v1Str, 28);
    free(v1Str);

    assert_int_equal(9, id3v1ReadTrack(metadata->id3v1));
    assert_int_equal(0, id3v1ReadYear(metadata->id3v1));
    assert_int_equal(69, id3v1ReadGenre(metadata->id3v1));

    v1Str = id3v1ReadComment(metadata->id3v1);
    assert_int_equal(0, v1Str[0]);
    free(v1Str);

    id3Destroy(&metadata);
}

static void id3ConvertId3v2ToId3v1_overwriteId3v1(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/boniver.mp3");
    char *strv1 = NULL;
    char *strv2 = NULL;

    id3v2WriteYear("9999", metadata->id3v2);
    id3v2WriteTrack("99", metadata->id3v2);
    id3v2WriteComment("test comment", metadata->id3v2);

    id3v1DestroyTag(&metadata->id3v1);

    assert_true(id3ConvertId3v2ToId3v1(metadata));

    assert_non_null(metadata->id3v1);

    strv1 = id3v1ReadTitle(metadata->id3v1);
    strv2 = id3v2ReadTitle(metadata->id3v2);
    assert_memory_equal(strv1, strv2, 30);
    free(strv1);
    free(strv2);

    strv1 = id3v1ReadArtist(metadata->id3v1);
    strv2 = id3v2ReadArtist(metadata->id3v2);
    assert_string_equal(strv1, strv2);
    free(strv1);
    free(strv2);

    strv1 = id3v1ReadAlbum(metadata->id3v1);
    strv2 = id3v2ReadAlbum(metadata->id3v2);
    assert_string_equal(strv1, strv2);
    free(strv1);
    free(strv2);

    assert_int_equal(99, id3v1ReadTrack(metadata->id3v1));
    assert_int_equal(9999, id3v1ReadYear(metadata->id3v1));
    assert_int_equal(65, id3v1ReadGenre(metadata->id3v1));

    strv1 = id3v1ReadComment(metadata->id3v1);
    strv2 = id3v2ReadComment(metadata->id3v2);
    assert_string_equal(strv1, strv2);
    free(strv1);
    free(strv2);

    id3Destroy(&metadata);
}

static void id3ReadTitle_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/sorry4dying.mp3");
    char *str = NULL;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_3);
    str = id3ReadTitle(metadata);
    assert_string_equal(str, "sorry4dying");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    str = id3ReadTitle(metadata);
    assert_string_equal(str, "sorry4dying");
    free(str);

    id3Destroy(&metadata);
}

static void id3ReadArtist_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/OnGP.mp3");
    char *str = NULL;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_3); // incorrect on purpose because it should work anyway
    str = id3ReadArtist(metadata);
    assert_string_equal(str, "Death Grips");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    str = id3ReadArtist(metadata);
    assert_string_equal(str, "Death Grips");
    free(str);

    id3Destroy(&metadata);
}

static void id3ReadAlbumArtist_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/sorry4dying.mp3");
    char *str = NULL;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_3); // incorrect on purpose because it should work anyway
    str = id3ReadAlbumArtist(metadata);
    assert_string_equal(str, "Quadeca");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    str = id3ReadAlbumArtist(metadata);
    assert_null(str);

    id3Destroy(&metadata);
}

static void id3ReadAlbum_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    char *str = NULL;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_3);
    str = id3ReadAlbum(metadata); // should read ID3v1 tag because ID3v2 is NULL
    assert_string_equal(str, "Blur");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    str = id3ReadAlbum(metadata);
    assert_string_equal(str, "Blur");
    free(str);

    id3Destroy(&metadata);
}

static void id3ReadYear_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    char *str = NULL;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_4);
    id3ConvertId3v1ToId3v2(metadata);

    str = id3ReadYear(metadata);
    assert_string_equal(str, "1997");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    str = id3ReadYear(metadata);
    assert_string_equal(str, "1997");
    free(str);

    id3Destroy(&metadata);
}

static void id3ReadGenre_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/boniver.mp3");
    char *str = NULL;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_2);
    str = id3ReadGenre(metadata);
    assert_string_equal(str, "Alternative");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    str = id3ReadGenre(metadata);
    assert_string_equal(str, "Alternative");
    free(str);

    id3Destroy(&metadata);
}

static void id3ReadTrack_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/danybrown2.mp3");
    char *str = NULL;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_2);
    str = id3ReadTrack(metadata);
    assert_string_equal(str, "06/15");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    str = id3ReadTrack(metadata);
    assert_string_equal(str, "6");
    free(str);

    id3Destroy(&metadata);
}

static void id3ReadComposer_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/boniver.mp3");
    char *str = NULL;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_2);
    str = id3ReadComposer(metadata);
    assert_string_equal(str, "Bon Iver");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    str = id3ReadComposer(metadata);
    assert_null(str);

    id3Destroy(&metadata);
}

static void id3ReadDisc_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/boniver.mp3");
    char *str = NULL;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_2);
    str = id3ReadDisc(metadata);
    assert_string_equal(str, "01/01");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    str = id3ReadDisc(metadata);
    assert_null(str);

    id3Destroy(&metadata);
}

static void id3ReadLyrics_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/danybrown2.mp3");
    char *str = NULL;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_2);
    str = id3ReadLyrics(metadata);
    assert_string_equal(
        str,
        "haBDJHAsbdjkHASBDJahbsdkAHBSDHAbsdHBDUAHSBDUBAUIBFOASIUBDFOIAUBFOIAUWBFOAWBFAOUWEBFUOYBOUBUOBUOboubouboubouboubouboigndoignoisnjgsdfjnglksjdfngslkjfngskdjfnglskdnfgiserugisugnvfkdxjnvxlkjnijxdngixjdhfgoiserhgiusdng spoerijgsoergjnposeirhgposergn reigjosperijgsodfkgkldfmvxc.,vbm");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    str = id3ReadLyrics(metadata);
    assert_null(str);

    id3Destroy(&metadata);
}

static void id3ReadComment_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    char *str = NULL;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_2);
    id3ConvertId3v1ToId3v2(metadata);

    str = id3ReadComment(metadata);
    assert_string_equal(str, "test");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    str = id3ReadComment(metadata);
    assert_string_equal(str, "test");
    free(str);

    id3Destroy(&metadata);
}

static void id3ReadPicture_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/boniver.mp3");
    uint8_t *data = NULL;
    size_t size = 0;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_2);


    data = id3ReadPicture(0, metadata, &size);
    assert_non_null(data);
    free(data);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    data = id3ReadPicture(0, metadata, &size);
    assert_null(data);
    assert_int_equal(size, 0);

    id3Destroy(&metadata);
}

static void id3WriteTitle_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/sorry4dying.mp3");
    char *str = NULL;
    bool v = false;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_3);
    v = id3WriteTitle("test", metadata);
    assert_true(v);
    str = id3ReadTitle(metadata);
    assert_string_equal(str, "test");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    v = id3WriteTitle("test", metadata);
    assert_true(v);
    str = id3ReadTitle(metadata);
    assert_string_equal(str, "test");
    free(str);

    id3Destroy(&metadata);
}

static void id3WriteTitle_flipStd(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/sorry4dying.mp3");
    char *str = NULL;
    bool v = false;

    id3v2DestroyTag(&metadata->id3v2);
    id3SetPreferredStandard(ID3V2_TAG_VERSION_3);

    v = id3WriteTitle("test", metadata);
    assert_true(v);
    str = id3ReadTitle(metadata);
    assert_string_equal(str, "test");
    free(str);

    id3Destroy(&metadata);
}

static void id3WriteArtist_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/sorry4dying.mp3");
    char *str = NULL;
    bool v = false;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_3);
    v = id3WriteArtist("ben lasky", metadata);
    assert_true(v);
    str = id3ReadArtist(metadata);
    assert_string_equal(str, "ben lasky");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    v = id3WriteArtist("ben lasky", metadata);
    assert_true(v);
    str = id3ReadArtist(metadata);
    assert_string_equal(str, "ben lasky");
    free(str);

    id3Destroy(&metadata);
}

static void id3WriteAlbumArtist_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    char *str = NULL;
    bool v = false;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_3);
    id3ConvertId3v1ToId3v2(metadata);


    v = id3WriteAlbumArtist("blur", metadata);
    assert_true(v);
    str = id3ReadAlbumArtist(metadata);
    assert_string_equal(str, "blur");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    v = id3WriteAlbumArtist("damon albarn", metadata);
    assert_false(v);

    id3Destroy(&metadata);
}

static void id3WriteYear_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    char *str = NULL;
    bool v = false;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_3);
    id3ConvertId3v1ToId3v2(metadata);


    v = id3WriteYear("1999", metadata);
    assert_true(v);
    str = id3ReadYear(metadata);
    assert_string_equal(str, "1999");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    v = id3WriteYear("2", metadata);
    assert_true(v);
    str = id3ReadYear(metadata);
    assert_string_equal(str, "2");
    free(str);

    id3Destroy(&metadata);
}

static void id3WriteGenre_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    char *str = NULL;
    bool v = false;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_3);
    id3ConvertId3v1ToId3v2(metadata);


    v = id3WriteGenre("Rock", metadata);
    assert_true(v);
    str = id3ReadGenre(metadata);
    assert_string_equal(str, "Rock");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    v = id3WriteGenre("\x01", metadata);
    assert_true(v);
    str = id3ReadGenre(metadata);
    assert_string_equal(str, id3v1GenreFromTable(1));
    free(str);

    id3Destroy(&metadata);
}

static void id3WriteTrack_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    char *str = NULL;
    bool v = false;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_3);
    id3v1WriteTrack(0xff, metadata->id3v1);
    id3ConvertId3v1ToId3v2(metadata);


    v = id3WriteTrack("1", metadata);
    assert_true(v);
    str = id3ReadTrack(metadata);
    assert_string_equal(str, "1");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    v = id3WriteTrack("1", metadata);
    assert_true(v);
    str = id3ReadTrack(metadata);
    assert_string_equal(str, "1");
    free(str);

    id3Destroy(&metadata);
}

static void id3WriteDisc_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    char *str = NULL;
    bool v = false;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_3);
    id3ConvertId3v1ToId3v2(metadata);


    v = id3WriteDisc("1/10", metadata);
    assert_true(v);
    str = id3ReadDisc(metadata);
    assert_string_equal(str, "1/10");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    v = id3WriteAlbumArtist("1/1", metadata);
    assert_false(v);

    id3Destroy(&metadata);
}

static void id3WriteComposer_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    char *str = NULL;
    bool v = false;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_3);
    id3ConvertId3v1ToId3v2(metadata);


    v = id3WriteComposer("Damon Albarn", metadata);
    assert_true(v);
    str = id3ReadComposer(metadata);
    assert_string_equal(str, "Damon Albarn");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    v = id3WriteComposer("Damon Albarn", metadata);
    assert_false(v);

    id3Destroy(&metadata);
}

static void id3WriteLyrics_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    char *str = NULL;
    bool v = false;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_3);
    id3ConvertId3v1ToId3v2(metadata);


    v = id3WriteLyrics(
        "Beetlebum\nWhat you've done\nShe's a gun\nNow what you've done\nBeetlebum\nGet nothing done\nYou beetlebum\nJust get numb\nNow what you've done\nBeetlebum",
        metadata);
    assert_true(v);
    str = id3ReadLyrics(metadata);
    assert_string_equal(
        str,
        "Beetlebum\nWhat you've done\nShe's a gun\nNow what you've done\nBeetlebum\nGet nothing done\nYou beetlebum\nJust get numb\nNow what you've done\nBeetlebum");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    v = id3WriteLyrics(
        "Beetlebum\nWhat you've done\nShe's a gun\nNow what you've done\nBeetlebum\nGet nothing done\nYou beetlebum\nJust get numb\nNow what you've done\nBeetlebum",
        metadata);
    assert_false(v);

    id3Destroy(&metadata);
}

static void id3WriteComment_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    char *str = NULL;
    bool v = false;

    id3SetPreferredStandard(ID3V2_TAG_VERSION_3);
    id3ConvertId3v1ToId3v2(metadata);


    v = id3WriteComment("comment", metadata);
    assert_true(v);
    str = id3ReadComment(metadata);
    assert_string_equal(str, "comment");
    free(str);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    v = id3WriteComment("comment", metadata);
    assert_true(v);
    str = id3ReadComment(metadata);
    assert_string_equal(str, "comment");
    free(str);

    id3Destroy(&metadata);
}

static void id3WritePicture_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    uint8_t *data = NULL;
    bool v = false;
    uint8_t *picture = NULL;
    size_t picSize = 0;
    size_t dataSize = 0;
    FILE *fp = NULL;

    fp = fopen("assets/cat.png", "rb");
    assert_non_null(fp);
    // NOLINTNEXTLINE
    (void) fseek(fp, 0, SEEK_END);
    picSize = ftell(fp);
    (void) fseek(fp, 0, SEEK_SET);

    picture = malloc(picSize);
    (void) fread(picture, 1, picSize, fp);
    (void) fclose(fp);


    id3SetPreferredStandard(ID3V2_TAG_VERSION_3);
    id3ConvertId3v1ToId3v2(metadata);


    v = id3WritePicture(picture, picSize, "png", 0, metadata);
    assert_true(v);
    data = id3ReadPicture(0, metadata, &dataSize);
    assert_memory_equal(data, picture, dataSize);
    free(data);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    v = id3WritePicture(picture, picSize, "png", 0, metadata);
    assert_false(v);

    free(picture);
    id3Destroy(&metadata);
}

static void id3WritePictureFromFile_v1v2(void **state) {
    (void) state;
    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    uint8_t *data = NULL;
    bool v = false;
    uint8_t *picture = NULL;
    size_t picSize = 0;
    size_t dataSize = 0;
    FILE *fp = NULL;

    fp = fopen("assets/cat.png", "rb");
    // NOLINTNEXTLINE
    (void) fseek(fp, 0, SEEK_END);
    picSize = ftell(fp);
    (void) fseek(fp, 0, SEEK_SET);

    picture = malloc(picSize);
    (void) fread(picture, 1, picSize, fp);
    (void) fclose(fp);

    id3SetPreferredStandard(ID3V2_TAG_VERSION_3);
    id3ConvertId3v1ToId3v2(metadata);

    v = id3WritePictureFromFile("assets/cat.png", "png", 0, metadata);
    assert_true(v);
    data = id3ReadPicture(0, metadata, &dataSize);
    assert_memory_equal(data, picture, dataSize);
    free(data);

    id3SetPreferredStandard(ID3V1_TAG_VERSION);
    v = id3WritePictureFromFile("assets/cat.png", "png", 0, metadata);
    assert_false(v);

    free(picture);
    id3Destroy(&metadata);
}

static void id3ToJSON_v1Only(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    char *json = NULL;

    json = id3ToJSON(metadata);


    assert_string_equal(json,
                        "{\"ID3v1\":{\"title\":\"Beetlebum\",\"artist\":\"Blur\",\"album\":\"Blur\",\"year\":1997,\"track\":0,\"comment\":\"test\",\"genreNumber\":17,\"genre\":\"Rock\"},\"ID3v2\":{}}");

    free(json);
    id3Destroy(&metadata);
}

static void id3ToJSON_v2Only(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    char *json = NULL;

    id3ConvertId3v1ToId3v2(metadata);
    id3v1DestroyTag(&metadata->id3v1);

    json = id3ToJSON(metadata);

    assert_string_equal(json,
                        "{\"ID3v1\":{},\"ID3v2\":{\"header\":{\"major\":3,\"minor\":0,\"flags\":0,\"extended\":{}},\"content\":[{\"header\":{\"id\":\"TIT2\",\"tagAlterPreservation\":false,\"fileAlterPreservation\":false,\"readOnly\":false,\"decompressionSize\":0,\"encryptionSymbol\":0,\"groupSymbol\":0},\"content\":[{\"value\":\"1\",\"size\":1},{\"value\":\"Beetlebum\",\"size\":9}]},{\"header\":{\"id\":\"TPE1\",\"tagAlterPreservation\":false,\"fileAlterPreservation\":false,\"readOnly\":false,\"decompressionSize\":0,\"encryptionSymbol\":0,\"groupSymbol\":0},\"content\":[{\"value\":\"1\",\"size\":1},{\"value\":\"Blur\",\"size\":4}]},{\"header\":{\"id\":\"TALB\",\"tagAlterPreservation\":false,\"fileAlterPreservation\":false,\"readOnly\":false,\"decompressionSize\":0,\"encryptionSymbol\":0,\"groupSymbol\":0},\"content\":[{\"value\":\"1\",\"size\":1},{\"value\":\"Blur\",\"size\":4}]},{\"header\":{\"id\":\"TYER\",\"tagAlterPreservation\":false,\"fileAlterPreservation\":false,\"readOnly\":false,\"decompressionSize\":0,\"encryptionSymbol\":0,\"groupSymbol\":0},\"content\":[{\"value\":\"1\",\"size\":1},{\"value\":\"1997\",\"size\":4}]},{\"header\":{\"id\":\"TCON\",\"tagAlterPreservation\":false,\"fileAlterPreservation\":false,\"readOnly\":false,\"decompressionSize\":0,\"encryptionSymbol\":0,\"groupSymbol\":0},\"content\":[{\"value\":\"1\",\"size\":1},{\"value\":\"Rock\",\"size\":4}]},{\"header\":{\"id\":\"COMM\",\"tagAlterPreservation\":false,\"fileAlterPreservation\":false,\"readOnly\":false,\"decompressionSize\":0,\"encryptionSymbol\":0,\"groupSymbol\":0},\"content\":[{\"value\":\"1\",\"size\":1},{\"value\":\"enh4\",\"size\":3},{\"value\":\"\",\"size\":1},{\"value\":\"test\",\"size\":4}]}]}}");

    free(json);
    id3Destroy(&metadata);
}

static void id3ToJSON_v1v2(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    FILE *fp = NULL;
    size_t sz = 0;
    char *json = NULL;
    uint8_t *data = NULL;


    fp = fopen("assets/beetlebum.json", "rb");
    // NOLINTNEXTLINE
    (void) fseek(fp, 0, SEEK_END);
    sz = ftell(fp);
    (void) fseek(fp, 0, SEEK_SET);
    data = calloc(sz + 1, 1);
    (void) fread(data, 1, sz, fp);
    (void) fclose(fp);


    id3ConvertId3v1ToId3v2(metadata);


    json = id3ToJSON(metadata);

    assert_string_equal(json, (char *)data);

    free(json);
    free(data);
    id3Destroy(&metadata);
}

static void id3WriteToFile_v1NoTag(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    ID3 *metadata2 = NULL;

    assert_true(id3WriteToFile("assets/tmp", metadata));
    metadata2 = id3FromFile("assets/tmp");
    (void) remove("assets/tmp");

    assert_non_null(metadata2);
    assert_non_null(metadata2->id3v1);

    assert_memory_equal(metadata->id3v1->albumTitle, metadata2->id3v1->albumTitle, 30);
    assert_memory_equal(metadata->id3v1->artist, metadata2->id3v1->artist, 30);
    assert_memory_equal(metadata->id3v1->comment, metadata2->id3v1->comment, 30);
    assert_int_equal(metadata->id3v1->genre, metadata2->id3v1->genre);
    assert_memory_equal(metadata->id3v1->title, metadata2->id3v1->title, 30);
    assert_int_equal(metadata->id3v1->track, metadata2->id3v1->track);
    assert_int_equal(metadata->id3v1->year, metadata2->id3v1->year);


    id3Destroy(&metadata2);
    id3Destroy(&metadata);
}


static void id3WriteToFile_v1UpdateExisting(void **state) {
    (void) state;

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    ID3 *metadata2 = NULL;

    id3v1WriteTagToFile("assets/tmp", metadata->id3v1);


    id3WriteTitle("test title", metadata);
    assert_true(id3WriteToFile("assets/tmp", metadata));

    metadata2 = id3FromFile("assets/tmp");
    (void) remove("assets/tmp");

    assert_non_null(metadata2);
    assert_non_null(metadata2->id3v1);

    assert_memory_equal(metadata->id3v1->albumTitle, metadata2->id3v1->albumTitle, 30);
    assert_memory_equal(metadata->id3v1->artist, metadata2->id3v1->artist, 30);
    assert_memory_equal(metadata->id3v1->comment, metadata2->id3v1->comment, 30);
    assert_int_equal(metadata->id3v1->genre, metadata2->id3v1->genre);
    assert_string_equal("test title", (char *)metadata2->id3v1->title);
    assert_int_equal(metadata->id3v1->track, metadata2->id3v1->track);
    assert_int_equal(metadata->id3v1->year, metadata2->id3v1->year);


    id3Destroy(&metadata2);
    id3Destroy(&metadata);
}

static void id3WriteToFile_v1AppendTag(void **state) {
    (void) state;

    char str[6] = "music";

    FILE *fp = fopen("assets/tmp", "w");
    ID3 *metadata = NULL;


    assert_non_null(fp);
    (void) fwrite(str, sizeof(char), strlen(str), fp);
    (void) fclose(fp);


    metadata = id3FromFile("assets/Beetlebum.mp3");
    assert_true(id3WriteToFile("assets/tmp", metadata));


    fp = fopen("assets/tmp", "r");
    assert_non_null(fp);
    (void) fread(str, sizeof(char), 5, fp);
    (void) fclose(fp);

    assert_memory_equal(str, "music", 5);

    (void) remove("assets/tmp");
    id3Destroy(&metadata);
}

static void id3WriteToFile_v2NoTag(void **state) {
    (void) state;

    ID3 *metadata = NULL;
    ID3 *metadata2 = NULL;
    bool v = false;

    metadata = id3FromFile("assets/Beetlebum.mp3");
    id3ConvertId3v1ToId3v2(metadata);
    id3v1DestroyTag(&metadata->id3v1);

    v = id3WriteToFile("assets/tmp", metadata);
    assert_true(v);

    metadata2 = id3FromFile("assets/tmp");

    v = id3Compare(metadata, metadata2);
    assert_true(v);

    (void) remove("assets/tmp");
    id3Destroy(&metadata);
    id3Destroy(&metadata2);
}

static void id3WriteToFile_v2UpdateExisting(void **state) {
    (void) state;

    ID3 *metadata = NULL;
    ID3 *metadata2 = NULL;
    bool v = false;

    metadata = id3FromFile("assets/Beetlebum.mp3");
    id3ConvertId3v1ToId3v2(metadata);
    id3v1DestroyTag(&metadata->id3v1);

    v = id3WriteToFile("assets/tmp", metadata);
    assert_true(v);

    metadata2 = id3FromFile("assets/tmp");
    id3WriteTitle("test title", metadata2);
    v = id3WriteToFile("assets/tmp", metadata2);
    assert_true(v);

    id3Destroy(&metadata);
    metadata = id3FromFile("assets/tmp");
    v = id3Compare(metadata, metadata2);
    assert_true(v);


    id3Destroy(&metadata);
    id3Destroy(&metadata2);
}

static void id3WriteToFile_v2PrependTag(void **state) {
    (void) state;

    char str[6] = "music";

    FILE *fp = fopen("assets/tmp", "w");
    ID3 *metadata = NULL;


    assert_non_null(fp);
    (void) fwrite(str, sizeof(char), strlen(str), fp);
    (void) fclose(fp);


    metadata = id3FromFile("assets/Beetlebum.mp3");
    id3ConvertId3v1ToId3v2(metadata);
    id3v1DestroyTag(&metadata->id3v1);
    assert_true(id3WriteToFile("assets/tmp", metadata));


    fp = fopen("assets/tmp", "r");
    assert_non_null(fp);
    (void) fseek(fp, -5, SEEK_END);
    (void) fread(str, sizeof(char), 5, fp);
    (void) fclose(fp);

    assert_memory_equal(str, "music", 5);

    (void) remove("assets/tmp");
    id3Destroy(&metadata);
}

static void id3WriteToFile_v1v2NoTags(void **state) {
    (void) state;

    ID3 *metadata = NULL;
    ID3 *metadata2 = NULL;
    bool v = false;

    metadata = id3FromFile("assets/sorry4dying.mp3");

    v = id3WriteToFile("assets/tmp", metadata);
    assert_true(v);

    metadata2 = id3FromFile("assets/tmp");
    (void) remove("assets/tmp");


    v = id3Compare(metadata, metadata2);
    assert_true(v);

    id3Destroy(&metadata);
    id3Destroy(&metadata2);
}

int main() {
    FILE *fp = NULL;

    fp = fopen("assets/tmp", "r");

    if (fp != NULL) {
        (void) fclose(fp);
        (void) remove("assets/tmp");
    }


    const struct CMUnitTest tests[] = {

        // id3Create & id3Destroy
        cmocka_unit_test(id3CreateAndDestroy_allInOne),
        cmocka_unit_test(id3CreateAndDestroy_allInOneNoV1),

        // id3SetPreferredStandard & id3GetPreferredStandard
        cmocka_unit_test(id3SetPreferredStandard_changeVersion),

        // id3TagFromFile
        cmocka_unit_test(id3FromFile_badPath),
        cmocka_unit_test(id3FromFile_noV2),

        // id3Copy
        cmocka_unit_test(id3Copy_fullTags),
        cmocka_unit_test(id3Copy_noId3v2),

        // id3Compare
        cmocka_unit_test(id3Compare_sameTags),
        cmocka_unit_test(id3Compare_diffTags),
        cmocka_unit_test(id3Compare_nullId3v1Tag),
        cmocka_unit_test(id3Compare_bothNullId3v1Tag),
        cmocka_unit_test(id3Compare_bothNullId3v2Tag),

        // id3ConvertId3v1ToId3v2
        cmocka_unit_test(id3ConvertId3v1ToId3v2_nullArg),
        cmocka_unit_test(id3ConvertId3v1ToId3v2_noId3v2),
        cmocka_unit_test(id3ConvertId3v1ToId3v2_presentId3v2),

        // id3ConvertId3v2ToId3v1
        cmocka_unit_test(id3ConvertId3v2ToId3v1_nullArg),
        cmocka_unit_test(id3ConvertId3v2ToId3v1_noId3v1),
        cmocka_unit_test(id3ConvertId3v2ToId3v1_overwriteId3v1),

        // id3Read*
        cmocka_unit_test(id3ReadTitle_v1v2),
        cmocka_unit_test(id3ReadArtist_v1v2),
        cmocka_unit_test(id3ReadAlbumArtist_v1v2),
        cmocka_unit_test(id3ReadAlbum_v1v2),
        cmocka_unit_test(id3ReadYear_v1v2),
        cmocka_unit_test(id3ReadGenre_v1v2),
        cmocka_unit_test(id3ReadTrack_v1v2),
        cmocka_unit_test(id3ReadComposer_v1v2),
        cmocka_unit_test(id3ReadDisc_v1v2),
        cmocka_unit_test(id3ReadLyrics_v1v2),
        cmocka_unit_test(id3ReadComment_v1v2),
        cmocka_unit_test(id3ReadPicture_v1v2),

        // id3Write*
        cmocka_unit_test(id3WriteTitle_v1v2),
        cmocka_unit_test(id3WriteTitle_flipStd),
        cmocka_unit_test(id3WriteArtist_v1v2),
        cmocka_unit_test(id3WriteAlbumArtist_v1v2),
        cmocka_unit_test(id3WriteYear_v1v2),
        cmocka_unit_test(id3WriteGenre_v1v2),
        cmocka_unit_test(id3WriteTrack_v1v2),
        cmocka_unit_test(id3WriteDisc_v1v2),
        cmocka_unit_test(id3WriteComposer_v1v2),
        cmocka_unit_test(id3WriteLyrics_v1v2),
        cmocka_unit_test(id3WriteComment_v1v2),
        cmocka_unit_test(id3WritePicture_v1v2),
        cmocka_unit_test(id3WritePictureFromFile_v1v2),

        // id3ToJSON
        cmocka_unit_test(id3ToJSON_v1Only),
        cmocka_unit_test(id3ToJSON_v2Only),
        cmocka_unit_test(id3ToJSON_v1v2),

        // id3WriteToFile
        cmocka_unit_test(id3WriteToFile_v1NoTag),
        cmocka_unit_test(id3WriteToFile_v1UpdateExisting),
        cmocka_unit_test(id3WriteToFile_v1AppendTag),
        cmocka_unit_test(id3WriteToFile_v2NoTag),
        cmocka_unit_test(id3WriteToFile_v2UpdateExisting),
        cmocka_unit_test(id3WriteToFile_v2PrependTag),
        cmocka_unit_test(id3WriteToFile_v1v2NoTags),

    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
