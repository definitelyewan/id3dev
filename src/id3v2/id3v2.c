#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "byteStream.h"
#include "id3v2.h"
#include "id3v2Frame.h"
#include "id3v2Parser.h"
#include "id3v2TagIdentity.h"

/**
 * @brief Generates a Id3v2Tag structure from a file. If
 * this function fails to generate a tag, it will return NULL.
 * 
 * @param filename 
 * @return Id3v2Tag* 
 */
Id3v2Tag *id3v2TagFromFile(const char *filename){

    ByteStream *stream = NULL;
    Id3v2Tag *tag = NULL;

    if(filename == NULL){
        return NULL;
    }

    if((stream = byteStreamFromFile(filename)) == NULL){
        return NULL;
    }

    tag = id3v2ParseTagFromStream(stream, NULL);
    byteStreamDestroy(stream);

    return tag;
}

/**
 * @brief Deep copies an Id3v2Tag structure.
 * 
 * @param toCopy 
 * @return Id3v2Tag* 
 */
Id3v2Tag *id3v2CopyTag(Id3v2Tag *toCopy){

    Id3v2TagHeader *header = NULL;

    if(toCopy == NULL){
        return NULL;
    }

    if(toCopy->header == NULL){
        return NULL;
    }
    
    header = id3v2CreateTagHeader(toCopy->header->majorVersion, toCopy->header->minorVersion, toCopy->header->flags, NULL);
    header->extendedHeader = (toCopy->header->extendedHeader != NULL) ? id3v2CreateExtendedTagHeader(toCopy->header->extendedHeader->padding, toCopy->header->extendedHeader->crc, toCopy->header->extendedHeader->update, toCopy->header->extendedHeader->tagRestrictions, toCopy->header->extendedHeader->restrictions) : NULL;

    return id3v2CreateTag(header, listDeepCopy(toCopy->frames));
}

/**
 * @brief Compares two tags and returns true if they are the same, false otherwise.
 * 
 * @param tag1 
 * @param tag2 
 * @return true 
 * @return false 
 */
bool id3v2CompareTag(Id3v2Tag *tag1, Id3v2Tag *tag2){

    if(tag1 == NULL || tag2 == NULL){
        return false;
    }

    if(tag1->header == NULL || tag2->header == NULL){
        return false;

    }

    // check header

    if(tag1->header->majorVersion != tag2->header->majorVersion){
        return false;
    }

    if(tag1->header->minorVersion != tag2->header->minorVersion){
        return false;
    }

    if(tag1->header->flags != tag2->header->flags){
        return false;
    }

    if(tag1->header->extendedHeader != tag2->header->extendedHeader){
        return false;

    }

    // check extended header

    if(tag1->header->extendedHeader != NULL && tag2->header->extendedHeader != NULL){
        if(tag1->header->extendedHeader->padding != tag2->header->extendedHeader->padding){
            return false;
        }

        if(tag1->header->extendedHeader->crc != tag2->header->extendedHeader->crc){
            return false;
        }

        if(tag1->header->extendedHeader->update != tag2->header->extendedHeader->update){
            return false;
        }

        if(tag1->header->extendedHeader->tagRestrictions != tag2->header->extendedHeader->tagRestrictions){
            return false;
        }

        if(tag1->header->extendedHeader->restrictions != tag2->header->extendedHeader->restrictions){
            return false;
        }

    }

    // check frames

    ListIter frames1 = id3v2CreateFrameTraverser(tag1);
    ListIter frames2 = id3v2CreateFrameTraverser(tag2);

    Id3v2Frame *f1 = NULL;
    Id3v2Frame *f2 = NULL;


    while((f1 = id3v2FrameTraverse(&frames1)) != NULL){
        f2 = id3v2FrameTraverse(&frames2);

        if(f2 == NULL){
            return false;
        }

        if(id3v2CompareFrame(f1, f2) != 0){
            return false;
        }
    }

    if((f2 = id3v2FrameTraverse(&frames2)) != NULL){
        return false;
    }
    
    return true;
}

/**
 * @brief Returns a copy of the first occurance of a frame with the given id. If
 * no frame is found, this function will return NULL.
 * 
 * @param id 
 * @param tag 
 * @return Id3v2Frame* 
 */
