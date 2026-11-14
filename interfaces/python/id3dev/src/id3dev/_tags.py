"""! @brief ID3v1Tag and ID3v2Tag: direct access to one tag, ignoring the preferred standard."""

from ._genre import Genre
from ._id3dev import ffi, lib
from ._util import take_string


class ID3v1Tag:
    """! @brief The ID3v1 tag of an ID3, read and written directly (id3v1Read* / id3v1Write*).

    Unlike the fields on ID3, these always use the ID3v1 tag, whatever the
    preferred standard is. Get one from ID3.id3v1; it stays tied to that ID3.

    @par Fields
    | Property  | Type        | C functions                       |
    |-----------|-------------|-----------------------------------|
    | `title`   | `str`       | id3v1ReadTitle / id3v1WriteTitle  |
    | `artist`  | `str`       | id3v1ReadArtist / id3v1WriteArtist |
    | `album`   | `str`       | id3v1ReadAlbum / id3v1WriteAlbum  |
    | `comment` | `str`       | id3v1ReadComment / id3v1WriteComment |
    | `year`    | `int`       | id3v1ReadYear / id3v1WriteYear    |
    | `track`   | `int`       | id3v1ReadTrack / id3v1WriteTrack  |
    | `genre`   | Genre       | id3v1ReadGenre / id3v1WriteGenre  |

    Text fields read as '' when unset and hold at most 30 bytes; id3dev cuts
    longer values short. Writes that id3dev rejects raise ValueError.

    @code{.py}
    v1 = tag.id3v1
    v1.title = "Happier Than Ever"
    v1.year = 2021
    v1.genre = Genre.POP
    @endcode
    """

    def __init__(self, owner):
        """! @brief A view of `owner`'s ID3v1 tag. Use ID3.id3v1 instead.
        @param owner  the ID3 holding the tag
        """
        self._owner = owner

    @property
    def year(self) -> int:
        """! @brief The year as a number, 0 when unset (id3v1ReadYear / id3v1WriteYear)."""
        return lib.id3v1ReadYear(self._tag())

    @year.setter
    def year(self, value: int) -> None:
        _check(lib.id3v1WriteYear(int(value), self._tag()), "year")

    @property
    def track(self) -> int:
        """! @brief The track number, 0 when unset (id3v1ReadTrack / id3v1WriteTrack).
        @details Saved only when it is 1-255, as ID3v1.1 stores it in one byte.
        """
        return lib.id3v1ReadTrack(self._tag())

    @track.setter
    def track(self, value: int) -> None:
        _check(lib.id3v1WriteTrack(int(value), self._tag()), "track")

    @property
    def genre(self) -> "Genre | None":
        """! @brief The genre, or None if the stored number isn't a known genre (id3v1ReadGenre).
        @details Assign a Genre, its number, or its name such as "Pop" (id3v1WriteGenre).
        @throws ValueError on assigning a value that isn't an ID3v1 genre
        """
        value = lib.id3v1ReadGenre(self._tag())
        try:
            return Genre(value)
        except ValueError:
            return None

    @genre.setter
    def genre(self, value: "Genre | int | str") -> None:
        genre = Genre.from_text(value) if isinstance(value, str) else Genre(value)
        _check(lib.id3v1WriteGenre(genre, self._tag()), "genre")

    def __repr__(self) -> str:
        """! @brief `<ID3v1Tag 'title' by 'artist'>`"""
        return f"<ID3v1Tag {self.title!r} by {self.artist!r}>"

    # The live Id3v1Tag *, looked up each time so a replaced tag is never stale.
    def _tag(self):
        tag = self._owner._live().id3v1
        if tag == ffi.NULL:
            raise ValueError("this ID3 no longer has an ID3v1 tag")
        return tag


