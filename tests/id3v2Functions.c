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
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "id3v2.h"
#include "id3v2Frame.h"

static void id3v2TagFromFile_v3(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_non_null(tag);

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    int c = 0;
    while((f = id3v2FrameTraverse(&frames)) != NULL){
        c++;
    }
    
    assert_int_equal(c, 15);

    id3v2DestroyTag(&tag);
}

static void id3v2TagFromFile_null(void **state){

    Id3v2Tag *tag = id3v2TagFromFile(NULL);

    assert_null(tag);
}


static void id3v2CopyTag_v3(void **state){

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

    Id3v2Tag *tag1 = id3v2TagFromFile("assets/sorry4dying.mp3");
    Id3v2Tag *tag2 = id3v2TagFromFile("assets/OnGP.mp3");

    assert_false(id3v2CompareTag(tag1, tag2));

    id3v2DestroyTag(&tag1);
    id3v2DestroyTag(&tag2);

}

static void id3v2CompareTag_v3same(void **state){

    Id3v2Tag *tag1 = id3v2TagFromFile("assets/sorry4dying.mp3");
    Id3v2Tag *tag2 = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_true(id3v2CompareTag(tag1, tag2));

    id3v2DestroyTag(&tag1);
    id3v2DestroyTag(&tag2);

}

static void id3v2ReadFrameByID_v3(void **state){

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

    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    Id3v2Frame *frame = id3v2ReadFrameByID("XXX", tag);
    assert_null(frame);

    id3v2DestroyTag(&tag);

}

static void id3v2RemoveFrameByID_v3EveryFrame(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    int total = tag->frames->length;

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

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_false(id3v2RemoveFrameByID("ASWA", tag));

    id3v2DestroyTag(&tag);


}


static void id3v2ReadTextFrameContent_TRK(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadTextFrameContent("TRK", tag);

    assert_non_null(str);
    assert_string_equal(str, "06/15");

    free(str);

    id3v2DestroyTag(&tag);

}

static void id3v2ReadTextFrameContent_TXX(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadTextFrameContent("TXX", tag);

    assert_null(str);

    id3v2DestroyTag(&tag);

}

static void id3v2ReadTextFrameContent_PIC(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadTextFrameContent("PIC", tag);

    assert_null(str);

    id3v2DestroyTag(&tag);

}

