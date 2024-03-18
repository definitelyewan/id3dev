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
#include "byteInt.h"


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

static void id3v2ReadFrameEntry_TextFrameAsChar(void **state){

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
                assert_memory_equal(tmp, "\x01", 1);
                assert_int_equal(s, 1);
            }else if(i == 1){
                assert_string_equal(tmp, "sorry4dying");
                assert_int_equal(s, 11);
            }

            if(tmp){
                free(tmp);
            }
            
            i++;
        }
        break;

    }

    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);
}


static void id3v2ReadFrameEntry_TextFrameEncodings(void **state){

    ByteStream *stream = byteStreamFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromStream(stream, NULL);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;
    int i = 0;
    while((f = id3v2FrameTraverse(&frames)) != NULL){
        
        if(f->header->id[0] == 'T'){

            ListIter entries = id3v2CreateFrameEntryTraverser(f);
            uint8_t encoding = id3v2ReadFrameEntryAsU8(&entries);

            switch(i){
                
                case 0:
                    assert_int_equal(encoding, 3);
                    break;
                case 7:
                    assert_int_equal(encoding, 0);
                    break;
                case 8:
                    assert_int_equal(encoding, 0);
                    break;
                default:
                    assert_int_equal(encoding, 1);
            }

            i++;

        }

    }

    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);
}

static void id3v2ReadFrameEntry_checkETCO(void **state){

    ByteStream *stream = byteStreamFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromStream(stream, NULL);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    while((f = id3v2FrameTraverse(&frames)) != NULL){
        
        if(!memcmp(f->header->id, "ETCO", 4)){

            ListIter entries = id3v2CreateFrameEntryTraverser(f);
            
            assert_int_equal(2, id3v2ReadFrameEntryAsU8(&entries));
            assert_int_equal(6, id3v2ReadFrameEntryAsU8(&entries));
            assert_int_equal(1220000U, id3v2ReadFrameEntryAsU32(&entries));
            assert_int_equal(2, id3v2ReadFrameEntryAsU8(&entries));
            assert_int_equal(610000U, id3v2ReadFrameEntryAsU32(&entries));
            
        }

    }

    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);
}

static void id3v2WriteFrameEntry_greatestHits(void **state){

    ByteStream *stream = byteStreamFromFile("assets/boniver.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromStream(stream, NULL);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;
    bool exit = false;
    while((f = id3v2FrameTraverse(&frames)) != NULL){
        
        if(memcmp(f->header->id, "TXX", 3) == 0){

            ListIter entries = id3v2CreateFrameEntryTraverser(f);
            size_t s = 0;
            char *tmp = NULL;

            while((tmp = id3v2ReadFrameEntryAsChar(&entries, &s)) != NULL){

                if(memcmp("IS_GREATEST_HITS", tmp, s) == 0){

                    uint8_t zero = 0;
                    id3v2WriteFrameEntry(f, &entries, 1, (void *)&zero);

                    uint8_t ret = id3v2ReadFrameEntryAsU8(&entries);
                    assert_int_equal(ret, 0);
                    exit = true;
                    free(tmp);
                    break;

                }

                free(tmp);
            }
        }
        
        if(exit){
            break;
        }

    }

    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);
}


static void id3v2WriteFrameEntry_updateTitle(void **state){

    ByteStream *stream = byteStreamFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromStream(stream, NULL);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    while((f = id3v2FrameTraverse(&frames)) != NULL){
        
        if(memcmp(f->header->id, "TIT2", 4) == 0){

            ListIter entries = id3v2CreateFrameEntryTraverser(f);
            size_t s = 0;

            id3v2ReadFrameEntryAsU8(&entries);
            
            assert_true(id3v2WriteFrameEntry(f, &entries, 15, (void *) "A better title"));

            char *newTitle = id3v2ReadFrameEntryAsChar(&entries, &s);

            assert_int_equal(s, 14);
            assert_string_equal(newTitle, "A better title");

            free(newTitle);

        }

    }

    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);
}

