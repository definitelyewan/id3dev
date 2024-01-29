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
#include "byteStream.h"

static void id3v2ParseExtendedTagHeader_nullData(void **state){
    
    Id3v2ExtendedTagHeader *h = id3v2ParseExtendedTagHeader(NULL, ID3V2_TAG_VERSION_2);

    assert_null(h);
}

static void id3v2ParseExtendedTagHeader_v2(void **state){
    
    ByteStream *stream = byteStreamCreate(NULL, 1);
    
    Id3v2ExtendedTagHeader *h = id3v2ParseExtendedTagHeader(stream, ID3V2_TAG_VERSION_2);

    byteStreamDestroy(stream);
    assert_null(h);
}

static void id3v2ParseExtendedTagHeader_v3(void **state){
    
    uint8_t ext[14] = {0,0,0,0x0A,      // size
                       0x80,0,          // flags
                       0,0,0xff,0xff,   // padding
                       0,0,0,15};       // crc

    ByteStream *stream = byteStreamCreate(ext, 14);

    Id3v2ExtendedTagHeader *h = id3v2ParseExtendedTagHeader(stream, ID3V2_TAG_VERSION_3);

    assert_non_null(h);
    assert_int_equal(h->padding, 0xffff);
    assert_int_equal(h->crc, 15);
    
    id3v2DestroyExtendedTagHeader(&h);
    byteStreamDestroy(stream);
}

static void id3v2ParseExtendedTagHeader_v3NoCrc(void **state){
    
    uint8_t ext[10] = {0,0,0,0x0A,      // size
                       0x80,0,          // flags
                       0,0,0xff,0xff};  // padding
                       

    ByteStream *stream = byteStreamCreate(ext, 10);

    Id3v2ExtendedTagHeader *h = id3v2ParseExtendedTagHeader(stream, ID3V2_TAG_VERSION_3);

    assert_non_null(h);
    assert_int_equal(h->padding, 0xffff);
    assert_int_equal(h->crc, 0);
    
    id3v2DestroyExtendedTagHeader(&h);
    byteStreamDestroy(stream);
}

static void id3v2ParseExtendedTagHeader_v3NoPadding(void **state){
    
    uint8_t ext[10] = {0,0,0,0x0A,      // size
                       0x80,0};         // flags
                       

    ByteStream *stream = byteStreamCreate(ext, 10);

    Id3v2ExtendedTagHeader *h = id3v2ParseExtendedTagHeader(stream, ID3V2_TAG_VERSION_3);

    assert_null(h);
    
    byteStreamDestroy(stream);
}

int main(){
    const struct CMUnitTest tests[] = {
        
        // id3v2ParseExtendedTagHeader tests
        cmocka_unit_test(id3v2ParseExtendedTagHeader_nullData),
        cmocka_unit_test(id3v2ParseExtendedTagHeader_v2),
        cmocka_unit_test(id3v2ParseExtendedTagHeader_v3),
        cmocka_unit_test(id3v2ParseExtendedTagHeader_v3NoCrc),
        cmocka_unit_test(id3v2ParseExtendedTagHeader_v3NoPadding)

    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}