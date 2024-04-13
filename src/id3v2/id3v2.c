/**
 * @file id3v2.c
 * @author Ewan Jones
 * @brief Contains all main functions for the id3v2 library.
 * @version 0.1
 * @date 2024-04-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "byteStream.h"
#include "byteUnicode.h"
#include "id3v2/id3v2.h"
#include "id3v2/id3v2Frame.h"
#include "id3v2/id3v2Parser.h"
#include "id3v2/id3v2Context.h"
#include "id3v2/id3v2TagIdentity.h"
#include "byteInt.h"

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
 * @brief Creates a new text frame with the given id, desired encoding, and string null terminated string. if successful,
 * the frame will be inserted into the tag and true will be returned otherwise, no tag will be created and false will be returned.
 * 
 * @param id 
 * @param encoding 
 * @param string 
 * @param tag 
 * @return int 
 */
int id3v2InsertTextFrame(const char id[ID3V2_FRAME_ID_MAX_SIZE], const uint8_t encoding, const char *string, Id3v2Tag *tag){

    if(id == NULL || string == NULL || tag == NULL || encoding > BYTE_UTF8){
        return false;
    }

    if(strlen(string) == 0){
        return false;
    }

    Id3v2FrameHeader *header = NULL;
    Id3v2Frame *f = NULL;
    Id3v2ContentEntry *entry = NULL;
    uint8_t *usableString = NULL;
    size_t outLen = 0;
    bool convi = false;

    // set up frame
    header = id3v2CreateFrameHeader((uint8_t *)id, 0, 0, 0, 0, 0, 0, 0);
    f = id3v2CreateFrame(header, id3v2CreateTextFrameContext(), listCreate(id3v2PrintContentEntry, id3v2DeleteContentEntry, id3v2CompareContentEntry, id3v2CopyContentEntry));
    
    // add encoding
    entry = id3v2CreateContentEntry((void *) &encoding, 1);
    listInsertBack(f->entries, (void *)entry);

    //add text
    convi = byteConvertTextFormat((unsigned char *)string, BYTE_UTF8, strlen(string), &usableString, encoding, &outLen);
    
    if(convi == false || outLen == 0 || usableString == NULL){
        id3v2DestroyFrame(&f);
        return false;
    }

    // already converted
    if(convi == true && outLen == 0){
        usableString = (uint8_t *) string;
        outLen = strlen(string);
    }

    // reenable utf16 len support
    bytePrependBOM(encoding, &usableString, &outLen);

    if(encoding == BYTE_UTF16BE || encoding == BYTE_UTF16LE){
        usableString = realloc(usableString, outLen + BYTE_PADDING);
        memset(usableString + outLen, 0, BYTE_PADDING);
    }

    // add encoded text
    entry = id3v2CreateContentEntry((void *)usableString, outLen);
    listInsertBack(f->entries, (void *)entry);
    free(usableString);

    listInsertBack(tag->frames, (void *)f);
    

    return true;
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
 * @brief Writes a null-terminated UTF8 string to a text frame with a given id.
 * The provided string will be converted to the appropriate encoding for the frame.
 * If no text frame is found one will be created with a UTF16LE encoding.
 * Any failure will result in this function returning false otherwise, true.
 * 
 * @param string 
 * @param tag 
 * @return int 
 */
int id3v2WriteTextFrameContent(const char id[ID3V2_FRAME_ID_MAX_SIZE], const char *string, Id3v2Tag *tag){

    if(id == NULL || string == NULL || tag == NULL){
        return false;

    }

    // null is an invalid string
    if(strlen(string) == 0){
        return false;

    }

    Id3v2Frame *f = NULL;
    ListIter frames = id3v2CreateFrameTraverser(tag);
    ListIter context = {0};
    ListIter entries = {0};
    Id3v2ContentContext *cc = NULL;
    uint8_t encoding = 0;
    uint8_t *usableString = NULL;
    size_t outLen = 0;
    int i = 0;
    bool convi = false;



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

    if(f == NULL){
        return id3v2InsertTextFrame(id, BYTE_UTF16LE, string, tag);
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

    encoding = id3v2ReadFrameEntryAsU8(&entries); // encoding

    convi = byteConvertTextFormat((unsigned char *)string, BYTE_UTF8, strlen(string), &usableString, encoding, &outLen);

    if(!convi && outLen == 0 && usableString == NULL){
        return false;
    }

    // data is already in utf8
    if(convi && outLen == 0){
        usableString = (uint8_t *) string;
        outLen = strlen(string);
    }

    bytePrependBOM(encoding, &usableString, &outLen);

    // reenable utf16 len support
    if(encoding == BYTE_UTF16BE || encoding == BYTE_UTF16LE){
        usableString = realloc(usableString, outLen + BYTE_PADDING);
        memset(usableString + outLen, 0, BYTE_PADDING);
    }

    if(id3v2WriteFrameEntry(f, &entries, byteStrlen(encoding, usableString), (void *) usableString)){
        free(usableString);
        return true;
    }
    
    if(usableString != NULL){
        free(usableString);
    }
    
    return false;
}

/**
 * @brief Writes a new title to the first Title/Song Name/Content Description (TT2 or TIT2) 
 * frame of a tag. If this function fails false is returned otherwise, true.
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
 * frame of a tag. If this function fails false is returned otherwise, true.
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
 * of a tag. If this function fails false is returned otherwise, true.
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

/**
 * @brief Writes a new album to the first Album/Movie/Show title (TAL or TALB) frame of a tag.
 * If this function fails false is returned otherwise, true.
 * 
 * @param album 
 * @param tag 
 * @return int 
 */
int id3v2WriteAlbum(const char *album, Id3v2Tag *tag){
    
    if(album == NULL || tag == NULL){
        return false;
    }

    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            return id3v2WriteTextFrameContent("TAL", album, tag);
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteTextFrameContent("TALB", album, tag);
        default:
            return false;
    }

    // dummy
    return false;
}

/**
 * @brief Writes a new year to the first Year (TYE or TYER) frame of a tag. If this function fails
 * false is returned otherwise, true.
 * 
 * @param year 
 * @param tag 
 * @return int 
 */
int id3v2WriteYear(const char *year, Id3v2Tag *tag){
        
    if(year == NULL || tag == NULL){
        return false;
    }

    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            return id3v2WriteTextFrameContent("TYE", year, tag);
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteTextFrameContent("TYER", year, tag);
        default:
            return false;
    }

    // dummy
    return false;
}