Id3v2Frame *id3v2ReadFrameByID(const char id[ID3V2_FRAME_ID_MAX_SIZE], Id3v2Tag *tag){

    if(id == NULL || tag == NULL){
        return NULL;
    }

    int i = 0;
    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;
    

    // sanitize id
    for(i = 0; i < ID3V2_FRAME_ID_MAX_SIZE; i++){
        if(id[i] == '\0'){
            break;
        }
    }

    // find frame
    while((f = id3v2FrameTraverse(&frames)) != NULL){
        if(memcmp(id, f->header->id, i) == 0){
            return id3v2CopyFrame(f);
        }
    }

    return NULL;
}

/**
 * @brief Removes a frame from a tag with a matching ID. if no frame is found, 
 * this function will return 0. Otherwise, it will return 1.
 * 
 * @param id 
 * @param tag 
 * @return int 
 */
int id3v2RemoveFrameByID(const char *id, Id3v2Tag *tag){
    
    Id3v2Frame *test = NULL;
    Id3v2Frame *remove = NULL;

    test = id3v2ReadFrameByID(id, tag);
    remove = id3v2DetatchFrameFromTag(tag, test);

    id3v2DestroyFrame(&test);

    if(remove != NULL){
        id3v2DestroyFrame(&remove);
        return 1;
    }

    return 0;
}

/**
 * @brief Reads the content of a text frame with the given id. If no frame is found,
 * NULL is returned.
 * 
 * @param id 
 * @param tag 
 * @return char* 
 */
char *id3v2ReadTextFrameContent(const char id[ID3V2_FRAME_ID_MAX_SIZE], Id3v2Tag *tag){
    
    Id3v2Frame *frame = id3v2ReadFrameByID(id, tag);
    ListIter entries = {0};
    ListIter context = {0};
    Id3v2ContentContext *cc = NULL;
    size_t dataSize = 0;
    char *ret = NULL;

    if(frame == NULL){
        return NULL;
    }

    // verify text frame via context
    if(frame->contexts->length != 2 && frame->entries->length != 2){
        id3v2DestroyFrame(&frame);
        return NULL;
    }

    context = listCreateIterator(frame->contexts);

    for(int i = 0; i < 2; i++){
        cc = listIteratorNext(&context);

        if(cc == NULL){
            id3v2DestroyFrame(&frame);
            return NULL;
        }

        if(i == 0){
            if(cc->type != numeric_context){
                id3v2DestroyFrame(&frame);
                return NULL;
            }
        }else if(i == 1){
            if(cc->type != encodedString_context){
                id3v2DestroyFrame(&frame);
                return NULL;
            }
        }
    }

    // read text frame
    entries = id3v2CreateFrameEntryTraverser(frame);
    id3v2ReadFrameEntryAsU8(&entries);

    ret = id3v2ReadFrameEntryAsChar(&entries, &dataSize);
    id3v2DestroyFrame(&frame);

    return ret;
}

/**
 * @brief Reads the first Title/Song Name/Content Description (TT2 or TIT2) of a tag. 
 * If no title is found, NULL is returned.
 * 
 * @param tag 
 * @return char* 
 */
char *id3v2ReadTitle(Id3v2Tag *tag){
    
    if(tag == NULL){
        return NULL;
    }

    char *str = NULL;

    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            str = id3v2ReadTextFrameContent("TT2", tag);
            break;
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            str = id3v2ReadTextFrameContent("TIT2", tag);
            break;
        default:
            return NULL;
    }

    return str;
}

/**
 * @brief Reads the first Lead artist/Lead performer/Soloist/Performing group (TP1 or TPE1) 
 * of a tag. If no artist is found, NULL is returned.
 * 
 * @param tag 
 * @return char* 
 */
char *id3v2ReadArtist(Id3v2Tag *tag){
        
        if(tag == NULL){
            return NULL;
        }
    
        char *str = NULL;
    
        switch(tag->header->majorVersion){
            case ID3V2_TAG_VERSION_2:
                str = id3v2ReadTextFrameContent("TP1", tag);
                break;
            case ID3V2_TAG_VERSION_3:
            case ID3V2_TAG_VERSION_4:
                str = id3v2ReadTextFrameContent("TPE1", tag);
                break;
            default:
                return NULL;
        }
    
        return str;
}
/**
 * @brief Reads the first Band/Orchestra/Accompaniment (TP2 or TPE2) of a tag. 
 * If no album artist is found, NULL is returned.
 * 
 * @param tag 
 * @return char* 
 */
