/**
 * @file id3v1.c
 * @author Ewan Jones
 * @brief Function definitions for utility
 * @version 2.0
 * @date 2023-10-03
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "id3v1/id3v1Parser.h"
#include "id3v1/id3v1.h"
#include "id3dependencies/ByteStream/include/byteStream.h"
#include "id3dependencies/ByteStream/include/byteInt.h"

/**
 * @brief Creates an Id3v1Tag from a provided file path.
 * @details This function will read the last 128 bytes of the file and parse them into an Id3v1Tag structure. On success
 * this function returns a pointer to the Id3v1Tag structure, otherwise it returns NULL.
 * @param filePath 
 * @return Id3v1Tag* 
 */
Id3v1Tag *id3v1TagFromFile(const char *filePath){

    if(filePath == NULL){
        return NULL;
    }

    FILE *fp = NULL;
    uint8_t id3Bytes[ID3V1_MAX_SIZE];
    
    // make sure the file can really be read
    fp = fopen(filePath, "rb");
    if(fp == NULL){
        return NULL;
    }

    // seek to the start of metadata
    if((fseek(fp, -ID3V1_MAX_SIZE, SEEK_END)) != 0){
        fclose(fp);
        return NULL;
    }

    if((fread(id3Bytes, ID3V1_MAX_SIZE, 1, fp)) != 1){
        fclose(fp);
        return NULL;
    }

    fclose(fp);    
    return id3v1TagFromBuffer(id3Bytes);
}

/**
 * @brief Deep copies an Id3v1Tag structure.
 * @details On success this function returns a pointer to the new heap allocated Id3v1Tag structure, otherwise it
 * returns NULL.
 * @param toCopy 
 * @return Id3v1Tag* 
 */
Id3v1Tag *id3v1CopyTag(Id3v1Tag *toCopy){
    
    if(toCopy == NULL){
        return NULL;
    }

    return id3v1CreateTag(toCopy->title, toCopy->artist, toCopy->albumTitle, toCopy->year, toCopy->track,
        toCopy->comment, toCopy->genre);
}

/**
 * @brief Writes ID3V1_FIELD_SIZE bytes to a buffer.
 * @details This function is not in any header and is private to this library. It returns 1 on success and 0 otherwise.
 * @param src 
 * @param dest 
 * @return int 
 */
static int internal_id3v1CharsToStructUint8(const char *src, uint8_t *dest){

    memset(dest, 0, ID3V1_FIELD_SIZE);
    
    if(src != NULL){
        const size_t len = strlen(src);
        size_t wLen = ((len > ID3V1_FIELD_SIZE) ? ID3V1_FIELD_SIZE : len);

        memcpy(dest, (uint8_t *)src, wLen);    
    }
    
    return 1;

}

/**
 * @brief Writes a title to tag.
 * @details Writes a title to the tag structure. This function will write up to ID3V1_FIELD_SIZE bytes to the title
 * field of the tag. On success this function returns 1, otherwise it returns 0.
 * @param title 
 * @param tag 
 * @return int 
 */
int id3v1WriteTitle(const char *title, Id3v1Tag *tag){
    return (tag == NULL) ? 0 : internal_id3v1CharsToStructUint8(title, tag->title);
}

/**
 * @brief Writes an artist to tag.
 * @details Writes an artist to the tag structure. This function will write up to ID3V1_FIELD_SIZE bytes to the artist
 * field of the tag. On success this function returns 1, otherwise it returns 0.
 * @param artist
 * @param tag 
 * @return int 
 */
int id3v1WriteArtist(const char *artist, Id3v1Tag *tag){
    return (tag == NULL) ? 0 : internal_id3v1CharsToStructUint8(artist, tag->artist);
}

/**
 * @brief Writes an album to tag.
 * @details Writes an album to the tag structure. This function will write up to ID3V1_FIELD_SIZE bytes to the album
 * field of the tag. On success this function returns 1, otherwise it returns 0.
 * @param album
 * @param tag 
 * @return int 
 */
int id3v1WriteAlbum(const char *album, Id3v1Tag *tag){
    return (tag == NULL) ? 0 : internal_id3v1CharsToStructUint8(album, tag->albumTitle);
}

