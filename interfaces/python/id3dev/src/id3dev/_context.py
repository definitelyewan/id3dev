"""Context: the layout of a frame's content, as a list of Id3v2ContentContext."""

from ._id3dev import ffi, lib

# id3v2Types.h Id3v2ContextType values, read from the compiled enum
NO_ENCODING = lib.noEncoding_context
BINARY = lib.binary_context
ENCODED_STRING = lib.encodedString_context
LATIN1 = lib.latin1Encoding_context
NUMERIC = lib.numeric_context
PRECISION = lib.precision_context
BIT = lib.bit_context
ITER = lib.iter_context
ADJUSTMENT = lib.adjustment_context


class Context:
    """An owned C List * of Id3v2ContentContext, freed with listFree.

    Build one field at a time; each field is a (type, key, max, min) entry.
    """

    def __init__(self):
        ctx = lib.id3py_context_new()
        if ctx == ffi.NULL:
            raise MemoryError("listCreate failed")
        self._ctx = ffi.gc(ctx, lib.listFree)

    def add(self, type_: int, key: str, max_size: int, min_size: int = 0) -> "Context":
        """Append a field. Sizes are bytes (bits for BIT). Returns self, so calls chain."""
        if lib.id3py_context_add(self._ctx, type_, key.encode("ascii"), max_size, min_size) != 0:
            raise MemoryError(f"could not add context field {key!r}")
        return self

    def __len__(self) -> int:
        return lib.listLength(self._ctx)