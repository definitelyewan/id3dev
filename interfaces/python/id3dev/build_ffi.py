import sys
from pathlib import Path

from cffi import FFI

HERE = Path(__file__).resolve().parent

# setuptools rejects absolute paths, so pip builds get paths relative to this
# folder (pip runs from here). The standalone check below changes into
# build/ffi-check before compiling, so it needs absolute paths instead.
BASE = HERE if __name__ == "__main__" else Path(".")

ID3DEV = BASE / "vendor" / "id3dev"
DEPS = ID3DEV / "id3dependencies"

SOURCES = [
    ID3DEV / "src" / "id3dev.c",
    ID3DEV / "src" / "id3v1" / "id3v1.c",
    ID3DEV / "src" / "id3v1" / "id3v1Parser.c",
    ID3DEV / "src" / "id3v2" / "id3v2.c",
    ID3DEV / "src" / "id3v2" / "id3v2Context.c",
    ID3DEV / "src" / "id3v2" / "id3v2Frame.c",
    ID3DEV / "src" / "id3v2" / "id3v2Parser.c",
    ID3DEV / "src" / "id3v2" / "id3v2TagIdentity.c",
    DEPS / "ByteStream" / "src" / "byteStream.c",
    DEPS / "ByteStream" / "src" / "modules" / "byteEndian.c",
    DEPS / "ByteStream" / "src" / "modules" / "byteInt.c",
    DEPS / "ByteStream" / "src" / "modules" / "byteUnicode.c",
    DEPS / "LinkedListLib" / "src" / "LinkedList.c",
    DEPS / "HashTableLib" / "src" / "hashTable.c",
    BASE / "csrc" / "id3py.c",
    ]

INCLUDE_DIRS = [
    ID3DEV,                               # for "id3dependencies/ByteStream/include/..."
    ID3DEV / "include",                   # for "id3dev.h", "id3v2/id3v2Frame.h"
    DEPS / "ByteStream" / "include",      # ByteStream's own sources use bare names
    DEPS / "LinkedListLib" / "include",
    DEPS / "HashTableLib" / "include",
    BASE / "csrc",
    ]

ffibuilder = FFI()

