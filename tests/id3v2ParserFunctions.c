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
                            uint8_t es) {
    assert_non_null(f);
    assert_non_null(f->header);
    assert_memory_equal(f->header->id, id, 4);
    assert_int_equal(f->header->unsynchronisation, u);
    assert_int_equal(f->header->readOnly, ro);
    assert_int_equal(f->header->tagAlterPreservation, tap);
    assert_int_equal(f->header->fileAlterPreservation, fap);
    assert_int_equal(f->header->decompressionSize, ds);
    assert_int_equal(f->header->groupSymbol, gs);
    assert_int_equal(f->header->encryptionSymbol, es);
}

static void testEntry(Id3v2ContentEntry *ce, size_t size, const uint8_t *data) {
    assert_non_null(ce);
    assert_int_equal(ce->size, size);
    assert_memory_equal(ce->entry, data, size);
}

static void id3v2ParseExtendedTagHeader_nullData(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *h;

    uint32_t v = id3v2ParseExtendedTagHeader(NULL, 0, ID3V2_TAG_VERSION_2, &h);

    assert_int_equal(v, 0),
            assert_null(h);
}

static void id3v2ParseExtendedTagHeader_v2(void **state) {
    (void) state;
    ByteStream *stream = byteStreamCreate(NULL, 1);

    Id3v2ExtendedTagHeader *h;

    uint32_t v = id3v2ParseExtendedTagHeader(stream->buffer, 0, ID3V2_TAG_VERSION_2, &h);

    byteStreamDestroy(stream);
    assert_null(h);
    assert_int_equal(v, 0);
}

// parses `in` and checks it was rejected as junk
static void internal_assertExtRejected(const uint8_t *in, size_t inl, uint8_t version) {
    Id3v2ExtendedTagHeader *h = (Id3v2ExtendedTagHeader *) 0x1;
    uint32_t v = id3v2ParseExtendedTagHeader((uint8_t *) in, inl, version, &h);

    assert_int_equal(v, 0);
    assert_null(h);
}

static void id3v2ParseExtendedTagHeader_v3(void **state) {
    (void) state;
    uint8_t ext[16] = {
        0, 0, 0, 0x0A, // size, excludes itself
        0x80, 0, // flags, CRC present
        0, 0, 0, 2, // padding
        0, 0, 0, 15, // crc
        0, 0 // the padding itself
    };

    Id3v2ExtendedTagHeader *h;
    uint32_t v = id3v2ParseExtendedTagHeader(ext, sizeof(ext), ID3V2_TAG_VERSION_3, &h);

    assert_int_equal(v, 14);
    assert_non_null(h);
    assert_int_equal(h->padding, 2);
    assert_int_equal(h->crc, 0xf);

    id3v2DestroyExtendedTagHeader(&h);
}

static void id3v2ParseExtendedTagHeader_v3NoCrc(void **state) {
    (void) state;
    uint8_t ext[10] = {
        0, 0, 0, 0x06, // size
        0, 0, // flags
        0, 0, 0, 0 // padding
    };

    Id3v2ExtendedTagHeader *h;
    uint32_t v = id3v2ParseExtendedTagHeader(ext, sizeof(ext), ID3V2_TAG_VERSION_3, &h);

    assert_int_equal(v, 10);
    assert_non_null(h);
    assert_int_equal(h->padding, 0);
    assert_int_equal(h->crc, 0);

    id3v2DestroyExtendedTagHeader(&h);
}

