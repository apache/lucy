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

#define C_TESTLUCY_TESTHIGHLIGHTER
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Highlight/TestHighlighter.h"
#include "Lucy/Highlight/Highlighter.h"

#include "Lucy/Analysis/StandardTokenizer.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Highlight/HeatMap.h"
#include "Lucy/Index/Indexer.h"
#include "Lucy/Plan/FullTextType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/Hits.h"
#include "Lucy/Search/IndexSearcher.h"
#include "Lucy/Search/Span.h"
#include "Lucy/Search/TermQuery.h"
#include "Lucy/Store/RAMFolder.h"

#define PHI      "\xCE\xA6"
#define ELLIPSIS "\xE2\x80\xA6"

#define TEST_STRING \
    "1 2 3 4 5 1 2 3 4 5 1 2 3 4 5 1 2 3 4 5 1 2 3 4 5 " \
    "1 2 3 4 5 1 2 3 4 5 1 2 3 4 5 1 2 3 4 5 1 2 3 4 5 " \
    "1 2 3 4 5 1 2 3 4 5 1 2 3 4 5 1 2 3 4 5 1 2 3 4 5 " \
    "1 2 3 4 5 1 2 3 4 5 1 2 3 4 5 1 2 3 4 5 1 2 3 4 5 " \
    PHI " a b c d x y z h i j k " \
    "6 7 8 9 0 6 7 8 9 0 6 7 8 9 0 6 7 8 9 0 6 7 8 9 0 " \
    "6 7 8 9 0 6 7 8 9 0 6 7 8 9 0 6 7 8 9 0 6 7 8 9 0 " \
    "6 7 8 9 0 6 7 8 9 0 6 7 8 9 0 6 7 8 9 0 6 7 8 9 0 " \
    "6 7 8 9 0 6 7 8 9 0 6 7 8 9 0 6 7 8 9 0 6 7 8 9 0 " \
    "6 7 8 9 0 6 7 8 9 0 6 7 8 9 0 6 7 8 9 0 6 7 8 9 0 "
#define TEST_STRING_LEN 425

TestHighlighter*
TestHighlighter_new() {
    return (TestHighlighter*)Class_Make_Obj(TESTHIGHLIGHTER);
}

static void
test_Raw_Excerpt(TestBatchRunner *runner, Searcher *searcher, Obj *query) {
    String *content = (String*)SSTR_WRAP_UTF8("content", 7);
    Highlighter *highlighter = Highlighter_new(searcher, query, content, 6);
    int32_t top;
    String *raw_excerpt;

    String *field_val = (String *)SSTR_WRAP_UTF8("Ook.  Urk.  Ick.  ", 18);
    VArray *spans = VA_new(1);
    VA_Push(spans, (Obj*)Span_new(0, 18, 1.0f));
    HeatMap *heat_map = HeatMap_new(spans, 133);
    DECREF(spans);
    raw_excerpt = Highlighter_Raw_Excerpt(highlighter, field_val, &top,
                                          heat_map);
    TEST_TRUE(runner,
              Str_Equals_Utf8(raw_excerpt, "Ook.", 4),
              "Raw_Excerpt at top %s", Str_Get_Ptr8(raw_excerpt));
    TEST_TRUE(runner,
              top == 0,
              "top is 0");
    DECREF(raw_excerpt);
    DECREF(heat_map);

    spans = VA_new(1);
    VA_Push(spans, (Obj*)Span_new(6, 12, 1.0f));
    heat_map = HeatMap_new(spans, 133);
    DECREF(spans);
    raw_excerpt = Highlighter_Raw_Excerpt(highlighter, field_val, &top,
                                          heat_map);
    TEST_TRUE(runner,
              Str_Equals_Utf8(raw_excerpt, "Urk.", 4),
              "Raw_Excerpt in middle, with 2 bounds");
    TEST_TRUE(runner,
              top == 6,
              "top in the middle modified by Raw_Excerpt");
    DECREF(raw_excerpt);
    DECREF(heat_map);

    field_val = (String *)SSTR_WRAP_UTF8("Ook urk ick i.", 14);
    spans     = VA_new(1);
    VA_Push(spans, (Obj*)Span_new(12, 1, 1.0f));
    heat_map = HeatMap_new(spans, 133);
    DECREF(spans);
    raw_excerpt = Highlighter_Raw_Excerpt(highlighter, field_val, &top,
                                          heat_map);
    TEST_TRUE(runner,
              Str_Equals_Utf8(raw_excerpt, ELLIPSIS " i.", 6),
              "Ellipsis at top");
    TEST_TRUE(runner,
              top == 10,
              "top correct when leading ellipsis inserted");
    DECREF(heat_map);
    DECREF(raw_excerpt);

    field_val = (String *)SSTR_WRAP_UTF8("Urk.  Iz no good.", 17);
    spans     = VA_new(1);
    VA_Push(spans, (Obj*)Span_new(6, 2, 1.0f));
    heat_map = HeatMap_new(spans, 133);
    DECREF(spans);
    raw_excerpt = Highlighter_Raw_Excerpt(highlighter, field_val, &top,
                                          heat_map);
    TEST_TRUE(runner,
              Str_Equals_Utf8(raw_excerpt, "Iz no" ELLIPSIS, 8),
              "Ellipsis at end");
    TEST_TRUE(runner,
              top == 6,
              "top trimmed");
    DECREF(heat_map);
    DECREF(raw_excerpt);

    // Words longer than excerpt len

    field_val = (String *)SSTR_WRAP_UTF8("abc/def/ghi/jkl/mno", 19);

    spans = VA_new(1);
    VA_Push(spans, (Obj*)Span_new(0, 3, 1.0f));
    heat_map = HeatMap_new(spans, 133);
    DECREF(spans);
    raw_excerpt = Highlighter_Raw_Excerpt(highlighter, field_val, &top,
                                          heat_map);
    TEST_TRUE(runner,
              Str_Equals_Utf8(raw_excerpt, "abc/d" ELLIPSIS, 8),
              "Long word at top");
    DECREF(heat_map);
    DECREF(raw_excerpt);

    spans = VA_new(1);
    VA_Push(spans, (Obj*)Span_new(8, 3, 1.0f));
    heat_map = HeatMap_new(spans, 133);
    DECREF(spans);
    raw_excerpt = Highlighter_Raw_Excerpt(highlighter, field_val, &top,
                                          heat_map);
    TEST_TRUE(runner,
              Str_Equals_Utf8(raw_excerpt, ELLIPSIS " f/g" ELLIPSIS, 10),
              "Long word in middle");
    DECREF(heat_map);
    DECREF(raw_excerpt);

    DECREF(highlighter);
}

