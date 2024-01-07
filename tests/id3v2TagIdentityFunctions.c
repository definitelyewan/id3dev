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

static void id3v2NewTagHeader_validStruct(void **state){
    (void) state;

    Id3v2TagHeader *header = id3v2NewTagHeader(4,0,0,NULL);

    assert_int_equal(header->majorVersion,4);
    assert_int_equal(header->minorVersion,0);
    assert_int_equal(header->flags,0);
    assert_null(header->extendedHeader);

    free(header);
}

static void id3v2DestroyTagHeader_freeStruct(void **state){
    (void) state;

    Id3v2TagHeader *header = id3v2NewTagHeader(4,0,0,NULL);
    id3v2DestroyTagHeader(&header);

    assert_null(header);
}

static void id3v2DestroyTagHeader_failToFreeStruct(void **state){
    (void) state;
    //fails if segfault
    Id3v2TagHeader *test = NULL;
    id3v2DestroyTagHeader(&test);
}

static void id3v2SetUnsynchronisationIndicator_set0(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2NewTagHeader(4,0,128,NULL);

    int v = id3v2WriteUnsynchronisationIndicator(header, 0);
    
    assert_true(v);
    assert_int_equal(header->flags, 0);
    id3v2DestroyTagHeader(&header);
}

static void id3v2SetUnsynchronisationIndicator_set1While1(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2NewTagHeader(4,0,128,NULL);

    int v = id3v2WriteUnsynchronisationIndicator(header, 1);
    
    assert_true(v);
    assert_int_equal(header->flags, 128);
    id3v2DestroyTagHeader(&header);
}

static void id3v2SetUnsynchronisationIndicator_NULLHeader(void **state){
    (void) state;
    
    Id3v2TagHeader *header = NULL;

    int v = id3v2WriteUnsynchronisationIndicator(header, 1);
    
    assert_false(v);
}

