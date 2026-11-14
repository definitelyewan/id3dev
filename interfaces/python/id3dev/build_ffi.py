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
    #define ID3V2_ENCODING_ISO_8859_1 ...
    #define ID3V2_ENCODING_UTF16LE ...
    #define ID3V2_ENCODING_UTF16BE ...
    #define ID3V2_ENCODING_UTF8 ...
    #define ID3V2_ENCODING_ASCII ...
    #define ID3V2_ENCODING_OTHER ...

    /* --- types --- */
    typedef struct _List List;
    typedef struct _HashTable HashTable;
    typedef struct _Node Node;
    typedef struct Iter { Node *current; } ListIter;

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

    /* id3v1Types.h Genre; values are checked against the real header */
    typedef enum _Genre {
        GENRE_UNKNOWN, BLUES_GENRE, CLASSIC_ROCK_GENRE, COUNTRY_GENRE, DANCE_GENRE, DISCO_GENRE,
        FUNK_GENRE, GRUNGE_GENRE, HIP_HOP_GENRE, JAZZ_GENRE, METAL_GENRE, NEW_AGE_GENRE,
        OLDIES_GENRE, OTHER_GENRE, POP_GENRE, RHYTHM_AND_BLUES_GENRE, RAP_GENRE, REGGAE_GENRE,
        ROCK_GENRE, TECHNO_GENRE, INDUSTRIAL_GENRE, ALTERNATIVE_GENRE, SKA_GENRE, DEATH_METAL_GENRE,
        PRANKS_GENRE, SOUNDTRACK_GENRE, EURO_TECHNO_GENRE, AMBIENT_GENRE, TRIP_HOP_GENRE,
        VOCAL_GENRE, JAZZ_AND_FUNK_GENRE, FUSION_GENRE, TRANCE_GENRE, CLASSICAL_GENRE,
        INSTRUMENTAL_GENRE, ACID_GENRE, HOUSE_GENRE, GAME_GENRE, SOUND_CLIP_GENRE, GOSPEL_GENRE,
        NOISE_GENRE, ALTERNATIVE_ROCK_GENRE, BASS_GENRE, SOUL_GENRE, PUNK_GENRE, SPACE_GENRE,
        MEDITATIVE_GENRE, INSTRUMENTAL_POP_GENRE, INSTRUMENTAL_ROCK_GENRE, ETHNIC_GENRE,
        GOTHIC_GENRE, DARKWAVE_GENRE, TECHNO_INDUSTRIAL_GENRE, ELECTRONIC_GENRE, POP_FOLK_GENRE,
        EURODANCE_GENRE, DREAM_GENRE, SOUTHERN_ROCK_GENRE, COMEDY_GENRE, CULT_GENRE, GANGSTA_GENRE,
        TOP_40_GENRE, CHRISTIAN_RAP_GENRE, POP_FUNK_GENRE, JUNGLE_MUSIC_GENRE, NATIVE_US_GENRE,
        CABARET_GENRE, NEW_WAVE_GENRE, PSYCHEDELIC_GENRE, RAVE_GENRE, SHOWTUNES_GENRE,
        TRAILER_GENRE, LO_FI_GENRE, TRIBAL_GENRE, ACID_PUNK_GENRE, ACID_JAZZ_GENRE, POLKA_GENRE,
        RETRO_GENRE, MUSICAL_GENRE, ROCKNROLL_GENRE, HARD_ROCK_GENRE, FOLK_GENRE, FOLK_ROCK_GENRE,
        NATIONAL_FOLK_GENRE, SWING_GENRE, FAST_FUSION_GENRE, BEBOP_GENRE, LATIN_GENRE,
        REVIVAL_GENRE, CELTIC_GENRE, BLUEGRASS_GENRE, AVANTGARDE_GENRE, GOTHIC_ROCK_GENRE,
        PROGRESSIVE_ROCK_GENRE, PSYCHEDELIC_ROCK_GENRE, SYMPHONIC_ROCK_GENRE, SLOW_ROCK_GENRE,
        BIG_BAND_GENRE, CHORUS_GENRE, EASY_LISTENING_GENRE, ACOUSTIC_GENRE, HUMOUR_GENRE,
        SPEECH_GENRE, CHANSON_GENRE, OPERA_GENRE, CHAMBER_MUSIC_GENRE, SONATA_GENRE, SYMPHONY_GENRE,
        BOOTY_BASS_GENRE, PRIMUS_GENRE, PORN_GROOVE_GENRE, SATIRE_GENRE, SLOW_JAM_GENRE, CLUB_GENRE,
        TANGO_GENRE, SAMBA_GENRE, FOLKLORE_GENRE, BALLAD_GENRE, POWER_BALLAD_GENRE,
        RHYTHMIC_SOUL_GENRE, FREESTYLE_GENRE, DUET_GENRE, PUNK_ROCK_GENRE, DRUM_SOLO_GENRE,
        A_CAPPELLA_GENRE, EURO_HOUSE_GENRE, DANCE_HALL_GENRE, GOA_MUSIC_GENRE, DRUM_AND_BASS_GENRE,
        CLUB_HOUSE_GENRE, HARDCORE_TECHNO_GENRE, TERROR_GENRE, INDIE_GENRE, BRITPOP_GENRE,
        NEGERPUNK_GENRE, POLSK_PUNK_GENRE, BEAT_GENRE, CHRISTIAN_GANGSTA_RAP_GENRE,
        HEAVY_METAL_GENRE, BLACK_METAL_GENRE, CROSSOVER_GENRE, CONTEMPORARY_CHRISTIAN_GENRE,
        CHRISTIAN_ROCK_GENRE, MERENGUE_GENRE, SALSA_GENRE, THRASH_METAL_GENRE, ANIME_GENRE,
        JPOP_GENRE, SYNTHPOP_GENRE, CHRISTMAS_GENRE, ABSTRACT_GENRE, ART_ROCK_GENRE, BAROQUE_GENRE,
        BHANGRA_GENRE, BIG_BEAT_GENRE, BREAKBEAT_GENRE, CHILLOUT_GENRE, DOWNTEMPO_GENRE, DUB_GENRE,
        EBM_GENRE, ECLECTIC_GENRE, ELECTRO_GENRE, ELECTROCLASH_GENRE, EMO_GENRE, EXPERIMENTAL_GENRE,
        GARAGE_GENRE, GLOBAL_GENRE, IDM_GENRE, ILLBIENT_GENRE, INDUSTRO_GOTH_GENRE, JAM_BAND_GENRE,
        KRAUTROCK_GENRE, LEFTFIELD_GENRE, LOUNGE_GENRE, MATH_ROCK_GENRE, NEW_ROMANTIC_GENRE,
        NU_BREAKZ_GENRE, POST_PUNK_GENRE, POST_ROCK_GENRE, PSYTRANCE_GENRE, SHOEGAZE_GENRE,
        SPACE_ROCK_GENRE, TROP_ROCK_GENRE, WORLD_MUSIC_GENRE, NEOCLASSICAL_GENRE, AUDIOBOOK_GENRE,
        AUDIO_THEATRE_GENRE, NEUE_DEUTSCHE_WELLE_GENRE, PODCAST_GENRE, INDIE_ROCK_GENRE,
        G_FUNK_GENRE, DUBSTEP_GENRE, GARAGE_ROCK_GENRE, PSYBIENT_GENRE, ...
    } Genre;

    /* --- memory: every char * / uint8_t * id3dev returns is malloc'd --- */
    void free(void *ptr);

    /* --- id3dev.h: whole-file API --- */
    ID3 *id3Create(Id3v2Tag *id3v2, Id3v1Tag *id3v1);
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

    /* --- tags --- */
    void id3v2DestroyTag(Id3v2Tag **toDelete);
    void id3v1DestroyTag(Id3v1Tag **toDelete);
    char *id3v1GenreFromTable(Genre val);   /* static string: never free */

    /* --- id3v1.h: one ID3v1 tag, ignoring the preferred standard --- */
    char *id3v1ReadTitle(const Id3v1Tag *tag);
    char *id3v1ReadArtist(const Id3v1Tag *tag);
    char *id3v1ReadAlbum(const Id3v1Tag *tag);
    int id3v1ReadYear(const Id3v1Tag *tag);
    char *id3v1ReadComment(const Id3v1Tag *tag);
    Genre id3v1ReadGenre(const Id3v1Tag *tag);
    int id3v1ReadTrack(const Id3v1Tag *tag);
    int id3v1WriteTitle(const char *title, Id3v1Tag *tag);
    int id3v1WriteArtist(const char *artist, Id3v1Tag *tag);
    int id3v1WriteAlbum(const char *album, Id3v1Tag *tag);
    int id3v1WriteYear(int year, Id3v1Tag *tag);
    int id3v1WriteComment(const char *comment, Id3v1Tag *tag);
    int id3v1WriteGenre(Genre genre, Id3v1Tag *tag);
    int id3v1WriteTrack(int track, Id3v1Tag *tag);

    /* --- id3v2.h: one ID3v2 tag, ignoring the preferred standard --- */
    char *id3v2ReadTitle(Id3v2Tag *tag);
    char *id3v2ReadArtist(Id3v2Tag *tag);
    char *id3v2ReadAlbumArtist(Id3v2Tag *tag);
    char *id3v2ReadAlbum(Id3v2Tag *tag);
    char *id3v2ReadYear(Id3v2Tag *tag);
    char *id3v2ReadGenre(Id3v2Tag *tag);
    char *id3v2ReadTrack(Id3v2Tag *tag);
    char *id3v2ReadComposer(Id3v2Tag *tag);
    char *id3v2ReadDisc(Id3v2Tag *tag);
    char *id3v2ReadLyrics(Id3v2Tag *tag);
    char *id3v2ReadComment(Id3v2Tag *tag);
    int id3v2WriteTitle(const char *title, Id3v2Tag *tag);
    int id3v2WriteArtist(const char *artist, Id3v2Tag *tag);
    int id3v2WriteAlbumArtist(const char *albumArtist, Id3v2Tag *tag);
    int id3v2WriteAlbum(const char *album, Id3v2Tag *tag);
    int id3v2WriteYear(const char *year, Id3v2Tag *tag);
    int id3v2WriteGenre(const char *genre, Id3v2Tag *tag);
    int id3v2WriteTrack(const char *track, Id3v2Tag *tag);
    int id3v2WriteDisc(const char *disc, Id3v2Tag *tag);
    int id3v2WriteComposer(const char *composer, Id3v2Tag *tag);
    int id3v2WriteLyrics(const char *lyrics, Id3v2Tag *tag);
    int id3v2WriteComment(const char *comment, Id3v2Tag *tag);

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
    char *id3v2ReadFrameEntryAsChar(ListIter *traverser, size_t *dataSize);
    uint8_t id3v2ReadFrameEntryAsU8(ListIter *traverser);
    uint16_t id3v2ReadFrameEntryAsU16(ListIter *traverser);
    uint32_t id3v2ReadFrameEntryAsU32(ListIter *traverser);
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
    Id3v2Tag *id3py_v2_tag_new(uint8_t version);
    Id3v1Tag *id3py_v1_tag_new(void);
    """
)

# 2. set_source: every .c file compiled into one extension module, id3dev._id3dev.
ffibuilder.set_source(
    "id3dev._id3dev",
    '#include <stdlib.h>\n#include "id3py.h"\n#include "id3v2/id3v2.h"\n#include "id3v2/id3v2Frame.h"\n'
    '#include "id3v2/id3v2TagIdentity.h"\n#include "id3v1/id3v1Parser.h"\n#include "id3v1/id3v1.h"\n',
    sources=[str(p) for p in SOURCES],
    include_dirs=[str(p) for p in INCLUDE_DIRS],
    libraries=[] if sys.platform == "win32" else ["m"],
)

if __name__ == "__main__":
    ffibuilder.compile(tmpdir="build/ffi-check", verbose=True)