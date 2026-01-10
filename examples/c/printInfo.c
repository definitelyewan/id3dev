/**
 * @file printInfo.c
 * @author Ewan Jones
 * @brief print the basic information of a mp3 file to a console
 * @version 0.1
 * @date 2024-04-22
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h> // printf
#include <stdlib.h> // EXIT_SUCCESS, EXIT_FAILURE
#include <id3dev.h> // id3FromFile, id3ReadTitle, id3ReadArtist, id3ReadAlbumArtist, id3ReadAlbum, id3ReadYear, id3ReadGenre, id3ReadTrack, id3ReadComposer, id3ReadDisc, id3ReadLyrics, id3ReadComment, id3ReadPicture, id3Destroy



int main(int argc, char *argv[]){

    if(argc < 2){
        printf("USAGE: <mp3 file>\n");
        return EXIT_FAILURE;
    }    

    ID3 *id3 = NULL;
    char *title = NULL;
    char *artist = NULL;
    char *albumArtist = NULL;
    char *album = NULL;
    char *year = NULL;
    char *genre = NULL;
    char *track = NULL;
    char *composer = NULL;
    char *disc = NULL;
    char *lyrics = NULL;
    char *comment = NULL;


    // Parse all versions of ID3 and store it in a structure
    id3 = id3FromFile(argv[1]);

    // Read basic information about the song
    title = id3ReadTitle(id3);
    artist = id3ReadArtist(id3);
    albumArtist = id3ReadAlbumArtist(id3);
    album = id3ReadAlbum(id3);
    year = id3ReadYear(id3);
    genre = id3ReadGenre(id3);
    track = id3ReadTrack(id3);
    composer = id3ReadComposer(id3);
    disc = id3ReadDisc(id3);
    lyrics = id3ReadLyrics(id3);
    comment = id3ReadComment(id3);


    if(title != NULL){
        printf("Title: %s\n", title);
        free(title);
    }

    if(artist != NULL){
        printf("Artist: %s\n", artist);
        free(artist);
    }

    if(albumArtist != NULL){
        printf("Album Artist: %s\n", albumArtist);
        free(albumArtist);
    }

    if(album != NULL){
        printf("Album: %s\n", album);
        free(album);
    }

    if(year != NULL){
        printf("Year: %s\n", year);
        free(year);
    }

    if(genre != NULL){
        printf("Genre: %s\n", genre);
        free(genre);
    }

    if(track != NULL){
        printf("Track: %s\n", track);
        free(track);
    }

    if(composer != NULL){
        printf("Composer: %s\n", composer);
        free(composer);
    }

    if(disc != NULL){
        printf("Disc: %s\n", disc);
        free(disc);
    }

    if(lyrics != NULL){
        printf("Lyrics: %s\n", lyrics);
        free(lyrics);
    }

    if(comment != NULL){
        printf("Comment: %s\n", comment);
        free(comment);
    }

    // Frees all memory used by an ID3 structure
    id3Destroy(&id3);

    return EXIT_SUCCESS;
}