class ID3v2Tag:
    """! @brief The ID3v2 tag of an ID3, read and written directly (id3v2Read* / id3v2Write*).

    Unlike the fields on ID3, these always use the ID3v2 tag, whatever the
    preferred standard is. Get one from ID3.id3v2; it stays tied to that ID3.
    Frames, pictures and JSON stay on ID3, which already works on the ID3v2 tag.

    @par Fields
    Every field reads as a `str`, or None when its frame is missing, and is
    written as a `str`; writes that id3dev rejects raise ValueError.
    `genre` also accepts a Genre, stored as its name.
    | Property       | C functions                                |
    |----------------|--------------------------------------------|
    | `title`        | id3v2ReadTitle / id3v2WriteTitle (TIT2)    |
    | `artist`       | id3v2ReadArtist / id3v2WriteArtist (TPE1)  |
    | `album_artist` | id3v2ReadAlbumArtist / id3v2WriteAlbumArtist (TPE2) |
    | `album`        | id3v2ReadAlbum / id3v2WriteAlbum (TALB)    |
    | `year`         | id3v2ReadYear / id3v2WriteYear             |
    | `track`        | id3v2ReadTrack / id3v2WriteTrack (TRCK)    |
    | `composer`     | id3v2ReadComposer / id3v2WriteComposer (TCOM) |
    | `disc`         | id3v2ReadDisc / id3v2WriteDisc (TPOS)      |
    | `lyrics`       | id3v2ReadLyrics / id3v2WriteLyrics (USLT)  |
    | `comment`      | id3v2ReadComment / id3v2WriteComment (COMM) |
    | `genre`        | id3v2ReadGenre / id3v2WriteGenre (TCON)    |

    @code{.py}
    v2 = tag.id3v2
    v2.title = "Happier Than Ever"
    v2.track = "15/16"
    v2.genre = Genre.POP
    @endcode
    """

    def __init__(self, owner):
        """! @brief A view of `owner`'s ID3v2 tag. Use ID3.id3v2 instead.
        @param owner  the ID3 holding the tag
        """
        self._owner = owner

    @property
    def version(self) -> int:
        """! @brief The major version: 2, 3 or 4."""
        return self._tag().header.majorVersion

    @property
    def genre(self) -> "str | None":
        """! @brief The genre text, or None when there is no genre frame (id3v2ReadGenre).
        @details Assign a Genre (stored as its name, e.g. "Pop") or any `str` (id3v2WriteGenre).
        @throws ValueError if id3dev rejects the write
        """
        return take_string(lib.id3v2ReadGenre(self._tag()))

    @genre.setter
    def genre(self, value: "Genre | int | str") -> None:
        text = value if isinstance(value, str) else Genre(value).text
        _check(lib.id3v2WriteGenre(text.encode("utf-8"), self._tag()), "genre")

    def __repr__(self) -> str:
        """! @brief `<ID3v2.3 tag 'title' by 'artist'>`"""
        return f"<ID3v2.{self.version} tag {self.title!r} by {self.artist!r}>"

    # The live Id3v2Tag *, looked up each time so a replaced tag is never stale.
    def _tag(self):
        tag = self._owner._live().id3v2
        if tag == ffi.NULL:
            raise ValueError("this ID3 no longer has an ID3v2 tag")
        return tag


# Text fields both versions share a C read/write pair for: Python name -> C name part.
_V1_TEXT = {"title": "Title", "artist": "Artist", "album": "Album", "comment": "Comment"}
_V2_TEXT = {
    "title": "Title", "artist": "Artist", "album_artist": "AlbumArtist", "album": "Album",
    "year": "Year", "track": "Track", "composer": "Composer", "disc": "Disc",
    "lyrics": "Lyrics", "comment": "Comment",
}


# Builds a str property over <prefix>Read<c_name> / <prefix>Write<c_name>.
def _text_field(prefix: str, name: str, c_name: str, doc: str) -> property:
    read = getattr(lib, f"{prefix}Read{c_name}")
    write = getattr(lib, f"{prefix}Write{c_name}")

    def getter(self) -> "str | None":
        return take_string(read(self._tag()))

    def setter(self, value: str) -> None:
        _check(write(value.encode("utf-8"), self._tag()), name)

    return property(getter, setter, doc=doc)


for _name, _c_name in _V1_TEXT.items():
    setattr(ID3v1Tag, _name, _text_field(
        "id3v1", _name, _c_name, f"The {_name} (id3v1Read{_c_name} / id3v1Write{_c_name}); '' when unset."))

for _name, _c_name in _V2_TEXT.items():
    setattr(ID3v2Tag, _name, _text_field(
        "id3v2", _name, _c_name, f"The {_name.replace('_', ' ')} (id3v2Read{_c_name} / id3v2Write{_c_name})."))


# Raises ValueError when an id3v1Write*/id3v2Write* call returns 0.
def _check(ok: int, name: str) -> None:
    if not ok:
        raise ValueError(f"id3dev could not write {name}")