static void
test_Highlight_Excerpt(TestBatchRunner *runner, Searcher *searcher, Obj *query) {
    String *content = (String*)SSTR_WRAP_UTF8("content", 7);
    Highlighter *highlighter = Highlighter_new(searcher, query, content, 3);
    String *highlighted;

    VArray *spans = VA_new(1);
    VA_Push(spans, (Obj*)Span_new(2, 1, 0.0f));
    String *raw_excerpt = (String *)SSTR_WRAP_UTF8("a b c", 5);
    highlighted = Highlighter_Highlight_Excerpt(highlighter, spans,
                                                raw_excerpt, 0);
    TEST_TRUE(runner,
              Str_Equals_Utf8(highlighted, "a <strong>b</strong> c", 22),
              "basic Highlight_Excerpt");
    DECREF(highlighted);
    DECREF(spans);

    spans = VA_new(2);
    VA_Push(spans, (Obj*)Span_new(0, 1, 1.0f));
    VA_Push(spans, (Obj*)Span_new(10, 10, 1.0f));
    raw_excerpt = (String *)SSTR_WRAP_UTF8(PHI, 2);
    highlighted = Highlighter_Highlight_Excerpt(highlighter, spans,
                                                raw_excerpt, 0);
    TEST_TRUE(runner,
              Str_Equals_Utf8(highlighted, "<strong>&#934;</strong>", 23),
              "don't surround spans off end of raw excerpt.");
    DECREF(highlighted);
    DECREF(spans);

    spans = VA_new(1);
    VA_Push(spans, (Obj*)Span_new(3, 1, 1.0f));
    raw_excerpt = (String *)SSTR_WRAP_UTF8(PHI " " PHI " " PHI, 8);
    highlighted = Highlighter_Highlight_Excerpt(highlighter, spans,
                                                raw_excerpt, 1);
    TEST_TRUE(runner,
              Str_Equals_Utf8(highlighted,
                            "&#934; <strong>&#934;</strong> &#934;", 37),
              "Highlight_Excerpt pays attention to offset");
    DECREF(highlighted);
    DECREF(spans);

    spans = VA_new(4);
    VA_Push(spans, (Obj*)Span_new(2, 10, 1.0f));
    VA_Push(spans, (Obj*)Span_new(2,  4, 1.0f));
    VA_Push(spans, (Obj*)Span_new(8,  9, 1.0f));
    VA_Push(spans, (Obj*)Span_new(8,  4, 1.0f));
    raw_excerpt = (String *)SSTR_WRAP_UTF8(PHI " Oook. Urk. Ick. " PHI, 21);
    highlighted = Highlighter_Highlight_Excerpt(highlighter, spans,
                                                raw_excerpt, 0);
    TEST_TRUE(runner,
              Str_Equals_Utf8(highlighted,
                            "&#934; <strong>Oook. Urk. Ick.</strong> &#934;",
                            46),
              "Highlight_Excerpt works with overlapping spans");
    DECREF(highlighted);
    DECREF(spans);

    DECREF(highlighter);
}

