/**
 * @file extractTag.c
 * @author Ewan Jones
 * @brief extracts a tag from a file, prints it as JSON and writes it to another file.
 * @version 0.1
 * @date 2024-04-23
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#include <stdio.h> // printf
#include <stdlib.h> // EXIT_SUCCESS, EXIT_FAILURE
#include <id3dev.h> // id3FromFile, id3ToJSON, id3WriteToFile, id3Destroy

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("USAGE: <mp3 file> <output file>\n");
        return EXIT_FAILURE;
    }

    ID3 *id3 = NULL;
    char *json = NULL;

    // Parse ID3 metadata from the from provided file
    id3 = id3FromFile(argv[1]);

    // Print the ID3 metadata as JSON
    json = id3ToJSON(id3);
    printf("%s\n", json);
    free(json);

    // Write the ID3 metadata to a file without audio content
    id3WriteToFile(argv[2], id3);

    id3Destroy(&id3);

    return EXIT_SUCCESS;
}
