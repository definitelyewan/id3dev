#include <stdlib.h>
#include "id3py.h"
#include "id3v1/id3v1Parser.h"
#include "id3v2/id3v2Parser.h"

HashTable *id3py_pairs_create(size_t size) {
    return hashTableCreate(size, id3v2DeleteContentContextList, id3v2PrintContentContextList,
                           id3v2CopyContentContextList);
}

List *id3py_context_new(void) {
    return listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                      id3v2CopyContentContext);
}

int id3py_context_add(List *ctx, Id3v2ContextType type, const char *key, size_t max, size_t min) {
    if (ctx == NULL || key == NULL) {
        return 1;
    }

    Id3v2ContentContext *cc = id3v2CreateContentContext(type, id3v2djb2(key), max, min);
    if (cc == NULL) {
        return 1;
    }

    if (!listInsertBack(ctx, cc)) {
        id3v2DeleteContentContext(cc);
        return 1;
    }

    return 0;
}

ID3 *id3py_from_buffer(uint8_t *buf, size_t len, HashTable *userPairs) {
    Id3v2Tag *v2 = NULL;
    Id3v1Tag *v1 = NULL;

    if (buf == NULL || len == 0) {
        return NULL;
    }

    v2 = id3v2ParseTagFromBuffer(buf, len, userPairs);

    // an ID3v1 tag if present
    if (len >= ID3V1_MAX_SIZE && id3v1HasTag(buf + len - ID3V1_MAX_SIZE)) {
        v1 = id3v1TagFromBuffer(buf + len - ID3V1_MAX_SIZE);
    }

    if (v2 == NULL && v1 == NULL) {
        return NULL;
    }

    return id3Create(v2, v1);
}