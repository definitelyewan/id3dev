#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v2Context.h"
#include "id3v2Parser.h"
#include "id3v2TagIdentity.h"
#include "byteStream.h"


Id3v2ExtendedTagHeader *id3v2ParseExtendedTagHeader(ByteStream *stream, uint8_t version){

    if(!stream){
        return NULL;
    }

    // values needed
    uint32_t padding = 0;
    uint32_t crc = 0;
    bool update = false;
    bool tagRestrictions = false;
    uint8_t restrictions = 0;

    // values for parsing
    int8_t flags = 0;
    uint8_t nBytes = 0;
    bool hasCrc = false;
    bool hasRestrictions = false;

    switch(version){
        
        // crc and padding
        case ID3V2_TAG_VERSION_3:

            // read size
            if(!(byteStreamReturnInt(stream))){
                return NULL;
            }       

            // read flags
            if((flags = byteStreamReadBit(stream, 7)) == -1){ // MSB -> 7
                return NULL;
            }

            // skip over flags
            byteStreamSeek(stream, 2, SEEK_CUR);

            // read padding
            if(!(padding = byteStreamReturnInt(stream))){
                return NULL;
            }
            
            // check to see if a crc is there
            if(flags){
                // optional
                crc = byteStreamReturnInt(stream);
            }

            break; 
        case ID3V2_TAG_VERSION_4:
            
            // read size
            if(!(byteStreamReturnInt(stream))){
                return NULL;
            }  

            // number of flag bytes
            if(!(nBytes = byteStreamGetCh(stream))){
                break;
            }
            
            if(byteStreamReadBit(stream, 6)){
                update = true;
            }

            if(byteStreamReadBit(stream, 5)){
                hasCrc = true;
            }

            if(byteStreamReadBit(stream, 4)){
                hasRestrictions = true;
            }


            if(hasCrc){
                
            }

            break;

        // no extended header support
        default:
            return NULL;
    }

    return id3v2CreateExtendedTagHeader(padding, crc, update, tagRestrictions, restrictions);

}