/**
 * @brief Writes a new genre to the first Genre (TCO or TCON) frame of a tag. If this function fails
 * false is returned otherwise, true.
 * 
 * @param genre 
 * @param tag 
 * @return int 
 */
int id3v2WriteGenre(const char *genre, Id3v2Tag *tag){

    if(genre == NULL || tag == NULL){
        return false;
    }

    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            return id3v2WriteTextFrameContent("TCO", genre, tag);
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteTextFrameContent("TCON", genre, tag);
        default:
            return false;
    }

    // dummy
    return false;
}

/**
 * @brief Writes a new track to the first Track number/Position in set (TRK or TRCK) frame of a tag.
 * if this function fails false is returned otherwise, true.
 * 
 * @param track 
 * @param tag 
 * @return int 
 */
int id3v2WriteTrack(const char *track, Id3v2Tag *tag){

    if(track == NULL || tag == NULL){
        return false;
    }

    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            return id3v2WriteTextFrameContent("TRK", track, tag);
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteTextFrameContent("TRCK", track, tag);
        default:
            return false;
    }

    // dummy
    return false;
}

/**
 * @brief Writes a new disc to the first Disc number (TPA or TPOS) frame of a tag. If this function fails
 * false is returned otherwise, true.
 * 
 * @param disc 
 * @param tag 
 * @return int 
 */
int id3v2WriteDisc(const char *disc, Id3v2Tag *tag){

    if(disc == NULL || tag == NULL){
        return false;
    }

    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            return id3v2WriteTextFrameContent("TPA", disc, tag);
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteTextFrameContent("TPOS", disc, tag);
        default:
            return false;
    }

    // dummy
    return false;
}

/**
 * @brief Writes a new composer to the first Composer (TCM or TCOM) frame of a tag.
 * if this function fails false is returned otherwise, true.
 * 
 * @param composer 
 * @param tag 
 * @return int 
 */
int id3v2WriteComposer(const char *composer, Id3v2Tag *tag){

    if(composer == NULL || tag == NULL){
        return false;
    }

    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            return id3v2WriteTextFrameContent("TCM", composer, tag);
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            return id3v2WriteTextFrameContent("TCOM", composer, tag);
        default:
            return false;
    }

    // dummy
    return false;
}

// internal function ------------------------------------------------------------------------
static int _id3v2CreateLyricFrameUTF16LE(const uint8_t v, const char *lyrics, Id3v2Tag *tag){

    // check for legal args
    if(lyrics == NULL){
        return false;
    }

    if(strlen(lyrics) == 0){
        return false;
    }

    Id3v2Frame *f = NULL;
    ListIter entires = {0};
    uint8_t encoding = BYTE_UTF16LE;
    uint8_t *usableString = NULL;
    size_t outLen = 0;
    bool convi = false;

    // create frame
    switch(v){
        case ID3V2_TAG_VERSION_2:
            f = id3v2CreateEmptyFrame("ULT\0", v, NULL);
            break;
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            f = id3v2CreateEmptyFrame("USLT", v, NULL);
            break;
        default:
            return false;
    }

    if(f == NULL){
        return false;
    }

    // update entries
    entires = id3v2CreateFrameEntryTraverser(f);

    // add encoding
    if(!id3v2WriteFrameEntry(f, &entires, 1, (void *) &encoding)){
        id3v2DestroyFrame(&f);
        return false;
    }
    id3v2ReadFrameEntryAsU8(&entires);

    // add language
    if(!id3v2WriteFrameEntry(f, &entires, 3, (void *) "zxx")){ // unknown language
        id3v2DestroyFrame(&f);
        return false;
    }
    id3v2ReadFrameEntryAsU8(&entires);

    // ensure description is nothing
    if(!id3v2WriteFrameEntry(f, &entires, 1, (void *) "\0")){
        id3v2DestroyFrame(&f);
        return false;
    }
    id3v2ReadFrameEntryAsU8(&entires);

    // add lyrics

    convi = byteConvertTextFormat((unsigned char *)lyrics, BYTE_UTF8, strlen(lyrics), &usableString, BYTE_UTF16LE, &outLen);

    if(convi == false || outLen == 0 || usableString == NULL){
        id3v2DestroyFrame(&f);
        return false;
    }

    // re add bom and padding
    bytePrependBOM(encoding, &usableString, &outLen);
    usableString = realloc(usableString, outLen + BYTE_PADDING);
    memset(usableString + outLen, 0, BYTE_PADDING);
    

    if(!id3v2WriteFrameEntry(f, &entires, outLen, (void *) usableString)){
        id3v2DestroyFrame(&f);
        free(usableString);
        return false;
    }
    id3v2ReadFrameEntryAsU8(&entires);


    listInsertBack(tag->frames, (void *) f);
    free(usableString);
    return true;
}
/**
 * @brief Writes new lyrics to the first Unsynchronised Lyric/Text Transcription (ULT or USLT) 
 * frame of a tag. If this function fails false is returned otherwise, true.
 * 
 * @param composer 
 * @param tag 
 * @return int 
 */
