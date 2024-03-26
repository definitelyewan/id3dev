/**
 * @file id3v2Functions.c
 * @author Ewan Jones
 * @brief unit tests for id3v2.c
 * @version 0.1
 * @date 2024-03-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "id3v2.h"
#include "id3v2Frame.h"

static void id3v2TagFromFile_v3(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_non_null(tag);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    int c = 0;
    while((f = id3v2FrameTraverse(&frames)) != NULL){
        c++;
    }
    
    assert_int_equal(c, 15);

    id3v2DestroyTag(&tag);
}

static void id3v2TagFromFile_null(void **state){

    Id3v2Tag *tag = id3v2TagFromFile(NULL);

    assert_null(tag);
}


static void id3v2CopyTag_v3(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    Id3v2Tag *copy = id3v2CopyTag(tag);

    assert_non_null(copy);

    assert_non_null(copy->header);
    assert_non_null(copy->frames);

    assert_int_equal(tag->header->majorVersion, copy->header->majorVersion);
    assert_int_equal(tag->header->minorVersion, copy->header->minorVersion);
    assert_int_equal(tag->header->flags, copy->header->flags);
    assert_null(copy->header->extendedHeader);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;


    for(int i = 0; i < 15; i++){
        f = id3v2FrameTraverse(&frames);
        listFindElement(tag->frames, id3v2CompareFrame, f);
        assert_non_null(f);
    }

    id3v2DestroyTag(&tag);
    id3v2DestroyTag(&copy);
}


static void id3v2CompareTag_v3v4(void **state){

    Id3v2Tag *tag1 = id3v2TagFromFile("assets/sorry4dying.mp3");
    Id3v2Tag *tag2 = id3v2TagFromFile("assets/OnGP.mp3");

    assert_false(id3v2CompareTag(tag1, tag2));

    id3v2DestroyTag(&tag1);
    id3v2DestroyTag(&tag2);

}

static void id3v2CompareTag_v3same(void **state){

    Id3v2Tag *tag1 = id3v2TagFromFile("assets/sorry4dying.mp3");
    Id3v2Tag *tag2 = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_true(id3v2CompareTag(tag1, tag2));

    id3v2DestroyTag(&tag1);
    id3v2DestroyTag(&tag2);

}

static void id3v2ReadFrameByID_v3(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    Id3v2Frame *frame = id3v2ReadFrameByID("TIT2", tag);
    assert_non_null(frame);
    assert_string_equal((char *)frame->header->id, "TIT2");
    id3v2DestroyFrame(&frame);


    frame = id3v2ReadFrameByID("TALB", tag);
    assert_non_null(frame);
    assert_string_equal((char *)frame->header->id, "TALB");
    id3v2DestroyFrame(&frame);


    id3v2DestroyTag(&tag);

}

static void id3v2ReadFrameByID_v3MultiTXXX(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");
    Id3v2Frame *frame = NULL;
    ListIter entries = {0};

    char *str = NULL;
    size_t s = 0;

    frame = id3v2ReadFrameByID("TXXX", tag);
    assert_non_null(frame);
    assert_string_equal((char *)frame->header->id, "TXXX");

    entries = id3v2CreateFrameEntryTraverser(frame);

    str = id3v2ReadFrameEntryAsChar(&entries, &s);
    assert_string_equal(str, "\x01");
    free(str);

    str = id3v2ReadFrameEntryAsChar(&entries, &s);
    assert_string_equal(str, "LABEL");
    free(str);

    str = id3v2ReadFrameEntryAsChar(&entries, &s);
    assert_string_equal(str, "deadAir");
    free(str);

    id3v2DestroyFrame(&frame);

    // the exact same frame should be returned

    frame = id3v2ReadFrameByID("TXXX", tag);
    assert_non_null(frame);
    assert_string_equal((char *)frame->header->id, "TXXX");

    entries = id3v2CreateFrameEntryTraverser(frame);

    str = id3v2ReadFrameEntryAsChar(&entries, &s);
    assert_string_equal(str, "\x01");
    free(str);

    str = id3v2ReadFrameEntryAsChar(&entries, &s);
    assert_string_equal(str, "LABEL");
    free(str);

    str = id3v2ReadFrameEntryAsChar(&entries, &s);
    assert_string_equal(str, "deadAir");
    free(str);

    id3v2DestroyFrame(&frame);
    id3v2DestroyTag(&tag);

}

static void id3v2ReadFrameByID_v2Null(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    Id3v2Frame *frame = id3v2ReadFrameByID("XXX", tag);
    assert_null(frame);

    id3v2DestroyTag(&tag);

}

static void id3v2RemoveFrameByID_v3EveryFrame(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    //int total = tag->frames->length;
    id3v2RemoveFrameByID("APIC", tag);
    //assert_true(id3v2RemoveFrameByID("APIC", tag));
    //assert_int_equal(total--, tag->frames->length);

    id3v2DestroyTag(&tag);

}

int main(){
    const struct CMUnitTest tests[] = {
        
        // id3v2TagFromFile
        cmocka_unit_test(id3v2TagFromFile_v3),
        cmocka_unit_test(id3v2TagFromFile_null),

        // id3v2CopyTag
        cmocka_unit_test(id3v2CopyTag_v3),

        // id3v2CompareTag
        cmocka_unit_test(id3v2CompareTag_v3v4),
        cmocka_unit_test(id3v2CompareTag_v3same),

        // id3v2ReadFrameByID
        cmocka_unit_test(id3v2ReadFrameByID_v3),
        cmocka_unit_test(id3v2ReadFrameByID_v3MultiTXXX),
        cmocka_unit_test(id3v2ReadFrameByID_v2Null),

        // id3v2RemoveFrameByID
        cmocka_unit_test(id3v2RemoveFrameByID_v3EveryFrame),

    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}