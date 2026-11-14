"""! @brief Frame: one ID3v2 frame, either owned by Python or a view into a tag."""

import re

from ._id3dev import ffi, lib
from ._util import frame_id, take_bytes, take_string


class Frame:
    """! @brief One ID3v2 frame: an ID plus a list of content entries.

    The entries follow the frame's layout (its Context). For example, a text frame
    like TIT2 has an encoding byte followed by the text, and APIC has an encoding,
    MIME type, picture type, description and image data. Entries are read and
    written as raw bytes.

    Backed by an `Id3v2Frame *`. A Frame is one of two kinds:
    - **owned**: made by Frame.new() or ID3.frame(). Python frees it, unless it
      is passed to ID3.add_frame(), which turns it into a view.
    - **view**: from ID3.frames. The frame belongs to its tag, so changes
      change the tag. The view keeps the ID3 alive but never frees the frame.
      Don't use it after the frame is removed or the ID3 is closed.

    @code{.py}
    title = tag.frame("TIT2")          # an owned copy
    encoding, text = title.entries

    for frame in tag.frames:           # views into the tag
        print(frame.id, frame.entries)

    r = tag.frame("TXXX").reader()     # decode entries one at a time
    encoding, description, text = r.u8(), r.text(), r.text()
    @endcode
    """

    def __init__(self, ptr, *, owner=None, version: int):
        """! @brief Wraps an `Id3v2Frame *`. Use Frame.new(), ID3.frame() or ID3.frames instead.
        @param ptr      cffi pointer to the frame
        @param owner    the ID3 the frame belongs to (a view), or None to take ownership
        @param version  ID3v2 major version: 2, 3 or 4
        """
        self._version = version
        self._owner = owner          # the ID3 a view belongs to, keeps it alive
        if owner is None:
            self._ptr = ffi.gc(ptr, _destroy_frame)
        else:
            self._ptr = ptr

    @classmethod
    def new(cls, id_: str, version: int = 3, pairs=None) -> "Frame":
        """! @brief Creates an empty frame, laid out for its ID (id3v2CreateEmptyFrame).
        @details Each entry starts as a single zero byte; fill them with set_entries(),
                 then add the frame to a tag with ID3.add_frame(). The layout comes from
                 id3dev's built-in frames, or the T (text) and W (URL) defaults, then
                 `pairs`, and otherwise one binary entry.
        @param id_      frame ID, e.g. "TIT2", or "TT2" for ID3v2.2
        @param version  ID3v2 major version: 2, 3 or 4. Match the tag it will be added to.
        @param pairs    Id3Pairs with layouts for custom frame IDs
        @return a new owned Frame
        @throws ValueError if `id_` is not 3-4 ASCII characters or id3dev returns nothing
        """
        table = pairs._table if pairs is not None else ffi.NULL
        ptr = lib.id3v2CreateEmptyFrame(frame_id(id_), version, table)
        if ptr == ffi.NULL:
            raise ValueError(f"could not create frame {id_!r}")
        return cls(ptr, version=version)

    @property
    def id(self) -> str:
        """! @brief The frame ID such as "TIT2", or 3 characters like "TT2" for ID3v2.2."""
        raw = bytes(ffi.unpack(ffi.cast("char *", self._ptr.header.id), 4))
        return raw.rstrip(b"\0").decode("ascii", errors="replace")

    @property
    def entries(self) -> "list[bytes]":
        """! @brief Every content entry as raw bytes, in order.
        @details Each call returns fresh copies; changing them doesn't change the frame.
                 Text is returned as stored, including its encoding's terminator if any.
        @return list of entries
        """
        return list(self._iter_entries())

    def reader(self) -> "EntryReader":
        """! @brief Starts reading this frame's entries in order, decoding each as you go.
        @return an EntryReader positioned at the first entry
        """
        return EntryReader(self)

    # Yields a copy of each entry; id3v2ReadFrameEntry returns NULL after the last one.
    def _iter_entries(self):
        it = ffi.new("ListIter *", lib.id3v2CreateFrameEntryTraverser(self._ptr))
        size = ffi.new("size_t *")
        while True:
            data = take_bytes(lib.id3v2ReadFrameEntry(it, size), size[0])
            if data is None:
                return
            yield data

    def set_entries(self, *values: bytes) -> None:
        """! @brief Overwrites entries in order, starting from the first.
        @details Entries after the last value are left as they are, so to change the
                 third entry the first two must be passed too. Values longer than their
                 field's max size are cut short; shorter than its min size are padded
                 with zero bytes.
        @param values  one value per entry: bytes-like, or an int 0-255 (such as an
                       Encoding) written as a single byte. Values can't be empty;
                       write b"\x00" for an empty field.
        @throws ValueError if a value is empty, or there are more values than entries
        """
        it = ffi.new("ListIter *", lib.id3v2CreateFrameEntryTraverser(self._ptr))
        size = ffi.new("size_t *")
        for value in values:
            value = bytes([value]) if isinstance(value, int) else bytes(value)
            if not value:
                raise ValueError("entries can't be empty; write b'\\x00' instead")
            if not lib.id3v2WriteFrameEntry(self._ptr, it, len(value), value):
                raise ValueError(f"frame {self.id} has no entry for value {value!r}")
            take_bytes(lib.id3v2ReadFrameEntry(it, size), size[0])   # advance the iterator

    def to_json(self) -> str:
        """! @brief The frame as a JSON string (id3v2FrameToJSON): its header flags and entries.
        @return JSON text
        """
        return take_string(lib.id3v2FrameToJSON(self._ptr, self._version))

    def __repr__(self) -> str:
        """! @brief `<Frame TIT2>`"""
        return f"<Frame {self.id}>"


