"""
@file customTag.py
@author Ewan Jones
@brief Creates a custom XER1 frame with its own layout, fills it and prints it
@version 2026.11
@date 2026-11-13

@copyright Copyright (c) 2026
"""

from id3dev import ID3, Context, ContextType, Encoding, Frame, Id3Pairs, ID3V2_4

# describe the layout of XER1: a one byte encoding followed by 3 strings in that encoding.
# the "encoding" key is special, id3dev uses it to decode the encoded string fields after it
xer1 = (Context()
        .add(ContextType.NUMERIC, "encoding", 1, 1)
        .add(ContextType.ENCODED_STRING, "artist", 256, 1)
        .add(ContextType.ENCODED_STRING, "song", 256, 1)
        .add(ContextType.ENCODED_STRING, "note", 256, 1))

# register the layout so id3dev knows how to build (and later read) XER1
pairs = Id3Pairs()
pairs["XER1"] = xer1

# create an empty ID3v2.4 tag
tag = ID3.new(ID3V2_4)

# create an XER1 frame and fill its 4 entries in order
frame = Frame.new("XER1", ID3V2_4, pairs)
frame.set_entries(
    Encoding.UTF8,
    "Billie Eilish".encode("utf-8"),
    "Happier Than Ever".encode("utf-8"),
    "Recorded in Finneas' bedroom studio".encode("utf-8"),
)
tag.add_frame(frame)

# print every XER1 frame in the tag
for frame in tag.frames:
    if frame.id == "XER1":
        entries = frame.reader()
        print("XER1 frame:")
        print(f"\t|encoding: {Encoding(entries.u8()).name}")
        print(f"\t|artist: {entries.text()}")
        print(f"\t|song: {entries.text()}")
        print(f"\t|note: {entries.text()}")

tag.close()
