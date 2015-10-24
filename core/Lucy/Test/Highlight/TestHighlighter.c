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
    String *content = SSTR_WRAP_C("content");
    Highlighter *highlighter = Highlighter_new(searcher, query, content, 6);
    int32_t top;
    String *raw_excerpt;

    String *field_val = SSTR_WRAP_C("Ook.  Urk.  Ick.  ");
    Vector *spans = Vec_new(1);
    Vec_Push(spans, (Obj*)Span_new(0, 18, 1.0f));
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

    spans = Vec_new(1);
    Vec_Push(spans, (Obj*)Span_new(6, 12, 1.0f));
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

    field_val = SSTR_WRAP_C("Ook urk ick i.");
    spans     = Vec_new(1);
    Vec_Push(spans, (Obj*)Span_new(12, 1, 1.0f));
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

    field_val = SSTR_WRAP_C("Urk.  Iz no good.");
    spans     = Vec_new(1);
    Vec_Push(spans, (Obj*)Span_new(6, 2, 1.0f));
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

    field_val = SSTR_WRAP_C("abc/def/ghi/jkl/mno");

    spans = Vec_new(1);
    Vec_Push(spans, (Obj*)Span_new(0, 3, 1.0f));
    heat_map = HeatMap_new(spans, 133);
    DECREF(spans);
    raw_excerpt = Highlighter_Raw_Excerpt(highlighter, field_val, &top,
                                          heat_map);
    TEST_TRUE(runner,
              Str_Equals_Utf8(raw_excerpt, "abc/d" ELLIPSIS, 8),
              "Long word at top");
    DECREF(heat_map);
    DECREF(raw_excerpt);

    spans = Vec_new(1);
    Vec_Push(spans, (Obj*)Span_new(8, 3, 1.0f));
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
    String *content = SSTR_WRAP_C("content");
    Highlighter *highlighter = Highlighter_new(searcher, query, content, 3);
    String *highlighted;

    Vector *spans = Vec_new(1);
    Vec_Push(spans, (Obj*)Span_new(2, 1, 0.0f));
    String *raw_excerpt = SSTR_WRAP_C("a b c");
    highlighted = Highlighter_Highlight_Excerpt(highlighter, spans,
                                                raw_excerpt, 0);
    TEST_TRUE(runner,
              Str_Equals_Utf8(highlighted, "a <strong>b</strong> c", 22),
              "basic Highlight_Excerpt");
    DECREF(highlighted);
    DECREF(spans);

    spans = Vec_new(2);
    Vec_Push(spans, (Obj*)Span_new(0, 1, 1.0f));
    Vec_Push(spans, (Obj*)Span_new(10, 10, 1.0f));
    raw_excerpt = SSTR_WRAP_C(PHI);
    highlighted = Highlighter_Highlight_Excerpt(highlighter, spans,
                                                raw_excerpt, 0);
    TEST_TRUE(runner,
              Str_Equals_Utf8(highlighted, "<strong>&#934;</strong>", 23),
              "don't surround spans off end of raw excerpt.");
    DECREF(highlighted);
    DECREF(spans);

    spans = Vec_new(1);
    Vec_Push(spans, (Obj*)Span_new(3, 1, 1.0f));
    raw_excerpt = SSTR_WRAP_C(PHI " " PHI " " PHI);
    highlighted = Highlighter_Highlight_Excerpt(highlighter, spans,
                                                raw_excerpt, 1);
    TEST_TRUE(runner,
              Str_Equals_Utf8(highlighted,
                            "&#934; <strong>&#934;</strong> &#934;", 37),
              "Highlight_Excerpt pays attention to offset");
    DECREF(highlighted);
    DECREF(spans);

    spans = Vec_new(4);
    Vec_Push(spans, (Obj*)Span_new(2, 10, 1.0f));
    Vec_Push(spans, (Obj*)Span_new(2,  4, 1.0f));
    Vec_Push(spans, (Obj*)Span_new(8,  9, 1.0f));
    Vec_Push(spans, (Obj*)Span_new(8,  4, 1.0f));
    raw_excerpt = SSTR_WRAP_C(PHI " Oook. Urk. Ick. " PHI);
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
    String *content = SSTR_WRAP_C("content");
    Highlighter *highlighter = Highlighter_new(searcher, query, content, 200);

    HitDoc *hit = Hits_Next(hits);
    String *excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(runner,
              Str_Contains_Utf8(excerpt,
                                "<strong>&#934;</strong> a b c d <strong>x y z</strong>",
                                54),
              "highlighter tagged phrase and single term");
    DECREF(excerpt);

    String *pre_tag = SSTR_WRAP_C("\x1B[1m");
    Highlighter_Set_Pre_Tag(highlighter, pre_tag);
    String *post_tag = SSTR_WRAP_C("\x1B[0m");
    Highlighter_Set_Post_Tag(highlighter, post_tag);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(runner,
              Str_Contains_Utf8(excerpt,
                                "\x1B[1m&#934;\x1B[0m a b c d \x1B[1mx y z\x1B[0m",
                                36),
              "set_pre_tag and set_post_tag");
    DECREF(excerpt);
    DECREF(hit);

    hit = Hits_Next(hits);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(runner,
              Str_Contains_Utf8(excerpt, "x", 1),
              "excerpt field with partial hit doesn't cause highlighter freakout");
    DECREF(excerpt);
    DECREF(hit);
    DECREF(highlighter);

    query = (Obj*)SSTR_WRAP_C("x \"x y z\" AND b");
    hits = Searcher_Hits(searcher, query, 0, 10, NULL);
    highlighter = Highlighter_new(searcher, query, content, 200);
    hit = Hits_Next(hits);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(runner,
              Str_Contains_Utf8(excerpt,
                                "<strong>b</strong> c d <strong>x y z</strong>",
                                45),
              "query with same word in both phrase and term doesn't cause freakout");
    DECREF(excerpt);
    DECREF(hit);
    DECREF(highlighter);
    DECREF(hits);

    query = (Obj*)SSTR_WRAP_C("blind");
    hits = Searcher_Hits(searcher, query, 0, 10, NULL);
    highlighter = Highlighter_new(searcher, query, content, 200);
    hit = Hits_Next(hits);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(runner,
              Str_Contains_Utf8(excerpt, "&quot;", 6),
              "HTML entity encoded properly");
    DECREF(excerpt);
    DECREF(hit);
    DECREF(highlighter);
    DECREF(hits);

    query = (Obj*)SSTR_WRAP_C("why");
    hits = Searcher_Hits(searcher, query, 0, 10, NULL);
    highlighter = Highlighter_new(searcher, query, content, 200);
    hit = Hits_Next(hits);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_FALSE(runner,
               Str_Contains_Utf8(excerpt, "&#934;", 6),
               "no ellipsis for short excerpt");
    DECREF(excerpt);
    DECREF(hit);
    DECREF(highlighter);
    DECREF(hits);

    Obj *term = (Obj*)SSTR_WRAP_C("x");
    query = (Obj*)TermQuery_new(content, term);
    hits = Searcher_Hits(searcher, query, 0, 10, NULL);
    hit = Hits_Next(hits);
    highlighter = Highlighter_new(searcher, query, content, 200);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(runner,
              Str_Contains_Utf8(excerpt, "strong", 5),
              "specify field highlights correct field...");
    DECREF(excerpt);
    DECREF(highlighter);
    String *alt = SSTR_WRAP_C("alt");
    highlighter = Highlighter_new(searcher, query, alt, 200);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_FALSE(runner,
               Str_Contains_Utf8(excerpt, "strong", 5),
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
    String *content = SSTR_WRAP_C("content");
    Schema_Spec_Field(schema, content, (FieldType*)plain_type);
    String *alt = SSTR_WRAP_C("alt");
    Schema_Spec_Field(schema, alt, (FieldType*)dunked_type);
    DECREF(plain_type);
    DECREF(dunked_type);
    DECREF(tokenizer);

    RAMFolder *folder = RAMFolder_new(NULL);
    Indexer *indexer = Indexer_new(schema, (Obj*)folder, NULL, 0);

    Doc *doc = Doc_new(NULL, 0);
    String *string = SSTR_WRAP_C(TEST_STRING);
    Doc_Store(doc, content, (Obj*)string);
    Indexer_Add_Doc(indexer, doc, 1.0f);
    DECREF(doc);

    doc = Doc_new(NULL, 0);
    string = SSTR_WRAP_C("\"I see,\" said the blind man.");
    Doc_Store(doc, content, (Obj*)string);
    Indexer_Add_Doc(indexer, doc, 1.0f);
    DECREF(doc);

    doc = Doc_new(NULL, 0);
    string = SSTR_WRAP_C("x but not why or 2ee");
    Doc_Store(doc, content, (Obj*)string);
    string = SSTR_WRAP_C(TEST_STRING " and extra stuff so it scores lower");
    Doc_Store(doc, alt, (Obj*)string);
    Indexer_Add_Doc(indexer, doc, 1.0f);
    DECREF(doc);

    Indexer_Commit(indexer);
    DECREF(indexer);

    Searcher *searcher = (Searcher*)IxSearcher_new((Obj*)folder);
    Obj *query = (Obj*)SSTR_WRAP_C("\"x y z\" AND " PHI);
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
    String *content = SSTR_WRAP_C("content");
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
    String *string = SSTR_WRAP_C(test_string);
    Doc_Store(doc, content, (Obj*)string);
    Indexer_Add_Doc(indexer, doc, 1.0f);
    DECREF(doc);

    Indexer_Commit(indexer);
    DECREF(indexer);

    Searcher *searcher = (Searcher*)IxSearcher_new((Obj*)folder);
    Obj *query = (Obj*)SSTR_WRAP_C("NNN MMM");
    Highlighter *highlighter = Highlighter_new(searcher, query, content, 200);
    Hits *hits = Searcher_Hits(searcher, query, 0, 10, NULL);
    HitDoc *hit = Hits_Next(hits);
    String *excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    String *mmm = SSTR_WRAP_C("MMM");
    String *nnn = SSTR_WRAP_C("NNN");
    TEST_TRUE(runner, Str_Contains(excerpt, mmm) || Str_Contains(excerpt, nnn),
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


