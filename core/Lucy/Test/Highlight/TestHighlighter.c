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

#define C_LUCY_TESTHIGHLIGHTER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Highlight/TestHighlighter.h"
#include "Lucy/Highlight/Highlighter.h"

#include "Lucy/Analysis/RegexTokenizer.h"
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

static void
test_Find_Best_Fragment(TestBatch *batch, Searcher *searcher, Obj *query) {
    CharBuf *content = (CharBuf*)ZCB_WRAP_STR("content", 7);
    Highlighter *highlighter = Highlighter_new(searcher, query, content, 3);
    ViewCharBuf *target = (ViewCharBuf*)ZCB_BLANK();

    VArray *spans = VA_new(1);
    VA_Push(spans, (Obj*)Span_new(2, 1, 1.0f));
    HeatMap *heat_map = HeatMap_new(spans, 133);
    DECREF(spans);
    CharBuf *field_val = (CharBuf *)ZCB_WRAP_STR("a " PHI " " PHI " b c", 11);
    int32_t top = Highlighter_Find_Best_Fragment(highlighter, field_val,
                                                 target, heat_map);
    TEST_TRUE(batch,
              CB_Equals_Str((CharBuf *)target, PHI " " PHI " b", 7),
              "Find_Best_Fragment");
    TEST_TRUE(batch,
              top == 2,
              "correct offset returned by Find_Best_Fragment");
    field_val = (CharBuf *)ZCB_WRAP_STR("aa" PHI, 4);
    top = Highlighter_Find_Best_Fragment(highlighter, field_val,
                                         target, heat_map);
    TEST_TRUE(batch,
              CB_Equals_Str((CharBuf *)target, "aa" PHI, 4),
              "Find_Best_Fragment returns whole field when field is short");
    TEST_TRUE(batch,
              top == 0,
              "correct offset");
    DECREF(heat_map);

    spans = VA_new(1);
    VA_Push(spans, (Obj*)Span_new(6, 2, 1.0f));
    heat_map = HeatMap_new(spans, 133);
    DECREF(spans);
    field_val = (CharBuf *)ZCB_WRAP_STR("aaaab" PHI PHI, 9);
    top = Highlighter_Find_Best_Fragment(highlighter, field_val,
                                         target, heat_map);
    TEST_TRUE(batch,
              CB_Equals_Str((CharBuf *)target, "b" PHI PHI, 5),
              "Find_Best_Fragment shifts left to deal with overrun");
    TEST_TRUE(batch,
              top == 4,
              "correct offset");
    DECREF(heat_map);

    spans = VA_new(1);
    VA_Push(spans, (Obj*)Span_new(0, 1, 1.0f));
    heat_map = HeatMap_new(spans, 133);
    DECREF(spans);
    field_val = (CharBuf *)ZCB_WRAP_STR("a" PHI "bcde", 7);
    top = Highlighter_Find_Best_Fragment(highlighter, field_val,
                                         target, heat_map);
    TEST_TRUE(batch,
              CB_Equals_Str((CharBuf *)target, "a" PHI "bcd", 6),
              "Find_Best_Fragment start at field beginning");
    TEST_TRUE(batch,
              top == 0,
              "correct offset");
    DECREF(heat_map);

    DECREF(highlighter);
}

