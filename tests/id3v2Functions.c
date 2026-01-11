/**
 * @file id3v2Functions.c
 * @author Ewan Jones
 * @brief unit tests for id3v2.c
 * @version 0.1
 * @date 2024-03-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <stdio.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include "id3v2/id3v2.h"
#include "id3v2/id3v2Frame.h"
#include "byteStream.h"
#include "id3v2/id3v2Parser.h"

static void id3v2TagFromFile_v3(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_non_null(tag);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    int c = 0;

    for (c = 0; ; c++) {
        f = id3v2FrameTraverse(&frames);
        if (f == NULL) {
            break;
        }
    }

    
    assert_int_equal(c, 15);

    id3v2DestroyTag(&tag);
}

static void id3v2TagFromFile_null(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile(NULL);

    assert_null(tag);
}


static void id3v2CopyTag_v3(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    Id3v2Tag *copy = id3v2CopyTag(tag);

    assert_non_null(copy);

    assert_non_null(copy->header);
    assert_non_null(copy->frames);

    assert_int_equal(tag->header->majorVersion, copy->header->majorVersion);
    assert_int_equal(tag->header->minorVersion, copy->header->minorVersion);
    assert_int_equal(tag->header->flags, copy->header->flags);
    assert_null(copy->header->extendedHeader);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;


    for(int i = 0; i < 15; i++){
        f = id3v2FrameTraverse(&frames);
        listFindElement(tag->frames, id3v2CompareFrame, f);
        assert_non_null(f);
    }

    id3v2DestroyTag(&tag);
    id3v2DestroyTag(&copy);
}


static void id3v2CompareTag_v3v4(void **state){
    (void) state;
    Id3v2Tag *tag1 = id3v2TagFromFile("assets/sorry4dying.mp3");
    Id3v2Tag *tag2 = id3v2TagFromFile("assets/OnGP.mp3");

    assert_false(id3v2CompareTag(tag1, tag2));

    id3v2DestroyTag(&tag1);
    id3v2DestroyTag(&tag2);

}

static void id3v2CompareTag_v3same(void **state){
    (void) state;
    Id3v2Tag *tag1 = id3v2TagFromFile("assets/sorry4dying.mp3");
    Id3v2Tag *tag2 = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_true(id3v2CompareTag(tag1, tag2));

    id3v2DestroyTag(&tag1);
    id3v2DestroyTag(&tag2);

}

static void id3v2ReadFrameByID_v3(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    Id3v2Frame *frame = id3v2ReadFrameByID("TIT2", tag);
    assert_non_null(frame);
    assert_string_equal((char *)frame->header->id, "TIT2");
    id3v2DestroyFrame(&frame);


    frame = id3v2ReadFrameByID("TALB", tag);
    assert_non_null(frame);
    assert_string_equal((char *)frame->header->id, "TALB");
    id3v2DestroyFrame(&frame);


    id3v2DestroyTag(&tag);

}

static void id3v2ReadFrameByID_v3MultiTXXX(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");
    Id3v2Frame *frame = NULL;
    ListIter entries = {0};

    char *str = NULL;
    size_t s = 0;

    frame = id3v2ReadFrameByID("TXXX", tag);
    assert_non_null(frame);
    assert_string_equal((char *)frame->header->id, "TXXX");

    entries = id3v2CreateFrameEntryTraverser(frame);

    str = id3v2ReadFrameEntryAsChar(&entries, &s);
    assert_string_equal(str, "\x01");
    free(str);

    str = id3v2ReadFrameEntryAsChar(&entries, &s);
    assert_string_equal(str, "LABEL");
    free(str);

    str = id3v2ReadFrameEntryAsChar(&entries, &s);
    assert_string_equal(str, "deadAir");
    free(str);

    id3v2DestroyFrame(&frame);

    // the exact same frame should be returned

    frame = id3v2ReadFrameByID("TXXX", tag);
    assert_non_null(frame);
    assert_string_equal((char *)frame->header->id, "TXXX");

    entries = id3v2CreateFrameEntryTraverser(frame);

    str = id3v2ReadFrameEntryAsChar(&entries, &s);
    assert_string_equal(str, "\x01");
    free(str);

    str = id3v2ReadFrameEntryAsChar(&entries, &s);
    assert_string_equal(str, "LABEL");
    free(str);

    str = id3v2ReadFrameEntryAsChar(&entries, &s);
    assert_string_equal(str, "deadAir");
    free(str);

    id3v2DestroyFrame(&frame);
    id3v2DestroyTag(&tag);

}

static void id3v2ReadFrameByID_v2Null(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    Id3v2Frame *frame = id3v2ReadFrameByID("XXX", tag);
    assert_null(frame);

    id3v2DestroyTag(&tag);

}

static void id3v2RemoveFrameByID_v3EveryFrame(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    size_t total = tag->frames->length;

    assert_true(id3v2RemoveFrameByID("APIC", tag));
    total--;
    assert_int_equal(total, tag->frames->length);
    assert_false(id3v2RemoveFrameByID("APIC", tag));

    for(int i = 0; i < 3; i++){
        assert_true(id3v2RemoveFrameByID("TXXX", tag));
        total--;
        assert_int_equal(total, tag->frames->length);
        
    }
    assert_false(id3v2RemoveFrameByID("TXXX", tag));

    assert_true(id3v2RemoveFrameByID("TCOM", tag));
    total--;
    assert_int_equal(total, tag->frames->length);
    assert_false(id3v2RemoveFrameByID("TCOM", tag));

    assert_true(id3v2RemoveFrameByID("TPE2", tag));
    total--;
    assert_int_equal(total, tag->frames->length);
    assert_false(id3v2RemoveFrameByID("TPE2", tag));

    assert_true(id3v2RemoveFrameByID("TPE1", tag));
    total--;
    assert_int_equal(total, tag->frames->length);
    assert_false(id3v2RemoveFrameByID("TPE1", tag));

    assert_true(id3v2RemoveFrameByID("TPOS", tag));
    total--;
    assert_int_equal(total, tag->frames->length);
    assert_false(id3v2RemoveFrameByID("TPOS", tag));

    assert_true(id3v2RemoveFrameByID("TRCK", tag));
    total--;
    assert_int_equal(total, tag->frames->length);
    assert_false(id3v2RemoveFrameByID("TRCK", tag));

    assert_true(id3v2RemoveFrameByID("TYER", tag));
    total--;
    assert_int_equal(total, tag->frames->length);
    assert_false(id3v2RemoveFrameByID("TYER", tag));

    assert_true(id3v2RemoveFrameByID("TCON", tag));
    total--;
    assert_int_equal(total, tag->frames->length);
    assert_false(id3v2RemoveFrameByID("TCON", tag));

    assert_true(id3v2RemoveFrameByID("TCOP", tag));
    total--;
    assert_int_equal(total, tag->frames->length);
    assert_false(id3v2RemoveFrameByID("TCOP", tag));

    assert_true(id3v2RemoveFrameByID("TSRC", tag));
    total--;
    assert_int_equal(total, tag->frames->length);
    assert_false(id3v2RemoveFrameByID("TSRC", tag));

    assert_true(id3v2RemoveFrameByID("TALB", tag));
    total--;
    assert_int_equal(total, tag->frames->length);
    assert_false(id3v2RemoveFrameByID("TALB", tag));

    assert_true(id3v2RemoveFrameByID("TIT2", tag));
    total--;
    assert_int_equal(total, tag->frames->length);
    assert_false(id3v2RemoveFrameByID("TIT2", tag));

    assert_int_equal(0, tag->frames->length);

    id3v2DestroyTag(&tag);

}

static void id3v2RemoveFrameByID_Null(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_false(id3v2RemoveFrameByID("ASWA", tag));

    id3v2DestroyTag(&tag);


}


static void id3v2ReadTextFrameContent_TRK(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadTextFrameContent("TRK", tag);

    assert_non_null(str);
    assert_string_equal(str, "06/15");

    free(str);

    id3v2DestroyTag(&tag);

}

static void id3v2ReadTextFrameContent_TXX(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadTextFrameContent("TXX", tag);

    assert_null(str);

    id3v2DestroyTag(&tag);

}

static void id3v2ReadTextFrameContent_PIC(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadTextFrameContent("PIC", tag);

    assert_null(str);

    id3v2DestroyTag(&tag);

}

static void id3v2ReadTitle_TT2(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadTitle(tag);

    assert_non_null(str);

    assert_string_equal(str, "Ain't It Funny");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadTitle_TIT2(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadTitle(tag);

    assert_non_null(str);

    assert_string_equal(str, "sorry4dying");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadArtist_TP1(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadArtist(tag);

    assert_non_null(str);

    assert_string_equal(str, "Danny Brown");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadArtist_TPE1(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadArtist(tag);

    assert_non_null(str);

    assert_string_equal(str, "Quadeca");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadArtist_TP2(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadAlbumArtist(tag);

    assert_non_null(str);

    assert_string_equal(str, "Danny Brown");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadArtist_TPE2(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadAlbumArtist(tag);

    assert_non_null(str);

    assert_string_equal(str, "Quadeca");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadArtist_TAL(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadAlbum(tag);

    assert_non_null(str);

    assert_string_equal(str, "Atrocity Exhibition");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadArtist_TALB(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadAlbum(tag);

    assert_non_null(str);

    assert_string_equal(str, "I Didn't Mean To Haunt You");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadYear_TYE(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadYear(tag);

    assert_non_null(str);

    assert_string_equal(str, "2016");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadYear_TYER(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadYear(tag);

    assert_non_null(str);

    assert_string_equal(str, "2022");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadGenre_TCO(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadGenre(tag);

    assert_non_null(str);

    assert_string_equal(str, "Experimental Hip-Hop, Hardcore Hip-Hop, Abstract Hip-Hop, Industrial Hip-Hop & Post-Punk");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadGenre_TCON(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadGenre(tag);

    assert_non_null(str);

    assert_string_equal(str, "Art Pop, Folktronica Glitch Pop, Ambient Pop, Experimental Hip-Hop, Neo-Psychedelia, Alternative R&B, Emo Rap & Chamber Pop");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadTrack_TRK(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadTrack(tag);

    assert_non_null(str);

    assert_string_equal(str, "06/15");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadTrack_TRCK(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadTrack(tag);

    assert_non_null(str);

    assert_string_equal(str, "01/11");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadComposer_TCM(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadComposer(tag);

    assert_non_null(str);

    assert_string_equal(str, "Danny Brown");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadComposer_TCOM(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadComposer(tag);

    assert_non_null(str);

    assert_string_equal(str, "Ben Lasky");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadDisc_TPA(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadDisc(tag);

    assert_non_null(str);

    assert_string_equal(str, "01/01");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadDisc_TPOS(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadDisc(tag);

    assert_non_null(str);

    assert_string_equal(str, "1/1");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadLyrics_ULT(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadLyrics(tag);

    assert_non_null(str);

    assert_string_equal(str, "haBDJHAsbdjkHASBDJahbsdkAHBSDHAbsdHBDUAHSBDUBAUIBFOASIUBDFOIAUBFOIAUWBFOAWBFAOUWEBFUOYBOUBUOBUOboubouboubouboubouboigndoignoisnjgsdfjnglksjdfngslkjfngskdjfnglskdnfgiserugisugnvfkdxjnvxlkjnijxdngixjdhfgoiserhgiusdng spoerijgsoergjnposeirhgposergn reigjosperijgsodfkgkldfmvxc.,vbm");
    free(str);
    id3v2DestroyTag(&tag);
}


static void id3v2ReadLyrics_Null(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadLyrics(tag);

    assert_null(str);

    id3v2DestroyTag(&tag);
}

static void id3v2ReadComment_COM(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadComment(tag);

    assert_non_null(str);

    assert_string_equal(str, "test");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadComment_Null(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadComment(tag);

    assert_null(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadPicture_PIC(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    size_t dataSize = 0;
    uint8_t *data = id3v2ReadPicture(0, tag, &dataSize);

    assert_non_null(data);
    assert_int_equal(dataSize, 107904);
    free(data);

    data = id3v2ReadPicture(78, tag, &dataSize);

    assert_non_null(data);
    assert_int_equal(dataSize, 107904);
    free(data);


    id3v2DestroyTag(&tag);
}

static void id3v2ReadPicture_APIC(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/OnGP.mp3");

    size_t dataSize = 0;
    uint8_t *data = NULL;

    data = id3v2ReadPicture(0, tag, &dataSize);
    assert_non_null(data);
    free(data);

    data = id3v2ReadPicture(3, tag, &dataSize);
    assert_non_null(data);
    free(data);

    id3v2DestroyTag(&tag);
}


static void id3v2WriteTextFrameContent_TIT2(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/OnGP.mp3");

    assert_true(id3v2WriteTextFrameContent("TIT2", "test", tag));

    char *str = id3v2ReadTitle(tag);
    assert_non_null(str);
    assert_string_equal(str, "test");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteTextFrameContent_TCOM(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/OnGP.mp3");

    assert_true(id3v2WriteTextFrameContent("TCOM", "test", tag));

    id3v2DestroyTag(&tag);
}

static void id3v2WriteTitle_TT2(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/boniver.mp3");

    assert_true(id3v2WriteTitle("death breast", tag));

    char *str = id3v2ReadTitle(tag);
    assert_non_null(str);
    assert_string_equal(str, "death breast");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteTitle_TIT2(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_true(id3v2WriteTitle("title", tag));

    char *str = id3v2ReadTitle(tag);
    assert_non_null(str);
    assert_string_equal(str, "title");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteArtist_TP1(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/boniver.mp3");

    assert_true(id3v2WriteArtist("good winter", tag));

    char *str = id3v2ReadArtist(tag);
    assert_non_null(str);
    assert_string_equal(str, "good winter");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteArtist_TEP1(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_true(id3v2WriteArtist("ab", tag));

    char *str = id3v2ReadArtist(tag);
    assert_non_null(str);
    assert_string_equal(str, "ab");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteAlbumArtist_TP2(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/boniver.mp3");

    assert_true(id3v2WriteAlbumArtist("justin", tag));

    char *str = id3v2ReadAlbumArtist(tag);
    assert_non_null(str);
    assert_string_equal(str, "justin");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteAlbumArtist_TEP2(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_true(id3v2WriteAlbumArtist("ben", tag));

    char *str = id3v2ReadAlbumArtist(tag);
    assert_non_null(str);
    assert_string_equal(str, "ben");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteAlbum_TAL(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/boniver.mp3");

    assert_false(id3v2WriteAlbum("", tag));

    id3v2DestroyTag(&tag);
}

static void id3v2WriteAlbum_TALB(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_true(id3v2WriteAlbum("SCRAPYARD", tag));

    char *str = id3v2ReadAlbum(tag);
    assert_non_null(str);
    assert_string_equal(str, "SCRAPYARD");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteYear_TYE(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/boniver.mp3");

    assert_true(id3v2WriteYear("1910", tag));

    char *str = id3v2ReadYear(tag);
    assert_non_null(str);
    assert_string_equal(str, "1910");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteYear_TYER(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_true(id3v2WriteYear("0", tag));

    char *str = id3v2ReadYear(tag);
    assert_non_null(str);
    assert_string_equal(str, "0");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteGenre_TCO(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/boniver.mp3");

    assert_true(id3v2WriteGenre("Bossa Nova", tag));

    char *str = id3v2ReadGenre(tag);
    assert_non_null(str);
    assert_string_equal(str, "Bossa Nova");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteGenre_TCON(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_true(id3v2WriteGenre("Death Metal", tag));

    char *str = id3v2ReadGenre(tag);
    assert_non_null(str);
    assert_string_equal(str, "Death Metal");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteTrack_TRK(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/boniver.mp3");

    assert_true(id3v2WriteTrack("99/99", tag));

    char *str = id3v2ReadTrack(tag);
    assert_non_null(str);
    assert_string_equal(str, "99/99");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteTrack_TRCK(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_true(id3v2WriteTrack("1/40", tag));

    char *str = id3v2ReadTrack(tag);
    assert_non_null(str);
    assert_string_equal(str, "1/40");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteDisc_TPA(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/boniver.mp3");

    assert_true(id3v2WriteDisc("1/1", tag));

    char *str = id3v2ReadDisc(tag);
    assert_non_null(str);
    assert_string_equal(str, "1/1");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteDisc_TPOS(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_true(id3v2WriteDisc("10/100", tag));

    char *str = id3v2ReadDisc(tag);
    assert_non_null(str);
    assert_string_equal(str, "10/100");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteComposer_TCM(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/boniver.mp3");

    assert_true(id3v2WriteComposer("justion vernon", tag));

    char *str = id3v2ReadComposer(tag);
    assert_non_null(str);
    assert_string_equal(str, "justion vernon");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteComposer_TCOM(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_true(id3v2WriteComposer("Lasky", tag));

    char *str = id3v2ReadComposer(tag);
    assert_non_null(str);
    assert_string_equal(str, "Lasky");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteLyrics_ULT(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/boniver.mp3");

    assert_true(id3v2WriteLyrics("there is no lyrics frame", tag));
    
    char *str = id3v2ReadLyrics(tag);
    assert_non_null(str);
    assert_string_equal(str, "there is no lyrics frame");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteLyrics_ULT2(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    assert_true(id3v2WriteLyrics("Verbal couture, parkour with the metaphors The flow house of horror, dead bolted with metal doors Grinch bitch, six sense with a nose drip Mind skydive, sniffing bumps in the cockpit Locksmith of hip-hop, appraisal the wrist watch The rocks 'bout the size as the teeth in Chris Rock's mouth Sock out the mic, prototype for Adderall Your work's killing fiends 'cause you cut it with Fentanyl So much coke just to sniff, need a ski lift Flip your table over if you cut it with the bullshit Nosebleed on red carpets, but it just blend in Snapping pictures feeling my chest being sunk in Live a fast life, seen many die slowly Unhappy when they left so I try to seize the moment Funny how it happens who ever would imagine That joke's on you but Satan the one laughing Ain't it funny how it happens? Ain't it? Ain't it funny how it happens? Ain't it? Ain't it funny how it happens? Ain't it? I can sell honey to a bee In the fall time, make trees, take back they leaves Octopus in a straight jacket, savage with bad habits Broke, serving fiends, got rich, became a addict Ain't it funny how it happens, who would ever would imagine? Nose running right now, could ya pass me a napkin? Managed to somehow to have the upper advantage Panic when the drugs are gone and nobody is answering Ain't it funny how it happens? Ain't it? Ain't it funny how it happens? No way to mask it, a lot became has-beens Rolling up that hundred dollar bill 'til they cash in Think it's gon' last, going too fast Man, it's fucked up, ain't it funny how it happens? Ain't it funny how it happens? Ain't it? Ain't it funny how it happens? Ain't it? Ain't it funny how it happens? Ain't it? Ain't it funny how it happens? Upcoming heavy traffic Say you need to slow down 'cause you feel yourself crashing Staring in the devil face but you can't stop laughing Staring in the devil face but you can't stop laughing It's a living nightmare, that most of us might share Inherited in our blood, it's why we stuck in the mud Can't quit the drug use or the alcohol abuse Even if I wanted to, tell you what I'm gonna do I'ma wash away my problems with a bottle of Henny Anxiety got the best of me so I'm popping them Xannies Might need rehab but to me that shit pussy Pray for me y'all, 'cause I don't know what coming to me Bought a 8-ball of coke and my nigga on the way Got three hoes with him and they all tryna play Ain't it funny how it happens, ever would imagine Joke's on you but Satan the one laughing Ain't it funny how it happens? Ain't it? Ain't it funny how it happens? Ain't it? Ain't it funny how it happens? Ain't it?", tag));

    char *str = id3v2ReadLyrics(tag);
    assert_non_null(str);
    assert_string_equal(str, "Verbal couture, parkour with the metaphors The flow house of horror, dead bolted with metal doors Grinch bitch, six sense with a nose drip Mind skydive, sniffing bumps in the cockpit Locksmith of hip-hop, appraisal the wrist watch The rocks 'bout the size as the teeth in Chris Rock's mouth Sock out the mic, prototype for Adderall Your work's killing fiends 'cause you cut it with Fentanyl So much coke just to sniff, need a ski lift Flip your table over if you cut it with the bullshit Nosebleed on red carpets, but it just blend in Snapping pictures feeling my chest being sunk in Live a fast life, seen many die slowly Unhappy when they left so I try to seize the moment Funny how it happens who ever would imagine That joke's on you but Satan the one laughing Ain't it funny how it happens? Ain't it? Ain't it funny how it happens? Ain't it? Ain't it funny how it happens? Ain't it? I can sell honey to a bee In the fall time, make trees, take back they leaves Octopus in a straight jacket, savage with bad habits Broke, serving fiends, got rich, became a addict Ain't it funny how it happens, who would ever would imagine? Nose running right now, could ya pass me a napkin? Managed to somehow to have the upper advantage Panic when the drugs are gone and nobody is answering Ain't it funny how it happens? Ain't it? Ain't it funny how it happens? No way to mask it, a lot became has-beens Rolling up that hundred dollar bill 'til they cash in Think it's gon' last, going too fast Man, it's fucked up, ain't it funny how it happens? Ain't it funny how it happens? Ain't it? Ain't it funny how it happens? Ain't it? Ain't it funny how it happens? Ain't it? Ain't it funny how it happens? Upcoming heavy traffic Say you need to slow down 'cause you feel yourself crashing Staring in the devil face but you can't stop laughing Staring in the devil face but you can't stop laughing It's a living nightmare, that most of us might share Inherited in our blood, it's why we stuck in the mud Can't quit the drug use or the alcohol abuse Even if I wanted to, tell you what I'm gonna do I'ma wash away my problems with a bottle of Henny Anxiety got the best of me so I'm popping them Xannies Might need rehab but to me that shit pussy Pray for me y'all, 'cause I don't know what coming to me Bought a 8-ball of coke and my nigga on the way Got three hoes with him and they all tryna play Ain't it funny how it happens, ever would imagine Joke's on you but Satan the one laughing Ain't it funny how it happens? Ain't it? Ain't it funny how it happens? Ain't it? Ain't it funny how it happens? Ain't it?");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteComment_COM(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    assert_true(id3v2WriteComment("not a test", tag));

    char *str = id3v2ReadComment(tag);
    assert_non_null(str);
    assert_string_equal(str, "not a test");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteComment_COMM(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_true(id3v2WriteComment("not a test", tag));

    char *str = id3v2ReadComment(tag);
    assert_non_null(str);
    assert_string_equal("not a test", str);

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WritePicture_PIC(void **state){
    (void) state;
    FILE *fp = NULL;
    size_t sz = 0;
    size_t charsz = 0;
    uint8_t *data = NULL;

    fp = fopen("assets/cat.png", "rb");
    // NOLINTNEXTLINE
    (void) fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    (void) fseek(fp, 0L, SEEK_SET);
    data = malloc(sz);
    (void) fread(data, 1, sz, fp);
    (void) fclose(fp);
    
    assert_non_null(data);

    Id3v2Tag *tag = id3v2TagFromFile("assets/boniver.mp3");
    assert_true(id3v2WritePicture(data, sz, "PNG", 0x00, tag));

    Id3v2Frame *f = id3v2ReadFrameByID("PIC", tag);
    assert_non_null(f);

    ListIter i = id3v2CreateFrameEntryTraverser(f);

    id3v2ReadFrameEntryAsU8(&i); //encoding

    char *str = id3v2ReadFrameEntryAsChar(&i, &charsz); // mime type
    assert_non_null(str);
    assert_string_equal(str, "PNG");

    id3v2ReadFrameEntryAsU8(&i); // picture type
    id3v2ReadFrameEntryAsU8(&i);

    uint8_t *test = (uint8_t *) id3v2ReadFrameEntry(&i, &charsz);

    assert_memory_equal(test, data, sz);
    free(test);

    free(str);
    free(data);
    id3v2DestroyFrame(&f);
    id3v2DestroyTag(&tag);
}

static void id3v2WritePictureFromFile_PIC(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/boniver.mp3");
    assert_true(id3v2WritePictureFromFile("assets/cat.png", "PNG", 0x00, tag));

    Id3v2Frame *f = id3v2ReadFrameByID("PIC", tag);
    assert_non_null(f);

    ListIter i = id3v2CreateFrameEntryTraverser(f);

    id3v2ReadFrameEntryAsU8(&i); //encoding
    size_t charsz = 0;
    char *str = id3v2ReadFrameEntryAsChar(&i, &charsz); // mime type
    assert_string_equal(str, "PNG");
    free(str);

    id3v2ReadFrameEntryAsU8(&i); // picture type
    id3v2ReadFrameEntryAsU8(&i); // desc

    uint8_t *data = (uint8_t *) id3v2ReadFrameEntry(&i, &charsz);

    FILE *fp = NULL;
    fp = fopen("assets/cat.png", "rb");
    // NOLINTNEXTLINE
    (void) fseek(fp, 0L, SEEK_END);
    size_t sz = ftell(fp);
    (void) fseek(fp, 0L, SEEK_SET);
    uint8_t *test = malloc(sz);
    (void) fread(test, 1, sz, fp);
    (void) fclose(fp);


    assert_memory_equal(data, test, sz);

    free(data);
    free(test);
    id3v2DestroyFrame(&f);
    id3v2DestroyTag(&tag);
}

static void id3v2InsertTextFrame_TSOA(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/OnGP.mp3");

    assert_true(id3v2InsertTextFrame("TSOA", BYTE_UTF16LE, "test", tag));

    Id3v2Frame *f = id3v2ReadFrameByID("TSOA", tag);

    assert_non_null(f);

    ListIter i = id3v2CreateFrameEntryTraverser(f);

    assert_int_equal(id3v2ReadFrameEntryAsU8(&i), BYTE_UTF16LE);

    size_t charsz = 0;
    char *str = id3v2ReadFrameEntryAsChar(&i, &charsz);

    assert_string_equal(str, "test");
    
    free(str);
    id3v2DestroyFrame(&f);
    id3v2DestroyTag(&tag);
}

static void id3v2InsertTextFrame_TSOAnoString(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/OnGP.mp3");

    assert_false(id3v2InsertTextFrame("TSOA", BYTE_UTF16LE, NULL, tag));

    Id3v2Frame *f = id3v2ReadFrameByID("TSOA", tag);

    assert_null(f);

    id3v2DestroyFrame(&f);
    id3v2DestroyTag(&tag);
}

static void id3v2InsertTextFrame_NoID(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/OnGP.mp3");

    assert_false(id3v2InsertTextFrame(NULL, BYTE_UTF16LE, "test", tag));

    id3v2DestroyTag(&tag);
}

static void id3v2TagSerialize_v3(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");
    size_t outl = 0;
    uint8_t *out = id3v2TagSerialize(tag, &outl);
    ByteStream *stream = byteStreamCreate(out, outl);

    assert_non_null(stream);

    Id3v2Tag *tag2 = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);
    bool v = id3v2CompareTag(tag, tag2);

    byteStreamDestroy(stream);
    free(out);
    id3v2DestroyTag(&tag);
    id3v2DestroyTag(&tag2);

    assert_true(v);
}

static void id3v2TagSerialize_v2(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");
    size_t outl = 0;
    uint8_t *out = id3v2TagSerialize(tag, &outl);
    ByteStream *stream = byteStreamCreate(out, outl);
    
    assert_non_null(stream);

    Id3v2Tag *tag2 = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);
    bool v = id3v2CompareTag(tag, tag2);

    byteStreamDestroy(stream);
    free(out);
    id3v2DestroyTag(&tag);
    id3v2DestroyTag(&tag2);

    assert_true(v);
}

static void id3v2TagSerialize_v4(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/OnGP.mp3");
    size_t outl = 0;
    uint8_t *out = id3v2TagSerialize(tag, &outl);
    ByteStream *stream = byteStreamCreate(out, outl);
    
    assert_non_null(stream);

    Id3v2Tag *tag2 = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);
    bool v = id3v2CompareTag(tag, tag2);
    

    byteStreamDestroy(stream);
    free(out);
    id3v2DestroyTag(&tag);
    id3v2DestroyTag(&tag2);

    assert_true(v);
}

static void id3v2TagSerialize_v3ext(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");
    tag->header->extendedHeader = id3v2CreateExtendedTagHeader(0, 0x74657374, 0, 0, 0); // crc is equal to test in hex
    id3v2WriteExtendedHeaderIndicator(tag->header, true);
    
    size_t outl = 0;
    uint8_t *out = id3v2TagSerialize(tag, &outl);
    ByteStream *stream = byteStreamCreate(out, outl);

    assert_non_null(stream);

    Id3v2Tag *tag2 = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);

    bool v = id3v2CompareTag(tag, tag2);

    byteStreamDestroy(stream);
    free(out);
    id3v2DestroyTag(&tag);
    id3v2DestroyTag(&tag2);

    assert_true(v);
}

static void id3v2TagSerialize_v4ext(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/OnGP.mp3");
    tag->header->extendedHeader = id3v2CreateExtendedTagHeader(0, 0, 0, 1, 0);
    id3v2WriteExtendedHeaderIndicator(tag->header, true);

    size_t outl = 0;
    uint8_t *out = id3v2TagSerialize(tag, &outl);
    ByteStream *stream = byteStreamCreate(out, outl);
    assert_non_null(stream);

    Id3v2Tag *tag2 = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);

    bool v = id3v2CompareTag(tag, tag2);

    byteStreamDestroy(stream);
    free(out);
    id3v2DestroyTag(&tag);
    id3v2DestroyTag(&tag2);

    assert_true(v);
}

static void id3v2TagSerialize_v4footer(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/OnGP.mp3");
    id3v2WriteFooterIndicator(tag->header, true);

    size_t outl = 0;
    uint8_t *out = id3v2TagSerialize(tag, &outl);
    ByteStream *stream = byteStreamCreate(out, outl);
    assert_non_null(stream);

    Id3v2Tag *tag2 = id3v2ParseTagFromBuffer(stream->buffer, stream->bufferSize, NULL);

    bool v = id3v2CompareTag(tag, tag2);

    byteStreamDestroy(stream);
    free(out);
    id3v2DestroyTag(&tag);
    id3v2DestroyTag(&tag2);

    assert_true(v);
}

/**
 * This test is so computationally expensive that it is not worth running unless its 100% necessary
 * This can take past an hour to run on an M3 with 16gb of ram.
 */

