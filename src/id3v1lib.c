#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v1.h"


Id3v1Tag *Id3v1TagFromFile(const char* filePath){
    
    FILE *fp = NULL;
    unsigned char id3Bytes[ID3V1_MAX_BYTES];
    
    //make sure the file can really be read
    if((fp = fopen(filePath, "rb")) == NULL){
        return NULL;
    }

    //seek to the start of metadata
    if((fseek(fp, -ID3V1_MAX_BYTES, SEEK_END)) != 0){
        fclose(fp);
        return NULL;
    }

    if((fread(id3Bytes, ID3V1_MAX_BYTES, 1, fp)) != 1){
        fclose(fp);
        return NULL;
    }

    fclose(fp);    
    return id3v1TagFromBuffer(id3Bytes);  
}

Id3v1Tag *id3v1TagFromBuffer(unsigned char *buffer){

    int trackno = 0;
    unsigned char *holdTitle = NULL;
    unsigned char *holdArtist = NULL;
    unsigned char *holdAlbum = NULL;
    unsigned char *holdComment = NULL;
    unsigned char year[ID3V1_YEAR_LEN];
    Genre genre;
    Id3Reader *stream = id3NewReader(buffer, ID3V1_MAX_BYTES);

    if(strncmp((char *)id3ReaderCursor(stream), "TAG", ID3V1_ID_LEN)){
        id3FreeReader(stream);
        return NULL;
    }
    id3ReaderSeek(stream, ID3V1_ID_LEN, SEEK_CUR);
    
    //get song title and set index for next tag
    holdTitle = calloc(sizeof(unsigned char), ID3V1_TAG_LEN + 1);    
    id3ReaderRead(stream, holdTitle, ID3V1_TAG_LEN);
    
    //get artist and set index for next tag
    holdArtist = calloc(sizeof(unsigned char), ID3V1_TAG_LEN + 1);
    id3ReaderRead(stream, holdArtist, ID3V1_TAG_LEN);


    //get album title and set index for next tag
    holdAlbum = calloc(sizeof(unsigned char), ID3V1_TAG_LEN + 1);
    id3ReaderRead(stream, holdAlbum, ID3V1_TAG_LEN);

    //get year and set index for next tag
    strncpy((char *)year, (char *)id3ReaderCursor(stream), ID3V1_YEAR_LEN);
    year[4] = '\0';
    id3ReaderSeek(stream, ID3V1_YEAR_LEN, SEEK_CUR);

    //check for a track number, ID3V1.1 has the 28th bit nulled so that the 29th can be a track number
    id3ReaderSeek(stream, ID3V1_TAG_LEN - 2, SEEK_CUR);
    if(!id3ReaderCursor(stream)[0] && id3ReaderCursor(stream)[1]){
        trackno = 1;
    }
    id3ReaderSeek(stream, -(ID3V1_TAG_LEN - 2), SEEK_CUR);

    //get comment and set index for next tag
    holdComment = calloc(sizeof(unsigned char), ID3V1_TAG_LEN + 1);
    id3ReaderRead(stream, holdComment, ID3V1_TAG_LEN - trackno);

    //read and set track number + move index
    if(trackno){
        trackno = (int)id3ReaderGetCh(stream);
    }else{
        trackno = 0;
    }
    
    id3ReaderSeek(stream, 1, SEEK_CUR);
    genre = id3ReaderGetCh(stream);

    id3FreeReader(stream);

    return id3v1NewTag(holdTitle, holdArtist, holdAlbum, atoi((char *)year), trackno, holdComment, genre);
}

Id3v1Tag *id3v1NewTag(unsigned char *title, unsigned char *artist, unsigned char *albumTitle, int year, int trackNumber, unsigned char *comment, Genre genre){

    Id3v1Tag *tag = malloc(sizeof(Id3v1Tag));

    tag->title = title;
    tag->artist = artist;
    tag->albumTitle = albumTitle;
    tag->year = year;
    tag->trackNumber = trackNumber;
    tag->comment = comment;
    tag->genre = genre;

    return tag;
}

char *genreFromTable(Genre val){

    //int to string
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
        return "Instrumrntal\0";
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
        return "Other\0";
    }
    //191 supported genres god this enum is huge
    return "Other\0";
}

bool containsId3v1(unsigned char *buffer){
    return (memcmp(buffer,"TAG",ID3V1_ID_LEN) == 0) ? true: false;
}


void id3v1FreeTag(void *toDelete){
    
    Id3v1Tag *toFree = (Id3v1Tag *)toDelete;

    //error checking until frees 
    if(toFree){
        if(toFree->albumTitle){
            free(toFree->albumTitle);
        }
        if(toFree->artist){
            free(toFree->artist);
        }
        if(toFree->title){
            free(toFree->title);
        }
        if(toFree->comment){
            free(toFree->comment);
        }
        
        free(toFree);
    }
}

