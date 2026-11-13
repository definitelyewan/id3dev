from pathlib import Path

from cffi import FFI

HERE = Path(__file__).parent
HTL = HERE / "vendor" / "HashTableLib"
CSRC = HERE / "csrc"

ffibuilder = FFI()

# what python can see
ffibuilder.cdef(
    """
    #define HASH_TABLE_MAX_KEY_SIZE ...

    typedef struct _HashTable {
        size_t size;
        size_t itemCount;
        ...;
    } HashTable;

    void *hashTableRetrieve(HashTable *hashTable, const char *key);
    void hashTableFree(HashTable *hashTable);
    void hashTablePrintf(HashTable *hashTable);

    HashTable *id3pairs_create(size_t size);
    int id3pairs_insert(HashTable *t, const char *key, const char *data, size_t len);
    int id3pairs_get(HashTable *t, const char *key, const char **data, size_t *len);
    """
)

# set_source: what gets compiled into the extension module.
ffibuilder.set_source(
    "id3pairs._id3pairs",
    '#include "id3pairs_helpers.h"',
    sources=[
        str(HTL / "src" / "hashTable.c"),
        str(CSRC / "id3pairs_helpers.c"),
    ],
    include_dirs=[str(HTL / "include"), str(CSRC)],
)

if __name__ == "__main__":
    ffibuilder.compile(tmpdir="build/ffi-check", verbose=True)