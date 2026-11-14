"""! @brief Id3Pairs: frame ID -> Context, passed to id3dev as userPairs."""

from ._context import Context
from ._id3dev import ffi, lib
from ._util import frame_id


class Id3Pairs:
    """! @brief Custom frame wiring: tells id3dev how to read and create frames it doesn't know.

    Maps frame IDs to Context layouts. Pass it to ID3.from_file(pairs=...) to
    parse custom frames into separate entries, and to Frame.new(pairs=...) to
    create them. Without it, id3dev reads an unknown frame as one binary blob.

    Backed by a HashTableLib `HashTable *` (the C `userPairs` argument), freed
    when the Id3Pairs is garbage collected.

    @par Choosing IDs
    When creating a frame, id3dev checks its built-in frames first, then treats
    any ID starting with "T" as a text frame and "W" as a URL frame, and only
    then looks here. Custom IDs starting with "X", "Y" or "Z" (e.g. "XFRM") are
    reserved for experimental frames by the ID3v2 spec and avoid all of that.

    @code{.py}
    pairs = Id3Pairs()
    pairs["XFRM"] = (Context()
                     .add(ContextType.LATIN1, "label", 64, 1)
                     .add(ContextType.BINARY, "data", 64, 1))

    frame = Frame.new("XFRM", 3, pairs)
    frame.set_entries(b"hello", b"\x01\x02\x03")
    tag = ID3.from_file("song.mp3", pairs=pairs)
    @endcode
    """

    def __init__(self, size: int = 8):
        """! @brief Creates an empty set of pairs.
        @param size  starting number of hash table buckets; the table grows as pairs are added
        @throws ValueError if `size` is less than 1
        @throws MemoryError if the C table could not be created
        """
        if size < 1:
            raise ValueError("size must be at least 1")
        table = lib.id3py_pairs_create(size)
        if table == ffi.NULL:
            raise MemoryError("hashTableCreate failed")
        self._table = ffi.gc(table, lib.hashTableFree)

    def __setitem__(self, key: str, context: Context) -> None:
        """! @brief Registers a layout for a frame ID: `pairs["XFRM"] = context`.
        @details The table stores its own copy of `context`, so changing the Context
                 afterwards has no effect here, and the Context stays yours to reuse.
        @param key      frame ID, 3 characters for ID3v2.2 or 4 for ID3v2.3/2.4
        @param context  the frame's layout
        @throws TypeError if `context` is not a Context
        @throws ValueError if `key` is not 3-4 ASCII characters
        @throws KeyError if `key` is already set; a pair can't be replaced
        @throws MemoryError if the pair could not be inserted
        """
        if not isinstance(context, Context):
            raise TypeError("values must be id3dev.Context objects")
        raw = frame_id(key)
        if key in self:
            raise KeyError(f"{key!r} is already set (HashTableLib cannot replace values)")
        if lib.hashTableInsert(self._table, raw, context._ctx) != 0:
            raise MemoryError(f"could not insert {key!r}")

    def __contains__(self, key: object) -> bool:
        """! @brief Whether a frame ID has a layout: `"XFRM" in pairs`.
        @param key  frame ID to look up
        @return True if set; False if not, or if `key` isn't a str
        @throws ValueError if `key` is a str but not 3-4 ASCII characters
        """
        if not isinstance(key, str):
            return False
        return lib.hashTableRetrieve(self._table, frame_id(key)) != ffi.NULL
