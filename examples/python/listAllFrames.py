"""
@file listAllFrames.py
@author Ewan Jones
@brief lists all id3v2 frames in a provided mp3 file
@version 2026.11
@date 2026-11-13

@copyright Copyright (c) 2026
"""

import sys
from id3dev import ID3


if len(sys.argv) < 2:
    print("USAGE: <mp3 file>")

# load the tag from a provided file
tag = ID3.from_file(sys.argv[1])

# check if the loaded tag has ID3v2.x data
if not tag.has_id3v2:
    print(f"ERROR: {sys.argv[1]} does not contain any ID3v2.x tags")

# print all frame ids
for n, frame in enumerate(tag.frames):
    print(f"Frame {n}: {frame.id}")


tag.close()
