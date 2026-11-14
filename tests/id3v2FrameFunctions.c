/**
 * @file id3v2FrameFunctions.c
 * @author Ewan Jones
 * @brief unit tests for id3v2Frame.c
 * @version 0.1
 * @date 2024-03-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <stdio.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include "id3v2/id3v2Frame.h"
#include "id3v2/id3v2Parser.h"
#include "id3v2/id3v2Context.h"
#include "id3v2/id3v2TagIdentity.h"
#include "byteStream.h"
#include "byteInt.h"
#include "byteUnicode.h"


static void id3v2CreateAndDestroyFrameHeader_allInOne(void **state) {
    (void) state;
    Id3v2FrameHeader *h = id3v2CreateFrameHeader((uint8_t *) "TT2", false, false, false, false, 0, 0xff, 0xff);

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

static void id3v2CreateAndDestroyContentEntry_allInOne(void **state) {
    (void) state;
    Id3v2ContentEntry *ce = id3v2CreateContentEntry((void *) "test", 5);

    assert_non_null(ce);
    assert_string_equal("test", (void *)ce->entry);
    assert_int_equal(ce->size, 5);

    id3v2DeleteContentEntry((void *) ce);
}

static void id3v2CreateAndDestroyHeader_allInOne(void **state) {
    (void) state;

    List *l = listCreate(id3v2PrintContentEntry, id3v2DeleteContentEntry, id3v2CompareContentEntry,
                         id3v2CopyContentEntry);
    listInsertBack(l, (void *) id3v2CreateContentEntry((void *) "test", 5));
    listInsertBack(l, (void *) id3v2CreateContentEntry((void *) "test2", 6));
    listInsertBack(l, (void *) id3v2CreateContentEntry((void *) "test3", 6));

    Id3v2FrameHeader *h = id3v2CreateFrameHeader((uint8_t *) "TT2", false, false, false, false, 0, 0xff, 0xff);

    Id3v2Frame *f = id3v2CreateFrame(h, id3v2CreateTextFrameContext(), l);

    assert_non_null(f);
    assert_non_null(f->header);
    assert_non_null(f->contexts);
    assert_non_null(f->entries);

    id3v2DestroyFrame(&f);

    assert_null(f);
}

static void id3v2Traverse_allInOne(void **state) {
    (void) state;
    ByteStream *stream = byteStreamFromFile("assets/boniver.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    int i = 0;

    for (i = 0;; i++) {
        f = id3v2FrameTraverse(&frames);

        if (f == NULL) {
            break;
        }
    }

    assert_int_equal(i, 93);


    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);
}

static void id3v2ReadFrameEntry_allEntries(void **state) {
    (void) state;
    ByteStream *stream = byteStreamFromFile("assets/boniver.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    while ((f = id3v2FrameTraverse(&frames)) != NULL) {
        ListIter entries = id3v2CreateFrameEntryTraverser(f);
        void *tmp = NULL;
        size_t s = 0;
        int i = 0;
        while ((tmp = id3v2ReadFrameEntry(&entries, &s)) != NULL) {
            assert_non_null(tmp);

            if (i == 0) {
                assert_memory_equal(tmp, "http://musicbrainz.org", 23);
                assert_int_equal(s, 23);
            } else if (i == 1) {
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

static void id3v2ReadFrameEntry_TextFrameAsChar(void **state) {
    (void) state;
    ByteStream *stream = byteStreamFromFile("assets/sorry4dying.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    while ((f = id3v2FrameTraverse(&frames)) != NULL) {
        ListIter entries = id3v2CreateFrameEntryTraverser(f);
        char *tmp = NULL;
        size_t s = 0;
        int i = 0;
        while ((tmp = id3v2ReadFrameEntryAsChar(&entries, &s)) != NULL) {
            assert_non_null(tmp);

            if (i == 0) {
                assert_memory_equal(tmp, "\x01", 1);
                assert_int_equal(s, 1);
            } else if (i == 1) {
                assert_string_equal(tmp, "sorry4dying");
                assert_int_equal(s, 11);
            }

            if (tmp) {
                free(tmp);
            }

            i++;
        }
        break;
    }

    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);
}


static void id3v2ReadFrameEntry_TextFrameEncodings(void **state) {
    (void) state;
    ByteStream *stream = byteStreamFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;
    int i = 0;
    while ((f = id3v2FrameTraverse(&frames)) != NULL) {
        if (f->header->id[0] == 'T') {
            ListIter entries = id3v2CreateFrameEntryTraverser(f);
            uint8_t encoding = id3v2ReadFrameEntryAsU8(&entries);

            switch (i) {
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

static void id3v2ReadFrameEntry_checkETCO(void **state) {
    (void) state;
    ByteStream *stream = byteStreamFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    while ((f = id3v2FrameTraverse(&frames)) != NULL) {
        if (!memcmp(f->header->id, "ETCO", 4)) {
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

static void id3v2WriteFrameEntry_greatestHits(void **state) {
    (void) state;
    ByteStream *stream = byteStreamFromFile("assets/boniver.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;
    bool exit = false;
    while ((f = id3v2FrameTraverse(&frames)) != NULL) {
        if (memcmp(f->header->id, "TXX", 3) == 0) {
            ListIter entries = id3v2CreateFrameEntryTraverser(f);
            size_t s = 0;
            char *tmp = NULL;

            while ((tmp = id3v2ReadFrameEntryAsChar(&entries, &s)) != NULL) {
                if (s >= 16 && memcmp("IS_GREATEST_HITS", tmp, 16) == 0) {
                    uint8_t zero = 0;
                    id3v2WriteFrameEntry(f, &entries, 1, (void *) &zero);

                    uint8_t ret = id3v2ReadFrameEntryAsU8(&entries);
                    assert_int_equal(ret, 0);
                    exit = true;
                    free(tmp);
                    break;
                }

                free(tmp);
            }
        }

        if (exit) {
            break;
        }
    }

    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);
}


static void id3v2WriteFrameEntry_updateTitle(void **state) {
    (void) state;
    ByteStream *stream = byteStreamFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    while ((f = id3v2FrameTraverse(&frames)) != NULL) {
        if (memcmp(f->header->id, "TIT2", 4) == 0) {
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

// entry shorter than the context's min: copied then zero padded, never read past entrySize
static void id3v2WriteFrameEntry_padToMin(void **state) {
    (void) state;
    HashTable *pairs = hashTableCreate(4, id3v2DeleteContentContextList, id3v2PrintContentContextList,
                                       id3v2CopyContentContextList);
    List *context = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                               id3v2CopyContentContext);
    assert_true(listInsertBack(context, id3v2CreateContentContext(binary_context, id3v2djb2("data"), 32, 16)));
    assert_int_equal(hashTableInsert(pairs, "XMIN", context), 0);

    Id3v2Frame *f = id3v2CreateEmptyFrame("XMIN", ID3V2_TAG_VERSION_3, pairs);
    assert_non_null(f);

    // heap allocated and exactly 1 byte so ASan reports any over read
    uint8_t *one = malloc(1);
    assert_non_null(one);
    one[0] = 0xAB;

    ListIter entries = id3v2CreateFrameEntryTraverser(f);
    assert_true(id3v2WriteFrameEntry(f, &entries, 1, one));
    free(one);

    size_t s = 0;
    uint8_t expected[16] = {0xAB};
    uint8_t *data = id3v2ReadFrameEntry(&entries, &s);
    assert_non_null(data);
    assert_int_equal(s, 16);
    assert_memory_equal(data, expected, 16);
    free(data);

    // longer than max is still cut to max
    uint8_t big[40];
    memset(big, 0x11, sizeof(big));
    entries = id3v2CreateFrameEntryTraverser(f);
    assert_true(id3v2WriteFrameEntry(f, &entries, sizeof(big), big));
    data = id3v2ReadFrameEntry(&entries, &s);
    assert_non_null(data);
    assert_int_equal(s, 32);
    assert_memory_equal(data, big, 32);
    free(data);

    id3v2DestroyFrame(&f);
    listFree(context);
    hashTableFree(pairs);
}

// AsU16 and AsU32 both read big-endian, as ID3v2 stores numbers; 1 byte entries zero extend
static void id3v2ReadFrameEntryAsU16U32_bigEndian(void **state) {
    (void) state;
    HashTable *pairs = hashTableCreate(4, id3v2DeleteContentContextList, id3v2PrintContentContextList,
                                       id3v2CopyContentContextList);
    List *context = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                               id3v2CopyContentContext);
    assert_true(listInsertBack(context, id3v2CreateContentContext(numeric_context, id3v2djb2("a"), 2, 2)));
    assert_true(listInsertBack(context, id3v2CreateContentContext(numeric_context, id3v2djb2("b"), 4, 4)));
    assert_true(listInsertBack(context, id3v2CreateContentContext(numeric_context, id3v2djb2("c"), 1, 1)));
    assert_int_equal(hashTableInsert(pairs, "XNUM", context), 0);

    Id3v2Frame *f = id3v2CreateEmptyFrame("XNUM", ID3V2_TAG_VERSION_3, pairs);
    assert_non_null(f);

    uint8_t u16[2] = {0x01, 0x02};
    uint8_t u32[4] = {0x01, 0x02, 0x03, 0x04};
    uint8_t one = 0xAB;

    ListIter entries = id3v2CreateFrameEntryTraverser(f);
    assert_true(id3v2WriteFrameEntry(f, &entries, sizeof(u16), u16));
    id3v2ReadFrameEntryAsU8(&entries);
    assert_true(id3v2WriteFrameEntry(f, &entries, sizeof(u32), u32));
    id3v2ReadFrameEntryAsU8(&entries);
    assert_true(id3v2WriteFrameEntry(f, &entries, 1, &one));

    entries = id3v2CreateFrameEntryTraverser(f);
    assert_int_equal(id3v2ReadFrameEntryAsU16(&entries), 0x0102);
    assert_int_equal(id3v2ReadFrameEntryAsU32(&entries), 0x01020304);
    assert_int_equal(id3v2ReadFrameEntryAsU16(&entries), 0x00AB);

    id3v2DestroyFrame(&f);
    listFree(context);
    hashTableFree(pairs);
}

static void id3v2AttachFrameFromTag_TSOA(void **state) {
    (void) state;
    ByteStream *stream = byteStreamFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);


    List *entries = listCreate(id3v2PrintContentEntry, id3v2DeleteContentEntry, id3v2CompareContentEntry,
                               id3v2CopyContentEntry);
    listInsertBack(entries, id3v2CreateContentEntry((void *) "\x0", 1));
    listInsertBack(entries, id3v2CreateContentEntry((void *) "SORT", 5));

    Id3v2FrameHeader *h = id3v2CreateFrameHeader((uint8_t *) "TSOA", false, false, false, false, 0, 0, 0);
    Id3v2Frame *f = id3v2CreateFrame(h, id3v2CreateTextFrameContext(), entries);


    assert_true(id3v2AttachFrameToTag(tag, f));

    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);

    // if the memory assigned to f is freed it was added successfully
}

static void id3v2DetachFrameFromTag_TIT2(void **state) {
    (void) state;
    ByteStream *stream = byteStreamFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    while ((f = id3v2FrameTraverse(&frames)) != NULL) {
        if (memcmp(f->header->id, "TIT2", 4) == 0) {
            Id3v2Frame *detach = id3v2DetachFrameFromTag(tag, f);

            assert_non_null(detach);

            id3v2DestroyFrame(&detach);

            assert_null(detach);
        }
    }


    frames = id3v2CreateFrameTraverser(tag);
    bool exit = false;
    while ((f = id3v2FrameTraverse(&frames)) != NULL) {
        if (memcmp(f->header->id, "TIT2", 4) == 0) {
            exit = true;
        }
    }


    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);

    assert_false(exit);
}

static void id3v2FrameHeaderSerialize_null(void **state) {
    (void) state;
    size_t outl = 0;
    uint8_t *out = id3v2FrameHeaderSerialize(NULL, 0, 0, &outl);

    assert_null(out);
}

static void id3v2FrameHeaderSerialize_v2(void **state) {
    (void) state;
    Id3v2FrameHeader *frameHeader = id3v2CreateFrameHeader((uint8_t *) "TT2", false, false, false, false, 0, 0, 0);
    size_t outl = 0;
    uint8_t *out = id3v2FrameHeaderSerialize(frameHeader, ID3V2_TAG_VERSION_2, 100, &outl);
    uint8_t *start = out;

    assert_memory_equal("TT2", out, 3);
    out += 3;

    assert_memory_equal("\x00\x00\x64", out, 3);

    id3v2DestroyFrameHeader(&frameHeader);
    free(start);
}

static void id3v2FrameHeaderSerialize_v3(void **state) {
    (void) state;
    Id3v2FrameHeader *frameHeader = id3v2CreateFrameHeader((uint8_t *) "TIT2", false, false, false, false, 0, 0, 0);
    size_t outl = 0;
    uint8_t *out = id3v2FrameHeaderSerialize(frameHeader, ID3V2_TAG_VERSION_3, 300, &outl);
    uint8_t *start = out;

    assert_memory_equal("TIT2", out, 4);
    out += 4;

    assert_int_equal(300, btou32(out, 4));
    out += 4;

    assert_memory_equal("\x00\x00", out, 2);

    free(start);
    id3v2DestroyFrameHeader(&frameHeader);
}

static void id3v2FrameHeaderSerialize_v3AllFlags(void **state) {
    (void) state;
    Id3v2FrameHeader *frameHeader =
            id3v2CreateFrameHeader((uint8_t *) "TALB", true, true, true, false, 500, 0x0F, 0xFF);
    size_t outl = 0;
    uint8_t *out = id3v2FrameHeaderSerialize(frameHeader, ID3V2_TAG_VERSION_3, 600, &outl);
    uint8_t *start = out;

    assert_memory_equal("TALB", out, 4);
    out += 4;

    assert_int_equal(600, btou32(out, 4));
    out += 4;

    assert_int_equal(0xE0, out[0]);
    out++;

    assert_int_equal(0xE0, out[0]);
    out++;

    assert_int_equal(500, btou32(out, 4));
    out += 4;

    assert_int_equal(0x0F, out[0]);
    out++;

    assert_int_equal(0xFF, out[0]);

    free(start);
    id3v2DestroyFrameHeader(&frameHeader);
}

static void id3v2FrameHeaderSerialize_v3OnlySymbols(void **state) {
    (void) state;
    Id3v2FrameHeader *frameHeader = id3v2CreateFrameHeader((uint8_t *) "TALB", false, false, false, false, 0, 0xAF,
                                                           0x56);
    size_t outl = 0;
    uint8_t *out = id3v2FrameHeaderSerialize(frameHeader, ID3V2_TAG_VERSION_3, 100, &outl);
    uint8_t *start = out;

    assert_memory_equal("TALB", out, 4);
    out += 4;

    assert_int_equal(100, btou32(out, 4));
    out += 4;

    assert_int_equal(0x00, out[0]);
    out++;

    assert_int_equal(0x60, out[0]);
    out++;

    assert_int_equal(0xAF, out[0]);
    out++;

    assert_int_equal(0x56, out[0]);

    free(start);
    id3v2DestroyFrameHeader(&frameHeader);
}

static void id3v2FrameHeaderSerialize_v4(void **state) {
    (void) state;
    Id3v2FrameHeader *frameHeader = id3v2CreateFrameHeader((uint8_t *) "TSOA", false, false, false, false, 0, 0, 0);
    size_t outl = 0;
    uint8_t *out = id3v2FrameHeaderSerialize(frameHeader, ID3V2_TAG_VERSION_3, 100, &outl);

    ByteStream *stream = byteStreamCreate(out, outl);

    assert_memory_equal("TSOA", byteStreamCursor(stream), 4);
    byteStreamSeek(stream, 4, SEEK_CUR);

    assert_int_equal(100, byteStreamReturnSyncInt(stream));

    assert_memory_equal("\x00\x00", byteStreamCursor(stream), 2);
    byteStreamSeek(stream, 2, SEEK_CUR);

    free(out);
    byteStreamDestroy(stream);
    id3v2DestroyFrameHeader(&frameHeader);
}

static void id3v2FrameHeaderSerialize_v4AllFlags(void **state) {
    (void) state;
    Id3v2FrameHeader *frameHeader = id3v2CreateFrameHeader((uint8_t *) "TSOA", true, true, true, true, 12345, 0x11,
                                                           0x22);
    size_t outl = 0;
    uint8_t *out = id3v2FrameHeaderSerialize(frameHeader, ID3V2_TAG_VERSION_4, 500, &outl);
    ByteStream *stream = byteStreamCreate(out, outl);


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

    free(out);
    byteStreamDestroy(stream);
    id3v2DestroyFrameHeader(&frameHeader);
}

static void id3v2FrameHeaderSerialize_v4NoUnsync(void **state) {
    (void) state;
    Id3v2FrameHeader *frameHeader = id3v2CreateFrameHeader((uint8_t *) "TSOA", true, true, true, false, 12345, 0x11,
                                                           0x22);
    size_t outl = 0;
    uint8_t *out = id3v2FrameHeaderSerialize(frameHeader, ID3V2_TAG_VERSION_4, 500, &outl);
    ByteStream *stream = byteStreamCreate(out, outl);

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

    free(out);
    byteStreamDestroy(stream);
    id3v2DestroyFrameHeader(&frameHeader);
}

static void id3v2FrameHeaderToJSON_null(void **state) {
    (void) state;
    char *json = id3v2FrameHeaderToJSON(NULL, 10);

    assert_string_equal("{}", json);

    free(json);
}

static void id3v2FrameHeaderToJSON_v2(void **state) {
    (void) state;
    Id3v2FrameHeader *frame = id3v2CreateFrameHeader((uint8_t *) "TT1", false, false, false, false, 0, 0, 0);
    char *json = id3v2FrameHeaderToJSON(frame, ID3V2_TAG_VERSION_2);

    assert_string_equal("{\"id\":\"TT1\"}",
                        json);

    free(json);
    id3v2DestroyFrameHeader(&frame);
}

static void id3v2FrameHeaderToJSON_v3(void **state) {
    (void) state;
    Id3v2FrameHeader *frame = id3v2CreateFrameHeader((uint8_t *) "TIT1", false, false, false, false, 0, 0, 0);
    char *json = id3v2FrameHeaderToJSON(frame, ID3V2_TAG_VERSION_3);

    assert_string_equal(
        "{\"id\":\"TIT1\",\"tagAlterPreservation\":false,\"fileAlterPreservation\":false,\"readOnly\":false,\"decompressionSize\":0,\"encryptionSymbol\":0,\"groupSymbol\":0}",
        json);

    free(json);
    id3v2DestroyFrameHeader(&frame);
}

static void id3v2FrameHeaderToJSON_v3Symbol(void **state) {
    (void) state;
    Id3v2FrameHeader *frame = id3v2CreateFrameHeader((uint8_t *) "TIT1", false, false, false, false, 0, 20, 0);
    char *json = id3v2FrameHeaderToJSON(frame, ID3V2_TAG_VERSION_3);

    assert_string_equal(
        "{\"id\":\"TIT1\",\"tagAlterPreservation\":false,\"fileAlterPreservation\":false,\"readOnly\":false,\"decompressionSize\":0,\"encryptionSymbol\":20,\"groupSymbol\":0}",
        json);

    free(json);
    id3v2DestroyFrameHeader(&frame);
}

static void id3v2FrameHeaderToJSON_v4WithUnsync(void **state) {
    (void) state;
    Id3v2FrameHeader *frame = id3v2CreateFrameHeader((uint8_t *) "TIT1", false, false, false, true, 0, 0, 0);
    char *json = id3v2FrameHeaderToJSON(frame, ID3V2_TAG_VERSION_4);

    assert_string_equal(
        "{\"id\":\"TIT1\",\"tagAlterPreservation\":false,\"fileAlterPreservation\":false,\"readOnly\":false,\"unsynchronisation\":true,\"decompressionSize\":0,\"encryptionSymbol\":0,\"groupSymbol\":0}",
        json);

    free(json);
    id3v2DestroyFrameHeader(&frame);
}

static void id3v2FrameSerialize_v4TALB(void **state) {
    (void) state;
    ByteStream *stream = byteStreamFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);

    size_t outl = 0;
    uint8_t *out = id3v2FrameSerialize((Id3v2Frame *) tag->frames->head->data, ID3V2_TAG_VERSION_4, &outl);
    ByteStream *rep = byteStreamCreate(out, outl);


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

    free(out);
    byteStreamDestroy(rep);
    byteStreamDestroy(stream);
    id3v2DestroyTag(&tag);
}

static void id3v2FrameSerialize_v4WCOM(void **state) {
    (void) state;
    ByteStream *stream = byteStreamFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);

    ListIter iter = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    ByteStream *rep = NULL;

    while ((f = id3v2FrameTraverse(&iter)) != NULL) {
        if (memcmp(f->header->id, "WCOM", ID3V2_FRAME_ID_MAX_SIZE) == 0) {
            size_t outl = 0;
            uint8_t *out = id3v2FrameSerialize(f, ID3V2_TAG_VERSION_4, &outl);
            rep = byteStreamCreate(out, outl);

            free(out);

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

static void id3v2FrameSerialize_v4ETCO(void **state) {
    (void) state;
    ByteStream *stream = byteStreamFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);

    ListIter iter = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    ByteStream *rep = NULL;

    while ((f = id3v2FrameTraverse(&iter)) != NULL) {
        if (memcmp(f->header->id, "ETCO", ID3V2_FRAME_ID_MAX_SIZE) == 0) {
            size_t outl = 0;
            uint8_t *out = id3v2FrameSerialize(f, ID3V2_TAG_VERSION_4, &outl);

            rep = byteStreamCreate(out, outl);

            free(out);
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

static void id3v2FrameSerialize_v2EQU(void **state) {
    (void) state;
    // EQU
    uint8_t equ[15] = {
        'E', 'Q', 'U', 0x00, 0x00, 0x09,
        0x10, // 16 adjustment bits -> 2 byte volumes
        0x03, 0xe9, // 0 000001111101001  inc/dec (MSB) + frequency
        0x40, 0x00,
        0x80, 0x28, // 1 000000000101000  inc/dec (MSB) + frequency
        0xfc, 0x00
    };


    ByteStream *stream = byteStreamCreate(equ, 15);
    ByteStream *rep = NULL;
    Id3v2Frame *f = NULL;
    List *context = id3v2CreateEqualizationFrameContext(ID3V2_TAG_VERSION_2);

    id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_2, &f);

    assert_non_null(f);

    size_t outl = 0;
    uint8_t *out = id3v2FrameSerialize(f, ID3V2_TAG_VERSION_2, &outl);
    rep = byteStreamCreate(out, outl);

    assert_memory_equal(byteStreamCursor(rep), "EQU", 3);
    byteStreamSeek(rep, 3, SEEK_CUR);

    assert_memory_equal(byteStreamCursor(rep), "\x00\x00\x09", 3);
    byteStreamSeek(rep, 3, SEEK_CUR);

    assert_memory_equal(byteStreamCursor(rep), "\x10", 1);
    byteStreamSeek(rep, 1, SEEK_CUR);

    assert_memory_equal(byteStreamCursor(rep), "\x03\xe9", 2);
    byteStreamSeek(rep, 2, SEEK_CUR);

    assert_memory_equal(byteStreamCursor(rep), "\x40\x00", 2);
    byteStreamSeek(rep, 2, SEEK_CUR);

    assert_memory_equal(byteStreamCursor(rep), "\x80\x28", 2);
    byteStreamSeek(rep, 2, SEEK_CUR);

    assert_memory_equal(byteStreamCursor(rep), "\xfc\x00", 2);
    byteStreamSeek(rep, 2, SEEK_CUR);

    // full round trip
    assert_int_equal(outl, 15);
    assert_memory_equal(out, equ, 15);

    free(out);
    listFree(context);
    byteStreamDestroy(stream);
    byteStreamDestroy(rep);
    id3v2DestroyFrame(&f);
}

/**
 * Parses a frame then serializes it and checks the output is byte for byte identical.
 */
