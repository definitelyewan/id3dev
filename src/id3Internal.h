/**
 * @file id3Internal.h
 * @author Ewan Jones
 * @brief Helpers shared between id3dev's source files, implemented in id3Internal.c. Not part of the public API
 * and not installed.
 * @version 26.01
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef ID3_INTERNAL
#define ID3_INTERNAL

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Escapes a UTF-8 string for use as a JSON string value.
 * @details Escapes quotes, backslashes and control characters below 0x20 as JSON requires. Bytes of 0x20 and
 * above, including multi-byte UTF-8 sequences, are copied unchanged.
 *
 * @param str - NUL terminated UTF-8 string
 *
 * @return char* - Heap allocated escaped string. Caller must free. NULL if str is NULL or allocation fails
 */
char *internal_id3JsonEscape(const char *str);

/**
 * @brief Reads a big-endian unsigned value from an entry's bytes.
 * @details Used to read the bit count stored in a field keyed "adjustment". Only the trailing sizeof(size_t)
 * bytes are used, which is far beyond any bit count the standard allows.
 *
 * @param data - Entry bytes, may be NULL when size is 0
 * @param size - Number of bytes in data
 *
 * @return size_t - The decoded value, 0 when data is NULL or size is 0
 */
size_t internal_id3v2AdjustmentBits(const uint8_t *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif
