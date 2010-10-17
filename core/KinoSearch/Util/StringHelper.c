#define C_KINO_STRINGHELPER
#include <string.h>

#define KINO_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "KinoSearch/Util/StringHelper.h"
#include "KinoSearch/Object/Err.h"
#include "KinoSearch/Util/Memory.h"

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

// The list of code points in this function is chosen based on the White_Space
// property in http://www.unicode.org/Public/UNIDATA/PropList.txt

/* 

EXHIBIT 1 UNICODE, INC. LICENSE AGREEMENT - DATA FILES AND SOFTWARE

Unicode Data Files include all data files under the directories
http://www.unicode.org/Public/, http://www.unicode.org/reports/, and
http://www.unicode.org/cldr/data/ . Unicode Software includes any source code
published in the Unicode Standard or under the directories
http://www.unicode.org/Public/, http://www.unicode.org/reports/, and
http://www.unicode.org/cldr/data/.

NOTICE TO USER: Carefully read the following legal agreement. BY DOWNLOADING,
INSTALLING, COPYING OR OTHERWISE USING UNICODE INC.'S DATA FILES ("DATA
FILES"), AND/OR SOFTWARE ("SOFTWARE"), YOU UNEQUIVOCALLY ACCEPT, AND AGREE TO
BE BOUND BY, ALL OF THE TERMS AND CONDITIONS OF THIS AGREEMENT. IF YOU DO NOT
AGREE, DO NOT DOWNLOAD, INSTALL, COPY, DISTRIBUTE OR USE THE DATA FILES OR
SOFTWARE.

COPYRIGHT AND PERMISSION NOTICE

Copyright 1991-2010 Unicode, Inc. All rights reserved. Distributed under the
Terms of Use in http://www.unicode.org/copyright.html.

Permission is hereby granted, free of charge, to any person obtaining a copy of
the Unicode data files and any associated documentation (the "Data Files") or
Unicode software and any associated documentation (the "Software") to deal in
the Data Files or Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, and/or sell copies
of the Data Files or Software, and to permit persons to whom the Data Files or
Software are furnished to do so, provided that (a) the above copyright
notice(s) and this permission notice appear with all copies of the Data Files
or Software, (b) both the above copyright notice(s) and this permission notice
appear in associated documentation, and (c) there is clear notice in each
modified Data File or in the Software as well as in the documentation
associated with the Data File(s) or Software that the data or software has been
modified.

THE DATA FILES AND SOFTWARE ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD
PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN
THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL
DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE DATA FILES OR
SOFTWARE.

Except as contained in this notice, the name of a copyright holder shall not be
used in advertising or otherwise to promote the sale, use or other dealings in
these Data Files or Software without prior written authorization of the
copyright holder.

Unicode and the Unicode logo are trademarks of Unicode, Inc., and may be
registered in some jurisdictions. All other trademarks and registered
trademarks mentioned herein are the property of their respective owners.

*/

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
    int bytes = StrHelp_UTF8_COUNT[retval];

    switch (bytes & 0x7) {
        case 1:
            break;

        case 2: 
            retval =   ((retval    & 0x1F) << 6)
                     |  (string[1] & 0x3F);
            break;

        case 3: 
            retval =   ((retval    & 0x0F) << 12)
                     | ((string[1] & 0x3F) << 6)
                     |  (string[2] & 0x3F);
            break;

        case 4: 
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

/* Copyright 2006-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

