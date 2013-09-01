/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define C_CFISH_CHARBUF
#define C_CFISH_STRING
#define CFISH_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "charmony.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "Clownfish/CharBuf.h"

#include "Clownfish/Err.h"
#include "Clownfish/String.h"
#include "Clownfish/Util/Memory.h"
#include "Clownfish/Util/StringHelper.h"
#include "Clownfish/VTable.h"

// Helper function for throwing invalid UTF-8 error. Since THROW uses
// a String internally, calling THROW with invalid UTF-8 would create an
// infinite loop -- so we fwrite some of the bogus text to stderr and
// invoke THROW with a generic message.
#define DIE_INVALID_UTF8(text, size) \
    S_die_invalid_utf8(text, size, __FILE__, __LINE__, CFISH_ERR_FUNC_MACRO)
static void
S_die_invalid_utf8(const char *text, size_t size, const char *file, int line,
                   const char *func);

// Helper function for throwing invalid pattern error.
static void
S_die_invalid_pattern(const char *pattern);

CharBuf*
CB_new(size_t size) {
    CharBuf *self = (CharBuf*)VTable_Make_Obj(CHARBUF);
    return CB_init(self, size);
}

CharBuf*
CB_init(CharBuf *self, size_t size) {
    // Derive.
    self->ptr = (char*)MALLOCATE(size + 1);

    // Init.
    *self->ptr = '\0'; // Empty string.

    // Assign.
    self->size = 0;
    self->cap  = size + 1;

    return self;
}

CharBuf*
CB_new_from_str(String *string) {
    return CB_new_from_trusted_utf8(string->ptr, string->size);
}

CharBuf*
CB_new_from_utf8(const char *ptr, size_t size) {
    if (!StrHelp_utf8_valid(ptr, size)) {
        DIE_INVALID_UTF8(ptr, size);
    }
    return CB_new_from_trusted_utf8(ptr, size);
}

CharBuf*
CB_new_from_trusted_utf8(const char *ptr, size_t size) {
    CharBuf *self = (CharBuf*)VTable_Make_Obj(CHARBUF);

    // Derive.
    self->ptr = (char*)MALLOCATE(size + 1);

    // Copy.
    memcpy(self->ptr, ptr, size);

    // Assign.
    self->size      = size;
    self->cap       = size + 1;
    self->ptr[size] = '\0'; // Null terminate.

    return self;
}

CharBuf*
CB_newf(const char *pattern, ...) {
    CharBuf *self = CB_new(strlen(pattern));
    va_list args;
    va_start(args, pattern);
    CB_VCatF(self, pattern, args);
    va_end(args);
    return self;
}

void
CB_Destroy_IMP(CharBuf *self) {
    FREEMEM(self->ptr);
    SUPER_DESTROY(self, CHARBUF);
}

static void
S_grow(CharBuf *self, size_t size) {
    if (size >= self->cap) {
        CB_Grow(self, size);
    }
}

char*
CB_Grow_IMP(CharBuf *self, size_t size) {
    if (size >= self->cap) {
        self->cap = size + 1;
        self->ptr = (char*)REALLOCATE(self->ptr, self->cap);
    }
    return self->ptr;
}

static void
S_die_invalid_utf8(const char *text, size_t size, const char *file, int line,
                   const char *func) {
    fprintf(stderr, "Invalid UTF-8, aborting: '");
    fwrite(text, sizeof(char), size < 200 ? size : 200, stderr);
    if (size > 200) { fwrite("[...]", sizeof(char), 5, stderr); }
    fprintf(stderr, "' (length %lu)\n", (unsigned long)size);
    Err_throw_at(ERR, file, line, func, "Invalid UTF-8");
}

static void
S_die_invalid_pattern(const char *pattern) {
    size_t  pattern_len = strlen(pattern);
    fprintf(stderr, "Invalid pattern, aborting: '");
    fwrite(pattern, sizeof(char), pattern_len, stderr);
    fprintf(stderr, "'\n");
    THROW(ERR, "Invalid pattern.");
}

void
CB_catf(CharBuf *self, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    CB_VCatF(self, pattern, args);
    va_end(args);
}

