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

#define C_CFISH_STRING
#define C_CFISH_STACKSTRING
#define C_CFISH_STRINGITERATOR
#define C_CFISH_STACKSTRINGITERATOR
#define CFISH_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "charmony.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "Clownfish/VTable.h"
#include "Clownfish/String.h"

#include "Clownfish/CharBuf.h"
#include "Clownfish/Err.h"
#include "Clownfish/Util/Memory.h"
#include "Clownfish/Util/StringHelper.h"

#define STR_STACKTOP(string) \
    Str_StackTop(string, alloca(sizeof(StackStringIterator)))
#define STR_STACKTAIL(string) \
    Str_StackTail(string, alloca(sizeof(StackStringIterator)))

// Helper function for throwing invalid UTF-8 error. Since THROW uses
// a String internally, calling THROW with invalid UTF-8 would create an
// infinite loop -- so we fwrite some of the bogus text to stderr and
// invoke THROW with a generic message.
#define DIE_INVALID_UTF8(text, size) \
    S_die_invalid_utf8(text, size, __FILE__, __LINE__, CFISH_ERR_FUNC_MACRO)
static void
S_die_invalid_utf8(const char *text, size_t size, const char *file, int line,
                   const char *func);

String*
Str_new_from_utf8(const char *utf8, size_t size) {
    if (!StrHelp_utf8_valid(utf8, size)) {
        DIE_INVALID_UTF8(utf8, size);
    }
    String *self = (String*)VTable_Make_Obj(STRING);
    return Str_init_from_trusted_utf8(self, utf8, size);
}

String*
Str_new_from_trusted_utf8(const char *utf8, size_t size) {
    String *self = (String*)VTable_Make_Obj(STRING);
    return Str_init_from_trusted_utf8(self, utf8, size);
}

String*
Str_init_from_trusted_utf8(String *self, const char *utf8, size_t size) {
    // Allocate.
    char *ptr = (char*)MALLOCATE(size + 1);

    // Copy.
    memcpy(ptr, utf8, size);
    ptr[size] = '\0'; // Null terminate.

    // Assign.
    self->ptr    = ptr;
    self->size   = size;
    self->origin = self;

    return self;
}

String*
Str_new_steal_utf8(const char *utf8, size_t size) {
    if (!StrHelp_utf8_valid(utf8, size)) {
        DIE_INVALID_UTF8(utf8, size);
    }
    String *self = (String*)VTable_Make_Obj(STRING);
    return Str_init_steal_trusted_utf8(self, utf8, size);
}

String*
Str_new_steal_trusted_utf8(const char *utf8, size_t size) {
    String *self = (String*)VTable_Make_Obj(STRING);
    return Str_init_steal_trusted_utf8(self, utf8, size);
}

String*
Str_init_steal_trusted_utf8(String *self, const char *utf8, size_t size) {
    self->ptr    = utf8;
    self->size   = size;
    self->origin = self;
    return self;
}

String*
Str_new_wrap_utf8(const char *utf8, size_t size) {
    if (!StrHelp_utf8_valid(utf8, size)) {
        DIE_INVALID_UTF8(utf8, size);
    }
    String *self = (String*)VTable_Make_Obj(STRING);
    return Str_init_wrap_trusted_utf8(self, utf8, size);
}

String*
Str_new_wrap_trusted_utf8(const char *utf8, size_t size) {
    String *self = (String*)VTable_Make_Obj(STRING);
    return Str_init_wrap_trusted_utf8(self, utf8, size);
}

String*
Str_init_wrap_trusted_utf8(String *self, const char *ptr, size_t size) {
    self->ptr    = ptr;
    self->size   = size;
    self->origin = NULL;
    return self;
}

String*
Str_new_from_char(int32_t code_point) {
    const size_t MAX_UTF8_BYTES = 4;
    char   *ptr  = (char*)MALLOCATE(MAX_UTF8_BYTES + 1);
    size_t  size = StrHelp_encode_utf8_char(code_point, (uint8_t*)ptr);
    ptr[size] = '\0';

    String *self = (String*)VTable_Make_Obj(STRING);
    self->ptr    = ptr;
    self->size   = size;
    self->origin = self;
    return self;
}

String*
Str_newf(const char *pattern, ...) {
    CharBuf *buf = CB_new(strlen(pattern));
    va_list args;
    va_start(args, pattern);
    CB_VCatF(buf, pattern, args);
    va_end(args);
    String *self = CB_Yield_String(buf);
    DECREF(buf);
    return self;
}