static void
test_Raw_Excerpt(TestBatch *batch, Searcher *searcher, Obj *query) {
    CharBuf *content = (CharBuf*)ZCB_WRAP_STR("content", 7);
    Highlighter *highlighter = Highlighter_new(searcher, query, content, 6);

    CharBuf *field_val   = (CharBuf *)ZCB_WRAP_STR("Ook.  Urk.  Ick.  ", 18);
    CharBuf *fragment    = (CharBuf *)ZCB_WRAP_STR("Ook.  Urk.", 10);
    CharBuf *raw_excerpt = CB_new(0);
    VArray *spans = VA_new(1);
    VA_Push(spans, (Obj*)Span_new(0, 18, 1.0f));
    HeatMap *heat_map = HeatMap_new(spans, 133);
    DECREF(spans);
    VArray *sentences = VA_new(2);
    VA_Push(sentences, (Obj*)Span_new(0, 4, 0.0f));
    VA_Push(sentences, (Obj*)Span_new(6, 4, 0.0f));
    int32_t top = Highlighter_Raw_Excerpt(highlighter, field_val, fragment,
                                          raw_excerpt, 0, heat_map, sentences);
    TEST_TRUE(batch,
              CB_Equals_Str(raw_excerpt, "Ook.", 4),
              "Raw_Excerpt at top");
    TEST_TRUE(batch,
              top == 0,
              "top still 0");
    DECREF(sentences);
    DECREF(raw_excerpt);

    fragment    = (CharBuf *)ZCB_WRAP_STR(".  Urk.  I", 10);
    raw_excerpt = CB_new(0);
    sentences   = VA_new(2);
    VA_Push(sentences, (Obj*)Span_new(6, 4, 0.0f));
    VA_Push(sentences, (Obj*)Span_new(12, 4, 0.0f));
    top = Highlighter_Raw_Excerpt(highlighter, field_val, fragment,
                                  raw_excerpt, 3, heat_map, sentences);
    TEST_TRUE(batch,
              CB_Equals_Str(raw_excerpt, "Urk.", 4),
              "Raw_Excerpt in middle, with 2 bounds");
    TEST_TRUE(batch,
              top == 6,
              "top in the middle modified by Raw_Excerpt");
    DECREF(sentences);
    DECREF(heat_map);
    DECREF(raw_excerpt);

    field_val   = (CharBuf *)ZCB_WRAP_STR("Ook urk ick i.", 14);
    fragment    = (CharBuf *)ZCB_WRAP_STR("ick i.", 6);
    raw_excerpt = CB_new(0);
    spans       = VA_new(1);
    VA_Push(spans, (Obj*)Span_new(0, 14, 1.0f));
    heat_map = HeatMap_new(spans, 133);
    DECREF(spans);
    sentences = VA_new(1);
    VA_Push(sentences, (Obj*)Span_new(0, 14, 0.0f));
    top = Highlighter_Raw_Excerpt(highlighter, field_val, fragment,
                                  raw_excerpt, 8, heat_map, sentences);
    TEST_TRUE(batch,
              CB_Equals_Str(raw_excerpt, ELLIPSIS " i.", 6),
              "Ellipsis at top");
    TEST_TRUE(batch,
              top == 10,
              "top correct when leading ellipsis inserted");
    DECREF(sentences);
    DECREF(heat_map);
    DECREF(raw_excerpt);

    field_val   = (CharBuf *)ZCB_WRAP_STR("Urk.  Iz no good.", 17);
    fragment    = (CharBuf *)ZCB_WRAP_STR("  Iz no go", 10);
    raw_excerpt = CB_new(0);
    spans       = VA_new(1);
    VA_Push(spans, (Obj*)Span_new(0, 17, 1.0f));
    heat_map = HeatMap_new(spans, 133);
    DECREF(spans);
    sentences = VA_new(1);
    VA_Push(sentences, (Obj*)Span_new(6, 11, 0.0f));
    top = Highlighter_Raw_Excerpt(highlighter, field_val, fragment,
                                  raw_excerpt, 4, heat_map, sentences);
    TEST_TRUE(batch,
              CB_Equals_Str(raw_excerpt, "Iz no" ELLIPSIS, 8),
              "Ellipsis at end");
    TEST_TRUE(batch,
              top == 6,
              "top trimmed");
    DECREF(sentences);
    DECREF(heat_map);
    DECREF(raw_excerpt);

    // Words longer than excerpt len

    field_val   = (CharBuf *)ZCB_WRAP_STR("abc/def/ghi/jkl/mno", 19);
    sentences = VA_new(1);
    VA_Push(sentences, (Obj*)Span_new(0, 19, 0.0f));

    raw_excerpt = CB_new(0);
    spans       = VA_new(1);
    VA_Push(spans, (Obj*)Span_new(0, 3, 1.0f));
    heat_map = HeatMap_new(spans, 133);
    DECREF(spans);
    top = Highlighter_Raw_Excerpt(highlighter, field_val, field_val,
                                  raw_excerpt, 0, heat_map, sentences);
    TEST_TRUE(batch,
              CB_Equals_Str(raw_excerpt, "abc/d" ELLIPSIS, 8),
              "Long word");
    DECREF(heat_map);
    DECREF(raw_excerpt);

    raw_excerpt = CB_new(0);
    spans       = VA_new(1);
    VA_Push(spans, (Obj*)Span_new(8, 3, 1.0f));
    heat_map = HeatMap_new(spans, 133);
    DECREF(spans);
    top = Highlighter_Raw_Excerpt(highlighter, field_val, field_val,
                                  raw_excerpt, 0, heat_map, sentences);
    TEST_TRUE(batch,
              CB_Equals_Str(raw_excerpt, ELLIPSIS " c/d" ELLIPSIS, 10),
              "Long word");
    DECREF(heat_map);
    DECREF(raw_excerpt);

    DECREF(sentences);

    DECREF(highlighter);
}