void
CB_VCatF_IMP(CharBuf *self, const char *pattern, va_list args) {
    size_t      pattern_len   = strlen(pattern);
    const char *pattern_start = pattern;
    const char *pattern_end   = pattern + pattern_len;
    char        buf[64];

    for (; pattern < pattern_end; pattern++) {
        const char *slice_end = pattern;

        // Consume all characters leading up to a '%'.
        while (slice_end < pattern_end && *slice_end != '%') { slice_end++; }
        if (pattern != slice_end) {
            size_t size = slice_end - pattern;
            CB_Cat_Trusted_UTF8(self, pattern, size);
            pattern = slice_end;
        }

        if (pattern < pattern_end) {
            pattern++; // Move past '%'.

            switch (*pattern) {
                case '%': {
                        CB_Cat_Trusted_UTF8(self, "%", 1);
                    }
                    break;
                case 'o': {
                        Obj *obj = va_arg(args, Obj*);
                        if (!obj) {
                            CB_Cat_Trusted_UTF8(self, "[NULL]", 6);
                        }
                        else if (Obj_Is_A(obj, STRING)) {
                            CB_Cat(self, (String*)obj);
                        }
                        else {
                            String *string = Obj_To_String(obj);
                            CB_Cat(self, string);
                            DECREF(string);
                        }
                    }
                    break;
                case 'i': {
                        int64_t val = 0;
                        size_t size;
                        if (pattern[1] == '8') {
                            val = va_arg(args, int32_t);
                            pattern++;
                        }
                        else if (pattern[1] == '3' && pattern[2] == '2') {
                            val = va_arg(args, int32_t);
                            pattern += 2;
                        }
                        else if (pattern[1] == '6' && pattern[2] == '4') {
                            val = va_arg(args, int64_t);
                            pattern += 2;
                        }
                        else {
                            S_die_invalid_pattern(pattern_start);
                        }
                        size = sprintf(buf, "%" PRId64, val);
                        CB_Cat_Trusted_UTF8(self, buf, size);
                    }
                    break;
                case 'u': {
                        uint64_t val = 0;
                        size_t size;
                        if (pattern[1] == '8') {
                            val = va_arg(args, uint32_t);
                            pattern += 1;
                        }
                        else if (pattern[1] == '3' && pattern[2] == '2') {
                            val = va_arg(args, uint32_t);
                            pattern += 2;
                        }
                        else if (pattern[1] == '6' && pattern[2] == '4') {
                            val = va_arg(args, uint64_t);
                            pattern += 2;
                        }
                        else {
                            S_die_invalid_pattern(pattern_start);
                        }
                        size = sprintf(buf, "%" PRIu64, val);
                        CB_Cat_Trusted_UTF8(self, buf, size);
                    }
                    break;
                case 'f': {
                        if (pattern[1] == '6' && pattern[2] == '4') {
                            double num  = va_arg(args, double);
                            char bigbuf[512];
                            size_t size = sprintf(bigbuf, "%g", num);
                            CB_Cat_Trusted_UTF8(self, bigbuf, size);
                            pattern += 2;
                        }
                        else {
                            S_die_invalid_pattern(pattern_start);
                        }
                    }
                    break;
                case 'x': {
                        if (pattern[1] == '3' && pattern[2] == '2') {
                            unsigned long val = va_arg(args, uint32_t);
                            size_t size = sprintf(buf, "%.8lx", val);
                            CB_Cat_Trusted_UTF8(self, buf, size);
                            pattern += 2;
                        }
                        else {
                            S_die_invalid_pattern(pattern_start);
                        }
                    }
                    break;
                case 's': {
                        char *string = va_arg(args, char*);
                        if (string == NULL) {
                            CB_Cat_Trusted_UTF8(self, "[NULL]", 6);
                        }
                        else {
                            size_t size = strlen(string);
                            if (StrHelp_utf8_valid(string, size)) {
                                CB_Cat_Trusted_UTF8(self, string, size);
                            }
                            else {
                                CB_Cat_Trusted_UTF8(self, "[INVALID UTF8]", 14);
                            }
                        }
                    }
                    break;
                default: {
                        // Assume NULL-terminated pattern string, which
                        // eliminates the need for bounds checking if '%' is
                        // the last visible character.
                        S_die_invalid_pattern(pattern_start);
                    }
            }
        }
    }
}

