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
    
    uint8_t ext[6] = {0,0,0,6,
                      0,
                      0x40};
                       

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

static void id3v2ParseTagHeader_happyPath(void **state){
    
    uint8_t ext[10] = {'I','D','3',
                      2, 
                      0,
                      0,
                      0,0x72,0x6C,0x2E};
                       

    ByteStream *stream = byteStreamCreate(ext, 10);

    Id3v2TagHeader *h;
    uint32_t size = 0;

    uint32_t v = id3v2ParseTagHeader(stream, &h, &size);
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

    uint32_t v = id3v2ParseTagHeader(stream, &h, &size);
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

    uint32_t v = id3v2ParseTagHeader(stream, &h, &size);
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

    uint32_t v = id3v2ParseTagHeader(stream, &h, &size);
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
    uint32_t v = id3v2ParseFrameHeader(stream, 99, &h, &size);

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
    uint32_t v = id3v2ParseFrameHeader(stream, ID3V2_TAG_VERSION_2, &h, &size);

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
    uint32_t v = id3v2ParseFrameHeader(stream, ID3V2_TAG_VERSION_2, &h, &size);

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
    uint32_t v = id3v2ParseFrameHeader(stream, ID3V2_TAG_VERSION_3, &h, &size);

    assert_non_null(h);
    assert_int_equal(v, 16);
    assert_int_equal(size, 94);

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
    uint32_t v = id3v2ParseFrameHeader(stream, ID3V2_TAG_VERSION_3, &h, &size);

    assert_non_null(h);
    assert_int_equal(v, 14);
    assert_int_equal(size, 96);

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
    uint32_t v = id3v2ParseFrameHeader(stream, ID3V2_TAG_VERSION_3, &h, &size);

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
    uint32_t v = id3v2ParseFrameHeader(stream, ID3V2_TAG_VERSION_3, &h, &size);

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
    uint32_t v = id3v2ParseFrameHeader(stream, ID3V2_TAG_VERSION_4, &h, &size);

    assert_non_null(h);
    assert_int_equal(v, 16);
    assert_int_equal(size, 250);

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
    uint32_t v = id3v2ParseFrameHeader(stream, ID3V2_TAG_VERSION_4, &h, &size);

    assert_non_null(h);
    assert_int_equal(v, 12);
    assert_int_equal(size, 250);

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
    uint32_t v = id3v2ParseFrameHeader(stream, ID3V2_TAG_VERSION_4, &h, &size);

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

    frameSize = id3v2ParseFrame(stream, context, ID3V2_TAG_VERSION_4, &f);


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

    frameSize = id3v2ParseFrame(stream, context, ID3V2_TAG_VERSION_3, &f);

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

    frameSize = id3v2ParseFrame(stream, context, ID3V2_TAG_VERSION_3, &f);

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

    frameSize = id3v2ParseFrame(stream, context, ID3V2_TAG_VERSION_4, &f);

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
    // TXXX 
    uint8_t txxx[31] = {'W', 'C', 'O', 'M', 0x00, 0x00, 0x00, 0x15, 0x00, 0x00,
                    'h','t','t','p','s',':','/','/','b','a','n','d','c','a','m','p','.','c','o','m','/'
    };

    

    ByteStream *stream = byteStreamCreate(txxx, 31);
    Id3v2Frame *f;
    List *context = id3v2CreateURLFrameContext();
    uint32_t frameSize = 0;

    frameSize = id3v2ParseFrame(stream, context, ID3V2_TAG_VERSION_4, &f);

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

    frameSize = id3v2ParseFrame(stream, context, ID3V2_TAG_VERSION_2, &f);

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

    frameSize = id3v2ParseFrame(stream, context, ID3V2_TAG_VERSION_2, &f);

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

    frameSize = id3v2ParseFrame(stream, context, ID3V2_TAG_VERSION_2, &f);

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

    frameSize = id3v2ParseFrame(stream, context, ID3V2_TAG_VERSION_3, &f);

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

}


static void playground(void **state){
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

    frameSize = id3v2ParseFrame(stream, context, ID3V2_TAG_VERSION_2, &f);

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

    // e = (Id3v2ContentEntry *) f->entries->head->next->next->data;
    // assert_int_equal(((uint8_t *)e->entry)[0], 2);
    // assert_int_equal(e->size, 1);

    // e = (Id3v2ContentEntry *) f->entries->head->next->next->next->data;
    // assert_int_equal(((uint8_t *)e->entry)[0], 1);
    // assert_int_equal(e->size, 1);

    // e = (Id3v2ContentEntry *) f->entries->head->next->next->next->next->data;
    // assert_memory_equal(e->entry, sylt + 16, 16);
    // assert_int_equal(e->size, 14);

    listFree(context);
    byteStreamDestroy(stream);
    id3v2DestroyFrame(&f);

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

        cmocka_unit_test(playground)

    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}