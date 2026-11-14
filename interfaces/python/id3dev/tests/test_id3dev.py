import json
import shutil
from pathlib import Path

import pytest

import id3dev
from id3dev import ID3, Context, ContextType, Encoding, Frame, Genre, Id3Pairs

ASSETS = Path(__file__).parent / "assets"


@pytest.fixture
def mp3(tmp_path):
    """A scratch copy of a v2.3 file, so tests can save without touching the original."""
    dst = tmp_path / "song.mp3"
    shutil.copy(ASSETS / "sorry4dying.mp3", dst)
    return dst


def test_read_fields():
    tag = ID3.from_file(ASSETS / "danybrown2.mp3")
    assert tag.version == "2.2"
    assert tag.title == "Ain't It Funny"
    assert tag.artist == "Danny Brown"
    assert tag.album == "Atrocity Exhibition"


def test_missing_file():
    with pytest.raises(FileNotFoundError):
        ID3.from_file(ASSETS / "nope.mp3")


def test_write_and_save_round_trip(mp3):
    tag = ID3.from_file(mp3)
    tag.title = "New Title"
    tag.artist = "Someone Else"
    tag.save(mp3)

    again = ID3.from_file(mp3)
    assert again.title == "New Title"
    assert again.artist == "Someone Else"


def test_picture_round_trip(mp3):
    png = (ASSETS / "cat.png").read_bytes()
    tag = ID3.from_file(mp3)
    tag.set_picture(png, mime="image/png", type_=3)
    tag.save(mp3)
    assert ID3.from_file(mp3).picture(3) == png


def test_json():
    tag = ID3.from_file(ASSETS / "danybrown2.mp3")
    data = tag.as_dict()
    assert json.loads(tag.to_json()) == data
    assert "ID3v2" in data


def test_frames_and_entries():
    tag = ID3.from_file(ASSETS / "danybrown2.mp3")
    ids = [f.id for f in tag.frames]
    assert "TT2" in ids
    title = tag.frame("TT2")
    assert title.entries == [b"\x00", b"Ain't It Funny\x00"]   # encoding byte, NUL-terminated text


def test_frame_views_outlive_their_variable():
    frames = ID3.from_file(ASSETS / "danybrown2.mp3").frames   # the ID3 object is never stored
    assert frames[1].id == "TT2"                                # views keep their tag alive


def test_remove_frames(mp3):
    tag = ID3.from_file(mp3)
    assert tag.remove_frames("TIT2") >= 1
    assert tag.frame("TIT2") is None


def test_custom_frame_with_pairs(mp3):
    ctx = Context().add(ContextType.LATIN1, "label", 64, 1).add(ContextType.BINARY, "data", 64, 1)
    pairs = Id3Pairs()
    pairs["XFRM"] = ctx

    tag = ID3.from_file(mp3)
    frame = Frame.new("XFRM", 3, pairs)
    frame.set_entries(b"hello", b"\x01\x02\x03")
    tag.add_frame(frame)
    tag.save(mp3)

    again = ID3.from_file(mp3, pairs=pairs)
    custom = again.frame("XFRM")
    assert custom.entries == [b"hello\x00", b"\x01\x02\x03"]   # Latin-1 fields are saved NUL-terminated

    plain = ID3.from_file(mp3).frame("XFRM")                         # without pairs: one generic blob
    assert plain.entries == [b"hello\x00\x01\x02\x03"]


def test_short_entry_is_padded_to_min():
    pairs = Id3Pairs()
    pairs["XMIN"] = Context().add(ContextType.BINARY, "data", 32, 16)
    frame = Frame.new("XMIN", 3, pairs)
    frame.set_entries(b"\xab")
    assert frame.entries == [b"\xab" + bytes(15)]


def test_context_type_is_public():
    assert id3dev.ContextType.LATIN1 == id3dev._id3dev.lib.latin1Encoding_context
    assert "ContextType" in id3dev.__all__


def test_close_and_context_manager():
    with ID3.from_file(ASSETS / "danybrown2.mp3") as tag:
        assert tag.title
    with pytest.raises(ValueError):
        tag.title
    tag.close()   # closing twice is safe


def test_many_loads_do_not_crash():
    for _ in range(200):
        tag = ID3.from_file(ASSETS / "danybrown2.mp3")
        _ = tag.title, tag.frames, tag.to_json()

# --- creating tags from scratch ---

def test_new_defaults_to_empty_v23():
    tag = ID3.new()
    assert tag.has_id3v2 and not tag.has_id3v1
    assert tag.version == "2.3"
    assert tag.frames == []
    assert tag.title is None