/**
 * @brief Writes a year to tag.
 * @details Writes a year to the tag structure. This function will write an integer to the year field of the tag. On
 * success this function returns 1, otherwise it returns 0.
 * @param year
 * @param tag 
 * @return int 
 */
int id3v1WriteYear(int year, Id3v1Tag *tag){
    
    if(tag == NULL){
        return 0;
    }

    tag->year = year;
    return 1;
}

/**
 * @brief Writes a comment to tag.
 * @details Writes a comment to the tag structure. This function will write up to ID3V1_FIELD_SIZE bytes to the comment
 * filed of the tag. On success this function returns 1, otherwise it returns 0.
 * @param comment
 * @param tag 
 * @return int 
 */
int id3v1WriteComment(const char *comment, Id3v1Tag *tag){
    return (tag == NULL) ? 0: internal_id3v1CharsToStructUint8(comment, tag->comment);
}

/**
 * @brief Writes a genre to tag.
 * @details Writes a genre to the tag structure. This function will write an integer (from 0 to 255) to the genre field
 * of the tag. On success this function returns 1, otherwise it returns 0.
 * @param genre
 * @param tag 
 * @return int 
 */
int id3v1WriteGenre(const Genre genre, Id3v1Tag *tag){
    
    if(tag == NULL){
        return 0;
    }

    tag->genre = genre;
    return 1;
}

/**
 * @brief Writes a track to tag.
 * @details Writes a track number to the tag structure. This function will write an integer (from 0 to 255) to the
 * track field of the tag. On success this function returns 1, otherwise it returns 0.
 * @param track
 * @param tag 
 * @return int 
 */
int id3v1WriteTrack(const int track, Id3v1Tag *tag){
    
    if(tag == NULL){
        return 0;
    }

    tag->track = track;
    return 1;

}

/**
 * @brief Compares two different tags.
 * @details Returns true if the provided tags are equal otherwise, false. 
 * @param tag1 
 * @param tag2 
 * @return true 
 * @return false 
 */
bool id3v1CompareTag(const Id3v1Tag *tag1, const Id3v1Tag *tag2){

    if(tag1 == NULL || tag2 == NULL){
        return false;
    }

    if(tag1->genre != tag2->genre){
        return false;
    }

    if(tag1->track != tag2->track){
        return false;
    }

    if(tag1->year != tag2->year){
        return false;
    }

    if(memcmp(tag1->albumTitle, tag2->albumTitle, ID3V1_FIELD_SIZE) != 0){
        return false;
    }

    if(memcmp(tag1->artist, tag2->artist, ID3V1_FIELD_SIZE) != 0){
        return false;
    }

    if(memcmp(tag1->comment, tag2->comment, ID3V1_FIELD_SIZE) != 0){
        return false;
    }

    if(memcmp(tag1->title, tag2->title, ID3V1_FIELD_SIZE) != 0){
        return false;
    }

    return true;
}

/**
 * @brief Provides a string equivalent to the genre enum.
 * @details Values outside the genre enum will be reported as other and return "Other".
 * @param val 
 * @return char* 
 */