static void id3v2AtatchFrameFromTag_TSOA(void **state){

    ByteStream *stream = byteStreamFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromStream(stream, NULL);
    
    
    List *entries = listCreate(id3v2PrintContentEntry, id3v2DeleteContentEntry, id3v2CompareContentEntry, id3v2CopyContentEntry);
    listInsertBack(entries, id3v2CreateContentEntry((void *)"\x0", 1));
    listInsertBack(entries, id3v2CreateContentEntry((void *)"SORT", 5));
    
    Id3v2FrameHeader *h = id3v2CreateFrameHeader((uint8_t *)"TSOA", false, false, false, false, 0, 0, 0);
    Id3v2Frame *f = id3v2CreateFrame(h, id3v2CreateTextFrameContext(), entries);


    assert_true(id3v2AttachFrameToTag(tag, f));

    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);

    // if the memory assigned to f is freed it was added successfully

}

static void id3v2DetatchFrameFromTag_TIT2(void **state){

    ByteStream *stream = byteStreamFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromStream(stream, NULL);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    while((f = id3v2FrameTraverse(&frames)) != NULL){
        
        if(memcmp(f->header->id, "TIT2", 4) == 0){

            Id3v2Frame *detach = id3v2DetatchFrameFromTag(tag, f);

            assert_non_null(detach);

            id3v2DestroyFrame(&detach);

            assert_null(detach);
        }

    }


    frames = id3v2CreateFrameTraverser(tag);
    bool exit = false;
    while((f = id3v2FrameTraverse(&frames)) != NULL){
        
        if(memcmp(f->header->id, "TIT2", 4) == 0){
            exit = true;
        }

    }


    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);

    assert_false(exit);
}

static void id3v2FrameHeaderToStream_null(void **state){

    ByteStream *stream = id3v2FrameHeaderToStream(NULL, 0, 0);
    
    assert_null(stream);
}

static void id3v2FrameHeaderToStream_v2(void **state){

    Id3v2FrameHeader *frameHeader = id3v2CreateFrameHeader((uint8_t *)"TT2", false, false, false, false, 0, 0, 0);
    ByteStream *stream = id3v2FrameHeaderToStream(frameHeader, ID3V2_TAG_VERSION_2, 100);

    assert_memory_equal("TT2", byteStreamCursor(stream), 3);
    byteStreamSeek(stream, 3, SEEK_CUR);

    assert_memory_equal("\x00\x00\x64", byteStreamCursor(stream), 3);
    byteStreamSeek(stream, 3, SEEK_CUR);

    byteStreamDestroy(stream);
    id3v2DestroyFrameHeader(&frameHeader);
}

static void id3v2FrameHeaderToStream_v3(void **state){

    Id3v2FrameHeader *frameHeader = id3v2CreateFrameHeader((uint8_t *)"TIT2", false, false, false, false, 0, 0, 0);
    ByteStream *stream = id3v2FrameHeaderToStream(frameHeader, ID3V2_TAG_VERSION_3, 300);

    assert_memory_equal("TIT2", byteStreamCursor(stream), 4);
    byteStreamSeek(stream, 4, SEEK_CUR);

    assert_int_equal(300, byteStreamReturnU32(stream));

    assert_memory_equal("\x00\x00", byteStreamCursor(stream), 2);

    byteStreamDestroy(stream);
    id3v2DestroyFrameHeader(&frameHeader);
}

static void id3v2FrameHeaderToStream_v3AllFlags(void **state){

    Id3v2FrameHeader *frameHeader = id3v2CreateFrameHeader((uint8_t *)"TALB", true, true, true, false, 500, 0x0F, 0xFF);
    ByteStream *stream = id3v2FrameHeaderToStream(frameHeader, ID3V2_TAG_VERSION_3, 600);

    

    assert_memory_equal("TALB", byteStreamCursor(stream), 4);
    byteStreamSeek(stream, 4, SEEK_CUR);

    assert_int_equal(600, byteStreamReturnU32(stream));

    assert_int_equal(0xE0, byteStreamCursor(stream)[0]);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(0xE0, byteStreamCursor(stream)[0]);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(500, byteStreamReturnU32(stream));

    assert_int_equal(0x0F, byteStreamCursor(stream)[0]);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(0xFF, byteStreamCursor(stream)[0]);
    byteStreamSeek(stream, 1, SEEK_CUR);

    byteStreamDestroy(stream);
    id3v2DestroyFrameHeader(&frameHeader);
}

