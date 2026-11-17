#include <stdio.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include "id3v2/id3v2TagIdentity.h"
#include "id3v2/id3v2Frame.h"
#include "byteStream.h"
#include "byteInt.h"

static void id3v2NewTagHeader_validStruct(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 0,NULL);

    assert_int_equal(header->majorVersion, 4);
    assert_int_equal(header->minorVersion, 0);
    assert_int_equal(header->flags, 0);
    assert_null(header->extendedHeader);

    free(header);
}

static void id3v2DestroyTagHeader_freeStruct(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 0,NULL);
    id3v2DestroyTagHeader(&header);

    assert_null(header);
}

static void id3v2DestroyTagHeader_failToFreeStruct(void **state) {
    (void) state;
    //fails if segfault
    Id3v2TagHeader *test = NULL;
    id3v2DestroyTagHeader(&test);
}

static void id3v2SetUnsynchronisationIndicator_set0(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 128,NULL);

    int v = id3v2WriteUnsynchronisationIndicator(header, 0);

    assert_true(v);
    assert_int_equal(header->flags, 0);
    id3v2DestroyTagHeader(&header);
}

static void id3v2SetUnsynchronisationIndicator_set1While1(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 128,NULL);

    int v = id3v2WriteUnsynchronisationIndicator(header, 1);

    assert_true(v);
    assert_int_equal(header->flags, 128);
    id3v2DestroyTagHeader(&header);
}

static void id3v2SetUnsynchronisationIndicator_NULLHeader(void **state) {
    (void) state;

    Id3v2TagHeader *header = NULL;

    int v = id3v2WriteUnsynchronisationIndicator(header, 1);

    assert_false(v);
}

