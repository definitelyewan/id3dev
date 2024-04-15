#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "id3dev.h"
#include "id3v2/id3v2.h"
#include "id3v1/id3v1.h"
#include "id3v1/id3v1Parser.h"

static void id3CreateAndDestroy_allInOne(void **state){

    ID3 *metadata = id3Create(id3v2TagFromFile("assets/sorry4dying.mp3"), id3v1TagFromFile("assets/sorry4dying.mp3"));

    assert_non_null(metadata);
    assert_non_null(metadata->id3v2);
    assert_non_null(metadata->id3v1); 


    id3Destroy(&metadata);
}

static void id3CreateAndDestroy_allInOneNoV1(void **state){

    ID3 *metadata = id3Create(id3v2TagFromFile("assets/sorry4dying.mp3"), NULL);

    assert_non_null(metadata);
    assert_non_null(metadata->id3v2);
    assert_null(metadata->id3v1);

    id3Destroy(&metadata);
}


static void id3SetPreferedStandard_changeVersion(void **state){

    id3SetPreferedStandard(ID3V2_TAG_VERSION_4);

    assert_int_equal(id3GetPreferedStandard(), ID3V2_TAG_VERSION_4);
}

static void id3FromFile_badPath(void **state){

    ID3 *metadata = id3FromFile("example/example/example");

    assert_non_null(metadata);
    assert_null(metadata->id3v2);
    assert_null(metadata->id3v1);

    id3Destroy(&metadata);
}

static void id3FromFile_noV2(void **state){

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    Id3v1Tag *tag = metadata->id3v1;

    assert_non_null(metadata);
    assert_null(metadata->id3v2);
    assert_non_null(metadata->id3v1);

    assert_string_equal(id3v1ReadAlbum(tag), "Blur");
    assert_string_equal(id3v1ReadArtist(tag), "Blur");
    assert_string_equal(id3v1ReadComment(tag), "test");
    assert_int_equal(id3v1ReadGenre(tag), 17);
    assert_string_equal(id3v1ReadTitle(tag), "Beetlebum");
    assert_int_equal(id3v1ReadTrack(tag), 0);
    assert_int_equal(id3v1ReadYear(tag), 1997);
    
    id3Destroy(&metadata);
}

static void id3Copy_fullTags(void **state){

    ID3 *metadata = id3FromFile("assets/sorry4dying.mp3");
    ID3 *metadata2 = NULL;

    metadata2 = id3Copy(metadata);

    assert_non_null(metadata2);
    assert_true(id3v1CompareTag(metadata->id3v1, metadata2->id3v1));
    assert_true(id3v2CompareTag(metadata->id3v2, metadata2->id3v2));
    
    id3Destroy(&metadata2);
    id3Destroy(&metadata);
}

static void id3Copy_noId3v2(void **state){

    ID3 *metadata = id3FromFile("assets/Beetlebum.mp3");
    ID3 *metadata2 = NULL;

    metadata2 = id3Copy(metadata);

    assert_non_null(metadata2);
    assert_true(id3v1CompareTag(metadata->id3v1, metadata2->id3v1));
    assert_false(id3v2CompareTag(metadata->id3v2, metadata2->id3v2));
    
    id3Destroy(&metadata2);
    id3Destroy(&metadata);
}

static void id3Compare_sameTags(void **state){
    
    ID3 *metadata = id3FromFile("assets/sorry4dying.mp3");
    ID3 *metadata2 = id3FromFile("assets/sorry4dying.mp3");

    assert_true(id3Compare(metadata, metadata2));

    id3Destroy(&metadata2);
    id3Destroy(&metadata);
}

static void id3Compare_diffTags(void **state){
    
    ID3 *metadata = id3FromFile("assets/sorry4dying.mp3");
    ID3 *metadata2 = id3FromFile("assets/OnGP.mp3");

    assert_false(id3Compare(metadata, metadata2));

    id3Destroy(&metadata2);
    id3Destroy(&metadata);
}

static void id3Compare_nullId3v1Tag(void **state){
    
    ID3 *metadata = id3FromFile("assets/sorry4dying.mp3");
    ID3 *metadata2 = id3FromFile("assets/OnGP.mp3");

    id3v1DestroyTag(&metadata2->id3v1);


    assert_false(id3Compare(metadata, metadata2));

    id3Destroy(&metadata2);
    id3Destroy(&metadata);
}

static void id3Compare_bothNullId3v1Tag(void **state){
    
    ID3 *metadata = id3FromFile("assets/sorry4dying.mp3");
    ID3 *metadata2 = id3FromFile("assets/sorry4dying.mp3");

    id3v1DestroyTag(&metadata->id3v1);
    id3v1DestroyTag(&metadata2->id3v1);

    assert_true(id3Compare(metadata, metadata2));

    id3Destroy(&metadata2);
    id3Destroy(&metadata);
}

static void id3Compare_bothNullId3v2Tag(void **state){
    
    ID3 *metadata = id3FromFile("assets/sorry4dying.mp3");
    ID3 *metadata2 = id3FromFile("assets/sorry4dying.mp3");

    id3v2DestroyTag(&metadata->id3v2);
    id3v2DestroyTag(&metadata2->id3v2);

    assert_true(id3Compare(metadata, metadata2));

    id3Destroy(&metadata2);
    id3Destroy(&metadata);
}

static void id3ConvertId3v1ToId3v2_nullArg(void **state){
    assert_false(id3ConvertId3v1ToId3v2(NULL));
}

static void id3ConvertId3v1ToId3v2_noId3v2(void **state){
    
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

static void id3ConvertId3v1ToId3v2_presentId3v2(void **state){
    
    ID3 *metadata = id3FromFile("assets/sorry4dying.mp3");
    char *str = 0;

    id3v2WriteAlbum("SCRAPYARD", metadata->id3v2);
    id3v2WriteYear("2024", metadata->id3v2);
    id3v2WriteTitle("Texas Blue (feat. Kevin Abstract)", metadata->id3v2);

    assert_true(id3ConvertId3v1ToId3v2(metadata));
    assert_non_null(metadata->id3v2);
    
    // str = id3v2ReadTitle(metadata->id3v2);
    // assert_string_equal(str, "sorry4dying");
    // free(str);

    // str = id3v2ReadAlbum(metadata->id3v2);
    // assert_string_equal(str, "I Didn't Mean To Haunt You");
    // free(str);

    // str = id3v2ReadArtist(metadata->id3v2);
    // assert_string_equal(str, "Quadeca");
    // free(str);

    // str = id3v2ReadComment(metadata->id3v2);
    // assert_null(str);
    // free(str);

    // str = id3v2ReadGenre(metadata->id3v2);
    // assert_null(str);
    // free(str);

    str = id3v2ReadTrack(metadata->id3v2);
    //assert_string_equal(str, "1");
    free(str);

    // str = id3v2ReadYear(metadata->id3v2);
    // assert_string_equal(str, "2022");
    // free(str);

    assert_null(id3v2ReadFrameByID("TXXX", metadata->id3v2));

    id3Destroy(&metadata);
}



int main(){
    
    const struct CMUnitTest tests[] = {

        // id3Create & id3Destroy
        cmocka_unit_test(id3CreateAndDestroy_allInOne),
        cmocka_unit_test(id3CreateAndDestroy_allInOneNoV1),

        // id3SetPreferedStandard & id3GetPreferedStandard
        cmocka_unit_test(id3SetPreferedStandard_changeVersion),

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
        cmocka_unit_test(id3ConvertId3v1ToId3v2_presentId3v2)

    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}