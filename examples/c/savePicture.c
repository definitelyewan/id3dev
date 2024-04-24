/**
 * @file savePicture.c
 * @author Ewan Jones
 * @brief checks for a picture in a provided mp3 file and saves it to a new file
 * @version 0.1
 * @date 2024-04-22
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h> // printf
#include <stdlib.h> // EXIT_SUCCESS, EXIT_FAILURE
#include <stdint.h> // uint8_t
#include <id3dev.h> // id3FromFile, id3ReadPicture, id3Destroy

int main(int argc, char *argv[]){

    if(argc < 3){
        printf("USAGE: <mp3 file> <output file>\n");
        return EXIT_FAILURE;
    }

    FILE *fp = NULL;
    ID3 *id3 = NULL;
    uint8_t *picture = NULL;
    size_t pictureSize = 0;

    // Parse all versions of ID3 and store it in a structure
    id3 = id3FromFile(argv[1]);

    /**
     * Reads the attached picture from an ID3v2.x tag with the image type of 0.
     * the image type is usually set to 0 for cover art in mp3 files despite not
     * matching the ID3v2.x specification.
     * 
     */
    picture = id3ReadPicture(0, id3, &pictureSize);

    /**
     * Check to see if the ID3 structure contains a picture, if not exit. Not all
     * mp3 files contain pictures thus, NULL will be returned if no picture is found
     * with a size of 0.
     * 
     */
    if(picture == NULL || pictureSize == 0){
        printf("No picture found\n");
        return EXIT_SUCCESS;
    }

    // create a new file and write the picture to it
    fp = fopen(argv[2], "wb");
    
    if(fp == NULL){
        printf("Failed to open file\n");
        return EXIT_FAILURE;
    }

    fwrite(picture, 1, pictureSize, fp);


    return EXIT_SUCCESS;
}