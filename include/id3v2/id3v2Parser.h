/**
 * @file id3v2Parser.h
 * @author Ewan Jones
 * @brief functions that make up the parser
 * @version 0.1
 * @date 2024-01-19
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef ID3V2_PARSER
#define ID3V2_PARSER

#ifdef __cplusplus
extern "C"{
#endif


#include "id3v2Types.h"


uint32_t id3v2ParseExtendedTagHeader(uint8_t *in, size_t inl, uint8_t version,
                                     Id3v2ExtendedTagHeader **extendedTagHeader);

uint32_t id3v2ParseTagHeader(uint8_t *in, size_t inl, Id3v2TagHeader **tagHeader, uint32_t *tagSize);

uint32_t id3v2ParseFrameHeader(uint8_t *in, size_t inl, uint8_t version, Id3v2FrameHeader **frameHeader,
                               uint32_t *frameSize);

uint32_t id3v2ParseFrame(uint8_t *in, size_t inl, List *context, uint8_t version, Id3v2Frame **frame);

Id3v2Tag *id3v2ParseTagFromBuffer(uint8_t *in, size_t inl, HashTable *userPairs);

#ifdef __cplusplus
} //extern c end
#endif

#endif
