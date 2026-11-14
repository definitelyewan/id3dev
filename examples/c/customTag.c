/**
 * @file customTag.c
 * @author Ewan Jones
 * @brief Creates a custom XER1 frame with its own layout, fills it and prints it
 * @version 2026.11
 * @date 2026-11-13
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <stdio.h> // printf
#include <stdlib.h> // EXIT_SUCCESS, EXIT_FAILURE, free
#include <string.h> // strlen, memcmp
#include <id3v2/id3v2.h> // id3v2CreateTagHeader, id3v2CreateTag
#include <id3v2/id3v2Frame.h> // id3v2CreateEmptyFrame, id3v2WriteFrameEntry, id3v2AttachFrameToTag, id3v2CreateFrameTraverser, id3v2FrameTraverse, id3v2CreateFrameEntryTraverser, id3v2ReadFrameEntryAsU8, id3v2ReadFrameEntryAsChar
#include <id3v2/id3v2Context.h> // id3v2CreateContentContext, id3v2djb2, id3v2InsertIdentifierContextPair
#include <id3dev.h> // id3Create, id3Destroy


int main(void) {
    // variables for the custom frame layout
    List *context = NULL;
    HashTable *pairs = NULL;

    // variables for ID3v2
    Id3v2Tag *v2tag = NULL;
    Id3v2Frame *frame = NULL;
    ListIter frames;
    ListIter frameEntries;
    uint8_t encoding = ID3V2_ENCODING_UTF8;
    const char *artist = "Billie Eilish";
    const char *song = "Happier Than Ever";
    const char *note = "Recorded in Finneas' bedroom studio";
    size_t size = 0;
    char *str = NULL;

    // variables for ID3
    ID3 *id3 = NULL;


    /**
     * Describe the layout of XER1: a one byte encoding followed by 3 strings in that encoding.
     * The "encoding" key is special, id3dev uses it to decode the encoded string fields after it.
     */
    context = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext,
                         id3v2CopyContentContext);

    listInsertBack(context, id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1));
    listInsertBack(context, id3v2CreateContentContext(encodedString_context, id3v2djb2("artist"), 256, 1));
    listInsertBack(context, id3v2CreateContentContext(encodedString_context, id3v2djb2("song"), 256, 1));
    listInsertBack(context, id3v2CreateContentContext(encodedString_context, id3v2djb2("note"), 256, 1));

    // register the layout so id3dev knows how to build (and later read) XER1
    pairs = hashTableCreate(8, id3v2DeleteContentContextList, id3v2PrintContentContextList,
                            id3v2CopyContentContextList);
    id3v2InsertIdentifierContextPair(pairs, "XER1", context);

    // the table stores its own copy of the layout
    listFree(context);

    // create an empty ID3v2.4 tag
    v2tag = id3v2CreateTag(id3v2CreateTagHeader(ID3V2_TAG_VERSION_4, 0, 0, NULL),
                           listCreate(id3v2PrintFrame, id3v2DeleteFrame, id3v2CompareFrame, id3v2CopyFrame));

    // create an XER1 frame, its entries start as a single zero byte
    frame = id3v2CreateEmptyFrame("XER1", ID3V2_TAG_VERSION_4, pairs);

    if (frame == NULL) {
        printf("could not create XER1\n");
        id3v2DestroyTag(&v2tag);
        hashTableFree(pairs);
        return EXIT_FAILURE;
    }

    // fill its 4 entries in order, the traverser moves to the next entry after each write
    frameEntries = id3v2CreateFrameEntryTraverser(frame);
    id3v2WriteFrameEntry(frame, &frameEntries, 1, &encoding);
    free(id3v2ReadFrameEntry(&frameEntries, &size));

    id3v2WriteFrameEntry(frame, &frameEntries, strlen(artist), artist);
    free(id3v2ReadFrameEntry(&frameEntries, &size));

    id3v2WriteFrameEntry(frame, &frameEntries, strlen(song), song);
    free(id3v2ReadFrameEntry(&frameEntries, &size));

    id3v2WriteFrameEntry(frame, &frameEntries, strlen(note), note);

    // the tag takes ownership of the frame
    id3v2AttachFrameToTag(v2tag, frame);

    id3 = id3Create(v2tag, NULL);

    // print every XER1 frame in the tag
    frames = id3v2CreateFrameTraverser(id3->id3v2);

    while ((frame = id3v2FrameTraverse(&frames)) != NULL) {
        if (memcmp(frame->header->id, "XER1", ID3V2_FRAME_ID_MAX_SIZE) == 0) {
            printf("XER1 frame:\n");
            frameEntries = id3v2CreateFrameEntryTraverser(frame);

            printf("\t|encoding: %d\n", id3v2ReadFrameEntryAsU8(&frameEntries));

            str = id3v2ReadFrameEntryAsChar(&frameEntries, &size);
            printf("\t|artist: %s\n", str);
            free(str);

            str = id3v2ReadFrameEntryAsChar(&frameEntries, &size);
            printf("\t|song: %s\n", str);
            free(str);

            str = id3v2ReadFrameEntryAsChar(&frameEntries, &size);
            printf("\t|note: %s\n", str);
            free(str);
        }
    }

    id3Destroy(&id3);
    hashTableFree(pairs);

    return EXIT_SUCCESS;
}
