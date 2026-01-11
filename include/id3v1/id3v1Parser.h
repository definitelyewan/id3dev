 /**
 * @file id3v1Parser.c
 * @author Ewan Jones
 * @brief Declarations of ID3v1 tag parsing and memory management functions
 * @version 26.01
 * @date 2023-10-03 - 2026-01-11
 * 
 * @copyright Copyright (c) 2023 - 2026
 * 
 */
#ifndef ID3V1_PARSER
#define ID3V1_PARSER

#ifdef __cplusplus
extern "C"{
#endif

#include "id3v1Types.h"

//mem

bool id3v1HasTag(const uint8_t *buffer);

Id3v1Tag *id3v1CreateTag(uint8_t *title, uint8_t *artist, uint8_t *albumTitle, int year, int track, uint8_t *comment,
                         Genre genre);

void id3v1ClearTag(Id3v1Tag *tag);

void id3v1DestroyTag(Id3v1Tag **toDelete);

//parse
Id3v1Tag *id3v1TagFromBuffer(uint8_t *buffer);

#ifdef __cplusplus
} //extern c end
#endif

#endif