char *id3v2ReadAlbumArtist(Id3v2Tag *tag){
        
        if(tag == NULL){
            return NULL;
        }
    
        char *str = NULL;
    
        switch(tag->header->majorVersion){
            case ID3V2_TAG_VERSION_2:
                str = id3v2ReadTextFrameContent("TP2", tag);
                break;
            case ID3V2_TAG_VERSION_3:
            case ID3V2_TAG_VERSION_4:
                str = id3v2ReadTextFrameContent("TPE2", tag);
                break;
            default:
                return NULL;
        }
    
        return str;
}

/**
 * @brief Reads the first Album/Movie/Show title (TAL or TALB) of a tag. 
 * If no album is found, NULL is returned.
 * 
 * @param tag 
 * @return char* 
 */
char *id3v2ReadAlbum(Id3v2Tag *tag){

    if(tag == NULL){
        return NULL;
    }

    char *str = NULL;

    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            str = id3v2ReadTextFrameContent("TAL", tag);
            break;
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            str = id3v2ReadTextFrameContent("TALB", tag);
            break;
        default:
            return NULL;

    }

    return str;
}

/**
 * @brief Reads the first Year (TYE or TYER) of a tag. If no year is found, 
 * NULL is returned.
 * 
 * @param tag 
 * @return char* 
 */
char *id3v2ReadYear(Id3v2Tag *tag){

    if(tag == NULL){
        return NULL;
    }

    char *str = NULL;

    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            str = id3v2ReadTextFrameContent("TYE", tag);
            break;
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            str = id3v2ReadTextFrameContent("TYER", tag);
            break;
        default:
            return NULL;

    }

    return str;
}

/**
 * @brief Reads the first Track number/Position in set (TRK or TRCK) of a tag.
 * If no track is found, NULL is returned.
 * 
 * @param tag 
 * @return char* 
 */
char *id3v2ReadGenre(Id3v2Tag *tag){

    if(tag == NULL){
        return NULL;
    }

    char *str = NULL;

    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            str = id3v2ReadTextFrameContent("TCO", tag);
            break;
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            str = id3v2ReadTextFrameContent("TCON", tag);
            break;
        default:
            return NULL;
    }

    return str;
}

/**
 * @brief Reads the first Track number/Position in set (TRK or TRCK) of a tag.
 * If no track is found, NULL is returned.
 * 
 * @param tag 
 * @return char* 
 */
char *id3v2ReadTrack(Id3v2Tag *tag){

    if(tag == NULL){
        return NULL;

    }

    char *str = NULL;

    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            str = id3v2ReadTextFrameContent("TRK", tag);
            break;
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            str = id3v2ReadTextFrameContent("TRCK", tag);
            break;
        default:
            return NULL;
    }

    return str;
}

/**
 * @brief Reads the first composer (TCM or TCOM) of a tag. If no composer is found,
 * NULL is returned.
 * 
 * @param tag 
 * @return char* 
 */
char *id3v2ReadComposer(Id3v2Tag *tag){

    if(tag == NULL){
        return NULL;
    }

    char *str = NULL;

    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            str = id3v2ReadTextFrameContent("TCM", tag);
            break;
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            str = id3v2ReadTextFrameContent("TCOM", tag);
            break;
        default:
            return NULL;
    }

    return str;
}

/**
 * @brief Reads the first Disc number (TPA or TPOS) of a tag. If no disc is found,
 * NULL is returned.
 * 
 * @param tag 
 * @return char* 
 */
char *id3v2ReadDisc(Id3v2Tag *tag){

    if(tag == NULL){
        return NULL;
    }

    char *str = NULL;

    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            str = id3v2ReadTextFrameContent("TPA", tag);
            break;
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            str = id3v2ReadTextFrameContent("TPOS", tag);
            break;
        default:
            return NULL;
    }

    return str;
}

