/**
 * @file buildTag.c
 * @author Ewan Jones
 * @brief Builds an ID3 tag programmatically and prints it
 * @version 0.1
 * @date 2024-04-22
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#include <stdio.h> // printf
#include <stdlib.h> // EXIT_SUCCESS
#include <string.h> // strlen
#include <id3v1/id3v1Parser.h> // id3v1CreateTag
#include <id3v2/id3v2.h> // id3v2CreateTagHeader, id3v2CreateTag, id3v2WriteTitle, id3v2WriteArtist, id3v2WriteAlbum, id3v2WriteYear, id3v2WriteTrack, id3v2WriteGenre
#include <id3v2/id3v2Frame.h> // id3v2CreateFrame, id3v2CreateFrameHeader, id3v2AttachFrameToTag, id3v2CreateFrameEntryTraverser, id3v2ReadFrameEntryAsU8, id3v2ReadFrameEntryAsChar
#include <id3v2/id3v2Context.h> // id3v2CreateUserDefinedTextFrameContext
#include <id3dev.h> // id3Create, id3SetPreferedStandard, id3ReadTitle, id3ReadArtist, id3ReadAlbum, id3ReadYear, id3ReadGenre, id3ReadTrack, id3Destroy

int main(void){

    // variables for ID3v1
    Id3v1Tag *v1tag = NULL;

    // variables for ID3v2
    Id3v2TagHeader *v2header = NULL;
    List *v2frames = NULL;
    Id3v2Tag *v2tag = NULL;
    Id3v2Frame *frame = NULL;
    Id3v2FrameHeader *frameHeader = NULL;
    Id3v2ContentEntry *entry = NULL;
    List *context = NULL;
    List *entries = NULL;

    // variables for ID3
    ID3 *id3 = NULL;
    ListIter frames;
    ListIter frameEntries;
    size_t size = 0;
    char *str = NULL;


    // Create an Id3v1 tag
    v1tag = id3v1CreateTag((uint8_t *)"Happier Than Ever", // title
                           (uint8_t *)"Billie Eilish",     // artist
                           (uint8_t *)"Happier Than Ever", // album
                           2021,                           // year
                           15,                             // track
                           NULL,                           // comment
                           POP_GENRE);                     // genre

    
    // Create an ID3v2.3 tag header
    v2header = id3v2CreateTagHeader(ID3V2_TAG_VERSION_3, 0, 0, NULL);

    // Create a list to hold future ID3v2 frames
    v2frames = listCreate(id3v2PrintFrame, id3v2DeleteFrame, id3v2CompareFrame, id3v2CopyFrame);
    
    // Join header and frames to create an ID3v2 tag
    v2tag = id3v2CreateTag(v2header, v2frames);

    // Create frames and attach them to ID3v2 tag
    id3v2WriteTitle("Happier Than Ever", v2tag);
    id3v2WriteArtist("Billie Eilish", v2tag);
    id3v2WriteAlbum("Happier Than Ever", v2tag);
    id3v2WriteYear("2021", v2tag);
    id3v2WriteTrack("15", v2tag);
    id3v2WriteGenre("Pop", v2tag);

    // Create a TXXX frame and attach it to ID3v2 tag

    // 1. Create context and entries lists
    context = id3v2CreateUserDefinedTextFrameContext();
    entries = listCreate(id3v2PrintContentEntry, id3v2DeleteContentEntry, id3v2CompareContentEntry, id3v2CopyContentEntry);

    // 2. Create entries
    entry = id3v2CreateContentEntry((void *)"\0",1);
    listInsertBack(entries, entry);

    entry = id3v2CreateContentEntry((void *)"producer", strlen("producer"));
    listInsertBack(entries, entry);

    entry = id3v2CreateContentEntry((void *)"Finneas O'Connell", strlen("Finneas O'Connell"));
    listInsertBack(entries, entry);

    // 3. Create a frame header
    frameHeader = id3v2CreateFrameHeader((uint8_t *)"TXXX", 0, 0, 0, 0, 0, 0, 0);

    // 4. Create frame and insert it into the ID3v2 tag
    frame = id3v2CreateFrame(frameHeader, context, entries);
    id3v2AttachFrameToTag(v2tag, frame);

    id3 = id3Create(v2tag, v1tag);


    // set standard to force reading from the ID3v1 tag
    id3SetPreferedStandard(ID3V1_TAG_VERSION);

    // print the ID3v1 tag
    printf("ID3v1 tag:\n");

    str = id3ReadTitle(id3);

    if(str != NULL){
        printf("\t|Title: %s\n", str);
        free(str);
    }

    str = id3ReadArtist(id3);

    if(str != NULL){
        printf("\t|Artist: %s\n", str);
        free(str);
    }

    str = id3ReadAlbum(id3);

    if(str != NULL){
        printf("\t|Album: %s\n", str);
        free(str);
    }

    str = id3ReadYear(id3);

    if(str != NULL){
        printf("\t|Year: %s\n", str);
        free(str);
    }

    str = id3ReadTrack(id3);

    if(str != NULL){
        printf("\t|Track: %s\n", str);
        free(str);
    }

    str = id3ReadGenre(id3);

    if(str != NULL){
        printf("\t|Genre: %s\n", str);
        free(str);
    }

    // set standard to force reading from the ID3v2.3 tag
    id3SetPreferedStandard(ID3V2_TAG_VERSION_3);

    // print the ID3v1 tag
    printf("ID3v2 tag:\n");

    str = id3ReadTitle(id3);

    if(str != NULL){
        printf("\t|Title: %s\n", str);
        free(str);
    }

    str = id3ReadArtist(id3);

    if(str != NULL){
        printf("\t|Artist: %s\n", str);
        free(str);
    }

    str = id3ReadAlbum(id3);

    if(str != NULL){
        printf("\t|Album: %s\n", str);
        free(str);
    }

    str = id3ReadYear(id3);

    if(str != NULL){
        printf("\t|Year: %s\n", str);
        free(str);
    }

    str = id3ReadTrack(id3);

    if(str != NULL){
        printf("\t|Track: %s\n", str);
        free(str);
    }

    str = id3ReadGenre(id3);

    if(str != NULL){
        printf("\t|Genre: %s\n", str);
        free(str);
    }

    frames = id3v2CreateFrameTraverser(id3->id3v2);

    while((frame = id3v2FrameTraverse(&frames)) != NULL){
        if(memcmp(frame->header->id, "TXXX", ID3V2_FRAME_ID_MAX_SIZE) == 0){
            printf("\t|TXXX frame:\n");
            frameEntries = id3v2CreateFrameEntryTraverser(frame);
            
            printf("\t\t|encoding: %d\n", id3v2ReadFrameEntryAsU8(&frameEntries));

            str = id3v2ReadFrameEntryAsChar(&frameEntries, &size);
            printf("\t\t|description: %s\n", str);
            free(str);

            str = id3v2ReadFrameEntryAsChar(&frameEntries, &size);
            printf("\t\t|text: %s\n", str);
            free(str);

        }
    }


    id3Destroy(&id3);

    return EXIT_SUCCESS;
}