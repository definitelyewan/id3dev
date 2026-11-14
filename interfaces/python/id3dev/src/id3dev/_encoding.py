"""! @brief Encoding: the ID3v2 text encoding byte (id3v2Types.h ID3V2_ENCODING_*)."""

from enum import IntEnum

from ._id3dev import lib


class Encoding(IntEnum):
    """! @brief The text encoding byte that starts most ID3v2 text frames.

    Values come from the compiled ID3V2_ENCODING_* constants in id3v2Types.h.
    Because these are ints, one can be passed straight to Frame.set_entries
    as a one-byte entry.

    @code{.py}
    from id3dev import Encoding, Frame

    txxx = Frame.new("TXXX", 3)
    txxx.set_entries(Encoding.UTF8, b"producer", "Finneas O'Connell".encode())
    @endcode
    """

    ## ISO-8859-1 (Latin-1), the default for ID3v2.2 and 2.3 (0x00).
    ISO_8859_1 = lib.ID3V2_ENCODING_ISO_8859_1
    ## UTF-16 with a byte order mark; id3dev writes it little-endian (0x01).
    UTF16LE = lib.ID3V2_ENCODING_UTF16LE
    ## UTF-16 big-endian without a byte order mark, ID3v2.4 only (0x02).
    UTF16BE = lib.ID3V2_ENCODING_UTF16BE
    ## UTF-8, ID3v2.4 only (0x03).
    UTF8 = lib.ID3V2_ENCODING_UTF8
    ## Plain ASCII; an id3dev value, not part of the ID3v2 spec (0x04).
    ASCII = lib.ID3V2_ENCODING_ASCII
    ## Any other encoding; an id3dev value, not part of the ID3v2 spec (0xFF).
    OTHER = lib.ID3V2_ENCODING_OTHER