static void id3v2ParseExtendedTagHeader_v3Rejected(void **state) {
    (void) state;
    // the size id3dev wrote before it followed the standard, counting its own 4 bytes
    const uint8_t oldSize[10] = {0, 0, 0, 0x0A, 0, 0, 0, 0, 0, 0};
    const uint8_t oldSizeCrc[14] = {0, 0, 0, 0x0E, 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    // size says CRC, flags don't
    const uint8_t sizeFlagMismatch[14] = {0, 0, 0, 0x0A, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    // flags say CRC, size doesn't
    const uint8_t flagSizeMismatch[10] = {0, 0, 0, 0x06, 0x80, 0, 0, 0, 0, 0};
    // undefined flag bits
    const uint8_t undefinedFlag[10] = {0, 0, 0, 0x06, 0x40, 0, 0, 0, 0, 0};
    const uint8_t undefinedLowFlag[10] = {0, 0, 0, 0x06, 0, 0x01, 0, 0, 0, 0};
    // more padding than bytes left in the tag
    const uint8_t paddingTooLarge[12] = {0, 0, 0, 0x06, 0, 0, 0, 0, 0, 3, 0, 0};
    // CRC flag set but the buffer ends before the CRC
    const uint8_t truncated[10] = {0, 0, 0, 0x0A, 0x80, 0, 0, 0, 0, 0};
    const uint8_t zeroSize[4] = {0, 0, 0, 0};

    internal_assertExtRejected(oldSize, sizeof(oldSize), ID3V2_TAG_VERSION_3);
    internal_assertExtRejected(oldSizeCrc, sizeof(oldSizeCrc), ID3V2_TAG_VERSION_3);
    internal_assertExtRejected(sizeFlagMismatch, sizeof(sizeFlagMismatch), ID3V2_TAG_VERSION_3);
    internal_assertExtRejected(flagSizeMismatch, sizeof(flagSizeMismatch), ID3V2_TAG_VERSION_3);
    internal_assertExtRejected(undefinedFlag, sizeof(undefinedFlag), ID3V2_TAG_VERSION_3);
    internal_assertExtRejected(undefinedLowFlag, sizeof(undefinedLowFlag), ID3V2_TAG_VERSION_3);
    internal_assertExtRejected(paddingTooLarge, sizeof(paddingTooLarge), ID3V2_TAG_VERSION_3);
    internal_assertExtRejected(truncated, sizeof(truncated), ID3V2_TAG_VERSION_3);
    internal_assertExtRejected(zeroSize, sizeof(zeroSize), ID3V2_TAG_VERSION_3);
    internal_assertExtRejected(zeroSize, 3, ID3V2_TAG_VERSION_3);
}

static void id3v2ParseExtendedTagHeader_v3PaddingFits(void **state) {
    (void) state;
    const uint8_t ext[13] = {0, 0, 0, 0x06, 0, 0, 0, 0, 0, 3, 0, 0, 0};
    Id3v2ExtendedTagHeader *h;

    uint32_t v = id3v2ParseExtendedTagHeader((uint8_t *) ext, sizeof(ext), ID3V2_TAG_VERSION_3, &h);

    assert_int_equal(v, 10);
    assert_non_null(h);
    assert_int_equal(h->padding, 3);

    id3v2DestroyExtendedTagHeader(&h);
}

static void id3v2ParseExtendedTagHeader_v4(void **state) {
    (void) state;
    uint8_t ext[15] = {
        0, 0, 0, 15, // syncsafe size of the whole extended header
        1, // number of flag bytes
        0x70, // update, CRC, restrictions
        0x00, // update length
        0x05, 0, 0, 0, 0x3E, 0x48, // CRC length and syncsafe CRC
        0x01, 0xff // restrictions length and restrictions
    };

    Id3v2ExtendedTagHeader *h;
    uint32_t v = id3v2ParseExtendedTagHeader(ext, sizeof(ext), ID3V2_TAG_VERSION_4, &h);

    assert_int_equal(v, 15);
    assert_non_null(h);
    assert_int_equal(h->crc, 8008);
    assert_true(h->update);
    assert_true(h->tagRestrictions);
    assert_int_equal(h->restrictions, 0xff);

    id3v2DestroyExtendedTagHeader(&h);
}

static void id3v2ParseExtendedTagHeader_v4NoRestrictions(void **state) {
    (void) state;
    uint8_t ext[13] = {
        0, 0, 0, 13,
        1,
        0x60,
        0x00,
        0x05, 0, 0, 0, 0x3E, 0x48
    };

    Id3v2ExtendedTagHeader *h;
    uint32_t v = id3v2ParseExtendedTagHeader(ext, sizeof(ext), ID3V2_TAG_VERSION_4, &h);

    assert_int_equal(v, 13);
    assert_non_null(h);
    assert_int_equal(h->crc, 8008);
    assert_true(h->update);
    assert_false(h->tagRestrictions);
    assert_int_equal(h->restrictions, 0);

    id3v2DestroyExtendedTagHeader(&h);
}

static void id3v2ParseExtendedTagHeader_v4NoCRC(void **state) {
    (void) state;
    uint8_t ext[7] = {
        0, 0, 0, 7,
        1,
        0x40,
        0x00
    };

    Id3v2ExtendedTagHeader *h;
    uint32_t v = id3v2ParseExtendedTagHeader(ext, sizeof(ext), ID3V2_TAG_VERSION_4, &h);

    assert_int_equal(v, 7);
    assert_non_null(h);
    assert_int_equal(h->crc, 0);
    assert_true(h->update);
    assert_false(h->tagRestrictions);
    assert_int_equal(h->restrictions, 0);

    id3v2DestroyExtendedTagHeader(&h);
}

static void id3v2ParseExtendedTagHeader_v4NoFlags(void **state) {
    (void) state;
    uint8_t ext[8] = {0, 0, 0, 6, 1, 0x00, 'T', 'I'}; // followed by a frame
    Id3v2ExtendedTagHeader *h;

    uint32_t v = id3v2ParseExtendedTagHeader(ext, sizeof(ext), ID3V2_TAG_VERSION_4, &h);

    assert_int_equal(v, 6);
    assert_non_null(h);
    assert_false(h->update);
    assert_false(h->tagRestrictions);
    assert_int_equal(h->crc, 0);

    id3v2DestroyExtendedTagHeader(&h);
}

static void id3v2ParseExtendedTagHeader_v4Rejected(void **state) {
    (void) state;
    // the layout id3dev wrote before it followed the standard: data byte count, no length bytes
    const uint8_t oldLayout[12] = {0, 0, 0, 12, 6, 0x70, 0, 0, 0, 0x3E, 0x48, 0xff};
    const uint8_t oldNoFlags[6] = {0, 0, 0, 6, 0, 0x00};
    // more than one flag byte
    const uint8_t flagCount[7] = {0, 0, 0, 7, 2, 0x00, 0x00};
    // undefined flag bits
    const uint8_t undefinedHigh[6] = {0, 0, 0, 6, 1, 0x80};
    const uint8_t undefinedLow[6] = {0, 0, 0, 6, 1, 0x01};
    // wrong length bytes
    const uint8_t updateLength[7] = {0, 0, 0, 7, 1, 0x40, 0x01};
    const uint8_t crcLength[12] = {0, 0, 0, 12, 1, 0x20, 0x04, 0, 0, 0x3E, 0x48, 0};
    const uint8_t restrictionsLength[8] = {0, 0, 0, 8, 1, 0x10, 0x02, 0xff};
    // CRC bytes that aren't syncsafe, or hold more than 32 bits
    const uint8_t crcNotSyncsafe[12] = {0, 0, 0, 12, 1, 0x20, 0x05, 0, 0, 0, 0x80, 0};
    const uint8_t crcTooLarge[12] = {0, 0, 0, 12, 1, 0x20, 0x05, 0x10, 0, 0, 0, 0};
    // size doesn't match the flags
    const uint8_t sizeTooLarge[8] = {0, 0, 0, 8, 1, 0x00, 0, 0};
    const uint8_t sizeTooSmall[8] = {0, 0, 0, 7, 1, 0x10, 0x01, 0xff};
    const uint8_t sizeBelowSix[6] = {0, 0, 0, 5, 1, 0x00};
    // size bytes that aren't syncsafe
    const uint8_t sizeNotSyncsafe[6] = {0, 0, 0, 0x86, 1, 0x00};
    // size beyond the bytes left in the tag
    const uint8_t truncated[7] = {0, 0, 0, 8, 1, 0x10, 0x01};

    internal_assertExtRejected(oldLayout, sizeof(oldLayout), ID3V2_TAG_VERSION_4);
    internal_assertExtRejected(oldNoFlags, sizeof(oldNoFlags), ID3V2_TAG_VERSION_4);
    internal_assertExtRejected(flagCount, sizeof(flagCount), ID3V2_TAG_VERSION_4);
    internal_assertExtRejected(undefinedHigh, sizeof(undefinedHigh), ID3V2_TAG_VERSION_4);
    internal_assertExtRejected(undefinedLow, sizeof(undefinedLow), ID3V2_TAG_VERSION_4);
    internal_assertExtRejected(updateLength, sizeof(updateLength), ID3V2_TAG_VERSION_4);
    internal_assertExtRejected(crcLength, sizeof(crcLength), ID3V2_TAG_VERSION_4);
    internal_assertExtRejected(restrictionsLength, sizeof(restrictionsLength), ID3V2_TAG_VERSION_4);
    internal_assertExtRejected(crcNotSyncsafe, sizeof(crcNotSyncsafe), ID3V2_TAG_VERSION_4);
    internal_assertExtRejected(crcTooLarge, sizeof(crcTooLarge), ID3V2_TAG_VERSION_4);
    internal_assertExtRejected(sizeTooLarge, sizeof(sizeTooLarge), ID3V2_TAG_VERSION_4);
    internal_assertExtRejected(sizeTooSmall, sizeof(sizeTooSmall), ID3V2_TAG_VERSION_4);
    internal_assertExtRejected(sizeBelowSix, sizeof(sizeBelowSix), ID3V2_TAG_VERSION_4);
    internal_assertExtRejected(sizeNotSyncsafe, sizeof(sizeNotSyncsafe), ID3V2_TAG_VERSION_4);
    internal_assertExtRejected(truncated, sizeof(truncated), ID3V2_TAG_VERSION_4);
}

// serializes, parses and serializes again; both byte arrays must match and the fields survive
static void internal_assertExtRoundTrip(Id3v2ExtendedTagHeader *ext, uint8_t version) {
    size_t outl = 0;
    size_t againl = 0;
    Id3v2ExtendedTagHeader *parsed = NULL;
    uint8_t *out = id3v2ExtendedTagHeaderSerialize(ext, version, &outl);
    uint8_t *buf = calloc(outl + ext->padding, 1); // room for the padding the header promises

    assert_non_null(out);
    memcpy(buf, out, outl);

    assert_int_equal(id3v2ParseExtendedTagHeader(buf, outl + ext->padding, version, &parsed), outl);
    assert_non_null(parsed);
    assert_true(id3v2ValidateExtendedTagHeader(parsed, version));
    assert_int_equal(parsed->crc, ext->crc);
    assert_int_equal(parsed->restrictions, ext->restrictions);
    assert_int_equal(parsed->tagRestrictions, ext->tagRestrictions);
    assert_int_equal(parsed->update, ext->update);
    assert_int_equal(parsed->padding, ext->padding);

    uint8_t *again = id3v2ExtendedTagHeaderSerialize(parsed, version, &againl);
    assert_int_equal(againl, outl);
    assert_memory_equal(again, out, outl);

    free(again);
    free(buf);
    free(out);
    id3v2DestroyExtendedTagHeader(&parsed);
}

static void id3v2ParseExtendedTagHeader_roundTrip(void **state) {
    (void) state;
    Id3v2ExtendedTagHeader *cases[] = {
        id3v2CreateExtendedTagHeader(0, 0, false, false, 0), // v3
        id3v2CreateExtendedTagHeader(16, 0xDEADBEEF, false, false, 0), // v3
        id3v2CreateExtendedTagHeader(0, 0, false, false, 0), // v4
        id3v2CreateExtendedTagHeader(0, 0, true, false, 0),
        id3v2CreateExtendedTagHeader(0, UINT32_MAX, false, false, 0),
        id3v2CreateExtendedTagHeader(0, 0, false, true, 0xA5),
        id3v2CreateExtendedTagHeader(0, 0xDEADBEEF, true, true, 0xff),
    };
    const uint8_t versions[] = {3, 3, 4, 4, 4, 4, 4};

    for (size_t i = 0; i < sizeof(versions); i++) {
        internal_assertExtRoundTrip(cases[i], versions[i]);
        id3v2DestroyExtendedTagHeader(&cases[i]);
    }
}

static void id3v2ParseTagHeader_happyPath(void **state) {
    (void) state;
    uint8_t ext[10] = {
        'I', 'D', '3',
        2,
        0,
        0,
        0, 0x72, 0x6C, 0x2E
    };


    ByteStream *stream = byteStreamCreate(ext, 10);

    Id3v2TagHeader *h;
    uint32_t size = 0;

    uint32_t v = id3v2ParseTagHeader(stream->buffer, stream->bufferSize, &h, &size);
    assert_int_equal(stream->cursor, 0);
    assert_int_equal(v, 10);
    assert_non_null(h);
    assert_int_equal(h->majorVersion, 2);
    assert_int_equal(h->minorVersion, 0);
    assert_int_equal(h->flags, 0);
    assert_int_equal(h->extendedHeader, NULL);
    assert_int_equal(size, byteSyncintDecode(0x726C2E));

    id3v2DestroyTagHeader(&h);
    byteStreamDestroy(stream);
}

static void id3v2ParseTagHeader_noTagSize(void **state) {
    (void) state;
    uint8_t ext[6] = {
        'I', 'D', '3',
        2,
        0,
        0
    };


    ByteStream *stream = byteStreamCreate(ext, 6);

    Id3v2TagHeader *h;
    uint32_t size = 0;

    uint32_t v = id3v2ParseTagHeader(stream->buffer, stream->bufferSize, &h, &size);
    assert_int_equal(stream->cursor, 0);
    assert_int_equal(v, 6);
    assert_non_null(h);
    assert_int_equal(h->majorVersion, 2);
    assert_int_equal(h->minorVersion, 0);
    assert_int_equal(h->flags, 0);
    assert_int_equal(h->extendedHeader, NULL);
    assert_int_equal(size, 0);

    id3v2DestroyTagHeader(&h);
    byteStreamDestroy(stream);
}

static void id3v2ParseTagHeader_noFlags(void **state) {
    (void) state;
    uint8_t ext[5] = {
        'I', 'D', '3',
        2,
        0
    };


    ByteStream *stream = byteStreamCreate(ext, 5);

    Id3v2TagHeader *h;
    uint32_t size = 0;

    uint32_t v = id3v2ParseTagHeader(stream->buffer, stream->bufferSize, &h, &size);
    assert_int_equal(stream->cursor, 0);
    assert_int_equal(v, 5);
    assert_non_null(h);
    assert_int_equal(h->majorVersion, 2);
    assert_int_equal(h->minorVersion, 0);
    assert_int_equal(h->flags, 0);
    assert_int_equal(h->extendedHeader, NULL);
    assert_int_equal(size, 0);

    id3v2DestroyTagHeader(&h);
    byteStreamDestroy(stream);
}

static void id3v2ParseTagHeader_noVersions(void **state) {
    (void) state;
    uint8_t ext[3] = {'I', 'D', '3'};


    ByteStream *stream = byteStreamCreate(ext, 3);
    Id3v2TagHeader *h;
    uint32_t size = 0;

    uint32_t v = id3v2ParseTagHeader(stream->buffer, stream->bufferSize, &h, &size);
    assert_int_equal(stream->cursor, 0);
    assert_int_equal(v, 3);
    assert_non_null(h);
    assert_int_equal(h->majorVersion, 0);
    assert_int_equal(h->minorVersion, 0);
    assert_int_equal(h->flags, 0);
    assert_int_equal(h->extendedHeader, NULL);
    assert_int_equal(size, 0);

    id3v2DestroyTagHeader(&h);
    byteStreamDestroy(stream);
}

static void id3v2ParseFrameHeader_noSupport(void **state) {
    (void) state;
    uint8_t ext[6] = {
        'T', 'A', 'L',
        0x00, 0x01, 0x00,
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


static void id3v2ParseFrameHeader_v2(void **state) {
    (void) state;
    uint8_t ext[6] = {
        'T', 'A', 'L',
        0x00, 0x01, 0x00,
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

static void id3v2ParseFrameHeader_v2MissingSize(void **state) {
    (void) state;
    uint8_t ext[3] = {'T', 'A', 'L'};

    ByteStream *stream = byteStreamCreate(ext, 3);
    Id3v2FrameHeader *h;
    uint32_t size = 0;
    uint32_t v = id3v2ParseFrameHeader(stream->buffer, stream->bufferSize, ID3V2_TAG_VERSION_2, &h, &size);

    assert_null(h);
    assert_int_equal(v, 3);


    byteStreamDestroy(stream);
}

static void id3v2ParseFrameHeader_v3(void **state) {
    (void) state;
    uint8_t ext[16] = {
        'T', 'A', 'L', 'B',
        0x00, 0x00, 0x00, 0x64,
        0xE0, 0xE0,
        0x00, 0x00, 0xEA, 0x60,
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

static void id3v2ParseFrameHeader_v3FlagsButNoSymbols(void **state) {
    (void) state;
    uint8_t ext[14] = {
        'T', 'A', 'L', 'B',
        0x00, 0x00, 0x00, 0x64,
        0xE0, 0xE0,
        0x00, 0x00, 0xEA, 0x60
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

static void id3v2ParseFrameHeader_v3noFlags(void **state) {
    (void) state;
    uint8_t ext[10] = {
        'T', 'A', 'L', 'B',
        0x00, 0x00, 0x00, 0x64,
        0x00, 0x00
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

static void id3v2ParseFrameHeader_v3noFlagBytes(void **state) {
    (void) state;
    uint8_t ext[8] = {
        'T', 'A', 'L', 'B',
        0x00, 0x00, 0x00, 0x64
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

static void id3v2ParseFrameHeader_v4(void **state) {
    (void) state;
    uint8_t ext[16] = {
        'T', 'I', 'T', '2',
        0x00, 0x00, 0x02, 0x00,
        0x70, 0x4F,
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

static void id3v2ParseFrameHeader_v4SetFlagButNoContent(void **state) {
    (void) state;
    uint8_t ext[12] = {
        'T', 'I', 'T', '2',
        0x00, 0x00, 0x02, 0x00,
        0x70, 0x4F,
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

static void id3v2ParseFrameHeader_v4NoSetFlagsButContent(void **state) {
    (void) state;
    uint8_t ext[16] = {
        'T', 'I', 'T', '2',
        0x00, 0x00, 0x02, 0x00,
        0x00, 0x00,
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

static void id3v2ParseFrame_parseTALBUTF8(void **state) {
    (void) state;
    uint8_t talb[77] = {
        0x54, 0x41, 0x4c, 0x42, 0x00, 0x00, 0x00, 0x43, 0x00, 0x00,
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
    assert_memory_equal(f->header->id, "TALB", 4);
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

    assert_memory_equal(e->entry, "The Powers That Butf8ÛȾℲⅧ♈ ♉ ♊ ♋ ♌ ♍ ♎ ♏utf8", 66);
    assert_int_equal(e->size, 67);

    listFree(context);
    byteStreamDestroy(stream);
    id3v2DestroyFrame(&f);
}

static void id3v2ParseFrame_parseTIT2UTF16(void **state) {
    (void) state;
    // TIT2
    uint8_t tit2[37] = {
        0x54, 0x49, 0x54, 0x32, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x00,
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
    assert_memory_equal(f->header->id, "TIT2", 4);
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

    assert_memory_equal(e->entry, tit2 + 11, 24);
    assert_int_equal(e->size, 24);

    listFree(context);
    byteStreamDestroy(stream);
    id3v2DestroyFrame(&f);
}

static void id3v2ParseFrame_parseTXXXUTF16(void **state) {
    (void) state;
    // TXXX
    uint8_t txxx[71] = {
        'T', 'X', 'X', 'X', 0x00, 0x00, 0x00, 0x3d, 0x00, 0x00,
        0x01,
        0xff, 0xfe, 'u', 0x00, 't', 0x00, 'f', 0x00, '8', 0x00, 0xdb, 0x00, '>', 0x02, '2', '!', 'g', '!', 0x00, 0x00,
        0xff, 0xfe, 'H', '&', ' ', 0x00, 'I', '&', ' ', 0x00, 'J', '&', ' ', 0x00, 'K', '&', ' ', 0x00, 'L', '&', ' ',
        0x00, 'M', '&', ' ', 0x00, 'N', '&', ' ', 0x00, 'O', '&', 'u', 0x00, 't', 0x00, 'f', 0x00, '8', 0x00
    };


    ByteStream *stream = byteStreamCreate(txxx, 71);
    Id3v2Frame *f;
    List *context = id3v2CreateUserDefinedTextFrameContext();
    uint32_t frameSize = 0;

    frameSize = id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_3, &f);

    assert_non_null(f);
    assert_non_null(f->header);
    assert_memory_equal(f->header->id, "TXXX", 4);
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

    assert_memory_equal(e->entry, txxx + 11, 18);
    assert_int_equal(e->size, 18);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->data;

    assert_memory_equal(e->entry, txxx + (11 + 20), 40);
    assert_int_equal(e->size, 40);

    listFree(context);
    byteStreamDestroy(stream);
    id3v2DestroyFrame(&f);
}

static void id3v2ParseFrame_parseTXXXLatin1(void **state) {
    (void) state;
    // TXXX
    uint8_t txxx[25] = {
        'T', 'X', 'X', 'X', 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00,
        0x00,
        'l', 'a', 'b', 'e', 'l', 0x00,
        'd', 'e', 'a', 'd', ' ', 'a', 'i', 'r'
    };


    ByteStream *stream = byteStreamCreate(txxx, 25);
    Id3v2Frame *f;
    List *context = id3v2CreateUserDefinedTextFrameContext();
    uint32_t frameSize = 0;

    frameSize = id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_4, &f);

    assert_non_null(f);
    assert_non_null(f->header);
    assert_memory_equal(f->header->id, "TXXX", 4);
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

    assert_memory_equal(e->entry, "label", 6);
    assert_int_equal(e->size, 6);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->data;

    assert_memory_equal(e->entry, "dead air", 9);
    assert_int_equal(e->size, 9);

    listFree(context);
    byteStreamDestroy(stream);
    id3v2DestroyFrame(&f);
}

static void id3v2ParseFrame_parseWCOM(void **state) {
    (void) state;
    uint8_t txxx[31] = {
        'W', 'C', 'O', 'M', 0x00, 0x00, 0x00, 0x15, 0x00, 0x00,
        'h', 't', 't', 'p', 's', ':', '/', '/', 'b', 'a', 'n', 'd', 'c', 'a', 'm', 'p', '.', 'c', 'o', 'm', '/'
    };


    ByteStream *stream = byteStreamCreate(txxx, 31);
    Id3v2Frame *f;
    List *context = id3v2CreateURLFrameContext();
    uint32_t frameSize = 0;

    frameSize = id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_4, &f);

    assert_non_null(f);
    assert_non_null(f->header);
    assert_memory_equal(f->header->id, "WCOM", 4);
    assert_int_equal(frameSize, 31);
    assert_false(f->header->unsynchronisation);
    assert_false(f->header->readOnly);
    assert_false(f->header->tagAlterPreservation);
    assert_false(f->header->fileAlterPreservation);

    assert_int_equal(f->header->decompressionSize, 0);
    assert_int_equal(f->header->groupSymbol, 0);
    assert_int_equal(f->header->encryptionSymbol, 0);

    Id3v2ContentEntry *e = (Id3v2ContentEntry *) f->entries->head->data;

    assert_memory_equal(e->entry, "https://bandcamp.com/", 21);
    assert_int_equal(e->size, 22);

    listFree(context);
    byteStreamDestroy(stream);
    id3v2DestroyFrame(&f);
}

static void id3v2ParseFrame_parseWXXUTF16(void **state) {
    (void) state;
    // WXX
    uint8_t wxx[31] = {
        'W', 'X', 'X', 0x00, 0x00, 0x19,
        0x01,
        0xff, 0xfe, 's', 0x00, 't', 0x00, 'o', 0x00, 'r', 0x00, 'e', 0x00, 0x00, 0x00,
        'i', 't', 'u', 'n', 'e', 's', '.', 'c', 'o', 'm'
    };


    ByteStream *stream = byteStreamCreate(wxx, 31);
    Id3v2Frame *f;
    List *context = id3v2CreateUserDefinedURLFrameContext();
    uint32_t frameSize = 0;

    frameSize = id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_2, &f);

    assert_non_null(f);
    assert_non_null(f->header);
    assert_memory_equal(f->header->id, "WXX", 3);
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

static void id3v2ParseFrame_parseCOMLatin1(void **state) {
    (void) state;
    // COM
    uint8_t com[25] = {
        'C', 'O', 'M', 0x00, 0x00, 0x13,
        0x00,
        'e', 'n', 'g',
        'c', 'o', 'm', 'm', 'e', 'n', 't', 0x00,
        't', 'e', 's', 't', 'i', 'n', 'g'
    };


    ByteStream *stream = byteStreamCreate(com, 25);
    Id3v2Frame *f;
    List *context = id3v2CreateCommentFrameContext();
    uint32_t frameSize = 0;

    frameSize = id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_2, &f);

    assert_non_null(f);
    assert_non_null(f->header);
    assert_memory_equal(f->header->id, "COM", 3);
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

static void id3v2ParseFrame_parseIPLLatin1(void **state) {
    (void) state;
    // ipl
    uint8_t ipl[51] = {
        'I', 'P', 'L', 0x00, 0x00, 0x2D,
        0x00,
        'm', 'i', 'x', 'i', 'n', 'g', 0x00,
        'j', 'o', 'h', 'n', 0x00,
        'e', 'n', 'g', 'i', 'n', 'e', 'e', 'r', 'i', 'n', 'g', 0x00,
        'j', 'a', 'n', 'e', 0x00,
        'm', 'a', 's', 't', 'e', 'r', 'i', 'n', 'g', 0x00,
        'b', 'u', 'd', 'd', 'y'
    };


    ByteStream *stream = byteStreamCreate(ipl, 51);
    Id3v2Frame *f;
    List *context = id3v2CreateInvolvedPeopleListFrameContext();
    uint32_t frameSize = 0;

    frameSize = id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_2, &f);

    assert_non_null(f);
    assert_non_null(f->header);
    assert_memory_equal(f->header->id, "IPL", 3);
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

static void id3v2ParseFrame_parseSYLTUTF16(void **state) {
    (void) state;
    // SYLT
    uint8_t sylt[116] = {
        'S', 'Y', 'L', 'T', 0x00, 0x00, 0x00, 0x6A, 0x00, 0x00,
        0x01,
        'e', 'n', 'g',
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
    assert_memory_equal(f->header->id, "SYLT", 4);
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

static void id3v2ParseFrame_parseEQU(void **state) {
    (void) state;
    // EQU
    uint8_t equ[15] = {
        'E', 'Q', 'U', 0x00, 0x00, 0x09,
        0x10, // 16 adjustment bits -> 2 byte volumes
        0x03, 0xe9, // 0000001111101001  inc/dec
        0x40, 0x00,
        0x80, 0x28, // 1000000000101000  inc/dec
        0xfc, 0x00
    };


    ByteStream *stream = byteStreamCreate(equ, 15);
    Id3v2Frame *f;
    List *context = id3v2CreateEqualizationFrameContext(ID3V2_TAG_VERSION_2);
    uint32_t frameSize = 0;

    frameSize = id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_2, &f);

    assert_non_null(f);
    assert_non_null(f->header);
    assert_memory_equal(f->header->id, "EQU", 3);
    assert_int_equal(frameSize, 15);
    assert_false(f->header->unsynchronisation);
    assert_false(f->header->readOnly);
    assert_false(f->header->tagAlterPreservation);
    assert_false(f->header->fileAlterPreservation);

    assert_int_equal(f->header->decompressionSize, 0);
    assert_int_equal(f->header->groupSymbol, 0);
    assert_int_equal(f->header->encryptionSymbol, 0);

    Id3v2ContentEntry *e = (Id3v2ContentEntry *) f->entries->head->data;

    assert_int_equal(((uint8_t *)e->entry)[0], 0x10);
    assert_int_equal(e->size, 1);

    e = (Id3v2ContentEntry *) f->entries->head->next->data;
    assert_int_equal(((uint8_t *)e->entry)[0], 0);
    assert_int_equal(e->size, 1);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->data;
    assert_int_equal(((uint8_t *)e->entry)[0], 0x03);
    assert_int_equal(((uint8_t *)e->entry)[1], 0xe9);
    assert_int_equal(e->size, 2);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->next->data;
    assert_int_equal(((uint8_t *)e->entry)[0], 0x40);
    assert_int_equal(((uint8_t *)e->entry)[1], 0x00);
    assert_int_equal(e->size, 2);

    e = (Id3v2ContentEntry *) f->entries->head->next->next->next->next->data;
    assert_int_equal(((uint8_t *)e->entry)[0], 0x01);
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

/**
 * Walks a frame's entries comparing each against an expected byte string. A NULL expected value
 * means an empty entry.
 */
static void internal_assertEntries(const Id3v2Frame *f, const uint8_t *const *values, const size_t *sizes, size_t count) {
    Node *n = f->entries->head;

    for (size_t i = 0; i < count; i++) {
        assert_non_null(n);
        const Id3v2ContentEntry *e = (const Id3v2ContentEntry *) n->data;

        assert_int_equal(e->size, sizes[i]);
        if (sizes[i] > 0) {
            assert_memory_equal(e->entry, values[i], sizes[i]);
        } else {
            assert_null(e->entry);
        }

        n = n->next;
    }

    assert_null(n);
}

static void id3v2ParseFrame_parseEQUANonByteAlignedBits(void **state) {
    (void) state;
    // 12 adjustment bits are stored in 2 bytes padded at the top
    uint8_t equa[19] = {
        'E', 'Q', 'U', 'A', 0x00, 0x00, 0x00, 0x09, 0x00, 0x00,
        0x0C,
        0x80, 0x64, 0x0f, 0xff, // increment, 100Hz, 0x0fff
        0x01, 0x00, 0x00, 0x01  // decrement, 256Hz, 0x0001
    };
    const uint8_t *values[] = {
        (const uint8_t *) "\x0c", (const uint8_t *) "\x01", (const uint8_t *) "\x00\x64", (const uint8_t *) "\x0f\xff",
        (const uint8_t *) "\x00", (const uint8_t *) "\x01\x00", (const uint8_t *) "\x00\x01"
    };
    const size_t sizes[] = {1, 1, 2, 2, 1, 2, 2};

    Id3v2Frame *f = NULL;
    List *context = id3v2CreateEqualizationFrameContext(ID3V2_TAG_VERSION_3);
    uint32_t frameSize = id3v2ParseFrame(equa, sizeof(equa), context, ID3V2_TAG_VERSION_3, &f);

    assert_non_null(f);
    assert_int_equal(frameSize, sizeof(equa));
    assert_memory_equal(f->header->id, "EQUA", 4);
    internal_assertEntries(f, values, sizes, 7);

    listFree(context);
    id3v2DestroyFrame(&f);
}

static void id3v2ParseFrame_parseRVA(void **state) {
    (void) state;
    uint8_t rva[12] = {
        'R', 'V', 'A', 0x00, 0x00, 0x06,
        0x01,       // right increment, left decrement
        0x08,       // 8 bits per volume description
        0x10, 0x20, // right, left
        0x7f, 0x7e  // peak right, peak left
    };
    const uint8_t *values[] = {
        (const uint8_t *) "\x01", (const uint8_t *) "\x08", (const uint8_t *) "\x10", (const uint8_t *) "\x20",
        (const uint8_t *) "\x7f", (const uint8_t *) "\x7e"
    };
    const size_t sizes[] = {1, 1, 1, 1, 1, 1};

    Id3v2Frame *f = NULL;
    List *context = id3v2CreateRelativeVolumeAdjustmentFrameContext(ID3V2_TAG_VERSION_2);
    uint32_t frameSize = id3v2ParseFrame(rva, sizeof(rva), context, ID3V2_TAG_VERSION_2, &f);

    assert_non_null(f);
    assert_int_equal(frameSize, sizeof(rva));
    assert_memory_equal(f->header->id, "RVA", 3);
    internal_assertEntries(f, values, sizes, 6);

    listFree(context);
    id3v2DestroyFrame(&f);
}

static void id3v2ParseFrame_parseRVADAllChannels(void **state) {
    (void) state;
    uint8_t rvad[36] = {
        'R', 'V', 'A', 'D', 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00,
        0x3f, 0x10,
        0x01, 0x00, 0x02, 0x00, 0x7f, 0xff, 0x7f, 0xfe, // front right/left, peaks
        0x00, 0x10, 0x00, 0x20, 0x10, 0x00, 0x20, 0x00, // back right/left, peaks
        0x00, 0x30, 0x30, 0x00,                         // center, peak
        0x00, 0x40, 0x40, 0x00                          // bass, peak
    };

    Id3v2Frame *f = NULL;
    List *context = id3v2CreateRelativeVolumeAdjustmentFrameContext(ID3V2_TAG_VERSION_3);
    uint32_t frameSize = id3v2ParseFrame(rvad, sizeof(rvad), context, ID3V2_TAG_VERSION_3, &f);

    assert_non_null(f);
    assert_int_equal(frameSize, sizeof(rvad));
    assert_int_equal(listLength(f->entries), 14);

    const uint8_t *values[14];
    size_t sizes[14];
    values[0] = (const uint8_t *) "\x3f";
    sizes[0] = 1;
    values[1] = (const uint8_t *) "\x10";
    sizes[1] = 1;
    for (size_t i = 0; i < 12; i++) {
        values[i + 2] = rvad + 12 + (i * 2);
        sizes[i + 2] = 2;
    }
    internal_assertEntries(f, values, sizes, 14);

    listFree(context);
    id3v2DestroyFrame(&f);
}

static void id3v2ParseFrame_parseRVADFrontOnly(void **state) {
    (void) state;
    // peaks and the optional channels are omitted
    uint8_t rvad[16] = {
        'R', 'V', 'A', 'D', 0x00, 0x00, 0x00, 0x06, 0x00, 0x00,
        0x03, 0x10,
        0x01, 0x00, 0x02, 0x00
    };
    const uint8_t *values[] = {
        (const uint8_t *) "\x03", (const uint8_t *) "\x10", (const uint8_t *) "\x01\x00", (const uint8_t *) "\x02\x00"
    };
    const size_t sizes[] = {1, 1, 2, 2};

    Id3v2Frame *f = NULL;
    List *context = id3v2CreateRelativeVolumeAdjustmentFrameContext(ID3V2_TAG_VERSION_3);
    uint32_t frameSize = id3v2ParseFrame(rvad, sizeof(rvad), context, ID3V2_TAG_VERSION_3, &f);

    assert_non_null(f);
    assert_int_equal(frameSize, sizeof(rvad));
    internal_assertEntries(f, values, sizes, 4);

    listFree(context);
    id3v2DestroyFrame(&f);
}

static void id3v2ParseFrame_parseRVA2PerChannelPeakBits(void **state) {
    (void) state;
    // every channel declares its own peak width: 16 bits, 0 bits (no peak) and 8 bits
    uint8_t rva2[31] = {
        'R', 'V', 'A', '2', 0x00, 0x00, 0x00, 0x15, 0x00, 0x00,
        't', 'r', 'a', 'c', 'k', 0x00,
        0x01, 0x04, 0x00, 0x10, 0x7f, 0xff, // master, +2dB, 16 bit peak
        0x02, 0xfc, 0x00, 0x00,             // front right, -2dB, no peak
        0x03, 0x00, 0x01, 0x08, 0xff        // front left, 8 bit peak
    };

    Id3v2Frame *f = NULL;
    List *context = id3v2CreateRelativeVolumeAdjustmentFrameContext(ID3V2_TAG_VERSION_4);
    uint32_t frameSize = id3v2ParseFrame(rva2, sizeof(rva2), context, ID3V2_TAG_VERSION_4, &f);

    assert_non_null(f);
    assert_int_equal(frameSize, sizeof(rva2));
    assert_memory_equal(f->header->id, "RVA2", 4);
    assert_int_equal(listLength(f->entries), 13);

    const Id3v2ContentEntry *e = (const Id3v2ContentEntry *) f->entries->head->data;
    assert_true(e->size >= 5);
    assert_memory_equal(e->entry, "track", 5);

    // check each channel after the identification
    const uint8_t *values[] = {
        (const uint8_t *) "\x01", (const uint8_t *) "\x04\x00", (const uint8_t *) "\x10", (const uint8_t *) "\x7f\xff",
        (const uint8_t *) "\x02", (const uint8_t *) "\xfc\x00", (const uint8_t *) "\x00", NULL,
        (const uint8_t *) "\x03", (const uint8_t *) "\x00\x01", (const uint8_t *) "\x08", (const uint8_t *) "\xff"
    };
    const size_t sizes[] = {1, 2, 1, 2, 1, 2, 1, 0, 1, 2, 1, 1};
    Node *n = f->entries->head->next;

    for (size_t i = 0; i < 12; i++) {
        assert_non_null(n);
        e = (const Id3v2ContentEntry *) n->data;
        assert_int_equal(e->size, sizes[i]);
        if (sizes[i] > 0) {
            assert_memory_equal(e->entry, values[i], sizes[i]);
        }
        n = n->next;
    }

    listFree(context);
    id3v2DestroyFrame(&f);
}

static void id3v2ParseFrame_parseRVA2LastChannelNoPeak(void **state) {
    (void) state;
    // the frame ends on a bits representing peak of $00, the peak is present but empty
    uint8_t rva2[16] = {
        'R', 'V', 'A', '2', 0x00, 0x00, 0x00, 0x06, 0x00, 0x00,
        'x', 0x00,
        0x01, 0x04, 0x00, 0x00
    };

    Id3v2Frame *f = NULL;
    List *context = id3v2CreateRelativeVolumeAdjustmentFrameContext(ID3V2_TAG_VERSION_4);
    uint32_t frameSize = id3v2ParseFrame(rva2, sizeof(rva2), context, ID3V2_TAG_VERSION_4, &f);

    assert_non_null(f);
    assert_int_equal(frameSize, sizeof(rva2));
    assert_int_equal(listLength(f->entries), 5);

    const Id3v2ContentEntry *e = (const Id3v2ContentEntry *) f->entries->tail->data;
    assert_int_equal(e->size, 0);
    assert_null(e->entry);

    e = (const Id3v2ContentEntry *) f->entries->tail->previous->data;
    assert_int_equal(e->size, 1);
    assert_int_equal(((uint8_t *) e->entry)[0], 0x00);

    listFree(context);
    id3v2DestroyFrame(&f);
}

static void id3v2ParseFrame_parseRVADOmittedPeaksNotEmitted(void **state) {
    (void) state;
    // with 16 bit volume descriptions, running out of data means the remaining fields are omitted
    uint8_t rvad[16] = {
        'R', 'V', 'A', 'D', 0x00, 0x00, 0x00, 0x06, 0x00, 0x00,
        0x03, 0x10, 0x01, 0x00, 0x02, 0x00
    };

    Id3v2Frame *f = NULL;
    List *context = id3v2CreateRelativeVolumeAdjustmentFrameContext(ID3V2_TAG_VERSION_3);

    id3v2ParseFrame(rvad, sizeof(rvad), context, ID3V2_TAG_VERSION_3, &f);

    assert_non_null(f);
    assert_int_equal(listLength(f->entries), 4);

    listFree(context);
    id3v2DestroyFrame(&f);
}

static void id3v2ParseFrame_zeroWidthIterationStops(void **state) {
    (void) state;
    // a custom layout whose repeated block is only a 0 bit wide field must not repeat forever
    uint8_t xzro[14] = {
        'X', 'Z', 'R', 'O', 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
        0x00, 0x11, 0x22, 0x33
    };

    List *context = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                               id3v2CopyContentContext);
    listInsertBack(context, id3v2CreateContentContext(numeric_context, id3v2djb2("adjustment"), 1, 1));
    listInsertBack(context, id3v2CreateContentContext(adjustment_context, id3v2djb2("data"), UINT_MAX, 0));
    listInsertBack(context, id3v2CreateContentContext(iter_context, id3v2djb2("iter"), UINT_MAX, 1));

    Id3v2Frame *f = NULL;
    uint32_t frameSize = id3v2ParseFrame(xzro, sizeof(xzro), context, ID3V2_TAG_VERSION_3, &f);

    assert_non_null(f);
    assert_int_equal(frameSize, sizeof(xzro));
    // the adjustment, the first empty field and one repeat before the guard stops the loop
    assert_int_equal(listLength(f->entries), 3);

    listFree(context);
    id3v2DestroyFrame(&f);
}

static void id3v2ParseFrame_parseEQU2(void **state) {
    (void) state;
    uint8_t equ2[22] = {
        'E', 'Q', 'U', '2', 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00,
        0x01,               // linear
        'e', 'q', 0x00,
        0x00, 0xc8, 0x04, 0x00, // 100Hz, +2dB
        0x07, 0xd0, 0xfc, 0x00  // 1000Hz, -2dB
    };

    Id3v2Frame *f = NULL;
    List *context = id3v2CreateEqualizationFrameContext(ID3V2_TAG_VERSION_4);
    uint32_t frameSize = id3v2ParseFrame(equ2, sizeof(equ2), context, ID3V2_TAG_VERSION_4, &f);

    assert_non_null(f);
    assert_int_equal(frameSize, sizeof(equ2));
    assert_int_equal(listLength(f->entries), 6);

    const uint8_t *values[] = {
        (const uint8_t *) "\x00\xc8", (const uint8_t *) "\x04\x00",
        (const uint8_t *) "\x07\xd0", (const uint8_t *) "\xfc\x00"
    };
    Node *n = f->entries->head;
    const Id3v2ContentEntry *e = (const Id3v2ContentEntry *) n->data;
    assert_int_equal(e->size, 1);
    assert_int_equal(((uint8_t *) e->entry)[0], 0x01);

    n = n->next->next;
    for (size_t i = 0; i < 4; i++) {
        e = (const Id3v2ContentEntry *) n->data;
        assert_int_equal(e->size, 2);
        assert_memory_equal(e->entry, values[i], 2);
        n = n->next;
    }

    listFree(context);
    id3v2DestroyFrame(&f);
}

static void id3v2ParseFrame_parseEncrypted(void **state) {
    (void) state;
    uint8_t txxx[30] = {
        'T', 'X', 'X', 'X', 0x00, 0x00, 0x00, 0x0f, 0x00, 0x05,
        0xff, 0x00, 0x1A, 0xE8, 0x5D, // 1763421 data length indicator before decode
        0x00,
        'l', 'a', 'b', 'e', 'l', 0x00,
        'd', 'e', 'a', 'd', ' ', 'a', 'i', 'r'
    };


    ByteStream *stream = byteStreamCreate(txxx, 30);
    Id3v2Frame *f;
    List *context = id3v2CreateUserDefinedTextFrameContext();

    id3v2ParseFrame(stream->buffer, stream->bufferSize, context, ID3V2_TAG_VERSION_4, &f);


    testFrameHeader(f, "TXXX", 0, 0, 0, 0, byteSyncintDecode(1763421), 0, 0xff);


    testEntry((Id3v2ContentEntry *) f->entries->head->data, 15, (uint8_t *) "\0label\0dead air");

    id3v2DestroyFrame(&f);
    listFree(context);
    byteStreamDestroy(stream);
}


static void id3v2ParseTagFromStream_v3(void **state) {
    (void) state;
    ByteStream *stream = byteStreamFromFile("assets/sorry4dying.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);
    uint8_t encodings[2] = {0, 1};

    assert_non_null(tag);

    assert_non_null(tag->header);
    assert_int_equal(tag->header->flags, 0);
    assert_int_equal(tag->header->majorVersion, 3);
    assert_int_equal(tag->header->minorVersion, 0);
    assert_null(tag->header->extendedHeader);

    // TIT2
    Id3v2Frame *f = (Id3v2Frame *) tag->frames->head->data;
    Id3v2ContentEntry *ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data1[26] = {
        0xff, 0xfe, 's', 0x00, 'o', 0x00, 'r', 0x00, 'r', 0x00, 'y', 0x00, '4',
        0x00, 'd', 0x00, 'y', 0x00, 'i', 0x00, 'n', 0x00, 'g', 0x00, 0x00, 0x00
    };

    testFrameHeader(f, "TIT2", 0, 0, 0, 0, 0, 0, 0);

    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 24, data1);

    // TALB
    f = (Id3v2Frame *) tag->frames->head->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data2[56] = {
        0xff, 0xfe, 'I', 0x00, ' ', 0x00, 'D', 0x00, 'i', 0x00,
        'd', 0x00, 'n', 0x00, '\'', 0x00, 't', 0x00, ' ', 0x00,
        'M', 0x00, 'e', 0x00, 'a', 0x00, 'n', 0x00, ' ', 0x00,
        'T', 0x00, 'o', 0x00, ' ', 0x00, 'H', 0x00, 'a', 0x00,
        'u', 0x00, 'n', 0x00, 't', 0x00, ' ', 0x00, 'Y', 0x00,
        'o', 0x00, 'u', 0x00, 0x00, 0x00
    };

    testFrameHeader(f, "TALB", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 54, data2);


    // TSRC
    f = (Id3v2Frame *) tag->frames->head->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data3[28] = {
        0xff, 0xfe, 'Q', 0x00, 'M', 0x00, 'D', 0x00, 'A', 0x00,
        '7', 0x00, '2', 0x00, '2', 0x00, '1', 0x0, '0', 0x00, '2',
        0x00, '3', 0x00, '6', 0x00, 0x00, 0x00
    };

    testFrameHeader(f, "TSRC", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 26, data3);

    // TCOP
    f = (Id3v2Frame *) tag->frames->head->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data4[298] = {
        0xff, 0xfe, '(', 0x00, 'C', 0x00, ')', 0x00, ' ', 0x00,
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
        'n', 0x00, 'c', 0x00, '.', 0x00, 0x00, 0x00
    };

    testFrameHeader(f, "TCOP", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 296, data4);

    // TCON
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data5[250] = {
        0xff, 0xfe, 'A', 0x00, 'r', 0x00, 't', 0x00, ' ', 0x00,
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
        ' ', 0x00, 'P', 0x00, 'o', 0x00, 'p', 0x00, 0x00, 0x00
    };

    testFrameHeader(f, "TCON", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 248, data5);

    // TYER
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data6[5] = {'2', '0', '2', '2', 0x00};

    testFrameHeader(f, "TYER", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[0]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 5, data6);

    // TRCK
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data7[6] = {'0', '1', '/', '1', '1', 0};

    testFrameHeader(f, "TRCK", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[0]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 6, data7);

    // TPOS
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data8[4] = {'1', '/', '1', 0};

    testFrameHeader(f, "TPOS", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[0]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    assert_int_equal(ce->size, 4);
    assert_memory_equal(ce->entry, data8, 4);

    // TPE1
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data9[18] = {
        0xff, 0xfe, 'Q', 0x00, 'u', 0x00, 'a', 0x00, 'd', 0x00,
        'e', 0x00, 'c', 0x00, 'a', 0x00, 0x00, 0x00
    };

    testFrameHeader(f, "TPE1", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 16, data9);

    // TPE2
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    //same data as TPE2

    testFrameHeader(f, "TPE2", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 16, data9);


    // TCOM
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data10[22] = {
        0xff, 0xfe, 'B', 0x00, 'e', 0x00, 'n', 0x00, ' ', 0x00,
        'L', 0x00, 'a', 0x00, 's', 0x00, 'k', 0x00, 'y', 0x00,
        0x00, 0x00
    };

    testFrameHeader(f, "TCOM", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 20, data10);

    // TXXX 1 label
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data11[14] = {0xff, 0xfe, 'L', 0x00, 'A', 0x00, 'B', 0x00, 'E', 0x00, 'L', 0x00, 0x00, 0x00};
    uint8_t data12[18] = {
        0xff, 0xfe, 'd', 0x00, 'e', 0x00, 'a', 0x00, 'd', 0x00, 'A', 0x00, 'i', 0x00, 'r', 0x00, 0x00, 0x00
    };

    testFrameHeader(f, "TXXX", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 12, data11);

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->data;

    testEntry(ce, 16, data12);

    // TXXX 2 performer
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data13[22] = {
        0xff, 0xfe, 'P', 0x00, 'E', 0x00, 'R', 0x00, 'F', 0x00,
        'O', 0x00, 'R', 0x00, 'M', 0x00, 'E', 0x00, 'R', 0x00,
        0x00, 0x00
    };

    testFrameHeader(f, "TXXX", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 20, data13);

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->data;

    testEntry(ce, 16, data9);

    // TXXX upc
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->next->next->next->next->next->next->next->next->next->
            data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data14[10] = {0xff, 0xfe, 'U', 0x00, 'P', 0x00, 'C', 0x00, 0x00, 0x00};
    uint8_t data15[30] = {
        0xff, 0xfe, '0', 0x00, '1', 0x00, '9', 0x00, '7', 0x00,
        '1', 0x00, '8', 0x00, '7', 0x00, '3', 0x00, '3', 0x00,
        '2', 0x00, '4', 0x00, '3', 0x00, '4', 0x00, 0x00, 0x00
    };

    testFrameHeader(f, "TXXX", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;

    testEntry(ce, 8, data14);

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->data;

    testEntry(ce, 28, data15);

    // APIC
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->next->next->next->next->next->next->next->next->next->
            next->data;
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

static void id3v2ParseTagFromStream_v4(void **state) {
    (void) state;
    ByteStream *stream = byteStreamFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);
    uint8_t encodings[4] = {0, 1, 2, 3};

    assert_non_null(tag);

    assert_non_null(tag->header);
    assert_int_equal(tag->header->flags, 0);
    assert_int_equal(tag->header->majorVersion, 4);
    assert_int_equal(tag->header->minorVersion, 0);

    assert_null(tag->header->extendedHeader);

    // TALB
    Id3v2Frame *f = (Id3v2Frame *) tag->frames->head->data;
    Id3v2ContentEntry *ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data[67] = "The Powers That Butf8ÛȾℲⅧ♈ ♉ ♊ ♋ ♌ ♍ ♎ ♏utf8";

    testFrameHeader(f, "TALB", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[3]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 67, data);

    // TRCK
    f = (Id3v2Frame *) tag->frames->head->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data2[12] = {0xff, 0xfe, '0', 0x00, '9', 0x00, '/', 0x00, '1', 0x00, '0', 0x00};

    testFrameHeader(f, "TRCK", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 12, data2);

    // TPOS
    f = (Id3v2Frame *) tag->frames->head->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data3[8] = {0xff, 0xfe, '2', 0x00, '/', 0x00, '2', 0x00};

    testFrameHeader(f, "TPOS", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 8, data3);

    // TPE1
    f = (Id3v2Frame *) tag->frames->head->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data4[24] = {
        0xff, 0xfe, 'D', 0x00, 'e', 0x00, 'a', 0x00, 't', 0x00,
        'h', 0x00, ' ', 0x00, 'G', 0x00, 'r', 0x00, 'i', 0x00,
        'p', 0x00, 's', 0x00
    };

    testFrameHeader(f, "TPE1", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 24, data4);

    // TIT2
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data5[12] = {
        0xff, 0xfe, 'O', 0x00, 'n', 0x00, ' ', 0x00, 'G', 0x00,
        'P', 0x00
    };

    testFrameHeader(f, "TIT2", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 12, data5);

    // TPE2
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    // same data as TPE1

    testFrameHeader(f, "TPE2", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 24, data4);

    // TCOM
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    // same data as TPE1

    testFrameHeader(f, "TCOM", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 24, data4);

    // APIC
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;

    testFrameHeader(f, "APIC", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[0]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 11, (uint8_t *) "image/jpeg");

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->data;
    testEntry(ce, 1, &(encodings[0])); //used just because encodings[0] == 0

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->next->data;
    testEntry(ce, 5, (uint8_t *) "test");

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->next->next->data;
    assert_non_null(ce);
    assert_int_equal(ce->size, 1845186);

    // APIC
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data6[12] = {0xff, 0xfe, 't', 0x00, 'e', 0x00, 's', 0x00, 't', 0x00, 0x00, 0x00};

    testFrameHeader(f, "APIC", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 11, (uint8_t *) "image/jpeg");

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->data;
    testEntry(ce, 1, &(encodings[3])); //used just because encodings[3] == 3

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->next->data;
    testEntry(ce, 10, data6);

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->next->next->data;
    assert_non_null(ce->entry);
    //assert_int_equal(ce->size, 34785);

    // TDRC
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;

    testFrameHeader(f, "TDRC", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[0]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 5, (uint8_t *) "2015");

    // TCON
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;

    testFrameHeader(f, "TCON", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[0]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 178,
              (uint8_t *)
              "Experimental Hip-Hop, Glitch Hop, Abstract Hip-Hop, Industrial Hip-Hop, Footwork, Wonky, Noise Rock, Rap Rock, Synth Punk, Hardcore Hip-Hop, Experimental Rock & Digital Hardcore");

    // TXXX
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;
    uint8_t data7[20] = {
        0xff, 0xfe, 0x75, 0x00, 0x74, 0x00, 0x66, 0x00, 0x38, 0x00,
        0xdb, 0x00, 0x3e, 0x02, 0x32, 0x21, 0x67, 0x21, 0x00, 0x00
    };
    uint8_t data8[40] = {
        0xff, 0xfe, 'H', '&', ' ', 0x00, 'I', '&', ' ', 0x00,
        'J', '&', ' ', 0x00, 'K', '&', ' ', 0x00, 'L', '&',
        ' ', 0x00, 'M', '&', ' ', 0x00, 'N', '&', ' ', 0x00,
        'O', '&', 'u', 0x00, 't', 0x00, 'f', 0x00, '8', 0x00
    };

    testFrameHeader(f, "TXXX", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &(encodings[1]));

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 18, data7);

    ce = (Id3v2ContentEntry *) f->entries->head->next->next->data;
    testEntry(ce, 40, data8);

    // WCOM
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->next->next->next->next->next->next->next->next->data;
    ce = (Id3v2ContentEntry *) f->entries->head->data;

    testFrameHeader(f, "WCOM", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 45, (uint8_t *) "The Powers That Butf8\xdb>2gH I J K L M N Outf8");

    // ETCO
    f = (Id3v2Frame *) tag->frames->head->next->next->next->next->next->next->next->next->next->next->next->next->next->
            data;
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

static void id3v2ParseTagFromStream_v2unsync(void **state) {
    (void) state;
    // tag body is 18 bytes once decoded, 19 as stored: the $FF at the end of the text
    // was followed by a stuffed $00 that the parser must strip
    uint8_t data[29] = {
        'I', 'D', '3', 0x02, 0x01, 0x80, 0x00, 0x00, 0x00, 0x13,
        'T', 'A', 'L', 0x00, 0x00, 0x0c,
        0x00,
        'F', 'a', 'm', 'i', 'l', 'y', ' ', 'G', 'u', 'y', 0xFF, 0x00
    };

    ByteStream *stream = byteStreamCreate(data, 29);
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);
    uint8_t encoding = 0;

    assert_non_null(tag);
    assert_non_null(tag->header);
    assert_null(tag->header->extendedHeader);
    assert_int_equal(tag->header->flags, 0x80);
    assert_int_equal(tag->header->majorVersion, 2);
    assert_int_equal(tag->header->minorVersion, 1);

    Id3v2Frame *f = (Id3v2Frame *) tag->frames->head->data;
    Id3v2ContentEntry *ce = (Id3v2ContentEntry *) f->entries->head->data;

    testFrameHeader(f, "TAL\0", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &encoding);

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 12, (uint8_t *) "Family Guy\xFF");

    assert_null(f->entries->head->next->next);

    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);
}

// v2.4 frame level unsync: TALB is flagged and stored with a stuffed $00, TIT2 after it
// is not, so this also checks the parser advances by the stored frame size
static void id3v2ParseTagFromStream_v4frameUnsync(void **state) {
    (void) state;
    uint8_t data[46] = {
        'I', 'D', '3', 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24,
        'T', 'A', 'L', 'B', 0x00, 0x00, 0x00, 0x0d, 0x00, 0x02,
        0x00,
        'F', 'a', 'm', 'i', 'l', 'y', ' ', 'G', 'u', 'y', 0xFF, 0x00,
        'T', 'I', 'T', '2', 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
        0x00, 'H', 'i'
    };

    Id3v2Tag *tag = id3v2ParseTagFromBuffer(data, sizeof(data), NULL);
    uint8_t encoding = 0;

    assert_non_null(tag);
    assert_int_equal(tag->header->majorVersion, 4);

    Id3v2Frame *f = (Id3v2Frame *) tag->frames->head->data;
    testFrameHeader(f, "TALB", 1, 0, 0, 0, 0, 0, 0);
    testEntry((Id3v2ContentEntry *) f->entries->head->data, 1, &encoding);
    testEntry((Id3v2ContentEntry *) f->entries->head->next->data, 12, (uint8_t *) "Family Guy\xFF");

    assert_non_null(tag->frames->head->next);
    f = (Id3v2Frame *) tag->frames->head->next->data;
    testFrameHeader(f, "TIT2", 0, 0, 0, 0, 0, 0, 0);
    testEntry((Id3v2ContentEntry *) f->entries->head->next->data, 3, (uint8_t *) "Hi");

    id3v2DestroyTag(&tag);
}

static void id3v2ParseTagFromStream_v3ext(void **state) {
    (void) state;
    uint8_t data[45] = {
        'I', 'D', '3', 0x03, 0x01, 0x40, 0x00, 0x00, 0x00, 0x23, // header
        0x00, 0x00, 0x00, 0x0a, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, // ext, no padding
        'T', 'A', 'L', 'B', 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00,
        0x00,
        'F', 'a', 'm', 'i', 'l', 'y', ' ', 'G', 'u', 'y'
    };

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
    assert_int_equal(tag->header->extendedHeader->padding, 0);

    Id3v2Frame *f = (Id3v2Frame *) tag->frames->head->data;
    Id3v2ContentEntry *ce = (Id3v2ContentEntry *) f->entries->head->data;

    testFrameHeader(f, "TALB", 0, 0, 0, 0, 0, 0, 0);
    testEntry(ce, 1, &encoding);

    ce = (Id3v2ContentEntry *) f->entries->head->next->data;
    testEntry(ce, 11, (uint8_t *) "Family Guy");

    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);
}


static void id3v2ParseTagFromStream_v2ULTWithMissingDesc(void **state) {
    (void) state;
    ByteStream *stream = byteStreamFromFile("assets/danybrown2.mp3");
    Id3v2Tag *tag = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);
    ListIter iter = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;
    Id3v2ContentEntry *ce = NULL;

    while ((f = id3v2FrameTraverse(&iter)) != NULL) {
        if (memcmp(f->header->id, "ULT", ID3V2_FRAME_ID_MAX_SIZE) == 0) {
            testFrameHeader(f, "ULT", 0, 0, 0, 0, 0, 0, 0);

            ce = (Id3v2ContentEntry *) f->entries->head->data;
            testEntry(ce, 1, (uint8_t *) "\x00");

            ce = (Id3v2ContentEntry *) f->entries->head->next->data;
            testEntry(ce, 3, (uint8_t *) "eng");

            ce = (Id3v2ContentEntry *) f->entries->head->next->data;
            testEntry(ce, 3, (uint8_t *) "eng");

            ce = (Id3v2ContentEntry *) f->entries->head->next->next->data;
            testEntry(ce, 1, (uint8_t *) "\x00");

            ce = (Id3v2ContentEntry *) f->entries->head->next->next->next->data;
            testEntry(ce, 279,
                      (uint8_t *)
                      "haBDJHAsbdjkHASBDJahbsdkAHBSDHAbsdHBDUAHSBDUBAUIBFOASIUBDFOIAUBFOIAUWBFOAWBFAOUWEBFUOYBOUBUOBUOboubouboubouboubouboigndoignoisnjgsdfjnglksjdfngslkjfngskdjfnglskdnfgiserugisugnvfkdxjnvxlkjnijxdngixjdhfgoiserhgiusdng spoerijgsoergjnposeirhgposergn reigjosperijgsodfkgkldfmvxc.,vbm");

            break;
        }
    }

    id3v2DestroyTag(&tag);
    byteStreamDestroy(stream);
}

// "ID3" inside audio with the bytes from the reported file (version 0x33) is not a tag
static void id3v2ParseTagFromStream_junkVersionInAudio(void **state) {
    (void) state;
    uint8_t data[] = {
        0xff, 0xfb, 0x90, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, // audio, 9 bytes so the old 3 byte stepping scan reached it
        'I', 'D', '3', 0x33, 0xe6, 0x36, 0x0f, 0xe8, 0xd9, 0x61,
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55
    };

    assert_null(id3v2ParseTagFromBuffer(data, sizeof(data), NULL));
}

// a plausible version but a size byte with its top bit set is not syncsafe, so not a tag
static void id3v2ParseTagFromStream_junkSize(void **state) {
    (void) state;
    uint8_t data[] = {
        'I', 'D', '3', 0x03, 0x00, 0x00, 0x00, 0x00, 0x80, 0x0b,
        'T', 'A', 'L', 'B', 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
    };

    assert_null(id3v2ParseTagFromBuffer(data, sizeof(data), NULL));
}

// undefined flag bits set
static void id3v2ParseTagFromStream_junkFlags(void **state) {
    (void) state;
    uint8_t data[] = {
        'I', 'D', '3', 0x03, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x0b,
        'T', 'A', 'L', 'B', 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
    };

    assert_null(id3v2ParseTagFromBuffer(data, sizeof(data), NULL));
}

// extended header flag set but the extended header size is 0
static void id3v2ParseTagFromStream_junkExtendedHeader(void **state) {
    (void) state;
    uint8_t data[] = {
        'I', 'D', '3', 0x03, 0x00, 0x40, 0x00, 0x00, 0x00, 0x15,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        'T', 'A', 'L', 'B', 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
    };

    assert_null(id3v2ParseTagFromBuffer(data, sizeof(data), NULL));
}

// a legitimate tag after leading data is still found
static void id3v2ParseTagFromStream_validAfterData(void **state) {
    (void) state;
    uint8_t data[] = {
        0xff, 0xfb, 0x90, 0x64,
        'I', 'D', '3', 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d,
        'T', 'A', 'L', 'B', 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 'h', 'i'
    };

    Id3v2Tag *tag = id3v2ParseTagFromBuffer(data, sizeof(data), NULL);
    assert_non_null(tag);
    assert_int_equal(tag->header->majorVersion, 3);
    assert_int_equal(tag->frames->length, 1);
    id3v2DestroyTag(&tag);
}

int main() {
    const struct CMUnitTest tests[] = {

        // id3v2ParseExtendedTagHeader tests
        cmocka_unit_test(id3v2ParseExtendedTagHeader_nullData),

        cmocka_unit_test(id3v2ParseExtendedTagHeader_v2),

        cmocka_unit_test(id3v2ParseExtendedTagHeader_v3),
        cmocka_unit_test(id3v2ParseExtendedTagHeader_v3NoCrc),
        cmocka_unit_test(id3v2ParseExtendedTagHeader_v3Rejected),
        cmocka_unit_test(id3v2ParseExtendedTagHeader_v3PaddingFits),

        cmocka_unit_test(id3v2ParseExtendedTagHeader_v4),
        cmocka_unit_test(id3v2ParseExtendedTagHeader_v4NoRestrictions),
        cmocka_unit_test(id3v2ParseExtendedTagHeader_v4NoCRC),
        cmocka_unit_test(id3v2ParseExtendedTagHeader_v4NoFlags),
        cmocka_unit_test(id3v2ParseExtendedTagHeader_v4Rejected),
        cmocka_unit_test(id3v2ParseExtendedTagHeader_roundTrip),

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
        cmocka_unit_test(id3v2ParseFrame_parseCOMLatin1),
        cmocka_unit_test(id3v2ParseFrame_parseIPLLatin1),
        cmocka_unit_test(id3v2ParseFrame_parseSYLTUTF16),
        cmocka_unit_test(id3v2ParseFrame_parseEQU),
        cmocka_unit_test(id3v2ParseFrame_parseEQUANonByteAlignedBits),
        cmocka_unit_test(id3v2ParseFrame_parseRVA),
        cmocka_unit_test(id3v2ParseFrame_parseRVADAllChannels),
        cmocka_unit_test(id3v2ParseFrame_parseRVADFrontOnly),
        cmocka_unit_test(id3v2ParseFrame_parseRVA2PerChannelPeakBits),
        cmocka_unit_test(id3v2ParseFrame_parseRVA2LastChannelNoPeak),
        cmocka_unit_test(id3v2ParseFrame_parseRVADOmittedPeaksNotEmitted),
        cmocka_unit_test(id3v2ParseFrame_zeroWidthIterationStops),
        cmocka_unit_test(id3v2ParseFrame_parseEQU2),
        cmocka_unit_test(id3v2ParseFrame_parseEncrypted),


        // id3v2ParseTagFromStream
        cmocka_unit_test(id3v2ParseTagFromStream_v3),
        cmocka_unit_test(id3v2ParseTagFromStream_v4),
        cmocka_unit_test(id3v2ParseTagFromStream_v2unsync),
        cmocka_unit_test(id3v2ParseTagFromStream_v3ext),
        cmocka_unit_test(id3v2ParseTagFromStream_v4frameUnsync),
        cmocka_unit_test(id3v2ParseTagFromStream_v2ULTWithMissingDesc),
        cmocka_unit_test(id3v2ParseTagFromStream_junkVersionInAudio),
        cmocka_unit_test(id3v2ParseTagFromStream_junkSize),
        cmocka_unit_test(id3v2ParseTagFromStream_junkFlags),
        cmocka_unit_test(id3v2ParseTagFromStream_junkExtendedHeader),
        cmocka_unit_test(id3v2ParseTagFromStream_validAfterData)

    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