static void internal_assertRoundTrip(const uint8_t *in, size_t inl, List *context, uint8_t version) {
    Id3v2Frame *f = NULL;
    size_t outl = 0;

    assert_int_equal(id3v2ParseFrame((uint8_t *) in, inl, context, version, &f), inl);
    assert_non_null(f);

    uint8_t *out = id3v2FrameSerialize(f, version, &outl);

    assert_non_null(out);
    assert_int_equal(outl, inl);
    assert_memory_equal(out, in, inl);

    free(out);
    id3v2DestroyFrame(&f);
}

static void id3v2FrameSerialize_v2RVA(void **state) {
    (void) state;
    const uint8_t rva[12] = {
        'R', 'V', 'A', 0x00, 0x00, 0x06,
        0x01, 0x08, 0x10, 0x20, 0x7f, 0x7e
    };
    List *context = id3v2CreateRelativeVolumeAdjustmentFrameContext(ID3V2_TAG_VERSION_2);

    internal_assertRoundTrip(rva, sizeof(rva), context, ID3V2_TAG_VERSION_2);
    listFree(context);
}

static void id3v2FrameSerialize_v3RVAD(void **state) {
    (void) state;
    const uint8_t all[36] = {
        'R', 'V', 'A', 'D', 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00,
        0x3f, 0x10,
        0x01, 0x00, 0x02, 0x00, 0x7f, 0xff, 0x7f, 0xfe,
        0x00, 0x10, 0x00, 0x20, 0x10, 0x00, 0x20, 0x00,
        0x00, 0x30, 0x30, 0x00,
        0x00, 0x40, 0x40, 0x00
    };
    const uint8_t frontOnly[16] = {
        'R', 'V', 'A', 'D', 0x00, 0x00, 0x00, 0x06, 0x00, 0x00,
        0x03, 0x10, 0x01, 0x00, 0x02, 0x00
    };
    // 12 bits per volume description is still 2 whole bytes
    const uint8_t twelveBits[20] = {
        'R', 'V', 'A', 'D', 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00,
        0x00, 0x0c, 0x0f, 0xff, 0x00, 0x01, 0x08, 0x00, 0x00, 0x80
    };
    List *context = id3v2CreateRelativeVolumeAdjustmentFrameContext(ID3V2_TAG_VERSION_3);

    internal_assertRoundTrip(all, sizeof(all), context, ID3V2_TAG_VERSION_3);
    internal_assertRoundTrip(frontOnly, sizeof(frontOnly), context, ID3V2_TAG_VERSION_3);
    internal_assertRoundTrip(twelveBits, sizeof(twelveBits), context, ID3V2_TAG_VERSION_3);
    listFree(context);
}