static void
test_Highlight_Excerpt(TestBatch *batch, Searcher *searcher, Obj *query) {
    CharBuf *content = (CharBuf*)ZCB_WRAP_STR("content", 7);
    Highlighter *highlighter = Highlighter_new(searcher, query, content, 3);

    VArray *spans = VA_new(1);
    VA_Push(spans, (Obj*)Span_new(2, 1, 0.0f));
    CharBuf *raw_excerpt = (CharBuf *)ZCB_WRAP_STR("a b c", 5);
    CharBuf *highlighted = CB_new(0);
    Highlighter_Highlight_Excerpt(highlighter, spans, raw_excerpt,
                                  highlighted, 0);
    TEST_TRUE(batch,
              CB_Equals_Str(highlighted, "a <strong>b</strong> c", 22),
              "basic Highlight_Excerpt");
    DECREF(highlighted);
    DECREF(spans);

    spans = VA_new(2);
    VA_Push(spans, (Obj*)Span_new(0, 1, 1.0f));
    VA_Push(spans, (Obj*)Span_new(10, 10, 1.0f));
    raw_excerpt = (CharBuf *)ZCB_WRAP_STR(PHI, 2);
    highlighted = CB_new(0);
    Highlighter_Highlight_Excerpt(highlighter, spans, raw_excerpt,
                                  highlighted, 0);
    TEST_TRUE(batch,
              CB_Equals_Str(highlighted, "<strong>&#934;</strong>", 23),
              "don't surround spans off end of raw excerpt.");
    DECREF(highlighted);
    DECREF(spans);

    spans = VA_new(1);
    VA_Push(spans, (Obj*)Span_new(3, 1, 1.0f));
    raw_excerpt = (CharBuf *)ZCB_WRAP_STR(PHI " " PHI " " PHI, 8);
    highlighted = CB_new(0);
    Highlighter_Highlight_Excerpt(highlighter, spans, raw_excerpt,
                                  highlighted, 1);
    TEST_TRUE(batch,
              CB_Equals_Str(highlighted,
                            "&#934; <strong>&#934;</strong> &#934;", 37),
              "Highlight_Excerpt pays attention to offset");
    DECREF(highlighted);
    DECREF(spans);

    spans = VA_new(4);
    VA_Push(spans, (Obj*)Span_new(2, 10, 1.0f));
    VA_Push(spans, (Obj*)Span_new(2,  4, 1.0f));
    VA_Push(spans, (Obj*)Span_new(8,  9, 1.0f));
    VA_Push(spans, (Obj*)Span_new(8,  4, 1.0f));
    raw_excerpt = (CharBuf *)ZCB_WRAP_STR(PHI " Oook. Urk. Ick. " PHI, 21);
    highlighted = CB_new(0);
    Highlighter_Highlight_Excerpt(highlighter, spans, raw_excerpt,
                                  highlighted, 0);
    TEST_TRUE(batch,
              CB_Equals_Str(highlighted,
                            "&#934; <strong>Oook. Urk. Ick.</strong> &#934;",
                            46),
              "Highlight_Excerpt works with overlapping spans");
    DECREF(highlighted);
    DECREF(spans);

    DECREF(highlighter);
}

