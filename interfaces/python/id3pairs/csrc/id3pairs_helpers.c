#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3pairs_helpers.h"


typedef struct {
    size_t len;
    char data[];
} id3pairs_value;



static void *id3pairs_copy(const void *v) {
    const id3pairs_value *src = v;
    id3pairs_value *dst = malloc(sizeof(*dst) + src->len);
    if (dst != NULL) {
        dst->len = src->len;
        memcpy(dst->data, src->data, src->len);
    }
    return dst;
}

static void id3pairs_delete(void *v) {
    free(v);
}

static char *id3pairs_print(const void *v) {
    const id3pairs_value *val = v;
    char *out = malloc(32);
    if (out != NULL) {
        snprintf(out, 32, "<%zu bytes>", val->len);
    }
    return out;
}

/* The functions Python calls. Not static: cffi's generated code links against them. */

HashTable *id3pairs_create(size_t size) {
    return hashTableCreate(size, id3pairs_delete, id3pairs_print, id3pairs_copy);
}

/* Returns hashTableInsert's result: 0 on success, 1 on failure. */
int id3pairs_insert(HashTable *t, const char *key, const char *data, size_t len) {
    /* hashTableInsert stores its own copy (via id3pairs_copy), so this temp is freed after */
    id3pairs_value *tmp = malloc(sizeof(*tmp) + len);
    if (tmp == NULL) {
        return 1;
    }
    tmp->len = len;
    memcpy(tmp->data, data, len);
    int result = hashTableInsert(t, key, tmp);
    free(tmp);
    return result;
}

int id3pairs_get(HashTable *t, const char *key, const char **data, size_t *len) {
    const id3pairs_value *val = hashTableRetrieve(t, key);
    if (val == NULL) {
        return 0;
    }
    *data = val->data;
    *len = val->len;
    return 1;
}