int id3v2WriteLyrics(const char *lyrics, Id3v2Tag *tag){

    if(lyrics == NULL || tag == NULL){
        return false;
    }

    // null is an invalid string
    if(strlen(lyrics) == 0){
        return false;
    }

    Id3v2Frame *f = NULL;
    ListIter frames = id3v2CreateFrameTraverser(tag);
    ListIter context = {0};
    ListIter entries = {0};
    Id3v2ContentContext *cc = NULL;
    uint8_t encoding = 0;
    uint8_t *usableString = NULL;
    size_t outLen = 0;
    char id[ID3V2_FRAME_ID_MAX_SIZE] = {0,0,0,0};
    int i = 0;
    bool convi = false;

    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            i = ID3V2_FRAME_ID_MAX_SIZE - 1;
            memcpy(id, "ULT", i);
            break;
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            i = ID3V2_FRAME_ID_MAX_SIZE;
            memcpy(id, "USLT", i);
            break;
        default:
            return false;

    }

    while((f = id3v2FrameTraverse(&frames)) != NULL){

        if(memcmp(id, f->header->id, i) == 0){
            break;
        }
    }

    if(f == NULL){
        return _id3v2CreateLyricFrameUTF16LE(tag->header->majorVersion, lyrics, tag);
    }

    // verify frame via context
    if(f->contexts->length != 4 && f->entries->length != 4){
        return false;
    }

    context = listCreateIterator(f->contexts);

    for(int i = 0; i < 4; i++){
        cc = listIteratorNext(&context);

        if(cc == NULL || (i == 0 && cc->type != numeric_context) ||
            (i == 1 && cc->type != noEncoding_context) ||
            (i == 2 && cc->type != encodedString_context) ||
            (i == 3 && cc->type != encodedString_context)){
            return false;
        }
    }

    entries = id3v2CreateFrameEntryTraverser(f);

    encoding = id3v2ReadFrameEntryAsU8(&entries); // encoding

    convi = byteConvertTextFormat((unsigned char *)lyrics, BYTE_UTF8, strlen(lyrics), &usableString, encoding, &outLen);

    if(!convi && outLen == 0 && usableString == NULL){
        return false;
    }

    // data is already in utf8
    if(convi && outLen == 0){
        usableString = (uint8_t *) lyrics;
    }

    bytePrependBOM(encoding, &usableString, &outLen);

    // reenable utf16 len support
    if(encoding == BYTE_UTF16BE || encoding == BYTE_UTF16LE){
        usableString = realloc(usableString, outLen + BYTE_PADDING);
        memset(usableString + outLen, 0, BYTE_PADDING);
    }

    id3v2ReadFrameEntryAsU8(&entries); // language
    id3v2ReadFrameEntryAsU8(&entries); // description

    if(id3v2WriteFrameEntry(f, &entries, byteStrlen(encoding, usableString), (void *) usableString)){
        free(usableString);
        return true;
    }
    
    if(usableString != NULL){
        free(usableString);
    }
    
    return false;
}