static void id3v2SetCompressionIndicator_set1(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2NewTagHeader(2,0,0,NULL);


    int v = id3v2WriteCompressionIndicator(header, 1);
    
    assert_true(v);
    assert_int_equal(64, header->flags);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetCompressionIndicator_setNoneVersion2(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2NewTagHeader(3,0,0,NULL);


    int v = id3v2WriteCompressionIndicator(header, 1);
    
    assert_false(v);
    assert_int_equal(0, header->flags);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetCompressionIndicator_setWithNullHeader(void **state){
    (void) state;
    
    Id3v2TagHeader *header = NULL;

    int v = id3v2WriteCompressionIndicator(header, 1);
    
    assert_false(v);
}

static void id3v2SetExtendedHeaderIndicator_set0(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2NewTagHeader(3,0,64,NULL);

    int v = id3v2WriteExtendedHeaderIndicator(header, 0);
    
    assert_true(v);
    assert_int_equal(header->flags, 0);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetExtendedHeaderIndicator_setWhileOtherFlagIsSet(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2NewTagHeader(3,0,128,NULL);

    int v = id3v2WriteExtendedHeaderIndicator(header, 1);
    
    assert_true(v);
    assert_int_equal(header->flags, 192);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetExtendedHeaderIndicator_setWrongVersion(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2NewTagHeader(2,0,128,NULL);

    int v = id3v2WriteExtendedHeaderIndicator(header, 1);
    
    assert_false(v);
    assert_int_equal(header->flags, 128);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetExperimentalIndicator_set1(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2NewTagHeader(4,0,0,NULL);

    int v = id3v2WriteExperimentalIndicator(header, 1);
    
    assert_true(v);
    assert_int_equal(header->flags, 32);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetExperimentalIndicator_setsetWrongVersion(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2NewTagHeader(2,0,0,NULL);

    int v = id3v2WriteExperimentalIndicator(header, 1);
    
    assert_false(v);
    assert_int_equal(header->flags, 0);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetFooterIndicator_set0(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2NewTagHeader(4,0,0,NULL);

    int v = id3v2WriteExperimentalIndicator(header, 1);
    
    assert_true(v);
    assert_int_equal(header->flags, 32);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetFooterIndicator_setWrongVersion(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2NewTagHeader(2,0,16,NULL);

    int v = id3v2WriteFooterIndicator(header,1);
    
    assert_false(v);
    assert_int_equal(header->flags, 16);

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadUnsynchronisationIndicator_validStruct(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2NewTagHeader(4,0,128,NULL);

    assert_int_equal(id3v2ReadUnsynchronisationIndicator(header), 1);
    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadUnsynchronisationIndicator_NULLStruct(void **state){
    (void) state;
    
    Id3v2TagHeader *header = NULL;

    assert_int_equal(id3v2ReadUnsynchronisationIndicator(header), -1);
}

static void id3v2ReadCompressionIndicator_validStruct(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2NewTagHeader(2,0,64,NULL);

    assert_int_equal(id3v2ReadCompressionIndicator(header), 1);
    header->flags = 0;
    assert_int_equal(id3v2ReadCompressionIndicator(header), 0);

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadCompressionIndicator_NULLStruct(void **state){
    (void) state;
    
    Id3v2TagHeader *header = NULL;

    assert_int_equal(id3v2ReadCompressionIndicator(header), -1);
}

static void id3v2ReadCompressionIndicator_WrongVersion(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2NewTagHeader(4,0,64,NULL);;

    assert_int_equal(id3v2ReadCompressionIndicator(header), -1);
    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadExtendedHeaderIndicator_validStruct(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2NewTagHeader(3,0,64,NULL);

    assert_int_equal(id3v2ReadExtendedHeaderIndicator(header), 1);
    header->flags = 0;
    assert_int_equal(id3v2ReadExtendedHeaderIndicator(header), 0);

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadExtendedHeaderIndicator_NULLStruct(void **state){
    (void) state;
    
    Id3v2TagHeader *header = NULL;

    assert_int_equal(id3v2ReadExtendedHeaderIndicator(header), -1);
}

static void id3v2ReadExtendedHeaderIndicator_WrongVersion(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2NewTagHeader(2,0,0,NULL);

    assert_int_equal(id3v2ReadExtendedHeaderIndicator(header), -1);
    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadExperimentalIndicator_validStruct(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2NewTagHeader(3,0,32,NULL);

    assert_int_equal(id3v2ReadExperimentalIndicator(header), 1);
    header->flags = 0;
    assert_int_equal(id3v2ReadExperimentalIndicator(header), 0);

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadExperimentalIndicator_NULLStruct(void **state){
    (void) state;
    
    Id3v2TagHeader *header = NULL;

    assert_int_equal(id3v2ReadExperimentalIndicator(header), -1);
}

static void id3v2ReadExperimentalIndicator_WrongVersion(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2NewTagHeader(2,0,0,NULL);;

    assert_int_equal(id3v2ReadExperimentalIndicator(header), -1);
    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadFooterIndicator_validStruct(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2NewTagHeader(4,0,16,NULL);

    assert_int_equal(id3v2ReadFooterIndicator(header), 1);
    header->flags = 0;
    assert_int_equal(id3v2ReadFooterIndicator(header), 0);

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadFooterIndicator_NULLStruct(void **state){
    (void) state;
    
    Id3v2TagHeader *header = NULL;

    assert_int_equal(id3v2ReadFooterIndicator(header), -1);
}

static void id3v2ReadFooterIndicator_WrongVersion(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2NewTagHeader(2,0,0,NULL);

    assert_int_equal(id3v2ReadFooterIndicator(header), -1);
    id3v2DestroyTagHeader(&header);
}

static void id3v2NewExtendedTagHeader_validStruct(void **state){
    
    Id3v2ExtendedTagHeader *ext = id3v2NewExtendedTagHeader(150,909,1,1,255);

    assert_int_equal(ext->padding,150);
    assert_int_equal(ext->crc,909);
    assert_int_equal(ext->update,1);
    assert_int_equal(ext->tagRestrictions,1);
    assert_int_equal(ext->restrictions,255);
    
    free(ext);
}

static void id3v2DestroyExtendedTagHeader_DestroyStruct(void **state){
    
    Id3v2ExtendedTagHeader *ext = id3v2NewExtendedTagHeader(150,909,1,1,255);
    id3v2DestroyExtendedTagHeader(&ext);
    assert_null(ext);
}

static void id3v2DestroyExtendedTagHeader_tryFreeNULL(void **state){
    //fails on segfault
    Id3v2ExtendedTagHeader *ext = NULL;
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2WriteTagSizeRestriction_noExtAlready(void **state){
    
    Id3v2TagHeader *header = id3v2NewTagHeader(4, 0, 64, NULL);

    assert_true(id3v2WriteTagSizeRestriction(header, 3));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 192);

    id3v2DestroyTagHeader(&header);

}

static void id3v2WriteTagSizeRestriction_changeCurrentValue(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2NewExtendedTagHeader(0, 0,0,1,192);
    Id3v2TagHeader *header = id3v2NewTagHeader(4, 0, 64, ext);

    assert_true(id3v2WriteTagSizeRestriction(header, 1));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 64);

    id3v2DestroyTagHeader(&header);

}

static void id3v2WriteTagSizeRestriction_wrongVersion(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2NewExtendedTagHeader(0, 0,0,1,192);
    Id3v2TagHeader *header = id3v2NewTagHeader(2, 0, 64, ext);

    assert_false(id3v2WriteTagSizeRestriction(header, 1));

    id3v2DestroyTagHeader(&header);

}

static void id3v2WriteTagSizeRestriction_notAOption(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2NewExtendedTagHeader(0, 0,0,1,192);
    Id3v2TagHeader *header = id3v2NewTagHeader(4, 0, 64, ext);

    assert_false(id3v2WriteTagSizeRestriction(header, 9));
    assert_int_equal(header->extendedHeader->restrictions, 192);
    id3v2DestroyTagHeader(&header);

}

static void id3v2WriteTextEncodingRestriction_noExtAlready(void **state){
    
    Id3v2TagHeader *header = id3v2NewTagHeader(4, 0, 64, NULL);

    assert_true(id3v2WriteTextEncodingRestriction(header, 1));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 32);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTextEncodingRestriction_changeCurrentValue(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2NewExtendedTagHeader(0, 0,0,1,32);
    Id3v2TagHeader *header = id3v2NewTagHeader(4, 0, 64, ext);

    assert_true(id3v2WriteTextEncodingRestriction(header, 0));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 0);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTextEncodingRestriction_wrongVersion(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2NewExtendedTagHeader(0, 0,0,1,32);
    Id3v2TagHeader *header = id3v2NewTagHeader(2, 0, 64, ext);

    assert_false(id3v2WriteTagSizeRestriction(header, 0));

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTextEncodingRestriction_notAOption(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2NewExtendedTagHeader(0, 0,0,1,32);
    Id3v2TagHeader *header = id3v2NewTagHeader(4, 0, 64, ext);

    assert_false(id3v2WriteTagSizeRestriction(header, 100));
    assert_int_equal(header->extendedHeader->restrictions, 32);
    id3v2DestroyTagHeader(&header);
}


static void id3v2WriteTextFieldsSizeRestriction_noExtAlready(void **state){
    
    Id3v2TagHeader *header = id3v2NewTagHeader(4, 0, 64, NULL);

    assert_true(id3v2WriteTextFieldsSizeRestriction(header, 2));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 16);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTextFieldsSizeRestriction_changeCurrentValue(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2NewExtendedTagHeader(0, 0,0,1,16);
    Id3v2TagHeader *header = id3v2NewTagHeader(4, 0, 64, ext);

    assert_true(id3v2WriteTextFieldsSizeRestriction(header, 3));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 24);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTextFieldsSizeRestriction_wrongVersion(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2NewExtendedTagHeader(0, 0,0,1,16);
    Id3v2TagHeader *header = id3v2NewTagHeader(2, 0, 64, ext);

    assert_false(id3v2WriteTextFieldsSizeRestriction(header, 0));

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTextFieldsSizeRestriction_notAOption(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2NewExtendedTagHeader(0, 0,0,1,24);
    Id3v2TagHeader *header = id3v2NewTagHeader(4, 0, 64, ext);

    assert_false(id3v2WriteTagSizeRestriction(header, 100));
    assert_int_equal(header->extendedHeader->restrictions, 24);
    id3v2DestroyTagHeader(&header);
}


static void id3v2WriteImageEncodingRestriction_noExtAlready(void **state){
    
    Id3v2TagHeader *header = id3v2NewTagHeader(4, 0, 64, NULL);

    assert_true(id3v2WriteImageEncodingRestriction(header, 1));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 4);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteImageEncodingRestriction_changeCurrentValue(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2NewExtendedTagHeader(0, 0,0,1,4);
    Id3v2TagHeader *header = id3v2NewTagHeader(4, 0, 64, ext);

    assert_true(id3v2WriteImageEncodingRestriction(header, 0));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 0);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteImageEncodingRestriction_wrongVersion(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2NewExtendedTagHeader(0, 0,0,1,4);
    Id3v2TagHeader *header = id3v2NewTagHeader(2, 0, 64, ext);
    bool v = id3v2WriteImageEncodingRestriction(header, 0);
    assert_false(v);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteImageSizeRestriction_noExtAlready(void **state){
    
    Id3v2TagHeader *header = id3v2NewTagHeader(4, 0, 64, NULL);

    assert_true(id3v2WriteImageSizeRestriction(header, 2));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 2);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteImageSizeRestriction_changeCurrentValue(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2NewExtendedTagHeader(0, 0,0,1,2);
    Id3v2TagHeader *header = id3v2NewTagHeader(4, 0, 64, ext);

    assert_true(id3v2WriteImageSizeRestriction(header, 3));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 3);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteImageSizeRestriction_wrongVersion(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2NewExtendedTagHeader(0, 0,0,1,3);
    Id3v2TagHeader *header = id3v2NewTagHeader(2, 0, 64, ext);

    assert_false(id3v2WriteImageSizeRestriction(header, 0));

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteImageSizeRestriction_notAOption(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2NewExtendedTagHeader(0, 0,0,1,3);
    Id3v2TagHeader *header = id3v2NewTagHeader(4, 0, 64, ext);

    assert_false(id3v2WriteImageSizeRestriction(header, 100));
    assert_int_equal(header->extendedHeader->restrictions, 3);
    id3v2DestroyTagHeader(&header);
}


// printf("%d%d%d%d%d%d%d%d\n",readBit(header->extendedHeader->restrictions, 7),
//                             readBit(header->extendedHeader->restrictions, 6),
//                             readBit(header->extendedHeader->restrictions, 5),
//                             readBit(header->extendedHeader->restrictions, 4),
//                             readBit(header->extendedHeader->restrictions, 3),
//                             readBit(header->extendedHeader->restrictions, 2),
//                             readBit(header->extendedHeader->restrictions, 1),
//                             readBit(header->extendedHeader->restrictions, 0));


int main(){

    const struct CMUnitTest tests[] = {
        
        //id3v2NewTagHeader tests
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
        cmocka_unit_test(id3v2SetExperimentalIndicator_setsetWrongVersion),

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

        //id3v2NewExtendedTagHeader tests
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

        //id3v2WriteImageEncodingRestriction
        cmocka_unit_test(id3v2WriteImageEncodingRestriction_noExtAlready),
        cmocka_unit_test(id3v2WriteImageEncodingRestriction_changeCurrentValue),
        cmocka_unit_test(id3v2WriteImageEncodingRestriction_wrongVersion),

        //id3v2WriteImageSizeRestriction
        cmocka_unit_test(id3v2WriteImageSizeRestriction_noExtAlready),
        cmocka_unit_test(id3v2WriteImageSizeRestriction_changeCurrentValue),
        cmocka_unit_test(id3v2WriteImageSizeRestriction_wrongVersion),
        cmocka_unit_test(id3v2WriteTextFieldsSizeRestriction_notAOption)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}