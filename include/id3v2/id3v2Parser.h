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
#include "byteTypes.h"

Id3v2ExtendedTagHeader *id3v2ParseExtendedTagHeader(ByteStream *stream, uint8_t version, bool unsynchronisation);




#ifdef __cplusplus
} //extern c end
#endif

#endif