// internal function -----------------------------------------------------------------------------------------------------------
static int _id3v2CreateCommentFrameUTF16LE(uint8_t v, const char lang[3], const char *desc, const char *comment, Id3v2Tag *tag){

    // check for legal args
    if(lang == NULL || desc == NULL || comment == NULL){
        return false;
    }

    if(strlen(lang) != 3 || strlen(comment) == 0){
        return false;
    }

    Id3v2Frame *f = NULL;
    Id3v2FrameHeader *header = NULL;
    Id3v2ContentEntry *ce = NULL;
    uint8_t *usableString = NULL;
    uint8_t encoding = BYTE_UTF16LE;
    size_t outLen = 0;
    bool convi = false;

    // get frame header
    switch(v){
        case ID3V2_TAG_VERSION_2:
            header = id3v2CreateFrameHeader((uint8_t *)"COM\0", 0,0,0,0,0,0,0);
            break;
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            header = id3v2CreateFrameHeader((uint8_t *)"COMM", 0,0,0,0,0,0,0);
            break;
        default:
            return false;
    }

    // create frame
    f = id3v2CreateFrame(header, id3v2CreateCommentFrameContext(), listCreate(id3v2PrintContentEntry, id3v2DeleteContentEntry, id3v2CompareContentEntry, id3v2CopyContentEntry));

    ce = id3v2CreateContentEntry((void *) &encoding, 1);
    listInsertBack(f->entries, (void *) ce);

    ce = id3v2CreateContentEntry((void *) lang, 3);
    listInsertBack(f->entries, (void *) ce);

    // make a description
    if(strlen(desc) != 0){
        convi = byteConvertTextFormat((unsigned char *) desc, BYTE_UTF8, strlen(desc), &usableString, encoding, &outLen);

        if(!convi && outLen == 0 && usableString == NULL){
            id3v2DestroyFrame(&f);
            return false;
        }

        bytePrependBOM(encoding, &usableString, &outLen);

        // reenable utf16 len support
        if(encoding == BYTE_UTF16BE || encoding == BYTE_UTF16LE){
            usableString = realloc(usableString, outLen + BYTE_PADDING);
            memset(usableString + outLen, 0, BYTE_PADDING);
        }

        ce = id3v2CreateContentEntry((void *) usableString, outLen);
        listInsertBack(f->entries, (void *) ce);

        free(usableString);
        outLen = 0;
    }else{
        ce = id3v2CreateContentEntry("\0", 1);
        listInsertBack(f->entries, (void *) ce);
    }
    
    convi = byteConvertTextFormat((unsigned char *) comment, BYTE_UTF8, strlen(comment), &usableString, encoding, &outLen);

    if(!convi && outLen == 0 && usableString == NULL){
        id3v2DestroyFrame(&f);
        return false;
    }
    
    bytePrependBOM(encoding, &usableString, &outLen);

    // reenable utf16 len support
    if(encoding == BYTE_UTF16BE || encoding == BYTE_UTF16LE){
        usableString = realloc(usableString, outLen + BYTE_PADDING);
        memset(usableString + outLen, 0, BYTE_PADDING);
    }

    // add comment
    ce = id3v2CreateContentEntry((void *) usableString, outLen);
    listInsertBack(f->entries, (void *) ce);
    free(usableString);

    listInsertBack(tag->frames, (void *) f);
    return true;
}
/**
 * @brief Writes a new comment to the first Comment (COM or COMM) frame of a tag. If no comment frame exits
 * one will be created with a UTF16LE encoding. If this function fails false is returned otherwise, true.
 * 
 * @param comment 
 * @param tag 
 * @return int 
 */
int id3v2WriteComment(const char *comment, Id3v2Tag *tag){

    if(comment == NULL || tag == NULL){
        return false;
    }

    // null is an invalid string
    if(strlen(comment) == 0){
        return false;
    }

    Id3v2Frame *f = NULL;
    ListIter frames = id3v2CreateFrameTraverser(tag);
    ListIter context = {0};
    ListIter entries = {0};
    Id3v2ContentContext *cc = NULL;
    uint8_t encoding = 0;
    uint8_t *usableString = NULL;
    size_t outLen = 0;
    char id[ID3V2_FRAME_ID_MAX_SIZE] = {0,0,0,0};
    int i = 0;
    bool convi = false;


    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            i = ID3V2_FRAME_ID_MAX_SIZE - 1;
            memcpy(id, "COM", i);
            break;
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            i = ID3V2_FRAME_ID_MAX_SIZE;
            memcpy(id, "COMM", i);
            break;
        default:
            return false;
    }

    while((f = id3v2FrameTraverse(&frames)) != NULL){
        if(memcmp(id, f->header->id, i) == 0){
            break;
        }
    }

    if(f == NULL){
        return _id3v2CreateCommentFrameUTF16LE(tag->header->majorVersion, "zxx", "", comment, tag); // zxx is no/unknown language
    }

    // verify frame via context
    if(f->contexts->length != 4 && f->entries->length != 4){
        return false;
    }

    context = listCreateIterator(f->contexts);

    for(int i = 0; i < 4; i++){
        cc = listIteratorNext(&context);

        if(cc == NULL || (i == 0 && cc->type != numeric_context) ||
            (i == 1 && cc->type != noEncoding_context) ||
            (i == 2 && cc->type != encodedString_context) ||
            (i == 3 && cc->type != encodedString_context)){
            return false;
        }
    }

    entries = id3v2CreateFrameEntryTraverser(f);

    encoding = id3v2ReadFrameEntryAsU8(&entries); // encoding

    convi = byteConvertTextFormat((unsigned char *)comment, BYTE_UTF8, strlen(comment), &usableString, encoding, &outLen);

    if(!convi && outLen == 0 && usableString == NULL){
        return false;
    }

    // data is already in utf8
    if(convi && outLen == 0){
        usableString = (uint8_t *) comment;
        outLen = strlen(comment);
    }

    bytePrependBOM(encoding, &usableString, &outLen);

    // reenable utf16 len support
    if(encoding == BYTE_UTF16BE || encoding == BYTE_UTF16LE){
        usableString = realloc(usableString, outLen + BYTE_PADDING);
        memset(usableString + outLen, 0, BYTE_PADDING);
    }

    id3v2ReadFrameEntryAsU8(&entries); // language
    id3v2ReadFrameEntryAsU8(&entries); // description

    if(id3v2WriteFrameEntry(f, &entries, byteStrlen(encoding, usableString), (void *) usableString)){
        free(usableString);
        return true;
    }
    
    if(usableString != NULL){
        free(usableString);
    }
    
    return false;
}

