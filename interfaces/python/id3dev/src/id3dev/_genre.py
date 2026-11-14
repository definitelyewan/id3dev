"""! @brief Genre: the ID3v1 genre list (id3v1Types.h Genre), usable for any tag."""

from enum import IntEnum

from ._id3dev import ffi, lib


class Genre(IntEnum):
    """! @brief An ID3v1 genre: the 80 standard genres plus the Winamp extensions.

    Values come from the compiled Genre enum in id3v1Types.h, so they are the
    numbers ID3v1 stores. Assign one to ID3.genre: an ID3v1 tag stores the
    number, an ID3v2 tag stores the genre's name (Genre.text).

    @code{.py}
    from id3dev import Genre

    tag.genre = Genre.POP
    Genre.HIP_HOP.text            # "Hip-Hop"
    Genre.from_text("hip-hop")    # Genre.HIP_HOP
    @endcode
    """

    ## Blues (0)
    BLUES = lib.BLUES_GENRE
    ## Classic Rock (1)
    CLASSIC_ROCK = lib.CLASSIC_ROCK_GENRE
    ## Country (2)
    COUNTRY = lib.COUNTRY_GENRE
    ## Dance (3)
    DANCE = lib.DANCE_GENRE
    ## Disco (4)
    DISCO = lib.DISCO_GENRE
    ## Funk (5)
    FUNK = lib.FUNK_GENRE
    ## Grunge (6)
    GRUNGE = lib.GRUNGE_GENRE
    ## Hip-Hop (7)
    HIP_HOP = lib.HIP_HOP_GENRE
    ## Jazz (8)
    JAZZ = lib.JAZZ_GENRE
    ## Metal (9)
    METAL = lib.METAL_GENRE
    ## New Age (10)
    NEW_AGE = lib.NEW_AGE_GENRE
    ## Oldies (11)
    OLDIES = lib.OLDIES_GENRE
    ## Other (12)
    OTHER = lib.OTHER_GENRE
    ## Pop (13)
    POP = lib.POP_GENRE
    ## Rhythm and Blues (14)
    RHYTHM_AND_BLUES = lib.RHYTHM_AND_BLUES_GENRE
    ## Rap (15)
    RAP = lib.RAP_GENRE
    ## Reggae (16)
    REGGAE = lib.REGGAE_GENRE
    ## Rock (17)
    ROCK = lib.ROCK_GENRE
    ## Techno (18)
    TECHNO = lib.TECHNO_GENRE
    ## Industrial (19)
    INDUSTRIAL = lib.INDUSTRIAL_GENRE
    ## Alternative (20)
    ALTERNATIVE = lib.ALTERNATIVE_GENRE
    ## Ska (21)
    SKA = lib.SKA_GENRE
    ## Death Metal (22)
    DEATH_METAL = lib.DEATH_METAL_GENRE
    ## Pranks (23)
    PRANKS = lib.PRANKS_GENRE
    ## Soundtrack (24)
    SOUNDTRACK = lib.SOUNDTRACK_GENRE
    ## Euro-Techno (25)
    EURO_TECHNO = lib.EURO_TECHNO_GENRE
    ## Ambient (26)
    AMBIENT = lib.AMBIENT_GENRE
    ## Trip-Hop (27)
    TRIP_HOP = lib.TRIP_HOP_GENRE
    ## Vocal (28)
    VOCAL = lib.VOCAL_GENRE
    ## Jazz and Funk (29)
    JAZZ_AND_FUNK = lib.JAZZ_AND_FUNK_GENRE
    ## Fusion (30)
    FUSION = lib.FUSION_GENRE
    ## Trance (31)
    TRANCE = lib.TRANCE_GENRE
    ## Classical (32)
    CLASSICAL = lib.CLASSICAL_GENRE
    ## Instrumental (33)
    INSTRUMENTAL = lib.INSTRUMENTAL_GENRE
    ## Acid (34)
    ACID = lib.ACID_GENRE
    ## House (35)
    HOUSE = lib.HOUSE_GENRE
    ## Game (36)
    GAME = lib.GAME_GENRE
    ## Sound Clip (37)
    SOUND_CLIP = lib.SOUND_CLIP_GENRE
    ## Gospel (38)
    GOSPEL = lib.GOSPEL_GENRE
    ## Noise (39)
    NOISE = lib.NOISE_GENRE
    ## Alternative Rock (40)
    ALTERNATIVE_ROCK = lib.ALTERNATIVE_ROCK_GENRE
    ## Bass (41)
    BASS = lib.BASS_GENRE
    ## Soul (42)
    SOUL = lib.SOUL_GENRE
    ## Punk (43)
    PUNK = lib.PUNK_GENRE
    ## Space (44)
    SPACE = lib.SPACE_GENRE
    ## Meditative (45)
    MEDITATIVE = lib.MEDITATIVE_GENRE
    ## Instrumental Pop (46)
    INSTRUMENTAL_POP = lib.INSTRUMENTAL_POP_GENRE
    ## Instrumental Rock (47)
    INSTRUMENTAL_ROCK = lib.INSTRUMENTAL_ROCK_GENRE
    ## Ethnic (48)
    ETHNIC = lib.ETHNIC_GENRE
    ## Gothic (49)
    GOTHIC = lib.GOTHIC_GENRE
    ## Darkwave (50)
    DARKWAVE = lib.DARKWAVE_GENRE
    ## Techno Industrial (51)
    TECHNO_INDUSTRIAL = lib.TECHNO_INDUSTRIAL_GENRE
    ## Electronic (52)
    ELECTRONIC = lib.ELECTRONIC_GENRE
    ## Pop Folk (53)
    POP_FOLK = lib.POP_FOLK_GENRE
    ## Eurodance (54)
    EURODANCE = lib.EURODANCE_GENRE
    ## Dream (55)
    DREAM = lib.DREAM_GENRE
    ## Southern Rock (56)
    SOUTHERN_ROCK = lib.SOUTHERN_ROCK_GENRE
    ## Comedy (57)
    COMEDY = lib.COMEDY_GENRE
    ## Cult (58)
    CULT = lib.CULT_GENRE
    ## Gangsta (59)
    GANGSTA = lib.GANGSTA_GENRE
    ## Top 40 (60)
    TOP_40 = lib.TOP_40_GENRE
    ## Christian Rap (61)
    CHRISTIAN_RAP = lib.CHRISTIAN_RAP_GENRE
    ## Pop Funk (62)
    POP_FUNK = lib.POP_FUNK_GENRE
    ## Jungle Music (63)
    JUNGLE_MUSIC = lib.JUNGLE_MUSIC_GENRE
    ## Native US (64)
    NATIVE_US = lib.NATIVE_US_GENRE
    ## Cabaret (65)
    CABARET = lib.CABARET_GENRE
    ## New Wave (66)
    NEW_WAVE = lib.NEW_WAVE_GENRE
    ## Psychedelic (67)
    PSYCHEDELIC = lib.PSYCHEDELIC_GENRE
    ## Rave (68)
    RAVE = lib.RAVE_GENRE
    ## Showtunes (69)
    SHOWTUNES = lib.SHOWTUNES_GENRE
    ## Trailer (70)
    TRAILER = lib.TRAILER_GENRE
    ## Lo-Fi (71)
    LO_FI = lib.LO_FI_GENRE
    ## Tribal (72)
    TRIBAL = lib.TRIBAL_GENRE
    ## Acid Punk (73)
    ACID_PUNK = lib.ACID_PUNK_GENRE
    ## Acid Jazz (74)
    ACID_JAZZ = lib.ACID_JAZZ_GENRE
    ## Polka (75)
    POLKA = lib.POLKA_GENRE
    ## Retro (76)
    RETRO = lib.RETRO_GENRE
    ## Musical (77)
    MUSICAL = lib.MUSICAL_GENRE
    ## Rock and Roll (78)
    ROCKNROLL = lib.ROCKNROLL_GENRE
    ## Hard Rock (79)
    HARD_ROCK = lib.HARD_ROCK_GENRE
    ## Folk (80)
    FOLK = lib.FOLK_GENRE
    ## Folk Rock (81)
    FOLK_ROCK = lib.FOLK_ROCK_GENRE
    ## National Folk (82)
    NATIONAL_FOLK = lib.NATIONAL_FOLK_GENRE
    ## Swing (83)
    SWING = lib.SWING_GENRE
    ## Fast Fusion (84)
    FAST_FUSION = lib.FAST_FUSION_GENRE
    ## Bebop (85)
    BEBOP = lib.BEBOP_GENRE
    ## Latin (86)
    LATIN = lib.LATIN_GENRE
    ## Revival (87)
    REVIVAL = lib.REVIVAL_GENRE
    ## Celtic (88)
    CELTIC = lib.CELTIC_GENRE
    ## Bluegrass (89)
    BLUEGRASS = lib.BLUEGRASS_GENRE
    ## Avantgarde (90)
    AVANTGARDE = lib.AVANTGARDE_GENRE
    ## Gothic Rock (91)
    GOTHIC_ROCK = lib.GOTHIC_ROCK_GENRE
    ## Progressive Rock (92)
    PROGRESSIVE_ROCK = lib.PROGRESSIVE_ROCK_GENRE
    ## Psychedelic Rock (93)
    PSYCHEDELIC_ROCK = lib.PSYCHEDELIC_ROCK_GENRE
    ## Symphonic Rock (94)
    SYMPHONIC_ROCK = lib.SYMPHONIC_ROCK_GENRE
    ## Slow Rock (95)
    SLOW_ROCK = lib.SLOW_ROCK_GENRE
    ## Big Band (96)
    BIG_BAND = lib.BIG_BAND_GENRE
    ## Chorus (97)
    CHORUS = lib.CHORUS_GENRE
    ## Easy Listening (98)
    EASY_LISTENING = lib.EASY_LISTENING_GENRE
    ## Acoustic (99)
    ACOUSTIC = lib.ACOUSTIC_GENRE
    ## Humour (100)
    HUMOUR = lib.HUMOUR_GENRE
    ## Speech (101)
    SPEECH = lib.SPEECH_GENRE
    ## Chanson (102)
    CHANSON = lib.CHANSON_GENRE
    ## Opera (103)
    OPERA = lib.OPERA_GENRE
    ## Chamber Music (104)
    CHAMBER_MUSIC = lib.CHAMBER_MUSIC_GENRE
    ## Sonata (105)
    SONATA = lib.SONATA_GENRE
    ## Symphony (106)
    SYMPHONY = lib.SYMPHONY_GENRE
    ## Booty Bass (107)
    BOOTY_BASS = lib.BOOTY_BASS_GENRE
    ## Primus (108)
    PRIMUS = lib.PRIMUS_GENRE
    ## Porn Groove (109)
    PORN_GROOVE = lib.PORN_GROOVE_GENRE
    ## Satire (110)
    SATIRE = lib.SATIRE_GENRE
    ## Slow Jam (111)
    SLOW_JAM = lib.SLOW_JAM_GENRE
    ## Club (112)
    CLUB = lib.CLUB_GENRE
    ## Tango (113)
    TANGO = lib.TANGO_GENRE
    ## Samba (114)
    SAMBA = lib.SAMBA_GENRE
    ## Folklore (115)
    FOLKLORE = lib.FOLKLORE_GENRE
    ## Ballad (116)
    BALLAD = lib.BALLAD_GENRE
    ## Power Ballad (117)
    POWER_BALLAD = lib.POWER_BALLAD_GENRE
    ## Rhythmic Soul (118)
    RHYTHMIC_SOUL = lib.RHYTHMIC_SOUL_GENRE
    ## Freestyle (119)
    FREESTYLE = lib.FREESTYLE_GENRE
    ## Duet (120)
    DUET = lib.DUET_GENRE
    ## Punk Rock (121)
    PUNK_ROCK = lib.PUNK_ROCK_GENRE
    ## Drum Solo (122)
    DRUM_SOLO = lib.DRUM_SOLO_GENRE
    ## A Cappella (123)
    A_CAPPELLA = lib.A_CAPPELLA_GENRE
    ## Euro-House (124)
    EURO_HOUSE = lib.EURO_HOUSE_GENRE
    ## Dance Hall (125)
    DANCE_HALL = lib.DANCE_HALL_GENRE
    ## Goa Music (126)
    GOA_MUSIC = lib.GOA_MUSIC_GENRE
    ## Drum and Bass (127)
    DRUM_AND_BASS = lib.DRUM_AND_BASS_GENRE
    ## Club-House (128)
    CLUB_HOUSE = lib.CLUB_HOUSE_GENRE
    ## Hardcore Techno (129)
    HARDCORE_TECHNO = lib.HARDCORE_TECHNO_GENRE
    ## Terror (130)
    TERROR = lib.TERROR_GENRE
    ## Indie (131)
    INDIE = lib.INDIE_GENRE
    ## Britpop (132)
    BRITPOP = lib.BRITPOP_GENRE
    ## Negerpunk (133)
    NEGERPUNK = lib.NEGERPUNK_GENRE
    ## Polsk Punk (134)
    POLSK_PUNK = lib.POLSK_PUNK_GENRE
    ## Beat (135)
    BEAT = lib.BEAT_GENRE
    ## Christian Gangsta Rap (136)
    CHRISTIAN_GANGSTA_RAP = lib.CHRISTIAN_GANGSTA_RAP_GENRE
    ## Heavy Metal (137)
    HEAVY_METAL = lib.HEAVY_METAL_GENRE
    ## Black Metal (138)
    BLACK_METAL = lib.BLACK_METAL_GENRE
    ## Crossover (139)
    CROSSOVER = lib.CROSSOVER_GENRE
    ## Contemporary Christian (140)
    CONTEMPORARY_CHRISTIAN = lib.CONTEMPORARY_CHRISTIAN_GENRE
    ## Christian Rock (141)
    CHRISTIAN_ROCK = lib.CHRISTIAN_ROCK_GENRE
    ## Merengue (142)
    MERENGUE = lib.MERENGUE_GENRE
    ## Salsa (143)
    SALSA = lib.SALSA_GENRE
    ## Thrash Metal (144)
    THRASH_METAL = lib.THRASH_METAL_GENRE
    ## Anime (145)
    ANIME = lib.ANIME_GENRE
    ## Jpop (146)
    JPOP = lib.JPOP_GENRE
    ## Synthpop (147)
    SYNTHPOP = lib.SYNTHPOP_GENRE
    ## Christmas (148)
    CHRISTMAS = lib.CHRISTMAS_GENRE
    ## Abstract (149)
    ABSTRACT = lib.ABSTRACT_GENRE
    ## Art Rock (150)
    ART_ROCK = lib.ART_ROCK_GENRE
    ## Baroque (151)
    BAROQUE = lib.BAROQUE_GENRE
    ## Bhangra (152)
    BHANGRA = lib.BHANGRA_GENRE
    ## Big Beat (153)
    BIG_BEAT = lib.BIG_BEAT_GENRE
    ## Breakbeat (154)
    BREAKBEAT = lib.BREAKBEAT_GENRE
    ## Chillout (155)
    CHILLOUT = lib.CHILLOUT_GENRE
    ## Downtempo (156)
    DOWNTEMPO = lib.DOWNTEMPO_GENRE
    ## Dub (157)
    DUB = lib.DUB_GENRE
    ## EBM (158)
    EBM = lib.EBM_GENRE
    ## Eclectic (159)
    ECLECTIC = lib.ECLECTIC_GENRE
    ## Electro (160)
    ELECTRO = lib.ELECTRO_GENRE
    ## Electroclash (161)
    ELECTROCLASH = lib.ELECTROCLASH_GENRE
    ## Emo (162)
    EMO = lib.EMO_GENRE
    ## Experimental (163)
    EXPERIMENTAL = lib.EXPERIMENTAL_GENRE
    ## Garage (164)
    GARAGE = lib.GARAGE_GENRE
    ## Global (165)
    GLOBAL = lib.GLOBAL_GENRE
    ## IDM (166)
    IDM = lib.IDM_GENRE
    ## Illbient (167)
    ILLBIENT = lib.ILLBIENT_GENRE
    ## Industro-Goth (168)
    INDUSTRO_GOTH = lib.INDUSTRO_GOTH_GENRE
    ## Jam Band (169)
    JAM_BAND = lib.JAM_BAND_GENRE
    ## Krautrock (170)
    KRAUTROCK = lib.KRAUTROCK_GENRE
    ## Leftfield (171)
    LEFTFIELD = lib.LEFTFIELD_GENRE
    ## Lounge (172)
    LOUNGE = lib.LOUNGE_GENRE
    ## Math Rock (173)
    MATH_ROCK = lib.MATH_ROCK_GENRE
    ## New Romantic (174)
    NEW_ROMANTIC = lib.NEW_ROMANTIC_GENRE
    ## Nu-Breakz (175)
    NU_BREAKZ = lib.NU_BREAKZ_GENRE
    ## Post-Punk (176)
    POST_PUNK = lib.POST_PUNK_GENRE
    ## Post-Rock (177)
    POST_ROCK = lib.POST_ROCK_GENRE
    ## Psytrance (178)
    PSYTRANCE = lib.PSYTRANCE_GENRE
    ## Shoegaze (179)
    SHOEGAZE = lib.SHOEGAZE_GENRE
    ## Space Rock (180)
    SPACE_ROCK = lib.SPACE_ROCK_GENRE
    ## Trop Rock (181)
    TROP_ROCK = lib.TROP_ROCK_GENRE
    ## World Music (182)
    WORLD_MUSIC = lib.WORLD_MUSIC_GENRE
    ## Neoclassical (183)
    NEOCLASSICAL = lib.NEOCLASSICAL_GENRE
    ## Audiobook (184)
    AUDIOBOOK = lib.AUDIOBOOK_GENRE
    ## Audio Theatre (185)
    AUDIO_THEATRE = lib.AUDIO_THEATRE_GENRE
    ## Neue Deutsche Welle (186)
    NEUE_DEUTSCHE_WELLE = lib.NEUE_DEUTSCHE_WELLE_GENRE
    ## Podcast (187)
    PODCAST = lib.PODCAST_GENRE
    ## Indie-Rock (188)
    INDIE_ROCK = lib.INDIE_ROCK_GENRE
    ## G-Funk (189)
    G_FUNK = lib.G_FUNK_GENRE
    ## Dubstep (190)
    DUBSTEP = lib.DUBSTEP_GENRE
    ## Garage Rock (191)
    GARAGE_ROCK = lib.GARAGE_ROCK_GENRE
    ## Psybient (192)
    PSYBIENT = lib.PSYBIENT_GENRE

    @property
    def text(self) -> str:
        """! @brief The genre's display name from id3dev's table, e.g. "Hip-Hop" (id3v1GenreFromTable)."""
        return ffi.string(lib.id3v1GenreFromTable(self)).decode("latin-1")

    @classmethod
    def from_text(cls, text: str) -> "Genre":
        """! @brief Finds a genre by its display name, ignoring case.
        @param text  a name such as "Pop" or "hip-hop"
        @return the matching Genre
        @throws ValueError if no ID3v1 genre has that name
        """
        wanted = text.strip().casefold()
        for genre in cls:
            if genre.text.casefold() == wanted:
                return genre
        raise ValueError(f"{text!r} is not an ID3v1 genre")
