/**
 * @file id3dev.h
 * @author Ewan Jones
 * @brief decleations of main library functions
 * @version 0.1
 * @date 2024-01-19
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef ID3DEV
#define ID3DEV

#ifdef __cplusplus
extern "C"{
#endif

#include "id3v1.h"
#include "id3v2.h"


typedef struct _ID3{

    Id3v1Tag *id3v1Tag;

    List *id3v2tags;

}ID3;




#ifdef __cplusplus
} //extern c end
#endif

#endif