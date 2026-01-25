/**
 * @file listAllFrames.c
 * @author Ewan Jones
 * @brief lists all id3v2 frames in a provided mp3 file
 * @version 0.1
 * @date 2024-04-22
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h> // printf
#include <stdlib.h> // EXIT_SUCCESS, EXIT_FAILURE
#include <id3dev.h> // id3FromFile, id3Destroy, ListIter
#include <id3v2/id3v2Frame.h> // Id3v2Frame, id3v2CreateFrameTraverser, id3v2FrameTraverse

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("USAGE: <mp3 file>\n");
        return EXIT_FAILURE;
    }

    ID3 *id3 = NULL;
    Id3v2Frame *f = NULL;
    ListIter frames;
    int n = 0;

    // Parse ID3 metadata from the from provided file
    id3 = id3FromFile(argv[1]);

    // Check if the file contains any ID3v2.x versions
    if (id3->id3v2 == NULL) {
        printf("ERROR: %s does not contain any ID3v2.x tags\n", argv[1]);
        id3Destroy(&id3);
        return EXIT_FAILURE;
    }


    // create a list iterator to traverse the frames
    frames = id3v2CreateFrameTraverser(id3->id3v2);

    // Traverse the frames and print the frame id
    while ((f = id3v2FrameTraverse(&frames)) != NULL) {
        n++;
        printf("frame %d: %s\n", n, (char *) f->header->id);
    }

    id3Destroy(&id3);
    return EXIT_SUCCESS;
}
