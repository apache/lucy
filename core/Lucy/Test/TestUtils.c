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

#define C_TESTLUCY_TESTUTILS
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"
#include <string.h>

#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Clownfish/TestHarness/TestUtils.h"
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
#include "Lucy/Store/FSFolder.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Store/RAMFile.h"
#include "Lucy/Util/Freezer.h"

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
                           const char *upper_term, bool include_lower,
                           bool include_upper) {
    CharBuf *f     = (CharBuf*)ZCB_WRAP_STR(field, strlen(field));
    CharBuf *lterm = (CharBuf*)ZCB_WRAP_STR(lower_term, strlen(lower_term));
    CharBuf *uterm = (CharBuf*)ZCB_WRAP_STR(upper_term, strlen(upper_term));
    return RangeQuery_new(f, (Obj*)lterm, (Obj*)uterm, include_lower,
                          include_upper);
}

void
TestUtils_test_analyzer(TestBatchRunner *runner, Analyzer *analyzer,
                        CharBuf *source, VArray *expected,
                        const char *message) {
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
    TEST_TRUE(runner, VA_Equals(expected, (Obj*)got),
              "Transform(): %s", message);
    DECREF(transformed);

    transformed = Analyzer_Transform_Text(analyzer, source);
    VA_Clear(got);
    while (NULL != (token = Inversion_Next(transformed))) {
        CharBuf *token_text
            = CB_new_from_utf8(Token_Get_Text(token), Token_Get_Len(token));
        VA_Push(got, (Obj*)token_text);
    }
    TEST_TRUE(runner, VA_Equals(expected, (Obj*)got),
              "Transform_Text(): %s", message);
    DECREF(transformed);

    DECREF(got);
    got = Analyzer_Split(analyzer, source);
    TEST_TRUE(runner, VA_Equals(expected, (Obj*)got), "Split(): %s", message);

    DECREF(got);
    DECREF(starter);
    DECREF(seed);
}

FSFolder*
TestUtils_modules_folder() {
    static const char *const paths[] = { "modules", "../modules" };

    for (int i = 0; i < sizeof(paths) / sizeof(char*); i++) {
        CharBuf *path = CB_newf(paths[i]);
        FSFolder *modules_folder = FSFolder_new(path);
        DECREF(path);
        if (FSFolder_Check(modules_folder)) {
            return modules_folder;
        }
        DECREF(modules_folder);
    }

    THROW(ERR, "Can't open modules folder");
    UNREACHABLE_RETURN(FSFolder*);
}