static void id3v2SetCompressionIndicator_set1(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(2, 0, 0,NULL);


    int v = id3v2WriteCompressionIndicator(header, 1);

    assert_true(v);
    assert_int_equal(64, header->flags);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetCompressionIndicator_setNoneVersion2(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(3, 0, 0,NULL);


    int v = id3v2WriteCompressionIndicator(header, 1);

    assert_false(v);
    assert_int_equal(0, header->flags);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetCompressionIndicator_setWithNullHeader(void **state) {
    (void) state;

    Id3v2TagHeader *header = NULL;

    int v = id3v2WriteCompressionIndicator(header, 1);

    assert_false(v);
}

static void id3v2SetExtendedHeaderIndicator_set0(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(3, 0, 64,NULL);

    int v = id3v2WriteExtendedHeaderIndicator(header, 0);

    assert_true(v);
    assert_int_equal(header->flags, 0);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetExtendedHeaderIndicator_setWhileOtherFlagIsSet(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(3, 0, 128,NULL);

    int v = id3v2WriteExtendedHeaderIndicator(header, 1);

    assert_true(v);
    assert_int_equal(header->flags, 192);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetExtendedHeaderIndicator_setWrongVersion(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(2, 0, 128,NULL);

    int v = id3v2WriteExtendedHeaderIndicator(header, 1);

    assert_false(v);
    assert_int_equal(header->flags, 128);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetExperimentalIndicator_set1(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 0,NULL);

    int v = id3v2WriteExperimentalIndicator(header, 1);

    assert_true(v);
    assert_int_equal(header->flags, 32);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetExperimentalIndicator_setWrongVersion(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(2, 0, 0,NULL);

    int v = id3v2WriteExperimentalIndicator(header, 1);

    assert_false(v);
    assert_int_equal(header->flags, 0);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetFooterIndicator_set0(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 0,NULL);

    int v = id3v2WriteExperimentalIndicator(header, 1);

    assert_true(v);
    assert_int_equal(header->flags, 32);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetFooterIndicator_setWrongVersion(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(2, 0, 16,NULL);

    int v = id3v2WriteFooterIndicator(header, 1);

    assert_false(v);
    assert_int_equal(header->flags, 16);

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadUnsynchronisationIndicator_validStruct(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 128,NULL);

    assert_int_equal(id3v2ReadUnsynchronisationIndicator(header), 1);
    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadUnsynchronisationIndicator_NULLStruct(void **state) {
    (void) state;

    Id3v2TagHeader *header = NULL;

    assert_int_equal(id3v2ReadUnsynchronisationIndicator(header), -1);
}

static void id3v2ReadCompressionIndicator_validStruct(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(2, 0, 64,NULL);

    assert_int_equal(id3v2ReadCompressionIndicator(header), 1);
    header->flags = 0;
    assert_int_equal(id3v2ReadCompressionIndicator(header), 0);

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadCompressionIndicator_NULLStruct(void **state) {
    (void) state;

    Id3v2TagHeader *header = NULL;

    assert_int_equal(id3v2ReadCompressionIndicator(header), -1);
}

static void id3v2ReadCompressionIndicator_WrongVersion(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64,NULL);;

    assert_int_equal(id3v2ReadCompressionIndicator(header), -1);
    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadExtendedHeaderIndicator_validStruct(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(3, 0, 64,NULL);

    assert_int_equal(id3v2ReadExtendedHeaderIndicator(header), 1);
    header->flags = 0;
    assert_int_equal(id3v2ReadExtendedHeaderIndicator(header), 0);

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadExtendedHeaderIndicator_NULLStruct(void **state) {
    (void) state;

    Id3v2TagHeader *header = NULL;

    assert_int_equal(id3v2ReadExtendedHeaderIndicator(header), -1);
}

static void id3v2ReadExtendedHeaderIndicator_WrongVersion(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(2, 0, 0,NULL);

    assert_int_equal(id3v2ReadExtendedHeaderIndicator(header), -1);
    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadExperimentalIndicator_validStruct(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(3, 0, 32,NULL);

    assert_int_equal(id3v2ReadExperimentalIndicator(header), 1);
    header->flags = 0;
    assert_int_equal(id3v2ReadExperimentalIndicator(header), 0);

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadExperimentalIndicator_NULLStruct(void **state) {
    (void) state;

    Id3v2TagHeader *header = NULL;

    assert_int_equal(id3v2ReadExperimentalIndicator(header), -1);
}

static void id3v2ReadExperimentalIndicator_WrongVersion(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(2, 0, 0,NULL);;

    assert_int_equal(id3v2ReadExperimentalIndicator(header), -1);
    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadFooterIndicator_validStruct(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 16,NULL);

    assert_int_equal(id3v2ReadFooterIndicator(header), 1);
    header->flags = 0;
    assert_int_equal(id3v2ReadFooterIndicator(header), 0);

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadFooterIndicator_NULLStruct(void **state) {
    (void) state;

    Id3v2TagHeader *header = NULL;

    assert_int_equal(id3v2ReadFooterIndicator(header), -1);
}

static void id3v2ReadFooterIndicator_WrongVersion(void **state) {
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(2, 0, 0,NULL);

    assert_int_equal(id3v2ReadFooterIndicator(header), -1);
    id3v2DestroyTagHeader(&header);
}

static void id3v2NewExtendedTagHeader_validStruct(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(150, 909, 1, 1, 255);

    assert_int_equal(ext->padding, 150);
    assert_int_equal(ext->crc, 909);
    assert_int_equal(ext->update, 1);
    assert_int_equal(ext->tagRestrictions, 1);
    assert_int_equal(ext->restrictions, 255);

    free(ext);
}

static void id3v2DestroyExtendedTagHeader_DestroyStruct(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(150, 909, 1, 1, 255);
    id3v2DestroyExtendedTagHeader(&ext);
    assert_null(ext);
}

static void id3v2DestroyExtendedTagHeader_tryFreeNULL(void **state) {
    (void) state;
    //fails on segfault
    Id3v2ExtendedTagHeader *ext = NULL;
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2WriteTagSizeRestriction_noExtAlready(void **state) {
    (void) state;
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, NULL);

    assert_true(id3v2WriteTagSizeRestriction(header, 3));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 192);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTagSizeRestriction_changeCurrentValue(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, 0, 1, 192);
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_true(id3v2WriteTagSizeRestriction(header, 1));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 64);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTagSizeRestriction_wrongVersion(void **state) {
    (void) state;

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, 0, 1, 192);
    Id3v2TagHeader *header = id3v2CreateTagHeader(2, 0, 64, ext);

    assert_false(id3v2WriteTagSizeRestriction(header, 1));

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTagSizeRestriction_notAOption(void **state) {
    (void) state;

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, 0, 1, 192);
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_false(id3v2WriteTagSizeRestriction(header, 9));
    assert_int_equal(header->extendedHeader->restrictions, 192);
    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTextEncodingRestriction_noExtAlready(void **state) {
    (void) state;
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, NULL);

    assert_true(id3v2WriteTextEncodingRestriction(header, 1));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 32);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTextEncodingRestriction_changeCurrentValue(void **state) {
    (void) state;

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, 0, 1, 32);
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_true(id3v2WriteTextEncodingRestriction(header, 0));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 0);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTextEncodingRestriction_wrongVersion(void **state) {
    (void) state;

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, 0, 1, 32);
    Id3v2TagHeader *header = id3v2CreateTagHeader(2, 0, 64, ext);

    assert_false(id3v2WriteTagSizeRestriction(header, 0));

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTextEncodingRestriction_notAOption(void **state) {
    (void) state;

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, 0, 1, 32);
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_false(id3v2WriteTagSizeRestriction(header, 100));
    assert_int_equal(header->extendedHeader->restrictions, 32);
    id3v2DestroyTagHeader(&header);
}


static void id3v2WriteTextFieldsSizeRestriction_noExtAlready(void **state) {
    (void) state;
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, NULL);

    assert_true(id3v2WriteTextFieldsSizeRestriction(header, 2));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 16);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTextFieldsSizeRestriction_changeCurrentValue(void **state) {
    (void) state;

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, 0, 1, 16);
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_true(id3v2WriteTextFieldsSizeRestriction(header, 3));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 24);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTextFieldsSizeRestriction_wrongVersion(void **state) {
    (void) state;

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, 0, 1, 16);
    Id3v2TagHeader *header = id3v2CreateTagHeader(2, 0, 64, ext);

    assert_false(id3v2WriteTextFieldsSizeRestriction(header, 0));

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTextFieldsSizeRestriction_notAOption(void **state) {
    (void) state;

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, 0, 1, 24);
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_false(id3v2WriteTagSizeRestriction(header, 100));
    assert_int_equal(header->extendedHeader->restrictions, 24);
    id3v2DestroyTagHeader(&header);
}


static void id3v2WriteImageEncodingRestriction_noExtAlready(void **state) {
    (void) state;
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, NULL);

    assert_true(id3v2WriteImageEncodingRestriction(header, 1));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 4);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteImageEncodingRestriction_changeCurrentValue(void **state) {
    (void) state;

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, 0, 1, 4);
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_true(id3v2WriteImageEncodingRestriction(header, 0));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 0);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteImageEncodingRestriction_wrongVersion(void **state) {
    (void) state;

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, 0, 1, 4);
    Id3v2TagHeader *header = id3v2CreateTagHeader(2, 0, 64, ext);
    bool v = id3v2WriteImageEncodingRestriction(header, 0);
    assert_false(v);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteImageSizeRestriction_noExtAlready(void **state) {
    (void) state;
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, NULL);

    assert_true(id3v2WriteImageSizeRestriction(header, 2));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 2);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteImageSizeRestriction_changeCurrentValue(void **state) {
    (void) state;

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, 0, 1, 2);
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_true(id3v2WriteImageSizeRestriction(header, 3));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 3);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteImageSizeRestriction_wrongVersion(void **state) {
    (void) state;

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, 0, 1, 3);
    Id3v2TagHeader *header = id3v2CreateTagHeader(2, 0, 64, ext);

    assert_false(id3v2WriteImageSizeRestriction(header, 0));

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteImageSizeRestriction_notAOption(void **state) {
    (void) state;

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, 0, 1, 3);
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_false(id3v2WriteImageSizeRestriction(header, 100));
    assert_int_equal(header->extendedHeader->restrictions, 3);
    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadTagSizeRestriction_readBits(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 192);
    //11000000

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_int_equal(3, id3v2ReadTagSizeRestriction(header));

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadTagSizeRestriction_readBits2(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 128);
    //11000000

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_int_equal(2, id3v2ReadTagSizeRestriction(header));

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadTextEncodingRestriction_readBit(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 32);
    //00100000

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_int_equal(1, id3v2ReadTextEncodingRestriction(header));

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadTextEncodingRestriction_readBit2(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 0);
    //00000000

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_int_equal(0, id3v2ReadTextEncodingRestriction(header));

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadTextFieldsSizeRestriction_readBit(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 24);
    //00011000

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_int_equal(3, id3v2ReadTextFieldsSizeRestriction(header));

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadTextFieldsSizeRestriction_readBit2(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 8);
    //00001000

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_int_equal(1, id3v2ReadTextFieldsSizeRestriction(header));

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadImageEncodingRestriction_readBit(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 4);
    //00000100

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_int_equal(1, id3v2ReadImageEncodingRestriction(header));

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadImageEncodingRestriction_readBit2(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 0);
    //00000000

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_int_equal(0, id3v2ReadImageEncodingRestriction(header));

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadImageSizeRestriction_readBit(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 3);
    //00000011

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_int_equal(3, id3v2ReadImageSizeRestriction(header));

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadImageSizeRestriction_readBit2(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 1);
    //00000001

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_int_equal(1, id3v2ReadImageSizeRestriction(header));

    id3v2DestroyTagHeader(&header);
}