char *id3v1GenreFromTable(const Genre val){

    // int to string
    switch (val){
        case BLUES_GENRE:
            return "Blues\0";
        case CLASSIC_ROCK_GENRE:
            return "Classic Rock\0";
        case COUNTRY_GENRE:
            return "Country\0";
        case DANCE_GENRE:
            return "Dance\0";
        case DISCO_GENRE:
            return "Disco\0";
        case FUNK_GENRE:
            return "Funk\0";
        case GRUNGE_GENRE:
            return "Grunge\0";
        case HIP_HOP_GENRE:
            return "Hip-Hop\0";
        case JAZZ_GENRE:
            return "Jazz\0";
        case METAL_GENRE:
            return "Metal\0";
        case NEW_AGE_GENRE:
            return "New Age\0";
        case OLDIES_GENRE:
            return "Oldies\0";
        case OTHER_GENRE:
            return "Other\0";
        case POP_GENRE:
            return "Pop\0";
        case RHYTHM_AND_BLUES_GENRE:
            return "Rhythm and Blues\0";
        case RAP_GENRE:
            return "Rap\0";
        case REGGAE_GENRE:
            return "Reggae\0";
        case ROCK_GENRE:
            return "Rock\0";
        case TECHNO_GENRE:
            return "Techno\0";
        case INDUSTRIAL_GENRE:
            return "Industrial\0";
        case ALTERNATIVE_GENRE:
            return "Alternative\0";
        case SKA_GENRE:
            return "Ska\0";
        case DEATH_METAL_GENRE:
            return "Death Metal\0";
        case PRANKS_GENRE:
            return "Pranks\0";
        case SOUNDTRACK_GENRE:
            return "Soundtrack\0";
        case EURO_TECHNO_GENRE:
            return "Euro-Techno\0";
        case AMBIENT_GENRE:
            return "Ambient\0";
        case TRIP_HOP_GENRE:
            return "Trip-Hop\0";
        case VOCAL_GENRE:
            return "Vocal\0";
        case JAZZ_AND_FUNK_GENRE:
            return "Jazz and Funk\0";
        case FUSION_GENRE:
            return "Fusion\0";
        case TRANCE_GENRE:
            return "Trance\0";
        case CLASSICAL_GENRE:
            return "Classical\0";
        case INSTRUMENTAL_GENRE:
            return "Instrumental\0";
        case ACID_GENRE:
            return "Acid\0";
        case HOUSE_GENRE:
            return "House\0";
        case GAME_GENRE:
            return "Game\0";
        case SOUND_CLIP_GENRE:
            return "Sound Clip\0";
        case GOSPEL_GENRE:
            return "Gospel\0";
        case NOISE_GENRE:
            return "Noise\0";
        case ALTERNATIVE_ROCK_GENRE:
            return "Alternative Rock\0";
        case BASS_GENRE:
            return "Bass\0";
        case SOUL_GENRE:
            return "Soul\0";
        case PUNK_GENRE:
            return "Punk\0";
        case SPACE_GENRE:
            return "Space\0";
        case MEDITATIVE_GENRE:
            return "Meditative\0";
        case INSTRUMENTAL_POP_GENRE:
            return "Instrumental Pop\0";
        case INSTRUMENTAL_ROCK_GENRE:
            return "Instrumental Rock\0";
        case ETHNIC_GENRE:
            return "Ethnic\0";
        case GOTHIC_GENRE:
            return "Gothic\0";
        case DARKWAVE_GENRE:
            return "Darkwave\0";
        case TECHNO_INDUSTRIAL_GENRE:
            return "Techno Industrial\0";
        case ELECTRONIC_GENRE:
            return "Electronic\0";
        case POP_FOLK_GENRE:
            return "Pop Folk\0";
        case EURODANCE_GENRE:
            return "Eurodance\0";
        case DREAM_GENRE:
            return "Dream\0";
        case SOUTHERN_ROCK_GENRE:
            return "Southern Rock\0";
        case COMEDY_GENRE:
            return "Comedy\0";
        case CULT_GENRE:
            return "Cult\0";
        case GANGSTA_GENRE:
            return "Gangsta\0";
        case TOP_40_GENRE:
            return "Top 40\0";
        case CHRISTIAN_RAP_GENRE:
            return "Christian Rap\0";
        case POP_FUNK_GENRE:
            return "Pop Funk\0";
        case JUNGLE_MUSIC_GENRE:
            return "Jungle Music\0";
        case NATIVE_US_GENRE:
            return "Native US\0";
        case CABARET_GENRE:
            return "Cabaret\0";
        case NEW_WAVE_GENRE:
            return "New Wave\0";
        case PSYCHEDELIC_GENRE:
            return "Psychedelic\0";
        case RAVE_GENRE:
            return "Rave\0";
        case SHOWTUNES_GENRE:
            return "Showtunes\0";
        case TRAILER_GENRE:
            return "Trailer\0";
        case LO_FI_GENRE:
            return "Lo-Fi\0";
        case TRIBAL_GENRE:
            return "Tribal\0";
        case ACID_PUNK_GENRE:
            return "Acid Punk\0";
        case ACID_JAZZ_GENRE:
            return "Acid Jazz\0";
        case POLKA_GENRE:
            return "Polka\0";
        case RETRO_GENRE:
            return "Retro\0";
        case MUSICAL_GENRE:
            return "Musical\0";
        case ROCKNROLL_GENRE:
            return "Rock and Roll\0";
        case HARD_ROCK_GENRE:
            return "Hard Rock\0";
        case FOLK_GENRE:
            return "Folk\0";
        case FOLK_ROCK_GENRE:
            return "Folk Rock\0";
        case NATIONAL_FOLK_GENRE:
            return "National Folk\0";
        case SWING_GENRE:
            return "Swing\0";
        case FAST_FUSION_GENRE:
            return "Fast Fusion\0";
        case BEBOP_GENRE:
            return "Bebop\0";
        case LATIN_GENRE:
            return "Latin\0";
        case REVIVAL_GENRE:
            return "Revival\0";
        case CELTIC_GENRE:
            return "Celtic\0";
        case BLUEGRASS_GENRE:
            return "Bluegrass\0";
        case AVANTGARDE_GENRE:
            return "Avantgarde\0";
        case GOTHIC_ROCK_GENRE:
            return "Gothic Rock\0";
        case PROGRESSIVE_ROCK_GENRE:
            return "Progressive Rock\0";
        case PSYCHEDELIC_ROCK_GENRE:
            return "Psychedelic Rock\0";
        case SYMPHONIC_ROCK_GENRE:
            return "Symphonic Rock\0";
        case SLOW_ROCK_GENRE:
            return "Slow Rock\0";
        case BIG_BAND_GENRE:
            return "Big Band\0";
        case CHORUS_GENRE:
            return "Chorus\0";
        case EASY_LISTENING_GENRE:
            return "Easy Listening\0";
        case ACOUSTIC_GENRE:
            return "Acoustic\0";
        case HUMOUR_GENRE:
            return "Humour\0";
        case SPEECH_GENRE:
            return "Speech\0";
        case CHANSON_GENRE:
            return "Chanson\0";
        case OPERA_GENRE:
            return "Opera\0";
        case CHAMBER_MUSIC_GENRE:
            return "Chamber Music\0";
        case SONATA_GENRE:
            return "Sonata\0";
        case SYMPHONY_GENRE:
            return "Symphony\0";
        case BOOTY_BASS_GENRE:
            return "Booty Bass\0";
        case PRIMUS_GENRE:
            return "Primus\0";
        case PORN_GROOVE_GENRE:
            return "Porn Groove\0";
        case SATIRE_GENRE:
            return "Satire\0";
        case SLOW_JAM_GENRE:
            return "Slow Jam\0";
        case CLUB_GENRE:
            return "Club\0";
        case TANGO_GENRE:
            return "Tango\0";
        case SAMBA_GENRE:
            return "Samba\0";
        case FOLKLORE_GENRE:
            return "Folklore\0";
        case BALLAD_GENRE:
            return "Ballad\0";
        case POWER_BALLAD_GENRE:
            return "Power Ballad\0";
        case RHYTHMIC_SOUL_GENRE:
            return "Rhythmic Soul\0";
        case FREESTYLE_GENRE:
            return "Freestyle\0";
        case DUET_GENRE:
            return "Duet\0";
        case PUNK_ROCK_GENRE:
            return "Punk Rock\0";
        case DRUM_SOLO_GENRE:
            return "Drum Solo\0";
        case A_CAPPELLA_GENRE:
            return "A Cappella\0";
        case EURO_HOUSE_GENRE:
            return "Euro-House\0";
        case DANCE_HALL_GENRE:
            return "Dance Hall\0";
        case GOA_MUSIC_GENRE:
            return "Goa Music\0";
        case DRUM_AND_BASS_GENRE:
            return "Drum and Bass\0";
        case CLUB_HOUSE_GENRE:
            return "Club-House\0";
        case HARDCORE_TECHNO_GENRE:
            return "Hardcore Techno\0";
        case TERROR_GENRE:
            return "Terror\0";
        case INDIE_GENRE:
            return "Indie\0";
        case BRITPOP_GENRE:
            return "Britpop\0";
        case NEGERPUNK_GENRE:
            return "Negerpunk\0";
        case POLSK_PUNK_GENRE:
            return "Polsk Punk\0";
        case BEAT_GENRE:
            return "Beat\0";
        case CHRISTIAN_GANGSTA_RAP_GENRE:
            return "Christian Gangsta Rap\0";
        case HEAVY_METAL_GENRE:
            return "Heavy Metal\0";
        case BLACK_METAL_GENRE:
            return "Black Metal\0";
        case CROSSOVER_GENRE:
            return "Crossover\0";
        case CONTEMPORARY_CHRISTIAN_GENRE:
            return "Contemporary Christian\0";
        case CHRISTIAN_ROCK_GENRE:
            return "Christian Rock\0";
        case MERENGUE_GENRE:
            return "Merengue\0";
        case SALSA_GENRE:
            return "Salsa\0";
        case  THRASH_METAL_GENRE:
            return "Thrash Metal\0";
        case ANIME_GENRE:
            return "Anime\0";
        case JPOP_GENRE:
            return "Jpop\0";
        case SYNTHPOP_GENRE:
            return "Synthpop\0";
        case ABSTRACT_GENRE:
            return "Abstract\0";
        case ART_ROCK_GENRE:
            return "Art Rock";
        case BAROQUE_GENRE:
            return "Baroque\0";
        case BIG_BEAT_GENRE:
            return "Big Beat\0";
        case BREAKBEAT_GENRE:
            return "Breakbeat\0";
        case CHILLOUT_GENRE:
            return "Chillout\0";
        case DOWNTEMPO_GENRE:
            return "Downtempo\0";
        case DUB_GENRE:
            return "Dub\0";
        case EBM_GENRE:
            return "EBM\0";
        case ECLECTIC_GENRE:
            return "Eclectic\0";
        case ELECTRO_GENRE:
            return "Electro\0";
        case ELECTROCLASH_GENRE:
            return "Electroclash\0";
        case EMO_GENRE:
            return "Emo\0";
        case EXPERIMENTAL_GENRE:
            return "Experimental\0";
        case GARAGE_GENRE:
            return "Garage\0";
        case GLOBAL_GENRE:
            return "Global\0";
        case IDM_GENRE:
            return "IDM\0";
        case ILLBIENT_GENRE:
            return "Illbient\0";
        case INDUSTRO_GOTH_GENRE:
            return "Industro-Goth\0";
        case JAM_BAND_GENRE:
            return "Jam Band\0";
        case KRAUTROCK_GENRE:
            return "Krautrock\0";
        case LEFTFIELD_GENRE:
            return "Leftfield\0";
        case LOUNGE_GENRE:
            return "Lounge\0";
        case MATH_ROCK_GENRE:
            return "Math Rock\0";
        case NEW_ROMANTIC_GENRE:
            return "New Romantic\0";
        case NU_BREAKZ_GENRE:
            return "Nu-Breakz\0";
        case POST_PUNK_GENRE:
            return "Post-Punk\0";
        case POST_ROCK_GENRE:
            return "Post-Rock\0";
        case PSYTRANCE_GENRE:
            return "Psytrance\0";
        case SHOEGAZE_GENRE:
            return "Shoegaze\0";
        case SPACE_ROCK_GENRE:
            return "Space Rock\0";
        case TROP_ROCK_GENRE:
            return "Trop Rock\0";
        case WORLD_MUSIC_GENRE:
            return "World Music\0";
        case NEOCLASSICAL_GENRE:
            return "Neoclassical\0";
        case AUDIOBOOK_GENRE:
            return "Audiobook\0";
        case AUDIO_THEATRE_GENRE:
            return "Audio Theatre\0";
        case NEUE_DEUTSCHE_WELLE_GENRE:
            return "Neue Deutsche Welle\0";
        case PODCAST_GENRE:
            return "Podcast\0";
        case INDIE_ROCK_GENRE:
            return "Indie-Rock\0";
        case G_FUNK_GENRE:
            return "G-Funk\0";
        case DUBSTEP_GENRE:
            return "Dubstep\0";
        case GARAGE_ROCK_GENRE:
            return "Garage Rock\0";
        case PSYBIENT_GENRE:
            return "Psybient\0";
        default:
            break;
    }
    // 191 supported genres god this enum is huge
    return "Other\0";
}

