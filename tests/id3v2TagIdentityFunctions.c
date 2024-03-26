#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "id3v2TagIdentity.h"
#include "id3v2Frame.h"
#include "byteStream.h"
#include "byteInt.h"

static void id3v2NewTagHeader_validStruct(void **state){
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(4,0,0,NULL);

    assert_int_equal(header->majorVersion,4);
    assert_int_equal(header->minorVersion,0);
    assert_int_equal(header->flags,0);
    assert_null(header->extendedHeader);

    free(header);
}

static void id3v2DestroyTagHeader_freeStruct(void **state){
    (void) state;

    Id3v2TagHeader *header = id3v2CreateTagHeader(4,0,0,NULL);
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
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(4,0,128,NULL);

    int v = id3v2WriteUnsynchronisationIndicator(header, 0);
    
    assert_true(v);
    assert_int_equal(header->flags, 0);
    id3v2DestroyTagHeader(&header);
}

static void id3v2SetUnsynchronisationIndicator_set1While1(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(4,0,128,NULL);

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
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(2,0,0,NULL);


    int v = id3v2WriteCompressionIndicator(header, 1);
    
    assert_true(v);
    assert_int_equal(64, header->flags);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetCompressionIndicator_setNoneVersion2(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(3,0,0,NULL);


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
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(3,0,64,NULL);

    int v = id3v2WriteExtendedHeaderIndicator(header, 0);
    
    assert_true(v);
    assert_int_equal(header->flags, 0);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetExtendedHeaderIndicator_setWhileOtherFlagIsSet(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(3,0,128,NULL);

    int v = id3v2WriteExtendedHeaderIndicator(header, 1);
    
    assert_true(v);
    assert_int_equal(header->flags, 192);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetExtendedHeaderIndicator_setWrongVersion(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(2,0,128,NULL);

    int v = id3v2WriteExtendedHeaderIndicator(header, 1);
    
    assert_false(v);
    assert_int_equal(header->flags, 128);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetExperimentalIndicator_set1(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(4,0,0,NULL);

    int v = id3v2WriteExperimentalIndicator(header, 1);
    
    assert_true(v);
    assert_int_equal(header->flags, 32);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetExperimentalIndicator_setsetWrongVersion(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(2,0,0,NULL);

    int v = id3v2WriteExperimentalIndicator(header, 1);
    
    assert_false(v);
    assert_int_equal(header->flags, 0);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetFooterIndicator_set0(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(4,0,0,NULL);

    int v = id3v2WriteExperimentalIndicator(header, 1);
    
    assert_true(v);
    assert_int_equal(header->flags, 32);

    id3v2DestroyTagHeader(&header);
}

static void id3v2SetFooterIndicator_setWrongVersion(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(2,0,16,NULL);

    int v = id3v2WriteFooterIndicator(header,1);
    
    assert_false(v);
    assert_int_equal(header->flags, 16);

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadUnsynchronisationIndicator_validStruct(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(4,0,128,NULL);

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
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(2,0,64,NULL);

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
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(4,0,64,NULL);;

    assert_int_equal(id3v2ReadCompressionIndicator(header), -1);
    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadExtendedHeaderIndicator_validStruct(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(3,0,64,NULL);

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
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(2,0,0,NULL);

    assert_int_equal(id3v2ReadExtendedHeaderIndicator(header), -1);
    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadExperimentalIndicator_validStruct(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(3,0,32,NULL);

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
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(2,0,0,NULL);;

    assert_int_equal(id3v2ReadExperimentalIndicator(header), -1);
    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadFooterIndicator_validStruct(void **state){
    (void) state;
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(4,0,16,NULL);

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
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(2,0,0,NULL);

    assert_int_equal(id3v2ReadFooterIndicator(header), -1);
    id3v2DestroyTagHeader(&header);
}

static void id3v2NewExtendedTagHeader_validStruct(void **state){
    
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(150,909,1,1,255);

    assert_int_equal(ext->padding,150);
    assert_int_equal(ext->crc,909);
    assert_int_equal(ext->update,1);
    assert_int_equal(ext->tagRestrictions,1);
    assert_int_equal(ext->restrictions,255);
    
    free(ext);
}

static void id3v2DestroyExtendedTagHeader_DestroyStruct(void **state){
    
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(150,909,1,1,255);
    id3v2DestroyExtendedTagHeader(&ext);
    assert_null(ext);
}

static void id3v2DestroyExtendedTagHeader_tryFreeNULL(void **state){
    //fails on segfault
    Id3v2ExtendedTagHeader *ext = NULL;
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2WriteTagSizeRestriction_noExtAlready(void **state){
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, NULL);

    assert_true(id3v2WriteTagSizeRestriction(header, 3));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 192);

    id3v2DestroyTagHeader(&header);

}

static void id3v2WriteTagSizeRestriction_changeCurrentValue(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0,0,1,192);
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_true(id3v2WriteTagSizeRestriction(header, 1));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 64);

    id3v2DestroyTagHeader(&header);

}

static void id3v2WriteTagSizeRestriction_wrongVersion(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0,0,1,192);
    Id3v2TagHeader *header = id3v2CreateTagHeader(2, 0, 64, ext);

    assert_false(id3v2WriteTagSizeRestriction(header, 1));

    id3v2DestroyTagHeader(&header);

}

static void id3v2WriteTagSizeRestriction_notAOption(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0,0,1,192);
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_false(id3v2WriteTagSizeRestriction(header, 9));
    assert_int_equal(header->extendedHeader->restrictions, 192);
    id3v2DestroyTagHeader(&header);

}

static void id3v2WriteTextEncodingRestriction_noExtAlready(void **state){
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, NULL);

    assert_true(id3v2WriteTextEncodingRestriction(header, 1));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 32);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTextEncodingRestriction_changeCurrentValue(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0,0,1,32);
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_true(id3v2WriteTextEncodingRestriction(header, 0));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 0);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTextEncodingRestriction_wrongVersion(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0,0,1,32);
    Id3v2TagHeader *header = id3v2CreateTagHeader(2, 0, 64, ext);

    assert_false(id3v2WriteTagSizeRestriction(header, 0));

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTextEncodingRestriction_notAOption(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0,0,1,32);
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_false(id3v2WriteTagSizeRestriction(header, 100));
    assert_int_equal(header->extendedHeader->restrictions, 32);
    id3v2DestroyTagHeader(&header);
}


static void id3v2WriteTextFieldsSizeRestriction_noExtAlready(void **state){
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, NULL);

    assert_true(id3v2WriteTextFieldsSizeRestriction(header, 2));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 16);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTextFieldsSizeRestriction_changeCurrentValue(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0,0,1,16);
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_true(id3v2WriteTextFieldsSizeRestriction(header, 3));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 24);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTextFieldsSizeRestriction_wrongVersion(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0,0,1,16);
    Id3v2TagHeader *header = id3v2CreateTagHeader(2, 0, 64, ext);

    assert_false(id3v2WriteTextFieldsSizeRestriction(header, 0));

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteTextFieldsSizeRestriction_notAOption(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0,0,1,24);
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_false(id3v2WriteTagSizeRestriction(header, 100));
    assert_int_equal(header->extendedHeader->restrictions, 24);
    id3v2DestroyTagHeader(&header);
}