//set family functions are pretty much all the same
/*
void setTitleId3v1(char *title, Id3v1 *tag){
    if(!tag){
        return;
    }
    if(title == NULL || strlen(title) > ID3V1_TAGLENGTH){
        return;
    }

    if(tag->title){
        free(tag->title);
    }
    
    char *newTitle = calloc(ID3V1_TAGLENGTH, sizeof(char));
    strcpy(newTitle,title);
    newTitle[ID3V1_TAGLENGTH-1] = '\0';

    tag->title = newTitle;
}

void setArtistId3v1(char *artist, Id3v1 *tag){
    if(!tag){
        return;
    }
    if(artist == NULL || strlen(artist) > ID3V1_TAGLENGTH){
        return;
    }

    if(tag->artist){
        free(tag->artist);
    }
    
    char *newArtist = calloc(ID3V1_TAGLENGTH, sizeof(char));
    strcpy(newArtist,artist);
    newArtist[ID3V1_TAGLENGTH-1] = '\0';

    tag->artist = newArtist;
}

void setAlbumId3v1(char *album, Id3v1 *tag){
    if(!tag){
        return;
    }
    if(album == NULL || strlen(album) > ID3V1_TAGLENGTH){
        return;
    }

    if(tag->albumTitle){
        free(tag->albumTitle);
    }
    
    char *newAlbum = calloc(ID3V1_TAGLENGTH, sizeof(char));
    strcpy(newAlbum,album);
    newAlbum[ID3V1_TAGLENGTH-1] = '\0';

    tag->albumTitle = newAlbum;    
}

void setYearId3v1(int year, Id3v1 *tag){
    if(!tag){
        return;
    }
    //restriction for 4 bytes when char* array happens
    if(year > 9999 || year < 0){
        return;
    }

    tag->year = year;
}

void setCommentId3v1(char *comment, Id3v1 *tag){
    if(!tag){
        return;
    }
    if(comment == NULL || strlen(comment) > ID3V1_TAGLENGTH){
        return;
    }

    if(tag->comment){
        free(tag->comment);
    }
    
    char *newComment = calloc(ID3V1_TAGLENGTH,sizeof(char));
    strcpy(newComment,comment);
    newComment[ID3V1_TAGLENGTH-1] = '\0';

    tag->comment = newComment;  
}

void setGenreId3v1(Genre genre, Id3v1 *tag){
    if(!tag){
        return;
    }

    tag->genre = genre;
}

void setTrackId3v1(int trackNumber, Id3v1 *tag){
    if(!tag){
        return;
    }

    //so the track byte is only 8bits 255
    if(trackNumber > 255 || trackNumber < 0){
        return;
    }

    tag->trackNumber = trackNumber;
}


void writeId3v1(char *filePath, Id3v1 *tag){
    if(!tag){
        return;
    }

    if(filePath == NULL){
        return;
    }
    
    FILE *fp = NULL;
    char isTag[4];

    if((fp = fopen(filePath, "r+b")) == NULL){
        return;
    }

    //file size
    if(fseek(fp ,0L, SEEK_END)){
        fclose(fp);
        return;
    }
    int index = ftell(fp);
    rewind(fp);

    if((fseek(fp, index - ID3V1_MAXBYTES, SEEK_SET)) != 0){
        fclose(fp);
        return;
    }

    //check to see if the file thats being written to has ID3 metadata
    if((fread(isTag, 3, 1, fp)) != 1){
        fclose(fp);
        return;
    }

    if(strncmp("TAG", isTag, 3)){

        //open file in append mode
        fclose(fp);
        if((fp = fopen(filePath, "a+b")) == 0){
            return; 
        }

        if((fseek(fp, index, SEEK_SET)) != 0){
            fclose(fp);
            return;
        }
    }
    
    //write TAG onto new file so its marked as ID3 then write tags
    if((fwrite("TAG", sizeof(char),sizeof(char) * 3, fp)) == 0){
        fclose(fp);
        return;
    }
    
    if((fwrite(tag->title, 1, sizeof(char) * ID3V1_TAGLENGTH, fp)) == 0){
        fclose(fp);
        return;
    }
    
    if((fwrite(tag->artist, 1, sizeof(char) * ID3V1_TAGLENGTH, fp)) == 0){
        fclose(fp);
        return;
    }
    
    if((fwrite(tag->albumTitle, 1, sizeof(char) * ID3V1_TAGLENGTH, fp)) == 0){
        fclose(fp);
        return;
    }

    //year conversion
    int n = log10(tag->year) + 1;
    int i;
    char *numberArray = calloc(n, sizeof(char));
    for (i = n-1; i >= 0; --i, tag->year /= 10)
    {
        numberArray[i] = (tag->year % 10) + '0';
    }

    if((fwrite(numberArray, 1, sizeof(char) * 4, fp)) == 0){
        fclose(fp);
        free(numberArray);
        return;
    }
    free(numberArray);

    int trackno = 0;

    if(tag->trackNumber > 0 && tag->trackNumber <= 255){
        trackno = 2;
    }

    if((fwrite(tag->comment, 1,sizeof(char) * (ID3V1_TAGLENGTH - trackno), fp)) == 0){
        fclose(fp);
        return;
    }

    if(trackno){
        fputc(0,fp);
        fputc(tag->trackNumber, fp);
    }

    if((fwrite(&tag->genre, 1, sizeof(char), fp)) == 0){
        fclose(fp);
        return;
    }
    
    fclose(fp);
}
*/




