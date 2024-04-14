/**
 * @file id3dev.c
 * @author Ewan Jones
 * @brief Main functions for interacting with ID3 metadata
 * @version 0.1
 * @date 2024-04-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3dev.h"
#include "id3v2/id3v2.h"
#include "id3v1/id3v1.h"
#include "id3v1/id3v1Parser.h"
#include "id3v2/id3v2Frame.h"

/**
 * @brief default standard for reading ID3 tags from a data structure representation.
 * 
 */
static uint8_t id3PreferedStandard = ID3V2_TAG_VERSION_3;

/**
 * @brief Creates a new ID3 structure with the given ID3v2 and ID3v1 tags.
 * 
 * @param id3v2 
 * @param id3v1 
 * @return ID3* 
 */
ID3 *id3Create(Id3v2Tag *id3v2, Id3v1Tag *id3v1){

    ID3 *metadata = malloc(sizeof(ID3));

    metadata->id3v2 = id3v2;
    metadata->id3v1 = id3v1;

    return metadata;

}

/**
 * @brief Destroys an ID3 structure and frees its memory.
 * 
 * @param toDelete 
 */
void id3Destroy(ID3 **toDelete){

    if(*toDelete){
        id3v2DestroyTag(&((*toDelete)->id3v2));
        id3v1DestroyTag(&((*toDelete)->id3v1));
        free(*toDelete);
        *toDelete = NULL;
        toDelete = NULL;
    }

}

/**
 * @brief Sets the prefered standard for reading ID3 tags from a data structure representation. If no standard is 
 * set the default will be set to ID3v2.3 as this is the most wide spread version with the most features. If
 * sucessful this function returns true otherwise, it will return false.
 * 
 * @param standard 
 * @return true 
 * @return false 
 */
bool id3SetPreferedStandard(uint8_t standard){

    switch(standard){
        case ID3V1_TAG_VERSION:
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            id3PreferedStandard = standard;
            break;
        
        default:
            return false;
    }

    return true;
}

/**
 * @brief Returns the current prefered standard
 * 
 * @return uint8_t 
 */
uint8_t id3GetPreferedStandard(void){
    return id3PreferedStandard;
}


/**
 * @brief Reads both ID3v1 and ID3v2 tags located in a provided file. If this function fails an
 * ID3 strcuture will be returned but one or both versions will be set to NULL.
 * 
 * @param filePath 
 * @return ID3* 
 */
ID3 *id3FromFile(const char *filePath){
    return id3Create(id3v2TagFromFile(filePath), id3v1TagFromFile(filePath));
}

/**
 * @brief Returns a copy of the given ID3 structure. If NUll is passed NULL is returned.
 * 
 * @param toCopy 
 * @return ID3* 
 */
ID3 *id3Copy(ID3 *toCopy){

    if(toCopy == NULL){
        return NULL;
    }

    return id3Create(id3v2CopyTag(toCopy->id3v2), id3v1CopyTag(toCopy->id3v1));
}

/**
 * @brief Compares tow ID3 structures. If they are the same this function will return true otherwise it will return false.
 * 
 * @param metadata1 
 * @param metadata2 
 * @return true 
 * @return false 
 */
bool id3Compare(ID3 *metadata1, ID3 *metadata2){

    if(metadata1 == NULL || metadata2 == NULL){
        return false;
    }

    bool v1 = false;
    bool v2 = false;

    v1 = id3v1CompareTag(metadata1->id3v1, metadata2->id3v1);
    v2 = id3v2CompareTag(metadata1->id3v2, metadata2->id3v2);

    if(v1 && v2){
        return true;
    }

    if(v1 == false && (v2 == true && metadata1->id3v1 == NULL && metadata2->id3v1 == NULL)){
        return true;

    }

    if(!v2 && (v1 && metadata1->id3v2 == NULL && metadata2->id3v2 == NULL)){
        return true;

    }

    return false;
}


bool id3ChangeId3v2Version(uint8_t version, ID3 *metadata){

    if(metadata == NULL || version > ID3V2_TAG_VERSION_4){
        return false;
    }

    if(metadata->id3v2 == NULL){
        return false;

    }

    Id3v2Frame *f = NULL;
    ListIter i = {0};
    List *newFrames = NULL;
    uint8_t convi = 0;

    newFrames = listCreate(id3v2PrintFrame, id3v2DeleteFrame, id3v2CompareFrame, id3v2CopyFrame);


    // what am i converting from?

    if(version == metadata->id3v2->header->majorVersion){
        return true;
    }

    




    // switch(version){
    //     case ID3V2_TAG_VERSION_2:
    //         i = id3v2CreateFrameTraverser(metadata->id3v2);

    //         while((f = id3v2FrameTraverse(&i)) != NULL){
    //             if(memcmp("BUF\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("CNT\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("COM\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("CRA\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("CRM\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("ETC\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("EQU\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("GEO\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("IPL\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("LNK\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("MCI\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("MLL\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("PIC\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("POP\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("REV\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("RVA\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("SLT\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("STC\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TAL\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TBP\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TCM\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TCO\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TCR\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TDA\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TDY\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TEN\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TFT\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TIM\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TKE\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TLA\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TLE\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TMT\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TOA\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TOF\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TOL\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TOR\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TOT\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TP1\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TP2\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TP3\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TP4\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TPA\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TPB\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TRC\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TRD\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TRK\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TSI\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TSS\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TT1\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TT2\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TT3\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TXT\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TXX\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("TYE\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("UFI\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("ULT\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("WAF\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("WAR\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("WAS\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("WCM\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("WCP\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("WPB\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //             if(memcmp("WXX\0", f->header->id, ID3V2_FRAME_ID_MAX_SIZE) == 0){}
    //         }


    //         break;
    //     case ID3V2_TAG_VERSION_3:
    //         break;
    //     case ID3V2_TAG_VERSION_4:
    //         break;
        
    //     default:
    //         return false;
    // }

    return true;
}