static void id3v2FrameHeaderToStream_v3OnlySymbols(void **state){

    Id3v2FrameHeader *frameHeader = id3v2CreateFrameHeader((uint8_t *)"TALB", false, false, false, false, 0, 0xAF, 0x56);
    ByteStream *stream = id3v2FrameHeaderToStream(frameHeader, ID3V2_TAG_VERSION_3, 100);

    assert_memory_equal("TALB", byteStreamCursor(stream), 4);
    byteStreamSeek(stream, 4, SEEK_CUR);

    assert_int_equal(100, byteStreamReturnU32(stream));

    assert_int_equal(0x00, byteStreamCursor(stream)[0]);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(0x60, byteStreamCursor(stream)[0]);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(0xAF, byteStreamCursor(stream)[0]);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(0x56, byteStreamCursor(stream)[0]);
    byteStreamSeek(stream, 1, SEEK_CUR);

    byteStreamDestroy(stream);
    id3v2DestroyFrameHeader(&frameHeader);
}

static void id3v2FrameHeaderToStream_v4(void **state){

    Id3v2FrameHeader *frameHeader = id3v2CreateFrameHeader((uint8_t *)"TSOA", false, false, false, false, 0, 0, 0);
    ByteStream *stream = id3v2FrameHeaderToStream(frameHeader, ID3V2_TAG_VERSION_3, 100);

    assert_memory_equal("TSOA", byteStreamCursor(stream), 4);
    byteStreamSeek(stream, 4, SEEK_CUR);

    assert_int_equal(100, byteStreamReturnSyncInt(stream));

    assert_memory_equal("\x00\x00", byteStreamCursor(stream), 2);
    byteStreamSeek(stream, 2, SEEK_CUR);

    byteStreamDestroy(stream);
    id3v2DestroyFrameHeader(&frameHeader);
}

static void id3v2FrameHeaderToStream_v4AllFlags(void **state){

    Id3v2FrameHeader *frameHeader = id3v2CreateFrameHeader((uint8_t *)"TSOA", true, true, true, true, 12345, 0x11, 0x22);
    ByteStream *stream = id3v2FrameHeaderToStream(frameHeader, ID3V2_TAG_VERSION_4, 500);

    assert_memory_equal("TSOA", byteStreamCursor(stream), 4);
    byteStreamSeek(stream, 4, SEEK_CUR);

    assert_int_equal(500, byteStreamReturnSyncInt(stream));

    assert_memory_equal("\x70\x4F", byteStreamCursor(stream), 2);
    byteStreamSeek(stream, 2, SEEK_CUR);

    assert_int_equal(0x22, byteStreamCursor(stream)[0]);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(0x11, byteStreamCursor(stream)[0]);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(12345, byteStreamReturnU32(stream));

    byteStreamDestroy(stream);
    id3v2DestroyFrameHeader(&frameHeader);
}

static void id3v2FrameHeaderToStream_v4NoUnsync(void **state){

    Id3v2FrameHeader *frameHeader = id3v2CreateFrameHeader((uint8_t *)"TSOA", true, true, true, false, 12345, 0x11, 0x22);
    ByteStream *stream = id3v2FrameHeaderToStream(frameHeader, ID3V2_TAG_VERSION_4, 500);

    assert_memory_equal("TSOA", byteStreamCursor(stream), 4);
    byteStreamSeek(stream, 4, SEEK_CUR);

    assert_int_equal(500, byteStreamReturnSyncInt(stream));

    assert_memory_equal("\x70\x4D", byteStreamCursor(stream), 2);
    byteStreamSeek(stream, 2, SEEK_CUR);

    assert_int_equal(0x22, byteStreamCursor(stream)[0]);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(0x11, byteStreamCursor(stream)[0]);
    byteStreamSeek(stream, 1, SEEK_CUR);

    assert_int_equal(12345, byteStreamReturnU32(stream));

    byteStreamDestroy(stream);
    id3v2DestroyFrameHeader(&frameHeader);
}