static void id3v2FrameSerialize_v3EQUA(void **state) {
    (void) state;
    const uint8_t equa[19] = {
        'E', 'Q', 'U', 'A', 0x00, 0x00, 0x00, 0x09, 0x00, 0x00,
        0x0C, 0x80, 0x64, 0x0f, 0xff, 0x01, 0x00, 0x00, 0x01
    };
    List *context = id3v2CreateEqualizationFrameContext(ID3V2_TAG_VERSION_3);

    internal_assertRoundTrip(equa, sizeof(equa), context, ID3V2_TAG_VERSION_3);
    listFree(context);
}

static void id3v2FrameSerialize_v4RVA2(void **state) {
    (void) state;
    const uint8_t perChannel[31] = {
        'R', 'V', 'A', '2', 0x00, 0x00, 0x00, 0x15, 0x00, 0x00,
        't', 'r', 'a', 'c', 'k', 0x00,
        0x01, 0x04, 0x00, 0x10, 0x7f, 0xff,
        0x02, 0xfc, 0x00, 0x00,
        0x03, 0x00, 0x01, 0x08, 0xff
    };
    // the last channel has no peak so the frame ends on the bits field
    const uint8_t lastNoPeak[16] = {
        'R', 'V', 'A', '2', 0x00, 0x00, 0x00, 0x06, 0x00, 0x00,
        'x', 0x00, 0x01, 0x04, 0x00, 0x00
    };
    List *context = id3v2CreateRelativeVolumeAdjustmentFrameContext(ID3V2_TAG_VERSION_4);

    internal_assertRoundTrip(perChannel, sizeof(perChannel), context, ID3V2_TAG_VERSION_4);
    internal_assertRoundTrip(lastNoPeak, sizeof(lastNoPeak), context, ID3V2_TAG_VERSION_4);
    listFree(context);
}

