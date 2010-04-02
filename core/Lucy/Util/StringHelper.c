#define C_LUCY_STRINGHELPER
#include <string.h>

#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "Lucy/Util/StringHelper.h"
#include "Lucy/Object/Err.h"
#include "Lucy/Util/Memory.h"

const uint8_t StrHelp_UTF8_SKIP[] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,7,7
};
#define SKIP_MASK 0x7

const uint8_t StrHelp_UTF8_TRAILING[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,7,7,7,7,7,7,7,7,7,7,7
};
#define TRAILING_MASK 0x7

int32_t
StrHelp_overlap(const char *a, const char *b, size_t a_len,  size_t b_len)
{
    size_t i;
    const size_t len = a_len <= b_len ? a_len : b_len;

    for (i = 0; i < len; i++) {
        if (*a++ != *b++) { break; }
    }
    return i;
}

static const char base36_chars[] = "0123456789abcdefghijklmnopqrstuvwxyz";

uint32_t
StrHelp_to_base36(uint64_t num, void *buffer) 
{
    char  my_buf[StrHelp_MAX_BASE36_BYTES];
    char *buf = my_buf + StrHelp_MAX_BASE36_BYTES - 1;
    char *end = buf;

    // Null terminate. 
    *buf = '\0';

    // Convert to base 36 characters. 
    do {
        *(--buf) = base36_chars[ num % 36 ];
        num /= 36;
    } while (num > 0);

    {
        uint32_t size = end - buf;
        memcpy(buffer, buf, size + 1);
        return size;
    }
}

// This function is adapted from sample code in RFC 2640. 
bool_t
StrHelp_utf8_valid(const char *ptr, size_t size)
{
    const unsigned char *      buf = (const unsigned char*)ptr;
    const unsigned char *const end = buf + size;
    unsigned char byte2_range_mask = 0x00;
    unsigned char c;
    int trailing = 0;  // Continuation bytes. 

    while (buf != end) {
        c = *buf++;
        if (trailing) {
            // Does trailing byte follow UTF-8 format? 
            if ((c & 0xC0) == 0x80) {
                // Need to check 2nd byte for proper range? 
                if (byte2_range_mask) {
                    // Reset mask if byte passes, otherwise fail. 
                    if (c & byte2_range_mask) { byte2_range_mask = 0x00; }
                    else                      { return false; }
                }
                trailing--;
            } 
            else {
                return false;
            }
        }
        else if ((c & 0x80) == 0x00) { // 1-byte UTF-8 
            continue;
        }
        else if ((c & 0xE0) == 0xC0) { // 2-byte UTF-8 
            // Verify UTF-8 byte in range. 
            if (c & 0x1E) { trailing = 1; }
            else          { return false; }
        }
        else if ((c & 0xF0) == 0xE0) { // 3-byte UTF-8 
            if (!(c & 0x0F)) { byte2_range_mask = 0x20; }
            trailing = 2;
        } 
        else if (c >= 0xF0 && c <= 0xF4) { // 4-byte UTF-8 
            if (!(c & 0x07)) { byte2_range_mask = 0x30; }
            trailing = 3;
        }
        else {
            return false;
        }
    }

    return trailing == 0 ? true : false;
}

uint32_t
StrHelp_encode_utf8_char(uint32_t code_point, void *buffer)
{
    uint8_t *buf = (uint8_t*)buffer;
    if (code_point <= 0x7F) { // ASCII 
        buf[0] = (uint8_t)code_point;
        return 1;
    }
    else if (code_point <= 0x07FF) { // 2 byte range 
        buf[0] = (uint8_t)(0xC0 | (code_point >> 6));
        buf[1] = (uint8_t)(0x80 | (code_point & 0x3f));
        return 2;
    }
    else if (code_point <= 0xFFFF) { // 3 byte range 
        buf[0] = (uint8_t)(0xE0 | ( code_point >> 12       ));
        buf[1] = (uint8_t)(0x80 | ((code_point >> 6) & 0x3F));
        buf[2] = (uint8_t)(0x80 | ( code_point       & 0x3f));
        return 3;
    }
    else if (code_point <= 0x10FFFF) { // 4 byte range 
        buf[0] = (uint8_t)(0xF0 | ( code_point >> 18        ));
        buf[1] = (uint8_t)(0x80 | ((code_point >> 12) & 0x3F));
        buf[2] = (uint8_t)(0x80 | ((code_point >> 6 ) & 0x3F));
        buf[3] = (uint8_t)(0x80 | ( code_point        & 0x3f));
        return 4;
    }
    else {
        THROW(ERR, "Illegal Unicode code point: %u32", code_point);
        UNREACHABLE_RETURN(uint32_t);
    }
}

bool_t
StrHelp_is_whitespace(uint32_t code_point)
{
    switch (code_point) {
                 // <control-0009>..<control-000D> 
    case 0x0009: case 0x000A: case 0x000B: case 0x000C: case 0x000D:
    case 0x0020: // SPACE 
    case 0x0085: // <control-0085>
    case 0x00A0: // NO-BREAK SPACE 
    case 0x1680: // OGHAM SPACE MARK 
    case 0x180E: // MONGOLIAN VOWEL SEPARATOR 
                 // EN QUAD..HAIR SPACE 
    case 0x2000: case 0x2001: case 0x2002: case 0x2003: case 0x2004: 
    case 0x2005: case 0x2006: case 0x2007: case 0x2008: case 0x2009: 
    case 0x200A:
    case 0x2028: // LINE SEPARATOR
    case 0x2029: // PARAGRAPH SEPARATOR
    case 0x202F: // NARROW NO-BREAK SPACE
    case 0x205F: // MEDIUM MATHEMATICAL SPACE
    case 0x3000: // IDEOGRAPHIC SPACE
        return true;

    default:
        return false;
    }
}

uint32_t
StrHelp_decode_utf8_char(const char *ptr)
{
    const uint8_t *const string = (const uint8_t*)ptr;
    uint32_t retval = *string;
    int trailing = StrHelp_UTF8_TRAILING[retval];

    switch (trailing & TRAILING_MASK) {
        case 0:
            break;

        case 1: 
            retval =   ((retval    & 0x1F) << 6)
                     |  (string[1] & 0x3F);
            break;

        case 2: 
            retval =   ((retval    & 0x0F) << 12)
                     | ((string[1] & 0x3F) << 6)
                     |  (string[2] & 0x3F);
            break;

        case 3: 
            retval =   ((retval    & 0x07) << 18)
                     | ((string[1] & 0x3F) << 12)
                     | ((string[2] & 0x3F) << 6)
                     |  (string[3] & 0x3F);
            break;

        default:
            THROW(ERR, "Invalid UTF-8 header byte: %x32", retval);
    }

    return retval;
}

const char*
StrHelp_back_utf8_char(const char *ptr, char *start)
{
    while (--ptr >= start) {
        if ((*ptr & 0xC0) != 0x80) { return ptr; }
    }
    return NULL;
}

/* Copyright 2009 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