static void id3v2FrameHeaderToJSON_null(void **state){

    char *json = id3v2FrameHeaderToJSON(NULL, 10);

    assert_string_equal("{}", json);

    free(json);
}

static void id3v2FrameHeaderToJSON_v2(void **state){

    Id3v2FrameHeader *frame = id3v2CreateFrameHeader((uint8_t *)"TT1", false, false, false, false, 0, 0, 0);
    char *json = id3v2FrameHeaderToJSON(frame, ID3V2_TAG_VERSION_2);

    assert_string_equal("{\"id\":\"TT1\"}", 
                        json);

    free(json);
    id3v2DestroyFrameHeader(&frame);
}

static void id3v2FrameHeaderToJSON_v3(void **state){

    Id3v2FrameHeader *frame = id3v2CreateFrameHeader((uint8_t *)"TIT1", false, false, false, false, 0, 0, 0);
    char *json = id3v2FrameHeaderToJSON(frame, ID3V2_TAG_VERSION_3);

    assert_string_equal("{\"id\":\"TIT1\",\"tagAlterPreservation\":false,\"fileAlterPreservation\":false,\"readOnly\":false,\"decompressionSize\":0,\"encryptionSymbol\":0,\"groupSymbol\":0}", 
                        json);

    free(json);
    id3v2DestroyFrameHeader(&frame);
}

static void id3v2FrameHeaderToJSON_v3Symbol(void **state){

    Id3v2FrameHeader *frame = id3v2CreateFrameHeader((uint8_t *)"TIT1", false, false, false, false, 0, 20, 0);
    char *json = id3v2FrameHeaderToJSON(frame, ID3V2_TAG_VERSION_3);

    assert_string_equal("{\"id\":\"TIT1\",\"tagAlterPreservation\":false,\"fileAlterPreservation\":false,\"readOnly\":false,\"decompressionSize\":0,\"encryptionSymbol\":20,\"groupSymbol\":0}", 
                        json);

    free(json);
    id3v2DestroyFrameHeader(&frame);
}

static void id3v2FrameHeaderToJSON_v4WithUnsync(void **state){

    Id3v2FrameHeader *frame = id3v2CreateFrameHeader((uint8_t *)"TIT1", false, false, false, true, 0, 0, 0);
    char *json = id3v2FrameHeaderToJSON(frame, ID3V2_TAG_VERSION_4);

    assert_string_equal("{\"id\":\"TIT1\",\"tagAlterPreservation\":false,\"fileAlterPreservation\":false,\"readOnly\":false,\"unsynchronisation\":true,\"decompressionSize\":0,\"encryptionSymbol\":0,\"groupSymbol\":0}", 
                        json);

    free(json);
    id3v2DestroyFrameHeader(&frame);
}

static void id3v2FrameToStream_v4TALB(void **state){

    ByteStream *stream = byteStreamFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromStream(stream, NULL);

    ByteStream *rep = id3v2FrameToStream((Id3v2Frame *)tag->frames->head->data, ID3V2_TAG_VERSION_4);
    

    assert_memory_equal(byteStreamCursor(rep), "TALB", 4);
    byteStreamSeek(rep, 4, SEEK_CUR);

    assert_memory_equal(byteStreamCursor(rep), "\x00\x00\x00\x43", 4);
    byteStreamSeek(rep, 4, SEEK_CUR);

    assert_memory_equal(byteStreamCursor(rep), "\x00\x00", 2);
    byteStreamSeek(rep, 2, SEEK_CUR);

    assert_memory_equal(byteStreamCursor(rep), "\x03", 1);
    byteStreamSeek(rep, 1, SEEK_CUR);

    assert_memory_equal(byteStreamCursor(rep), "The Powers That Butf8ÛȾℲⅧ♈ ♉ ♊ ♋ ♌ ♍ ♎ ♏utf8", 67);
    byteStreamSeek(rep, 67, SEEK_CUR);

    byteStreamDestroy(rep);
    byteStreamDestroy(stream);
    id3v2DestroyTag(&tag);
}

