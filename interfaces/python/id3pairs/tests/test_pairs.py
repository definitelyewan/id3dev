import gc

import pytest

from id3pairs import MAX_KEY_BYTES, Id3Pairs


def test_set_and_get():
    t = Id3Pairs()
    t["TXXX"] = b"\x00hello"
    assert t["TXXX"] == b"\x00hello"
    assert "TXXX" in t
    assert len(t) == 1


def test_missing_key():
    t = Id3Pairs()
    with pytest.raises(KeyError):
        t["NOPE"]
    assert t.get("NOPE") is None


def test_duplicate_rejected():
    t = Id3Pairs()
    t["ABCD"] = b"1"
    with pytest.raises(KeyError):
        t["ABCD"] = b"2"
    assert t["ABCD"] == b"1"


def test_key_length_guard():
    t = Id3Pairs()
    assert MAX_KEY_BYTES == 25
    t["k" * MAX_KEY_BYTES] = b"ok"
    assert t["k" * MAX_KEY_BYTES] == b"ok"
    with pytest.raises(ValueError):
        t["k" * (MAX_KEY_BYTES + 1)] = b"too long"


def test_resize_keeps_values():
    t = Id3Pairs(size=1)  # forces many resizes
    for i in range(500):
        t[f"key{i}"] = str(i).encode()
    assert len(t) == 500
    assert all(t[f"key{i}"] == str(i).encode() for i in range(500))


def test_close_and_context_manager():
    with Id3Pairs() as t:
        t["A"] = b"x"
    with pytest.raises(ValueError):
        t["A"]
    t.close()  # closing twice is safe


def test_gc_frees_table():
    for _ in range(1000):
        Id3Pairs()["A"] = b"x" * 1024
    gc.collect()