/**
 * @brief Reads a title from a tag.
 * @details Mainly for compatibility and use in ffi.
 * @param tag 
 * @return char* 
 */
char *id3v1ReadTitle(const Id3v1Tag *tag){
    char *r = calloc(sizeof(char), ID3V1_FIELD_SIZE + 1);
    memcpy(r, tag->title, ID3V1_FIELD_SIZE);
    return r;
}

/**
 * @brief Reads an artist from a tag.
 * @details Mainly for compatibility and use in ffi.
 * @param tag 
 * @return char* 
 */
char *id3v1ReadArtist(const Id3v1Tag *tag){
    char *r = calloc(sizeof(char), ID3V1_FIELD_SIZE + 1);
    memcpy(r, tag->artist, ID3V1_FIELD_SIZE);
    return r;
}

/**
 * @brief Reads an album from a tag.
 * @details Mainly for compatibility and use in ffi.
 * @param tag 
 * @return char* 
 */
char *id3v1ReadAlbum(const Id3v1Tag *tag){
    char *r = calloc(sizeof(char), ID3V1_FIELD_SIZE + 1);
    memcpy(r, tag->albumTitle, ID3V1_FIELD_SIZE);
    return r;
}

/**
 * @brief Reads a year from a tag.
 * @details Mainly for compatibility and use in ffi.
 * @param tag 
 * @return char* 
 */