static void id3v2FrameSerialize_v4EQU2(void **state) {
    (void) state;
    const uint8_t equ2[22] = {
        'E', 'Q', 'U', '2', 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00,
        0x01, 'e', 'q', 0x00,
        0x00, 0xc8, 0x04, 0x00,
        0x07, 0xd0, 0xfc, 0x00
    };
    List *context = id3v2CreateEqualizationFrameContext(ID3V2_TAG_VERSION_4);

    internal_assertRoundTrip(equ2, sizeof(equ2), context, ID3V2_TAG_VERSION_4);
    listFree(context);
}

static void id3v2FrameSerialize_adjustmentEntrySizeMismatch(void **state) {
    (void) state;
    uint8_t rvad[16] = {
        'R', 'V', 'A', 'D', 0x00, 0x00, 0x00, 0x06, 0x00, 0x00,
        0x03, 0x10, 0x01, 0x00, 0x02, 0x00
    };
    const uint8_t expected[16] = {
        'R', 'V', 'A', 'D', 0x00, 0x00, 0x00, 0x06, 0x00, 0x00,
        0x03, 0x10, 0x00, 0xab, 0x22, 0x33
    };
    Id3v2Frame *f = NULL;
    List *context = id3v2CreateRelativeVolumeAdjustmentFrameContext(ID3V2_TAG_VERSION_3);

    id3v2ParseFrame(rvad, sizeof(rvad), context, ID3V2_TAG_VERSION_3, &f);
    assert_non_null(f);

    // heap allocated so ASan reports any over read
    uint8_t *shorter = malloc(1);
    uint8_t *longer = malloc(3);
    assert_non_null(shorter);
    assert_non_null(longer);
    shorter[0] = 0xab;
    longer[0] = 0x11;
    longer[1] = 0x22;
    longer[2] = 0x33;

    ListIter entries = id3v2CreateFrameEntryTraverser(f);
    listIteratorNext(&entries);
    listIteratorNext(&entries);
    assert_true(id3v2WriteFrameEntry(f, &entries, 1, shorter));
    listIteratorNext(&entries);
    assert_true(id3v2WriteFrameEntry(f, &entries, 3, longer));
    free(shorter);
    free(longer);

    // a 1 byte entry is padded in front, a 3 byte entry keeps its low 2 bytes
    size_t outl = 0;
    uint8_t *out = id3v2FrameSerialize(f, ID3V2_TAG_VERSION_3, &outl);

    assert_int_equal(outl, sizeof(expected));
    assert_memory_equal(out, expected, sizeof(expected));

    free(out);
    listFree(context);
    id3v2DestroyFrame(&f);
}

