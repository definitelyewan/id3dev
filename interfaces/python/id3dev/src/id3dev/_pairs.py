"""Id3Pairs: frame ID -> Context, passed to id3dev as userPairs."""

from ._context import Context
from ._id3dev import ffi, lib
from ._util import frame_id


class Id3Pairs:
    """Custom frame wiring: tells id3dev how to read and create frames it doesn't know.

    The C table stores its own copy of each Context (via id3v2CopyContentContextList),
    so the Context you pass in stays yours and nothing is freed twice.
    """

    def __init__(self, size: int = 8):
        if size < 1:
            raise ValueError("size must be at least 1")
        table = lib.id3py_pairs_create(size)
        if table == ffi.NULL:
            raise MemoryError("hashTableCreate failed")
        self._table = ffi.gc(table, lib.hashTableFree)

    def __setitem__(self, key: str, context: Context) -> None:
        if not isinstance(context, Context):
            raise TypeError("values must be id3dev.Context objects")
        raw = frame_id(key)
        if key in self:
            raise KeyError(f"{key!r} is already set (HashTableLib cannot replace values)")
        if lib.hashTableInsert(self._table, raw, context._ctx) != 0:
            raise MemoryError(f"could not insert {key!r}")

    def __contains__(self, key: object) -> bool:
        if not isinstance(key, str):
            return False
        return lib.hashTableRetrieve(self._table, frame_id(key)) != ffi.NULL