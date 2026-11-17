"""Small helpers shared by the wrapper modules."""

from ._id3dev import ffi, lib


def take_string(ptr) -> "str | None":
    """Copy a malloc'd C string into Python, then free the C copy.

    Every char * that id3dev's read functions return was malloc'd for the caller.
    """
    if ptr == ffi.NULL:
        return None
    try:
        return ffi.string(ptr).decode("utf-8", errors="replace")
    finally:
        lib.free(ptr)


def take_bytes(ptr, size: int) -> "bytes | None":
    """Copy a malloc'd C buffer of `size` bytes into Python, then free it."""
    if ptr == ffi.NULL:
        return None
    try:
        return ffi.unpack(ffi.cast("char *", ptr), size)
    finally:
        lib.free(ptr)


def frame_id(raw: str) -> bytes:
    """Validate a frame ID ("TIT2", "TT2") and return it NUL-terminated for C."""
    if not isinstance(raw, str) or not 3 <= len(raw) <= 4 or not raw.isascii():
        raise ValueError(f"frame IDs are 3 or 4 ASCII characters, not {raw!r}")
    return raw.encode("ascii")

def key_value(key: str, value: "str | bytes") -> "tuple[bytes, bytes]":
    """Encode a context key and value for id3v2*KeyValue: a str value as UTF-8, bytes as given."""
    if not isinstance(key, str) or not key:
        raise ValueError(f"a context key is a non-empty str, not {key!r}")
    if isinstance(value, str):
        value = value.encode("utf-8")
    elif isinstance(value, (bytes, bytearray, memoryview)):
        value = bytes(value)
    else:
        raise TypeError(f"a context value is a str or bytes, not {type(value).__name__}")
    return key.encode("utf-8"), value
