# ID3dev
ID3dev is a library crafted to support all versions of the ID3 standard including ID3v1, ID3v1.1, ID3v2.2, ID3v2.3, and ID3v2.4. It is versatile and programmable allowing users to generate custom metadata that better suits their needs.

## Table Of Contents
- [Table Of Contents](#table-of-contents)
- 




- [What Can It Do?](#what-can-it-do-)
  * [Frame Support](#frame-support)
- [Building and Installing](#building-and-installing)
  * [Building Using GNU Make in UNIX Systems](#building-using-gnu-make-in-unix-systems)
  * [Building Using CMake](#building-using-cmake)
- [Usage](#usage)
- [API](#api)
  * [File Functions](#file-functions)
  * [Tag Functions](#tag-functions)
    + [Getter Functions](#getter-functions)
    + [Setter Functions](#setter-functions)
    + [Delete Functions](#delete-functions)
- [Examples](#examples)
    + [Load Tags](#load-tags)
    + [Edit Tags](#edit-tags)
    + [Delete Tags](#delete-tags)
- [Extending Functionality](#extending-functionality)
    + [Read New Frames](#read-new-frames)
    + [Edit New Frames](#edit-new-frames)
- [Compatibility Layer](#compatibility-layer)
- [Projects](#projects)
- [Copyright](#copyright)
- [Questions?](#questions-)

## What Can It Do?
ID3dev is capable of reading, manipulating, converting, and outputting all supported frames found within the ID3v2.x specifications with only a couple of exceptions when it comes to features (see below table). Additionally, it supports all previously mentioned features for ID3v1.x with the ability to dynamically switch between ID3v2.x and ID3v1.x standards.

### Frame Support
Keep in mind that not all frames are supported in every version.
✅ = full support, 🟡 = partial support, ❌ = no support.

| Frame | ID3v1 | ID3v1.1 | ID3v2.2 ID | ID3v2.3 ID | ID3v2.4 ID | Library Support |
|-------|-------|---------|------------|------------|------------|-----------------|
| Audio encryption                  |       |         | CRA        | AENC       | AENC       | ✅               |
| Attached picture                  |       |         | PIC        | APIC       | APIC       | ✅               |
| Seek point index                  |       |         |            |            | ASPI       | ✅               |
| Comments                          | Yes   | Yes     | COM        | COMM       | COMM       | ✅               |
| Commercial                        |       |         |            | COMR       | COMR       | ✅               |
| Encryption method registration    |       |         |            | ENCR       | ENCR       | ✅               |
| Equalization                      |       |         | EQU        | EQU2       | EQU2       | ✅               |
| Event timing codes                |       |         | ETC        | ETCO       | ETCO       | ✅               |
| General encapsulated object       |       |         | GEO        | GEOB       | GEOB       | ✅               |
| Group identification registration |       |         |            | GRID       | GRID       | ✅               |

|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|
|


```bash
                                                |2.4 |2.3 |2.2|SUPPORT?
Linked information                              |LINK|LINK|LNK|X|
Music CD identifier                             |MCDI|MCDI|MCI|X|
MPEG location lookup table                      |MLLT|MLLT|MLL|-|
Ownership frame                                 |OWNE|OWNE|   |X|
Private frame                                   |PRIV|PRIV|   |X|
Play counter                                    |PCNT|PCNT|CNT|X|
Popularimeter                                   |POPM|POPM|POP|X|
Position synchronisation frame                  |POSS|POSS|   |X|
Recommended buffer size                         |RBUF|RBUF|BUF|X|
Relative volume adjustment                      |RVA2|RVAD|RVA|-|
Reverb                                          |RVRB|RVRB|REV|X|
Seek frame                                      |SEEK|    |   |X|
Signature frame                                 |SIGN|    |   |X|
Synchronised lyric/text                         |SYLT|SYLT|SLT|X|
Synchronised tempo codes                        |SYTC|SYTC|STC|-|
Album/Movie/Show title                          |TALB|TALB|TAL|X|
BPM (beats per minute)                          |TBPM|TBPM|TBP|X|
Composer                                        |TCOM|TCOM|TCM|X|
Content type                                    |TCON|TCON|TCO|X|
Copyright message                               |TCOP|TCOP|TCR|X|
Encoding time                                   |TDEN|    |   |X|
Playlist delay                                  |TDLY|TDLY|TDY|X|
Original release time                           |TDOR|    |   |X|
Recording time                                  |TDRC|    |   |X|
Release time                                    |TDRL|    |   |X|
Tagging time                                    |TDTG|    |   |X|
Encoded by                                      |TENC|TENC|TEN|X|
Lyricist/Text writer                            |TEXT|TEXT|TXT|X|
FIle type                                       |TFLT|TFLT|TFT|X|
Involved people list                            |TIPL|IPLS|IPL|X|
Content group description                       |TIT1|TIT1|TT1|X|
Title/songname/content description              |TIT2|TIT2|TT2|X|
Subtitle/Description refinement                 |TIT3|TIT3|TT3|X|
Initial key                                     |TKEY|TKEY|TKE|X|
Language(s)                                     |TLAN|TLAN|TLA|X|
Length                                          |TLEN|TLEN|TLE|X|
Musician credits list                           |TMCL|    |   |X|
Media type                                      |TMED|TMED|TMT|X|
Mood                                            |TMOO|    |   |X|
Original album/movie/show title                 |TOAL|TOAL|TOT|X|
Original filename                               |TOFN|TOFN|TOF|X|
Original lyricist(s)/text writer(s)             |TOLY|TOLY|TOL|X|
Original artist(s)/performer(s)                 |TOPE|TOPE|TOA|X|
File owner/licensee                             |TOWN|TOWN|   |X|
Lead performer(s)/Soloist(s)                    |TPE1|TPE1|TP1|X|
Band/orchestra/accompaniment                    |TPE2|TPE2|TP2|X|
Conductor/performer refinement                  |TPE3|TPE3|TP3|X|
Interpreted, remixed, or otherwise modified by  |TPE4|TPE4|TP4|X|
Part of a set                                   |TPOS|TPOS|TPA|X|
Produced notice                                 |TPRO|    |   |X|
Publisher                                       |TPUB|TPUB|TPB|X|
Track number/Position in set                    |TRCK|TRCK|TRK|X|
Internet radio station name                     |TRSN|TRSN|   |X|
Internet radio station owner                    |TRSO|TRSO|   |X|
Album sort order                                |TSOA|    |   |X|
Performer sort order                            |TSOP|    |   |X|
Title sort order                                |TSOT|    |   |X|
ISRC (international standard recording code)    |TSRC|TSRC|TRC|X|
Software/Hardware and settings used for encoding|TSSE|TSSE|TSS|X|
Set subtitle                                    |TSST|    |   |X|
User defined text information frame             |TXXX|TXXX|TXX|X|
Unique file identifier                          |UFID|UFID|UFI|X|
Terms of use                                    |USER|USER|   |X|
Unsynchronised lyric/text transcription         |USLT|USLT|ULT|X|
Commercial information                          |WCOM|WCOM|WCM|X|
Copyright/Legal information                     |WCOP|WCOP|WCP|X|
Official audio file webpage                     |WOAF|WOAF|WAF|X|
Official artist/performer webpage               |WOAR|WOAR|WAR|X|
Official audio source webpage                   |WOAS|WOAS|WAS|X|
Official Internet radio station homepage        |WORS|WORS|   |X|
Payment                                         |WPAY|WPAY|   |X|
Publishers official webpage                     |WPUB|WPUB|WPB|X|
User defined URL link frame                     |WXXX|WXXX|WXX|X|
Date                                            |    |TDAT|TDA|X|
Time                                            |    |TIME|TIM|X|
Original release year                           |    |TORY|TOR|X|
Recording dates                                 |    |TRDA|TRD|X|
Size                                            |    |TSIZ|TSI|X|
Year                                            |    |TYER|TYE|X|
Encrypted meta frame                            |    |    |CRM|X|
```