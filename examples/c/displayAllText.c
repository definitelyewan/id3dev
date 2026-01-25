/**
 * @file displayAllText.c
 * @author Ewan Jones
 * @brief Displays all text information from an mp3 file
 * @version 0.1
 * @date 2024-04-22
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h> // printf
#include <stdlib.h> // EXIT_SUCCESS, EXIT_FAILURE
#include <id3dev.h> // id3FromFile, id3SetPreferredStandard, id3ReadTitle, id3ReadArtist, id3ReadAlbum, id3ReadYear, id3ReadGenre, id3ReadTrack, id3ReadComment, id3Destroy

#include <stdint.h> // uint8_t
#include <id3v2/id3v2Frame.h> // Id3v2Frame, id3v2CreateFrameTraverser, id3v2FrameTraverse, id3v2CreateFrameEntryTraverser, id3v2ReadFrameEntryAsU8, id3v2ReadFrameEntryAsChar


int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("USAGE: <mp3 file>\n");
        return EXIT_FAILURE;
    }


    ID3 *id3 = NULL;

    // Parse ID3 metadata from the from provided file
    id3 = id3FromFile(argv[1]);

    // Check to see if any ID2v2 tags are present
    if (id3->id3v2 == NULL) {
        printf("No ID3v2 tags found in %s\n", argv[1]);
    } else {
        Id3v2Frame *f = NULL;
        ListIter frames;
        ListIter entries;
        uint8_t encoding = 0;
        size_t size = 0;
        char *text = NULL;
        int n = 0;

        printf("ID3v2 tags found in %s ----------\n", argv[1]);


        // Create a list iterator to traverse the frames
        frames = id3v2CreateFrameTraverser(id3->id3v2);

        while ((f = id3v2FrameTraverse(&frames)) != NULL) {
            /**
             * All text frames will start with the character 'T' however, there are some exceptions
             * for example, user defined text frames (TXX, TXXX) have a different strcutre and will be
             * skipped.
             */

            if (f->header->id[0] != 'T' || f->header->id[1] == 'X') {
                continue;
            }

            n++;

            // traverse through the frame entries
            entries = id3v2CreateFrameEntryTraverser(f);

            // read the encoding and text from the frame
            encoding = id3v2ReadFrameEntryAsU8(&entries);

            // read the encoded text as UTF8
            text = id3v2ReadFrameEntryAsChar(&entries, &size);

            printf("[%s] frame %d:\n\tEncoding: %d\n\tText: %s\n", (char *) f->header->id, n, encoding, text);
            free(text);
        }
    }

    // Check to see if any ID3v1 tags are present
    if (id3->id3v1 == NULL) {
        printf("No ID3v1 tags found in %s\n", argv[1]);
    } else {
        char *str = NULL;

        printf("ID3v1 tag found in %s ----------\n", argv[1]);

        // Set the preferred standard to ID3v1 as to force the library to read ID3v1 tags
        id3SetPreferredStandard(ID3V1_TAG_VERSION);

        // Read ID3v1 tags title
        str = id3ReadTitle(id3);

        if (str != NULL) {
            printf("Title: %s\n", str);
            free(str);
        }

        // Reads the ID3v1 tags artist
        str = id3ReadArtist(id3);

        if (str != NULL) {
            printf("Artist: %s\n", str);
            free(str);
        }

        // Reads the ID3v1 tags album
        str = id3ReadAlbum(id3);

        if (str != NULL) {
            printf("Album: %s\n", str);
            free(str);
        }

        // Reads the ID3v1 tags year
        str = id3ReadYear(id3);

        if (str != NULL) {
            printf("Year: %s\n", str);
            free(str);
        }

        // Reads the ID3v1 tags genre
        str = id3ReadGenre(id3);

        if (str != NULL) {
            printf("Genre: %s\n", str);
            free(str);
        }

        // Reads the ID3v1 tags track
        str = id3ReadTrack(id3);

        if (str != NULL) {
            printf("Track: %s\n", str);
            free(str);
        }

        // Reads the ID3v1 tags comment
        str = id3ReadComment(id3);

        if (str != NULL) {
            printf("Comment: %s\n", str);
            free(str);
        }
    }

    // Frees all ID3 data
    id3Destroy(&id3);

    return EXIT_SUCCESS;
}