static void id3v2FrameToStream_v4WCOM(void **state){

    ByteStream *stream = byteStreamFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromStream(stream, NULL);

    ListIter iter = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    ByteStream *rep = NULL;

    while((f = id3v2FrameTraverse(&iter)) != NULL){
        if(memcmp(f->header->id, "WCOM", ID3V2_FRAME_ID_MAX_SIZE) == 0){
            
            rep = id3v2FrameToStream(f, ID3V2_TAG_VERSION_4);

            break;
        }
    }

    assert_memory_equal(byteStreamCursor(rep), "WCOM", 4);
    byteStreamSeek(rep, 4, SEEK_CUR);

    assert_memory_equal(byteStreamCursor(rep), "\x00\x00\x00\x2c", 4);
    byteStreamSeek(rep, 4, SEEK_CUR);

    assert_memory_equal(byteStreamCursor(rep), "\x00\x00", 2);
    byteStreamSeek(rep, 2, SEEK_CUR);

    assert_memory_equal(byteStreamCursor(rep), "The Powers That Butf8\xdb>2gH I J K L M N Outf8", 44);
    byteStreamSeek(rep, 44, SEEK_CUR);

    byteStreamDestroy(rep);
    byteStreamDestroy(stream);
    id3v2DestroyTag(&tag);

}

static void id3v2FrameToStream_v4ETCO(void **state){

    ByteStream *stream = byteStreamFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromStream(stream, NULL);

    ListIter iter = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    ByteStream *rep = NULL;

    while((f = id3v2FrameTraverse(&iter)) != NULL){
        
        if(memcmp(f->header->id, "ETCO", ID3V2_FRAME_ID_MAX_SIZE) == 0){
            printf("%c%c%c%c\n",f->header->id[0],f->header->id[1],f->header->id[2],f->header->id[3]);
            rep = id3v2FrameToStream(f, ID3V2_TAG_VERSION_4);

            break;
        }
    }

    assert_memory_equal("ETCO", byteStreamCursor(rep), 4);
    byteStreamSeek(rep, 4, SEEK_CUR);

    assert_memory_equal("\x00\x00\x00\x0b", byteStreamCursor(rep), 4);
    byteStreamSeek(rep, 4, SEEK_CUR);

    assert_memory_equal("\x00\x00", byteStreamCursor(rep), 2);
    byteStreamSeek(rep, 2, SEEK_CUR);

    assert_memory_equal("\x02", byteStreamCursor(rep), 1);
    byteStreamSeek(rep, 1, SEEK_CUR);

    assert_memory_equal("\x06", byteStreamCursor(rep), 1);
    byteStreamSeek(rep, 1, SEEK_CUR);

    assert_memory_equal("\x00\x12\x9d\xa0", byteStreamCursor(rep), 4);
    byteStreamSeek(rep, 4, SEEK_CUR);

    assert_memory_equal("\x02", byteStreamCursor(rep), 1);
    byteStreamSeek(rep, 1, SEEK_CUR);

    assert_memory_equal("\x00\x09\x4e\xd0", byteStreamCursor(rep), 4);
    byteStreamSeek(rep, 4, SEEK_CUR);

    byteStreamDestroy(rep);
    byteStreamDestroy(stream);
    id3v2DestroyTag(&tag);

}