static void id3v2ClearTagRestrictions_clear(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 1);
    //00000001

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);
    assert_true(id3v2ClearTagRestrictions(header));
    assert_false(header->extendedHeader->tagRestrictions);
    id3v2DestroyTagHeader(&header);
}


static void id3v2TagCreateAndDestroy_AllInOne(void **state) {
    (void) state;
    Id3v2Tag *tag = id3v2CreateTag(id3v2CreateTagHeader(1, 2, 10, id3v2CreateExtendedTagHeader(15, 50, 1, 1, 10)),
                                   listCreate(id3v2PrintFrame, id3v2DeleteFrame, id3v2CompareFrame, id3v2CopyFrame));

    assert_non_null(tag);
    assert_non_null(tag->frames);
    assert_non_null(tag->header);

    id3v2DestroyTag(&tag);

    assert_null(tag);
}

static void id3v2ExtendedTagHeaderSerialize_v3noCRC(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(100, 0, 0, 0, 0);
    const uint8_t expected[] = {0, 0, 0, 6 /* size, excludes itself */, 0, 0 /* flags */, 0, 0, 0, 100 /* padding */};
    size_t outl = 0;
    uint8_t *out = id3v2ExtendedTagHeaderSerialize(ext, ID3V2_TAG_VERSION_3, &outl);

    assert_non_null(out);
    assert_int_equal(outl, sizeof(expected));
    assert_memory_equal(out, expected, sizeof(expected));

    free(out);
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2ExtendedTagHeaderSerialize_v3CRC(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(UINT32_MAX, UINT32_MAX, 1, 1, 1);
    const uint8_t expected[] = {0, 0, 0, 10 /* size */, 0x80, 0 /* CRC flag */, 0xff, 0xff, 0xff, 0xff /* padding */, 0xff, 0xff, 0xff, 0xff /* crc */};
    size_t outl = 0;
    uint8_t *out = id3v2ExtendedTagHeaderSerialize(ext, ID3V2_TAG_VERSION_3, &outl);

    assert_non_null(out);
    assert_int_equal(outl, sizeof(expected));
    assert_memory_equal(out, expected, sizeof(expected));

    free(out);
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2ExtendedTagHeaderSerialize_null(void **state) {
    (void) state;
    size_t outl = 0;
    uint8_t *out = id3v2ExtendedTagHeaderSerialize(NULL, ID3V2_TAG_VERSION_3, &outl);

    assert_null(out);
}

static void id3v2ExtendedTagHeaderSerialize_v4WithEverything(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(UINT32_MAX, UINT32_MAX, 1, 1, 0xfe);
    const uint8_t expected[] = {0, 0, 0, 15 /* syncsafe size of the whole header */, 1 /* flag bytes */, 0x70 /* update, CRC, restrictions */, 0x00 /* update length */, 0x05, 0x0f, 0x7f, 0x7f, 0x7f, 0x7f /* CRC length and syncsafe CRC */, 0x01, 0xfe /* restrictions length and restrictions */};
    size_t outl = 0;
    uint8_t *out = id3v2ExtendedTagHeaderSerialize(ext, ID3V2_TAG_VERSION_4, &outl);

    assert_non_null(out);
    assert_int_equal(outl, sizeof(expected));
    assert_memory_equal(out, expected, sizeof(expected));

    free(out);
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2ExtendedTagHeaderSerialize_v4(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, 0, 0, 0);
    const uint8_t expected[] = {0, 0, 0, 6 /* size */, 1 /* flag bytes */, 0x00 /* flags */};
    size_t outl = 0;
    uint8_t *out = id3v2ExtendedTagHeaderSerialize(ext, ID3V2_TAG_VERSION_4, &outl);

    assert_non_null(out);
    assert_int_equal(outl, sizeof(expected));
    assert_memory_equal(out, expected, sizeof(expected));

    free(out);
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2ExtendedTagHeaderSerialize_v4crc(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 9000, 0, 0, 0);
    const uint8_t expected[] = {0, 0, 0, 12 /* size */, 1 /* flag bytes */, 0x20 /* CRC */, 0x05, 0, 0, 0, 0x46, 0x28 /* CRC length and syncsafe 9000 */};
    size_t outl = 0;
    uint8_t *out = id3v2ExtendedTagHeaderSerialize(ext, ID3V2_TAG_VERSION_4, &outl);

    assert_non_null(out);
    assert_int_equal(outl, sizeof(expected));
    assert_memory_equal(out, expected, sizeof(expected));

    free(out);
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2ExtendedTagHeaderSerialize_v4restrictions(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, 0, 1, 0xff);
    const uint8_t expected[] = {0, 0, 0, 8 /* size */, 1 /* flag bytes */, 0x10 /* restrictions */, 0x01, 0xff /* restrictions length and restrictions */};
    size_t outl = 0;
    uint8_t *out = id3v2ExtendedTagHeaderSerialize(ext, ID3V2_TAG_VERSION_4, &outl);

    assert_non_null(out);
    assert_int_equal(outl, sizeof(expected));
    assert_memory_equal(out, expected, sizeof(expected));

    free(out);
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2ExtendedTagHeaderToJSON_v3CRC(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(UINT32_MAX, UINT32_MAX, 1, 1, 0xfe);
    char *json = id3v2ExtendedTagHeaderToJSON(ext, ID3V2_TAG_VERSION_3);

    assert_non_null(json);
    assert_string_equal(json,
                        "{\"padding\":4294967295,\"crc\":4294967295}");

    free(json);
    id3v2DestroyExtendedTagHeader(&ext);
}


static void id3v2ExtendedTagHeaderToJSON_v3noCRC(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(100, 0, 0, 0, 0);
    char *json = id3v2ExtendedTagHeaderToJSON(ext, ID3V2_TAG_VERSION_3);

    assert_non_null(json);
    assert_string_equal(json,
                        "{\"padding\":100,\"crc\":0}");

    free(json);
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2ExtendedTagHeaderToJSON_null(void **state) {
    (void) state;
    char *json = id3v2ExtendedTagHeaderToJSON(NULL, ID3V2_TAG_VERSION_2);

    assert_non_null(json);
    assert_string_equal(json, "{}");

    free(json);
}

static void id3v2ExtendedTagHeaderToJSON_v4WithEverything(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(UINT32_MAX, UINT32_MAX, 1, 1, 0xfe);
    char *json = id3v2ExtendedTagHeaderToJSON(ext, ID3V2_TAG_VERSION_4);

    assert_non_null(json);
    assert_string_equal(json,
                        "{\"padding\":4294967295,\"crc\":4294967295,\"update\":true,\"tagRestrictions\":true,\"restrictions\":254}");

    free(json);
    id3v2DestroyExtendedTagHeader(&ext);
}


static void id3v2ExtendedTagHeaderToJSON_v4crc(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 9000, 0, 0, 0);
    char *json = id3v2ExtendedTagHeaderToJSON(ext, ID3V2_TAG_VERSION_4);

    assert_non_null(json);
    assert_string_equal(json,
                        "{\"padding\":0,\"crc\":9000,\"update\":false,\"tagRestrictions\":false,\"restrictions\":0}");

    free(json);
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2ExtendedTagHeaderToJSON_v4restrictions(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, 0, 1, 0xff);
    char *json = id3v2ExtendedTagHeaderToJSON(ext, ID3V2_TAG_VERSION_4);

    assert_non_null(json);
    assert_string_equal(json,
                        "{\"padding\":0,\"crc\":0,\"update\":false,\"tagRestrictions\":true,\"restrictions\":255}");
    free(json);
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2TagHeaderSerialize_v2(void **state) {
    (void) state;
    Id3v2TagHeader *h = id3v2CreateTagHeader(2, 0, 0, NULL);
    size_t outl = 0;
    uint8_t *out = id3v2TagHeaderSerialize(h, 1000, &outl);
    unsigned char *tmp = NULL;

    ByteStream *stream = byteStreamCreate(out, outl);

    assert_non_null(stream);

    assert_memory_equal(byteStreamCursor(stream), "ID3", 3);
    byteStreamSeek(stream, 3, SEEK_CUR);

    assert_int_equal(byteStreamCursor(stream)[0], 2);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(byteStreamCursor(stream)[0], 0);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(byteStreamCursor(stream)[0], 0);
    byteStreamSeek(stream, 1, SEEK_CUR);

    tmp = u32tob(byteSyncintEncode(1000));
    assert_memory_equal(byteStreamCursor(stream), tmp, 4);
    free(tmp);

    id3v2DestroyTagHeader(&h);
    free(out);
    byteStreamDestroy(stream);
}

static void id3v2TagHeaderSerialize_unsupportedVersion(void **state) {
    (void) state;
    Id3v2TagHeader *h = id3v2CreateTagHeader(10, 0, 0, NULL);
    size_t outl = 0;
    uint8_t *out = id3v2TagHeaderSerialize(h, 1000, &outl);

    assert_null(out);
    assert_int_equal(outl, 0);

    id3v2DestroyTagHeader(&h);
}

static void id3v2TagHeaderSerialize_null(void **state) {
    (void) state;
    size_t outl = 0;
    uint8_t *out = id3v2TagHeaderSerialize(NULL, 1000, &outl);

    assert_null(out);
    assert_int_equal(outl, 0);
}


static void id3v2TagHeaderSerialize_v3(void **state) {
    (void) state;
    Id3v2TagHeader *h = id3v2CreateTagHeader(3, 1, 0x20, NULL);
    size_t outl = 0;
    uint8_t *out = id3v2TagHeaderSerialize(h, 1000, &outl);
    ByteStream *stream = byteStreamCreate(out, outl);
    unsigned char *tmp = NULL;


    assert_non_null(stream);

    assert_memory_equal(byteStreamCursor(stream), "ID3", 3);
    byteStreamSeek(stream, 3, SEEK_CUR);

    assert_int_equal(byteStreamCursor(stream)[0], 3);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(byteStreamCursor(stream)[0], 1);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(byteStreamCursor(stream)[0], 0x20);
    byteStreamSeek(stream, 1, SEEK_CUR);

    tmp = u32tob(byteSyncintEncode(1000));
    assert_memory_equal(byteStreamCursor(stream), tmp, 4);
    free(tmp);

    id3v2DestroyTagHeader(&h);
    free(out);
    byteStreamDestroy(stream);
}

static void id3v2TagHeaderSerialize_v3WithExt(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(100, 0, 0, 0, 0);
    Id3v2TagHeader *h = id3v2CreateTagHeader(3, 1, 0x60, ext);
    const uint8_t extExpected[] = {0, 0, 0, 6, 0, 0, 0, 0, 0, 100};
    size_t outl = 0;
    uint8_t *out = id3v2TagHeaderSerialize(h, 1000, &outl);
    uint8_t *sizeBytes = u32tob(byteSyncintEncode(1000));

    assert_non_null(out);
    assert_int_equal(outl, 10 + sizeof(extExpected));
    assert_memory_equal(out, "ID3", 3);
    assert_int_equal(out[3], 3);
    assert_int_equal(out[4], 1);
    assert_int_equal(out[5], 0x60);
    assert_memory_equal(out + 6, sizeBytes, 4);
    assert_memory_equal(out + 10, extExpected, sizeof(extExpected));

    free(sizeBytes);
    id3v2DestroyTagHeader(&h);
    free(out);
}

static void id3v2TagHeaderSerialize_v4(void **state) {
    (void) state;
    Id3v2TagHeader *h = id3v2CreateTagHeader(4, 0, 0, NULL);
    size_t outl = 0;
    uint8_t *out = id3v2TagHeaderSerialize(h, 900, &outl);
    ByteStream *stream = byteStreamCreate(out, outl);
    unsigned char *tmp = NULL;


    assert_non_null(stream);

    assert_memory_equal(byteStreamCursor(stream), "ID3", 3);
    byteStreamSeek(stream, 3, SEEK_CUR);

    assert_int_equal(byteStreamCursor(stream)[0], 4);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(byteStreamCursor(stream)[0], 0);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(byteStreamCursor(stream)[0], 0);
    byteStreamSeek(stream, 1, SEEK_CUR);

    tmp = u32tob(byteSyncintEncode(900));
    assert_memory_equal(byteStreamCursor(stream), tmp, 4);
    free(tmp);
    byteStreamSeek(stream, 4, SEEK_CUR);

    id3v2DestroyTagHeader(&h);
    free(out);
    byteStreamDestroy(stream);
}

static void id3v2TagHeaderSerialize_v4WithExt(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(80, 870, 1, 1, 0xff);
    Id3v2TagHeader *h = id3v2CreateTagHeader(4, 0, 0xF0, ext);
    const uint8_t extExpected[] = {0, 0, 0, 15, 1, 0x70, 0x00, 0x05, 0, 0, 0, 0x06, 0x66, 0x01, 0xff};
    size_t outl = 0;
    uint8_t *out = id3v2TagHeaderSerialize(h, 900, &outl);
    uint8_t *sizeBytes = u32tob(byteSyncintEncode(900));

    assert_non_null(out);
    assert_int_equal(outl, 10 + sizeof(extExpected));
    assert_memory_equal(out, "ID3", 3);
    assert_int_equal(out[3], 4);
    assert_int_equal(out[4], 0);
    assert_int_equal(out[5], 0xF0);
    assert_memory_equal(out + 6, sizeBytes, 4);
    assert_memory_equal(out + 10, extExpected, sizeof(extExpected));

    free(sizeBytes);
    id3v2DestroyTagHeader(&h);
    free(out);
}

static void id3v2TagHeaderToJSON_null(void **state) {
    (void) state;
    char *json = id3v2TagHeaderToJSON(NULL);

    assert_string_equal(json, "{}");

    free(json);
}

static void id3v2TagHeaderToJSON_v2(void **state) {
    (void) state;
    Id3v2TagHeader *header = id3v2CreateTagHeader(2, 99, 0, NULL);

    char *json = id3v2TagHeaderToJSON(header);

    assert_string_equal(json, "{\"major\":2,\"minor\":99,\"flags\":0}");

    id3v2DestroyTagHeader(&header);
    free(json);
}

static void id3v2TagHeaderToJSON_v3(void **state) {
    (void) state;
    Id3v2TagHeader *header = id3v2CreateTagHeader(3, 0, 0, NULL);

    char *json = id3v2TagHeaderToJSON(header);

    assert_string_equal(json, "{\"major\":3,\"minor\":0,\"flags\":0,\"extended\":{}}");

    id3v2DestroyTagHeader(&header);
    free(json);
}

static void id3v2TagHeaderToJSON_v3ext(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(255, 0, 0, 0, 0);
    Id3v2TagHeader *header = id3v2CreateTagHeader(3, 0, 0, ext);

    char *json = id3v2TagHeaderToJSON(header);

    assert_string_equal(json, "{\"major\":3,\"minor\":0,\"flags\":0,\"extended\":{\"padding\":255,\"crc\":0}}");

    id3v2DestroyTagHeader(&header);
    free(json);
}


static void id3v2TagHeaderToJSON_v4(void **state) {
    (void) state;
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 0x40, NULL);

    char *json = id3v2TagHeaderToJSON(header);

    assert_string_equal(json, "{\"major\":4,\"minor\":0,\"flags\":64,\"extended\":{}}");

    id3v2DestroyTagHeader(&header);
    free(json);
}

static void id3v2TagHeaderToJSON_v4ext(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(100, 3232, 1, 1, 0x1F);
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 0xF0, ext);

    char *json = id3v2TagHeaderToJSON(header);

    assert_string_equal(
        json,
        "{\"major\":4,\"minor\":0,\"flags\":240,\"extended\":{\"padding\":100,\"crc\":3232,\"update\":true,\"tagRestrictions\":true,\"restrictions\":31}}");

    id3v2DestroyTagHeader(&header);
    free(json);
}

/**
 * NOTE
 *
 * the unsync flag does not work and will NOT work.
 * there will be code within the write to file and tag to bytestream that will handle this.
 * Its done this way as pretty much all structs are unaware of the unsync flag
 */

static void id3v2ValidateTagHeader_valid(void **state) {
    (void) state;

    const uint8_t versions[] = {ID3V2_TAG_VERSION_2, ID3V2_TAG_VERSION_3, ID3V2_TAG_VERSION_4};
    const uint8_t allFlags[] = {0x80, 0xE0, 0xF0}; // every defined flag (v2.2 compression excluded)

    for (size_t i = 0; i < sizeof(versions); i++) {
        Id3v2TagHeader *h = id3v2CreateTagHeader(versions[i], 0, 0, NULL);
        assert_true(id3v2ValidateTagHeader(h));
        h->flags = allFlags[i];
        h->minorVersion = 0xFE;
        assert_true(id3v2ValidateTagHeader(h));
        id3v2DestroyTagHeader(&h);
    }
}

static void id3v2ValidateTagHeader_badVersion(void **state) {
    (void) state;

    assert_false(id3v2ValidateTagHeader(NULL));

    // 0x33 is the version from "ID3" bytes found in audio
    const uint8_t majors[] = {0, 1, 5, 0x33, 0xFF};

    for (size_t i = 0; i < sizeof(majors); i++) {
        Id3v2TagHeader *h = id3v2CreateTagHeader(majors[i], 0, 0, NULL);
        assert_false(id3v2ValidateTagHeader(h));
        id3v2DestroyTagHeader(&h);
    }

    Id3v2TagHeader *h = id3v2CreateTagHeader(ID3V2_TAG_VERSION_3, 0xFF, 0, NULL);
    assert_false(id3v2ValidateTagHeader(h));
    id3v2DestroyTagHeader(&h);
}

static void id3v2ValidateTagHeader_undefinedFlags(void **state) {
    (void) state;

    const uint8_t versions[] = {ID3V2_TAG_VERSION_2, ID3V2_TAG_VERSION_3, ID3V2_TAG_VERSION_4};
    const uint8_t undefined[] = {0x3F, 0x1F, 0x0F};

    for (size_t i = 0; i < sizeof(versions); i++) {
        for (unsigned int bit = 0; bit < 8; bit++) {
            if ((undefined[i] >> bit) & 1) {
                Id3v2TagHeader *h = id3v2CreateTagHeader(versions[i], 0, (uint8_t) (1 << bit), NULL);
                assert_false(id3v2ValidateTagHeader(h));
                id3v2DestroyTagHeader(&h);
            }
        }
    }

    // ID3v2.2 compression
    Id3v2TagHeader *h = id3v2CreateTagHeader(ID3V2_TAG_VERSION_2, 0, 0x40, NULL);
    assert_false(id3v2ValidateTagHeader(h));
    id3v2DestroyTagHeader(&h);
}

static void id3v2ValidateTagHeader_extendedHeader(void **state) {
    (void) state;

    // flag set, extended header attached and valid
    Id3v2TagHeader *h = id3v2CreateTagHeader(ID3V2_TAG_VERSION_3, 0, 0x40,
                                             id3v2CreateExtendedTagHeader(10, 0, false, false, 0));
    assert_true(id3v2ValidateTagHeader(h));
    id3v2DestroyTagHeader(&h);

    // flag set, extended header not parsed yet
    h = id3v2CreateTagHeader(ID3V2_TAG_VERSION_4, 0, 0x40, NULL);
    assert_true(id3v2ValidateTagHeader(h));
    id3v2DestroyTagHeader(&h);

    // extended header attached without the flag
    h = id3v2CreateTagHeader(ID3V2_TAG_VERSION_3, 0, 0, id3v2CreateExtendedTagHeader(10, 0, false, false, 0));
    assert_false(id3v2ValidateTagHeader(h));
    id3v2DestroyTagHeader(&h);

    // extended header attached but invalid for the version
    h = id3v2CreateTagHeader(ID3V2_TAG_VERSION_3, 0, 0x40, id3v2CreateExtendedTagHeader(0, 0, true, false, 0));
    assert_false(id3v2ValidateTagHeader(h));
    id3v2DestroyTagHeader(&h);
}

static void id3v2ValidateExtendedTagHeader_v3(void **state) {
    (void) state;

    assert_false(id3v2ValidateExtendedTagHeader(NULL, ID3V2_TAG_VERSION_3));

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0xfefefefe, 0xffffffff, false, false, 0);
    assert_true(id3v2ValidateExtendedTagHeader(ext, ID3V2_TAG_VERSION_3));
    assert_false(id3v2ValidateExtendedTagHeader(ext, ID3V2_TAG_VERSION_2));
    id3v2DestroyExtendedTagHeader(&ext);

    // ID3v2.4 only fields
    ext = id3v2CreateExtendedTagHeader(0, 0, true, false, 0);
    assert_false(id3v2ValidateExtendedTagHeader(ext, ID3V2_TAG_VERSION_3));
    id3v2DestroyExtendedTagHeader(&ext);

    ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 0x12);
    assert_false(id3v2ValidateExtendedTagHeader(ext, ID3V2_TAG_VERSION_3));
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2ValidateExtendedTagHeader_v4(void **state) {
    (void) state;

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0x12345678, true, true, 0xFF);
    assert_true(id3v2ValidateExtendedTagHeader(ext, ID3V2_TAG_VERSION_4));
    id3v2DestroyExtendedTagHeader(&ext);

    // no padding field in ID3v2.4
    ext = id3v2CreateExtendedTagHeader(10, 0, false, false, 0);
    assert_false(id3v2ValidateExtendedTagHeader(ext, ID3V2_TAG_VERSION_4));
    id3v2DestroyExtendedTagHeader(&ext);

    // restrictions without the restrictions flag
    ext = id3v2CreateExtendedTagHeader(0, 0, false, false, 0x01);
    assert_false(id3v2ValidateExtendedTagHeader(ext, ID3V2_TAG_VERSION_4));
    id3v2DestroyExtendedTagHeader(&ext);
}