static void id3v2WriteImageEncodingRestriction_noExtAlready(void **state){
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, NULL);

    assert_true(id3v2WriteImageEncodingRestriction(header, 1));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 4);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteImageEncodingRestriction_changeCurrentValue(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0,0,1,4);
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_true(id3v2WriteImageEncodingRestriction(header, 0));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 0);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteImageEncodingRestriction_wrongVersion(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0,0,1,4);
    Id3v2TagHeader *header = id3v2CreateTagHeader(2, 0, 64, ext);
    bool v = id3v2WriteImageEncodingRestriction(header, 0);
    assert_false(v);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteImageSizeRestriction_noExtAlready(void **state){
    
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, NULL);

    assert_true(id3v2WriteImageSizeRestriction(header, 2));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 2);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteImageSizeRestriction_changeCurrentValue(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0,0,1,2);
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_true(id3v2WriteImageSizeRestriction(header, 3));

    assert_non_null(header->extendedHeader);
    assert_true(header->extendedHeader->tagRestrictions);
    assert_int_equal(header->extendedHeader->restrictions, 3);

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteImageSizeRestriction_wrongVersion(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0,0,1,3);
    Id3v2TagHeader *header = id3v2CreateTagHeader(2, 0, 64, ext);

    assert_false(id3v2WriteImageSizeRestriction(header, 0));

    id3v2DestroyTagHeader(&header);
}