// static void id3v2TagToStream_v4unsync(void **state){
    
//     Id3v2Tag *tag = id3v2TagFromFile("assets/OnGP.mp3");
//     id3v2WriteUnsynchronisationIndicator(tag->header, true);

//     ByteStream *stream = id3v2TagToStream(tag);
//     assert_non_null(stream);

//     Id3v2Tag *tag2 = id3v2ParseTagFromStream(stream, NULL);

//     bool v = id3v2CompareTag(tag, tag2);

//     byteStreamDestroy(stream);
//     id3v2DestroyTag(&tag);
//     id3v2DestroyTag(&tag2);

//     assert_true(v);
// }


static void id3v2TagToJSON_v2(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");
    FILE *fp = NULL;
    char *str = NULL;
    char *fileJson = NULL;
    size_t sz = 0;

    str = id3v2TagToJSON(tag);
    assert_non_null(str);
    id3v2DestroyTag(&tag);

    fp = fopen("assets/danybrown2.json", "rb");
    assert_non_null(fp);

    // NOLINTNEXTLINE
    (void) fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    (void) fseek(fp, 0L, SEEK_SET);

    // NOLINTNEXTLINE
    fileJson = calloc(sizeof(char), sz + 1);
    (void) fread(fileJson, 1, sz, fp);
    (void) fclose(fp);
    
    assert_string_equal(str, fileJson);

    free(fileJson);
    free(str);

}