// internal function ---------------------------------------------------------------------------------------------------------
static int _id3v2CreatePictureFrameUTF16LEtype0(uint8_t v, uint8_t *image, size_t imageSize, const char *kind, Id3v2Tag *tag){

    if(image == NULL || imageSize == 0 || kind == NULL || tag == NULL){
        return false;
    }

    if(strlen(kind) == 0){
        return false;
    }

    Id3v2Frame *f = NULL;
    ListIter entries = {0};
    uint8_t encoding = BYTE_UTF16LE;
    size_t kindLen = 0;

    // create frame
    switch(v){
        case ID3V2_TAG_VERSION_2:
            f = id3v2CreateEmptyFrame("PIC\0", v, NULL);
            break;
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            f = id3v2CreateEmptyFrame("APIC", v, NULL);
            break;
        default:
            return false;
    }

    if(f == NULL){
        return false;
    }

    // // update entries
    entries = id3v2CreateFrameEntryTraverser(f);

    // add encoding
    if(!id3v2WriteFrameEntry(f, &entries, 1, (void *) &encoding)){
        id3v2DestroyFrame(&f);
        return false;
    }
    id3v2ReadFrameEntryAsU8(&entries);

    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            kindLen = strlen(kind);

            if(kindLen > 3){
                kindLen = 3;
            }

            if(!id3v2WriteFrameEntry(f, &entries, kindLen, (void *) kind)){
                return false;
            }

            break;
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:{

            char *mime = NULL;

            mime = calloc(sizeof(char), strlen("image/") + strlen(kind) + 1);

            memcpy(mime, "image/", strlen("image/"));
            memcpy(mime + strlen("image/"), kind, strlen(kind));

            if(!id3v2WriteFrameEntry(f, &entries, strlen(mime), (void *) mime)){
                free(mime);
                return false;
            }

            free(mime);
        }
            break;
        default:
            id3v2DestroyFrame(&f);
            return false;

    }
    id3v2ReadFrameEntryAsU8(&entries); // mime type skip

    // add type
    if(!id3v2WriteFrameEntry(f, &entries, 1, (void *) "\0")){
        id3v2DestroyFrame(&f);
        return false;
    }
    id3v2ReadFrameEntryAsU8(&entries);

    // ensure description is nothing
    if(!id3v2WriteFrameEntry(f, &entries, 1, (void *) "\0")){
        id3v2DestroyFrame(&f);
        return false;
    }
    id3v2ReadFrameEntryAsU8(&entries);

    // write image
    if(!id3v2WriteFrameEntry(f, &entries, imageSize, (void *) image)){
        id3v2DestroyFrame(&f);
        return false;
    }
    id3v2ReadFrameEntryAsU8(&entries);


    listInsertBack(tag->frames, (void *) f);
    return true;

}
/**
 * @brief Writes a new picture to the first Attached picture (PIC or APIC) frame of a tag that matches the
 * given type. It will additionally write the mime type via the null terminated string kind. If this function fails
 * or any argument is null, false is returned otherwise, true.
 * 
 * @param image 
 * @param imageSize 
 * @param kind 
 * @param type 
 * @param tag 
 * @return int 
 */
int id3v2WritePicture(uint8_t *image, size_t imageSize, const char *kind, uint8_t type, Id3v2Tag *tag){

    if(image == NULL || imageSize == 0 || kind == NULL || tag == NULL){
        return false;
    }

    if(strlen(kind) == 0){
        return false;
    }

    Id3v2Frame *f = NULL;
    ListIter frames = id3v2CreateFrameTraverser(tag);
    ListIter context = {0};
    ListIter entries = {0};
    Id3v2ContentContext *cc = NULL;
    uint8_t usableType = ((type > 0x14) ? 0x00 : type); // clamp type
    char id[ID3V2_FRAME_ID_MAX_SIZE] = {0,0,0,0};
    int i = 0;
    int kindLen = 0;

    // what id will be used
    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            i = ID3V2_FRAME_ID_MAX_SIZE - 1;
            memcpy(id, "PIC", i);
            break;
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            i = ID3V2_FRAME_ID_MAX_SIZE;
            memcpy(id, "APIC", i);
            break;
        default:
            return false;
    }

    // find frame
    while(true){
        while((f = id3v2FrameTraverse(&frames)) != NULL){
            if(memcmp(id, f->header->id, i) == 0){
                break;
            }
        }

        if(f == NULL){
            return _id3v2CreatePictureFrameUTF16LEtype0(tag->header->majorVersion, image, imageSize, kind, tag);
        }

        // verify frame via context
        if(f->contexts->length != 5 && f->entries->length != 5){
            return false;
        }

        context = listCreateIterator(f->contexts);

        for(int i = 0; i < 4; i++){
            cc = listIteratorNext(&context);

            if(cc == NULL){
                return false;
            }

            if((i == 0 && cc->type != numeric_context) || 
            (i == 1 && (cc->type != noEncoding_context && cc->type != latin1Encoding_context)) ||
            (i == 2 && cc->type != numeric_context) ||
            (i == 3 && cc->type != encodedString_context) || 
            (i == 4 && cc->type != binary_context)){
                return false;

            }
        }

        entries = id3v2CreateFrameEntryTraverser(f);

        id3v2ReadFrameEntryAsU8(&entries); // encoding
        id3v2ReadFrameEntryAsU8(&entries); // mime type
        if(id3v2ReadFrameEntryAsU8(&entries) == usableType){
            break;
        }

    }

    entries = id3v2CreateFrameEntryTraverser(f);

    id3v2ReadFrameEntryAsU8(&entries); // encoding
    
    switch(tag->header->majorVersion){
        case ID3V2_TAG_VERSION_2:
            kindLen = strlen(kind);

            if(kindLen > 3){
                kindLen = 3;
            }

            if(!id3v2WriteFrameEntry(f, &entries, kindLen, (void *) kind)){
                return false;
            }

            break;
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:{

            char *mime = NULL;

            mime = calloc(sizeof(char), strlen("image/") + strlen(kind) + 1);

            memcpy(mime, "image/", strlen("image/"));
            memcpy(mime + strlen("image/"), kind, strlen(kind));

            if(!id3v2WriteFrameEntry(f, &entries, strlen(mime), (void *) mime)){
                free(mime);
                return false;
            }

            free(mime);
        }
            break;
        default:
            return false;

    }

    id3v2ReadFrameEntryAsU8(&entries); // mime type skip
    id3v2ReadFrameEntryAsU8(&entries); // picture type
    id3v2ReadFrameEntryAsU8(&entries); // description

    return id3v2WriteFrameEntry(f, &entries, imageSize, (void *) image);
}