static void
test_Create_Excerpt(TestBatchRunner *runner, Searcher *searcher, Obj *query,
                    Hits *hits) {
    String *content = (String*)SSTR_WRAP_UTF8("content", 7);
    Highlighter *highlighter = Highlighter_new(searcher, query, content, 200);

    HitDoc *hit = Hits_Next(hits);
    String *excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(runner,
              Str_Find_Utf8(excerpt,
                           "<strong>&#934;</strong> a b c d <strong>x y z</strong>",
                           54) >= 0,
              "highlighter tagged phrase and single term");
    DECREF(excerpt);

    String *pre_tag = (String*)SSTR_WRAP_UTF8("\x1B[1m", 4);
    Highlighter_Set_Pre_Tag(highlighter, pre_tag);
    String *post_tag = (String*)SSTR_WRAP_UTF8("\x1B[0m", 4);
    Highlighter_Set_Post_Tag(highlighter, post_tag);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(runner,
              Str_Find_Utf8(excerpt,
                          "\x1B[1m&#934;\x1B[0m a b c d \x1B[1mx y z\x1B[0m",
                          36) >= 0,
              "set_pre_tag and set_post_tag");
    DECREF(excerpt);
    DECREF(hit);

    hit = Hits_Next(hits);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(runner,
              Str_Find_Utf8(excerpt, "x", 1) >= 0,
              "excerpt field with partial hit doesn't cause highlighter freakout");
    DECREF(excerpt);
    DECREF(hit);
    DECREF(highlighter);

    query = (Obj*)SSTR_WRAP_UTF8("x \"x y z\" AND b", 15);
    hits = Searcher_Hits(searcher, query, 0, 10, NULL);
    highlighter = Highlighter_new(searcher, query, content, 200);
    hit = Hits_Next(hits);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(runner,
              Str_Find_Utf8(excerpt,
                          "<strong>b</strong> c d <strong>x y z</strong>",
                          45) >= 0,
              "query with same word in both phrase and term doesn't cause freakout");
    DECREF(excerpt);
    DECREF(hit);
    DECREF(highlighter);
    DECREF(hits);

    query = (Obj*)SSTR_WRAP_UTF8("blind", 5);
    hits = Searcher_Hits(searcher, query, 0, 10, NULL);
    highlighter = Highlighter_new(searcher, query, content, 200);
    hit = Hits_Next(hits);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(runner,
              Str_Find_Utf8(excerpt, "&quot;", 6) >= 0,
              "HTML entity encoded properly");
    DECREF(excerpt);
    DECREF(hit);
    DECREF(highlighter);
    DECREF(hits);

    query = (Obj*)SSTR_WRAP_UTF8("why", 3);
    hits = Searcher_Hits(searcher, query, 0, 10, NULL);
    highlighter = Highlighter_new(searcher, query, content, 200);
    hit = Hits_Next(hits);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(runner,
              Str_Find_Utf8(excerpt, "&#934;", 6) == -1,
              "no ellipsis for short excerpt");
    DECREF(excerpt);
    DECREF(hit);
    DECREF(highlighter);
    DECREF(hits);

    Obj *term = (Obj*)SSTR_WRAP_UTF8("x", 1);
    query = (Obj*)TermQuery_new(content, term);
    hits = Searcher_Hits(searcher, query, 0, 10, NULL);
    hit = Hits_Next(hits);
    highlighter = Highlighter_new(searcher, query, content, 200);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(runner,
              Str_Find_Utf8(excerpt, "strong", 5) >= 0,
              "specify field highlights correct field...");
    DECREF(excerpt);
    DECREF(highlighter);
    String *alt = (String*)SSTR_WRAP_UTF8("alt", 3);
    highlighter = Highlighter_new(searcher, query, alt, 200);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(runner,
              Str_Find_Utf8(excerpt, "strong", 5) == -1,
              "... but not another field");
    DECREF(excerpt);
    DECREF(highlighter);
    DECREF(hit);
    DECREF(hits);
    DECREF(query);
}

