"""! @brief Context: the layout of a frame's content, as a list of Id3v2ContentContext."""

from enum import IntEnum

from ._id3dev import ffi, lib


class ContextType(IntEnum):
    """! @brief The type of a Context field: how id3dev reads and writes it.

    Values come from the compiled Id3v2ContextType enum in id3v2Types.h. For every
    type except BIT and ITER, a field's max and min are sizes in bytes.
    These are ints, so they can be passed anywhere id3dev expects one.

    @code{.py}
    from id3dev import Context, ContextType

    ctx = Context().add(ContextType.LATIN1, "label", 64, 1)
    @endcode
    """

    ## Raw characters with no encoding byte and no terminator (noEncoding_context).
    NO_ENCODING = lib.noEncoding_context
    ## Binary data with no terminator; reads up to max or the end of the frame (binary_context).
    BINARY = lib.binary_context
    ## Text in the encoding given by an earlier field keyed "encoding" (encodedString_context).
    ENCODED_STRING = lib.encodedString_context
    ## Latin-1 text ending in a NUL byte (latin1Encoding_context).
    LATIN1 = lib.latin1Encoding_context
    ## An 8, 16, 32 or 64 bit integer; the size comes from max/min (numeric_context).
    NUMERIC = lib.numeric_context
    ## A float or double; the size comes from max/min (precision_context).
    PRECISION = lib.precision_context
    ## 1-8 bits; max/min count bits, and the next field starts on the same byte (bit_context).
    BIT = lib.bit_context
    ## Repeats earlier fields: min is the index to start from, max how many times to repeat (iter_context).
    ITER = lib.iter_context
    ## Field whose max size is taken from an earlier field keyed "adjustment" (adjustment_context).
    ADJUSTMENT = lib.adjustment_context


class Context:
    """! @brief The layout of one kind of frame: an ordered list of fields.

    Each field has a type (a ContextType such as LATIN1 or BINARY), a key
    naming it, and a max and min size. id3dev reads and creates
    a frame's entries by walking these fields in order. Register a Context
    under a frame ID with Id3Pairs so id3dev can parse and build that frame.

    Backed by an owned C `List *` of Id3v2ContentContext, freed with listFree
    when the Context is garbage collected.

    @par Special keys
    A field keyed "encoding" sets the text encoding for later ENCODED_STRING
    fields, and a field keyed "adjustment" sets the max size of a later
    ADJUSTMENT field.

    @code{.py}
    from id3dev import Context, ContextType, Id3Pairs

    ctx = (Context()
           .add(ContextType.LATIN1, "label", 64, 1)
           .add(ContextType.BINARY, "data", 64, 1))
    pairs = Id3Pairs()
    pairs["XFRM"] = ctx
    @endcode
    """

    def __init__(self):
        """! @brief Creates an empty layout. Add fields with add().
        @throws MemoryError if the C list could not be created
        """
        ctx = lib.id3py_context_new()
        if ctx == ffi.NULL:
            raise MemoryError("listCreate failed")
        self._ctx = ffi.gc(ctx, lib.listFree)

    def add(self, type_: ContextType, key: str, max_size: int, min_size: int = 0) -> "Context":
        """! @brief Appends a field to the end of the layout.
        @param type_     field type, e.g. ContextType.LATIN1
        @param key       ASCII name for the field. Only "encoding" and "adjustment" mean
                         something to id3dev; other names just label the field.
        @param max_size  largest size in bytes (bits for BIT, repeat count for ITER)
        @param min_size  smallest size in bytes (bits for BIT, start index for ITER)
        @return this Context, so calls can be chained
        @throws MemoryError if the field could not be added
        """
        if lib.id3py_context_add(self._ctx, type_, key.encode("ascii"), max_size, min_size) != 0:
            raise MemoryError(f"could not add context field {key!r}")
        return self

    def __len__(self) -> int:
        """! @brief The number of fields added so far."""
        return lib.listLength(self._ctx)
