 /**
 * @file id3v1Types.h
 * @author Ewan Jones
 * @brief Definitions for ID3v1 tag structure, constants, and genre enumeration
 * @version 26.01
 * @date 2023-10-02 - 2026-01-11
 *
 * @copyright Copyright (c) 2023 - 2026
 *
 */
#ifndef ID3V1_TYPES
#define ID3V1_TYPES

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

//! Size in bytes of the ID3v1 tag identifier "TAG" (3 bytes)
#define ID3V1_TAG_ID_SIZE 3

//! Size in bytes of the year field in ID3v1 tags (4 bytes)
#define ID3V1_YEAR_SIZE 4

//! Size in bytes of text fields (title, artist, album, comment) in ID3v1 tags (30 bytes)
#define ID3V1_FIELD_SIZE 30

//! Total size in bytes of a complete ID3v1/ID3v1.1 tag on disk (128 bytes)
#define ID3V1_MAX_SIZE 128

//! Major version number for ID3v1 and ID3v1.1 specification (always 1)
#define ID3V1_TAG_VERSION 1

/**
 * @brief ID3v1 genre enumeration with Winamp extensions (genres 0-191).
 * @details Complete list of standard ID3v1 genres (0-79) and Winamp extension genres (80-191). 
 * Enum values map directly to byte values stored in ID3v1 tags. OTHER_GENRE serves as the default for unknown genres.
 * @see https://en.wikipedia.org/wiki/List_of_ID3v1_Genres
 */