# id3v2ReadFrameEntryAsChar escapes these two characters for JSON; undo exactly that.
_UNESCAPE = re.compile(r'\\(["\\])')


class EntryReader:
    """! @brief Reads a frame's entries one at a time, in order (id3v2ReadFrameEntryAs*).

    Each read decodes the current entry and moves on to the next, like the C
    id3v2ReadFrameEntryAs* functions on a ListIter. Reading past the last entry
    raises IndexError. Get one from Frame.reader().

    @code{.py}
    r = frame.reader()             # a TXXX frame
    r.u8()                         # 3 (Encoding.UTF8)
    r.text()                       # "producer"
    r.text()                       # "Finneas O'Connell"
    r.remaining                    # False
    @endcode
    """

    def __init__(self, frame: Frame):
        """! @brief Starts at the first entry of `frame`. Use Frame.reader() instead.
        @param frame  the frame to read; kept alive while reading
        """
        self._frame = frame
        self._it = ffi.new("ListIter *", lib.id3v2CreateFrameEntryTraverser(frame._ptr))

    @property
    def remaining(self) -> bool:
        """! @brief Whether there are entries left to read."""
        return self._it.current != ffi.NULL

    def text(self) -> str:
        """! @brief Reads the entry as text (id3v2ReadFrameEntryAsChar).
        @details id3dev detects UTF-16 (by its byte order mark), UTF-8 and Latin-1 and
                 converts to UTF-8. Reading stops at the first NUL, so an entry holding
                 several strings returns only the first.
        @return the decoded text
        @throws IndexError if there are no entries left
        @throws ValueError if id3dev could not decode the entry
        """
        self._check()
        size = ffi.new("size_t *")
        ptr = lib.id3v2ReadFrameEntryAsChar(self._it, size)
        if ptr == ffi.NULL:
            raise ValueError("id3dev could not decode this entry as text")
        try:
            raw = ffi.string(ptr).decode("utf-8", errors="replace")
        finally:
            lib.free(ptr)
        return _UNESCAPE.sub(r"\1", raw)

    def u8(self) -> int:
        """! @brief Reads the entry's first byte as an unsigned integer (id3v2ReadFrameEntryAsU8).
        @return 0-255
        @throws IndexError if there are no entries left
        """
        self._check()
        return lib.id3v2ReadFrameEntryAsU8(self._it)

    def u16(self) -> int:
        """! @brief Reads the entry's first 2 bytes as an unsigned integer (id3v2ReadFrameEntryAsU16).
        @details Big-endian, like u32(). A 1-byte entry is returned as is.
        @return 0-65535
        @throws IndexError if there are no entries left
        """
        self._check()
        return lib.id3v2ReadFrameEntryAsU16(self._it)

    def u32(self) -> int:
        """! @brief Reads the entry's first 4 bytes as an unsigned integer (id3v2ReadFrameEntryAsU32).
        @details Big-endian. Shorter entries use the bytes they have.
        @return 0-4294967295
        @throws IndexError if there are no entries left
        """
        self._check()
        return lib.id3v2ReadFrameEntryAsU32(self._it)

    def bytes(self) -> bytes:
        """! @brief Reads the entry as raw bytes, undecoded (id3v2ReadFrameEntry).
        @return a copy of the entry
        @throws IndexError if there are no entries left
        """
        self._check()
        size = ffi.new("size_t *")
        return take_bytes(lib.id3v2ReadFrameEntry(self._it, size), size[0])

    def skip(self) -> None:
        """! @brief Moves past the current entry without decoding it.
        @throws IndexError if there are no entries left
        """
        self.bytes()

    # IndexError at the end, where the C readers would return 0 or NULL
    def _check(self) -> None:
        if self._it.current == ffi.NULL:
            raise IndexError("no entries left in this frame")


# ffi.gc finaliser for owned frames: id3v2DestroyFrame takes an Id3v2Frame ** so it can NULL it.
def _destroy_frame(ptr) -> None:
    holder = ffi.new("Id3v2Frame **", ptr)
    lib.id3v2DestroyFrame(holder)
