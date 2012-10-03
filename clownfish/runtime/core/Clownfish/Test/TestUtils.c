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

#define C_LUCY_TESTUTILS
#include "Lucy/Util/ToolSet.h"
#include <string.h>

#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test.h"
#include "Lucy/Analysis/Analyzer.h"
#include "Lucy/Analysis/Inversion.h"
#include "Lucy/Analysis/Token.h"
#include "Lucy/Search/TermQuery.h"
#include "Lucy/Search/PhraseQuery.h"
#include "Lucy/Search/LeafQuery.h"
#include "Lucy/Search/ANDQuery.h"
#include "Lucy/Search/NOTQuery.h"
#include "Lucy/Search/ORQuery.h"
#include "Lucy/Search/RangeQuery.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Store/RAMFile.h"
#include "Lucy/Util/Freezer.h"

uint64_t
TestUtils_random_u64() {
    uint64_t num = ((uint64_t)(rand()   & 0x7FFF) << 60)
                   | ((uint64_t)(rand() & 0x7FFF) << 45)
                   | ((uint64_t)(rand() & 0x7FFF) << 30)
                   | ((uint64_t)(rand() & 0x7FFF) << 15)
                   | ((uint64_t)(rand() & 0x7FFF) << 0);
    return num;
}

int64_t*
TestUtils_random_i64s(int64_t *buf, size_t count, int64_t min,
                      int64_t limit) {
    uint64_t  range = min < limit ? limit - min : 0;
    int64_t *ints = buf ? buf : (int64_t*)CALLOCATE(count, sizeof(int64_t));
    for (size_t i = 0; i < count; i++) {
        ints[i] = min + TestUtils_random_u64() % range;
    }
    return ints;
}

uint64_t*
TestUtils_random_u64s(uint64_t *buf, size_t count, uint64_t min,
                      uint64_t limit) {
    uint64_t  range = min < limit ? limit - min : 0;
    uint64_t *ints = buf ? buf : (uint64_t*)CALLOCATE(count, sizeof(uint64_t));
    for (size_t i = 0; i < count; i++) {
        ints[i] = min + TestUtils_random_u64() % range;
    }
    return ints;
}

double*
TestUtils_random_f64s(double *buf, size_t count) {
    double *f64s = buf ? buf : (double*)CALLOCATE(count, sizeof(double));
    for (size_t i = 0; i < count; i++) {
        uint64_t num = TestUtils_random_u64();
        f64s[i] = (double)num / U64_MAX;
    }
    return f64s;
}

uint32_t
S_random_code_point(void) {
    uint32_t code_point = 0;
    while (1) {
        uint8_t chance = (rand() % 9) + 1;
        switch (chance) {
            case 1: case 2: case 3:
                code_point = rand() % 0x80;
                break;
            case 4: case 5: case 6:
                code_point = (rand() % (0x0800  - 0x0080)) + 0x0080;
                break;
            case 7: case 8:
                code_point = (rand() % (0x10000 - 0x0800)) + 0x0800;
                break;
            case 9: {
                    uint64_t num = TestUtils_random_u64();
                    code_point = (num % (0x10FFFF - 0x10000)) + 0x10000;
                }
        }
        if (code_point > 0x10FFFF) {
            continue; // Too high.
        }
        if (code_point > 0xD7FF && code_point < 0xE000) {
            continue; // UTF-16 surrogate.
        }
        break;
    }
    return code_point;
}

CharBuf*
TestUtils_random_string(size_t length) {
    CharBuf *string = CB_new(length);
    while (length--) {
        CB_Cat_Char(string, S_random_code_point());
    }
    return string;
}

VArray*
TestUtils_doc_set() {
    VArray *docs = VA_new(10);

    VA_Push(docs, (Obj*)TestUtils_get_cb("x"));
    VA_Push(docs, (Obj*)TestUtils_get_cb("y"));
    VA_Push(docs, (Obj*)TestUtils_get_cb("z"));
    VA_Push(docs, (Obj*)TestUtils_get_cb("x a"));
    VA_Push(docs, (Obj*)TestUtils_get_cb("x a b"));
    VA_Push(docs, (Obj*)TestUtils_get_cb("x a b c"));
    VA_Push(docs, (Obj*)TestUtils_get_cb("x foo a b c d"));

    return docs;
}

CharBuf*
TestUtils_get_cb(const char *ptr) {
    return CB_new_from_utf8(ptr, strlen(ptr));
}

