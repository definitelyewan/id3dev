"""! @brief ID3: a whole file's tags (ID3v2 and/or ID3v1), backed by the C ID3 struct."""

import json
import os

from ._frame import Frame
from ._genre import Genre
from ._tags import ID3v1Tag, ID3v2Tag
from ._id3dev import ffi, lib
from ._util import frame_id, take_bytes, take_string

# The fields id3dev.h has a read/write pair for: Python name -> C name part.
# Each becomes a property on ID3 (see the loop at the bottom of this file).
_FIELDS = {
    "title": "Title",
    "artist": "Artist",
    "album_artist": "AlbumArtist",
    "album": "Album",
    "year": "Year",
    "track": "Track",
    "composer": "Composer",
    "disc": "Disc",
    "lyrics": "Lyrics",
    "comment": "Comment",
}


# Builds a property that calls id3Read<c_name> / id3Write<c_name>.
# Doxygen can't see properties made at runtime, so they are documented
# by hand in the ID3 class docstring.
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
    """! @brief Every ID3 tag in one file: an ID3v2 tag, an ID3v1 tag, or both.

    Load one with from_file(), or start from scratch with new(); change it, then
    write it into a file with save().
    The C memory is freed when the object is garbage collected, or straight
    away with close() or a `with` block.

    The genre has its own property, which takes a Genre.

    @par Tag fields
    To work on one tag regardless of the preferred standard, use the id3v1 and
    id3v2 properties instead, e.g. `tag.id3v1.title`.

    These properties read and write through id3dev's "preferred standard":
    the ID3v2 tag is used when present, otherwise the ID3v1 tag
    (see id3dev.set_preferred_standard). Reading returns a `str`, or `None`
    when the field isn't set. Assigning a `str` writes it, and raises
    `ValueError` if id3dev rejects it, e.g. when the file has no tags at all.
    | Property       | C functions                          |
    |----------------|--------------------------------------|
    | `title`        | id3ReadTitle / id3WriteTitle         |
    | `artist`       | id3ReadArtist / id3WriteArtist       |
    | `album_artist` | id3ReadAlbumArtist / id3WriteAlbumArtist |
    | `album`        | id3ReadAlbum / id3WriteAlbum         |
    | `year`         | id3ReadYear / id3WriteYear (must be a number for ID3v1) |
    | `track`        | id3ReadTrack / id3WriteTrack (ID3v2 may hold "3/12") |
    | `composer`     | id3ReadComposer / id3WriteComposer   |
    | `disc`         | id3ReadDisc / id3WriteDisc           |
    | `lyrics`       | id3ReadLyrics / id3WriteLyrics       |
    | `comment`      | id3ReadComment / id3WriteComment     |

    @code{.py}
    with ID3.from_file("song.mp3") as tag:
        print(tag.title, tag.artist)
        tag.album = "New Album"
        tag.save("song.mp3")
    @endcode
    """

    def __init__(self, ptr):
        """! @brief Wraps an `ID3 *` returned by id3dev. Use from_file() instead of calling this.
        @param ptr  cffi pointer to an ID3 struct. This object takes ownership and frees it.
        """
        self._ptr = ffi.gc(ptr, _destroy_id3)

    # --- loading and saving ---

    @classmethod
    def from_file(cls, path: "str | os.PathLike", pairs=None) -> "ID3":
        """! @brief Reads the ID3v2 and ID3v1 tags from a file such as an .mp3.
        @param path   path to the file, as a `str` or `os.PathLike`
        @param pairs  optional Id3Pairs describing custom frames, so they are parsed
                      into fields instead of left as raw bytes
        @return a new ID3 object
        @throws FileNotFoundError if `path` is not a file
        @throws ValueError if id3dev returns nothing. With `pairs` this happens when the
                file has no tags; without `pairs` a file with no tags still loads, with
                has_id3v1 and has_id3v2 both False.
        """
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

    @classmethod
    def new(cls, id3v2: "int | None" = lib.ID3V2_TAG_VERSION_3, id3v1: bool = False) -> "ID3":
        """! @brief Creates empty tags in memory, not tied to any file.
        @details Set fields and frames, then write the tags into a file with save().
                 The ID3v2 tag starts with no frames and the ID3v1 tag with blank fields.
        @param id3v2  ID3v2 version to create: 2, 3 or 4 (id3dev.ID3V2_2, ID3V2_3, ID3V2_4),
                      or None for no ID3v2 tag
        @param id3v1  whether to create an ID3v1 tag
        @return a new ID3 object
        @throws ValueError if neither tag is requested or `id3v2` is not 2, 3 or 4
        @throws MemoryError if id3dev could not allocate the tags

        @code{.py}
        tag = ID3.new(id3dev.ID3V2_4)
        tag.title = "Song"
        tag.save("untagged.mp3")
        @endcode
        """
        if id3v2 is None and not id3v1:
            raise ValueError("ID3.new needs an ID3v2 version, id3v1=True, or both")
        v2 = _new_v2(id3v2) if id3v2 is not None else ffi.NULL
        v1 = ffi.NULL
        if id3v1:
            v1 = lib.id3py_v1_tag_new()
            if v1 == ffi.NULL:
                _free_v2(v2)
                raise MemoryError("could not create an ID3v1 tag")
        ptr = lib.id3Create(v2, v1)
        if ptr == ffi.NULL:
            _free_v2(v2)
            _free_v1(v1)
            raise MemoryError("could not create ID3")
        return cls(ptr)

    def save(self, path: "str | os.PathLike") -> None:
        """! @brief Writes the tags into a file, replacing any tags it already has.
        @details Each tag this object holds (ID3v2, ID3v1) is written; the audio is left alone.
        @param path  file to write to, as a `str` or `os.PathLike`
        @throws OSError if id3dev could not write any tag
        """
        if not lib.id3WriteToFile(os.fspath(path).encode(), self._live()):
            raise OSError(f"id3dev could not write tags to {path}")

    def copy(self) -> "ID3":
        """! @brief Makes an independent deep copy (id3Copy).
        @return a new ID3; changing it does not affect this one
        """
        return ID3(lib.id3Copy(self._live()))

    def close(self) -> None:
        """! @brief Frees the C structures now instead of at garbage collection.
        @details Safe to call more than once. Using the object afterwards raises ValueError.
        """
        if self._ptr is not None:
            ptr, self._ptr = self._ptr, None
            ffi.gc(ptr, None)       # detaches the finaliser in place; returns None
            _destroy_id3(ptr)

    def __enter__(self):
        """! @brief Supports `with ID3.from_file(...) as tag:`.
        @return this object
        """
        return self

    def __exit__(self, *exc) -> None:
        """! @brief Calls close() at the end of a `with` block."""
        self.close()

    def __eq__(self, other: object) -> bool:
        """! @brief True when both objects hold equal ID3v1 and ID3v2 tags (id3Compare).
        @param other  object to compare with
        @return whether the tags match; NotImplemented if `other` is not an ID3
        """
        if not isinstance(other, ID3):
            return NotImplemented
        return bool(lib.id3Compare(self._live(), other._live()))

    # --- what's in the file ---

    @property
    def has_id3v1(self) -> bool:
        """! @brief Whether this object holds an ID3v1 tag."""
        return self._live().id3v1 != ffi.NULL

    @property
    def has_id3v2(self) -> bool:
        """! @brief Whether this object holds an ID3v2 tag."""
        return self._live().id3v2 != ffi.NULL

    @property
    def id3v1(self) -> "ID3v1Tag | None":
        """! @brief The ID3v1 tag, read and written directly, or None if there isn't one.
        @details Its fields ignore the preferred standard: `tag.id3v1.title = "x"`
                 always writes the ID3v1 tag.
        """
        return ID3v1Tag(self) if self._live().id3v1 != ffi.NULL else None

    @property
    def id3v2(self) -> "ID3v2Tag | None":
        """! @brief The ID3v2 tag, read and written directly, or None if there isn't one.
        @details Its fields ignore the preferred standard: `tag.id3v2.title = "x"`
                 always writes the ID3v2 tag.
        """
        return ID3v2Tag(self) if self._live().id3v2 != ffi.NULL else None

    @property
    def version(self) -> "str | None":
        """! @brief The ID3v2 version.
        @return "2.2", "2.3" or "2.4", or None when there is no ID3v2 tag
        """
        tag = self._live().id3v2
        if tag == ffi.NULL:
            return None
        return f"2.{tag.header.majorVersion}"

    def add_id3v2(self, version: int = lib.ID3V2_TAG_VERSION_3) -> None:
        """! @brief Adds an empty ID3v2 tag (no frames) to an object that doesn't have one.
        @param version  ID3v2 version: 2, 3 or 4 (id3dev.ID3V2_2, ID3V2_3, ID3V2_4)
        @throws ValueError if there is already an ID3v2 tag or `version` is not 2, 3 or 4
        @throws MemoryError if id3dev could not allocate the tag
        """
        live = self._live()
        if live.id3v2 != ffi.NULL:
            raise ValueError("there is already an ID3v2 tag")
        live.id3v2 = _new_v2(version)

    def add_id3v1(self) -> None:
        """! @brief Adds an empty ID3v1 tag (blank fields) to an object that doesn't have one.
        @throws ValueError if there is already an ID3v1 tag
        @throws MemoryError if id3dev could not allocate the tag
        """
        live = self._live()
        if live.id3v1 != ffi.NULL:
            raise ValueError("there is already an ID3v1 tag")
        v1 = lib.id3py_v1_tag_new()
        if v1 == ffi.NULL:
            raise MemoryError("could not create an ID3v1 tag")
        live.id3v1 = v1

    def id3v2_from_id3v1(self) -> None:
        """! @brief Builds a new ID3v2 tag from the ID3v1 tag (id3ConvertId3v1ToId3v2).
        @details Copies title, artist, album, year, track, genre and comment. The ID3v1 tag
                 is kept. The new tag's version is the preferred standard if that is an
                 ID3v2 version (see id3dev.set_preferred_standard), otherwise 2.3.
        @warning Any existing ID3v2 tag is replaced and its frames are lost. Frame views
                 from `frames` must not be used afterwards.
        @throws ValueError if there is no ID3v1 tag or the conversion fails
        """
        live = self._live()
        if live.id3v1 == ffi.NULL:
            raise ValueError("there is no ID3v1 tag to convert")
        if not lib.id3ConvertId3v1ToId3v2(live):
            raise ValueError("id3dev could not convert the ID3v1 tag")

    def id3v1_from_id3v2(self) -> None:
        """! @brief Builds a new ID3v1 tag from the ID3v2 tag (id3ConvertId3v2ToId3v1).
        @details Copies title, artist, album, year, track, comment and genre, cut to
                 ID3v1's fixed field sizes. The ID3v2 tag is kept.
        @warning Any existing ID3v1 tag is replaced.
        @throws ValueError if there is no ID3v2 tag or the conversion fails
        """
        live = self._live()
        if live.id3v2 == ffi.NULL:
            raise ValueError("there is no ID3v2 tag to convert")
        if not lib.id3ConvertId3v2ToId3v1(live):
            raise ValueError("id3dev could not convert the ID3v2 tag")

    # --- genre ---

    @property
    def genre(self) -> "str | None":
        """! @brief The genre's name, e.g. "Pop", or None when it isn't set (id3ReadGenre).
        @details Assign a Genre (or its number), or a name as a `str`. Which tag is written
                 follows the preferred standard, like the other fields: an ID3v1 tag stores
                 the genre's number, an ID3v2 tag stores its name.
        @code{.py}
        tag.genre = Genre.POP
        tag.genre = "Pop"            # same thing
        tag.genre = "Vaporwave"      # any text works for an ID3v2 tag
        @endcode
        @throws ValueError when writing to an ID3v1 tag and the value isn't an ID3v1 genre,
                or id3dev rejects the write (e.g. no tags at all)
        """
        return take_string(lib.id3ReadGenre(self._live()))

    @genre.setter
    def genre(self, value: "Genre | int | str") -> None:
        live = self._live()
        if _writes_to_id3v1(live):
            code = Genre.from_text(value) if isinstance(value, str) else Genre(value)
            raw = bytes([code])     # id3WriteGenre reads an ID3v1 genre number from the first byte
        else:
            text = value if isinstance(value, str) else Genre(value).text
            raw = text.encode("utf-8")
        if not lib.id3WriteGenre(raw, live):
            raise ValueError("id3dev could not write genre")

    # --- pictures ---

    def picture(self, type_: int = 3) -> "bytes | None":
        """! @brief Returns an embedded picture (APIC/PIC frame) by its picture type.
        @param type_  ID3v2 picture type, 0x00-0x14: 3 is the front cover, 4 the back cover,
                      0 "other". Each type appears at most once in a tag.
        @return the raw image bytes, or None if there is no picture of that type
                or no ID3v2 tag
        """
        size = ffi.new("size_t *")
        return take_bytes(lib.id3ReadPicture(type_, self._live(), size), size[0])

    def set_picture(self, data: bytes, mime: str = "jpeg", type_: int = 3) -> None:
        """! @brief Embeds a picture, replacing the existing picture of the same type.
        @details If the tag has no picture of `type_` yet, a new one is added with that
                 type and an empty description.
        @param data   image bytes: `bytes`, `bytearray` or `memoryview`
        @param mime   image format such as "jpeg" or "png". id3dev adds the "image/"
                      prefix itself, so "image/jpeg" is also accepted and trimmed.
                      ID3v2.2 keeps only the first 3 characters.
        @param type_  ID3v2 picture type, 0x00-0x14 (3 = front cover)
        @throws ValueError if there is no ID3v2 tag, or `data`/`mime` is empty
        """
        if mime.startswith("image/"):
            mime = mime[len("image/"):]
        buf = ffi.from_buffer("uint8_t[]", data)
        if not lib.id3WritePicture(buf, len(data), mime.encode("ascii"), type_, self._live()):
            raise ValueError("id3dev could not write the picture (needs an ID3v2 tag)")

    # --- the whole tag as JSON ---

    def to_json(self) -> str:
        """! @brief Every tag and frame as a JSON string (id3ToJSON).
        @return JSON text with an "ID3v1" and an "ID3v2" object
        """
        return take_string(lib.id3ToJSON(self._live()))

    def as_dict(self) -> dict:
        """! @brief to_json(), decoded into Python dicts and lists.
        @details Read-only: changing the dict does not change the tag.
        @return the decoded JSON
        """
        return json.loads(self.to_json())

    # --- ID3v2 frames ---

    @property
    def frames(self) -> "list[Frame]":
        """! @brief Every ID3v2 frame, in tag order.
        @details The frames are views into this tag, not copies: changes to them change
                 the tag. A view is valid until its frame is removed or this object is closed.
        @return list of Frame views
        @throws ValueError if there is no ID3v2 tag
        """
        tag = self._v2()
        version = tag.header.majorVersion
        it = ffi.new("ListIter *", lib.id3v2CreateFrameTraverser(tag))
        out = []
        while (ptr := lib.id3v2FrameTraverse(it)) != ffi.NULL:
            out.append(Frame(ptr, owner=self, version=version))
        return out

    def frame(self, id_: str) -> "Frame | None":
        """! @brief A copy of the first frame with this ID.
        @details The copy is independent: changing it does not change the tag.
        @param id_  frame ID, e.g. "TIT2" (or "TT2" for ID3v2.2)
        @return a Frame owned by Python, or None if the tag has no such frame
        @throws ValueError if there is no ID3v2 tag or `id_` is not 3-4 ASCII characters
        """
        tag = self._v2()
        ptr = lib.id3v2ReadFrameByID(frame_id(id_), tag)
        if ptr == ffi.NULL:
            return None
        return Frame(ptr, version=tag.header.majorVersion)

    def add_frame(self, frame: Frame) -> None:
        """! @brief Adds a frame to the end of the ID3v2 tag.
        @details The tag takes ownership: `frame` becomes a view into this tag, as if it
                 came from frames.
        @param frame  a Frame from Frame.new() or frame(), not already in a tag
        @throws ValueError if `frame` already belongs to a tag, there is no ID3v2 tag,
                or id3dev refuses the frame
        """
        if frame._owner is not None:
            raise ValueError("this frame already belongs to a tag")
        if not lib.id3v2AttachFrameToTag(self._v2(), frame._ptr):
            raise ValueError(f"could not attach frame {frame.id}")
        ffi.gc(frame._ptr, None)    # the tag frees it now, not Python
        frame._owner = self

    def remove_frames(self, id_: str) -> int:
        """! @brief Removes every frame with this ID.
        @details Views of removed frames must not be used afterwards.
        @param id_  frame ID, e.g. "COMM"
        @return how many frames were removed (0 if none matched)
        @throws ValueError if there is no ID3v2 tag or `id_` is not 3-4 ASCII characters
        """
        count = 0
        while lib.id3v2RemoveFrameByID(frame_id(id_), self._v2()):
            count += 1
        return count

    # --- internals ---
    # Plain comments, not docstrings, so Doxygen leaves these out of the docs.

    # The ID3 * this object wraps; raises once close() has been called.
    def _live(self):
        if self._ptr is None:
            raise ValueError("ID3 is closed")
        return self._ptr

    # The Id3v2Tag *; raises when the file has no ID3v2 tag.
    def _v2(self):
        tag = self._live().id3v2
        if tag == ffi.NULL:
            raise ValueError("this file has no ID3v2 tag")
        return tag