static void id3v2TagToJSON_v3(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");
    FILE *fp = NULL;
    char *str = NULL;
    char *fileJson = NULL;
    size_t sz = 0;

    str = id3v2TagToJSON(tag);
    assert_non_null(str);
    id3v2DestroyTag(&tag);

    fp = fopen("assets/sorry4dying.json", "rb");
    assert_non_null(fp);

    // NOLINTNEXTLINE
    (void) fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    (void) fseek(fp, 0L, SEEK_SET);

    // NOLINTNEXTLINE
    fileJson = calloc(sizeof(char), sz + 1);
    (void) fread(fileJson, 1, sz, fp);
    (void) fclose(fp);
    
    assert_string_equal(str, fileJson);

    free(fileJson);
    free(str);

}

static void id3v2WriteTagToFile_v2NoFile(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");
    

    int v = id3v2WriteTagToFile("assets/tmp", tag);
    assert_true(v);

    Id3v2Tag *tag2 = id3v2TagFromFile("assets/tmp");

    bool v2 = id3v2CompareTag(tag, tag2);
    assert_true(v2);
    id3v2DestroyTag(&tag);
    id3v2DestroyTag(&tag2);
    (void) remove("assets/tmp");
}

static void id3v2WriteTagToFile_v3Overwrite(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");
    Id3v2Tag *tag2 = NULL;
    FILE *fp = NULL;
    size_t sz = 0;
    uint8_t *data = NULL;

    id3v2WriteAlbum("SCRAPYARD", tag);

    fp = fopen("assets/sorry4dying.mp3", "rb");
    // NOLINTNEXTLINE
    (void) fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    (void) fseek(fp, 0L, SEEK_SET);
    data = malloc(sz);
    (void) fread(data, 1, sz, fp);
    (void) fclose(fp);

    fp = fopen("assets/tmp", "wb");

    assert_non_null(fp);
    // NOLINTNEXTLINE
    (void) fwrite(data, 1, sz, fp);
    free(data);
    (void) fclose(fp);

    id3v2WriteTagToFile("assets/tmp", tag);

    tag2 = id3v2TagFromFile("assets/tmp");

    (void) remove("assets/tmp");

    char *str = id3v2ReadAlbum(tag2);
    assert_string_equal("SCRAPYARD", str);
    free(str);
    id3v2DestroyTag(&tag2);
    id3v2DestroyTag(&tag);

}

