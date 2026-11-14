#ifndef ID3PY_H
#define ID3PY_H

#include <stddef.h>
#include "id3dev.h"
#include "id3v2/id3v2Context.h"

// pair creation
HashTable *id3py_pairs_create(size_t size);

//empty callback
List *id3py_context_new(void);

// append one Id3v2ContentContext
int id3py_context_add(List *ctx, Id3v2ContextType type, const char *key, size_t max, size_t min);

// parse from buffer and pairs
ID3 *id3py_from_buffer(uint8_t *buf, size_t len, HashTable *userPairs);

// empty ID3v2 tag (no frames) of version 2, 3 or 4; NULL for any other version or on allocation failure
Id3v2Tag *id3py_v2_tag_new(uint8_t version);

// empty ID3v1 tag: blank text fields, year and track 0, genre OTHER_GENRE; NULL on allocation failure
Id3v1Tag *id3py_v1_tag_new(void);

#endif