static void id3v2FrameToJSON_v4RVA2(void **state) {
    (void) state;
    uint8_t rva2[31] = {
        'R', 'V', 'A', '2', 0x00, 0x00, 0x00, 0x15, 0x00, 0x00,
        't', 'r', 'a', 'c', 'k', 0x00,
        0x01, 0x04, 0x00, 0x10, 0x7f, 0xff,
        0x02, 0xfc, 0x00, 0x00,
        0x03, 0x00, 0x01, 0x08, 0xff
    };
    Id3v2Frame *f = NULL;
    List *context = id3v2CreateRelativeVolumeAdjustmentFrameContext(ID3V2_TAG_VERSION_4);

    id3v2ParseFrame(rva2, sizeof(rva2), context, ID3V2_TAG_VERSION_4, &f);
    assert_non_null(f);

    char *json = id3v2FrameToJSON(f, ID3V2_TAG_VERSION_4);
    assert_non_null(json);

    // every peak is present, including the empty one, and each object is closed
    assert_non_null(strstr(json, "{\"value\":\"f/8=\",\"size\":2}"));
    assert_non_null(strstr(json, "{\"value\":\"\",\"size\":0}"));
    assert_non_null(strstr(json, "{\"value\":\"/w==\",\"size\":1}]}"));

    free(json);
    listFree(context);
    id3v2DestroyFrame(&f);
}

/**
 * Positions a fresh entry traverser on the entry at index.
 */
static ListIter internal_entryAt(Id3v2Frame *f, size_t index) {
    ListIter entries = id3v2CreateFrameEntryTraverser(f);

    for (size_t i = 0; i < index; i++) {
        listIteratorNext(&entries);
    }

    return entries;
}

