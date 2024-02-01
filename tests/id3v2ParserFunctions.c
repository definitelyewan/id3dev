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
    
    Id3v2ExtendedTagHeader *h;
    
    uint32_t v = id3v2ParseExtendedTagHeader(NULL, ID3V2_TAG_VERSION_2, &h);
    
    assert_int_equal(v, 0),
    assert_null(h);
}

static void id3v2ParseExtendedTagHeader_v2(void **state){
    
    ByteStream *stream = byteStreamCreate(NULL, 1);
    
    Id3v2ExtendedTagHeader *h;
    
    uint32_t v = id3v2ParseExtendedTagHeader(stream, ID3V2_TAG_VERSION_2, &h);

    byteStreamDestroy(stream);
    assert_null(h);
    assert_int_equal(v, 0);
}

static void id3v2ParseExtendedTagHeader_v3(void **state){
    
    uint8_t ext[14] = {0,0,0,0x0A,      // size
                       0x80,0,          // flags
                       0,0,0xff,0xff,   // padding
                       0,0,0,15};       // crc

    ByteStream *stream = byteStreamCreate(ext, 14);

    Id3v2ExtendedTagHeader *h;
    uint32_t v = id3v2ParseExtendedTagHeader(stream, ID3V2_TAG_VERSION_3, &h);

    assert_int_equal(v, 14);

    assert_non_null(h);
    assert_int_equal(h->padding, 0xffff);
    assert_int_equal(h->crc, 0xf);
    
    id3v2DestroyExtendedTagHeader(&h);
    byteStreamDestroy(stream);
}

static void id3v2ParseExtendedTagHeader_v3NoCrc(void **state){
    
    uint8_t ext[10] = {0,0,0,0x06,      // size
                       0x80,0,          // flags
                       0,0,0xff,0xff};  // padding
                       

    ByteStream *stream = byteStreamCreate(ext, 10);

    Id3v2ExtendedTagHeader *h;
    
    uint32_t v = id3v2ParseExtendedTagHeader(stream, ID3V2_TAG_VERSION_3, &h);

    assert_int_equal(v, 10);

    assert_non_null(h);
    assert_int_equal(h->padding, 0xffff);
    assert_int_equal(h->crc, 0);
    
    id3v2DestroyExtendedTagHeader(&h);
    byteStreamDestroy(stream);
}

static void id3v2ParseExtendedTagHeader_v3NoPadding(void **state){
    
    uint8_t ext[6] = {0,0,0,0x02,      // size
                       0x80,0};        // flags
                       

    ByteStream *stream = byteStreamCreate(ext, 6);

    Id3v2ExtendedTagHeader *h;
    
    uint32_t v = id3v2ParseExtendedTagHeader(stream, ID3V2_TAG_VERSION_3, &h);

    assert_non_null(h);
    assert_int_equal(v, 6);
    assert_int_equal(h->padding, 0);
    assert_int_equal(h->crc, 0);
    
    byteStreamDestroy(stream);
    id3v2DestroyExtendedTagHeader(&h);

}

static void id3v2ParseExtendedTagHeader_v3UnsupportedSize(void **state){
    
    uint8_t ext[14] = {0,0,0,0x90,      // size
                       0x80,0,          // flags
                       0,0,0xff,0xff,   // padding
                       0,0,0,15};       // crc

    ByteStream *stream = byteStreamCreate(ext, 14);

    Id3v2ExtendedTagHeader *h;
    uint32_t v = id3v2ParseExtendedTagHeader(stream, ID3V2_TAG_VERSION_3, &h);

    assert_int_equal(v, 14);

    assert_non_null(h);
    assert_int_equal(h->padding, 0xffff);
    assert_int_equal(h->crc, 0xf);
    
    id3v2DestroyExtendedTagHeader(&h);
    byteStreamDestroy(stream);

}

static void id3v2ParseExtendedTagHeader_v3SmallSizeWithData(void **state){
    
    uint8_t ext[6] = {0,0,0,0x90,      // size
                       0x80,0};        // flags

    ByteStream *stream = byteStreamCreate(ext, 6);

    Id3v2ExtendedTagHeader *h;
    uint32_t v = id3v2ParseExtendedTagHeader(stream, ID3V2_TAG_VERSION_3, &h);

    assert_int_equal(v, 10);

    assert_non_null(h);
    assert_int_equal(h->padding, 0);
    assert_int_equal(h->crc, 0);
    
    id3v2DestroyExtendedTagHeader(&h);
    byteStreamDestroy(stream);

}