static void
test_highlighting(TestBatchRunner *runner) {
    Schema *schema = Schema_new();
    StandardTokenizer *tokenizer = StandardTokenizer_new();
    FullTextType *plain_type = FullTextType_new((Analyzer*)tokenizer);
    FullTextType_Set_Highlightable(plain_type, true);
    FullTextType *dunked_type = FullTextType_new((Analyzer*)tokenizer);
    FullTextType_Set_Highlightable(dunked_type, true);
    FullTextType_Set_Boost(dunked_type, 0.1f);
    String *content = (String*)SSTR_WRAP_UTF8("content", 7);
    Schema_Spec_Field(schema, content, (FieldType*)plain_type);
    String *alt = (String*)SSTR_WRAP_UTF8("alt", 3);
    Schema_Spec_Field(schema, alt, (FieldType*)dunked_type);
    DECREF(plain_type);
    DECREF(dunked_type);
    DECREF(tokenizer);

    RAMFolder *folder = RAMFolder_new(NULL);
    Indexer *indexer = Indexer_new(schema, (Obj*)folder, NULL, 0);

    Doc *doc = Doc_new(NULL, 0);
    String *string = (String *)SSTR_WRAP_UTF8(TEST_STRING, TEST_STRING_LEN);
    Doc_Store(doc, content, (Obj*)string);
    Indexer_Add_Doc(indexer, doc, 1.0f);
    DECREF(doc);

    doc = Doc_new(NULL, 0);
    string = (String *)SSTR_WRAP_UTF8("\"I see,\" said the blind man.", 28);
    Doc_Store(doc, content, (Obj*)string);
    Indexer_Add_Doc(indexer, doc, 1.0f);
    DECREF(doc);

    doc = Doc_new(NULL, 0);
    string = (String *)SSTR_WRAP_UTF8("x but not why or 2ee", 20);
    Doc_Store(doc, content, (Obj*)string);
    string = (String *)SSTR_WRAP_UTF8(TEST_STRING
                                     " and extra stuff so it scores lower",
                                     TEST_STRING_LEN + 35);
    Doc_Store(doc, alt, (Obj*)string);
    Indexer_Add_Doc(indexer, doc, 1.0f);
    DECREF(doc);

    Indexer_Commit(indexer);
    DECREF(indexer);

    Searcher *searcher = (Searcher*)IxSearcher_new((Obj*)folder);
    Obj *query = (Obj*)SSTR_WRAP_UTF8("\"x y z\" AND " PHI, 14);
    Hits *hits = Searcher_Hits(searcher, query, 0, 10, NULL);

    test_Raw_Excerpt(runner, searcher, query);
    test_Highlight_Excerpt(runner, searcher, query);
    test_Create_Excerpt(runner, searcher, query, hits);

    DECREF(hits);
    DECREF(searcher);
    DECREF(folder);
    DECREF(schema);
}

static void
test_hl_selection(TestBatchRunner *runner) {
    Schema *schema = Schema_new();
    StandardTokenizer *tokenizer = StandardTokenizer_new();
    FullTextType *plain_type = FullTextType_new((Analyzer*)tokenizer);
    FullTextType_Set_Highlightable(plain_type, true);
    String *content = (String*)SSTR_WRAP_UTF8("content", 7);
    Schema_Spec_Field(schema, content, (FieldType*)plain_type);
    DECREF(plain_type);
    DECREF(tokenizer);

    RAMFolder *folder = RAMFolder_new(NULL);
    Indexer *indexer = Indexer_new(schema, (Obj*)folder, NULL, 0);

    static char test_string[] =
        "bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla. "
        "bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla. "
        "bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla. "
        "bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla. "
        "bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla. "
        "bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla NNN bla. "
        "bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla. "
        "bla bla bla MMM bla bla bla bla bla bla bla bla bla bla bla bla. "
        "bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla. "
        "bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla. "
        "bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla. "
        "bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla. "
        "bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla. ";
    Doc *doc = Doc_new(NULL, 0);
    String *string = (String *)SSTR_WRAP_UTF8(test_string, strlen(test_string));
    Doc_Store(doc, content, (Obj*)string);
    Indexer_Add_Doc(indexer, doc, 1.0f);
    DECREF(doc);

    Indexer_Commit(indexer);
    DECREF(indexer);

    Searcher *searcher = (Searcher*)IxSearcher_new((Obj*)folder);
    Obj *query = (Obj*)SSTR_WRAP_UTF8("NNN MMM", 7);
    Highlighter *highlighter = Highlighter_new(searcher, query, content, 200);
    Hits *hits = Searcher_Hits(searcher, query, 0, 10, NULL);
    HitDoc *hit = Hits_Next(hits);
    String *excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    String *mmm = (String*)SSTR_WRAP_UTF8("MMM", 3);
    String *nnn = (String*)SSTR_WRAP_UTF8("NNN", 3);
    TEST_TRUE(runner, Str_Find(excerpt, mmm) >= 0 || Str_Find(excerpt, nnn) >= 0,
              "Sentence boundary algo doesn't chop terms");

    DECREF(excerpt);
    DECREF(hit);
    DECREF(hits);
    DECREF(highlighter);
    DECREF(searcher);
    DECREF(folder);
    DECREF(schema);
}

void
TestHighlighter_Run_IMP(TestHighlighter *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 23);
    test_highlighting(runner);
    test_hl_selection(runner);
}


