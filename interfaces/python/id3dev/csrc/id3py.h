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

#endif