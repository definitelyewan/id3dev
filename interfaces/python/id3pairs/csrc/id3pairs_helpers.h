#ifndef ID3PAIRS_HELPERS_H
#define ID3PAIRS_HELPERS_H

#include <stddef.h>
#include "hashTable.h"

HashTable *id3pairs_create(size_t size);
int id3pairs_insert(HashTable *t, const char *key, const char *data, size_t len);
int id3pairs_get(HashTable *t, const char *key, const char **data, size_t *len);

#endif