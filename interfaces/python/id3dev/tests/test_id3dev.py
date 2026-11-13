import json
import shutil
from pathlib import Path

import pytest

import id3dev
from id3dev import ID3, Context, Frame, Id3Pairs

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
    ctx = Context().add(id3dev._context.LATIN1, "label", 64, 1).add(id3dev._context.BINARY, "data", 64, 1)
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