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