static void id3v2WriteFrameEntry_iteratedEQUBand(void **state) {
    (void) state;
    uint8_t equ[15] = {
        'E', 'Q', 'U', 0x00, 0x00, 0x09,
        0x10,
        0x03, 0xe9, 0x40, 0x00,
        0x80, 0x28, 0xfc, 0x00
    };
    // band 2 moves to 256Hz (increment bit kept) with a new adjustment
    const uint8_t expected[15] = {
        'E', 'Q', 'U', 0x00, 0x00, 0x09,
        0x10,
        0x03, 0xe9, 0x40, 0x00,
        0x81, 0x00, 0x12, 0x34
    };
    const uint8_t frequency[2] = {0x01, 0x00};
    const uint8_t volume[2] = {0x12, 0x34};
    Id3v2Frame *f = NULL;
    List *context = id3v2CreateEqualizationFrameContext(ID3V2_TAG_VERSION_2);

    id3v2ParseFrame(equ, sizeof(equ), context, ID3V2_TAG_VERSION_2, &f);
    assert_non_null(f);

    // entries: adjustment, then unary/frequency/volume per band
    ListIter entries = internal_entryAt(f, 5);
    assert_true(id3v2WriteFrameEntry(f, &entries, sizeof(frequency), frequency));
    entries = internal_entryAt(f, 6);
    assert_true(id3v2WriteFrameEntry(f, &entries, sizeof(volume), volume));

    size_t outl = 0;
    uint8_t *out = id3v2FrameSerialize(f, ID3V2_TAG_VERSION_2, &outl);

    assert_int_equal(outl, sizeof(expected));
    assert_memory_equal(out, expected, sizeof(expected));

    free(out);
    listFree(context);
    id3v2DestroyFrame(&f);
}

static void id3v2WriteFrameEntry_iteratedRVA2Channel(void **state) {
    (void) state;
    uint8_t rva2[31] = {
        'R', 'V', 'A', '2', 0x00, 0x00, 0x00, 0x15, 0x00, 0x00,
        't', 'r', 'a', 'c', 'k', 0x00,
        0x01, 0x04, 0x00, 0x10, 0x7f, 0xff,
        0x02, 0xfc, 0x00, 0x00,
        0x03, 0x00, 0x01, 0x08, 0xff
    };
    const uint8_t expected[31] = {
        'R', 'V', 'A', '2', 0x00, 0x00, 0x00, 0x15, 0x00, 0x00,
        't', 'r', 'a', 'c', 'k', 0x00,
        0x01, 0x04, 0x00, 0x10, 0x7f, 0xff,
        0x04, 0xfc, 0x00, 0x00,
        0x03, 0xfe, 0x00, 0x08, 0x34
    };
    const uint8_t channel = 0x04;
    const uint8_t volume[2] = {0xfe, 0x00};
    const uint8_t peak[2] = {0x12, 0x34};
    Id3v2Frame *f = NULL;
    List *context = id3v2CreateRelativeVolumeAdjustmentFrameContext(ID3V2_TAG_VERSION_4);

    id3v2ParseFrame(rva2, sizeof(rva2), context, ID3V2_TAG_VERSION_4, &f);
    assert_non_null(f);

    // entries: identifier, then channel/volume/bits/peak per channel
    ListIter entries = internal_entryAt(f, 5);
    assert_true(id3v2WriteFrameEntry(f, &entries, 1, &channel));

    // channel 2 declares a 0 bit peak, there is nothing to write into
    entries = internal_entryAt(f, 8);
    assert_false(id3v2WriteFrameEntry(f, &entries, sizeof(peak), peak));

    entries = internal_entryAt(f, 10);
    assert_true(id3v2WriteFrameEntry(f, &entries, sizeof(volume), volume));

    // channel 3 declares an 8 bit peak so only the low byte is kept
    entries = internal_entryAt(f, 12);
    assert_true(id3v2WriteFrameEntry(f, &entries, sizeof(peak), peak));
    assert_int_equal(((Id3v2ContentEntry *) f->entries->tail->data)->size, 1);

    size_t outl = 0;
    uint8_t *out = id3v2FrameSerialize(f, ID3V2_TAG_VERSION_4, &outl);

    assert_int_equal(outl, sizeof(expected));
    assert_memory_equal(out, expected, sizeof(expected));

    free(out);
    listFree(context);
    id3v2DestroyFrame(&f);
}

static void id3v2WriteFrameEntry_iteratedSYLTStamp(void **state) {
    (void) state;
    uint8_t sylt[30] = {
        'S', 'Y', 'L', 'T', 0x00, 0x00, 0x00, 0x14, 0x00, 0x00,
        0x00, 'e', 'n', 'g', 0x02, 0x01,
        'd', 0x00,
        'a', 0x00, 0x00, 0x00, 0x00, 0x10,
        'b', 0x00, 0x00, 0x00, 0x00, 0x20
    };
    const uint8_t stamp[4] = {0x00, 0x00, 0x01, 0x00};
    Id3v2Frame *f = NULL;
    List *context = id3v2CreateSynchronisedLyricFrameContext();

    assert_int_equal(id3v2ParseFrame(sylt, sizeof(sylt), context, ID3V2_TAG_VERSION_3, &f), sizeof(sylt));
    assert_non_null(f);
    assert_int_equal(listLength(f->entries), 9);

    // entries: encoding, language, format, symbol, desc, then text/stamp per line
    ListIter entries = internal_entryAt(f, 8);
    assert_true(id3v2WriteFrameEntry(f, &entries, sizeof(stamp), stamp));

    size_t outl = 0;
    uint8_t *out = id3v2FrameSerialize(f, ID3V2_TAG_VERSION_3, &outl);

    assert_int_equal(outl, sizeof(sylt));
    assert_memory_equal(out, sylt, sizeof(sylt) - 4);
    assert_memory_equal(out + sizeof(sylt) - 4, stamp, 4);

    free(out);
    listFree(context);
    id3v2DestroyFrame(&f);
}

static void id3v2FrameSerialize_v3TXXX(void **state) {
    (void) state;
    ByteStream *stream = byteStreamFromFile("assets/sorry4dying.mp3");
    ByteStream *rep = NULL;

    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);
    ListIter iter = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    int c = 0;

    while ((f = id3v2FrameTraverse(&iter)) != NULL) {
        if (memcmp(f->header->id, "TXXX", ID3V2_FRAME_ID_MAX_SIZE) == 0) {
            if (c == 1) {
                size_t outl = 0;
                uint8_t *out = id3v2FrameSerialize(f, ID3V2_TAG_VERSION_3, &outl);
                rep = byteStreamCreate(out, outl);
                free(out);

                break;
            }
            c++;
        }
    }

    assert_memory_equal("TXXX", byteStreamCursor(rep), 4);
    byteStreamSeek(rep, 4, SEEK_CUR);

    assert_memory_equal("\x00\x00\x00\x27", byteStreamCursor(rep), 4);
    byteStreamSeek(rep, 4, SEEK_CUR);

    assert_memory_equal("\x00\x00", byteStreamCursor(rep), 2);
    byteStreamSeek(rep, 2, SEEK_CUR);

    assert_memory_equal("\x01", byteStreamCursor(rep), 1);
    byteStreamSeek(rep, 1, SEEK_CUR);

    assert_memory_equal("\xff\xfe\x50\x00\x45\x00\x52\x00\x46\x00\x4f\x00\x52\x00\x4d\x00\x45\x00\x52\x00",
                        byteStreamCursor(rep), 20);
    byteStreamSeek(rep, 20, SEEK_CUR);

    assert_memory_equal("\x00\x00", byteStreamCursor(rep), 2);
    byteStreamSeek(rep, 2, SEEK_CUR);

    assert_memory_equal("\xff\xfe\x51\x00\x75\x00\x61\x00\x64\x00\x65\x00\x63\x00\x61\x00", byteStreamCursor(rep), 16);
    byteStreamSeek(rep, 16, SEEK_CUR);

    byteStreamDestroy(rep);
    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);
}