typedef enum _Genre {
    //! Blues Audio
    BLUES_GENRE,
    //! Classic Rock
    CLASSIC_ROCK_GENRE,
    //! Country Audio
    COUNTRY_GENRE,
    //! Dance Audio
    DANCE_GENRE,
    //! Disco Audio
    DISCO_GENRE,
    //! Funk Audio
    FUNK_GENRE,
    //! Grunge Audio
    GRUNGE_GENRE,
    //! Hip-Hop Audio
    HIP_HOP_GENRE,
    //! Jazz Audio
    JAZZ_GENRE,
    //! Metal Audio
    METAL_GENRE,
    //! New Age Audio
    NEW_AGE_GENRE,
    //! Oldies Music, pre-1950s
    OLDIES_GENRE,
    //! Other Music Genres not Defined
    OTHER_GENRE,
    //! Pop Audio
    POP_GENRE,
    //! Rythem and Blues Audio
    RHYTHM_AND_BLUES_GENRE,
    //! Rap Audio
    RAP_GENRE,
    //! Reggae Audio
    REGGAE_GENRE,
    //! Rock Audio
    ROCK_GENRE,
    //! Techno Audio
    TECHNO_GENRE,
    //! Industrial Audio
    INDUSTRIAL_GENRE,
    //! Alternative Audio
    ALTERNATIVE_GENRE,
    //! Ska Audio
    SKA_GENRE,
    //! Death Metal Audio
    DEATH_METAL_GENRE,
    //! Pranks Audio
    PRANKS_GENRE,
    //! Soundtracks and OSTs from other media
    SOUNDTRACK_GENRE,
    //! European Techno Audio
    EURO_TECHNO_GENRE,
    //! Ambient Audio
    AMBIENT_GENRE,
    //! Trip Audio
    TRIP_HOP_GENRE,
    //! Vocal Audio
    VOCAL_GENRE,
    //! Jazz and Funk Audio
    JAZZ_AND_FUNK_GENRE,
    //! Fusion of Genres
    FUSION_GENRE,
    //! Trance Audio
    TRANCE_GENRE,
    //! Classical Audio
    CLASSICAL_GENRE,
    //! Instrumental Audio
    INSTRUMENTAL_GENRE,
    //! Acid Audio
    ACID_GENRE,
    //! House Audio
    HOUSE_GENRE,
    //! Video Game Audio
    GAME_GENRE,
    //! Sound Clip Audio
    SOUND_CLIP_GENRE,
    //! Gospel Audio
    GOSPEL_GENRE,
    //! Noise Audio
    NOISE_GENRE,
    //! Alternative Rock Audio
    ALTERNATIVE_ROCK_GENRE,
    //! Bass Audio
    BASS_GENRE,
    //! Soul Audio
    SOUL_GENRE,
    //! Punk Audio
    PUNK_GENRE,
    //! Space Audio
    SPACE_GENRE,
    //! Meditative Audio
    MEDITATIVE_GENRE,
    //! Instrumental Pop Audio
    INSTRUMENTAL_POP_GENRE,
    //! Instrumental Rock Audio
    INSTRUMENTAL_ROCK_GENRE,
    //! Ethnic Audio
    ETHNIC_GENRE,
    //! Gothic Audio
    GOTHIC_GENRE,
    //! Darkwave Audio
    DARKWAVE_GENRE,
    //! Techno Industrial Audio
    TECHNO_INDUSTRIAL_GENRE,
    //! Electronic Audio
    ELECTRONIC_GENRE,
    //! Pop Folk Audio
    POP_FOLK_GENRE,
    //! European Dance Audio
    EURODANCE_GENRE,
    //! Dream Audio
    DREAM_GENRE,
    //! Southern Rock Audio
    SOUTHERN_ROCK_GENRE,
    //! Comedy Audio
    COMEDY_GENRE,
    //! Cult Audio
    CULT_GENRE,
    //! Gangsta Audio
    GANGSTA_GENRE,
    //! Top 40 Chart Hits
    TOP_40_GENRE,
    //! Christian Rap Audio
    CHRISTIAN_RAP_GENRE,
    //! Funk Pop Audio
    POP_FUNK_GENRE,
    //! Jungle Audio
    JUNGLE_MUSIC_GENRE,
    //! Native Audio
    NATIVE_US_GENRE,
    //! Cabaret Audio
    CABARET_GENRE,
    //! New Wave Audio
    NEW_WAVE_GENRE,
    //! Psychedelic Audio
    PSYCHEDELIC_GENRE,
    //! Rave Audio
    RAVE_GENRE,
    //! Theatre Show Tunes Audio
    SHOWTUNES_GENRE,
    //! Audio From Media Trailers
    TRAILER_GENRE,
    //! Lo-Fi Audio
    LO_FI_GENRE,
    //! Tribal Audio
    TRIBAL_GENRE,
    //! Acid Punk Audio
    ACID_PUNK_GENRE,
    //! Acid Jazz Audio
    ACID_JAZZ_GENRE,
    //! Polka Audio
    POLKA_GENRE,
    //! Retro Audio
    RETRO_GENRE,
    //! Audio from Musicals
    MUSICAL_GENRE,
    //! Rock and Roll Audio
    ROCKNROLL_GENRE,
    //! Hard Rock Audio
    HARD_ROCK_GENRE,
    //! Folk Audio
    FOLK_GENRE,
    //! Folk Rock Audio
    FOLK_ROCK_GENRE,
    //! National Folk Audio
    NATIONAL_FOLK_GENRE,
    //! Swing Audio
    SWING_GENRE,
    //! Fast Fusion Audio
    FAST_FUSION_GENRE,
    //! Bebop Audio
    BEBOP_GENRE,
    //! Latin Audio
    LATIN_GENRE,
    //! Revival Audio
    REVIVAL_GENRE,
    //! Celtic Audio
    CELTIC_GENRE,
    //! Blue Grass Audio
    BLUEGRASS_GENRE,
    //! Avantgarde Audio
    AVANTGARDE_GENRE,
    //! Gothic Rock Audio
    GOTHIC_ROCK_GENRE,
    //! Progressive Audio
    PROGRESSIVE_ROCK_GENRE,
    //! Psychedelic Audio
    PSYCHEDELIC_ROCK_GENRE,
    //! Symphonic Rock Audio
    SYMPHONIC_ROCK_GENRE,
    //! Slow Rock Audio
    SLOW_ROCK_GENRE,
    //! Big Band Audio
    BIG_BAND_GENRE,
    //! Chorus Audio
    CHORUS_GENRE,
    //! Easy Listening Audio
    EASY_LISTENING_GENRE,
    //! Acoustic Audio
    ACOUSTIC_GENRE,
    //! Humour Audio
    HUMOUR_GENRE,
    //! Speech Audio
    SPEECH_GENRE,
    //! Chanson Audio
    CHANSON_GENRE,
    //! Opera Audio
    OPERA_GENRE,
    //! Chamber Audio
    CHAMBER_MUSIC_GENRE,
    //! Sonata Audio
    SONATA_GENRE,
    //! Symphony Audio
    SYMPHONY_GENRE,
    //! Booty Bass Audio
    BOOTY_BASS_GENRE,
    //! Primus Audio
    PRIMUS_GENRE,
    //! Porn Groove Audio
    PORN_GROOVE_GENRE,
    //! Satire Audio
    SATIRE_GENRE,
    //! Slow Jam Audio
    SLOW_JAM_GENRE,
    //! Club Audio
    CLUB_GENRE,
    //! Tangp Audio
    TANGO_GENRE,
    //! Samba Audio
    SAMBA_GENRE,
    //! Folklore Audio
    FOLKLORE_GENRE,
    //! Ballad Audio
    BALLAD_GENRE,
    //! Power Ballad Audio
    POWER_BALLAD_GENRE,
    //! Rhythmic Soul Audio
    RHYTHMIC_SOUL_GENRE,
    //! Free Style Audio
    FREESTYLE_GENRE,
    //! Duet Audio
    DUET_GENRE,
    //! Punk Rock Audio
    PUNK_ROCK_GENRE,
    //! Drum Solo Audio
    DRUM_SOLO_GENRE,
    //! Acappella Audio
    A_CAPPELLA_GENRE,
    //! European House Audio
    EURO_HOUSE_GENRE,
    //! Dance Hall Audio
    DANCE_HALL_GENRE,
    //! GOA Audio
    GOA_MUSIC_GENRE,
    //! Drum and Bass Audio
    DRUM_AND_BASS_GENRE,
    //! Club House Audio
    CLUB_HOUSE_GENRE,
    //! Hardcore Techno Audio
    HARDCORE_TECHNO_GENRE,
    //! Terror Audio
    TERROR_GENRE,
    //! Indie Audio
    INDIE_GENRE,
    //! Britpop Audio
    BRITPOP_GENRE,
    //! Neger Punk Audio
    NEGERPUNK_GENRE,
    //! Polsk Punk Audio
    POLSK_PUNK_GENRE,
    //! Beat Audio
    BEAT_GENRE,
    //! Christian Gangsta Rap Audio
    CHRISTIAN_GANGSTA_RAP_GENRE,
    //! Heavy Metal Audio
    HEAVY_METAL_GENRE,
    //! Black Metal Audio
    BLACK_METAL_GENRE,
    //! Crossover Audio
    CROSSOVER_GENRE,
    //! Contemporary Christian Audio
    CONTEMPORARY_CHRISTIAN_GENRE,
    //! Christian Rock Audio
    CHRISTIAN_ROCK_GENRE,
    //! Merengue Audio
    MERENGUE_GENRE,
    //! Salsa Audio
    SALSA_GENRE,
    //! Thrash Metal Audio
    THRASH_METAL_GENRE,
    //! Anime Audio
    ANIME_GENRE,
    //! Japanese Pop Audio
    JPOP_GENRE,
    //! Synth Pop Audio
    SYNTHPOP_GENRE,
    //! Christmas Audio
    CHRISTMAS_GENRE,
    //! Abstract Audio
    ABSTRACT_GENRE,
    //! Art Rock Audio
    ART_ROCK_GENRE,
    //! Baroque Audio
    BAROQUE_GENRE,
    //! Bhangra Audio
    BHANGRA_GENRE,
    //! Big Beat Audio
    BIG_BEAT_GENRE,
    //! Beark Beat Audio
    BREAKBEAT_GENRE,
    //! Chillout Audio
    CHILLOUT_GENRE,
    //! Down tempo Audio
    DOWNTEMPO_GENRE,
    //! Dub Audio
    DUB_GENRE,
    //! EBM Audio
    EBM_GENRE,
    //! Eclectic Audio
    ECLECTIC_GENRE,
    //! Electro Audio
    ELECTRO_GENRE,
    //! Electro Clash Audio
    ELECTROCLASH_GENRE,
    //! EMO Audio
    EMO_GENRE,
    //! Experimental Audio
    EXPERIMENTAL_GENRE,
    //! Garage Audio
    GARAGE_GENRE,
    //! Global Audio
    GLOBAL_GENRE,
    //! IDM Audio
    IDM_GENRE,
    //! Illbient Audio
    ILLBIENT_GENRE,
    //! Industro Goth Audio
    INDUSTRO_GOTH_GENRE,
    //! Jam Band Audio
    JAM_BAND_GENRE,
    //! Krautrock Audio
    KRAUTROCK_GENRE,
    //! LeftField Audio
    LEFTFIELD_GENRE,
    //! Lounge Audio
    LOUNGE_GENRE,
    //! Math Rock Audio
    MATH_ROCK_GENRE,
    //! New Romantic Audio
    NEW_ROMANTIC_GENRE,
    //! Nu-Breakz Audio
    NU_BREAKZ_GENRE,
    //! Post Punk Audio
    POST_PUNK_GENRE,
    //! Post Rock Audio
    POST_ROCK_GENRE,
    //! Psytrance Audio
    PSYTRANCE_GENRE,
    //! Shoegaze Audio
    SHOEGAZE_GENRE,
    //! Space Rock Audio
    SPACE_ROCK_GENRE,
    //! Trop Rock Audio
    TROP_ROCK_GENRE,
    //! World Music Audio
    WORLD_MUSIC_GENRE,
    //! Neo Classical Audio
    NEOCLASSICAL_GENRE,
    //! Audio Book
    AUDIOBOOK_GENRE,
    //! Audio Theatre
    AUDIO_THEATRE_GENRE,
    //! Neue Deutsche Welle Audio
    NEUE_DEUTSCHE_WELLE_GENRE,
    //! Podcast Audio
    PODCAST_GENRE,
    //! Indie Rock Audio
    INDIE_ROCK_GENRE,
    //! G-Funk Audio
    G_FUNK_GENRE,
    //! Dubstep Audio
    DUBSTEP_GENRE,
    //! Garage Rock Audio
    GARAGE_ROCK_GENRE,
    //! Psybient Audio
    PSYBIENT_GENRE
} Genre;

