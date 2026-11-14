"""
@file extractTag.py
@author Ewan Jones
@brief extracts a tag from a file, prints it as JSON and writes it to another file.
@version 2026.11
@date 2026-11-13

@copyright Copyright (c) 2026
"""
import sys
from id3dev import ID3


if len(sys.argv) < 3:
    print("USAGE: <mp3 file> <output file>")

tag = ID3.from_file(sys.argv[1])

# create a read only json represen
print(tag.to_json())

# save the extracted tag to a new file
tag.save(sys.argv[2])

# close the tag and free its memory
tag.close()

