"""
@file buildTag.py
@author Ewan Jones
@brief Builds an ID3 tag programmatically and prints it
@version 2026.11
@date 2026-11-13

@copyright Copyright (c) 2026
"""

from id3dev import ID3, Encoding, Frame, Genre, ID3V2_3

# create an empty tag with an initialized ID3v2.3 and ID3v1 spec
tag = ID3.new(ID3V2_3, id3v1=True)

# create a version 1 tag
v1 = tag.id3v1
v1.title = "Happier Than Ever"
v1.artist = "Billie Eilish"
v1.album = "Happier Than Ever"
v1.track = 15
v1.year = 2021
v1.genre = Genre.POP

# create an id3v2 tag with common properties most readers understand
v2 = tag.id3v2
v2.title = "Happier Than Ever"
v2.artist = "Billie Eilish"
v2.album = "Happier Than Ever"
v2.year = "2021"
v2.track = "15"
v2.genre = Genre.POP

# create a custom text frame using ID3v2.3 standard
txxx = Frame.new("TXXX", ID3V2_3)
txxx.set_entries(Encoding.UTF8, b"producer", "Finneas O'Connell".encode("utf-8"))
tag.add_frame(txxx)

# Print the ID3v1 tag
print("ID3v1 tag:")
print(f"\t|Title: {v1.title}")
print(f"\t|Artist: {v1.artist}")
print(f"\t|Album: {v1.album}")
print(f"\t|Year: {v1.year}")
print(f"\t|Track: {v1.track}")
print(f"\t|Genre: {v1.genre.text}")

# Print the ID3v2 tag
print("ID3v2 tag:")
print(f"\t|Title: {v2.title}")
print(f"\t|Artist: {v2.artist}")
print(f"\t|Album: {v2.album}")
print(f"\t|Year: {v2.year}")
print(f"\t|Track: {v2.track}")
print(f"\t|Genre: {v2.genre}")

# find the new TXXX tag and print all its components
for frame in tag.frames:
    if frame.id == "TXXX":
        entries = frame.reader()
        print("\t|TXXX frame:")
        print(f"\t\t|encoding: {entries.u8()}")
        print(f"\t\t|description: {entries.text()}")
        print(f"\t\t|text: {entries.text()}")

# close the tag and free its memory
tag.close()