/**
 * @brief Writes a new picture from a file to the first Attached picture (PIC or APIC) frame of a tag that 
 * matches the given type. It will additionally write the mime type via the null terminated string kind. 
 * If this function fails or any argument is null, false is returned otherwise, true.
 * 
 * @param filename 
 * @param kind 
 * @param type 
 * @param tag 
 * @return int 
 */
int id3v2WritePictureFromFile(const char *filename, const char *kind, uint8_t type, Id3v2Tag *tag){
    
    if(filename == NULL || kind == NULL || tag == NULL){
        return false;
    }

    if(strlen(filename) == 0 || strlen(kind) == 0){
        return false;
    }

    FILE *f = NULL;
    uint8_t *data = NULL;
    size_t size = 0;
    int ret = 0;

    f = fopen(filename, "rb");

    if(f == NULL){
        return false;
    }

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);

    data = calloc(sizeof(uint8_t), size);

    if(fread(data, sizeof(uint8_t), size, f) != size){
        fclose(f);
        free(data);
        return false;
    }

    fclose(f);

    ret = id3v2WritePicture(data, size, kind, type, tag);

    free(data);

    return ret;
}

/**
 * @brief Converts an ID3v2.x tag data structure back to its binary representation. If this function fails
 * NULL is returned otherwise, a ByteStream pointer.
 * 
 * @param tag 
 * @return ByteStream* 
 */