/**
 * @brief Reads the first Unsynchronised Lyric/Text Transcription (ULT or USLT) of a tag.
 * If no lyrics are found, NULL is returned.
 * 
 * @param tag 
 * @return char* 
 */
char *id3v2ReadLyrics(Id3v2Tag *tag){

    Id3v2Frame *frame = NULL;
    ListIter entries = {0};
    ListIter context = {0};
    Id3v2ContentContext *cc = NULL;
    size_t dataSize = 0;
    char *ret = NULL;

    if(tag == NULL){
        return NULL;
    }


    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            frame = id3v2ReadFrameByID("ULT", tag);
            break;
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            frame = id3v2ReadFrameByID("USLT", tag);
            break;
        default:
            return NULL;
    }

    if(frame == NULL){
        return NULL;
    }

    // verify lyric frame via context
    if(frame->contexts->length != 4 && frame->entries->length != 4){
        id3v2DestroyFrame(&frame);
        return NULL;
    }

    context = listCreateIterator(frame->contexts);

    for(int i = 0; i < 4; i++){
        cc = listIteratorNext(&context);

        if(cc == NULL || (i == 0 && cc->type != numeric_context) ||
            (i == 1 && cc->type != noEncoding_context) ||
            (i == 2 && cc->type != encodedString_context) ||
            (i == 3 && cc->type != encodedString_context)){
            id3v2DestroyFrame(&frame);
            return NULL;
        }
    }

    // read text frame
    entries = id3v2CreateFrameEntryTraverser(frame);
    id3v2ReadFrameEntryAsU8(&entries);
    id3v2ReadFrameEntryAsU8(&entries);
    id3v2ReadFrameEntryAsU8(&entries);

    ret = id3v2ReadFrameEntryAsChar(&entries, &dataSize);
    id3v2DestroyFrame(&frame);

    return ret;
}

/**
 * @brief Reads the first Comment (COM or COMM) of a tag. If no comment is found,
 * NULL is returned.
 * 
 * @param tag 
 * @return char* 
 */
char *id3v2ReadComment(Id3v2Tag *tag){

    Id3v2Frame *frame = NULL;
    ListIter entries = {0};
    ListIter context = {0};
    Id3v2ContentContext *cc = NULL;
    size_t dataSize = 0;
    char *ret = NULL;

    if(tag == NULL){
        return NULL;
    }


    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            frame = id3v2ReadFrameByID("COM", tag);
            break;
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            frame = id3v2ReadFrameByID("COMM", tag);
            break;
        default:
            return NULL;
    }

    if(frame == NULL){
        return NULL;
    }

    // verify lyric frame via context
    if(frame->contexts->length != 4 && frame->entries->length != 4){
        id3v2DestroyFrame(&frame);
        return NULL;
    }

    context = listCreateIterator(frame->contexts);

    for(int i = 0; i < 4; i++){
        cc = listIteratorNext(&context);

        if(cc == NULL || (i == 0 && cc->type != numeric_context) ||
            (i == 1 && cc->type != noEncoding_context) ||
            (i == 2 && cc->type != encodedString_context) ||
            (i == 3 && cc->type != encodedString_context)){
            id3v2DestroyFrame(&frame);
            return NULL;
        }
    }

    // read text frame
    entries = id3v2CreateFrameEntryTraverser(frame);
    id3v2ReadFrameEntryAsU8(&entries);
    id3v2ReadFrameEntryAsU8(&entries);
    id3v2ReadFrameEntryAsU8(&entries);

    ret = id3v2ReadFrameEntryAsChar(&entries, &dataSize);
    id3v2DestroyFrame(&frame);

    return ret;

}

/**
 * @brief Reads the first Attached pircutre (PIC or APIC) of a tag. 
 * If no picture is found, NULL is returned.
 * 
 * @param type 
 * @param tag 
 * @return char* 
 */
