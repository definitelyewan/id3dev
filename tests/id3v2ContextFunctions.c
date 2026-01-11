/**
 * @file id3v2ContextFunctions.c
 * @author Ewan Jones
 * @brief uint tests for context functions
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
#include <limits.h>
#include "id3v2/id3v2Context.h"
#include "byteInt.h"

static void id3v2CreateContentContext_validStruct(void **state) {
    (void) state;

    Id3v2ContentContext *c = id3v2CreateContentContext(unknown_context, 6712, 10, 1);

    assert_non_null(c);
    assert_int_equal(c->type, unknown_context);
    assert_int_equal(c->key, 6712);
    assert_int_equal(c->max, 10);
    assert_int_equal(c->min, 1);

    free(c);
}


static void id3v2DestroyContentContext_freeStruct(void **state) {
    (void) state;

    Id3v2ContentContext *c = id3v2CreateContentContext(unknown_context, 6712, 10, 1);
    id3v2DestroyContentContext(&c);

    assert_null(c);
}

static void id3v2CreateTextFrameContext_valid(void **state) {
    (void) state;

    List *l = id3v2CreateTextFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("encoding"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("text"));

    listFree(l);
}

static void id3v2CreateUserDefinedTextFrameContext_valid(void **state) {
    (void) state;

    List *l = id3v2CreateUserDefinedTextFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("encoding"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("desc"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("text"));

    listFree(l);
}

static void id3v2CreateURLFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateURLFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, latin1Encoding_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("url"));

    listFree(l);
}

static void id3v2CreateUserDefinedURLFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateUserDefinedURLFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("encoding"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("desc"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, latin1Encoding_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("url"));

    listFree(l);
}

static void id3v2CreateAttachedPictureFrameContext_unknownVersion(void **state) {
    (void) state;
    List *l = id3v2CreateAttachedPictureFrameContext(12312);

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("encoding"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, unknown_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("format"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("type"));

    c = (Id3v2ContentContext *) n->next->next->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("desc"));

    c = (Id3v2ContentContext *) n->next->next->next->next->data;

    assert_int_equal(c->type, binary_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("data"));

    listFree(l);
}

static void id3v2CreateAttachedPictureFrameContext_version2(void **state) {
    (void) state;
    List *l = id3v2CreateAttachedPictureFrameContext(ID3V2_TAG_VERSION_2);

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("encoding"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, noEncoding_context);
    assert_int_equal(c->max, 3);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("format"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("type"));

    c = (Id3v2ContentContext *) n->next->next->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("desc"));

    c = (Id3v2ContentContext *) n->next->next->next->next->data;

    assert_int_equal(c->type, binary_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("data"));

    listFree(l);
}

static void id3v2CreateAttachedPictureFrameContext_version3(void **state) {
    (void) state;
    List *l = id3v2CreateAttachedPictureFrameContext(ID3V2_TAG_VERSION_3);

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("encoding"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, latin1Encoding_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("format"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("type"));

    c = (Id3v2ContentContext *) n->next->next->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("desc"));

    c = (Id3v2ContentContext *) n->next->next->next->next->data;

    assert_int_equal(c->type, binary_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("data"));

    listFree(l);
}

static void id3v2CreateAudioEncryptionContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateAudioEncryptionFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, latin1Encoding_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("identifier"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 2);
    assert_int_equal(c->min, 2);
    assert_int_equal(c->key, id3v2djb2("start"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 2);
    assert_int_equal(c->min, 2);
    assert_int_equal(c->key, id3v2djb2("length"));

    c = (Id3v2ContentContext *) n->next->next->next->data;

    assert_int_equal(c->type, binary_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("data"));

    listFree(l);
}

static void id3v2CreateAudioSeekPointIndexFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateAudioSeekPointIndexFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, binary_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("data"));

    listFree(l);
}

static void id3v2CreateCommentFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateCommentFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("encoding"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, noEncoding_context);
    assert_int_equal(c->max, 3);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("language"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("desc"));

    c = (Id3v2ContentContext *) n->next->next->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("text"));

    listFree(l);
}

static void id3v2CreateCommercialFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateCommercialFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("encoding"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, latin1Encoding_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("price"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, latin1Encoding_context);
    assert_int_equal(c->max, 8);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("date"));

    c = (Id3v2ContentContext *) n->next->next->next->data;

    assert_int_equal(c->type, latin1Encoding_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("url"));

    c = (Id3v2ContentContext *) n->next->next->next->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("type"));

    c = (Id3v2ContentContext *) n->next->next->next->next->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("name"));

    c = (Id3v2ContentContext *) n->next->next->next->next->next->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("desc"));

    c = (Id3v2ContentContext *) n->next->next->next->next->next->next->next->data;

    assert_int_equal(c->type, latin1Encoding_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("format"));

    c = (Id3v2ContentContext *) n->next->next->next->next->next->next->next->next->data;

    assert_int_equal(c->type, binary_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("data"));

    listFree(l);
}

static void id3v2CreateEncryptedMetaFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateEncryptedMetaFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, latin1Encoding_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("identifier"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, latin1Encoding_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("content"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, binary_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("data"));

    listFree(l);
}

static void id3v2CreateMusicCDIdentifierFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateMusicCDIdentifierFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, binary_context);
    assert_int_equal(c->max, 804);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("data"));

    listFree(l);
}

static void id3v2CreatePlayCounterFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreatePlayCounterFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, binary_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("data"));

    listFree(l);
}

static void id3v2CreateEqualizationFrameContext_validVersion2(void **state) {
    (void) state;
    List *l = id3v2CreateEqualizationFrameContext(ID3V2_TAG_VERSION_2);

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("adjustment"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, bit_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("unary"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, bit_context);
    assert_int_equal(c->max, 15);
    assert_int_equal(c->min, 15);
    assert_int_equal(c->key, id3v2djb2("frequency"));

    c = (Id3v2ContentContext *) n->next->next->next->data;

    assert_int_equal(c->type, adjustment_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("volume"));

    c = (Id3v2ContentContext *) n->next->next->next->next->data;

    assert_int_equal(c->type, iter_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("iter"));

    listFree(l);
}

void id3v2CreateEqualizationFrameContext_wrongVersion(void **state) {
    (void) state;
    List *l = id3v2CreateEqualizationFrameContext(123);

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, unknown_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("unknown"));

    listFree(l);
}

void id3v2CreateEqualizationFrameContext_validVersion4(void **state) {
    (void) state;
    List *l = id3v2CreateEqualizationFrameContext(ID3V2_TAG_VERSION_4);

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("symbol"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, latin1Encoding_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("identifier"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 2);
    assert_int_equal(c->min, 2);
    assert_int_equal(c->key, id3v2djb2("volume"));

    c = (Id3v2ContentContext *) n->next->next->next->data;

    assert_int_equal(c->type, iter_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 2);
    assert_int_equal(c->key, id3v2djb2("iter"));

    listFree(l);
}

static void id3v2CreateEventTimingCodesFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateEventTimingCodesFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("symbol"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("type"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 4);
    assert_int_equal(c->min, 4);
    assert_int_equal(c->key, id3v2djb2("stamp"));

    c = (Id3v2ContentContext *) n->next->next->next->data;

    assert_int_equal(c->type, iter_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("iter"));

    listFree(l);
}

static void id3v2CreateGeneralEncapsulatedObjectFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateGeneralEncapsulatedObjectFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("encoding"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, latin1Encoding_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("format"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("name"));

    c = (Id3v2ContentContext *) n->next->next->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("desc"));

    c = (Id3v2ContentContext *) n->next->next->next->next->data;

    assert_int_equal(c->type, binary_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("data"));

    listFree(l);
}

static void id3v2CreateInvolvedPeopleListFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateInvolvedPeopleListFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("encoding"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("name"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("text"));

    c = (Id3v2ContentContext *) n->next->next->next->data;

    assert_int_equal(c->type, iter_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("iter"));

    listFree(l);
}

static void id3v2CreateLinkedInformationFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateLinkedInformationFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, latin1Encoding_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("url"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, noEncoding_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("data"));

    listFree(l);
}

static void id3v2CreateMPEGLocationLookupTableFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateMPEGLocationLookupTableFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, binary_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("data"));

    listFree(l);
}

static void id3v2CreateOwnershipFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateOwnershipFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("encoding"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, latin1Encoding_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("price"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, latin1Encoding_context);
    assert_int_equal(c->max, 8);
    assert_int_equal(c->min, 8);
    assert_int_equal(c->key, id3v2djb2("date"));

    c = (Id3v2ContentContext *) n->next->next->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("name"));

    listFree(l);
}

static void id3v2CreatePopularimeterFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreatePopularimeterFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, latin1Encoding_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("identifier"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("symbol"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, binary_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("data"));

    listFree(l);
}

static void id3v2CreatePositionSynchronisationFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreatePositionSynchronisationFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("format"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 4);
    assert_int_equal(c->min, 4);
    assert_int_equal(c->key, id3v2djb2("stamp"));

    listFree(l);
}

static void id3v2CreatePrivateFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreatePrivateFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, latin1Encoding_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("identifier"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, binary_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("data"));

    listFree(l);
}

static void id3v2CreateRecommendedBufferSizeFrameContext_valid() {
    List *l = id3v2CreateRecommendedBufferSizeFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 3);
    assert_int_equal(c->min, 3);
    assert_int_equal(c->key, id3v2djb2("buffer"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("flag"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 4);
    assert_int_equal(c->min, 0);
    assert_int_equal(c->key, id3v2djb2("offset"));

    listFree(l);
}

static void id3v2CreateRelativeVolumeAdjustmentFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateRelativeVolumeAdjustmentFrameContext(1);

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, binary_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("data"));

    listFree(l);
}

static void id3v2CreateReverbFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateReverbFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 2);
    assert_int_equal(c->min, 2);
    assert_int_equal(c->key, id3v2djb2("left"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 2);
    assert_int_equal(c->min, 2);
    assert_int_equal(c->key, id3v2djb2("right"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 2);
    assert_int_equal(c->min, 2);
    assert_int_equal(c->key, id3v2djb2("right"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("bounce left"));

    c = (Id3v2ContentContext *) n->next->next->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("bounce right"));

    c = (Id3v2ContentContext *) n->next->next->next->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("feedback ll"));

    c = (Id3v2ContentContext *) n->next->next->next->next->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("feedback lr"));

    c = (Id3v2ContentContext *) n->next->next->next->next->next->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("feedback rr"));

    c = (Id3v2ContentContext *) n->next->next->next->next->next->next->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("feedback rl"));

    c = (Id3v2ContentContext *) n->next->next->next->next->next->next->next->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("p left"));

    c = (Id3v2ContentContext *) n->next->next->next->next->next->next->next->next->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("p right"));

    listFree(l);
}

static void id3v2CreateSeekPointIndexFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateSeekFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 4);
    assert_int_equal(c->min, 4);
    assert_int_equal(c->key, id3v2djb2("offset"));

    listFree(l);
}

static void id3v2CreateSignatureFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateSignatureFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("symbol"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, binary_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("data"));

    listFree(l);
}

static void id3v2CreateSynchronisedLyricFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateSynchronisedLyricFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("encoding"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, noEncoding_context);
    assert_int_equal(c->max, 3);
    assert_int_equal(c->min, 3);
    assert_int_equal(c->key, id3v2djb2("language"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("format"));

    c = (Id3v2ContentContext *) n->next->next->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("symbol"));

    c = (Id3v2ContentContext *) n->next->next->next->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("desc"));

    c = (Id3v2ContentContext *) n->next->next->next->next->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("text"));

    c = (Id3v2ContentContext *) n->next->next->next->next->next->next->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 4);
    assert_int_equal(c->min, 4);
    assert_int_equal(c->key, id3v2djb2("stamp"));

    c = (Id3v2ContentContext *) n->next->next->next->next->next->next->next->data;

    assert_int_equal(c->type, iter_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 5);
    assert_int_equal(c->key, id3v2djb2("iter"));
    listFree(l);
}

static void id3v2CreateSynchronisedTempoCodesFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateSynchronisedTempoCodesFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("format"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, binary_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("data"));

    listFree(l);
}

static void id3v2CreateUniqueFileIdentifierFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateUniqueFileIdentifierFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, latin1Encoding_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("url"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, binary_context);
    assert_int_equal(c->max, 64);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("data"));

    listFree(l);
}

static void id3v2CreateTermsOfUseFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateTermsOfUseFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("encoding"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, noEncoding_context);
    assert_int_equal(c->max, 3);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("language"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("text"));

    listFree(l);
}

static void id3v2CreateUnsynchronisedLyricFrameContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateUnsynchronisedLyricFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, numeric_context);
    assert_int_equal(c->max, 1);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("encoding"));

    c = (Id3v2ContentContext *) n->next->data;

    assert_int_equal(c->type, noEncoding_context);
    assert_int_equal(c->max, 3);
    assert_int_equal(c->min, 3);
    assert_int_equal(c->key, id3v2djb2("language"));

    c = (Id3v2ContentContext *) n->next->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("desc"));

    c = (Id3v2ContentContext *) n->next->next->next->data;

    assert_int_equal(c->type, encodedString_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("text"));

    listFree(l);
}

static void id3v2CreateGenericContext_valid(void **state) {
    (void) state;
    List *l = id3v2CreateGenericFrameContext();

    assert_non_null(l);

    Node *n = l->head;

    Id3v2ContentContext *c = (Id3v2ContentContext *) n->data;

    assert_int_equal(c->type, binary_context);
    assert_int_equal(c->max, UINT_MAX);
    assert_int_equal(c->min, 1);
    assert_int_equal(c->key, id3v2djb2("?"));

    listFree(l);
}

static void id3v2ContextSerialize_valid(void **state) {
    (void) state;
    Id3v2ContentContext *cc = id3v2CreateContentContext(iter_context, id3v2djb2("test"), INT16_MAX, 1);
    size_t outl = 0;
    uint8_t *out = id3v2ContextSerialize(cc, &outl);
    uint8_t *start = out;

    assert_non_null(out);
    assert_int_equal(out[0], 7U);
    out++;

    assert_int_equal(id3v2djb2("test"), btost(out, sizeof(size_t)));
    out += sizeof(size_t);

    assert_int_equal(INT16_MAX, btost(out, sizeof(size_t)));
    out += sizeof(size_t);

    assert_int_equal(1, btost(out, sizeof(size_t)));

    id3v2DestroyContentContext(&cc);
    free(start);
}


static void id3v2ContextSerialize_min(void **state) {
    (void) state;

    Id3v2ContentContext *cc = id3v2CreateContentContext(0, 0, 0, 0);
    size_t outl = 0;
    uint8_t *out = id3v2ContextSerialize(cc, &outl);
    uint8_t *start = out;

    assert_non_null(out);
    assert_int_equal(out[0], 0U);
    out++;

    assert_int_equal(0U, btost(out, sizeof(size_t)));
    out += sizeof(size_t);

    assert_int_equal(0U, btost(out, sizeof(size_t)));
    out += sizeof(size_t);

    assert_int_equal(0U, btost(out, sizeof(size_t)));

    id3v2DestroyContentContext(&cc);
    free(start);
}

static void id3v2ContextToJSON_valid(void **state) {
    (void) state;

    Id3v2ContentContext *cc = id3v2CreateContentContext(iter_context, id3v2djb2("test"), INT16_MAX, 1);
    char *json = id3v2ContextToJSON(cc);
    assert_non_null(json);
    assert_string_equal(json, "{\"type\":7,\"key\":6385723493,\"max\":32767,\"min\":1}");

    id3v2DestroyContentContext(&cc);
    free(json);
}


static void id3v2ContextToJSON_min(void **state) {
    (void) state;

    Id3v2ContentContext *cc = id3v2CreateContentContext(0, 0, 0, 0);
    char *json = id3v2ContextToJSON(cc);

    assert_non_null(json);
    assert_string_equal(json, "{\"type\":0,\"key\":0,\"max\":0,\"min\":0}");

    id3v2DestroyContentContext(&cc);
    free(json);
}

static void id3v2ContextToJSON_NULL(void **state) {
    (void) state;

    char *json = id3v2ContextToJSON(NULL);

    assert_non_null(json);
    assert_string_equal(json, "{}");
    free(json);
}


int main() {
    const struct CMUnitTest tests[] = {

        // id3v2CreateContentContext tests
        cmocka_unit_test(id3v2CreateContentContext_validStruct),

        // id3v2DestroyContentContext tests
        cmocka_unit_test(id3v2DestroyContentContext_freeStruct),

        // id3v2CreateTextFrameContext tests
        cmocka_unit_test(id3v2CreateTextFrameContext_valid),

        // id3v2CreateTextFrameContext tests
        cmocka_unit_test(id3v2CreateUserDefinedTextFrameContext_valid),

        // id3v2CreateURLFrameContext tests
        cmocka_unit_test(id3v2CreateURLFrameContext_valid),

        // id3v2CreateUserDefinedURLFrameContext tests
        cmocka_unit_test(id3v2CreateUserDefinedURLFrameContext_valid),

        // id3v2CreateAttachedPictureFrameContext tests
        cmocka_unit_test(id3v2CreateAttachedPictureFrameContext_unknownVersion),
        cmocka_unit_test(id3v2CreateAttachedPictureFrameContext_version2),
        cmocka_unit_test(id3v2CreateAttachedPictureFrameContext_version3),

        // id3v2CreateAudioEncryptionContext tests
        cmocka_unit_test(id3v2CreateAudioEncryptionContext_valid),

        // id3v2CreateAudioSeekPointIndexFrameContext tests
        cmocka_unit_test(id3v2CreateAudioSeekPointIndexFrameContext_valid),

        // id3v2CreateCommentFrameContext tests
        cmocka_unit_test(id3v2CreateCommentFrameContext_valid),

        // id3v2CreateCommercialFrameContext tests
        cmocka_unit_test(id3v2CreateCommercialFrameContext_valid),

        // id3v2CreateEncryptedMetaFrameContext tests
        cmocka_unit_test(id3v2CreateEncryptedMetaFrameContext_valid),

        // id3v2CreateMusicCDIdentifierFrameContext tests
        cmocka_unit_test(id3v2CreateMusicCDIdentifierFrameContext_valid),

        // id3v2CreatePlayCounterFrameContext tests
        cmocka_unit_test(id3v2CreatePlayCounterFrameContext_valid),

        // id3v2CreateEqualizationFrameContext tests
        cmocka_unit_test(id3v2CreateEqualizationFrameContext_wrongVersion),
        cmocka_unit_test(id3v2CreateEqualizationFrameContext_validVersion2),
        cmocka_unit_test(id3v2CreateEqualizationFrameContext_validVersion4),

        // id3v2CreateEventTimingCodesFrameContext tests
        cmocka_unit_test(id3v2CreateEventTimingCodesFrameContext_valid),

        // id3v2CreateGeneralEncapsulatedObjectFrameContext tests
        cmocka_unit_test(id3v2CreateGeneralEncapsulatedObjectFrameContext_valid),

        // id3v2CreateInvolvedPeopleListFrameContext tests
        cmocka_unit_test(id3v2CreateInvolvedPeopleListFrameContext_valid),

        // id3v2CreateLinkedInformationFrameContext tests
        cmocka_unit_test(id3v2CreateLinkedInformationFrameContext_valid),

        // id3v2CreateMPEGLocationLookupTableFrameContext tests
        cmocka_unit_test(id3v2CreateMPEGLocationLookupTableFrameContext_valid),

        // id3v2CreateOwnershipFrameContext_valid tests
        cmocka_unit_test(id3v2CreateOwnershipFrameContext_valid),

        // id3v2CreatePopularimeterFrameContext tests
        cmocka_unit_test(id3v2CreatePopularimeterFrameContext_valid),

        // id3v2CreatePositionSynchronisationFrameContext tests
        cmocka_unit_test(id3v2CreatePositionSynchronisationFrameContext_valid),

        // id3v2CreatePrivateFrameContext tests
        cmocka_unit_test(id3v2CreatePrivateFrameContext_valid),

        //id3v2CreateRecommendedBufferSizeFrameContext tests
        cmocka_unit_test(id3v2CreateRecommendedBufferSizeFrameContext_valid),

        // id3v2CreateRelativeVolumeAdjustmentFrameContext tests
        cmocka_unit_test(id3v2CreateRelativeVolumeAdjustmentFrameContext_valid),

        // id3v2CreateRelativeVolumeAdjustmentFrameContext tests
        cmocka_unit_test(id3v2CreateReverbFrameContext_valid),

        // id3v2CreateSeekFrameContext tests
        cmocka_unit_test(id3v2CreateSeekPointIndexFrameContext_valid),

        // id3v2CreateSignatureFrameContext tests
        cmocka_unit_test(id3v2CreateSignatureFrameContext_valid),

        // id3v2CreateSynchronisedLyricFrameContext tests
        cmocka_unit_test(id3v2CreateSynchronisedLyricFrameContext_valid),

        // id3v2CreateSynchronisedTempoCodesFrameContext tests
        cmocka_unit_test(id3v2CreateSynchronisedTempoCodesFrameContext_valid),

        // id3v2CreateUniqueFileIdentifierFrameContext tests
        cmocka_unit_test(id3v2CreateUniqueFileIdentifierFrameContext_valid),

        // id3v2CreateTermsOfUseFrameContext
        cmocka_unit_test(id3v2CreateTermsOfUseFrameContext_valid),

        // id3v2CreateUnsynchronisedLyricFrameContext tests
        cmocka_unit_test(id3v2CreateUnsynchronisedLyricFrameContext_valid),

        // id3v2CreateGenericContext tests
        cmocka_unit_test(id3v2CreateGenericContext_valid),

        // id3v2ContextToStream tests
        cmocka_unit_test(id3v2ContextSerialize_valid),
        cmocka_unit_test(id3v2ContextSerialize_min),

        // id3v2ContextToJSON tests
        cmocka_unit_test(id3v2ContextToJSON_valid),
        cmocka_unit_test(id3v2ContextToJSON_min),
        cmocka_unit_test(id3v2ContextToJSON_NULL)

    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