static void id3v2WriteTagToFile_v4OverwriteNoPictures(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag2 = NULL;
    FILE *fp = NULL;
    size_t sz = 0;
    uint8_t *data = NULL;

    assert_true(id3v2RemoveFrameByID("APIC", tag));
    assert_true(id3v2RemoveFrameByID("APIC", tag));

    fp = fopen("assets/OnGP.mp3", "rb");

    assert_non_null(fp);
    // NOLINTNEXTLINE
    (void) fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    (void) fseek(fp, 0L, SEEK_SET);
    data = malloc(sz);
    (void) fread(data, 1, sz, fp);
    (void) fclose(fp);

    fp = fopen("assets/tmp", "wb");

    assert_non_null(fp);
    // NOLINTNEXTLINE
    (void) fwrite(data, 1, sz, fp);
    free(data);
    (void) fclose(fp);

    id3v2WriteTagToFile("assets/tmp", tag);

    tag2 = id3v2TagFromFile("assets/tmp");

    (void) remove("assets/tmp");

    assert_false(id3v2ReadFrameByID("APIC", tag2));

    id3v2DestroyTag(&tag2);
    id3v2DestroyTag(&tag);

}

static void id3v2WriteTagToFile_v4OverwriteNoPicturesAsUpdate(void **state){
    (void) state;
    Id3v2Tag *tag = id3v2TagFromFile("assets/OnGP.mp3");
    Id3v2Tag *tag2 = NULL;
    FILE *fp = NULL;
    size_t sz = 0;
    uint8_t *data = NULL;

    assert_true(id3v2RemoveFrameByID("APIC", tag));
    assert_true(id3v2RemoveFrameByID("APIC", tag));
    id3v2WriteExtendedHeaderIndicator(tag->header, true);
    tag->header->extendedHeader = id3v2CreateExtendedTagHeader(0, 0, 1, 0, 0);


    fp = fopen("assets/OnGP.mp3", "rb");

    if (fp == NULL) {
        fail_msg("Failed to open file assets/OnGP.mp3");
    }

    (void) fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    (void) fseek(fp, 0L, SEEK_SET);
    data = malloc(sz);
    (void) fread(data, 1, sz, fp);
    (void) fclose(fp);

    fp = fopen("assets/tmp", "wb");

    if (fp == NULL) {
        fail_msg("Failed to open file assets/tmp for writing");
    }

    (void) fwrite(data, 1, sz, fp);
    free(data);
    (void) fclose(fp);

    id3v2WriteTagToFile("assets/tmp", tag);

    tag2 = id3v2TagFromFile("assets/tmp");

    ByteStream *stream = byteStreamFromFile("assets/tmp");
    byteStreamSeek(stream, 6, SEEK_SET);
    uint32_t size = byteStreamReturnSyncInt(stream);
    byteStreamSeek(stream, size, SEEK_CUR);
    
    assert_int_equal(byteStreamGetCh(stream), 'I');
    byteStreamSeek(stream, 1, SEEK_CUR);
    assert_int_equal(byteStreamGetCh(stream), 'D');
    byteStreamSeek(stream, 1, SEEK_CUR);
    assert_int_equal(byteStreamGetCh(stream), '3');


    (void) remove("assets/tmp");

    assert_false(id3v2ReadFrameByID("APIC", tag2));
    byteStreamDestroy(stream);
    id3v2DestroyTag(&tag2);
    id3v2DestroyTag(&tag);

}