uint8_t *id3v2ReadPicture(uint8_t type, Id3v2Tag *tag, size_t *dataSize){
    
    *dataSize = 0;

    if(tag == NULL){
        return NULL;
    }

    Id3v2Frame *f = NULL;
    ListIter frames = listCreateIterator(tag->frames);
    uint8_t usableType = ((type > 0x14) ? 0x00 : type); // clamp type

    while((f = listIteratorNext(&frames)) != NULL){
        
        ListIter entries = id3v2CreateFrameEntryTraverser(f);

        if(memcmp("PIC",f->header->id, ID3V2_FRAME_ID_MAX_SIZE - 1) == 0 ||
           memcmp("APIC",f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){

            id3v2ReadFrameEntryAsU8(&entries); // encoding
            id3v2ReadFrameEntryAsU8(&entries); // mime type

            if(id3v2ReadFrameEntryAsU8(&entries) == usableType){

                id3v2ReadFrameEntryAsU8(&entries); // description
                return id3v2ReadFrameEntry(&entries, dataSize);
            }
        }
    }

    return NULL;
}

/**
 * @brief Writes a null-terminated string to a text frame with a given id.
 * If the frame does not exist, this function fails. Any failure will result
 * in this function returning false otherwise, true.
 * 
 * @param string 
 * @param tag 
 * @return int 
 */
int id3v2WriteTextFrameContent(const char id[ID3V2_FRAME_ID_MAX_SIZE], const char *string, Id3v2Tag *tag){

    if(id == NULL || string == NULL || tag == NULL){
        return false;

    }

    Id3v2Frame *f = NULL;
    ListIter frames = id3v2CreateFrameTraverser(tag);
    ListIter context = {0};
    ListIter entries = {0};
    Id3v2ContentContext *cc = NULL;
    int i = 0;


    // sanitize id
    for(i = 0; i < ID3V2_FRAME_ID_MAX_SIZE; i++){
        if(id[i] == '\0'){
            break;
        }
    }

    while((f = id3v2FrameTraverse(&frames)) != NULL){

        if(memcmp(id, f->header->id, i) == 0){
            break;
        }
    }

    // verify text frame via context
    if(f->contexts->length != 2 && f->entries->length != 2){
        return false;
    }

    context = listCreateIterator(f->contexts);

    for(int i = 0; i < 2; i++){
        cc = listIteratorNext(&context);

        if(cc == NULL || (i == 0 && cc->type != numeric_context) ||
            (i == 1 && cc->type != encodedString_context)){
            return false;
        }
    }

    entries = id3v2CreateFrameEntryTraverser(f);

    id3v2ReadFrameEntryAsU8(&entries); // encoding
    
    return id3v2WriteFrameEntry(f, &entries, strlen(string), (void *) string);
}

/**
 * @brief Writes a new title to the first Title/Song Name/Content Description (TT2 or TIT2) 
 * frame of a tag. If no title is found, false is returned otherwise, true.
 * @param title 
 * @param tag 
 * @return int 
 */
int id3v2WriteTitle(const char *title, Id3v2Tag *tag){

    if(title == NULL || tag == NULL){
        return false;
    }

    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            return id3v2WriteTextFrameContent("TT2", title, tag);
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteTextFrameContent("TIT2", title, tag);
        default:
            return false;
    }

    // dummy
    return false;
}

/**
 * @brief Writes a new artist to the first Lead artist/Lead performer/Soloist/Performing group (TP1 or TPE1)
 * frame of a tag. If no artist is found, false is returned otherwise, true.
 * 
 * @param artist 
 * @param tag 
 * @return int 
 */
int id3v2WriteArtist(const char *artist, Id3v2Tag *tag){

    if(artist == NULL || tag == NULL){
        return false;
    }

    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            return id3v2WriteTextFrameContent("TP1", artist, tag);
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteTextFrameContent("TPE1", artist, tag);
        default:
            return false;
    }

    // dummy
    return false;
}

/**
 * @brief Writes a new album artist to the first Band/Orchestra/Accompaniment (TP2 or TPE2) frame
 * of a tag. If no album artist is found, false is returned otherwise, true.
 * 
 * @param albumArtist 
 * @param tag 
 * @return int 
 */
int id3v2WriteAlbumArtist(const char *albumArtist, Id3v2Tag *tag){

    if(albumArtist == NULL || tag == NULL){
        return false;
    }

    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            return id3v2WriteTextFrameContent("TP2", albumArtist, tag);
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteTextFrameContent("TPE2", albumArtist, tag);
        default:
            return false;
    }

    // dummy
    return false;
}