def test_new_rejects_bad_arguments():
    with pytest.raises(ValueError):
        ID3.new(None, id3v1=False)
    with pytest.raises(ValueError):
        ID3.new(5)


def test_new_tags_saved_into_untagged_file(tmp_path):
    path = tmp_path / "untagged.mp3"
    shutil.copy(ASSETS / "null.mp3", path)
    audio = path.read_bytes()

    tag = ID3.new(id3dev.ID3V2_4, id3v1=True)
    tag.title = "Made in Python"
    tag.artist = "id3dev"
    tag.id3v1_from_id3v2()
    tag.save(path)

    data = path.read_bytes()
    assert audio in data                       # the original bytes are untouched
    assert data.startswith(b"ID3\x04")         # ID3v2.4 tag at the front
    assert data[-128:].startswith(b"TAG")      # ID3v1 tag at the end

    again = ID3.from_file(path)
    assert again.version == "2.4"
    assert again.title == "Made in Python"
    id3dev.set_preferred_standard(id3dev.ID3V1)
    try:
        assert again.title == "Made in Python"  # read from the ID3v1 tag
    finally:
        id3dev.set_preferred_standard(id3dev.ID3V2_3)


def test_add_tags(mp3):
    tag = ID3.from_file(ASSETS / "empty.mp3")   # ID3v1 only
    assert tag.has_id3v1 and not tag.has_id3v2
    tag.add_id3v2(id3dev.ID3V2_4)
    assert tag.version == "2.4" and tag.frames == []
    with pytest.raises(ValueError):
        tag.add_id3v2()
    with pytest.raises(ValueError):
        tag.add_id3v1()

    blank = ID3.new()
    blank.add_id3v1()
    assert blank.has_id3v1


def test_id3v2_from_id3v1():
    tag = ID3.from_file(ASSETS / "empty.mp3")   # ID3v1: "new title", "new artist"
    tag.id3v2_from_id3v1()
    assert tag.version == "2.3"
    ids = [f.id for f in tag.frames]
    assert "TIT2" in ids and "TPE1" in ids
    assert tag.title == "new title"
    assert tag.artist == "new artist"


def test_converters_need_a_source():
    with pytest.raises(ValueError):
        ID3.new().id3v2_from_id3v1()
    with pytest.raises(ValueError):
        ID3.new(None, id3v1=True).id3v1_from_id3v2()


# --- genres ---

def test_genre_enum():
    assert Genre.POP == 13
    assert Genre.HIP_HOP.text == "Hip-Hop"
    assert Genre.from_text(" hip-hop ") is Genre.HIP_HOP
    assert Genre.CHRISTMAS.text == "Christmas"
    assert len({g.text for g in Genre}) == len(Genre)   # every genre has its own name
    with pytest.raises(ValueError):
        Genre.from_text("Vaporwave")


def test_genre_id3v1(tmp_path):
    tag = ID3.new(None, id3v1=True)
    tag.genre = Genre.POP
    assert tag.genre == "Pop"
    tag.genre = Genre.BLUES                  # 0: a NUL byte to id3WriteGenre
    assert tag.genre == "Blues"
    tag.genre = "classic rock"               # names are looked up for ID3v1
    assert tag.genre == "Classic Rock"
    tag.genre = 13
    assert tag.genre == "Pop"
    with pytest.raises(ValueError):
        tag.genre = "Vaporwave"              # not an ID3v1 genre
    with pytest.raises(ValueError):
        tag.genre = 999

    path = tmp_path / "v1.mp3"
    shutil.copy(ASSETS / "null.mp3", path)
    tag.save(path)
    assert ID3.from_file(path).genre == "Pop"


def test_genre_id3v2(mp3):
    tag = ID3.from_file(mp3)
    tag.genre = Genre.HIP_HOP
    assert tag.genre == "Hip-Hop"
    tag.genre = "Vaporwave"                  # ID3v2 takes any text
    assert tag.genre == "Vaporwave"
    tag.save(mp3)
    assert ID3.from_file(mp3).genre == "Vaporwave"


def test_genre_follows_preferred_standard():
    tag = ID3.new(id3dev.ID3V2_3, id3v1=True)
    id3dev.set_preferred_standard(id3dev.ID3V1)
    try:
        tag.genre = Genre.JAZZ
        assert tag.genre == "Jazz"
    finally:
        id3dev.set_preferred_standard(id3dev.ID3V2_3)
    assert tag.genre is None                  # the ID3v2 tag was not touched