static void id3v2ParseExtendedTagHeader_v4(void **state){
    
    uint8_t ext[12] = {0,0,0,8,
                       6,
                       0x70,
                       0,0,0,0x3E,0x48,
                       0xff};
                       

    ByteStream *stream = byteStreamCreate(ext, 12);

    Id3v2ExtendedTagHeader *h;
    uint32_t v = id3v2ParseExtendedTagHeader(stream, ID3V2_TAG_VERSION_4, &h);

    assert_int_equal(v, 12);

    assert_int_equal(h->crc, 8008);
    assert_true(h->update);
    assert_true(h->tagRestrictions);
    assert_int_equal(h->restrictions, 0xff);
    
    byteStreamDestroy(stream);
    id3v2DestroyExtendedTagHeader(&h);
}

static void id3v2ParseExtendedTagHeader_v4NoRestrictions(void **state){
    
    uint8_t ext[11] = {0,0,0,7,
                       5,
                       0x70,
                       0,0,0,0x3E,0x48};
                       

    ByteStream *stream = byteStreamCreate(ext, 11);

    Id3v2ExtendedTagHeader *h;
    uint32_t v = id3v2ParseExtendedTagHeader(stream, ID3V2_TAG_VERSION_4, &h);

    assert_int_equal(v, 11);
    assert_int_equal(h->crc, 8008);
    assert_true(h->update);
    assert_false(h->tagRestrictions);
    assert_int_equal(h->restrictions, 0);

    byteStreamDestroy(stream);
    id3v2DestroyExtendedTagHeader(&h);
}

static void id3v2ParseExtendedTagHeader_v4NoCRC(void **state){
    
    uint8_t ext[6] = {0,0,0,7,
                       0,
                       0x50};
                       

    ByteStream *stream = byteStreamCreate(ext, 6);

    Id3v2ExtendedTagHeader *h;
    uint32_t v = id3v2ParseExtendedTagHeader(stream, ID3V2_TAG_VERSION_4, &h);

    assert_int_equal(v, 6);
    assert_int_equal(h->crc, 0);
    assert_true(h->update);
    assert_false(h->tagRestrictions);
    assert_int_equal(h->restrictions, 0);

    byteStreamDestroy(stream);
    id3v2DestroyExtendedTagHeader(&h);
}

// static void playground(void **state){
    
//     uint8_t ext[6] = {0,0,0,7,
//                        6,
//                        0x50};
                       

//     ByteStream *stream = byteStreamCreate(ext, 11);

//     Id3v2ExtendedTagHeader *h = id3v2ParseExtendedTagHeader(stream, ID3V2_TAG_VERSION_4);

//     assert_int_equal(h->crc, 0);
//     assert_true(h->update);
//     assert_false(h->tagRestrictions);
//     assert_int_equal(h->restrictions, 0);

//     byteStreamDestroy(stream);
//     id3v2DestroyExtendedTagHeader(&h);
// }

int main(){
    const struct CMUnitTest tests[] = {
        
        // id3v2ParseExtendedTagHeader tests
        cmocka_unit_test(id3v2ParseExtendedTagHeader_nullData),

        cmocka_unit_test(id3v2ParseExtendedTagHeader_v2),

        cmocka_unit_test(id3v2ParseExtendedTagHeader_v3),
        cmocka_unit_test(id3v2ParseExtendedTagHeader_v3NoCrc),
        cmocka_unit_test(id3v2ParseExtendedTagHeader_v3NoPadding),
        cmocka_unit_test(id3v2ParseExtendedTagHeader_v3UnsupportedSize),
        cmocka_unit_test(id3v2ParseExtendedTagHeader_v3SmallSizeWithData),
        
        cmocka_unit_test(id3v2ParseExtendedTagHeader_v4),
        cmocka_unit_test(id3v2ParseExtendedTagHeader_v4NoRestrictions),
        cmocka_unit_test(id3v2ParseExtendedTagHeader_v4NoCRC),

        // cmocka_unit_test(playground)

    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}