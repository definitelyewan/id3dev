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


static void id3v2CreateAndDestroyFrameHeader_allInOne(void **state){
    
    Id3v2FrameHeader *h = id3v2CreateFrameHeader((uint8_t *)"TT2", false, false, false, false, 0, 0xff, 0xff);

    assert_non_null(h);
    assert_string_equal((char *)h->id, "TT2");
    assert_false(h->tagAlterPreservation);
    assert_false(h->fileAlterPreservation);
    assert_false(h->readOnly);
    assert_int_equal(h->decompressionSize, 0);
    assert_int_equal(h->encryptionSymbol, 0xff);
    assert_int_equal(h->groupSymbol, 0xff);

    id3v2DestroyFrameHeader(&h);

    assert_null(h);

}

static void id3v2CreateAndDestroyContentEntry_allInOne(void **state){

    Id3v2ContentEntry *ce = id3v2CreateContentEntry((void *)"test", 5);

    assert_non_null(ce);
    assert_string_equal("test", (void *)ce->entry);
    assert_int_equal(ce->size, 5);

    id3v2DeleteContentEntry((void *)ce);

}

static void id3v2CreateAndDestroyHeader_allInOne(void **state){


    List *l = listCreate(id3v2PrintContentEntry, id3v2DeleteContentEntry, id3v2CompareContentEntry, id3v2CopyContentEntry);
    listInsertBack(l, (void *)id3v2CreateContentEntry((void *)"test", 5));
    listInsertBack(l, (void *)id3v2CreateContentEntry((void *)"test2", 6));
    listInsertBack(l, (void *)id3v2CreateContentEntry((void *)"test3", 6));

    Id3v2FrameHeader *h = id3v2CreateFrameHeader((uint8_t *)"TT2", false, false, false, false, 0, 0xff, 0xff);
    
    Id3v2Frame *f = id3v2CreateFrame(h, id3v2CreateTextFrameContext(),l);

    assert_non_null(f);
    assert_non_null(f->header);
    assert_non_null(f->contexts);
    assert_non_null(f->entries);

    id3v2DestroyFrame(&f);

    assert_null(f);
}

static void id3v2Traverse_allInOne(void **state){

    ByteStream *stream = byteStreamFromFile("assets/boniver.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromStream(stream, NULL);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    int i = 0;

    while((f = id3v2FrameTraverse(&frames)) != NULL){
        i++;
    }
    
    assert_int_equal(i, 93);


    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);
}

static void id3v2ReadFrameEntry_allEntries(void **state){

    ByteStream *stream = byteStreamFromFile("assets/boniver.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromStream(stream, NULL);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    while((f = id3v2FrameTraverse(&frames)) != NULL){
        
        ListIter entries = id3v2CreateFrameEntryTraverser(f);
        void *tmp = NULL;
        size_t s = 0;
        int i = 0;
        while((tmp = id3v2ReadFrameEntry(&entries, &s)) != NULL){
            assert_non_null(tmp);
            
            if(i == 0){
                assert_memory_equal(tmp, "http://musicbrainz.org", 23);
                assert_int_equal(s, 23);
            }else if(i == 1){
                assert_memory_equal(tmp, "test", 4);
                assert_int_equal(s, 4);
            }

            free(tmp);
            i++;
        }

        break;

    }

    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);
}

static void id3v2ReadFrameEntry_allEntriesAsChar(void **state){

    ByteStream *stream = byteStreamFromFile("assets/sorry4dying.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromStream(stream, NULL);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    while((f = id3v2FrameTraverse(&frames)) != NULL){
        
        ListIter entries = id3v2CreateFrameEntryTraverser(f);
        char *tmp = NULL;
        size_t s = 0;
        int i = 0;
        while((tmp = id3v2ReadFrameEntryAsChar(&entries, &s)) != NULL){
            assert_non_null(tmp);
            
            if(i == 0){
                // assert_string_equal((char *)tmp, "http://musicbrainz.org");
                // assert_int_equal(s, 22);
            }else if(i == 1){
                // assert_string_equal((char *)tmp, "test");
                // assert_int_equal(s, 4);
            }
            //printf("%s\n", tmp);

            if(tmp){
                free(tmp);
            }
            
            i++;
        }
        //printf("NEW FRAME\n");
        //break;

    }

    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);
}


int main(){

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(id3v2CreateAndDestroyFrameHeader_allInOne),
        cmocka_unit_test(id3v2CreateAndDestroyContentEntry_allInOne),
        cmocka_unit_test(id3v2CreateAndDestroyHeader_allInOne),
        cmocka_unit_test(id3v2Traverse_allInOne),

        cmocka_unit_test(id3v2ReadFrameEntry_allEntries),
        cmocka_unit_test(id3v2ReadFrameEntry_allEntriesAsChar),

    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}