PolyQuery*
TestUtils_make_poly_query(uint32_t boolop, ...) {
    va_list args;
    Query *child;
    PolyQuery *retval;
    VArray *children = VA_new(0);

    va_start(args, boolop);
    while (NULL != (child = va_arg(args, Query*))) {
        VA_Push(children, (Obj*)child);
    }
    va_end(args);

    retval = boolop == BOOLOP_OR
             ? (PolyQuery*)ORQuery_new(children)
             : (PolyQuery*)ANDQuery_new(children);
    DECREF(children);
    return retval;
}

TermQuery*
TestUtils_make_term_query(const char *field, const char *term) {
    CharBuf *field_cb = (CharBuf*)ZCB_WRAP_STR(field, strlen(field));
    CharBuf *term_cb  = (CharBuf*)ZCB_WRAP_STR(term, strlen(term));
    return TermQuery_new((CharBuf*)field_cb, (Obj*)term_cb);
}

PhraseQuery*
TestUtils_make_phrase_query(const char *field, ...) {
    CharBuf *field_cb = (CharBuf*)ZCB_WRAP_STR(field, strlen(field));
    va_list args;
    VArray *terms = VA_new(0);
    PhraseQuery *query;
    char *term_str;

    va_start(args, field);
    while (NULL != (term_str = va_arg(args, char*))) {
        VA_Push(terms, (Obj*)TestUtils_get_cb(term_str));
    }
    va_end(args);

    query = PhraseQuery_new(field_cb, terms);
    DECREF(terms);
    return query;
}

LeafQuery*
TestUtils_make_leaf_query(const char *field, const char *term) {
    CharBuf *term_cb  = (CharBuf*)ZCB_WRAP_STR(term, strlen(term));
    CharBuf *field_cb = field
                        ? (CharBuf*)ZCB_WRAP_STR(field, strlen(field))
                        : NULL;
    return LeafQuery_new(field_cb, term_cb);
}

NOTQuery*
TestUtils_make_not_query(Query* negated_query) {
    NOTQuery *not_query = NOTQuery_new(negated_query);
    DECREF(negated_query);
    return not_query;
}

RangeQuery*
TestUtils_make_range_query(const char *field, const char *lower_term,
                           const char *upper_term, bool_t include_lower,
                           bool_t include_upper) {
    CharBuf *f     = (CharBuf*)ZCB_WRAP_STR(field, strlen(field));
    CharBuf *lterm = (CharBuf*)ZCB_WRAP_STR(lower_term, strlen(lower_term));
    CharBuf *uterm = (CharBuf*)ZCB_WRAP_STR(upper_term, strlen(upper_term));
    return RangeQuery_new(f, (Obj*)lterm, (Obj*)uterm, include_lower,
                          include_upper);
}

Obj*
TestUtils_freeze_thaw(Obj *object) {
    if (object) {
        RAMFile *ram_file = RAMFile_new(NULL, false);
        OutStream *outstream = OutStream_open((Obj*)ram_file);
        FREEZE(object, outstream);
        OutStream_Close(outstream);
        DECREF(outstream);

        InStream *instream = InStream_open((Obj*)ram_file);
        Obj *retval = THAW(instream);
        DECREF(instream);
        DECREF(ram_file);
        return retval;
    }
    else {
        return NULL;
    }
}

void
TestUtils_test_analyzer(TestBatch *batch, Analyzer *analyzer, CharBuf *source,
                        VArray *expected, const char *message) {
    Token *seed = Token_new((char*)CB_Get_Ptr8(source), CB_Get_Size(source),
                            0, 0, 1.0f, 1);
    Inversion *starter = Inversion_new(seed);
    Inversion *transformed = Analyzer_Transform(analyzer, starter);
    VArray *got = VA_new(1);
    Token *token;
    while (NULL != (token = Inversion_Next(transformed))) {
        CharBuf *token_text
            = CB_new_from_utf8(Token_Get_Text(token), Token_Get_Len(token));
        VA_Push(got, (Obj*)token_text);
    }
    TEST_TRUE(batch, VA_Equals(expected, (Obj*)got),
              "Transform(): %s", message);
    DECREF(transformed);

    transformed = Analyzer_Transform_Text(analyzer, source);
    VA_Clear(got);
    while (NULL != (token = Inversion_Next(transformed))) {
        CharBuf *token_text
            = CB_new_from_utf8(Token_Get_Text(token), Token_Get_Len(token));
        VA_Push(got, (Obj*)token_text);
    }
    TEST_TRUE(batch, VA_Equals(expected, (Obj*)got),
              "Transform_Text(): %s", message);
    DECREF(transformed);

    DECREF(got);
    got = Analyzer_Split(analyzer, source);
    TEST_TRUE(batch, VA_Equals(expected, (Obj*)got), "Split(): %s", message);

    DECREF(got);
    DECREF(starter);
    DECREF(seed);
}


