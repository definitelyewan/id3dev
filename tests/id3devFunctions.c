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


int main(){
    
    const struct CMUnitTest tests[] = {

        // id3Create & id3Destroy
        cmocka_unit_test(id3CreateAndDestroy_allInOne),
        cmocka_unit_test(id3CreateAndDestroy_allInOneNoV1),

        // id3SetPreferedStandard & id3GetPreferedStandard
        cmocka_unit_test(id3SetPreferedStandard_changeVersion),

        // id3TagFromFile
        cmocka_unit_test(id3FromFile_badPath),
        cmocka_unit_test(id3FromFile_noV2)

    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}