int id3v1ReadYear(const Id3v1Tag *tag){
    return tag->year;
}

/**
 * @brief Reads a comment from a tag.
 * @details Mainly for compatibility and use in ffi.
 * @param tag 
 * @return char* 
 */
char *id3v1ReadComment(const Id3v1Tag *tag){
    char *r = calloc(sizeof(char), ID3V1_FIELD_SIZE + 1);
    memcpy(r, tag->comment, ID3V1_FIELD_SIZE);
    return r;
}

/**
 * @brief Reads a genre from a tag
 * @details Mainly for compatibility and use in ffi
 * @param tag 
 * @return char* 
 */
Genre id3v1ReadGenre(const Id3v1Tag *tag){
    return tag->genre;
}

/**
 * @brief Reads a track from a tag
 * @details Mainly for compatibility and use in ffi
 * @param tag 
 * @return char* 
 */
int id3v1ReadTrack(const Id3v1Tag *tag){
    return tag->track;
}

/**
 * @brief Converts an Id3v1Tag to a JSON string.
 * @param tag 
 * @return char* 
 */
char *id3v1ToJSON(const Id3v1Tag *tag){

    char *jsonStr = NULL;
    int memCount = 0;

    if(tag == NULL){
        jsonStr = calloc(sizeof(char), 3);
        strncpy(jsonStr,"{}",3);
        return jsonStr;
    }

    // 98 is for formating
    memCount = (int) (98 + ID3V1_MAX_SIZE + (sizeof(int) * 3) + strlen(id3v1GenreFromTable(tag->genre)));
    jsonStr = calloc(sizeof(char), memCount);

    sprintf(jsonStr,"{\"title\":\"%s\",\"artist\":\"%s\",\"album\":\"%s\",\"year\":%d,\"track\":%d,\"comment\":\"%s\",\"genreNumber\":%d,\"genre\":\"%s\"}",
    (char *) tag->title, 
    (char *) tag->artist,
    (char *) tag->albumTitle, 
    tag->year,
    tag->track, 
    (char *) tag->comment,
    tag->genre, 
    id3v1GenreFromTable(tag->genre));
    return jsonStr;
}