static void id3v2WriteImageSizeRestriction_notAOption(void **state){
    

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0,0,1,3);
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_false(id3v2WriteImageSizeRestriction(header, 100));
    assert_int_equal(header->extendedHeader->restrictions, 3);
    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadTagSizeRestriction_readBits(void **state){
    
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 192);
    //11000000

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_int_equal(3, id3v2ReadTagSizeRestriction(header));

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadTagSizeRestriction_readBits2(void **state){
    
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 128);
    //11000000

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_int_equal(2, id3v2ReadTagSizeRestriction(header));

    id3v2DestroyTagHeader(&header);
}

static void id3v2ReadTextEncodingRestriction_readBit(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 32);
    //00100000

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_int_equal(1, id3v2ReadTextEncodingRestriction(header));

    id3v2DestroyTagHeader(&header); 
}

static void id3v2ReadTextEncodingRestriction_readBit2(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 0);
    //00000000

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_int_equal(0, id3v2ReadTextEncodingRestriction(header));

    id3v2DestroyTagHeader(&header); 
}

static void id3v2ReadTextFieldsSizeRestriction_readBit(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 24);
    //00011000

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_int_equal(3, id3v2ReadTextFieldsSizeRestriction(header));

    id3v2DestroyTagHeader(&header); 
}

static void id3v2ReadTextFieldsSizeRestriction_readBit2(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 8);
    //00001000

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_int_equal(1, id3v2ReadTextFieldsSizeRestriction(header));

    id3v2DestroyTagHeader(&header); 
}

static void id3v2ReadImageEncodingRestriction_readBit(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 4);
    //00000100

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_int_equal(1, id3v2ReadImageEncodingRestriction(header));

    id3v2DestroyTagHeader(&header); 
}

static void id3v2ReadImageEncodingRestriction_readBit2(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 0);
    //00000000

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_int_equal(0, id3v2ReadImageEncodingRestriction(header));

    id3v2DestroyTagHeader(&header); 
}

static void id3v2ReadImageSizeRestriction_readBit(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 3);
    //00000011

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_int_equal(3, id3v2ReadImageSizeRestriction(header));

    id3v2DestroyTagHeader(&header); 
}

static void id3v2ReadImageSizeRestriction_readBit2(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 1);
    //00000001

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);

    assert_int_equal(1, id3v2ReadImageSizeRestriction(header));

    id3v2DestroyTagHeader(&header); 
}

static void id3v2ClearTagRestrictions_clear(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, false, true, 1);
    //00000001

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 64, ext);
    assert_true(id3v2ClearTagRestrictions(header));
    assert_false(header->extendedHeader->tagRestrictions);
    id3v2DestroyTagHeader(&header); 
}


static void id3v2TagCreateAndDestroy_AllInOne(void **state){

    Id3v2Tag *tag = id3v2CreateTag(id3v2CreateTagHeader(1,2,10,id3v2CreateExtendedTagHeader(15,50,1,1,10)), listCreate(id3v2PrintFrame, id3v2DeleteFrame, id3v2CompareFrame, id3v2CopyFrame));

    assert_non_null(tag);
    assert_non_null(tag->frames);
    assert_non_null(tag->header);

    id3v2DestroyTag(&tag);

    assert_null(tag);

}