String*
CB_To_String_IMP(CharBuf *self) {
    return Str_new_from_trusted_utf8(self->ptr, self->size);
}

String*
CB_Yield_String_IMP(CharBuf *self) {
    String *retval
        = Str_new_steal_from_trusted_str(self->ptr, self->size, self->cap);
    self->ptr  = NULL;
    self->size = 0;
    self->cap  = 0;
    return retval;
}

void
CB_Cat_Char_IMP(CharBuf *self, uint32_t code_point) {
    const size_t MAX_UTF8_BYTES = 4;
    if (self->size + MAX_UTF8_BYTES >= self->cap) {
        S_grow(self, Memory_oversize(self->size + MAX_UTF8_BYTES,
                                     sizeof(char)));
    }
    char *end = self->ptr + self->size;
    size_t count = StrHelp_encode_utf8_char(code_point, (uint8_t*)end);
    self->size += count;
    *(end + count) = '\0';
}

CharBuf*
CB_Clone_IMP(CharBuf *self) {
    return CB_new_from_trusted_utf8(self->ptr, self->size);
}

void
CB_Mimic_UTF8_IMP(CharBuf *self, const char* ptr, size_t size) {
    if (!StrHelp_utf8_valid(ptr, size)) {
        DIE_INVALID_UTF8(ptr, size);
    }
    if (size >= self->cap) { S_grow(self, size); }
    memmove(self->ptr, ptr, size);
    self->size = size;
    self->ptr[size] = '\0';
}

void
CB_Mimic_IMP(CharBuf *self, Obj *other) {
    if (Obj_Is_A(other, CHARBUF)) {
        CharBuf *twin = (CharBuf*)other;
        if (twin->size >= self->cap) { S_grow(self, twin->size); }
        memmove(self->ptr, twin->ptr, twin->size);
        self->size = twin->size;
        self->ptr[twin->size] = '\0';
    }
    else if (Obj_Is_A(other, STRING)) {
        String *twin = (String*)other;
        if (twin->size >= self->cap) { S_grow(self, twin->size); }
        memmove(self->ptr, twin->ptr, twin->size);
        self->size = twin->size;
        self->ptr[twin->size] = '\0';
    }
    else {
        THROW(ERR, "CharBuf can't mimic %o", Obj_Get_Class_Name(other));
    }
}

void
CB_Cat_UTF8_IMP(CharBuf *self, const char* ptr, size_t size) {
    if (!StrHelp_utf8_valid(ptr, size)) {
        DIE_INVALID_UTF8(ptr, size);
    }
    CB_Cat_Trusted_UTF8_IMP(self, ptr, size);
}

void
CB_Cat_Trusted_UTF8_IMP(CharBuf *self, const char* ptr, size_t size) {
    const size_t new_size = self->size + size;
    if (new_size >= self->cap) {
        size_t amount = Memory_oversize(new_size, sizeof(char));
        S_grow(self, amount);
    }
    memcpy((self->ptr + self->size), ptr, size);
    self->size = new_size;
    self->ptr[new_size] = '\0';
}

void
CB_Cat_IMP(CharBuf *self, const String *string) {
    const size_t new_size = self->size + string->size;
    if (new_size >= self->cap) {
        size_t amount = Memory_oversize(new_size, sizeof(char));
        S_grow(self, amount);
    }
    memcpy((self->ptr + self->size), string->ptr, string->size);
    self->size = new_size;
    self->ptr[new_size] = '\0';
}

void
CB_Set_Size_IMP(CharBuf *self, size_t size) {
    if (size >= self->cap) {
        THROW(ERR, "Can't set size of CharBuf beyond capacity");
    }
    self->size = size;
}

size_t
CB_Get_Size_IMP(CharBuf *self) {
    return self->size;
}

uint8_t*
CB_Get_Ptr8_IMP(CharBuf *self) {
    return (uint8_t*)self->ptr;
}


