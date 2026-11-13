"""Python bindings for HashTableLib."""

from ._id3pairs import ffi, lib

__all__ = ["Id3Pairs"]

# HashTableLib rejects keys longer than HASH_TABLE_MAX_KEY_SIZE (25) characters.
MAX_KEY_BYTES = lib.HASH_TABLE_MAX_KEY_SIZE


def _encode_key(key: str) -> bytes:
    if not isinstance(key, str):
        raise TypeError(f"key must be str, not {type(key).__name__}")
    raw = key.encode("utf-8")
    if len(raw) > MAX_KEY_BYTES:
        raise ValueError(f"key is {len(raw)} bytes; Id3Pairs allows at most {MAX_KEY_BYTES}")
    if b"\0" in raw:
        raise ValueError("key must not contain NUL characters")
    return raw


class Id3Pairs:
    """A string -> bytes table backed by HashTableLib.

    HashTableLib has no remove or replace, so a key can only be set once.
    """

    def __init__(self, size: int = 16):
        if size < 1:
            raise ValueError("size must be at least 1")
        table = lib.id3pairs_create(size)
        if table == ffi.NULL:
            raise MemoryError("Id3Pairs failed")
        # ffi.gc ties the C table's lifetime to this Python object:
        # hashTableFree runs when the wrapper is garbage collected.
        self._table = ffi.gc(table, lib.hashTableFree)

    def __setitem__(self, key: str, value: bytes) -> None:
        raw = _encode_key(key)
        value = bytes(value)
        if key in self:
            raise KeyError(f"{key!r} is already set (Id3Pairs cannot replace values)")
        if lib.id3pairs_insert(self._live(), raw, value, len(value)) != 0:
            raise MemoryError(f"Id3Pairs could not insert {key!r}")

    def __getitem__(self, key: str) -> bytes:
        data = ffi.new("const char **")
        size = ffi.new("size_t *")
        if not lib.id3pairs_get(self._live(), _encode_key(key), data, size):
            raise KeyError(key)
        # data[0] points into memory the table owns: copy it out right away
        return ffi.unpack(data[0], size[0])

    def __contains__(self, key: object) -> bool:
        if not isinstance(key, str):
            return False
        return lib.hashTableRetrieve(self._live(), _encode_key(key)) != ffi.NULL

    def __len__(self) -> int:
        return self._live().itemCount

    def get(self, key: str, default=None):
        try:
            return self[key]
        except KeyError:
            return default

    def close(self) -> None:
        # gc
        if self._table is not None:
            table, self._table = self._table, None
            ffi.gc(table, None)  # detach the finaliser in place (returns None) so it can't run twice
            lib.hashTableFree(table)

    def __enter__(self):
        return self

    def __exit__(self, *exc) -> None:
        self.close()

    def _live(self):
        if self._table is None:
            raise ValueError("table is closed")
        return self._table