/**
 * @brief ID3v1/ID3v1.1 tag structure containing all metadata fields.
 * @details Holds fixed-size string buffers (ID3V1_FIELD_SIZE bytes each) for title, artist, album, 
 * and comment, plus numeric values for year, track number, and genre byte. Represents 
 * both ID3v1 and ID3v1.1 formats.
 */
typedef struct _Id3v1Tag {
    //! Song title (ID3V1_FIELD_SIZE bytes, null-padded)
    uint8_t title[ID3V1_FIELD_SIZE];

    //! Artist name (ID3V1_FIELD_SIZE bytes, null-padded)
    uint8_t artist[ID3V1_FIELD_SIZE];

    //! Album title (ID3V1_FIELD_SIZE bytes, null-padded)
    uint8_t albumTitle[ID3V1_FIELD_SIZE];

    //! Release year (4-digit integer)
    int year;

    //! Track number (0 if not present, ID3v1.1 only)
    int track;

    //! Comment text (ID3V1_FIELD_SIZE bytes, null-padded; ID3V1_FIELD_SIZE - 2 bytes if track number present)
    uint8_t comment[ID3V1_FIELD_SIZE];

    //! Genre value (0-191, maps to Genre enum)
    Genre genre;
} Id3v1Tag;

#ifdef __cplusplus
} // extern c end
#endif

#endif
