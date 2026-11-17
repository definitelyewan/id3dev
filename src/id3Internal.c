/**
 * @file id3Internal.c
 * @author Ewan Jones
 * @brief Implementations of helpers shared between id3dev's source files. Not part of the public API.
 * @details Every internal_ function used by more than one source file lives here and is declared in
 * id3Internal.h. Helpers used by a single file stay static in that file.
 * @version 26.01
 * @date 2026-11-16
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <stdlib.h>
#include <limits.h>
#include "src/id3Internal.h"

/**
 * @brief Escapes a UTF-8 string for use as a JSON string value.
 * @details Escapes quotes, backslashes and control characters below 0x20 as JSON requires. Bytes of 0x20 and
 * above, including multi-byte UTF-8 sequences, are copied unchanged.
 *
 * @param str - NUL terminated UTF-8 string
 *
 * @return char* - Heap allocated escaped string. Caller must free. NULL on failure
 */
char *internal_id3JsonEscape(const char *str) {
    const char hex[] = "0123456789abcdef";
    size_t len = 0;
    size_t j = 0;
    char *out = NULL;

    if (str == NULL) {
        return NULL;
    }

    for (size_t i = 0; str[i] != '\0'; i++) {
        const unsigned char c = (unsigned char) str[i];

        if (c == '"' || c == '\\' || c == '\b' || c == '\f' || c == '\n' || c == '\r' || c == '\t') {
            len += 2;
        } else if (c < 0x20) {
            len += 6; // \u00XX
        } else {
            len++;
        }
    }

    out = malloc(len + 1);

    if (out == NULL) {
        return NULL;
    }

    for (size_t i = 0; str[i] != '\0'; i++) {
        const unsigned char c = (unsigned char) str[i];

        switch (c) {
            case '"':
            case '\\':
                out[j++] = '\\';
                out[j++] = (char) c;
                break;
            case '\b':
                out[j++] = '\\';
                out[j++] = 'b';
                break;
            case '\f':
                out[j++] = '\\';
                out[j++] = 'f';
                break;
            case '\n':
                out[j++] = '\\';
                out[j++] = 'n';
                break;
            case '\r':
                out[j++] = '\\';
                out[j++] = 'r';
                break;
            case '\t':
                out[j++] = '\\';
                out[j++] = 't';
                break;
            default:
                if (c < 0x20) {
                    out[j++] = '\\';
                    out[j++] = 'u';
                    out[j++] = '0';
                    out[j++] = '0';
                    out[j++] = hex[c >> 4];
                    out[j++] = hex[c & 0x0F];
                } else {
                    out[j++] = (char) c;
                }
                break;
        }
    }

    out[j] = '\0';
    return out;
}

/**
 * @brief Reads a big-endian unsigned value from an entry's bytes.
 * @details Used to read the bit count stored in a field keyed "adjustment". Only the trailing
 * sizeof(size_t) bytes are used, which is far beyond any bit count the standard allows.
 *
 * @param data - Entry bytes, may be NULL when size is 0.
 * @param size - Number of bytes in data.
 * @return size_t - The decoded value, 0 when data is NULL or size is 0.
 */
size_t internal_id3v2AdjustmentBits(const uint8_t *data, size_t size) {
    size_t value = 0;

    if (data == NULL) {
        return 0;
    }

    for (size_t i = (size > sizeof(size_t)) ? size - sizeof(size_t) : 0; i < size; i++) {
        value = (value << CHAR_BIT) | data[i];
    }

    return value;
}
