"""
@file customFrameAllContexts.py
@author Ewan Jones
@brief Creates a custom XRET frame that uses every context type, fills it and prints it
@version 2026.11
@date 2026-11-14

@copyright Copyright (c) 2026
"""

import struct

from id3dev import ID3, Context, ContextType, Encoding, Frame, Id3Pairs, ID3V2_4

# describe the layout of XRET, one field for every context type. two keys mean something to id3dev:
#   "encoding"   sets the encoding of later ENCODED_STRING fields
#   "adjustment" sets the width in bits of later ADJUSTMENT fields (the most recent one is used)
xret = (Context()
        # 0: a 1 byte integer, the text encoding for "title"
        .add(ContextType.NUMERIC, "encoding", 1, 1)
        # 1: text in the encoding above
        .add(ContextType.ENCODED_STRING, "title", 256, 1)
        # 2: latin-1 text ending in a NUL
        .add(ContextType.LATIN1, "url", 256, 1)
        # 3: exactly 3 raw characters, no encoding and no terminator
        .add(ContextType.NO_ENCODING, "language", 3, 3)
        # 4: a 4 byte float
        .add(ContextType.PRECISION, "gain", 4, 4)
        # 5-6: bit fields share bytes, here 1 bit + 7 bits = one byte
        .add(ContextType.BIT, "enabled", 1, 1)
        .add(ContextType.BIT, "level", 7, 7)
        # 7: exactly 4 bytes of binary data
        .add(ContextType.BINARY, "checksum", 4, 4)
        # 8: how many bits wide the next ADJUSTMENT field is
        .add(ContextType.NUMERIC, "adjustment", 1, 1)
        # 9: ceil(adjustment / 8) bytes, so 12 bits are stored in 2 bytes. the width comes
        # from "adjustment", so max/min are not used; they are set to "no limit" here
        .add(ContextType.ADJUSTMENT, "peak", 0xFFFFFFFF, 0)
        # 10: repeat fields 8-9 until the frame data runs out. for ITER, min is the
        # index to start from and max is how many times to repeat
        .add(ContextType.ITER, "repeat", 0xFFFFFFFF, 8))

# register the layout so id3dev knows how to build (and later read) XRET
pairs = Id3Pairs()
pairs["XRET"] = xret

# create an empty ID3v2.4 tag
tag = ID3.new(ID3V2_4)

# create an XRET frame. a new frame has one entry per field, so the ITER block holds a
# single repetition here; frames read from a file can hold as many as the data has
frame = Frame.new("XRET", ID3V2_4, pairs)

# fill the entries in order. set "adjustment" before "peak" so the peak is sized to 12 bits
frame.set_entries(
    Encoding.UTF8,
    "Happier Than Ever".encode("utf-8"),
    b"https://example.com/happier",
    b"eng",
    struct.pack("=f", -3.5),  # id3dev keeps PRECISION bytes as given, native order here
    0b1,                                     # enabled: 1 bit
    0b1010101,                               # level: 7 bits
    b"\xde\xad\xbe\xef",
    12,                                      # the peak below is 12 bits wide
    b"\x0a\xbc",                             # 0xabc fits in 12 bits
)
tag.add_frame(frame)

# print every XRET frame in the tag
for frame in tag.frames:
    if frame.id == "XRET":
        entries = frame.reader()
        print("XRET frame:")
        print(f"\t|encoding: {Encoding(entries.u8()).name}")
        print(f"\t|title: {entries.text()}")
        print(f"\t|url: {entries.text()}")
        print(f"\t|language: {entries.bytes().decode('latin-1')}")
        print(f"\t|gain: {struct.unpack('=f', entries.bytes())[0]} dB")
        print(f"\t|enabled: {bool(entries.u8())}")
        print(f"\t|level: {entries.u8():#09b}")
        print(f"\t|checksum: {entries.bytes().hex()}")

        # walk the repeated block until the entries run out
        while entries.remaining:
            bits = entries.u8()
            # a field 0 bits wide is empty, which reads back as None
            peak = (entries.bytes() or b"") if entries.remaining else b""
            print(f"\t|peak: {int.from_bytes(peak, 'big'):#x} ({bits} bits in {len(peak)} bytes)")

        print(f"\t|json: {frame.to_json()}")

tag.close()