static void id3v2ReadTitle_TT2(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadTitle(tag);

    assert_non_null(str);

    assert_string_equal(str, "Ain't It Funny");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadTitle_TIT2(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadTitle(tag);

    assert_non_null(str);

    assert_string_equal(str, "sorry4dying");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadArtist_TP1(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadArtist(tag);

    assert_non_null(str);

    assert_string_equal(str, "Danny Brown");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadArtist_TPE1(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadArtist(tag);

    assert_non_null(str);

    assert_string_equal(str, "Quadeca");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadArtist_TP2(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadAlbumArtist(tag);

    assert_non_null(str);

    assert_string_equal(str, "Danny Brown");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadArtist_TPE2(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadAlbumArtist(tag);

    assert_non_null(str);

    assert_string_equal(str, "Quadeca");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadArtist_TAL(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadAlbum(tag);

    assert_non_null(str);

    assert_string_equal(str, "Atrocity Exhibition");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadArtist_TALB(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadAlbum(tag);

    assert_non_null(str);

    assert_string_equal(str, "I Didn't Mean To Haunt You");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadYear_TYE(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadYear(tag);

    assert_non_null(str);

    assert_string_equal(str, "2016");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadYear_TYER(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadYear(tag);

    assert_non_null(str);

    assert_string_equal(str, "2022");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadGenre_TCO(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadGenre(tag);

    assert_non_null(str);

    assert_string_equal(str, "Experimental Hip-Hop, Hardcore Hip-Hop, Abstract Hip-Hop, Industrial Hip-Hop & Post-Punk");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadGenre_TCON(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadGenre(tag);

    assert_non_null(str);

    assert_string_equal(str, "Art Pop, Folktronica Glitch Pop, Ambient Pop, Experimental Hip-Hop, Neo-Psychedelia, Alternative R&B, Emo Rap & Chamber Pop");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadTrack_TRK(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadTrack(tag);

    assert_non_null(str);

    assert_string_equal(str, "06/15");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadTrack_TRCK(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadTrack(tag);

    assert_non_null(str);

    assert_string_equal(str, "01/11");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadComposer_TCM(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadComposer(tag);

    assert_non_null(str);

    assert_string_equal(str, "Danny Brown");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadComposer_TCOM(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadComposer(tag);

    assert_non_null(str);

    assert_string_equal(str, "Ben Lasky");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadDisc_TPA(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadDisc(tag);

    assert_non_null(str);

    assert_string_equal(str, "01/01");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadDisc_TPOS(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadDisc(tag);

    assert_non_null(str);

    assert_string_equal(str, "1/1");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadLyrics_ULT(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadLyrics(tag);

    assert_non_null(str);

    assert_string_equal(str, "haBDJHAsbdjkHASBDJahbsdkAHBSDHAbsdHBDUAHSBDUBAUIBFOASIUBDFOIAUBFOIAUWBFOAWBFAOUWEBFUOYBOUBUOBUOboubouboubouboubouboigndoignoisnjgsdfjnglksjdfngslkjfngskdjfnglskdnfgiserugisugnvfkdxjnvxlkjnijxdngixjdhfgoiserhgiusdng spoerijgsoergjnposeirhgposergn reigjosperijgsodfkgkldfmvxc.,vbm");
    free(str);
    id3v2DestroyTag(&tag);
}


static void id3v2ReadLyrics_Null(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadLyrics(tag);

    assert_null(str);

    id3v2DestroyTag(&tag);
}

static void id3v2ReadComment_COM(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/danybrown2.mp3");

    char *str = id3v2ReadComment(tag);

    assert_non_null(str);

    assert_string_equal(str, "test");
    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadComment_Null(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    char *str = id3v2ReadComment(tag);

    assert_null(str);
    id3v2DestroyTag(&tag);
}

static void id3v2ReadPicture_PIC(void **state){

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

static void id3v2ReadPictre_APIC(void **state){

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

    Id3v2Tag *tag = id3v2TagFromFile("assets/OnGP.mp3");

    assert_true(id3v2WriteTextFrameContent("TIT2", "test", tag));

    char *str = id3v2ReadTitle(tag);
    assert_non_null(str);
    assert_string_equal(str, "test");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteTextFrameContent_WCOM(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/OnGP.mp3");

    assert_false(id3v2WriteTextFrameContent("WCOM", "test", tag));

    id3v2DestroyTag(&tag);
}

static void id3v2WriteTitle_TT2(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/boniver.mp3");

    assert_true(id3v2WriteTitle("death breast", tag));

    char *str = id3v2ReadTitle(tag);
    assert_non_null(str);
    assert_string_equal(str, "death breast");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteTitle_TIT2(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_true(id3v2WriteTitle("title", tag));

    char *str = id3v2ReadTitle(tag);
    assert_non_null(str);
    assert_string_equal(str, "title");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteArtist_TP1(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/boniver.mp3");

    assert_true(id3v2WriteArtist("good winter", tag));

    char *str = id3v2ReadArtist(tag);
    assert_non_null(str);
    assert_string_equal(str, "good winter");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteArtist_TEP1(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_true(id3v2WriteArtist("t", tag));

    char *str = id3v2ReadArtist(tag);
    assert_non_null(str);
    assert_string_equal(str, "t");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteAlbumArtist_TP2(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/boniver.mp3");

    assert_true(id3v2WriteAlbumArtist("justin", tag));

    char *str = id3v2ReadAlbumArtist(tag);
    assert_non_null(str);
    assert_string_equal(str, "justin");

    free(str);
    id3v2DestroyTag(&tag);
}

static void id3v2WriteAlbumArtist_TEP2(void **state){

    Id3v2Tag *tag = id3v2TagFromFile("assets/sorry4dying.mp3");

    assert_true(id3v2WriteAlbumArtist("ben", tag));

    char *str = id3v2ReadAlbumArtist(tag);
    assert_non_null(str);
    assert_string_equal(str, "ben");

    free(str);
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
        cmocka_unit_test(id3v2ReadPictre_APIC),

        // id3v2WriteTextFrameContent
        cmocka_unit_test(id3v2WriteTextFrameContent_TIT2),
        cmocka_unit_test(id3v2WriteTextFrameContent_WCOM),

        // id3v2WriteTitle
        cmocka_unit_test(id3v2WriteTitle_TT2),
        cmocka_unit_test(id3v2WriteTitle_TIT2),

        // id3v2WriteArtist
        cmocka_unit_test(id3v2WriteArtist_TP1),
        cmocka_unit_test(id3v2WriteArtist_TEP1),

        // id3v2WriteArtist
        cmocka_unit_test(id3v2WriteAlbumArtist_TP2),
        cmocka_unit_test(id3v2WriteAlbumArtist_TEP2)

    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}