static void
test_Create_Excerpt(TestBatch *batch, Searcher *searcher, Obj *query,
                    Hits *hits) {
    CharBuf *content = (CharBuf*)ZCB_WRAP_STR("content", 7);
    Highlighter *highlighter = Highlighter_new(searcher, query, content, 200);

    HitDoc *hit = Hits_Next(hits);
    CharBuf *excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(batch,
              CB_Find_Str(excerpt,
                          "<strong>&#934;</strong> a b c d <strong>x y z</strong>",
                          54) >= 0,
              "highlighter tagged phrase and single term");
    DECREF(excerpt);

    CharBuf *pre_tag = (CharBuf*)ZCB_WRAP_STR("\x1B[1m", 4);
    Highlighter_Set_Pre_Tag(highlighter, pre_tag);
    CharBuf *post_tag = (CharBuf*)ZCB_WRAP_STR("\x1B[0m", 4);
    Highlighter_Set_Post_Tag(highlighter, post_tag);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(batch,
              CB_Find_Str(excerpt,
                          "\x1B[1m&#934;\x1B[0m a b c d \x1B[1mx y z\x1B[0m",
                          36) >= 0,
              "set_pre_tag and set_post_tag");
    DECREF(excerpt);
    DECREF(hit);

    hit = Hits_Next(hits);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(batch,
              CB_Find_Str(excerpt, "x", 1) >= 0,
              "excerpt field with partial hit doesn't cause highlighter freakout");
    DECREF(excerpt);
    DECREF(hit);
    DECREF(highlighter);

    query = (Obj*)ZCB_WRAP_STR("x \"x y z\" AND b", 15);
    hits = Searcher_Hits(searcher, query, 0, 10, NULL);
    highlighter = Highlighter_new(searcher, query, content, 200);
    hit = Hits_Next(hits);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(batch,
              CB_Find_Str(excerpt,
                          "<strong>b</strong> c d <strong>x y z</strong>",
                          45) >= 0,
              "query with same word in both phrase and term doesn't cause freakout");
    DECREF(excerpt);
    DECREF(hit);
    DECREF(highlighter);
    DECREF(hits);

    query = (Obj*)ZCB_WRAP_STR("blind", 5);
    hits = Searcher_Hits(searcher, query, 0, 10, NULL);
    highlighter = Highlighter_new(searcher, query, content, 200);
    hit = Hits_Next(hits);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(batch,
              CB_Find_Str(excerpt, "&quot;", 6) >= 0,
              "HTML entity encoded properly");
    DECREF(excerpt);
    DECREF(hit);
    DECREF(highlighter);
    DECREF(hits);

    query = (Obj*)ZCB_WRAP_STR("why", 3);
    hits = Searcher_Hits(searcher, query, 0, 10, NULL);
    highlighter = Highlighter_new(searcher, query, content, 200);
    hit = Hits_Next(hits);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(batch,
              CB_Find_Str(excerpt, "&#934;", 6) == -1,
              "no ellipsis for short excerpt");
    DECREF(excerpt);
    DECREF(hit);
    DECREF(highlighter);
    DECREF(hits);

    Obj *term = (Obj*)ZCB_WRAP_STR("x", 1);
    query = (Obj*)TermQuery_new(content, term);
    hits = Searcher_Hits(searcher, query, 0, 10, NULL);
    hit = Hits_Next(hits);
    highlighter = Highlighter_new(searcher, query, content, 200);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(batch,
              CB_Find_Str(excerpt, "strong", 5) >= 0,
              "specify field highlights correct field...");
    DECREF(excerpt);
    DECREF(highlighter);
    CharBuf *alt = (CharBuf*)ZCB_WRAP_STR("alt", 3);
    highlighter = Highlighter_new(searcher, query, alt, 200);
    excerpt = Highlighter_Create_Excerpt(highlighter, hit);
    TEST_TRUE(batch,
              CB_Find_Str(excerpt, "strong", 5) == -1,
              "... but not another field");
    DECREF(excerpt);
    DECREF(highlighter);
    DECREF(hit);
    DECREF(hits);
    DECREF(query);
}

