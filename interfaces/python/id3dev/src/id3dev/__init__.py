"""Python bindings for id3dev: read and write ID3v1 and ID3v2 tags."""

from ._context import Context, ContextType
from ._encoding import Encoding
from ._frame import EntryReader, Frame
from ._genre import Genre
from ._id3 import ID3
from ._id3dev import lib as _lib
from ._pairs import Id3Pairs
from ._tags import ID3v1Tag, ID3v2Tag

__all__ = ["ID3", "ID3v1Tag", "ID3v2Tag", "Frame", "EntryReader", "Context", "ContextType",
           "Encoding", "Genre", "Id3Pairs", "set_preferred_standard", "get_preferred_standard"]

ID3V1 = _lib.ID3V1_TAG_VERSION
ID3V2_2 = _lib.ID3V2_TAG_VERSION_2
ID3V2_3 = _lib.ID3V2_TAG_VERSION_3
ID3V2_4 = _lib.ID3V2_TAG_VERSION_4


def set_preferred_standard(standard: int) -> None:
    """Which tag id3dev reads from and writes to when a file has both (process-wide)."""
    if not _lib.id3SetPreferredStandard(standard):
        raise ValueError(f"unknown standard {standard!r}")


def get_preferred_standard() -> int:
    return _lib.id3GetPreferredStandard()