# --- direct ID3v1 / ID3v2 access ---

def test_direct_tags_ignore_preferred_standard():
    tag = ID3.new(id3dev.ID3V2_3, id3v1=True)
    tag.id3v1.title = "from v1"
    tag.id3v2.title = "from v2"
    assert tag.title == "from v2"                 # preferred standard is ID3v2.3
    id3dev.set_preferred_standard(id3dev.ID3V1)
    try:
        assert tag.title == "from v1"
        tag.id3v2.artist = "still v2"             # ignores the ID3v1 preference
    finally:
        id3dev.set_preferred_standard(id3dev.ID3V2_3)
    assert tag.id3v2.artist == "still v2"
    assert tag.id3v1.artist == ""


def test_id3v1_fields(tmp_path):
    tag = ID3.new(None, id3v1=True)
    assert tag.id3v2 is None
    v1 = tag.id3v1
    v1.title = "Happier Than Ever"
    v1.artist = "Billie Eilish"
    v1.album = "x" * 40                           # cut to ID3v1's 30 bytes
    v1.year = 2021
    v1.track = 15
    v1.genre = Genre.POP
    assert (v1.title, v1.year, v1.track, v1.genre) == ("Happier Than Ever", 2021, 15, Genre.POP)
    assert v1.album == "x" * 30
    v1.genre = "rock"
    assert v1.genre is Genre.ROCK
    with pytest.raises(ValueError):
        v1.genre = "Vaporwave"

    path = tmp_path / "v1.mp3"
    shutil.copy(ASSETS / "null.mp3", path)
    tag.save(path)
    again = ID3.from_file(path).id3v1
    assert (again.title, again.year, again.track, again.genre) == ("Happier Than Ever", 2021, 15, Genre.ROCK)


def test_id3v1_unknown_genre_reads_none():
    assert ID3.from_file(ASSETS / "sorry4dying.mp3").id3v1.genre is None   # genre byte 255


def test_id3v2_fields(mp3):
    tag = ID3.from_file(mp3)
    v2 = tag.id3v2
    assert v2.version == 3
    v2.title = "Title"
    v2.track = "15/16"
    v2.composer = "Finneas"
    v2.genre = Genre.HIP_HOP
    tag.save(mp3)
    again = ID3.from_file(mp3).id3v2
    assert (again.title, again.track, again.composer, again.genre) == ("Title", "15/16", "Finneas", "Hip-Hop")


def test_tag_view_survives_replacement():
    tag = ID3.new(id3dev.ID3V2_3, id3v1=True)
    v1 = tag.id3v1
    tag.id3v2.title = "converted"
    tag.id3v1_from_id3v2()                        # replaces the ID3v1 tag
    assert v1.title == "converted"                # the view follows the new tag


# --- reading entries and encodings ---

def test_encoding_enum():
    assert Encoding.ISO_8859_1 == 0
    assert Encoding.UTF8 == 3


def test_entry_reader_txxx():
    tag = ID3.new()
    txxx = Frame.new("TXXX", 3)
    txxx.set_entries(Encoding.UTF8, b"producer", 'say "hi" \\ back'.encode())
    tag.add_frame(txxx)

    r = tag.frame("TXXX").reader()
    assert r.remaining
    assert r.u8() == Encoding.UTF8
    assert r.text() == "producer"
    assert r.text() == 'say "hi" \\ back'        # id3dev's JSON escaping is undone
    assert not r.remaining
    with pytest.raises(IndexError):
        r.text()


def test_entry_reader_decodes_utf16():
    frame = ID3.from_file(ASSETS / "danybrown2.mp3").frame("TT2")
    r = frame.reader()
    r.skip()                                      # encoding byte
    assert r.text() == "Ain't It Funny"


def test_entry_reader_numbers_and_bytes():
    pairs = Id3Pairs()
    pairs["XNUM"] = (Context()
                     .add(ContextType.NUMERIC, "a", 4, 4)
                     .add(ContextType.NUMERIC, "b", 2, 2)
                     .add(ContextType.BINARY, "c", 8, 1))
    frame = Frame.new("XNUM", 3, pairs)
    frame.set_entries(b"\x00\x00\x01\x02", b"\x01\x02", b"\xff\x00")
    r = frame.reader()
    assert r.u32() == 0x0102                      # big-endian
    assert r.u16() == 0x0102                      # big-endian
    assert r.bytes() == b"\xff\x00"