int main() {
    const struct CMUnitTest tests[] = {

        //id3v2CreateTagHeader tests
        cmocka_unit_test(id3v2NewTagHeader_validStruct),

        //id3v2DestroyTagHeader tests
        cmocka_unit_test(id3v2DestroyTagHeader_freeStruct),
        cmocka_unit_test(id3v2DestroyTagHeader_failToFreeStruct),

        //id3v2WriteUnsynchronisationIndicator tests
        cmocka_unit_test(id3v2SetUnsynchronisationIndicator_set0),
        cmocka_unit_test(id3v2SetUnsynchronisationIndicator_set1While1),
        cmocka_unit_test(id3v2SetUnsynchronisationIndicator_NULLHeader),

        //id3v2WriteUnsynchronisationIndicator tests
        cmocka_unit_test(id3v2SetCompressionIndicator_set1),
        cmocka_unit_test(id3v2SetCompressionIndicator_setNoneVersion2),
        cmocka_unit_test(id3v2SetCompressionIndicator_setWithNullHeader),

        //id3v2WriteExtendedHeaderIndicator tests
        cmocka_unit_test(id3v2SetExtendedHeaderIndicator_set0),
        cmocka_unit_test(id3v2SetExtendedHeaderIndicator_setWhileOtherFlagIsSet),
        cmocka_unit_test(id3v2SetExtendedHeaderIndicator_setWrongVersion),

        //id3v2WriteExperimentalIndicator tests
        cmocka_unit_test(id3v2SetExperimentalIndicator_set1),
        cmocka_unit_test(id3v2SetExperimentalIndicator_setWrongVersion),

        //id3v2WriteFooterIndicator tests
        cmocka_unit_test(id3v2SetFooterIndicator_set0),
        cmocka_unit_test(id3v2SetFooterIndicator_setWrongVersion),

        //id3v2ReadUnsynchronisationIndicator tests
        cmocka_unit_test(id3v2ReadUnsynchronisationIndicator_validStruct),
        cmocka_unit_test(id3v2ReadUnsynchronisationIndicator_NULLStruct),

        //id3v2ReadCompressionIndicator tests
        cmocka_unit_test(id3v2ReadCompressionIndicator_validStruct),
        cmocka_unit_test(id3v2ReadCompressionIndicator_NULLStruct),
        cmocka_unit_test(id3v2ReadCompressionIndicator_WrongVersion),

        //id3v2ReadExtendedHeaderIndicator tests
        cmocka_unit_test(id3v2ReadExtendedHeaderIndicator_validStruct),
        cmocka_unit_test(id3v2ReadExtendedHeaderIndicator_NULLStruct),
        cmocka_unit_test(id3v2ReadExtendedHeaderIndicator_WrongVersion),

        //id3v2ReadExtendedHeaderIndicator tests
        cmocka_unit_test(id3v2ReadExtendedHeaderIndicator_validStruct),
        cmocka_unit_test(id3v2ReadExtendedHeaderIndicator_NULLStruct),
        cmocka_unit_test(id3v2ReadExtendedHeaderIndicator_WrongVersion),

        //id3v2ReadExperimentalIndicator tests
        cmocka_unit_test(id3v2ReadExperimentalIndicator_validStruct),
        cmocka_unit_test(id3v2ReadExperimentalIndicator_NULLStruct),
        cmocka_unit_test(id3v2ReadExperimentalIndicator_WrongVersion),

        //id3v2ReadFooterIndicator tests
        cmocka_unit_test(id3v2ReadFooterIndicator_validStruct),
        cmocka_unit_test(id3v2ReadFooterIndicator_NULLStruct),
        cmocka_unit_test(id3v2ReadFooterIndicator_WrongVersion),

        //id3v2CreateExtendedTagHeader tests
        cmocka_unit_test(id3v2NewExtendedTagHeader_validStruct),

        //d3v2DestroyExtendedTagHeader tests
        cmocka_unit_test(id3v2DestroyExtendedTagHeader_DestroyStruct),
        cmocka_unit_test(id3v2DestroyExtendedTagHeader_tryFreeNULL),

        //id3v2WriteTagSizeRestriction tests
        cmocka_unit_test(id3v2WriteTagSizeRestriction_noExtAlready),
        cmocka_unit_test(id3v2WriteTagSizeRestriction_changeCurrentValue),
        cmocka_unit_test(id3v2WriteTagSizeRestriction_wrongVersion),
        cmocka_unit_test(id3v2WriteTagSizeRestriction_notAOption),

        //id3v2WriteTextEncodingRestriction tests
        cmocka_unit_test(id3v2WriteTextEncodingRestriction_noExtAlready),
        cmocka_unit_test(id3v2WriteTextEncodingRestriction_changeCurrentValue),
        cmocka_unit_test(id3v2WriteTextEncodingRestriction_wrongVersion),
        cmocka_unit_test(id3v2WriteTextEncodingRestriction_notAOption),

        //id3v2WriteTextFieldsSizeRestriction tests
        cmocka_unit_test(id3v2WriteTextFieldsSizeRestriction_noExtAlready),
        cmocka_unit_test(id3v2WriteTextFieldsSizeRestriction_changeCurrentValue),
        cmocka_unit_test(id3v2WriteTextFieldsSizeRestriction_wrongVersion),
        cmocka_unit_test(id3v2WriteTextFieldsSizeRestriction_notAOption),

        //id3v2WriteImageEncodingRestriction tests
        cmocka_unit_test(id3v2WriteImageEncodingRestriction_noExtAlready),
        cmocka_unit_test(id3v2WriteImageEncodingRestriction_changeCurrentValue),
        cmocka_unit_test(id3v2WriteImageEncodingRestriction_wrongVersion),

        //id3v2WriteImageSizeRestriction tests
        cmocka_unit_test(id3v2WriteImageSizeRestriction_noExtAlready),
        cmocka_unit_test(id3v2WriteImageSizeRestriction_changeCurrentValue),
        cmocka_unit_test(id3v2WriteImageSizeRestriction_wrongVersion),
        cmocka_unit_test(id3v2WriteImageSizeRestriction_notAOption),

        //id3v2ReadTagSizeRestriction tests
        cmocka_unit_test(id3v2ReadTagSizeRestriction_readBits),
        cmocka_unit_test(id3v2ReadTagSizeRestriction_readBits2),

        //id3v2ReadTextEncodingRestriction tests
        cmocka_unit_test(id3v2ReadTextEncodingRestriction_readBit),
        cmocka_unit_test(id3v2ReadTextEncodingRestriction_readBit2),

        //id3v2ReadTextFieldsSizeRestriction tests
        cmocka_unit_test(id3v2ReadTextFieldsSizeRestriction_readBit),
        cmocka_unit_test(id3v2ReadTextFieldsSizeRestriction_readBit2),

        //id3v2ReadImageEncodingRestriction tests
        cmocka_unit_test(id3v2ReadImageEncodingRestriction_readBit),
        cmocka_unit_test(id3v2ReadImageEncodingRestriction_readBit2),

        //id3v2ReadImageSizeRestriction test
        cmocka_unit_test(id3v2ReadImageSizeRestriction_readBit),
        cmocka_unit_test(id3v2ReadImageSizeRestriction_readBit2),

        //id3v2ClearTagRestrictions test
        cmocka_unit_test(id3v2ClearTagRestrictions_clear),

        // tag create/destroy
        cmocka_unit_test(id3v2TagCreateAndDestroy_AllInOne),

        // id3v2ExtendedTagHeader tests
        cmocka_unit_test(id3v2ExtendedTagHeaderSerialize_v3noCRC),
        cmocka_unit_test(id3v2ExtendedTagHeaderSerialize_v3CRC),
        cmocka_unit_test(id3v2ExtendedTagHeaderSerialize_null),
        cmocka_unit_test(id3v2ExtendedTagHeaderSerialize_v4WithEverything),
        cmocka_unit_test(id3v2ExtendedTagHeaderSerialize_v4),
        cmocka_unit_test(id3v2ExtendedTagHeaderSerialize_v4crc),
        cmocka_unit_test(id3v2ExtendedTagHeaderSerialize_v4restrictions),

        // id3v2ExtendedTagHeaderToJSON
        cmocka_unit_test(id3v2ExtendedTagHeaderToJSON_v3CRC),
        cmocka_unit_test(id3v2ExtendedTagHeaderToJSON_v3noCRC),
        cmocka_unit_test(id3v2ExtendedTagHeaderToJSON_null),
        cmocka_unit_test(id3v2ExtendedTagHeaderToJSON_v4WithEverything),
        cmocka_unit_test(id3v2ExtendedTagHeaderToJSON_v4crc),
        cmocka_unit_test(id3v2ExtendedTagHeaderToJSON_v4restrictions),

        // id3v2TagHeaderSerialize
        cmocka_unit_test(id3v2TagHeaderSerialize_v2),
        cmocka_unit_test(id3v2TagHeaderSerialize_unsupportedVersion),
        cmocka_unit_test(id3v2TagHeaderSerialize_null),
        cmocka_unit_test(id3v2TagHeaderSerialize_v3),
        cmocka_unit_test(id3v2TagHeaderSerialize_v3WithExt),
        cmocka_unit_test(id3v2TagHeaderSerialize_v4),
        cmocka_unit_test(id3v2TagHeaderSerialize_v4WithExt),

        // id3v2TagHeaderToJSON
        cmocka_unit_test(id3v2TagHeaderToJSON_null),
        cmocka_unit_test(id3v2TagHeaderToJSON_v2),
        cmocka_unit_test(id3v2TagHeaderToJSON_v3),
        cmocka_unit_test(id3v2TagHeaderToJSON_v3ext),
        cmocka_unit_test(id3v2TagHeaderToJSON_v4),
        cmocka_unit_test(id3v2TagHeaderToJSON_v4ext),
        cmocka_unit_test(id3v2ValidateTagHeader_valid),
        cmocka_unit_test(id3v2ValidateTagHeader_badVersion),
        cmocka_unit_test(id3v2ValidateTagHeader_undefinedFlags),
        cmocka_unit_test(id3v2ValidateTagHeader_extendedHeader),
        cmocka_unit_test(id3v2ValidateExtendedTagHeader_v3),
        cmocka_unit_test(id3v2ValidateExtendedTagHeader_v4)


    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