static String*
S_new_substring(String *string, size_t byte_offset, size_t size) {
    String *self = (String*)VTable_Make_Obj(STRING);

    if (string->origin == NULL) {
        // Copy substring of wrapped strings.
        Str_init_from_trusted_utf8(self, string->ptr + byte_offset, size);
    }
    else {
        self->ptr    = string->ptr + byte_offset;
        self->size   = size;
        self->origin = (String*)INCREF(string->origin);
    }

    return self;
}

Obj*
Str_Inc_RefCount_IMP(String *self) {
    if (self->origin == NULL) {
        // Copy wrapped strings when the refcount is increased.
        String *copy = (String*)VTable_Make_Obj(STRING);
        return (Obj*)Str_init_from_trusted_utf8(copy, self->ptr, self->size);
    }
    else {
        Str_Inc_RefCount_t super_incref
            = SUPER_METHOD_PTR(STRING, CFISH_Str_Inc_RefCount);
        return super_incref(self);
    }
}

void
Str_Destroy_IMP(String *self) {
    if (self->origin == self) {
        FREEMEM((char*)self->ptr);
    }
    else {
        DECREF(self->origin);
    }
    SUPER_DESTROY(self, STRING);
}

int32_t
Str_Hash_Sum_IMP(String *self) {
    uint32_t hashvalue = 5381;
    StackStringIterator *iter = STR_STACKTOP(self);

    const StrIter_Next_t next
        = METHOD_PTR(STRINGITERATOR, CFISH_StrIter_Next);
    int32_t code_point;
    while (STRITER_DONE != (code_point = next((StringIterator*)iter))) {
        hashvalue = ((hashvalue << 5) + hashvalue) ^ code_point;
    }

    return (int32_t) hashvalue;
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

String*
Str_To_String_IMP(String *self) {
    return (String*)INCREF(self);
}

String*
Str_Swap_Chars_IMP(String *self, int32_t match, int32_t replacement) {
    CharBuf *charbuf = CB_new(self->size);
    StackStringIterator *iter = STR_STACKTOP(self);
    int32_t code_point;

    while (STRITER_DONE != (code_point = SStrIter_Next(iter))) {
        if (code_point == match) { code_point = replacement; }
        CB_Cat_Char(charbuf, code_point);
    }

    String *retval = CB_Yield_String(charbuf);
    DECREF(charbuf);
    return retval;
}

int64_t
Str_To_I64_IMP(String *self) {
    return Str_BaseX_To_I64(self, 10);
}

int64_t
Str_BaseX_To_I64_IMP(String *self, uint32_t base) {
    StackStringIterator *iter = STR_STACKTOP(self);
    int64_t retval = 0;
    bool is_negative = false;
    int32_t code_point = SStrIter_Next(iter);

    // Advance past minus sign.
    if (code_point == '-') {
        code_point = SStrIter_Next(iter);
        is_negative = true;
    }

    // Accumulate.
    while (code_point != STRITER_DONE) {
        if (isalnum(code_point)) {
            int32_t addend = isdigit(code_point)
                             ? code_point - '0'
                             : tolower(code_point) - 'a' + 10;
            if (addend > (int32_t)base) { break; }
            retval *= base;
            retval += addend;
        }
        else {
            break;
        }
        code_point = SStrIter_Next(iter);
    }

    // Apply minus sign.
    if (is_negative) { retval = 0 - retval; }

    return retval;
}

static double
S_safe_to_f64(String *self) {
    size_t amount = self->size < 511 ? self->size : 511;
    char buf[512];
    memcpy(buf, self->ptr, amount);
    buf[amount] = 0; // NULL-terminate.
    return strtod(buf, NULL);
}

double
Str_To_F64_IMP(String *self) {
    char   *end;
    double  value    = strtod(self->ptr, &end);
    size_t  consumed = end - self->ptr;
    if (consumed > self->size) { // strtod overran
        value = S_safe_to_f64(self);
    }
    return value;
}

char*
Str_To_Utf8_IMP(String *self) {
    char *buf = (char*)malloc(self->size + 1);
    memcpy(buf, self->ptr, self->size);
    buf[self->size] = '\0'; // NULL-terminate.
    return buf;
}

String*
Str_Clone_IMP(String *self) {
    return (String*)INCREF(self);
}

String*
Str_Cat_IMP(String *self, String *other) {
    return Str_Cat_Trusted_Utf8(self, other->ptr, other->size);
}

String*
Str_Cat_Utf8_IMP(String *self, const char* ptr, size_t size) {
    if (!StrHelp_utf8_valid(ptr, size)) {
        DIE_INVALID_UTF8(ptr, size);
    }
    return Str_Cat_Trusted_Utf8(self, ptr, size);
}

String*
Str_Cat_Trusted_Utf8_IMP(String *self, const char* ptr, size_t size) {
    size_t  result_size = self->size + size;
    char   *result_ptr  = (char*)MALLOCATE(result_size + 1);
    memcpy(result_ptr, self->ptr, self->size);
    memcpy(result_ptr + self->size, ptr, size);
    result_ptr[result_size] = '\0';
    String *result = (String*)VTable_Make_Obj(STRING);
    return Str_init_steal_trusted_utf8(result, result_ptr, result_size);
}

bool
Str_Starts_With_IMP(String *self, String *prefix) {
    return Str_Starts_With_Utf8_IMP(self, prefix->ptr, prefix->size);
}

bool
Str_Starts_With_Utf8_IMP(String *self, const char *prefix, size_t size) {
    if (size <= self->size
        && (memcmp(self->ptr, prefix, size) == 0)
       ) {
        return true;
    }
    else {
        return false;
    }
}

bool
Str_Equals_IMP(String *self, Obj *other) {
    String *const twin = (String*)other;
    if (twin == self)              { return true; }
    if (!Obj_Is_A(other, STRING)) { return false; }
    return Str_Equals_Utf8_IMP(self, twin->ptr, twin->size);
}

int32_t
Str_Compare_To_IMP(String *self, Obj *other) {
    CERTIFY(other, STRING);
    return Str_compare(&self, &other);
}

bool
Str_Equals_Utf8_IMP(String *self, const char *ptr, size_t size) {
    if (self->size != size) {
        return false;
    }
    return (memcmp(self->ptr, ptr, self->size) == 0);
}

bool
Str_Ends_With_IMP(String *self, String *postfix) {
    return Str_Ends_With_Utf8_IMP(self, postfix->ptr, postfix->size);
}

bool
Str_Ends_With_Utf8_IMP(String *self, const char *postfix, size_t postfix_len) {
    if (postfix_len <= self->size) {
        const char *start = self->ptr + self->size - postfix_len;
        if (memcmp(start, postfix, postfix_len) == 0) {
            return true;
        }
    }

    return false;
}

int64_t
Str_Find_IMP(String *self, String *substring) {
    return Str_Find_Utf8(self, substring->ptr, substring->size);
}

int64_t
Str_Find_Utf8_IMP(String *self, const char *ptr, size_t size) {
    StackStringIterator *iter = STR_STACKTOP(self);
    int64_t location = 0;

    while (iter->byte_offset + size <= self->size) {
        if (memcmp(self->ptr + iter->byte_offset, ptr, size) == 0) {
            return location;
        }
        SStrIter_Advance(iter, 1);
        location++;
    }

    return -1;
}

String*
Str_Trim_IMP(String *self) {
    StackStringIterator *top = STR_STACKTOP(self);
    SStrIter_Skip_Next_Whitespace(top);

    StackStringIterator *tail = NULL;
    if (top->byte_offset < self->size) {
        tail = STR_STACKTAIL(self);
        SStrIter_Skip_Prev_Whitespace(tail);
    }

    return StrIter_substring((StringIterator*)top, (StringIterator*)tail);
}

String*
Str_Trim_Top_IMP(String *self) {
    StackStringIterator *top = STR_STACKTOP(self);
    SStrIter_Skip_Next_Whitespace(top);
    return StrIter_substring((StringIterator*)top, NULL);
}

String*
Str_Trim_Tail_IMP(String *self) {
    StackStringIterator *tail = STR_STACKTAIL(self);
    SStrIter_Skip_Prev_Whitespace(tail);
    return StrIter_substring(NULL, (StringIterator*)tail);
}

size_t
Str_Length_IMP(String *self) {
    StackStringIterator *iter = STR_STACKTOP(self);
    return SStrIter_Advance(iter, SIZE_MAX);
}

int32_t
Str_Code_Point_At_IMP(String *self, size_t tick) {
    StackStringIterator *iter = STR_STACKTOP(self);
    SStrIter_Advance(iter, tick);
    int32_t code_point = SStrIter_Next(iter);
    return code_point == STRITER_DONE ? 0 : code_point;
}

int32_t
Str_Code_Point_From_IMP(String *self, size_t tick) {
    if (tick == 0) { return 0; }
    StackStringIterator *iter = STR_STACKTAIL(self);
    SStrIter_Recede(iter, tick - 1);
    int32_t code_point = SStrIter_Prev(iter);
    return code_point == STRITER_DONE ? 0 : code_point;
}

String*
Str_SubString_IMP(String *self, size_t offset, size_t len) {
    StackStringIterator *iter = STR_STACKTOP(self);

    SStrIter_Advance(iter, offset);
    size_t start_offset = iter->byte_offset;

    SStrIter_Advance(iter, len);
    size_t size = iter->byte_offset - start_offset;

    return S_new_substring(self, start_offset, size);
}

int
Str_compare(const void *va, const void *vb) {
    String *a = *(String**)va;
    String *b = *(String**)vb;
    size_t min_size;
    int    tie;

    if (a->size <= b->size) {
        min_size = a->size;
        tie      = a->size < b->size ? -1 : 0;
    }
    else {
        min_size = b->size;
        tie      = 1;
    }

    int comparison = memcmp(a->ptr, b->ptr, min_size);
    if (comparison < 0) { return -1; }
    if (comparison > 0) { return 1; }

    return tie;
}

bool
Str_less_than(const void *va, const void *vb) {
    return Str_compare(va, vb) < 0 ? true : false;
}

size_t
Str_Get_Size_IMP(String *self) {
    return self->size;
}

const char*
Str_Get_Ptr8_IMP(String *self) {
    return self->ptr;
}

StringIterator*
Str_Top_IMP(String *self) {
    return StrIter_new(self, 0);
}

StringIterator*
Str_Tail_IMP(String *self) {
    return StrIter_new(self, self->size);
}

StackStringIterator*
Str_StackTop_IMP(String *self, void *allocation) {
    return SStrIter_new(allocation, self, 0);
}

StackStringIterator*
Str_StackTail_IMP(String *self, void *allocation) {
    return SStrIter_new(allocation, self, self->size);
}

/*****************************************************************/

StackString*
SStr_new_from_str(void *allocation, size_t alloc_size, String *string) {
    size_t  size = string->size;
    char   *ptr  = ((char*)allocation) + sizeof(StackString);

    if (alloc_size < sizeof(StackString) + size + 1) {
        THROW(ERR, "alloc_size of StackString too small");
    }

    memcpy(ptr, string->ptr, size);
    ptr[size] = '\0';

    StackString *self = (StackString*)VTable_Init_Obj(STACKSTRING, allocation);
    self->ptr    = ptr;
    self->size   = size;
    self->origin = NULL;
    return self;
}

StackString*
SStr_wrap_str(void *allocation, const char *ptr, size_t size) {
    StackString *self
        = (StackString*)VTable_Init_Obj(STACKSTRING, allocation);
    self->size   = size;
    self->ptr    = ptr;
    self->origin = NULL;
    return self;
}

StackString*
SStr_wrap(void *allocation, String *source) {
    return SStr_wrap_str(allocation, source->ptr, source->size);
}

size_t
SStr_size() {
    return sizeof(StackString);
}

void
SStr_Destroy_IMP(StackString *self) {
    THROW(ERR, "Can't destroy a StackString ('%o')", self);
}

/*****************************************************************/

StringIterator*
StrIter_new(String *string, size_t byte_offset) {
    StringIterator *self = (StringIterator*)VTable_Make_Obj(STRINGITERATOR);
    self->string      = (String*)INCREF(string);
    self->byte_offset = byte_offset;
    return self;
}

String*
StrIter_substring(StringIterator *top, StringIterator *tail) {
    String *string;
    size_t  top_offset;
    size_t  tail_offset;

    if (tail == NULL) {
        if (top == NULL) {
            THROW(ERR, "StrIter_substring: Both top and tail are NULL");
        }
        string      = top->string;
        tail_offset = string->size;
    }
    else {
        string = tail->string;
        if (top != NULL && string != top->string) {
            THROW(ERR, "StrIter_substring: strings don't match");
        }

        tail_offset = tail->byte_offset;
        if (tail_offset > string->size) {
            THROW(ERR, "Invalid StringIterator offset");
        }
    }

    if (top == NULL) {
        top_offset = 0;
    }
    else {
        top_offset = top->byte_offset;
        if (top_offset > tail_offset) {
            THROW(ERR, "StrIter_substring: top is behind tail");
        }
    }

    return S_new_substring(string, top_offset, tail_offset - top_offset);
}

StringIterator*
StrIter_Clone_IMP(StringIterator *self) {
    return StrIter_new(self->string, self->byte_offset);
}

void
StrIter_Assign_IMP(StringIterator *self, StringIterator *other) {
    if (self->string != other->string) {
        DECREF(self->string);
        self->string = (String*)INCREF(other->string);
    }
    self->byte_offset = other->byte_offset;
}

bool
StrIter_Equals_IMP(StringIterator *self, Obj *other) {
    StringIterator *const twin = (StringIterator*)other;
    if (twin == self)                     { return true; }
    if (!Obj_Is_A(other, STRINGITERATOR)) { return false; }
    return self->string == twin->string
           && self->byte_offset == twin->byte_offset;
}

int32_t
StrIter_Compare_To_IMP(StringIterator *self, Obj *other) {
    StringIterator *twin = (StringIterator*)CERTIFY(other, STRINGITERATOR);
    if (self->string != twin->string) {
        THROW(ERR, "Can't compare iterators of different strings");
    }
    if (self->byte_offset < twin->byte_offset) { return -1; }
    if (self->byte_offset > twin->byte_offset) { return 1; }
    return 0;
}

bool
StrIter_Has_Next_IMP(StringIterator *self) {
    return self->byte_offset < self->string->size;
}

bool
StrIter_Has_Prev_IMP(StringIterator *self) {
    return self->byte_offset != 0;
}

int32_t
StrIter_Next_IMP(StringIterator *self) {
    String *string      = self->string;
    size_t  byte_offset = self->byte_offset;
    size_t  size        = string->size;

    if (byte_offset >= size) { return STRITER_DONE; }

    const uint8_t *const ptr = (const uint8_t*)string->ptr;
    int32_t retval = ptr[byte_offset++];

    if (retval >= 0x80) {
        /*
         * The 'mask' bit is tricky. In each iteration, 'retval' is
         * left-shifted by 6 and 'mask' by 5 bits. So relative to the first
         * byte of the sequence, 'mask' moves one bit to the right.
         *
         * The possible outcomes after the loop are:
         *
         * Two byte sequence
         * retval: 110aaaaa bbbbbb
         * mask:   00100000 000000
         *
         * Three byte sequence
         * retval: 1110aaaa bbbbbb cccccc
         * mask:   00010000 000000 000000
         *
         * Four byte sequence
         * retval: 11110aaa bbbbbb cccccc dddddd
         * mask:   00001000 000000 000000 000000
         *
         * This also illustrates why the exit condition (retval & mask)
         * works. After the first iteration, the third most significant bit
         * is tested. After the second iteration, the fourth, and so on.
         */

        int32_t mask = 1 << 6;

        do {
            if (byte_offset >= size) {
                THROW(ERR, "StrIter_Next: Invalid UTF-8");
            }

            retval = (retval << 6) | (ptr[byte_offset++] & 0x3F);
            mask <<= 5;
        } while (retval & mask);

        retval &= mask - 1;
    }

    self->byte_offset = byte_offset;
    return retval;
}

int32_t
StrIter_Prev_IMP(StringIterator *self) {
    size_t byte_offset = self->byte_offset;

    if (byte_offset == 0) { return STRITER_DONE; }

    const uint8_t *const ptr = (const uint8_t*)self->string->ptr;
    int32_t retval = ptr[--byte_offset];

    if (retval >= 0x80) {
        // Construct the result from right to left.

        if (byte_offset == 0) {
            THROW(ERR, "StrIter_Prev: Invalid UTF-8");
        }

        retval &= 0x3F;
        int shift = 6;
        int32_t first_byte_mask = 0x1F;
        int32_t byte = ptr[--byte_offset];

        while ((byte & 0xC0) == 0x80) {
            if (byte_offset == 0) {
                THROW(ERR, "StrIter_Prev: Invalid UTF-8");
            }

            retval |= (byte & 0x3F) << shift;
            shift += 6;
            first_byte_mask >>= 1;
            byte = ptr[--byte_offset];
        }

        retval |= (byte & first_byte_mask) << shift;
    }

    self->byte_offset = byte_offset;
    return retval;
}

size_t
StrIter_Advance_IMP(StringIterator *self, size_t num) {
    size_t num_skipped = 0;
    size_t byte_offset = self->byte_offset;
    size_t size        = self->string->size;
    const uint8_t *const ptr = (const uint8_t*)self->string->ptr;

    while (num_skipped < num) {
        if (byte_offset >= size) {
            break;
        }
        uint8_t first_byte = ptr[byte_offset];
        byte_offset += StrHelp_UTF8_COUNT[first_byte];
        ++num_skipped;
    }

    if (byte_offset > size) {
        THROW(ERR, "StrIter_Advance: Invalid UTF-8");
    }

    self->byte_offset = byte_offset;
    return num_skipped;
}

size_t
StrIter_Recede_IMP(StringIterator *self, size_t num) {
    size_t num_skipped = 0;
    size_t byte_offset = self->byte_offset;
    const uint8_t *const ptr = (const uint8_t*)self->string->ptr;

    while (num_skipped < num) {
        if (byte_offset == 0) {
            break;
        }

        uint8_t byte;
        do {
            if (byte_offset == 0) {
                THROW(ERR, "StrIter_Recede: Invalid UTF-8");
            }

            byte = ptr[--byte_offset];
        } while ((byte & 0xC0) == 0x80);
        ++num_skipped;
    }

    self->byte_offset = byte_offset;
    return num_skipped;
}

size_t
StrIter_Skip_Next_Whitespace_IMP(StringIterator *self) {
    size_t  num_skipped = 0;
    size_t  byte_offset = self->byte_offset;
    int32_t code_point;

    while (STRITER_DONE != (code_point = StrIter_Next(self))) {
        if (!StrHelp_is_whitespace(code_point)) { break; }
        byte_offset = self->byte_offset;
        ++num_skipped;
    }

    self->byte_offset = byte_offset;
    return num_skipped;
}

size_t
StrIter_Skip_Prev_Whitespace_IMP(StringIterator *self) {
    size_t  num_skipped = 0;
    size_t  byte_offset = self->byte_offset;
    int32_t code_point;

    while (STRITER_DONE != (code_point = StrIter_Prev(self))) {
        if (!StrHelp_is_whitespace(code_point)) { break; }
        byte_offset = self->byte_offset;
        ++num_skipped;
    }

    self->byte_offset = byte_offset;
    return num_skipped;
}

bool
StrIter_Starts_With_IMP(StringIterator *self, String *prefix) {
    return StrIter_Starts_With_Utf8_IMP(self, prefix->ptr, prefix->size);
}

bool
StrIter_Starts_With_Utf8_IMP(StringIterator *self, const char *prefix,
                             size_t size) {
    String *string      = self->string;
    size_t  byte_offset = self->byte_offset;

    if (byte_offset > string->size) {
        THROW(ERR, "Invalid StringIterator offset");
    }

    if (string->size - byte_offset < size) { return false; }

    return memcmp(string->ptr + byte_offset, prefix, size) == 0;
}

bool
StrIter_Ends_With_IMP(StringIterator *self, String *postfix) {
    return StrIter_Ends_With_Utf8_IMP(self, postfix->ptr, postfix->size);
}

bool
StrIter_Ends_With_Utf8_IMP(StringIterator *self, const char *postfix,
                           size_t size) {
    String *string      = self->string;
    size_t  byte_offset = self->byte_offset;

    if (byte_offset > string->size) {
        THROW(ERR, "Invalid StringIterator offset");
    }

    if (byte_offset < size) { return false; }

    return memcmp(string->ptr + byte_offset - size, postfix, size) == 0;
}

void
StrIter_Destroy_IMP(StringIterator *self) {
    DECREF(self->string);
    SUPER_DESTROY(self, STRINGITERATOR);
}

/*****************************************************************/

StackStringIterator*
SStrIter_new(void *allocation, String *string, size_t byte_offset) {
    StackStringIterator *self
        = (StackStringIterator*)VTable_Init_Obj(STACKSTRINGITERATOR,
                                                allocation);
    // Assume that the string will be available for the lifetime of the
    // iterator and don't increase its refcount.
    self->string      = string;
    self->byte_offset = byte_offset;
    return self;
}

void
SStrIter_Destroy_IMP(StackStringIterator *self) {
    UNUSED_VAR(self);
    THROW(ERR, "Can't destroy a StackStringIterator");
}


