"""Frame: one ID3v2 frame, either owned by Python or a view into a tag."""

from ._id3dev import ffi, lib
from ._util import frame_id, take_bytes, take_string


class Frame:
    """Wraps an Id3v2Frame *.

    Owned frames (Frame.new, ID3.frame) are freed with id3v2DestroyFrame.
    Views (iterating ID3.frames) belong to their tag: they keep the tag alive
    and never free anything themselves.
    """

    def __init__(self, ptr, *, owner=None, version: int):
        self._version = version
        self._owner = owner          # the ID3 a view belongs to, keeps it alive
        if owner is None:
            self._ptr = ffi.gc(ptr, _destroy_frame)
        else:
            self._ptr = ptr

    @classmethod
    def new(cls, id_: str, version: int = 3, pairs=None) -> "Frame":
        """An empty frame laid out by id3dev's built-in contexts, or by `pairs` for custom IDs."""
        table = pairs._table if pairs is not None else ffi.NULL
        ptr = lib.id3v2CreateEmptyFrame(frame_id(id_), version, table)
        if ptr == ffi.NULL:
            raise ValueError(f"could not create frame {id_!r}")
        return cls(ptr, version=version)

    @property
    def id(self) -> str:
        raw = bytes(ffi.unpack(ffi.cast("char *", self._ptr.header.id), 4))
        return raw.rstrip(b"\0").decode("ascii", errors="replace")

    @property
    def entries(self) -> "list[bytes]":
        """Every content entry as raw bytes, in order."""
        return list(self._iter_entries())

    def _iter_entries(self):
        it = ffi.new("ListIter *", lib.id3v2CreateFrameEntryTraverser(self._ptr))
        size = ffi.new("size_t *")
        while True:
            data = take_bytes(lib.id3v2ReadFrameEntry(it, size), size[0])
            if data is None:
                return
            yield data

    def set_entries(self, *values: bytes) -> None:
        """Write entries in order, starting from the first.

        Each value is clamped by id3dev to its context's min and max size.
        """
        it = ffi.new("ListIter *", lib.id3v2CreateFrameEntryTraverser(self._ptr))
        size = ffi.new("size_t *")
        for value in values:
            value = bytes(value)
            if not value:
                raise ValueError("entries can't be empty; write b'\\x00' instead")
            if not lib.id3v2WriteFrameEntry(self._ptr, it, len(value), value):
                raise ValueError(f"frame {self.id} has no entry for value {value!r}")
            take_bytes(lib.id3v2ReadFrameEntry(it, size), size[0])   # advance the iterator

    def to_json(self) -> str:
        return take_string(lib.id3v2FrameToJSON(self._ptr, self._version))

    def __repr__(self) -> str:
        return f"<Frame {self.id}>"


def _destroy_frame(ptr) -> None:
    holder = ffi.new("Id3v2Frame **", ptr)
    lib.id3v2DestroyFrame(holder)