static void id3v2FrameToJSON_v3TXXX(void **state) {
    (void) state;
    ByteStream *stream = byteStreamFromFile("assets/sorry4dying.mp3");
    char *json = NULL;

    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);
    ListIter iter = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    int c = 0;

    while ((f = id3v2FrameTraverse(&iter)) != NULL) {
        if (memcmp(f->header->id, "TXXX", ID3V2_FRAME_ID_MAX_SIZE) == 0) {
            if (c == 1) {
                json = id3v2FrameToJSON(f, ID3V2_TAG_VERSION_3);
                break;
            }
            c++;
        }
    }

    assert_string_equal(json,
                        "{\"header\":{\"id\":\"TXXX\",\"tagAlterPreservation\":false,\"fileAlterPreservation\":false,\"readOnly\":false,\"decompressionSize\":0,\"encryptionSymbol\":0,\"groupSymbol\":0},\"content\":[{\"value\":\"1\",\"size\":1},{\"value\":\"PERFORMER\",\"size\":9},{\"value\":\"Quadeca\",\"size\":7}]}");

    free(json);
    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);
}

static void id3v2FrameToJSON_v3APIC(void **state) {
    (void) state;
    ByteStream *stream = byteStreamFromFile("assets/sorry4dying.mp3");
    char *json = NULL;

    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);
    ListIter iter = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    while ((f = id3v2FrameTraverse(&iter)) != NULL) {
        if (memcmp(f->header->id, "APIC", ID3V2_FRAME_ID_MAX_SIZE) == 0) {
            json = id3v2FrameToJSON(f, ID3V2_TAG_VERSION_3);
            break;
        }
    }

    assert_non_null(json);
    assert_memory_equal(
        json,
        "{\"header\":{\"id\":\"APIC\",\"tagAlterPreservation\":false,\"fileAlterPreservation\":false,\"readOnly\":false,\"decompressionSize\":0,\"encryptionSymbol\":0,\"groupSymbol\":0},\"content\":[{\"value\":\"image/jpeg\",\"size\":10},{\"value\":\"\",",
        144);

    /**
     * vscode actually crashes when i try and compare the json string so i cant do it?
     */

    free(json);
    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);
}

static void id3v2FrameToJSON_v4ETCO(void **state) {
    (void) state;
    ByteStream *stream = byteStreamFromFile("assets/OnGP.mp3");
    char *json = NULL;

    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);
    ListIter iter = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    while ((f = id3v2FrameTraverse(&iter)) != NULL) {
        if (memcmp(f->header->id, "ETCO", ID3V2_FRAME_ID_MAX_SIZE) == 0) {
            json = id3v2FrameToJSON(f, ID3V2_TAG_VERSION_4);
            break;
        }
    }


    assert_non_null(json);
    assert_string_equal(
        json,
        "{\"header\":{\"id\":\"ETCO\",\"tagAlterPreservation\":false,\"fileAlterPreservation\":false,\"readOnly\":false,\"unsynchronisation\":false,\"decompressionSize\":0,\"encryptionSymbol\":0,\"groupSymbol\":0},\"content\":[{\"value\":\"2\",\"size\":1},{\"value\":\"6\",\"size\":1},{\"value\":\"1220000\",\"size\":4},{\"value\":\"2\",\"size\":1},{\"value\":\"610000\",\"size\":4}]}");

    free(json);
    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);
}

static void id3v2CreateEmptyFrame_noID(void **state) {
    (void) state;
    Id3v2Frame *f = id3v2CreateEmptyFrame(NULL, ID3V2_TAG_VERSION_4, NULL);
    assert_null(f);
}

static void id3v2CreateEmptyFrame_TT2(void **state) {
    (void) state;
    Id3v2Frame *f = id3v2CreateEmptyFrame("TT2", ID3V2_TAG_VERSION_2, NULL);
    assert_non_null(f);

    assert_memory_equal(f->header->id, "TT2", 3);
    assert_int_equal(f->contexts->length, 2);
    assert_int_equal(f->entries->length, 2);

    Id3v2ContentContext *cc = NULL;
    ListIter i = {0};
    int c = 0;

    while ((cc = listIteratorNext(&i)) != NULL) {
        if (c == 0) {
            assert_int_equal(cc->type, numeric_context);
        } else if (c == 1) {
            assert_int_equal(cc->type, encodedString_context);
        }

        c++;
    }

    i = id3v2CreateFrameEntryTraverser(f);

    assert_int_equal(id3v2ReadFrameEntryAsU8(&i), 0);
    assert_int_equal(id3v2ReadFrameEntryAsU8(&i), 0);

    id3v2DestroyFrame(&f);
}


static void id3v2CompareFrameId_badArgs(void **state) {
    (void) state;
    char *id = "TIT2";
    bool v = id3v2CompareFrameId(NULL, id);

    assert_false(v);
}

static void id3v2CompareFrameId_EQU(void **state) {
    (void) state;
    // EQU
    uint8_t equ[15] = {
        'E', 'Q', 'U', 0x00, 0x00, 0x09,
        0x10, // 16 adjustment bits -> 2 byte volumes
        0x03, 0xe9, // 000000111110100 1  inc/dec
        0x40, 0x00,
        0x00, 0x28, // 000000000010100 0  inc/dec
        0xfc, 0x00
    };


    ByteStream *stream = byteStreamCreate(equ, 15);
    Id3v2Frame *f = NULL;
    List *context = id3v2CreateEqualizationFrameContext(ID3V2_TAG_VERSION_2);

    id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_2, &f);

    byteStreamDestroy(stream);
    listFree(context);
    assert_true(id3v2CompareFrameId(f, "EQU"));
    id3v2DestroyFrame(&f);
}

static void internal_unsyncCase(const uint8_t *in, size_t inl, const uint8_t *expected, size_t expectedl) {
    size_t outl = 0;
    uint8_t *out = id3v2ApplyUnsynchronisation(in, inl, &outl);

    assert_non_null(out);
    assert_int_equal(outl, expectedl);
    assert_memory_equal(out, expected, expectedl);

    // decoding must give back the original
    assert_int_equal(id3v2RemoveUnsynchronisation(out, outl), inl);
    assert_memory_equal(out, in, inl);
    free(out);
}