for _name, _c_name in _FIELDS.items():
    setattr(ID3, _name, _field(_name, _c_name))


# Whether id3dev's id3Write* functions will write to the ID3v1 tag.
# Mirrors internal_getSafePrefStd in id3dev.c: the preferred standard, unless that tag is missing.
def _writes_to_id3v1(live) -> bool:
    pref = lib.id3GetPreferredStandard()
    if pref != lib.ID3V1_TAG_VERSION:
        return live.id3v2 == ffi.NULL
    return live.id3v1 != ffi.NULL


# An empty Id3v2Tag *; raises ValueError for a version other than 2, 3 or 4.
def _new_v2(version: int):
    if version not in (lib.ID3V2_TAG_VERSION_2, lib.ID3V2_TAG_VERSION_3, lib.ID3V2_TAG_VERSION_4):
        raise ValueError(f"ID3v2 version must be 2, 3 or 4, not {version!r}")
    v2 = lib.id3py_v2_tag_new(version)
    if v2 == ffi.NULL:
        raise MemoryError("could not create an ID3v2 tag")
    return v2


# Frees a tag that never made it into an ID3 struct.
def _free_v2(ptr) -> None:
    if ptr != ffi.NULL:
        lib.id3v2DestroyTag(ffi.new("Id3v2Tag **", ptr))


def _free_v1(ptr) -> None:
    if ptr != ffi.NULL:
        lib.id3v1DestroyTag(ffi.new("Id3v1Tag **", ptr))


# ffi.gc finaliser: id3Destroy takes an ID3 ** so it can NULL the caller's pointer.
def _destroy_id3(ptr) -> None:
    holder = ffi.new("ID3 **", ptr)
    lib.id3Destroy(holder)