static void id3v2ExtendedTagHeaderToStream_v3noCRC(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(100, 0, 0, 0, 0);
    ByteStream *stream = id3v2ExtendedTagHeaderToStream(ext, ID3V2_TAG_VERSION_3);

    assert_int_equal(byteStreamReturnInt(stream), 10);

    assert_int_equal(byteStreamGetCh(stream), 0);
    byteStreamSeek(stream, 1, SEEK_CUR);
    assert_int_equal(byteStreamGetCh(stream), 0);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(byteStreamReturnInt(stream), 100);

    byteStreamDestroy(stream);
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2ExtendedTagHeaderToStream_v3CRC(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(UINT32_MAX, UINT32_MAX, 1,1,1);
    ByteStream *stream = id3v2ExtendedTagHeaderToStream(ext, ID3V2_TAG_VERSION_3);

    assert_int_equal(byteStreamReturnInt(stream), 14);

    assert_int_equal(byteStreamGetCh(stream), 0x80);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(byteStreamGetCh(stream), 0);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(byteStreamReturnU32(stream), UINT32_MAX);
    assert_int_equal(byteStreamReturnU32(stream), UINT32_MAX);
    
    byteStreamDestroy(stream);
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2ExtendedTagHeaderToStream_null(void **state){

    ByteStream *stream = id3v2ExtendedTagHeaderToStream(NULL, ID3V2_TAG_VERSION_3);

    assert_null(stream);

}

static void id3v2ExtendedTagHeaderToStream_v4WithEverything(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(UINT32_MAX, UINT32_MAX, 1,1,0xfe);
    ByteStream *stream = id3v2ExtendedTagHeaderToStream(ext, ID3V2_TAG_VERSION_4);
    
    assert_int_equal(byteStreamReturnInt(stream), 12);

    assert_int_equal(byteStreamCursor(stream)[0], 6);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(byteStreamCursor(stream)[0], 0x70);
    byteStreamSeek(stream, 1, SEEK_CUR);


    unsigned char tmp[5] = {0,0,0,0,0};
    byteStreamRead(stream, tmp, 5);

    assert_int_equal(btost(tmp, 5), byteSyncintEncode(UINT32_MAX));
    byteStreamSeek(stream, 5, SEEK_CUR);

    

    assert_int_equal(byteStreamCursor(stream)[0], 0xfe);
    byteStreamSeek(stream, 1, SEEK_CUR);

    byteStreamDestroy(stream);
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2ExtendedTagHeaderToStream_v4(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, 0, 0, 0);
    ByteStream *stream = id3v2ExtendedTagHeaderToStream(ext, ID3V2_TAG_VERSION_4);
    
    assert_int_equal(byteStreamReturnInt(stream), 6);

    assert_int_equal(byteStreamCursor(stream)[0], 0);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(byteStreamCursor(stream)[0], 0);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(stream->bufferSize, 6);

    byteStreamDestroy(stream);
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2ExtendedTagHeaderToStream_v4crc(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 9000, 0, 0, 0);
    ByteStream *stream = id3v2ExtendedTagHeaderToStream(ext, ID3V2_TAG_VERSION_4);

    assert_int_equal(byteStreamReturnInt(stream), 11);

    assert_int_equal(byteStreamCursor(stream)[0], 5);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(byteStreamCursor(stream)[0], 0x20);
    byteStreamSeek(stream, 1, SEEK_CUR);

    unsigned char tmp[5] = {0,0,0,0,0};
    byteStreamRead(stream, tmp, 5);

    assert_int_equal(btost(tmp, 5), byteSyncintEncode(9000));

    assert_int_equal(stream->bufferSize, 11);

    byteStreamDestroy(stream);
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2ExtendedTagHeaderToStream_v4restrictions(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, 0, 1, 0xff);
    ByteStream *stream = id3v2ExtendedTagHeaderToStream(ext, ID3V2_TAG_VERSION_4);

    assert_int_equal(byteStreamReturnInt(stream), 7);

    assert_int_equal(byteStreamCursor(stream)[0], 1);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(byteStreamCursor(stream)[0], 0x10);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(byteStreamCursor(stream)[0], 0xff);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(stream->bufferSize, 7);

    byteStreamDestroy(stream);
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2ExtendedTagHeaderToJSON_v3CRC(void **state){
    
    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(UINT32_MAX, UINT32_MAX, 1,1,0xfe);
    char *json = id3v2ExtendedTagHeaderToJSON(ext, ID3V2_TAG_VERSION_3);

    assert_non_null(json);
    assert_string_equal(json,
    "{\"padding\":4294967295,\"crc\":4294967295}");

    free(json);
    id3v2DestroyExtendedTagHeader(&ext);
}


static void id3v2ExtendedTagHeaderToJSON_v3noCRC(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(100, 0, 0, 0, 0);
    char *json = id3v2ExtendedTagHeaderToJSON(ext, ID3V2_TAG_VERSION_3);

    assert_non_null(json);
    assert_string_equal(json,
    "{\"padding\":100,\"crc\":0}");

    free(json);
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2ExtendedTagHeaderToJSON_null(void **state){

    char *json = id3v2ExtendedTagHeaderToJSON(NULL, ID3V2_TAG_VERSION_2);

    assert_non_null(json);
    assert_string_equal(json, "{}");

    free(json);

}

static void id3v2ExtendedTagHeaderToJSON_v4WithEverything(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(UINT32_MAX, UINT32_MAX, 1,1,0xfe);
    char *json = id3v2ExtendedTagHeaderToJSON(ext, ID3V2_TAG_VERSION_4);
    
    assert_non_null(json);
    assert_string_equal(json,
    "{\"padding\":4294967295,\"crc\":4294967295,\"update\":true,\"tagRestrictions\":true,\"restrictions\":254}");
    
    free(json);
    id3v2DestroyExtendedTagHeader(&ext);
}


static void id3v2ExtendedTagHeaderToJSON_v4crc(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 9000, 0, 0, 0);
    char *json = id3v2ExtendedTagHeaderToJSON(ext, ID3V2_TAG_VERSION_4);

    assert_non_null(json);
    assert_string_equal(json,
                        "{\"padding\":0,\"crc\":9000,\"update\":false,\"tagRestrictions\":false,\"restrictions\":0}");

    free(json);
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2ExtendedTagHeaderToJSON_v4restrictions(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(0, 0, 0, 1, 0xff);
    char *json = id3v2ExtendedTagHeaderToJSON(ext, ID3V2_TAG_VERSION_4);

    assert_non_null(json);
    assert_string_equal(json,
    "{\"padding\":0,\"crc\":0,\"update\":false,\"tagRestrictions\":true,\"restrictions\":255}");
    free(json);
    id3v2DestroyExtendedTagHeader(&ext);
}

static void id3v2TagHeaderToStream_v2(void **state){

    Id3v2TagHeader *h = id3v2CreateTagHeader(2, 0, 0, NULL);
    ByteStream *stream = id3v2TagHeaderToStream(h, 1000);
    unsigned char *tmp = NULL;


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
    byteStreamDestroy(stream);

}

static void id3v2TagHeaderToStream_unsupportedVersion(void **state){

    Id3v2TagHeader *h = id3v2CreateTagHeader(10, 0, 0, NULL);
    ByteStream *stream = id3v2TagHeaderToStream(h, 1000);

    assert_null(stream);

    id3v2DestroyTagHeader(&h);

}

static void id3v2TagHeaderToStream_null(void **state){

    ByteStream *stream = id3v2TagHeaderToStream(NULL, 1000);
    assert_null(stream);

}


static void id3v2TagHeaderToStream_v3(void **state){

    Id3v2TagHeader *h = id3v2CreateTagHeader(3, 1, 0x20, NULL);
    ByteStream *stream = id3v2TagHeaderToStream(h, 1000);
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
    byteStreamDestroy(stream);

}

static void id3v2TagHeaderToStream_v3WithExt(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(100, 0, 0, 0, 0);
    Id3v2TagHeader *h = id3v2CreateTagHeader(3, 1, 0x60, ext);
    ByteStream *stream = id3v2TagHeaderToStream(h, 1000);
    unsigned char *tmp = NULL;


    assert_non_null(stream);
    
    assert_memory_equal(byteStreamCursor(stream), "ID3", 3);
    byteStreamSeek(stream, 3, SEEK_CUR);
    
    assert_int_equal(byteStreamCursor(stream)[0], 3);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(byteStreamCursor(stream)[0], 1);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(byteStreamCursor(stream)[0], 0x60);
    byteStreamSeek(stream, 1, SEEK_CUR);

    tmp = u32tob(byteSyncintEncode(1000));
    assert_memory_equal(byteStreamCursor(stream), tmp, 4);
    free(tmp);
    byteStreamSeek(stream, 4, SEEK_CUR);

    assert_int_equal(byteStreamReturnU32(stream), 10);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_memory_equal(byteStreamCursor(stream), "\x00\x00", 2);
    byteStreamSeek(stream, 2, SEEK_CUR);

    assert_int_equal(byteStreamReturnU32(stream), 100);

    id3v2DestroyTagHeader(&h);
    byteStreamDestroy(stream);

}


static void id3v2TagHeaderToStream_v4(void **state){

    Id3v2TagHeader *h = id3v2CreateTagHeader(4, 0, 0, NULL);
    ByteStream *stream = id3v2TagHeaderToStream(h, 900);
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
    byteStreamDestroy(stream);

}

static void id3v2TagHeaderToStream_v4WithExt(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(80, 870, 1, 1, 0xff);
    Id3v2TagHeader *h = id3v2CreateTagHeader(4, 0, 0xF0, ext);
    ByteStream *stream = id3v2TagHeaderToStream(h, 900);
    unsigned char *tmp = NULL;

    assert_non_null(stream);

    assert_memory_equal(byteStreamCursor(stream), "ID3", 3);
    byteStreamSeek(stream, 3, SEEK_CUR);
    
    assert_int_equal(byteStreamCursor(stream)[0], 4);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(byteStreamCursor(stream)[0], 0);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(byteStreamCursor(stream)[0], 0xF0);
    byteStreamSeek(stream, 1, SEEK_CUR);

    tmp = u32tob(byteSyncintEncode(900));
    assert_memory_equal(byteStreamCursor(stream), tmp, 4);
    free(tmp);
    byteStreamSeek(stream, 4, SEEK_CUR);

    assert_int_equal(byteStreamReturnInt(stream), 12);
    
    assert_int_equal(byteStreamCursor(stream)[0], 6);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(byteStreamCursor(stream)[0], 0x70);
    byteStreamSeek(stream, 1, SEEK_CUR);

    tmp = malloc(5);
    byteStreamRead(stream, tmp, 5);
    assert_int_equal(btost(tmp, 5), byteSyncintEncode(870));
    free(tmp);

    assert_int_equal(byteStreamCursor(stream)[0], 0xff);


    id3v2DestroyTagHeader(&h);
    byteStreamDestroy(stream);

}

static void id3v2TagHeaderToJSON_null(void **state){

    char *json = id3v2TagHeaderToJSON(NULL);

    assert_string_equal(json, "{}");

    free(json);
}

static void id3v2TagHeaderToJSON_v2(void **state){

    Id3v2TagHeader *header = id3v2CreateTagHeader(2, 99, 0, NULL);

    char *json = id3v2TagHeaderToJSON(header);

    assert_string_equal(json, "{\"major\":2,\"minor\":99,\"flags\":0}");

    id3v2DestroyTagHeader(&header);
    free(json);
}

static void id3v2TagHeaderToJSON_v3(void **state){

    Id3v2TagHeader *header = id3v2CreateTagHeader(3, 0, 0, NULL);

    char *json = id3v2TagHeaderToJSON(header);

    assert_string_equal(json, "{\"major\":3,\"minor\":0,\"flags\":0,\"extended\":{}}");

    id3v2DestroyTagHeader(&header);
    free(json);
}

static void id3v2TagHeaderToJSON_v3ext(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(255, 0, 0, 0, 0);
    Id3v2TagHeader *header = id3v2CreateTagHeader(3, 0, 0, ext);

    char *json = id3v2TagHeaderToJSON(header);

    assert_string_equal(json, "{\"major\":3,\"minor\":0,\"flags\":0,\"extended\":{\"padding\":255,\"crc\":0}}");

    id3v2DestroyTagHeader(&header);
    free(json);
}


static void id3v2TagHeaderToJSON_v4(void **state){

    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 0x40, NULL);

    char *json = id3v2TagHeaderToJSON(header);

    assert_string_equal(json, "{\"major\":4,\"minor\":0,\"flags\":64,\"extended\":{}}");

    id3v2DestroyTagHeader(&header);
    free(json);
}

static void id3v2TagHeaderToJSON_v4ext(void **state){

    Id3v2ExtendedTagHeader *ext = id3v2CreateExtendedTagHeader(100, 3232, 1, 1, 0x1F);
    Id3v2TagHeader *header = id3v2CreateTagHeader(4, 0, 0xF0, ext);

    char *json = id3v2TagHeaderToJSON(header);

    assert_string_equal(json, "{\"major\":4,\"minor\":0,\"flags\":240,\"extended\":{\"padding\":100,\"crc\":3232,\"update\":true,\"tagRestrictions\":true,\"restrictions\":31}}");

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

int main(){

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
        cmocka_unit_test(id3v2ExtendedTagHeaderToStream_v3noCRC),
        cmocka_unit_test(id3v2ExtendedTagHeaderToStream_v3CRC),
        cmocka_unit_test(id3v2ExtendedTagHeaderToStream_null),
        cmocka_unit_test(id3v2ExtendedTagHeaderToStream_v4WithEverything),
        cmocka_unit_test(id3v2ExtendedTagHeaderToStream_v4),
        cmocka_unit_test(id3v2ExtendedTagHeaderToStream_v4crc),
        cmocka_unit_test(id3v2ExtendedTagHeaderToStream_v4restrictions),

        // id3v2ExtendedTagHeaderToJSON
        cmocka_unit_test(id3v2ExtendedTagHeaderToJSON_v3CRC),
        cmocka_unit_test(id3v2ExtendedTagHeaderToJSON_v3noCRC),
        cmocka_unit_test(id3v2ExtendedTagHeaderToJSON_null),
        cmocka_unit_test(id3v2ExtendedTagHeaderToJSON_v4WithEverything),
        cmocka_unit_test(id3v2ExtendedTagHeaderToJSON_v4crc),
        cmocka_unit_test(id3v2ExtendedTagHeaderToJSON_v4restrictions),

        // id3v2TagHeaderToStream
        cmocka_unit_test(id3v2TagHeaderToStream_v2),
        cmocka_unit_test(id3v2TagHeaderToStream_unsupportedVersion),
        cmocka_unit_test(id3v2TagHeaderToStream_null),
        cmocka_unit_test(id3v2TagHeaderToStream_v3),
        cmocka_unit_test(id3v2TagHeaderToStream_v3WithExt),
        cmocka_unit_test(id3v2TagHeaderToStream_v4),
        cmocka_unit_test(id3v2TagHeaderToStream_v4WithExt),

        // id3v2TagHeaderToJSON
        cmocka_unit_test(id3v2TagHeaderToJSON_null),
        cmocka_unit_test(id3v2TagHeaderToJSON_v2),
        cmocka_unit_test(id3v2TagHeaderToJSON_v3),
        cmocka_unit_test(id3v2TagHeaderToJSON_v3ext),
        cmocka_unit_test(id3v2TagHeaderToJSON_v4),
        cmocka_unit_test(id3v2TagHeaderToJSON_v4ext)
        

    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}