static void id3v2ApplyUnsynchronisation_noSync(void **state) {
    (void) state;
    const uint8_t in[] = {0x00, 0x01, 0xFE, 0xDF, 'a'};
    internal_unsyncCase(in, sizeof(in), in, sizeof(in));
}

static void id3v2ApplyUnsynchronisation_falseSyncs(void **state) {
    (void) state;
    // $FF $E0, $FF $00 and $FF $FF all need a $00; $FF $DF and $FF $12 do not
    const uint8_t in[] = {0xFF, 0xE0, 0xFF, 0x00, 0xFF, 0xDF, 0xFF, 0xFF, 0x12};
    const uint8_t expected[] = {0xFF, 0x00, 0xE0, 0xFF, 0x00, 0x00, 0xFF, 0xDF, 0xFF, 0x00, 0xFF, 0x12};
    internal_unsyncCase(in, sizeof(in), expected, sizeof(expected));
}

static void id3v2ApplyUnsynchronisation_trailingFF(void **state) {
    (void) state;
    const uint8_t in[] = {'a', 0xFF};
    const uint8_t expected[] = {'a', 0xFF, 0x00};
    internal_unsyncCase(in, sizeof(in), expected, sizeof(expected));
}

static void id3v2ApplyUnsynchronisation_allFF(void **state) {
    (void) state;
    uint8_t in[64];
    uint8_t expected[128];

    memset(in, 0xFF, sizeof(in));
    for (size_t i = 0; i < sizeof(expected); i += 2) {
        expected[i] = 0xFF;
        expected[i + 1] = 0x00;
    }

    internal_unsyncCase(in, sizeof(in), expected, sizeof(expected));
}

static void id3v2ApplyUnsynchronisation_badArgs(void **state) {
    (void) state;
    const uint8_t in[] = {0xFF};
    size_t outl = 1;

    assert_null(id3v2ApplyUnsynchronisation(NULL, 1, &outl));
    assert_int_equal(outl, 0);
    assert_null(id3v2ApplyUnsynchronisation(in, 0, &outl));
    assert_null(id3v2ApplyUnsynchronisation(in, 1, NULL));
}

static void id3v2RemoveUnsynchronisation_lenientInput(void **state) {
    (void) state;
    // a lone $FF $E0 (not properly unsynchronised) and a trailing $FF are kept as-is
    uint8_t buf[] = {0xFF, 0xE0, 0xFF, 0x00, 0x00, 0xFF};
    const uint8_t expected[] = {0xFF, 0xE0, 0xFF, 0x00, 0xFF};

    assert_int_equal(id3v2RemoveUnsynchronisation(buf, sizeof(buf)), sizeof(expected));
    assert_memory_equal(buf, expected, sizeof(expected));
    assert_int_equal(id3v2RemoveUnsynchronisation(NULL, 4), 0);
}

int main() {
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
        cmocka_unit_test(id3v2WriteFrameEntry_padToMin),
        cmocka_unit_test(id3v2ReadFrameEntryAsU16U32_bigEndian),
        cmocka_unit_test(id3v2WriteFrameEntry_updateTitle),
        cmocka_unit_test(id3v2AttachFrameFromTag_TSOA),
        cmocka_unit_test(id3v2DetachFrameFromTag_TIT2),

        // id3v2FrameHeaderSerialize
        cmocka_unit_test(id3v2FrameHeaderSerialize_null),
        cmocka_unit_test(id3v2FrameHeaderSerialize_v2),
        cmocka_unit_test(id3v2FrameHeaderSerialize_v3),
        cmocka_unit_test(id3v2FrameHeaderSerialize_v3AllFlags),
        cmocka_unit_test(id3v2FrameHeaderSerialize_v3OnlySymbols),
        cmocka_unit_test(id3v2FrameHeaderSerialize_v4),
        cmocka_unit_test(id3v2FrameHeaderSerialize_v4AllFlags),
        cmocka_unit_test(id3v2FrameHeaderSerialize_v4NoUnsync),

        // id3v2FrameHeaderToJSON
        cmocka_unit_test(id3v2FrameHeaderToJSON_null),
        cmocka_unit_test(id3v2FrameHeaderToJSON_v2),
        cmocka_unit_test(id3v2FrameHeaderToJSON_v3),
        cmocka_unit_test(id3v2FrameHeaderToJSON_v3Symbol),
        cmocka_unit_test(id3v2FrameHeaderToJSON_v4WithUnsync),

        // id3v2FrameSerialize
        cmocka_unit_test(id3v2FrameSerialize_v4TALB),
        cmocka_unit_test(id3v2FrameSerialize_v4WCOM),
        cmocka_unit_test(id3v2FrameSerialize_v4ETCO),
        cmocka_unit_test(id3v2FrameSerialize_v2EQU),
        cmocka_unit_test(id3v2FrameSerialize_v2RVA),
        cmocka_unit_test(id3v2FrameSerialize_v3RVAD),
        cmocka_unit_test(id3v2FrameSerialize_v3EQUA),
        cmocka_unit_test(id3v2FrameSerialize_v4RVA2),
        cmocka_unit_test(id3v2FrameSerialize_v4EQU2),
        cmocka_unit_test(id3v2FrameSerialize_adjustmentEntrySizeMismatch),
        cmocka_unit_test(id3v2WriteFrameEntry_iteratedEQUBand),
        cmocka_unit_test(id3v2WriteFrameEntry_iteratedRVA2Channel),
        cmocka_unit_test(id3v2WriteFrameEntry_iteratedSYLTStamp),
        cmocka_unit_test(id3v2FrameSerialize_v3TXXX),

        // id3v2FrameToJSON
        cmocka_unit_test(id3v2FrameToJSON_v3TXXX),
        cmocka_unit_test(id3v2FrameToJSON_v3APIC),
        cmocka_unit_test(id3v2FrameToJSON_v4ETCO),
        cmocka_unit_test(id3v2FrameToJSON_v4RVA2),

        // unsynchronisation
        cmocka_unit_test(id3v2ApplyUnsynchronisation_noSync),
        cmocka_unit_test(id3v2ApplyUnsynchronisation_falseSyncs),
        cmocka_unit_test(id3v2ApplyUnsynchronisation_trailingFF),
        cmocka_unit_test(id3v2ApplyUnsynchronisation_allFF),
        cmocka_unit_test(id3v2ApplyUnsynchronisation_badArgs),
        cmocka_unit_test(id3v2RemoveUnsynchronisation_lenientInput),

        // id3v2CreateEmptyFrame
        cmocka_unit_test(id3v2CreateEmptyFrame_noID),
        cmocka_unit_test(id3v2CreateEmptyFrame_TT2),

        // id3v2CompareFrameId
        cmocka_unit_test(id3v2CompareFrameId_badArgs),
        cmocka_unit_test(id3v2CompareFrameId_EQU)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