static void
test_Find_Sentences(TestBatch *batch, Searcher *searcher, Obj *query) {
    CharBuf *content = (CharBuf*)ZCB_WRAP_STR("content", 7);
    Highlighter *highlighter = Highlighter_new(searcher, query, content, 200);
    CharBuf *text = (CharBuf*)ZCB_WRAP_STR(
                        "This is a sentence. This is a sentence. This is a sentence. "
                        "This is a sentence. This is a sentence. This is a sentence. "
                        "This is a sentence. This is a sentence. This is a sentence. "
                        "This is a sentence. This is a sentence. This is a sentence. "
                        "This is a sentence. This is a sentence. This is a sentence. ",
                        300);

    VArray *got = Highlighter_Find_Sentences(highlighter, text, 101, 50);
    VArray *wanted = VA_new(2);
    VA_push(wanted, (Obj*)Span_new(120, 19, 0.0f));
    VA_push(wanted, (Obj*)Span_new(140, 19, 0.0f));
    TEST_TRUE(batch,
              VA_Equals(got, (Obj*)wanted),
              "find_sentences with explicit args");
    DECREF(wanted);
    DECREF(got);

    got = Highlighter_Find_Sentences(highlighter, text, 101, 4);
    TEST_TRUE(batch,
              VA_Get_Size(got) == 0,
              "find_sentences with explicit args, finding nothing");
    DECREF(got);

    got = Highlighter_Find_Sentences(highlighter, text, 0, 0);
    wanted = VA_new(15);
    for (int i = 0; i < 15; ++i) {
        VA_push(wanted, (Obj*)Span_new(i * 20, 19, 0.0f));
    }
    TEST_TRUE(batch,
              VA_Equals(got, (Obj*)wanted),
              "find_sentences with default offset and length");
    DECREF(wanted);
    DECREF(got);

    text = (CharBuf*)ZCB_WRAP_STR(" Foo", 4);
    got = Highlighter_Find_Sentences(highlighter, text, 0, 0);
    wanted = VA_new(1);
    VA_push(wanted, (Obj*)Span_new(1, 3, 0.0f));
    TEST_TRUE(batch,
              VA_Equals(got, (Obj*)wanted),
              "Skip leading whitespace but get first sentence");
    DECREF(wanted);
    DECREF(got);

    DECREF(highlighter);
}

static void
test_highlighting(TestBatch *batch) {
    Schema *schema = Schema_new();
    RegexTokenizer *tokenizer = RegexTokenizer_new(NULL);
    FullTextType *plain_type = FullTextType_new((Analyzer*)tokenizer);
    FullTextType_Set_Highlightable(plain_type, true);
    FullTextType *dunked_type = FullTextType_new((Analyzer*)tokenizer);
    FullTextType_Set_Highlightable(dunked_type, true);
    FullTextType_Set_Boost(dunked_type, 0.1f);
    CharBuf *content = (CharBuf*)ZCB_WRAP_STR("content", 7);
    Schema_Spec_Field(schema, content, (FieldType*)plain_type);
    CharBuf *alt = (CharBuf*)ZCB_WRAP_STR("alt", 3);
    Schema_Spec_Field(schema, alt, (FieldType*)dunked_type);
    DECREF(plain_type);
    DECREF(dunked_type);
    DECREF(tokenizer);

    RAMFolder *folder = RAMFolder_new(NULL);
    Indexer *indexer = Indexer_new(schema, (Obj*)folder, NULL, 0);

    Doc *doc = Doc_new(NULL, 0);
    CharBuf *string = (CharBuf *)ZCB_WRAP_STR(TEST_STRING, TEST_STRING_LEN);
    Doc_Store(doc, content, (Obj*)string);
    Indexer_Add_Doc(indexer, doc, 1.0f);
    DECREF(doc);

    doc = Doc_new(NULL, 0);
    string = (CharBuf *)ZCB_WRAP_STR("\"I see,\" said the blind man.", 28);
    Doc_Store(doc, content, (Obj*)string);
    Indexer_Add_Doc(indexer, doc, 1.0f);
    DECREF(doc);

    doc = Doc_new(NULL, 0);
    string = (CharBuf *)ZCB_WRAP_STR("x but not why or 2ee", 20);
    Doc_Store(doc, content, (Obj*)string);
    string = (CharBuf *)ZCB_WRAP_STR(TEST_STRING
                                     " and extra stuff so it scores lower",
                                     TEST_STRING_LEN + 35);
    Doc_Store(doc, alt, (Obj*)string);
    Indexer_Add_Doc(indexer, doc, 1.0f);
    DECREF(doc);

    Indexer_Commit(indexer);
    DECREF(indexer);

    Searcher *searcher = (Searcher*)IxSearcher_new((Obj*)folder);
    Obj *query = (Obj*)ZCB_WRAP_STR("\"x y z\" AND " PHI, 14);
    Hits *hits = Searcher_Hits(searcher, query, 0, 10, NULL);

    test_Find_Best_Fragment(batch, searcher, query);
    test_Raw_Excerpt(batch, searcher, query);
    test_Highlight_Excerpt(batch, searcher, query);
    test_Create_Excerpt(batch, searcher, query, hits);
    test_Find_Sentences(batch, searcher, query);

    DECREF(hits);
    DECREF(searcher);
    DECREF(folder);
    DECREF(schema);
}

void
TestHighlighter_run_tests() {
    TestBatch *batch = TestBatch_new(34);

    TestBatch_Plan(batch);

    test_highlighting(batch);

    DECREF(batch);
}