int main(){
    const struct CMUnitTest tests[] = {
        
        // id3v2TagFromFile
        cmocka_unit_test(id3v2TagFromFile_v3),
        cmocka_unit_test(id3v2TagFromFile_null),

        // id3v2CopyTag
        cmocka_unit_test(id3v2CopyTag_v3),

        // id3v2CompareTag
        cmocka_unit_test(id3v2CompareTag_v3v4),
        cmocka_unit_test(id3v2CompareTag_v3same),

        // id3v2ReadFrameByID
        cmocka_unit_test(id3v2ReadFrameByID_v3),
        cmocka_unit_test(id3v2ReadFrameByID_v3MultiTXXX),
        cmocka_unit_test(id3v2ReadFrameByID_v2Null),

        // id3v2RemoveFrameByID
        cmocka_unit_test(id3v2RemoveFrameByID_v3EveryFrame),
        cmocka_unit_test(id3v2RemoveFrameByID_Null),

        // id3v2ReadTextFrameContent
        cmocka_unit_test(id3v2ReadTextFrameContent_TRK),
        cmocka_unit_test(id3v2ReadTextFrameContent_TXX),
        cmocka_unit_test(id3v2ReadTextFrameContent_PIC),

        // id3v2ReadTitle
        cmocka_unit_test(id3v2ReadTitle_TT2),
        cmocka_unit_test(id3v2ReadTitle_TIT2),

        // id3v2ReadArtist
        cmocka_unit_test(id3v2ReadArtist_TP1),
        cmocka_unit_test(id3v2ReadArtist_TPE1),

        // id3v2ReadAlbumArtist
        cmocka_unit_test(id3v2ReadArtist_TP2),
        cmocka_unit_test(id3v2ReadArtist_TPE2),

        // id3v2ReadAlbum
        cmocka_unit_test(id3v2ReadArtist_TAL),
        cmocka_unit_test(id3v2ReadArtist_TALB),

        // id3v2ReadYear
        cmocka_unit_test(id3v2ReadYear_TYE),
        cmocka_unit_test(id3v2ReadYear_TYER),

        // id3v2ReadGenre
        cmocka_unit_test(id3v2ReadGenre_TCO),
        cmocka_unit_test(id3v2ReadGenre_TCON),

        // id3v2ReadTrack
        cmocka_unit_test(id3v2ReadTrack_TRK),
        cmocka_unit_test(id3v2ReadTrack_TRCK),

        // id3v2ReadComposer
        cmocka_unit_test(id3v2ReadComposer_TCM),
        cmocka_unit_test(id3v2ReadComposer_TCOM),

        // id3v2ReadDisc
        cmocka_unit_test(id3v2ReadDisc_TPA),
        cmocka_unit_test(id3v2ReadDisc_TPOS),

        // id3v2ReadLyrics
        cmocka_unit_test(id3v2ReadLyrics_ULT),
        cmocka_unit_test(id3v2ReadLyrics_Null),

        // id3v2ReadComment
        cmocka_unit_test(id3v2ReadComment_COM),
        cmocka_unit_test(id3v2ReadComment_Null),

        // id3v2ReadPicture
        cmocka_unit_test(id3v2ReadPicture_PIC),
        cmocka_unit_test(id3v2ReadPicture_APIC),

        // id3v2WriteTextFrameContent
        cmocka_unit_test(id3v2WriteTextFrameContent_TIT2),
        cmocka_unit_test(id3v2WriteTextFrameContent_TCOM),

        // id3v2WriteTitle
        cmocka_unit_test(id3v2WriteTitle_TT2),
        cmocka_unit_test(id3v2WriteTitle_TIT2),

        // id3v2WriteArtist
        cmocka_unit_test(id3v2WriteArtist_TP1),
        cmocka_unit_test(id3v2WriteArtist_TEP1),

        // id3v2WriteAlbumArtist
        cmocka_unit_test(id3v2WriteAlbumArtist_TP2),
        cmocka_unit_test(id3v2WriteAlbumArtist_TEP2),

        // id3v2WriteAlbum
        cmocka_unit_test(id3v2WriteAlbum_TAL),
        cmocka_unit_test(id3v2WriteAlbum_TALB),

        // id3v2WriteYear
        cmocka_unit_test(id3v2WriteYear_TYE),
        cmocka_unit_test(id3v2WriteYear_TYER),

        // id3v2WriteGenre
        cmocka_unit_test(id3v2WriteGenre_TCO),
        cmocka_unit_test(id3v2WriteGenre_TCON),

        // id3v2WriteTrack
        cmocka_unit_test(id3v2WriteTrack_TRK),
        cmocka_unit_test(id3v2WriteTrack_TRCK),

        // id3v2WriteDisc
        cmocka_unit_test(id3v2WriteDisc_TPA),
        cmocka_unit_test(id3v2WriteDisc_TPOS),

        // id3v2WriteComposer
        cmocka_unit_test(id3v2WriteComposer_TCM),
        cmocka_unit_test(id3v2WriteComposer_TCOM),

        // id3v2WriteLyrics
        cmocka_unit_test(id3v2WriteLyrics_ULT),
        cmocka_unit_test(id3v2WriteLyrics_ULT2),

        // id3v2WriteComment
        cmocka_unit_test(id3v2WriteComment_COM),
        cmocka_unit_test(id3v2WriteComment_COMM),

        // id3v2WritePicture
        cmocka_unit_test(id3v2WritePicture_PIC),

        // id3v2WritePictureFromFile
        cmocka_unit_test(id3v2WritePictureFromFile_PIC),

        // id3v2InsertTextFrame
        cmocka_unit_test(id3v2InsertTextFrame_TSOA),
        cmocka_unit_test(id3v2InsertTextFrame_TSOAnoString),
        cmocka_unit_test(id3v2InsertTextFrame_NoID),

        // id3v2TagToStream
        cmocka_unit_test(id3v2TagSerialize_v3),
        cmocka_unit_test(id3v2TagSerialize_v2),
        cmocka_unit_test(id3v2TagSerialize_v4),
        cmocka_unit_test(id3v2TagSerialize_v3ext),
        cmocka_unit_test(id3v2TagSerialize_v4ext),
        cmocka_unit_test(id3v2TagSerialize_v4footer),
        
        // outdated!
        // cmocka_unit_test(id3v2TagToStream_v4unsync),

        // id3v2TagToJSON
        cmocka_unit_test(id3v2TagToJSON_v2),
        cmocka_unit_test(id3v2TagToJSON_v3),

        // id3v2WriteTagToFile
        cmocka_unit_test(id3v2WriteTagToFile_v2NoFile),
        cmocka_unit_test(id3v2WriteTagToFile_v3Overwrite),
        cmocka_unit_test(id3v2WriteTagToFile_v4OverwriteNoPictures),
        cmocka_unit_test(id3v2WriteTagToFile_v4OverwriteNoPicturesAsUpdate)

    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}