ByteStream *id3v2TagToStream(Id3v2Tag *tag){

    if(tag == NULL){
        return NULL;
    }

    if(tag->header == NULL || tag->frames == NULL){
        return NULL;
    }

    ByteStream *stream = NULL;
    ByteStream *headerStream = NULL;
    ByteStream *frameStream = NULL;
    ByteStream *footerStream = NULL;
    Id3v2Frame *f = NULL;
    ListIter frames = id3v2CreateFrameTraverser(tag);
    uint32_t fsize = 0;
    uint32_t padding = 0;
    uint8_t *sizeBytes = NULL;

    // frame stream
    while((f = id3v2FrameTraverse(&frames)) != NULL){
        ByteStream *tmp = NULL;

        tmp = id3v2FrameToStream(f, tag->header->majorVersion);

        if(tmp == NULL){
            break;
        }

        if(frameStream == NULL){
            frameStream = byteStreamCreate(byteStreamCursor(tmp), tmp->bufferSize);
            byteStreamSeek(frameStream, 0, SEEK_END);
        }else{
            byteStreamResize(frameStream, frameStream->bufferSize + tmp->bufferSize);
            byteStreamWrite(frameStream, tmp->buffer, tmp->bufferSize);
            
        }
        byteStreamDestroy(tmp);
    }

    byteStreamRewind(frameStream);

    // unsync?
    if(id3v2ReadUnsynchronisationIndicator(tag->header)){
        ByteStream *tmp = NULL;

        tmp = byteStreamCreate(NULL, frameStream->bufferSize * 2);

        for(size_t i = 0; i < frameStream->bufferSize; i++){
            
            byteStreamWrite(tmp, byteStreamCursor(frameStream), 1);
            
            byteStreamSeek(tmp, 1, SEEK_CUR);
            byteStreamSeek(frameStream, 1, SEEK_CUR);

        }

        byteStreamDestroy(frameStream);
        frameStream = tmp;
        fsize = frameStream->bufferSize;
        byteStreamRewind(frameStream);

    }else{
        fsize = frameStream->bufferSize;
    }

    // header stream
    headerStream = id3v2TagHeaderToStream(tag->header, 0);

    // unsync?
    if(id3v2ReadUnsynchronisationIndicator(tag->header) && headerStream->bufferSize > 10){
        ByteStream *tmp = NULL;
        uint32_t size = 0;

        // ext size
        size = (headerStream->bufferSize - 10) * 2;
        
        tmp = byteStreamCreate(NULL, 10 + size);

        // write header back
        byteStreamWrite(tmp, byteStreamCursor(headerStream), 10);
        byteStreamSeek(headerStream, 10, SEEK_SET);

        for(uint32_t i = 0; i < size; i++){
            
            byteStreamWrite(tmp, byteStreamCursor(headerStream), 1);
            
            byteStreamSeek(tmp, 1, SEEK_CUR);
            byteStreamSeek(headerStream, 1, SEEK_CUR);

        }

        byteStreamDestroy(headerStream);
        headerStream = tmp;
        fsize += size;

    }else{
        fsize += ((headerStream->bufferSize > 10 ) ? headerStream->bufferSize - 10 : 0);
    }

    byteStreamRewind(headerStream);

    // footer stream?
    if(id3v2ReadFooterIndicator(tag->header) && tag->header->majorVersion == ID3V2_TAG_VERSION_4){

        footerStream = byteStreamCreate(byteStreamCursor(headerStream), 10);
        byteStreamWrite(footerStream, (unsigned char *) "3DI", ID3V2_TAG_ID_SIZE);
        byteStreamRewind(footerStream);
    }

    if(tag->header->extendedHeader != NULL){
        fsize += tag->header->extendedHeader->padding;
        padding = tag->header->extendedHeader->padding;
    }

    // insert size
    sizeBytes = u32tob(byteSyncintEncode(fsize));
    byteStreamSeek(headerStream, 6, SEEK_SET);
    byteStreamWrite(headerStream, sizeBytes, 4);
    byteStreamRewind(headerStream);

    if(footerStream != NULL){
        byteStreamSeek(footerStream, 6, SEEK_SET);
        byteStreamWrite(footerStream, sizeBytes, 4);
        byteStreamRewind(footerStream);
    }

    free(sizeBytes);

    // create stream
    stream = byteStreamCreate(NULL, frameStream->bufferSize + headerStream->bufferSize + ((footerStream != NULL) ? footerStream->bufferSize : 0));
    byteStreamWrite(stream, byteStreamCursor(headerStream), headerStream->bufferSize);
    byteStreamWrite(stream, byteStreamCursor(frameStream), frameStream->bufferSize);
    byteStreamSeek(stream, padding, SEEK_CUR);
    if(footerStream != NULL){
        byteStreamWrite(stream, byteStreamCursor(footerStream), footerStream->bufferSize);
        byteStreamDestroy(footerStream);
    }

    byteStreamDestroy(headerStream);
    byteStreamDestroy(frameStream);
    byteStreamRewind(stream);
    return stream;
}

/**
 * @brief Converts an ID3v2.x tag data structure back to its json representation.
 * 
 * @param tag 
 * @return char* 
 */
char *id3v2TagToJSON(Id3v2Tag *tag){

    char *json = NULL;
    char *headerJson = NULL;
    char **contentJson = NULL;
    char *concatenatedString = NULL;
    size_t contentJsonSize = 0;
    size_t memCount = 3;
    size_t concatenatedStringLength = 0;

    if(tag == NULL){
        json = calloc(memCount, sizeof(char));
        memcpy(json, "{}\0", memCount);
        return json;
    }

    if(tag->frames == NULL || tag->header == NULL){
        json = calloc(memCount, sizeof(char));
        memcpy(json, "{}\0", memCount);
        return json;
    }

    if(tag->header->majorVersion > ID3V2_TAG_VERSION_4){
        json = calloc(memCount, sizeof(char));
        memcpy(json, "{}\0", memCount);
        return json;
    }

    // get frames

    ListIter frames = id3v2CreateFrameTraverser(tag);
    Id3v2Frame *f = NULL;

    while((f = id3v2FrameTraverse(&frames)) != NULL){
        char *tmp = NULL;
        size_t jsonSize = 0;

        tmp = id3v2FrameToJSON(f, tag->header->majorVersion);
        jsonSize = strlen(tmp);

        contentJsonSize++;
        if(contentJson == NULL){
            contentJson = calloc(contentJsonSize, sizeof(char *));
            contentJson[contentJsonSize - 1] = calloc(jsonSize + 1, sizeof(char));
                
        }else{
            contentJson = realloc(contentJson, (contentJsonSize) * sizeof(char *));
            contentJson[contentJsonSize - 1] = calloc(jsonSize + 1, sizeof(char)); 
        }

        memcpy(contentJson[contentJsonSize - 1], tmp, jsonSize);

        free(tmp);

    }

    // get header
    headerJson = id3v2TagHeaderToJSON(tag->header);


    // concatenate all JSON data stored in contentJson into a single string split by ","
    for(size_t i = 0; i < contentJsonSize; i++){
        concatenatedStringLength += strlen(contentJson[i]) + 1;
    }
    

    concatenatedString = calloc(concatenatedStringLength + 1, sizeof(char));
    for(size_t i = 0; i < contentJsonSize; i++){
        strcat(concatenatedString, contentJson[i]);
        if(i < contentJsonSize - 1){
            strcat(concatenatedString, ",");
        }
    }

    memCount += snprintf(NULL, 0,
                        "{\"header\":%s,\"content\":[%s]}",
                        headerJson,
                        concatenatedString);


    json = malloc((memCount + 1) * sizeof(char));
    strcpy(json, "{\"header\":");
    strcat(json, headerJson);
    strcat(json, ",\"content\":[");
    strcat(json, concatenatedString);
    strcat(json, "]}");


    free(headerJson);

    if(concatenatedString != NULL){
        free(concatenatedString);
    }

    if(contentJson != NULL){
        for(size_t i = 0; i < contentJsonSize; i++){
            free(contentJson[i]);

        }
        free(contentJson);
    }

    return json;

}

