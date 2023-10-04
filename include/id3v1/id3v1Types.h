/**
 * @file id3v1Types.h
 * @author Ewan Jones
 * @brief Defintions and types for id3v1 types
 * @version 2.0
 * @date 2023-10-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef ID3V1_TYPES
#define ID3V1_TYPES

#ifdef __cplusplus
extern "C"{
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdint.h>


//! The size of the id3v1 and id3v1.1 tag identifier "TAG"
#define ID3V1_TAG_ID_SIZE 3

//! Number of bytes allocated to the year in a id3v1 and id3v1.1 tag
#define ID3V1_YEAR_SIZE 4

//! Number of bytes allocated to all text fields in a id3v1 and id3v1.1 tag
#define ID3V1_FIELD_SIZE 30

//! Max id3v1 and id3v1.1 tag size
#define ID3V1_MAX_SIZE 128

//https://en.wikipedia.org/wiki/List_of_ID3v1_Genres
//! All supported genres with winamp extension
typedef enum _Genre{
    BLUES_GENRE,              
    CLASSIC_ROCK_GENRE,      
    COUNTRY_GENRE,          
    DANCE_GENRE,          
    DISCO_GENRE,           
    FUNK_GENRE,            
    GRUNGE_GENRE,           
    HIP_HOP_GENRE,         
    JAZZ_GENRE,           
    METAL_GENRE,      
    NEW_AGE_GENRE,          
    OLDIES_GENRE,      
    OTHER_GENRE,       
    POP_GENRE,           
    RHYTHM_AND_BLUES_GENRE,  
    RAP_GENRE,              
    REGGAE_GENRE,        
    ROCK_GENRE,            
    TECHNO_GENRE,         
    INDUSTRIAL_GENRE,       
    ALTERNATIVE_GENRE,        
    SKA_GENRE,               
    DEATH_METAL_GENRE,        
    PRANKS_GENRE,
    SOUNDTRACK_GENRE,
    EURO_TECHNO_GENRE,
    AMBIENT_GENRE,
    TRIP_HOP_GENRE,
    VOCAL_GENRE,
    JAZZ_AND_FUNK_GENRE,
    FUSION_GENRE,
    TRANCE_GENRE,
    CLASSICAL_GENRE,
    INSTRUMENTAL_GENRE,
    ACID_GENRE,
    HOUSE_GENRE,
    GAME_GENRE,
    SOUND_CLIP_GENRE,
    GOSPEL_GENRE,
    NOISE_GENRE,
    ALTERNATIVE_ROCK_GENRE,
    BASS_GENRE,
    SOUL_GENRE,
    PUNK_GENRE,
    SPACE_GENRE,
    MEDITATIVE_GENRE,
    INSTRUMENTAL_POP_GENRE,
    INSTRUMENTAL_ROCK_GENRE,
    ETHNIC_GENRE,
    GOTHIC_GENRE,
    DARKWAVE_GENRE,
    TECHNO_INDUSTRIAL_GENRE,
    ELECTRONIC_GENRE,
    POP_FOLK_GENRE,
    EURODANCE_GENRE,
    DREAM_GENRE,
    SOUTHERN_ROCK_GENRE,
    COMEDY_GENRE,
    CULT_GENRE,
    GANGSTA_GENRE,
    TOP_40_GENRE,
    CHRISTIAN_RAP_GENRE,
    POP_FUNK_GENRE,
    JUNGLE_MUSIC_GENRE,
    NATIVE_US_GENRE,
    CABARET_GENRE,
    NEW_WAVE_GENRE,
    PSYCHEDELIC_GENRE,
    RAVE_GENRE,
    SHOWTUNES_GENRE,
    TRAILER_GENRE,
    LO_FI_GENRE,
    TRIBAL_GENRE,
    ACID_PUNK_GENRE,
    ACID_JAZZ_GENRE,
    POLKA_GENRE,
    RETRO_GENRE,
    MUSICAL_GENRE,
    ROCKNROLL_GENRE,
    HARD_ROCK_GENRE,
    FOLK_GENRE,
    FOLK_ROCK_GENRE,
    NATIONAL_FOLK_GENRE,
    SWING_GENRE,
    FAST_FUSION_GENRE,
    BEBOP_GENRE,
    LATIN_GENRE,
    REVIVAL_GENRE,
    CELTIC_GENRE,
    BLUEGRASS_GENRE,
    AVANTGARDE_GENRE,
    GOTHIC_ROCK_GENRE,
    PROGRESSIVE_ROCK_GENRE,
    PSYCHEDELIC_ROCK_GENRE,
    SYMPHONIC_ROCK_GENRE,
    SLOW_ROCK_GENRE,
    BIG_BAND_GENRE,
    CHORUS_GENRE,
    EASY_LISTENING_GENRE,
    ACOUSTIC_GENRE,
    HUMOUR_GENRE,
    SPEECH_GENRE,
    CHANSON_GENRE,
    OPERA_GENRE,
    CHAMBER_MUSIC_GENRE,
    SONATA_GENRE,
    SYMPHONY_GENRE,
    BOOTY_BASS_GENRE,
    PRIMUS_GENRE,
    PORN_GROOVE_GENRE,
    SATIRE_GENRE,
    SLOW_JAM_GENRE,
    CLUB_GENRE,
    TANGO_GENRE,
    SAMBA_GENRE,
    FOLKLORE_GENRE,
    BALLAD_GENRE,
    POWER_BALLAD_GENRE,
    RHYTHMIC_SOUL_GENRE,
    FREESTYLE_GENRE,
    DUET_GENRE,
    PUNK_ROCK_GENRE,
    DRUM_SOLO_GENRE,
    A_CAPPELLA_GENRE,
    EURO_HOUSE_GENRE,
    DANCE_HALL_GENRE,
    GOA_MUSIC_GENRE,
    DRUM_AND_BASS_GENRE,
    CLUB_HOUSE_GENRE,
    HARDCORE_TECHNO_GENRE,
    TERROR_GENRE,
    INDIE_GENRE,
    BRITPOP_GENRE,
    NEGERPUNK_GENRE,
    POLSK_PUNK_GENRE,
    BEAT_GENRE,
    CHRISTIAN_GANGSTA_RAP_GENRE,
    HEAVY_METAL_GENRE,
    BLACK_METAL_GENRE,
    CROSSOVER_GENRE,
    CONTEMPORARY_CHRISTIAN_GENRE,
    CHRISTIAN_ROCK_GENRE,
    MERENGUE_GENRE,
    SALSA_GENRE,
    THRASH_METAL_GENRE,
    ANIME_GENRE,
    JPOP_GENRE,
    SYNTHPOP_GENRE,
    ABSTRACT_GENRE,
    ART_ROCK_GENRE,
    BAROQUE_GENRE,
    BHANGRA_GENRE,
    BIG_BEAT_GENRE,
    BREAKBEAT_GENRE,
    CHILLOUT_GENRE,
    DOWNTEMPO_GENRE,
    DUB_GENRE,
    EBM_GENRE,
    ECLECTIC_GENRE,
    ELECTRO_GENRE,
    ELECTROCLASH_GENRE,
    EMO_GENRE,
    EXPERIMENTAL_GENRE,
    GARAGE_GENRE,
    GLOBAL_GENRE,
    IDM_GENRE,
    ILLBIENT_GENRE,
    INDUSTRO_GOTH_GENRE,
    JAM_BAND_GENRE,
    KRAUTROCK_GENRE,
    LEFTFIELD_GENRE,
    LOUNGE_GENRE,
    MATH_ROCK_GENRE,
    NEW_ROMANTIC_GENRE,
    NU_BREAKZ_GENRE,
    POST_PUNK_GENRE,
    POST_ROCK_GENRE,
    PSYTRANCE_GENRE,
    SHOEGAZE_GENRE,
    SPACE_ROCK_GENRE,
    TROP_ROCK_GENRE,
    WORLD_MUSIC_GENRE,
    NEOCLASSICAL_GENRE,
    AUDIOBOOK_GENRE,
    AUDIO_THEATRE_GENRE,
    NEUE_DEUTSCHE_WELLE_GENRE,
    PODCAST_GENRE,
    INDIE_ROCK_GENRE,
    G_FUNK_GENRE,
    DUBSTEP_GENRE,
    GARAGE_ROCK_GENRE,
    PSYBIENT_GENRE

}Genre;


/**
 * @brief Main structure to hold all id3v1 values and buffers parsed from a file or generated by a caller.
 * 
 */
typedef struct _Id3v1Tag{

    //! Title buffer for a tag
    uint8_t title[ID3V1_FIELD_SIZE];

    //! Artist buffer for a tag
    uint8_t artist[ID3V1_FIELD_SIZE];
    
    //! Album buffer for a tag
    uint8_t albumTitle[ID3V1_FIELD_SIZE];
    
    //! Year value
    int year;

    //! Track value
    int track;

    //! Comment buffer for a tag
    uint8_t comment[ID3V1_FIELD_SIZE];
    
    //! Genere value
    Genre genre;

}Id3v1Tag;


#ifdef __cplusplus
} //extern c end
#endif

#endif