/**
 * @brief Writes a Id3v1Tag structure to a file located at file path.
 * @details If the file does not exist it will be created. otherwise, if a tag is found it will be overwritten and
 * if not it will be appended.
 * @param filePath 
 * @param tag 
 * @return int 
 */
int id3v1WriteTagToFile(const char *filePath, const Id3v1Tag *tag){

    if(filePath == NULL || tag == NULL){
        return 0;
    }

    char *tmp = NULL;
    unsigned char byte = 0x00;
    int n = 0;
    int yearW = 0;
    FILE *fp = NULL;
    ByteStream *stream = byteStreamCreate(NULL, ID3V1_MAX_SIZE);

    byteStreamWrite(stream, (unsigned char *)"TAG", ID3V1_TAG_ID_SIZE);
    byteStreamWrite(stream, (unsigned char *)tag->title, ID3V1_FIELD_SIZE);
    byteStreamWrite(stream, (unsigned char *)tag->artist, ID3V1_FIELD_SIZE);
    byteStreamWrite(stream, (unsigned char *)tag->albumTitle, ID3V1_FIELD_SIZE);
    
    // int to string
    n = (int) log10(tag->year) + 1;
    yearW = tag->year;
    tmp = calloc(n, sizeof(char));

    for(int i = n - 1; i >= 0; --i, yearW /= 10){
        tmp[i] = (char) ((yearW % 10) + '0');
    }

    // write convert
    byteStreamWrite(stream, (unsigned char *)tmp, ID3V1_YEAR_SIZE);
    free(tmp);

    byteStreamWrite(stream, (unsigned char *)tag->comment, ID3V1_FIELD_SIZE);
    
    // track is one byte but an int can be more so clamp it
    if(tag->track <= 0xFF && tag->track > 0x00){
        byteStreamSeek(stream, -1, SEEK_CUR);
        byte = (unsigned char) tag->track & 0xFF;
        byteStreamWrite(stream, &byte, 1);
    }

    // genre is one byte as well
    byte = (unsigned char) tag->genre & 0xFF;
    byteStreamWrite(stream, &byte, 1);

    byteStreamRewind(stream);

    fp = fopen(filePath, "r+b");
    if(fp == NULL){
        
        // create a new file and write the bytes to it
        fp = fopen(filePath, "wb");
        if(fp == NULL){
            byteStreamDestroy(stream);
            return 0;
        }

        if((fwrite(byteStreamCursor(stream), 1, ID3V1_MAX_SIZE, fp)) == 0){
            byteStreamDestroy(stream);
            fclose(fp);
            return 0;
        }
        
    }else{

        // get file size
        fseek(fp, 0, SEEK_END);
        const long index = ftell(fp);
        unsigned char *buffer[ID3V1_MAX_SIZE];

        /**
         * seek to tag start
         * if the file is less than 128 bytes then we can catch that here.
         * there will be no tag due to size or if there is its corrupt and
         * will be treated as regular file data and skipped over.
         */
        if((fseek(fp, index - ID3V1_MAX_SIZE, SEEK_SET)) != 0){
            
            fseek(fp, 0, SEEK_END);
            if(fwrite(byteStreamCursor(stream), 1, ID3V1_MAX_SIZE, fp) != 0){
                byteStreamDestroy(stream);
                fclose(fp);
                return 1;
            }
            
            byteStreamDestroy(stream);
            fclose(fp);
            return 0;
        }

        // check to see if the file has ID3 metadata
        if(fread((void *)buffer, 1, ID3V1_TAG_ID_SIZE, fp) == 0){
            byteStreamDestroy(stream);
            fclose(fp);
            return 0;
        }

        // coupled not find 'TAG'
        if(!id3v1HasTag((uint8_t *)buffer)){
            if(fseek(fp, 0, SEEK_END) != 0){
                byteStreamDestroy(stream);
                fclose(fp);
                return 0;
            }

        }else{
            if(fseek(fp, index - ID3V1_MAX_SIZE, SEEK_SET) != 0){
                byteStreamDestroy(stream);
                fclose(fp);
                return 0;
            }
        }

        // write bytes to the file at the right position
        if((fwrite(byteStreamCursor(stream), 1, ID3V1_MAX_SIZE, fp)) == 0){
            byteStreamDestroy(stream);
            fclose(fp);
            return 0;
        }
    }

    byteStreamDestroy(stream);
    fclose(fp);
    return 1;
}
