# ID3dev
ID3dev is a library crafted to support all versions of the ID3 standard including ID3v1, ID3v1.1, ID3v2.2, ID3v2.3, and ID3v2.4. It is versatile and programmable allowing users to generate custom metadata that better suits their needs.

## Table Of Contents
- [Table Of Contents](#table-of-contents)
- [What Can It Do?](#what-can-it-do)
    * [Frame Support](#frame-support)
- [Cloning](#cloning)
- [Building](#building)
- [Installing](#installing)
    * [Linux Systems](#linux-systems)
    * [Mac Systems](#mac-systems)
    * [Windows Systems](#windows-systems)
- [Usage](#usage)
- [Documentation](#documentation)
    + [Standards](#standards)
    + [Doxygen](#doxygen)
    + [Examples](#examples)
    + [Tests](#tests)

## What Can It Do?
ID3dev is capable of reading, manipulating, converting, and outputting all supported frames found within the ID3v2.x specifications with only a couple of exceptions when it comes to features (see below table). Additionally, it supports all previously mentioned features for ID3v1.x with the ability to dynamically switch between ID3v2.x and ID3v1.x standards.

### Frame Support
Keep in mind that not all frames are supported in every version.
✅ = full support, 🟡 = partial support, ❌ = no support.

| Frame | ID3v1 | ID3v1.1 | ID3v2.2 ID | ID3v2.3 ID | ID3v2.4 ID | Library Support |
|-------|-------|---------|------------|------------|------------|-----------------|
| Audio encryption                                 |     |     | CRA | AENC | AENC | ✅ |
| Attached picture                                 |     |     | PIC | APIC | APIC | ✅ |
| Seek point index                                 |     |     |     |      | ASPI | ✅ |
| Comments                                         | Yes | Yes | COM | COMM | COMM | ✅ |
| Commercial                                       |     |     |     | COMR | COMR | ✅ |
| Encryption method registration                   |     |     |     | ENCR | ENCR | ✅ |
| Equalization                                     |     |     | EQU | EQUA | EQU2 | ✅ |
| Event timing codes                               |     |     | ETC | ETCO | ETCO | ✅ |
| General encapsulated object                      |     |     | GEO | GEOB | GEOB | ✅ |
| Group identification registration                |     |     |     | GRID | GRID | ✅ |
| Linked information                               |     |     | LNK | LINK | LINK | ✅ |
| Music CD identifier                              |     |     | MCI | MCDI | MCDI | ✅ |
| MPEG location lookup table                       |     |     | MLL | MLLT | MLLT | 🟡 |
| Ownership                                        |     |     |     | OWNE | OWNE | ✅ |
| Private                                          |     |     |     | PRIV | PRIV | ✅ |
| Play counter                                     |     |     | CNT | PCNT | PCNT | ✅ |
| Popularimeter                                    |     |     | POP | POPM | POPM | ✅ |
| Position synchronisation frame                   |     |     |     | POSS | POSS | ✅ |
| Recommended buffer size                          |     |     | BUF | RBUF | RBUF | ✅ |
| Relative volume adjustment                       |     |     | RVA | RVAD | RVA2 | 🟡 |
| Reverb                                           |     |     | REV | RVRB | RVRB | ✅ |
| Seek                                             |     |     |     |      | SEEK | ✅ |
| Signature                                        |     |     |     |      | SIGN | ✅ |
| Synchronised lyric/text                          |     |     | SLT | SYLT | SYLT | ✅ |
| Synchronised tempo codes                         |     |     | STC | SYTC | SYTC | 🟡 |
| Album/Movie/Show title                           | Yes | Yes | TAL | TALB | TALB | ✅ |
| BPM (beats per minute)                           |     |     | TBP | TBPM | TBPM | ✅ |
| Composer                                         |     |     | TCM | TCOM | TCOM | ✅ |
| Content type                                     | Yes | Yes | TCO | TCON | TCON | ✅ |
| Copyright message                                |     |     | TCR | TCOP | TCOP | ✅ |
| Encoding time                                    |     |     |     |      | TDEN | ✅ |
| Playlist delay                                   |     |     | TDY | TDLY | TDLY | ✅ |
| Original release time                            |     |     |     |      | TDOR | ✅ |
| Recording time                                   |     |     |     |      | TDRC | ✅ |
| Release time                                     |     |     |     |      | TDRL | ✅ |
| Tagging time                                     |     |     |     |      | TDTG | ✅ |
| Encoded by                                       |     |     | TEN | TENC | TENC | ✅ |
| Lyricist/Text writer                             |     |     | TXT | TEXT | TEXT | ✅ |
| File type                                        |     |     | TFT | TFLT | TFLT | ✅ |
| Involved people list                             |     |     | IPL | IPLS | TIPL | ✅ |
| Content group description                        | Yes | Yes | TT1 | TIT1 | TIT1 | ✅ |
| Title/songname/content description               |     |     | TT2 | TIT2 | TIT2 | ✅ |
| Subtitle/Description refinement                  |     |     | TT3 | TIT3 | TIT3 | ✅ |
| Initial key                                      |     |     | TKE | TKEY | TKEY | ✅ |
| Language(s)                                      |     |     | TLA | TLAN | TLAN | ✅ |
| Length                                           |     |     | TLE | TLEN | TLEN | ✅ |
| Musician credits list                            |     |     |     |      | TMCL | ✅ |
| Media type                                       |     |     | TMT | TMED | TMED | ✅ |
| Mood                                             |     |     |     |      | TMOO | ✅ |
| Original album/movie/show title                  |     |     | TOT | TOAL | TOAL | ✅ |
| Original filename                                |     |     | TOF | TOFN | TOFN | ✅ |
| Original lyricist(s)/text writer(s)              |     |     | TOL | TOLY | TOLY | ✅ |
| Original artist(s)/performer(s)                  |     |     | TOA | TOPE | TOPE | ✅ |
| File owner/licensee                              |     |     |     | TOWN | TOWN | ✅ |
| Lead performer(s)/Soloist(s)                     | Yes | Yes | TP1 | TPE1 | TPE1 | ✅ |
| Band/orchestra/accompaniment                     |     |     | TP2 | TPE2 | TPE2 | ✅ |
| Conductor/performer refinement                   |     |     | TP3 | TPE3 | TPE3 | ✅ |
| Interpreted, remixed, or otherwise modified by   |     |     | TP4 | TPE4 | TPE4 | ✅ |
| Part of a set                                    |     |     | TPA | TPOS | TPOS | ✅ |
| Produced notice                                  |     |     |     |      | TPRO | ✅ |
| Publisher                                        |     |     | TPB | TPUB | TPUB | ✅ |
| Track number/Position in set                     |     |     | TRK | TRCK | TRCK | ✅ |
| Internet radio station name                      |     |     |     | TRSN | TRSN | ✅ |
| Internet radio station owner                     |     |     |     | TRSO | TRSO | ✅ |
| Album sort order                                 |     |     |     |      | TSOA | ✅ |
| Performer sort order                             |     |     |     |      | TSOP | ✅ |
| Title sort order                                 |     |     |     |      | TSOT | ✅ |
| ISRC (international standard recording code)     |     |     | TRC | TSRC | TSRC | ✅ |
| Software/Hardware and settings used for encoding |     |     | TSS | TSSE | TSSE | ✅ |
| Set subtitle                                     |     |     |     |      | TSST | ✅ |
| User defined text information                    |     |     | TXX | TXXX | TXXX | ✅ |
| Unique file identifier                           |     |     | UFI | UFID | UFID | ✅ |
| Terms of use                                     |     |     |     | USER | USER | ✅ |
| Unsynchronised lyric/text transcription          |     |     | ULT | USLT | USLT | ✅ |
| Commercial information                           |     |     | WCM | WCOM | WCOM | ✅ |
| Copyright/Legal information                      |     |     | WCP | WCOP | WCOP | ✅ |
| Official audio file webpage                      |     |     | WAF | WOAF | WOAF | ✅ |
| Official artist/performer webpage                |     |     | WAR | WOAR | WOAR | ✅ |
| Official audio source webpage                    |     |     | WAS | WOAS | WOAS | ✅ |
| Official Internet radio station homepage         |     |     |     | WORS | WORS | ✅ |
| Payment                                          |     |     |     | WPAY | WPAY | ✅ |
| Publishers official webpage                      |     |     | WPB | WPUB | WPUB | ✅ |
| User defined URL link frame                      |     |     | WXX | WXXX | WXXX | ✅ |
| Date                                             |     |     | TDA | TDAT |      | ✅ |
| Time                                             |     |     | TIM | TIME |      | ✅ |
| Original release year                            |     |     | TOR | TORY |      | ✅ | 
| Recording dates                                  |     |     | TRD | TRDA |      | ✅ | 
| Size                                             |     |     | TSI | TSIZ |      | ✅ |
| Year                                             | Yes | Yes | TYE | TYER |      | ✅ |
| Encrypted meta frame                             |     |     | CRM |      |      | ✅ |

## Cloning
ID3dev utilizes four distinct C libraries to operate. These libraries include LinkedListLib, HashTableLib, ByteStream, and cmocka. ByteStream is essential for handling binary data and converting it into C data types. LinkedListLib facilitates the addition of a generic doubly linked list, while HashTableLib enables the addition of a generic dynamic hash table. The sole optional library is cmocka, which is exclusively necessary for testing purposes and can be disregarded otherwise.

To download this project and all of it's dependencies run :
```bash
git clone --recurse-submodules https://github.com/definitelyewan/id3dev.git
```

## Building
This project comes with a Python3 script intended to be used to build it. This script is located at `./scripts/build.py` and should be run from the root directory of this project. However, if you wish to build this project via standard cmake to gain access to extra build options you can.
build options:
- `BUILD_SHARED_LIBS` can be toggled on for a shared library and off for a static one
- `BUILD_ID3_DOCS` can be toggled to generate documentation
- `BUILD_ID3_TESTS` can be toggled to generate unit tests
- `DEBUG_ID3_SYMBOLS` can be toggled to include or not include debug symbols
- `BUILD_ID3_C_EXAMPLES` can  be toggled to create example C programs

To build ID3dev with these options you will need to use CMake along with the below commands (OS dependant).
```bash
$ mkdir build
$ cmake -S . -B build
$ cd build
$ make
``` 
Do note that this project and these scripts only work with GCC, Clang, MinGW, and MSVC and have not been tested with other compilers.

## Installing
> Ensure all commands are run from the root of the cloned project directory.
> The below commands assume you built ID3dev via the `build.py` file described in the above section
### Linux Systems
To install this library system wide you can do the following:
```bash
cp ./build/libid3dev.so /usr/lib
```
```bash
cp -r ./include/* /usr/include/
rsync -avm --include='*.h' --include='*/' --exclude='cmocka/' --exclude='*' ./id3dependencies /usr/include
```

### Mac Systems
To install this library system wide you can do the following:
```bash
cp ./build/libid3dev.dylib /usr/lib
```
```bash
cp -r ./include/* /usr/include/
rsync -avm --include='*.h' --include='*/' --exclude='cmocka/' --exclude='*' ./id3dependencies /usr/include
```

### Windows Systems
Instead of installing ID3dev system wide on Windows its recommended that you just install it along side a single project. You can copy this script to a `.ps1` file and run it to install alongside a project.
```powershell
Copy-Item -Path .\build\libid3dev.dll -Destination "your project dir"
```
```powershell
Copy-Item -Path .\include\* -Destination "your project dir" -Recurse -Force

Get-ChildItem -Path .\id3dependencies -Recurse -Include *.h | Where-Object { $_.FullName -notmatch 'cmocka' } | ForEach-Object {
    $destination = $_.FullName.Replace(".\id3dependencies", "your project dir")
    $destinationDir = Split-Path -Path $destination -Parent
    if (!(Test-Path -Path $destinationDir)) {
        New-Item -ItemType Directory -Path $destinationDir | Out-Null
    }
    Copy-Item -Path $_.FullName -Destination $destination
}
```
## Usage
Depending on the feature set you wish to use you may want to include different headers
- `id3v1.h` contains functions exclusive to ID3v1 and ID3v1.1 tags
- `id3v2.h` contains functions exclusive to ID3v2.2, ID3v2.3, and ID3v2.4
- `id3dev.h` contains functions for editing any tag version


```C
#include <id3dev.h>
// #include <id3v1/id3v1.h> only ID3v1.x
// #include <id3v2/id3v2.h> only ID3v2.x

int main(int argc, char *argv[]){
    // code

    return 0;
}

```

## Documentation
ID3dev is fully documented! all documentation can be viewed online via Github Pages at https://definitelyewan.github.io/id3dev/ . Alternativly, it can be built from source for offline use with the below command `python3 ./scripts/docs.py` and opened in any web browser from the build folder however, you may want to move it.

### Standards
ID3 source documentation is included with this project and located in `docs/` in a text and html format. 

### Doxygen
Full ID3dev function documenation is located online at https://definitelyewan.github.io/id3dev/ or can be built form source via `python3 ./scripts/docs.py`

### Examples
A couple examples on how to use this project are located in the `example` folder and can be built using `python3 ./scripts/examples`

### Tests
There are well over 100 test cases located in the test folder for proof of functionallity. to build and run these tests cmocka must be installed on your system for `python3 ./scripts/test.py`.