/**
 * @brief Writes an ID3v2.x tag data structure to a file. If this function fails false is returned otherwise, true.
 * If the file does not exist it will be created if it does and there is no tag it will be prepended. If the update
 * flag is set in the tag header it will be prepended to the file without overwriting any existing tags. 
 * 
 * @param filename 
 * @param tag 
 * @return int 
 */
int id3v2WriteTagToFile(const char *filePath, Id3v2Tag *tag){

    if(filePath == NULL || tag == NULL){
        return false;
    }

    FILE *fp = NULL;
    ByteStream *stream = NULL;

    stream = id3v2TagToStream(tag);
    fp = fopen(filePath, "r+b");
    
    // write to a new file
    if(fp == NULL){

        // create a new file and write the bytes to it    
        fp = fopen(filePath, "wb");

        if(fp == NULL){
            byteStreamDestroy(stream);
            return false;
        }

        if((fwrite(byteStreamCursor(stream), 1, stream->bufferSize, fp)) == 0){
            byteStreamDestroy(stream);
            fclose(fp);
            return false;
        }

    // update file
    }else{

        char id[ID3V2_TAG_ID_SIZE] = {0};
        bool hasTag = false;
        long fileSize = 0;
        bool prepend = 0;
        uint8_t *tmp = NULL;
        uint8_t *upperTmp = NULL;
        size_t upperBytes = 0;
        uint32_t oldTagSize = 0;

        // does the tag exist?
        while(hasTag == false && feof(fp) == 0){
            
            upperBytes += fread(id, sizeof(char), ID3V2_TAG_ID_SIZE, fp);
            hasTag = (memcmp(id, "ID3", ID3V2_TAG_ID_SIZE) != 0) ? false : true;
            
        }

        // 1. update flag is set
        if(hasTag == true && tag->header->extendedHeader != NULL){

            prepend = false;
            
            if(tag->header->extendedHeader->update == true){
                prepend = true;
            }
        
        // 2. a tag exists
        }else if(hasTag == true){
            prepend = false;

            // get the tag size
            fseek(fp, 3, SEEK_CUR); // skips version and flag
            tmp = malloc(4);
            fread(tmp, 1, 4, fp);
            oldTagSize = byteSyncintDecode(btou32(tmp, 4));
            free(tmp);

        // 3. no tag exists
        }else{
            prepend = true;
        }

        // correct for ID3 tag
        upperBytes = (upperBytes > ID3V2_TAG_ID_SIZE) ? upperBytes - ID3V2_TAG_ID_SIZE : 0;

        // get file size
        fseek(fp, 0, SEEK_END);
        fileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        // read the file
        tmp = malloc(fileSize);
        if(fread(tmp, 1, fileSize, fp) != fileSize){
            free(tmp);
            fclose(fp);
            byteStreamDestroy(stream);
            return 0;
        }

        fseek(fp, 0, SEEK_SET);

        if(prepend){

            // write the stream to a file
            if(fwrite(byteStreamCursor(stream), 1, stream->bufferSize, fp) != stream->bufferSize){
                free(tmp);
                fclose(fp);
                byteStreamDestroy(stream);
                return 0;
            }

            // write the existing file data back to the file
            if(fwrite(tmp, 1, fileSize, fp) != fileSize){
                free(tmp);
                fclose(fp);
                byteStreamDestroy(stream);
                return 0;
            }

            free(tmp);
        }else{
            
            uint32_t offset = 10;

            if(id3v2ReadFooterIndicator(tag->header) && tag->header->majorVersion == ID3V2_TAG_VERSION_4){
                offset += 10;
            }

            if(tag->header->extendedHeader != NULL){
                if(tag->header->extendedHeader->padding > 0){
                    offset += tag->header->extendedHeader->padding;
                }
            }

            // prepend data above the tag
            if(upperBytes > 0){
                
                upperTmp = calloc(sizeof(char), upperBytes);
                
                if(fread(upperTmp, sizeof(char), upperBytes, fp) != upperBytes){
                    free(upperTmp);
                    free(tmp);
                    fclose(fp);
                    byteStreamDestroy(stream);
                    return false;
                }
                free(upperTmp);

                fseek(fp, 0, SEEK_SET);
            }

            // write the stream to a file
            if(fwrite(byteStreamCursor(stream), 1, stream->bufferSize, fp) != stream->bufferSize){
                free(tmp);
                fclose(fp);
                byteStreamDestroy(stream);
                return 0;
            }

            // no need to read the old tag
            fileSize = fileSize - (oldTagSize + offset) - upperBytes;

            if(fwrite(tmp + upperBytes + oldTagSize + offset, 1, fileSize, fp)){
                free(tmp);
                fclose(fp);
                byteStreamDestroy(stream);
                return 0;
            }

            free(tmp);
        }
    }

    fclose(fp);
    byteStreamDestroy(stream);
    return true;
}
