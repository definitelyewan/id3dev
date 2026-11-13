"""ID3: a whole file's tags (ID3v2 and/or ID3v1), backed by the C ID3 struct."""

import json
import os

from ._frame import Frame
from ._id3dev import ffi, lib
from ._util import frame_id, take_bytes, take_string

# The fields id3dev.h has a read/write pair for: Python name -> C name part.
_FIELDS = {
    "title": "Title",
    "artist": "Artist",
    "album_artist": "AlbumArtist",
    "album": "Album",
    "year": "Year",
    "genre": "Genre",
    "track": "Track",
    "composer": "Composer",
    "disc": "Disc",
    "lyrics": "Lyrics",
    "comment": "Comment",
}


def _field(name: str, c_name: str) -> property:
    read = getattr(lib, f"id3Read{c_name}")
    write = getattr(lib, f"id3Write{c_name}")

    def getter(self) -> "str | None":
        return take_string(read(self._live()))

    def setter(self, value: str) -> None:
        if not write(value.encode("utf-8"), self._live()):
            raise ValueError(f"id3dev could not write {name}")

    return property(getter, setter, doc=f"The {name.replace('_', ' ')} (id3{c_name}).")


class ID3:
    """Tags read from an audio file. Use ID3.from_file(); write back with save()."""

    def __init__(self, ptr):
        self._ptr = ffi.gc(ptr, _destroy_id3)

    # --- loading and saving ---

    @classmethod
    def from_file(cls, path: "str | os.PathLike", pairs=None) -> "ID3":
        """Read a file's tags. Pass an Id3Pairs to parse custom frames with your contexts."""
        path = os.fspath(path)
        if not os.path.isfile(path):
            raise FileNotFoundError(path)
        if pairs is None:
            ptr = lib.id3FromFile(path.encode())
        else:
            with open(path, "rb") as f:
                data = f.read()
            ptr = lib.id3py_from_buffer(ffi.from_buffer("uint8_t[]", data), len(data), pairs._table)
        if ptr == ffi.NULL:
            raise ValueError(f"no ID3 tags could be read from {path}")
        return cls(ptr)

    def save(self, path: "str | os.PathLike") -> None:
        """Write the tags into `path`, replacing any tags it already has."""
        if not lib.id3WriteToFile(os.fspath(path).encode(), self._live()):
            raise OSError(f"id3dev could not write tags to {path}")

    def copy(self) -> "ID3":
        return ID3(lib.id3Copy(self._live()))

    def close(self) -> None:
        """Free the C structures now instead of at garbage collection."""
        if self._ptr is not None:
            ptr, self._ptr = self._ptr, None
            ffi.gc(ptr, None)       # detaches the finaliser in place; returns None
            _destroy_id3(ptr)

    def __enter__(self):
        return self

    def __exit__(self, *exc) -> None:
        self.close()

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, ID3):
            return NotImplemented
        return bool(lib.id3Compare(self._live(), other._live()))

    # --- what's in the file ---

    @property
    def has_id3v1(self) -> bool:
        return self._live().id3v1 != ffi.NULL

    @property
    def has_id3v2(self) -> bool:
        return self._live().id3v2 != ffi.NULL

    @property
    def version(self) -> "str | None":
        """The ID3v2 version as "2.3" or "2.4", or None without an ID3v2 tag."""
        tag = self._live().id3v2
        if tag == ffi.NULL:
            return None
        return f"2.{tag.header.majorVersion}"

    # --- pictures ---

    def picture(self, type_: int = 3) -> "bytes | None":
        """Image bytes for an APIC picture type (3 = front cover), or None."""
        size = ffi.new("size_t *")
        return take_bytes(lib.id3ReadPicture(type_, self._live(), size), size[0])

    def set_picture(self, data: bytes, mime: str = "image/jpeg", type_: int = 3) -> None:
        buf = ffi.from_buffer("uint8_t[]", data)
        if not lib.id3WritePicture(buf, len(data), mime.encode("ascii"), type_, self._live()):
            raise ValueError("id3dev could not write the picture (needs an ID3v2 tag)")

    # --- the whole tag as JSON ---

    def to_json(self) -> str:
        return take_string(lib.id3ToJSON(self._live()))

    def as_dict(self) -> dict:
        """Every tag and frame, decoded from id3ToJSON. Read-only: changes aren't written back."""
        return json.loads(self.to_json())

    # --- ID3v2 frames ---

    @property
    def frames(self) -> "list[Frame]":
        """Views of every ID3v2 frame. Valid until the frame is removed from this tag."""
        tag = self._v2()
        version = tag.header.majorVersion
        it = ffi.new("ListIter *", lib.id3v2CreateFrameTraverser(tag))
        out = []
        while (ptr := lib.id3v2FrameTraverse(it)) != ffi.NULL:
            out.append(Frame(ptr, owner=self, version=version))
        return out

    def frame(self, id_: str) -> "Frame | None":
        """An independent copy of the first frame with this ID, or None."""
        tag = self._v2()
        ptr = lib.id3v2ReadFrameByID(frame_id(id_), tag)
        if ptr == ffi.NULL:
            return None
        return Frame(ptr, version=tag.header.majorVersion)

    def add_frame(self, frame: Frame) -> None:
        """Attach an owned frame. The tag takes ownership; `frame` becomes a view of it."""
        if frame._owner is not None:
            raise ValueError("this frame already belongs to a tag")
        if not lib.id3v2AttachFrameToTag(self._v2(), frame._ptr):
            raise ValueError(f"could not attach frame {frame.id}")
        ffi.gc(frame._ptr, None)    # the tag frees it now, not Python
        frame._owner = self

    def remove_frames(self, id_: str) -> int:
        """Remove every frame with this ID. Returns how many were removed."""
        count = 0
        while lib.id3v2RemoveFrameByID(frame_id(id_), self._v2()):
            count += 1
        return count

    # --- internals ---

    def _live(self):
        if self._ptr is None:
            raise ValueError("ID3 is closed")
        return self._ptr

    def _v2(self):
        tag = self._live().id3v2
        if tag == ffi.NULL:
            raise ValueError("this file has no ID3v2 tag")
        return tag


for _name, _c_name in _FIELDS.items():
    setattr(ID3, _name, _field(_name, _c_name))


def _destroy_id3(ptr) -> None:
    holder = ffi.new("ID3 **", ptr)
    lib.id3Destroy(holder)