# 1. cdef: everything Python may touch, written by hand (cdef has no preprocessor).
#    Structs are opaque or partial ("...;") so their layouts come from the real headers.
ffibuilder.cdef(
    """
    /* --- constants --- */
    #define ID3V1_TAG_VERSION ...
    #define ID3V2_TAG_VERSION_2 ...
    #define ID3V2_TAG_VERSION_3 ...
    #define ID3V2_TAG_VERSION_4 ...

    /* --- types --- */
    typedef struct _List List;
    typedef struct _HashTable HashTable;
    typedef struct Iter { ...; } ListIter;

    typedef enum _Id3v2ContextType {
        unknown_context, noEncoding_context, binary_context, encodedString_context,
        latin1Encoding_context, numeric_context, precision_context, bit_context,
        iter_context, adjustment_context, ...
    } Id3v2ContextType;

    typedef struct _Id3v2FrameHeader { uint8_t id[4]; ...; } Id3v2FrameHeader;
    typedef struct _Id3v2Frame { Id3v2FrameHeader *header; ...; } Id3v2Frame;
    typedef struct _Id3v2TagHeader { uint8_t majorVersion; uint8_t minorVersion; ...; } Id3v2TagHeader;
    typedef struct _Id3v2Tag { Id3v2TagHeader *header; ...; } Id3v2Tag;
    typedef struct _Id3v1Tag Id3v1Tag;
    typedef struct _ID3 { Id3v2Tag *id3v2; Id3v1Tag *id3v1; } ID3;

    /* --- memory: every char * / uint8_t * id3dev returns is malloc'd --- */
    void free(void *ptr);

    /* --- id3dev.h: whole-file API --- */
    ID3 *id3FromFile(const char *filePath);
    ID3 *id3Copy(const ID3 *toCopy);
    void id3Destroy(ID3 **toDelete);
    bool id3Compare(const ID3 *metadata1, const ID3 *metadata2);
    bool id3SetPreferredStandard(uint8_t standard);
    uint8_t id3GetPreferredStandard(void);
    bool id3ConvertId3v1ToId3v2(ID3 *metadata);
    bool id3ConvertId3v2ToId3v1(ID3 *metadata);

    char *id3ReadTitle(const ID3 *metadata);
    char *id3ReadArtist(const ID3 *metadata);
    char *id3ReadAlbumArtist(const ID3 *metadata);
    char *id3ReadAlbum(const ID3 *metadata);
    char *id3ReadYear(const ID3 *metadata);
    char *id3ReadGenre(const ID3 *metadata);
    char *id3ReadTrack(const ID3 *metadata);
    char *id3ReadComposer(const ID3 *metadata);
    char *id3ReadDisc(const ID3 *metadata);
    char *id3ReadLyrics(const ID3 *metadata);
    char *id3ReadComment(const ID3 *metadata);
    uint8_t *id3ReadPicture(uint8_t type, const ID3 *metadata, size_t *dataSize);

    int id3WriteTitle(const char *title, ID3 *metadata);
    int id3WriteArtist(const char *artist, ID3 *metadata);
    int id3WriteAlbumArtist(const char *albumArtist, ID3 *metadata);
    int id3WriteAlbum(const char *album, ID3 *metadata);
    int id3WriteYear(const char *year, ID3 *metadata);
    int id3WriteGenre(const char *genre, ID3 *metadata);
    int id3WriteTrack(const char *track, ID3 *metadata);
    int id3WriteDisc(const char *disc, ID3 *metadata);
    int id3WriteComposer(const char *composer, ID3 *metadata);
    int id3WriteLyrics(const char *lyrics, ID3 *metadata);
    int id3WriteComment(const char *comment, ID3 *metadata);
    int id3WritePicture(uint8_t *image, size_t imageSize, const char *kind, uint8_t type, ID3 *metadata);

    char *id3ToJSON(const ID3 *metadata);
    int id3WriteToFile(const char *filePath, const ID3 *metadata);

    /* --- id3v2 frames --- */
    int id3v2ReadTagVersion(const Id3v2Tag *tag);
    int id3v2RemoveFrameByID(const char *id, Id3v2Tag *tag);
    Id3v2Frame *id3v2ReadFrameByID(const char *id, Id3v2Tag *tag);
    Id3v2Frame *id3v2CreateEmptyFrame(const char *id, uint8_t version, HashTable *userPairs);
    void id3v2DestroyFrame(Id3v2Frame **toDelete);
    bool id3v2AttachFrameToTag(Id3v2Tag *tag, Id3v2Frame *frame);
    char *id3v2FrameToJSON(Id3v2Frame *frame, uint8_t version);

    ListIter id3v2CreateFrameTraverser(Id3v2Tag *tag);
    Id3v2Frame *id3v2FrameTraverse(ListIter *traverser);
    ListIter id3v2CreateFrameEntryTraverser(Id3v2Frame *frame);
    void *id3v2ReadFrameEntry(ListIter *traverser, size_t *dataSize);
    bool id3v2WriteFrameEntry(Id3v2Frame *frame, ListIter *entries, size_t entrySize, const void *entry);

    /* --- LinkedListLib / HashTableLib, only what the wrappers need --- */
    void listFree(List *list);
    size_t listLength(List *list);
    void hashTableFree(HashTable *hashTable);
    int hashTableInsert(HashTable *hashTable, const char *key, void *value);
    void *hashTableRetrieve(HashTable *hashTable, const char *key);

    /* --- csrc/id3py.h --- */
    HashTable *id3py_pairs_create(size_t size);
    List *id3py_context_new(void);
    int id3py_context_add(List *ctx, Id3v2ContextType type, const char *key, size_t max, size_t min);
    ID3 *id3py_from_buffer(uint8_t *buf, size_t len, HashTable *userPairs);
    """
)

# 2. set_source: every .c file compiled into one extension module, id3dev._id3dev.
ffibuilder.set_source(
    "id3dev._id3dev",
    '#include <stdlib.h>\n#include "id3py.h"\n#include "id3v2/id3v2.h"\n#include "id3v2/id3v2Frame.h"\n',
    sources=[str(p) for p in SOURCES],
    include_dirs=[str(p) for p in INCLUDE_DIRS],
    libraries=[] if sys.platform == "win32" else ["m"],
)

if __name__ == "__main__":
    ffibuilder.compile(tmpdir="build/ffi-check", verbose=True)