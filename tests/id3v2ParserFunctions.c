/**
 * @file id3v2ParserFunctions.c
 * @author Ewan Jones
 * @brief unit tests for id3v2Parser.c
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
#include "id3v2/id3v2Parser.h"
#include "id3v2/id3v2TagIdentity.h"
#include "id3v2/id3v2Frame.h"
#include "id3v2/id3v2Context.h"
#include "byteStream.h"
#include "byteInt.h"

static void testFrameHeader(Id3v2Frame *f, char id[ID3V2_FRAME_ID_MAX_SIZE], uint8_t u, 
                            uint8_t ro, uint8_t tap, uint8_t fap, uint32_t ds, uint8_t gs, 
                            uint8_t es){

    assert_non_null(f);
    assert_non_null(f->header);
    assert_memory_equal(f->header->id,id,4);
    assert_int_equal(f->header->unsynchronisation, u);
    assert_int_equal(f->header->readOnly, ro);
    assert_int_equal(f->header->tagAlterPreservation, tap);
    assert_int_equal(f->header->fileAlterPreservation, fap);
    assert_int_equal(f->header->decompressionSize, ds);
    assert_int_equal(f->header->groupSymbol, gs);
    assert_int_equal(f->header->encryptionSymbol, es);

}

static void testEntry(Id3v2ContentEntry *ce, size_t size, uint8_t *data){

    assert_non_null(ce);
    assert_int_equal(ce->size, size);
    assert_memory_equal(ce->entry, data, size);

}




static void id3v2ParseExtendedTagHeader_nullData(void **state){
    
    Id3v2ExtendedTagHeader *h;
    
    uint32_t v = id3v2ParseExtendedTagHeader(NULL, 0, ID3V2_TAG_VERSION_2, &h);
    
    assert_int_equal(v, 0),
    assert_null(h);
}

static void id3v2ParseExtendedTagHeader_v2(void **state){
    
    ByteStream *stream = byteStreamCreate(NULL, 1);
    
    Id3v2ExtendedTagHeader *h;
    
    uint32_t v = id3v2ParseExtendedTagHeader(stream->buffer, 0, ID3V2_TAG_VERSION_2, &h);

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
    uint32_t v = id3v2ParseExtendedTagHeader(stream->buffer, stream->bufferSize, ID3V2_TAG_VERSION_3, &h);

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
    
    uint32_t v = id3v2ParseExtendedTagHeader(stream->buffer, stream->bufferSize, ID3V2_TAG_VERSION_3, &h);

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
    
    uint32_t v = id3v2ParseExtendedTagHeader(stream->buffer, stream->bufferSize, ID3V2_TAG_VERSION_3, &h);

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
    uint32_t v = id3v2ParseExtendedTagHeader(stream->buffer, stream->bufferSize, ID3V2_TAG_VERSION_3, &h);

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
    uint32_t v = id3v2ParseExtendedTagHeader(stream->buffer, stream->bufferSize, ID3V2_TAG_VERSION_3, &h);

    assert_int_equal(v, 6);

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
    uint32_t v = id3v2ParseExtendedTagHeader(stream->buffer, stream->bufferSize, ID3V2_TAG_VERSION_4, &h);

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
    uint32_t v = id3v2ParseExtendedTagHeader(stream->buffer, stream->bufferSize, ID3V2_TAG_VERSION_4, &h);

    assert_int_equal(v, 11);
    assert_int_equal(h->crc, 8008);
    assert_true(h->update);
    assert_true(h->tagRestrictions);
    assert_int_equal(h->restrictions, 0);

    byteStreamDestroy(stream);
    id3v2DestroyExtendedTagHeader(&h);
}

static void id3v2ParseExtendedTagHeader_v4NoCRC(void **state){
    
    uint8_t ext[6] = {0,0,0,6,
                      0,
                      0x40};
                       

    ByteStream *stream = byteStreamCreate(ext, 6);

    Id3v2ExtendedTagHeader *h;
    uint32_t v = id3v2ParseExtendedTagHeader(stream->buffer, stream->bufferSize, ID3V2_TAG_VERSION_4, &h);
    
    assert_int_equal(v, 6);
    assert_int_equal(h->crc, 0);
    assert_true(h->update);
    assert_false(h->tagRestrictions);
    assert_int_equal(h->restrictions, 0);

    byteStreamDestroy(stream);
    id3v2DestroyExtendedTagHeader(&h);
}

static void id3v2ParseTagHeader_happyPath(void **state){
    
    uint8_t ext[10] = {'I','D','3',
                      2, 
                      0,
                      0,
                      0,0x72,0x6C,0x2E};
                       

    ByteStream *stream = byteStreamCreate(ext, 10);

    Id3v2TagHeader *h;
    uint32_t size = 0;

    uint32_t v = id3v2ParseTagHeader(stream->buffer, stream->bufferSize, &h, &size);
    assert_int_equal(stream->cursor, 0);
    assert_int_equal(v,10);
    assert_non_null(h);
    assert_int_equal(h->majorVersion, 2);
    assert_int_equal(h->minorVersion, 0);
    assert_int_equal(h->flags, 0);
    assert_int_equal(h->extendedHeader, NULL);
    assert_int_equal(size, byteSyncintDecode(0x726C2E));

    id3v2DestroyTagHeader(&h);
    byteStreamDestroy(stream);
}

static void id3v2ParseTagHeader_noTagSize(void **state){
    
    uint8_t ext[6] = {'I','D','3',
                      2, 
                      0,
                      0};
                       

    ByteStream *stream = byteStreamCreate(ext, 6);

    Id3v2TagHeader *h;
    uint32_t size = 0;

    uint32_t v = id3v2ParseTagHeader(stream->buffer, stream->bufferSize, &h, &size);
    assert_int_equal(stream->cursor, 0);
    assert_int_equal(v,6);
    assert_non_null(h);
    assert_int_equal(h->majorVersion, 2);
    assert_int_equal(h->minorVersion, 0);
    assert_int_equal(h->flags, 0);
    assert_int_equal(h->extendedHeader, NULL);
    assert_int_equal(size, 0);

    id3v2DestroyTagHeader(&h);
    byteStreamDestroy(stream);
}

static void id3v2ParseTagHeader_noFlags(void **state){
    
    uint8_t ext[5] = {'I','D','3',
                      2, 
                      0};
                       

    ByteStream *stream = byteStreamCreate(ext,5);

    Id3v2TagHeader *h;
    uint32_t size = 0;

    uint32_t v = id3v2ParseTagHeader(stream->buffer, stream->bufferSize, &h, &size);
    assert_int_equal(stream->cursor, 0);
    assert_int_equal(v,5);
    assert_non_null(h);
    assert_int_equal(h->majorVersion, 2);
    assert_int_equal(h->minorVersion, 0);
    assert_int_equal(h->flags, 0);
    assert_int_equal(h->extendedHeader, NULL);
    assert_int_equal(size, 0);

    id3v2DestroyTagHeader(&h);
    byteStreamDestroy(stream);
}

static void id3v2ParseTagHeader_noVersions(void **state){
    
    uint8_t ext[3] = {'I','D','3'};
                       

    ByteStream *stream = byteStreamCreate(ext,3);
    Id3v2TagHeader *h;
    uint32_t size = 0;

    uint32_t v = id3v2ParseTagHeader(stream->buffer, stream->bufferSize, &h, &size);
    assert_int_equal(stream->cursor, 0);
    assert_int_equal(v,3);
    assert_non_null(h);
    assert_int_equal(h->majorVersion, 0);
    assert_int_equal(h->minorVersion, 0);
    assert_int_equal(h->flags, 0);
    assert_int_equal(h->extendedHeader, NULL);
    assert_int_equal(size, 0);

    id3v2DestroyTagHeader(&h);
    byteStreamDestroy(stream);
}

static void id3v2ParseFrameHeader_noSupport(void **state){

    uint8_t ext[6] = {'T','A','L',
                      0x00,0x01,0x00,
    };

    ByteStream *stream = byteStreamCreate(ext, 6);
    Id3v2FrameHeader *h;
    uint32_t size = 0;
    uint32_t v = id3v2ParseFrameHeader(stream->buffer, stream->bufferSize, 99, &h, &size);

    assert_null(h);
    assert_int_equal(v, 0);
    assert_int_equal(size, 0);


    byteStreamDestroy(stream);

}


static void id3v2ParseFrameHeader_v2(void **state){

    uint8_t ext[6] = {'T','A','L',
                      0x00,0x01,0x00,
    };

    ByteStream *stream = byteStreamCreate(ext, 6);
    Id3v2FrameHeader *h;
    uint32_t size = 0;
    uint32_t v = id3v2ParseFrameHeader(stream->buffer, stream->bufferSize, ID3V2_TAG_VERSION_2, &h, &size);

    assert_non_null(h);
    assert_memory_equal(h->id, "TAL", 3);
    assert_int_equal(size, 256);
    assert_int_equal(v, 6);


    byteStreamDestroy(stream);
    id3v2DestroyFrameHeader(&h);

}

static void id3v2ParseFrameHeader_v2MissingSize(void **state){

    uint8_t ext[3] = {'T','A','L'};

    ByteStream *stream = byteStreamCreate(ext, 3);
    Id3v2FrameHeader *h;
    uint32_t size = 0;
    uint32_t v = id3v2ParseFrameHeader(stream->buffer, stream->bufferSize, ID3V2_TAG_VERSION_2, &h, &size);

    assert_null(h);
    assert_int_equal(v, 3);


    byteStreamDestroy(stream);

}

static void id3v2ParseFrameHeader_v3(void **state){

    uint8_t ext[16] = {'T','A','L','B',
                      0x00,0x00,0x00,0x64,
                      0xE0,0xE0,
                      0x00,0x00,0xEA,0x60,
                      0xFF,
                      0xFE
    };

    ByteStream *stream = byteStreamCreate(ext, 16);
    Id3v2FrameHeader *h;
    uint32_t size = 0;
    uint32_t v = id3v2ParseFrameHeader(stream->buffer, stream->bufferSize, ID3V2_TAG_VERSION_3, &h, &size);

    assert_non_null(h);
    assert_int_equal(v, 16);
    assert_int_equal(size, 100);

    assert_true(h->tagAlterPreservation);
    assert_true(h->fileAlterPreservation);
    assert_true(h->readOnly);

    assert_int_equal(h->decompressionSize, 0xEA60);
    assert_int_equal(h->encryptionSymbol, 0xFF);
    assert_int_equal(h->groupSymbol, 0xFE);


    byteStreamDestroy(stream);
    id3v2DestroyFrameHeader(&h);

}

static void id3v2ParseFrameHeader_v3FlagsButNoSymbols(void **state){

    uint8_t ext[14] = {'T','A','L','B',
                      0x00,0x00,0x00,0x64,
                      0xE0,0xE0,
                      0x00,0x00,0xEA,0x60
    };

    ByteStream *stream = byteStreamCreate(ext, 14);
    Id3v2FrameHeader *h;
    uint32_t size = 0;
    uint32_t v = id3v2ParseFrameHeader(stream->buffer, stream->bufferSize, ID3V2_TAG_VERSION_3, &h, &size);

    assert_non_null(h);
    assert_int_equal(v, 14);
    assert_int_equal(size, 100);

    assert_true(h->tagAlterPreservation);
    assert_true(h->fileAlterPreservation);
    assert_true(h->readOnly);

    assert_int_equal(h->decompressionSize, 0xEA60);
    assert_int_equal(h->encryptionSymbol, 0);
    assert_int_equal(h->groupSymbol, 0);


    byteStreamDestroy(stream);
    id3v2DestroyFrameHeader(&h);

}

static void id3v2ParseFrameHeader_v3noFlags(void **state){

    uint8_t ext[10] = {'T','A','L','B',
                      0x00,0x00,0x00,0x64,
                      0x00,0x00
    };

    ByteStream *stream = byteStreamCreate(ext, 10);
    Id3v2FrameHeader *h;
    uint32_t size = 0;
    uint32_t v = id3v2ParseFrameHeader(stream->buffer, stream->bufferSize, ID3V2_TAG_VERSION_3, &h, &size);

    assert_non_null(h);
    assert_int_equal(v, 10);
    assert_int_equal(size, 100);

    assert_false(h->tagAlterPreservation);
    assert_false(h->fileAlterPreservation);
    assert_false(h->readOnly);

    assert_int_equal(h->decompressionSize, 0);
    assert_int_equal(h->encryptionSymbol, 0);
    assert_int_equal(h->groupSymbol, 0);


    byteStreamDestroy(stream);
    id3v2DestroyFrameHeader(&h);

}

static void id3v2ParseFrameHeader_v3noFlagBytes(void **state){

    uint8_t ext[8] = {'T','A','L','B',
                      0x00,0x00,0x00,0x64
    };

    ByteStream *stream = byteStreamCreate(ext, 8);
    Id3v2FrameHeader *h;
    uint32_t size = 0;
    uint32_t v = id3v2ParseFrameHeader(stream->buffer, stream->bufferSize, ID3V2_TAG_VERSION_3, &h, &size);

    assert_null(h);
    assert_int_equal(v, 8);
    assert_int_equal(size, 100);


    byteStreamDestroy(stream);
    id3v2DestroyFrameHeader(&h);

}

static void id3v2ParseFrameHeader_v4(void **state){

    uint8_t ext[16] = {'T','I','T','2',
                      0x00,0x00,0x02,0x00,
                      0x70,0x4F,
                      0xFF,
                      0xFE,
                      0x00, 0x01, 0x0F, 0x2C


    };

    ByteStream *stream = byteStreamCreate(ext, 16);
    Id3v2FrameHeader *h;
    uint32_t size = 0;
    uint32_t v = id3v2ParseFrameHeader(stream->buffer, stream->bufferSize, ID3V2_TAG_VERSION_4, &h, &size);

    assert_non_null(h);
    assert_int_equal(v, 16);
    assert_int_equal(size, 256);

    assert_true(h->tagAlterPreservation);
    assert_true(h->fileAlterPreservation);
    assert_true(h->readOnly);

    assert_int_equal(h->groupSymbol, 0xFF);
    assert_int_equal(h->encryptionSymbol, 0xFE);
    assert_int_equal(h->decompressionSize, byteSyncintDecode(69420));
    assert_true(h->unsynchronisation);


    byteStreamDestroy(stream);
    id3v2DestroyFrameHeader(&h);

}

static void id3v2ParseFrameHeader_v4SetFlagButNoContent(void **state){

    uint8_t ext[12] = {'T','I','T','2',
                      0x00,0x00,0x02,0x00,
                      0x70,0x4F,
                      0xFF,
                      0xFE


    };

    ByteStream *stream = byteStreamCreate(ext, 12);
    Id3v2FrameHeader *h;
    uint32_t size = 0;
    uint32_t v = id3v2ParseFrameHeader(stream->buffer, stream->bufferSize, ID3V2_TAG_VERSION_4, &h, &size);

    assert_non_null(h);
    assert_int_equal(v, 12);
    assert_int_equal(size, 256);

    assert_true(h->tagAlterPreservation);
    assert_true(h->fileAlterPreservation);
    assert_true(h->readOnly);

    assert_int_equal(h->groupSymbol, 0xFF);
    assert_int_equal(h->encryptionSymbol, 0xFE);
    assert_int_equal(h->decompressionSize, 0);
    assert_true(h->unsynchronisation);


    byteStreamDestroy(stream);
    id3v2DestroyFrameHeader(&h);

}

static void id3v2ParseFrameHeader_v4NoSetFlagsButContent(void **state){

    uint8_t ext[16] = {'T','I','T','2',
                      0x00,0x00,0x02,0x00,
                      0x00,0x00,
                      0xFF,
                      0xFE,
                      0x00, 0x01, 0x0F, 0x2C


    };

    ByteStream *stream = byteStreamCreate(ext, 16);
    Id3v2FrameHeader *h;
    uint32_t size = 0;
    uint32_t v = id3v2ParseFrameHeader(stream->buffer, stream->bufferSize, ID3V2_TAG_VERSION_4, &h, &size);

    assert_non_null(h);
    assert_int_equal(v, 10);
    assert_int_equal(size, 256);

    assert_false(h->tagAlterPreservation);
    assert_false(h->fileAlterPreservation);
    assert_false(h->readOnly);

    assert_int_equal(h->groupSymbol, 0);
    assert_int_equal(h->encryptionSymbol, 0);
    assert_int_equal(h->decompressionSize, 0);
    assert_false(h->unsynchronisation);


    byteStreamDestroy(stream);
    id3v2DestroyFrameHeader(&h);

}

static void id3v2ParseFrame_parseTALBUTF8(void **state){

    uint8_t talb[77] = {0x54, 0x41, 0x4c, 0x42, 0x00, 0x00, 0x00, 0x43, 0x00, 0x00, 
                        0x03, 0x54, 0x68, 0x65, 0x20, 0x50, 0x6f, 0x77, 0x65, 0x72, 
                        0x73, 0x20, 0x54, 0x68, 0x61, 0x74, 0x20, 0x42, 0x75, 0x74, 
                        0x66, 0x38, 0xc3, 0x9b, 0xc8, 0xbe, 0xe2, 0x84, 0xb2, 0xe2, 
                        0x85, 0xa7, 0xe2, 0x99, 0x88, 0x20, 0xe2, 0x99, 0x89, 0x20, 
                        0xe2, 0x99, 0x8a, 0x20, 0xe2, 0x99, 0x8b, 0x20, 0xe2, 0x99, 
                        0x8c, 0x20, 0xe2, 0x99, 0x8d, 0x20, 0xe2, 0x99, 0x8e, 0x20, 
                        0xe2, 0x99, 0x8f, 0x75, 0x74, 0x66, 0x38
    };


    ByteStream *stream = byteStreamCreate(talb, 77);
    Id3v2Frame *f;
    List *context = id3v2CreateTextFrameContext();
    uint32_t frameSize = 0;

    frameSize = id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_4, &f);


    assert_non_null(f);
    assert_non_null(f->header);
    assert_memory_equal(f->header->id,"TALB",4);
    assert_int_equal(frameSize, 77);
    assert_false(f->header->unsynchronisation);
    assert_false(f->header->readOnly);
    assert_false(f->header->tagAlterPreservation);
    assert_false(f->header->fileAlterPreservation);

    assert_int_equal(f->header->decompressionSize, 0);
    assert_int_equal(f->header->groupSymbol, 0);
    assert_int_equal(f->header->encryptionSymbol, 0);

    Id3v2ContentEntry *e = (Id3v2ContentEntry *) f->entries->head->data;
    
    assert_int_equal(((uint8_t *)e->entry)[0], 3);
    assert_int_equal(e->size, 1);
    e = (Id3v2ContentEntry *) f->entries->head->next->data;

    assert_memory_equal(e->entry, "The Powers That Butf8ÛȾℲⅧ♈ ♉ ♊ ♋ ♌ ♍ ♎ ♏utf8",66);
    assert_int_equal(e->size, 67);

    listFree(context);
    byteStreamDestroy(stream);
    id3v2DestroyFrame(&f);
}

static void id3v2ParseFrame_parseTIT2UTF16(void **state){

    // TIT2
    uint8_t tit2[37] = {0x54, 0x49, 0x54, 0x32, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x00,
                        0x01, 0xFF, 0xFE, 's', 0x00, 'o', 0x00, 'r', 0x00, 'r',
                        0x00, 'y', 0x00, '4', 0x00, 'd', 0x00, 'y', 0x00, 'i',
                        0x00, 'n', 0x00, 'g', 0x00, 0x00, 0x00
    };

    

    ByteStream *stream = byteStreamCreate(tit2, 37);
    Id3v2Frame *f;
    List *context = id3v2CreateTextFrameContext();
    uint32_t frameSize = 0;

    frameSize = id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_3, &f);

    assert_non_null(f);
    assert_non_null(f->header);
    assert_memory_equal(f->header->id,"TIT2",4);
    assert_int_equal(frameSize, 37);
    assert_false(f->header->unsynchronisation);
    assert_false(f->header->readOnly);
    assert_false(f->header->tagAlterPreservation);
    assert_false(f->header->fileAlterPreservation);

    assert_int_equal(f->header->decompressionSize, 0);
    assert_int_equal(f->header->groupSymbol, 0);
    assert_int_equal(f->header->encryptionSymbol, 0);

    Id3v2ContentEntry *e = (Id3v2ContentEntry *) f->entries->head->data;
    
    assert_int_equal(((uint8_t *)e->entry)[0], 1);
    assert_int_equal(e->size, 1);

    e = (Id3v2ContentEntry *) f->entries->head->next->data;

    assert_memory_equal(e->entry, tit2 + 11 ,24);
    assert_int_equal(e->size, 24);

    listFree(context);
    byteStreamDestroy(stream);
    id3v2DestroyFrame(&f);

}

static void id3v2ParseFrame_parseTXXXUTF16(void **state){

    // TXXX 
    uint8_t txxx[71] = {'T', 'X', 'X', 'X', 0x00, 0x00, 0x00, 0x3d, 0x00, 0x00,
                    0x01,
                    0xff, 0xfe, 'u', 0x00, 't', 0x00, 'f', 0x00, '8', 0x00, 0xdb, 0x00, '>', 0x02, '2', '!', 'g', '!', 0x00, 0x00,
                    0xff, 0xfe, 'H', '&', ' ', 0x00, 'I', '&', ' ', 0x00, 'J', '&', ' ', 0x00, 'K', '&', ' ', 0x00, 'L', '&', ' ', 0x00, 'M', '&', ' ', 0x00, 'N', '&', ' ', 0x00, 'O', '&', 'u', 0x00, 't', 0x00, 'f', 0x00, '8', 0x00
    };

    

    ByteStream *stream = byteStreamCreate(txxx, 71);
    Id3v2Frame *f;
    List *context = id3v2CreateUserDefinedTextFrameContext();
    uint32_t frameSize = 0;

    frameSize = id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_3, &f);

    assert_non_null(f);
    assert_non_null(f->header);
    assert_memory_equal(f->header->id,"TXXX",4);
    assert_int_equal(frameSize, 71);
    assert_false(f->header->unsynchronisation);
    assert_false(f->header->readOnly);
    assert_false(f->header->tagAlterPreservation);
    assert_false(f->header->fileAlterPreservation);

    assert_int_equal(f->header->decompressionSize, 0);
    assert_int_equal(f->header->groupSymbol, 0);
    assert_int_equal(f->header->encryptionSymbol, 0);

    Id3v2ContentEntry *e = (Id3v2ContentEntry *) f->entries->head->data;
    
    assert_int_equal(((uint8_t *)e->entry)[0], 1);
    assert_int_equal(e->size, 1);

    e = (Id3v2ContentEntry *) f->entries->head->next->data;

    assert_memory_equal(e->entry, txxx + 11 ,18);
    assert_int_equal(e->size, 18);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->data;

    assert_memory_equal(e->entry, txxx + (11 + 20) ,40);
    assert_int_equal(e->size, 40);

    listFree(context);
    byteStreamDestroy(stream);
    id3v2DestroyFrame(&f);

}

static void id3v2ParseFrame_parseTXXXLatin1(void **state){
    // TXXX 
    uint8_t txxx[25] = {'T', 'X', 'X', 'X', 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00,
                    0x00,
                    'l','a','b','e','l',0x00,
                    'd','e','a','d',' ','a','i','r'
    };

    

    ByteStream *stream = byteStreamCreate(txxx, 25);
    Id3v2Frame *f;
    List *context = id3v2CreateUserDefinedTextFrameContext();
    uint32_t frameSize = 0;

    frameSize = id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_4, &f);

    assert_non_null(f);
    assert_non_null(f->header);
    assert_memory_equal(f->header->id,"TXXX",4);
    assert_int_equal(frameSize, 25);
    assert_false(f->header->unsynchronisation);
    assert_false(f->header->readOnly);
    assert_false(f->header->tagAlterPreservation);
    assert_false(f->header->fileAlterPreservation);

    assert_int_equal(f->header->decompressionSize, 0);
    assert_int_equal(f->header->groupSymbol, 0);
    assert_int_equal(f->header->encryptionSymbol, 0);

    Id3v2ContentEntry *e = (Id3v2ContentEntry *) f->entries->head->data;
    
    assert_int_equal(((uint8_t *)e->entry)[0], 0);
    assert_int_equal(e->size, 1);

    e = (Id3v2ContentEntry *) f->entries->head->next->data;

    assert_memory_equal(e->entry, "label" ,6);
    assert_int_equal(e->size, 6);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->data;

    assert_memory_equal(e->entry,  "dead air",9);
    assert_int_equal(e->size, 9);

    listFree(context);
    byteStreamDestroy(stream);
    id3v2DestroyFrame(&f);

}

static void id3v2ParseFrame_parseWCOM(void **state){

    uint8_t txxx[31] = {'W', 'C', 'O', 'M', 0x00, 0x00, 0x00, 0x15, 0x00, 0x00,
                    'h','t','t','p','s',':','/','/','b','a','n','d','c','a','m','p','.','c','o','m','/'
    };

    

    ByteStream *stream = byteStreamCreate(txxx, 31);
    Id3v2Frame *f;
    List *context = id3v2CreateURLFrameContext();
    uint32_t frameSize = 0;

    frameSize = id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_4, &f);

    assert_non_null(f);
    assert_non_null(f->header);
    assert_memory_equal(f->header->id,"WCOM",4);
    assert_int_equal(frameSize, 31);
    assert_false(f->header->unsynchronisation);
    assert_false(f->header->readOnly);
    assert_false(f->header->tagAlterPreservation);
    assert_false(f->header->fileAlterPreservation);

    assert_int_equal(f->header->decompressionSize, 0);
    assert_int_equal(f->header->groupSymbol, 0);
    assert_int_equal(f->header->encryptionSymbol, 0);

    Id3v2ContentEntry *e = (Id3v2ContentEntry *) f->entries->head->data;

    assert_memory_equal(e->entry, "https://bandcamp.com/", 22);
    assert_int_equal(e->size, 22);

    listFree(context);
    byteStreamDestroy(stream);
    id3v2DestroyFrame(&f);

}

static void id3v2ParseFrame_parseWXXUTF16(void **state){

    // WXX 
    uint8_t wxx[31] = {'W', 'X', 'X', 0x00, 0x00, 0x19,
                    0x01,
                    0xff, 0xfe, 's', 0x00, 't', 0x00, 'o', 0x00, 'r', 0x00, 'e', 0x00, 0x00, 0x00,
                    'i', 't','u','n','e','s','.','c','o','m'
    };

    

    ByteStream *stream = byteStreamCreate(wxx, 31);
    Id3v2Frame *f;
    List *context = id3v2CreateUserDefinedURLFrameContext();
    uint32_t frameSize = 0;

    frameSize = id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_2, &f);

    assert_non_null(f);
    assert_non_null(f->header);
    assert_memory_equal(f->header->id,"WXX",3);
    assert_int_equal(frameSize, 31);
    assert_false(f->header->unsynchronisation);
    assert_false(f->header->readOnly);
    assert_false(f->header->tagAlterPreservation);
    assert_false(f->header->fileAlterPreservation);

    assert_int_equal(f->header->decompressionSize, 0);
    assert_int_equal(f->header->groupSymbol, 0);
    assert_int_equal(f->header->encryptionSymbol, 0);

    Id3v2ContentEntry *e = (Id3v2ContentEntry *) f->entries->head->data;

    assert_int_equal(((uint8_t *)e->entry)[0], 1);
    assert_int_equal(e->size, 1);

    e = (Id3v2ContentEntry *) f->entries->head->next->data;

    assert_memory_equal(e->entry, wxx + 7, 12);
    assert_int_equal(e->size, 12);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->data;

    assert_memory_equal(e->entry, "itunes.com", 11);
    assert_int_equal(e->size, 11);

    listFree(context);
    byteStreamDestroy(stream);
    id3v2DestroyFrame(&f);

}

static void id3v2ParseFrame_parseCOMLatain1(void **state){

    // COM 
    uint8_t com[25] = {'C', 'O', 'M', 0x00, 0x00, 0x13,
                        0x00,
                        'e','n','g',
                        'c','o','m','m','e','n','t', 0x00,
                        't', 'e', 's', 't','i','n','g'
    };

    

    ByteStream *stream = byteStreamCreate(com, 25);
    Id3v2Frame *f;
    List *context = id3v2CreateCommentFrameContext();
    uint32_t frameSize = 0;

    frameSize = id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_2, &f);

    assert_non_null(f);
    assert_non_null(f->header);
    assert_memory_equal(f->header->id,"COM",3);
    assert_int_equal(frameSize, 25);
    assert_false(f->header->unsynchronisation);
    assert_false(f->header->readOnly);
    assert_false(f->header->tagAlterPreservation);
    assert_false(f->header->fileAlterPreservation);

    assert_int_equal(f->header->decompressionSize, 0);
    assert_int_equal(f->header->groupSymbol, 0);
    assert_int_equal(f->header->encryptionSymbol, 0);

    Id3v2ContentEntry *e = (Id3v2ContentEntry *) f->entries->head->data;

    assert_int_equal(((uint8_t *)e->entry)[0], 0);
    assert_int_equal(e->size, 1);

    e = (Id3v2ContentEntry *) f->entries->head->next->data;

    assert_memory_equal(e->entry, "eng", 3);
    assert_int_equal(e->size, 3);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->data;

    assert_memory_equal(e->entry, "comment", 8);
    assert_int_equal(e->size, 8);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->next->data;

    assert_memory_equal(e->entry, "testing", 8);
    assert_int_equal(e->size, 8);

    listFree(context);
    byteStreamDestroy(stream);
    id3v2DestroyFrame(&f);

}

static void id3v2ParseFrame_parseIPLLatin1(void **state){

    // ipl 
    uint8_t ipl[51] = {'I', 'P', 'L', 0x00, 0x00, 0x2D,
                     0x00,
                     'm','i','x','i','n','g',0x00,
                     'j','o','h','n', 0x00,
                     'e','n','g','i','n','e','e','r','i','n','g', 0x00,
                     'j','a','n','e', 0x00,
                     'm','a','s','t','e','r','i','n','g', 0x00,
                     'b','u','d','d','y'
    };

    

    ByteStream *stream = byteStreamCreate(ipl, 51);
    Id3v2Frame *f;
    List *context = id3v2CreateInvolvedPeopleListFrameContext();
    uint32_t frameSize = 0;

    frameSize = id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_2, &f);

    assert_non_null(f);
    assert_non_null(f->header);
    assert_memory_equal(f->header->id,"IPL",3);
    assert_int_equal(frameSize, 51);
    assert_false(f->header->unsynchronisation);
    assert_false(f->header->readOnly);
    assert_false(f->header->tagAlterPreservation);
    assert_false(f->header->fileAlterPreservation);

    assert_int_equal(f->header->decompressionSize, 0);
    assert_int_equal(f->header->groupSymbol, 0);
    assert_int_equal(f->header->encryptionSymbol, 0);

    Id3v2ContentEntry *e = (Id3v2ContentEntry *) f->entries->head->data;

    assert_int_equal(((uint8_t *)e->entry)[0], 0);
    assert_int_equal(e->size, 1);

    e = (Id3v2ContentEntry *) f->entries->head->next->data;

    assert_memory_equal(e->entry, "mixing", 7);
    assert_int_equal(e->size, 7);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->data;

    assert_memory_equal(e->entry, "john", 5);
    assert_int_equal(e->size, 5);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->next->data;

    assert_memory_equal(e->entry, "engineering", 12);
    assert_int_equal(e->size, 12);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->next->next->data;

    assert_memory_equal(e->entry, "jane", 5);
    assert_int_equal(e->size, 5);

    listFree(context);
    byteStreamDestroy(stream);
    id3v2DestroyFrame(&f);

}

static void id3v2ParseFrame_parseSYLTUTF16(void **state){

    // SYLT 
    uint8_t sylt[116] = {'S', 'Y', 'L', 'T', 0x00, 0x00, 0x00, 0x6A, 0x00, 0x00,
                      0x01,
                      'e','n','g',
                      0x02,
                      0x01,
                      0xff, 0xfe, 'g', 0x00, 'e', 0x00, 'n', 0x00, 'i', 0x00, 'u', 0x00, 's', 0x00, 0x00, 0x00,
                      0xff, 0xfe, 'S', 0x00, 't', 0x00, 'r', 0x00, 'a', 0x00, 'n', 0x00, 'g', 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x00,
                      0xff, 0xfe, 'e', 0x00, 'r', 0x00, 's', 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x25,
                      0xff, 0xfe, ' ', 0x00, 'i', 0x00, 'n', 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x89,
                      0xff, 0xfe, ' ', 0x00, 't', 0x00, 'h', 0x00, 'e', 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x95,
                      0xff, 0xfe, ' ', 0x00, 'n', 0x00, 'i', 0x00, 'g', 0x00, 'h', 0x00, 't', 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0xA9          
    };

    

    ByteStream *stream = byteStreamCreate(sylt, 116);
    Id3v2Frame *f;
    List *context = id3v2CreateSynchronisedLyricFrameContext();
    uint32_t frameSize = 0;

    frameSize = id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_3, &f);

    assert_non_null(f);
    assert_non_null(f->header);
    assert_memory_equal(f->header->id,"SYLT",4);
    assert_int_equal(frameSize, 116);
    assert_false(f->header->unsynchronisation);
    assert_false(f->header->readOnly);
    assert_false(f->header->tagAlterPreservation);
    assert_false(f->header->fileAlterPreservation);

    assert_int_equal(f->header->decompressionSize, 0);
    assert_int_equal(f->header->groupSymbol, 0);
    assert_int_equal(f->header->encryptionSymbol, 0);

    Id3v2ContentEntry *e = (Id3v2ContentEntry *) f->entries->head->data;

    assert_int_equal(((uint8_t *)e->entry)[0], 1);
    assert_int_equal(e->size, 1);

    e = (Id3v2ContentEntry *) f->entries->head->next->data;
    assert_memory_equal(e->entry, "eng", 3);
    assert_int_equal(e->size, 3);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->data;
    assert_int_equal(((uint8_t *)e->entry)[0], 2);
    assert_int_equal(e->size, 1);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->next->data;
    assert_int_equal(((uint8_t *)e->entry)[0], 1);
    assert_int_equal(e->size, 1);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->next->next->data;
    assert_memory_equal(e->entry, sylt + 16, 14);
    assert_int_equal(e->size, 14);

    listFree(context);
    byteStreamDestroy(stream);
    id3v2DestroyFrame(&f);

}

static void id3v2ParseFrame_parseEQU(void **state){
    // EQU 
    uint8_t equ[15] = {'E', 'Q', 'U', 0x00, 0x00, 0x09,
                        2U,
                        0x03, 0xe9, // 000000111110100 1  inc/dec 
                        0x40, 0x00,
                        0x00, 0x28, // 000000000010100 0  inc/dec
                        0xfc, 0x00
    };

    

    ByteStream *stream = byteStreamCreate(equ, 15);
    Id3v2Frame *f;
    List *context = id3v2CreateEqulizationFrameContext(ID3V2_TAG_VERSION_2);
    uint32_t frameSize = 0;

    frameSize = id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_2, &f);

    assert_non_null(f);
    assert_non_null(f->header);
    assert_memory_equal(f->header->id,"EQU",3);
    assert_int_equal(frameSize, 15);
    assert_false(f->header->unsynchronisation);
    assert_false(f->header->readOnly);
    assert_false(f->header->tagAlterPreservation);
    assert_false(f->header->fileAlterPreservation);

    assert_int_equal(f->header->decompressionSize, 0);
    assert_int_equal(f->header->groupSymbol, 0);
    assert_int_equal(f->header->encryptionSymbol, 0);

    Id3v2ContentEntry *e = (Id3v2ContentEntry *) f->entries->head->data;

    assert_int_equal(((uint8_t *)e->entry)[0], 2);
    assert_int_equal(e->size, 1);

    e = (Id3v2ContentEntry *) f->entries->head->next->data;
    assert_int_equal(((uint8_t *)e->entry)[0], 1);
    assert_int_equal(e->size, 1);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->data;
    assert_int_equal(((uint8_t *)e->entry)[0], 0);
    assert_int_equal(((uint8_t *)e->entry)[1], 0xe9);
    assert_int_equal(e->size, 2);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->next->data;
    assert_int_equal(((uint8_t *)e->entry)[0], 0x40);
    assert_int_equal(((uint8_t *)e->entry)[1], 0x00);
    assert_int_equal(e->size, 2);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->next->next->data;
    assert_int_equal(((uint8_t *)e->entry)[0], 0x00);
    assert_int_equal(e->size, 1);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->next->next->next->data;
    assert_int_equal(((uint8_t *)e->entry)[0], 0x00);
    assert_int_equal(((uint8_t *)e->entry)[1], 0x28);
    assert_int_equal(e->size, 2);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->next->next->next->next->data;
    assert_int_equal(((uint8_t *)e->entry)[0], 0xfc);
    assert_int_equal(((uint8_t *)e->entry)[1], 0x00);
    assert_int_equal(e->size, 2);


    listFree(context);
    byteStreamDestroy(stream);
    id3v2DestroyFrame(&f);
}

static void id3v2ParseFrame_parseEncrypted(void **state){

    uint8_t txxx[30] = {'T', 'X', 'X', 'X', 0x00, 0x00, 0x00, 0x0f, 0x00, 0x05,
                        0xff, 0x00, 0x1A, 0xE8, 0x5D, // 1763421 data length indicator before decode
                        0x00,
                        'l','a','b','e','l',0x00,
                        'd','e','a','d',' ','a','i','r'
    };

    

    ByteStream *stream = byteStreamCreate(txxx, 30);
    Id3v2Frame *f;
    List *context = id3v2CreateUserDefinedTextFrameContext();

    id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_4, &f);


    testFrameHeader(f, "TXXX", 0, 0, 0, 0, byteSyncintDecode(1763421), 0, 0xff);
    

    testEntry((Id3v2ContentEntry *) f->entries->head->data, 15, (uint8_t *)"\0label\0dead air");

    id3v2DestroyFrame(&f);
    listFree(context);
    byteStreamDestroy(stream);

}



static void id3v2ParseTagFromStream_v3(void **state){

    ByteStream *stream = byteStreamFromFile("assets/sorry4dying.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);
    uint8_t encodings[2] = {0,1};

    assert_non_null(tag);

    assert_non_null(tag->header);
    assert_int_equal(tag->header->flags, 0);
    assert_int_equal(tag->header->majorVersion, 3);
    assert_int_equal(tag->header->minorVersion, 0);
    assert_null(tag->header->extendedHeader);
    
    // TIT2
    Id3v2Frame *f = (Id3v2Frame *)tag->frames->head->data;
    Id3v2ContentEntry *ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data1[26] = {0xff, 0xfe, 's', 0x00, 'o', 0x00, 'r', 0x00, 'r', 0x00, 'y', 0x00, '4', 
                         0x00, 'd', 0x00, 'y', 0x00, 'i', 0x00, 'n', 0x00, 'g', 0x00, 0x00, 0x00};

    testFrameHeader(f, "TIT2", 0, 0, 0, 0, 0, 0, 0);

    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 24, data1);

    // TALB
    f = (Id3v2Frame *)tag->frames->head->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data2[56] = {0xff, 0xfe, 'I', 0x00, ' ', 0x00, 'D', 0x00, 'i', 0x00, 
                       'd', 0x00, 'n', 0x00, '\'', 0x00, 't', 0x00, ' ', 0x00,
                       'M', 0x00, 'e', 0x00, 'a', 0x00, 'n', 0x00, ' ', 0x00,
                       'T', 0x00, 'o', 0x00, ' ', 0x00, 'H', 0x00, 'a', 0x00,
                       'u', 0x00, 'n', 0x00, 't', 0x00, ' ', 0x00, 'Y', 0x00,
                       'o', 0x00, 'u', 0x00, 0x00, 0x00};

    testFrameHeader(f, "TALB", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 54, data2);


    // TSRC
    f = (Id3v2Frame *)tag->frames->head->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data3[28] = {0xff, 0xfe, 'Q', 0x00, 'M', 0x00, 'D', 0x00, 'A', 0x00,
                       '7', 0x00, '2', 0x00, '2', 0x00, '1', 0x0, '0', 0x00, '2', 
                       0x00, '3', 0x00, '6', 0x00, 0x00, 0x00};
    
    testFrameHeader(f, "TSRC", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 26, data3);

    // TCOP
    f = (Id3v2Frame *)tag->frames->head->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data4[298] = {0xff, 0xfe, '(', 0x00, 'C', 0x00, ')', 0x00, ' ', 0x00, 
                       '2', 0x00, '0', 0x00, '2', 0x00, '2', 0x00, ' ', 0x00, 
                       'd', 0x00, 'e', 0x00, 'a', 0x00, 'd', 0x00, 'A', 0x00,
                       'i', 0x00, 'r', 0x00, ' ', 0x00, 'u', 0x00, 'n', 0x00,
                       'd', 0x00, 'e', 0x00, 'r', 0x00, ' ', 0x00, 'e', 0x00,
                       'x', 0x00, 'c', 0x00, 'l', 0x00, 'u', 0x00, 's', 0x00,
                       'i', 0x00, 'v', 0x00, 'e', 0x00, ' ', 0x00, 'l', 0x00,
                       'i', 0x00, 'c', 0x00, 'e', 0x00, 'n', 0x00, 's', 0x00,
                       'e', 0x00, ' ', 0x00, 't', 0x00, 'o', 0x00, ' ', 0x00,
                       'A', 0x00, 'W', 0x00, 'A', 0x00, 'L', 0x00, ' ', 0x00,
                       'R', 0x00, 'e', 0x00, 'c', 0x00, 'o', 0x00, 'r', 0x00,
                       'd', 0x00, 'i', 0x00, 'n', 0x00, 'g', 0x00, 's', 0x00,
                       ' ', 0x00, 'A', 0x00, 'm', 0x00, 'e', 0x00, 'r', 0x00,
                       'i', 0x00, 'c', 0x00, 'a', 0x00, ',', 0x00, ' ', 0x00,
                       'I', 0x00, 'n', 0x00, 'c', 0x00, '.', 0x00, ' ', 0x00,
                       '(', 0x00, 'P', 0x00, ')', 0x00, ' ', 0x00, '2', 0x00,
                       '0', 0x00, '2', 0x00, '2', 0x00, ' ', 0x00, 'd', 0x00,
                       'e', 0x00, 'a', 0x00, 'd', 0x00, 'A', 0x00, 'i', 0x00,
                       'r', 0x00, ' ', 0x00, 'u', 0x00, 'n', 0x00, 'd', 0x00,
                       'e', 0x00, 'r', 0x00, ' ', 0x00, 'e', 0x00, 'x', 0x00,
                       'c', 0x00, 'l', 0x00, 'u', 0x00, 's', 0x00, 'i', 0x00,
                       'v', 0x00, 'e', 0x00, ' ', 0x00, 'l', 0x00, 'i', 0x00,
                       'c', 0x00, 'e', 0x00, 'n', 0x00, 's', 0x00, 'e', 0x00,
                       ' ', 0x00, 't', 0x00, 'o', 0x00, ' ', 0x00, 'A', 0x00,
                       'W', 0x00, 'A', 0x00, 'L', 0x00, ' ', 0x00, 'R', 0x00,
                       'e', 0x00, 'c', 0x00, 'o', 0x00, 'r', 0x00, 'd', 0x00,
                       'i', 0x00, 'n', 0x00, 'g', 0x00, 's', 0x00, ' ', 0x00,
                       'A', 0x00, 'm', 0x00, 'e', 0x00, 'r', 0x00, 'i', 0x00,
                       'c', 0x00, 'a', 0x00, ',', 0x00, ' ', 0x00, 'I', 0x00,
                       'n', 0x00, 'c', 0x00, '.', 0x00, 0x00, 0x00};

    testFrameHeader(f, "TCOP", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 296, data4);

    // TCON
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data5[250] = {0xff, 0xfe, 'A', 0x00, 'r', 0x00, 't', 0x00, ' ', 0x00, 
                       'P', 0x00, 'o', 0x00, 'p', 0x00, ',', 0x00, ' ', 0x00,
                       'F', 0x00, 'o', 0x00, 'l', 0x00, 'k', 0x00, 't', 0x00,
                       'r', 0x00, 'o', 0x00, 'n', 0x00, 'i', 0x00, 'c', 0x00,
                       'a', 0x00, ' ', 0x00, 'G', 0x00, 'l', 0x00, 'i', 0x00,
                       't', 0x00, 'c', 0x00, 'h', 0x00, ' ', 0x00, 'P', 0x00,
                       'o', 0x00, 'p', 0x00, ',', 0x00, ' ', 0x00, 'A', 0x00,
                       'm', 0x00, 'b', 0x00, 'i', 0x00, 'e', 0x00, 'n', 0x00,
                       't', 0x00, ' ', 0x00, 'P', 0x00, 'o', 0x00, 'p', 0x00,
                       ',', 0x00, ' ', 0x00, 'E', 0x00, 'x', 0x00, 'p', 0x00,
                       'e', 0x00, 'r', 0x00, 'i', 0x00, 'm', 0x00, 'e', 0x00,
                       'n', 0x00, 't', 0x00, 'a', 0x00, 'l', 0x00, ' ', 0x00,
                       'H', 0x00, 'i', 0x00, 'p', 0x00, '-', 0x00, 'H', 0x00,
                       'o', 0x00, 'p', 0x00, ',', 0x00, ' ', 0x00, 'N', 0x00,
                       'e', 0x00, 'o', 0x00, '-', 0x00, 'P', 0x00, 's', 0x00,
                       'y', 0x00, 'c', 0x00, 'h', 0x00, 'e', 0x00, 'd', 0x00,
                       'e', 0x00, 'l', 0x00, 'i', 0x00, 'a', 0x00, ',', 0x00,
                       ' ', 0x00, 'A', 0x00, 'l', 0x00, 't', 0x00, 'e', 0x00,
                       'r', 0x00, 'n', 0x00, 'a', 0x00, 't', 0x00, 'i', 0x00,
                       'v', 0x00, 'e', 0x00, ' ', 0x00, 'R', 0x00, '&', 0x00,
                       'B', 0x00, ',', 0x00, ' ', 0x00, 'E', 0x00, 'm', 0x00,
                       'o', 0x00, ' ', 0x00, 'R', 0x00, 'a', 0x00, 'p', 0x00,
                       ' ', 0x00, '&', 0x00, ' ', 0x00, 'C', 0x00, 'h', 0x00,
                       'a', 0x00, 'm', 0x00, 'b', 0x00, 'e', 0x00, 'r', 0x00,
                       ' ', 0x00, 'P', 0x00, 'o', 0x00, 'p', 0x00, 0x00, 0x00};
    
    testFrameHeader(f, "TCON", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 248, data5);

    // TYER
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data6[5] = {'2', '0', '2', '2', 0x00};

    testFrameHeader(f, "TYER", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[0]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 5, data6);

    // TRCK
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data7[6] = {'0', '1', '/', '1', '1', 0};

    testFrameHeader(f, "TRCK", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[0]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 6, data7);

    // TPOS
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data8[4] = {'1','/','1', 0};

    testFrameHeader(f, "TPOS", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[0]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    assert_int_equal(ce->size, 4);
    assert_memory_equal(ce->entry, data8, 4);

    // TPE1
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data9[18] = {0xff, 0xfe, 'Q', 0x00, 'u', 0x00, 'a', 0x00, 'd', 0x00, 
                         'e', 0x00, 'c', 0x00, 'a', 0x00, 0x00, 0x00};

    testFrameHeader(f, "TPE1", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 16, data9);

    // TPE2
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    //same data as TPE2

    testFrameHeader(f, "TPE2", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 16, data9);


    // TCOM
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data10[22] = {0xff, 0xfe, 'B', 0x00, 'e', 0x00, 'n', 0x00, ' ', 0x00, 
                         'L', 0x00, 'a', 0x00, 's', 0x00, 'k', 0x00, 'y', 0x00, 
                         0x00, 0x00};

    testFrameHeader(f, "TCOM", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 20, data10);

    // TXXX 1 label
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data11[14] = {0xff, 0xfe, 'L', 0x00, 'A', 0x00, 'B', 0x00, 'E', 0x00, 'L', 0x00, 0x00, 0x00};
    uint8_t data12[18] = {0xff, 0xfe, 'd', 0x00, 'e', 0x00, 'a', 0x00, 'd', 0x00, 'A', 0x00, 'i', 0x00, 'r', 0x00, 0x00, 0x00};

    testFrameHeader(f, "TXXX", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 12, data11);

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->data;

    testEntry(ce, 16, data12);

    // TXXX 2 performer
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data13[22] = {0xff, 0xfe, 'P', 0x00, 'E', 0x00, 'R', 0x00, 'F', 0x00, 
                          'O', 0x00, 'R', 0x00, 'M', 0x00, 'E', 0x00, 'R', 0x00, 
                          0x00, 0x00};

    testFrameHeader(f, "TXXX", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 20, data13);

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->data;

    testEntry(ce, 16, data9);

    // TXXX upc
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->next->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data14[10] = {0xff, 0xfe, 'U', 0x00, 'P', 0x00, 'C', 0x00, 0x00, 0x00};
    uint8_t data15[30] = {0xff, 0xfe, '0', 0x00, '1', 0x00, '9', 0x00, '7', 0x00, 
                         '1', 0x00, '8', 0x00, '7', 0x00, '3', 0x00, '3', 0x00,
                         '2', 0x00, '4', 0x00, '3', 0x00, '4', 0x00, 0x00, 0x00};

    testFrameHeader(f, "TXXX", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 8, data14);

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->data;

    testEntry(ce, 28, data15);

    // APIC
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->next->next->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;

    testFrameHeader(f, "APIC", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[0]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 11, (uint8_t *) "image/jpeg");

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->data;

    uint8_t pic = 3;
    testEntry(ce, 1, &pic);


    ce = (Id3v2ContentEntry *) f->entries->head->next->next->next->data;

    uint8_t desc = 0;
    testEntry(ce, 1, &desc);


    ce = (Id3v2ContentEntry *) f->entries->head->next->next->next->next->data;

    assert_int_equal(ce->size, 3098075);
    assert_non_null(ce->entry);


    // printf("%ld\n[",ce->size);
    // for(int i = 0; i < ce->size; i++){
    //     printf("[%c]",((uint8_t *)ce->entry)[i]);
    // }
    // printf("]\n");

    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);

}

static void id3v2ParseTagFromStream_v4(void **state){

    ByteStream *stream = byteStreamFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);
    uint8_t encodings[4] = {0,1,2,3};

    assert_non_null(tag);

    assert_non_null(tag->header);
    assert_int_equal(tag->header->flags, 0);
    assert_int_equal(tag->header->majorVersion, 4);
    assert_int_equal(tag->header->minorVersion, 0);

    assert_null(tag->header->extendedHeader);

    // TALB
    Id3v2Frame *f = (Id3v2Frame *)tag->frames->head->data;
    Id3v2ContentEntry *ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data[67] = "The Powers That Butf8ÛȾℲⅧ♈ ♉ ♊ ♋ ♌ ♍ ♎ ♏utf8"; 

    testFrameHeader(f, "TALB", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[3]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 67, data);

    // TRCK
    f = (Id3v2Frame *)tag->frames->head->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data2[12] = {0xff, 0xfe, '0', 0x00, '9', 0x00, '/', 0x00, '1', 0x00, '0', 0x00};

    testFrameHeader(f, "TRCK", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 12, data2);

    // TPOS
    f = (Id3v2Frame *)tag->frames->head->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data3[8] = {0xff, 0xfe, '2', 0x00, '/', 0x00, '2', 0x00};

    testFrameHeader(f, "TPOS", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 8, data3);

    // TPE1
    f = (Id3v2Frame *)tag->frames->head->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data4[24] = {0xff, 0xfe, 'D', 0x00, 'e', 0x00, 'a', 0x00, 't', 0x00, 
                        'h', 0x00, ' ', 0x00, 'G', 0x00, 'r', 0x00, 'i', 0x00, 
                        'p', 0x00, 's', 0x00};

    testFrameHeader(f, "TPE1", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 24, data4);

    // TIT2
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data5[12] = {0xff, 0xfe, 'O', 0x00, 'n', 0x00, ' ', 0x00, 'G', 0x00, 
                         'P', 0x00};

    testFrameHeader(f, "TIT2", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 12, data5);

    // TPE2
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    // same data as TPE1

    testFrameHeader(f, "TPE2", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 24, data4);

    // TCOM
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    // same data as TPE1

    testFrameHeader(f, "TCOM", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 24, data4);

    // APIC
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;

    testFrameHeader(f, "APIC", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[0]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 11, (uint8_t *)"image/jpeg");

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->data;
    testEntry(ce, 1, &(encodings[0])); //used just because encodings[0] == 0

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->next->data;
    testEntry(ce, 5, (uint8_t *)"test");

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->next->next->data;
    assert_non_null(ce);
    assert_int_equal(ce->size, 1845186);

    // APIC
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data6[12] = {0xff, 0xfe, 't', 0x00, 'e', 0x00, 's', 0x00, 't', 0x00, 0x00, 0x00};

    testFrameHeader(f, "APIC", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 11, (uint8_t *)"image/jpeg");

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->data;
    testEntry(ce, 1, &(encodings[3])); //used just because encodings[3] == 3

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->next->data;
    testEntry(ce, 10, data6);

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->next->next->data;
    assert_non_null(ce->entry);
    //assert_int_equal(ce->size, 34785);

    // TDRC
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;

    testFrameHeader(f, "TDRC", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[0]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 5, (uint8_t *)"2015");

    // TCON
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;

    testFrameHeader(f, "TCON", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[0]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 178, (uint8_t *) "Experimental Hip-Hop, Glitch Hop, Abstract Hip-Hop, Industrial Hip-Hop, Footwork, Wonky, Noise Rock, Rap Rock, Synth Punk, Hardcore Hip-Hop, Experimental Rock & Digital Hardcore");

    // TXXX
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data7[20] = {0xff, 0xfe, 0x75, 0x00, 0x74, 0x00, 0x66, 0x00, 0x38, 0x00, 
                         0xdb, 0x00, 0x3e, 0x02, 0x32, 0x21, 0x67, 0x21, 0x00, 0x00};
    uint8_t data8[40] = {0xff, 0xfe, 'H', '&', ' ', 0x00, 'I', '&', ' ', 0x00, 
                        'J', '&', ' ', 0x00, 'K', '&', ' ', 0x00, 'L', '&', 
                        ' ', 0x00, 'M', '&', ' ', 0x00, 'N', '&', ' ', 0x00, 
                        'O', '&', 'u', 0x00, 't', 0x00, 'f', 0x00, '8', 0x00};

    testFrameHeader(f, "TXXX", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 18, data7);

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->data;
    testEntry(ce, 40, data8);

    // WCOM
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;

    testFrameHeader(f, "WCOM", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 45, (uint8_t *)"The Powers That Butf8\xdb>2gH I J K L M N Outf8");

    // ETCO
    f = (Id3v2Frame *)tag->frames->head->next->next->next->next->next->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t v = 0;

    testFrameHeader(f, "ETCO", 0, 0, 0, 0, 0, 0, 0);
    
    v = 2U;
    testEntry(ce, 1, &v);

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    v = 6U;
    testEntry(ce, 1, &v);

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->data;
    testEntry(ce, 4, (uint8_t *) "\x00\x12\x9D\xA0");

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->next->data;
    v = 2U;
    testEntry(ce, 1, &v);

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->next->next->data;
    testEntry(ce, 4, (uint8_t *) "\x00\x09\x4e\xd0");


    // printf("%ld\n[",ce->size);
    // for(int i = 0; i < ce->size; i++){
    //     printf("[%x]",((uint8_t *)ce->entry)[i]);
    // }
    // printf("]\n");

    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);
}

static void id3v2ParseTagFromStream_v2unsync(void **state){

    uint8_t data[43] = {'I', 'D', '3', 0x02, 0x01, 0x80, 0x00, 0x00, 0x00, 0x21,
                        'T', 0x00, 'A', 0x00, 'L', 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 
                        0x00, 0x00, 
                        'F', 0x00, 'a', 0x00, 'm', 0x00, 'i', 0x00, 'l', 0x00, 'y', 0x00, ' ', 0x00, 'G', 0x00, 'u', 0x00, 'y'};

    ByteStream *stream = byteStreamCreate(data, 43);
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);
    uint8_t encoding = 0;

    assert_non_null(tag);
    assert_non_null(tag->header);
    assert_null(tag->header->extendedHeader);
    assert_int_equal(tag->header->flags, 0x80);
    assert_int_equal(tag->header->majorVersion, 2);
    assert_int_equal(tag->header->minorVersion, 1);

    Id3v2Frame *f = (Id3v2Frame *) tag->frames->head->data;
    Id3v2ContentEntry *ce = (Id3v2ContentEntry *)f->entries->head->data;

    testFrameHeader(f, "TAL\0", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &encoding); 

    ce = (Id3v2ContentEntry *)f->entries->head->next->data;
    testEntry(ce, 11, (uint8_t *)"Family Guy");

    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);

}

static void id3v2ParseTagFromStream_v3ext(void **state){

    uint8_t data[45] = {'I', 'D', '3', 0x03, 0x01, 0x40, 0x00, 0x00, 0x00, 0x23, // header
                        0x00, 0x00, 0x00, 0x0a, 0x80, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, // ext
                        'T', 'A', 'L', 'B', 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00,
                        0x00,
                        'F', 'a', 'm', 'i', 'l', 'y', ' ', 'G', 'u', 'y'};

    ByteStream *stream = byteStreamCreate(data, 45);
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);
    uint8_t encoding = 0;

    assert_non_null(tag);
    assert_non_null(tag->header);
    assert_non_null(tag->header->extendedHeader);
    assert_int_equal(tag->header->flags, 0x40);
    assert_int_equal(tag->header->majorVersion, 3);
    assert_int_equal(tag->header->minorVersion, 1);

    assert_int_equal(tag->header->extendedHeader->crc, 0xffffffff);
    assert_int_equal(tag->header->extendedHeader->padding, 0xfefefefe);

    Id3v2Frame *f = (Id3v2Frame *) tag->frames->head->data;
    Id3v2ContentEntry *ce = (Id3v2ContentEntry *)f->entries->head->data;

    testFrameHeader(f, "TALB", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &encoding); 

    ce = (Id3v2ContentEntry *)f->entries->head->next->data;
    testEntry(ce, 11, (uint8_t *)"Family Guy");

    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);
}


static void id3v2ParseTagFromStream_v2ULTWithMissingDesc(void **state){

    ByteStream *stream = byteStreamFromFile("assets/danybrown2.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);
    ListIter iter = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;
    Id3v2ContentEntry *ce = NULL;

    while((f = id3v2FrameTraverse(&iter)) != NULL){
        
        if(memcmp(f->header->id, "ULT", ID3V2_FRAME_ID_MAX_SIZE) == 0){
            testFrameHeader(f, "ULT", 0, 0, 0, 0, 0, 0, 0);

            ce = (Id3v2ContentEntry *) f->entries->head->data;
            testEntry(ce, 1, (uint8_t *)"\x00");

            ce = (Id3v2ContentEntry *) f->entries->head->next->data;
            testEntry(ce, 3, (uint8_t *)"eng");

            ce = (Id3v2ContentEntry *) f->entries->head->next->data;
            testEntry(ce, 3, (uint8_t *)"eng");

            ce = (Id3v2ContentEntry *) f->entries->head->next->next->data;
            testEntry(ce, 1, (uint8_t *)"\x00");

            ce = (Id3v2ContentEntry *) f->entries->head->next->next->next->data;
            testEntry(ce, 279, (uint8_t *)"haBDJHAsbdjkHASBDJahbsdkAHBSDHAbsdHBDUAHSBDUBAUIBFOASIUBDFOIAUBFOIAUWBFOAWBFAOUWEBFUOYBOUBUOBUOboubouboubouboubouboigndoignoisnjgsdfjnglksjdfngslkjfngskdjfnglskdnfgiserugisugnvfkdxjnvxlkjnijxdngixjdhfgoiserhgiusdng spoerijgsoergjnposeirhgposergn reigjosperijgsodfkgkldfmvxc.,vbm");

            break;
        }
    }
    
    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);

}

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

        // id3v2ParseTagHeader tests
        cmocka_unit_test(id3v2ParseTagHeader_happyPath),
        cmocka_unit_test(id3v2ParseTagHeader_noTagSize),
        cmocka_unit_test(id3v2ParseTagHeader_noFlags),
        cmocka_unit_test(id3v2ParseTagHeader_noVersions),

        // id3v2ParseFrameHeader tests
        cmocka_unit_test(id3v2ParseFrameHeader_noSupport),

        cmocka_unit_test(id3v2ParseFrameHeader_v2),
        cmocka_unit_test(id3v2ParseFrameHeader_v2MissingSize),

        cmocka_unit_test(id3v2ParseFrameHeader_v3),
        cmocka_unit_test(id3v2ParseFrameHeader_v3FlagsButNoSymbols),
        cmocka_unit_test(id3v2ParseFrameHeader_v3noFlags),
        cmocka_unit_test(id3v2ParseFrameHeader_v3noFlagBytes),

        cmocka_unit_test(id3v2ParseFrameHeader_v4),
        cmocka_unit_test(id3v2ParseFrameHeader_v4SetFlagButNoContent),
        cmocka_unit_test(id3v2ParseFrameHeader_v4NoSetFlagsButContent),

        // id3v2ParseFrame
        cmocka_unit_test(id3v2ParseFrame_parseTALBUTF8),
        cmocka_unit_test(id3v2ParseFrame_parseTIT2UTF16),
        cmocka_unit_test(id3v2ParseFrame_parseTXXXUTF16),
        cmocka_unit_test(id3v2ParseFrame_parseTXXXLatin1),
        cmocka_unit_test(id3v2ParseFrame_parseWCOM),
        cmocka_unit_test(id3v2ParseFrame_parseWXXUTF16),
        cmocka_unit_test(id3v2ParseFrame_parseCOMLatain1),
        cmocka_unit_test(id3v2ParseFrame_parseIPLLatin1),
        cmocka_unit_test(id3v2ParseFrame_parseSYLTUTF16),
        cmocka_unit_test(id3v2ParseFrame_parseEQU),
        cmocka_unit_test(id3v2ParseFrame_parseEncrypted),


        // id3v2ParseTagFromStream
        cmocka_unit_test(id3v2ParseTagFromStream_v3),
        cmocka_unit_test(id3v2ParseTagFromStream_v4),
        cmocka_unit_test(id3v2ParseTagFromStream_v2unsync),
        cmocka_unit_test(id3v2ParseTagFromStream_v3ext),
        cmocka_unit_test(id3v2ParseTagFromStream_v2ULTWithMissingDesc)

    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}