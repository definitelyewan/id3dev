"""
@file displayAllText.py
@author Ewan Jones
@brief Displays all text information from an mp3 file
@version 2026.11
@date 2026-11-13

@copyright Copyright (c) 2026
"""
import sys
from id3dev import ID3

if len(sys.argv) < 2:
    print("USAGE: <mp3 file>")
    sys.exit(1)

file_path = sys.argv[1]

# read a file and store it as an ID3 object
tag = ID3.from_file(file_path)


# check if the provided file contained an ID3v2.x tag
if not tag.has_id3v2:
    print(f"No ID3v2 tags found in {file_path}")
else:
    print(f"ID3v2 tags found in {file_path} ----------")

    # loop through the frames present in the tag and scan for id's starting with 'T'
    # tags that start with 'T' are always text frames
    for frame in tag.frames:
        if not frame.id.startswith("T"):
            continue

        if frame.id == "TXXX" or frame.id == "TXX":
            # create a frame reader for user defined text frames and read their structure
            entries = frame.reader()
            encoding = entries.u8()
            desc = entries.text()
            text = entries.text()
            print(f"{frame.id}: Encoding: {encoding}, Desc: {desc}, Text: {text}")
        else:
            # create a frame reader for standard defined text frames and read their structure
            entries = frame.reader()
            encoding = entries.u8()
            text = entries.text()
            print(f"{frame.id}: Encoding: {encoding}, Text: {text}")


# check if the provided file contained an ID3v1.x tag
if not tag.has_id3v1:
    print(f"No ID3v1 tags found in {file_path}")
else:
    print(f"ID3v1 tag found in {file_path} ----------")
    print(f"Title: {tag.id3v1.title}")
    print(f"Artist: {tag.id3v1.artist}")
    print(f"Album: {tag.id3v1.title}")
    print(f"Year: {tag.id3v1.year}")
    print(f"genre: {tag.id3v1.genre}")
    print(f"Track: {tag.id3v1.track}")
    print(f"Comment: {tag.id3v1.comment}")

# close the tag and free its memory
tag.close()