static void playground(void **state){

    ByteStream *stream = byteStreamFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromStream(stream, NULL);

    ListIter iter = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    ByteStream *rep = NULL;

    while((f = id3v2FrameTraverse(&iter)) != NULL){
        
        if(memcmp(f->header->id, "ETCO", ID3V2_FRAME_ID_MAX_SIZE) == 0){
            printf("%c%c%c%c\n",f->header->id[0],f->header->id[1],f->header->id[2],f->header->id[3]);
            rep = id3v2FrameToStream(f, ID3V2_TAG_VERSION_4);

            break;
        }
    }

    assert_memory_equal("ETCO", byteStreamCursor(rep), 4);
    byteStreamSeek(rep, 4, SEEK_CUR);

    assert_memory_equal("\x00\x00\x00\x0b", byteStreamCursor(rep), 4);
    byteStreamSeek(rep, 4, SEEK_CUR);

    assert_memory_equal("\x00\x00", byteStreamCursor(rep), 2);
    byteStreamSeek(rep, 2, SEEK_CUR);

    assert_memory_equal("\x02", byteStreamCursor(rep), 1);
    byteStreamSeek(rep, 1, SEEK_CUR);

    assert_memory_equal("\x06", byteStreamCursor(rep), 1);
    byteStreamSeek(rep, 1, SEEK_CUR);

    assert_memory_equal("\x00\x12\x9d\xa0", byteStreamCursor(rep), 4);
    byteStreamSeek(rep, 4, SEEK_CUR);

    assert_memory_equal("\x02", byteStreamCursor(rep), 1);
    byteStreamSeek(rep, 1, SEEK_CUR);

    assert_memory_equal("\x00\x09\x4e\xd0", byteStreamCursor(rep), 4);
    byteStreamSeek(rep, 4, SEEK_CUR);

    byteStreamDestroy(rep);
    byteStreamDestroy(stream);
    id3v2DestroyTag(&tag);
}


int main(){

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(id3v2CreateAndDestroyFrameHeader_allInOne),
        cmocka_unit_test(id3v2CreateAndDestroyContentEntry_allInOne),
        cmocka_unit_test(id3v2CreateAndDestroyHeader_allInOne),
        cmocka_unit_test(id3v2Traverse_allInOne),

        cmocka_unit_test(id3v2ReadFrameEntry_allEntries),
        cmocka_unit_test(id3v2ReadFrameEntry_TextFrameAsChar),
        cmocka_unit_test(id3v2ReadFrameEntry_TextFrameEncodings),
        cmocka_unit_test(id3v2ReadFrameEntry_checkETCO),

        cmocka_unit_test(id3v2WriteFrameEntry_greatestHits),
        cmocka_unit_test(id3v2WriteFrameEntry_updateTitle),
        cmocka_unit_test(id3v2AtatchFrameFromTag_TSOA),
        cmocka_unit_test(id3v2DetatchFrameFromTag_TIT2),

        // id3v2FrameHeaderToStream
        cmocka_unit_test(id3v2FrameHeaderToStream_null),
        cmocka_unit_test(id3v2FrameHeaderToStream_v2),
        cmocka_unit_test(id3v2FrameHeaderToStream_v3),
        cmocka_unit_test(id3v2FrameHeaderToStream_v3AllFlags),
        cmocka_unit_test(id3v2FrameHeaderToStream_v3OnlySymbols),
        cmocka_unit_test(id3v2FrameHeaderToStream_v4),
        cmocka_unit_test(id3v2FrameHeaderToStream_v4AllFlags),
        cmocka_unit_test(id3v2FrameHeaderToStream_v4NoUnsync),

        // id3v2FrameHeaderToJSON
        cmocka_unit_test(id3v2FrameHeaderToJSON_null),
        cmocka_unit_test(id3v2FrameHeaderToJSON_v2),
        cmocka_unit_test(id3v2FrameHeaderToJSON_v3),
        cmocka_unit_test(id3v2FrameHeaderToJSON_v3Symbol),
        cmocka_unit_test(id3v2FrameHeaderToJSON_v4WithUnsync),

        // id3v2FrameToStream
        cmocka_unit_test(id3v2FrameToStream_v4TALB),
        cmocka_unit_test(id3v2FrameToStream_v4WCOM),
        cmocka_unit_test(id3v2FrameToStream_v4ETCO),


        cmocka_unit_test(playground)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}