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


#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Plan/TestFieldMisc.h"

#include "Lucy/Analysis/EasyAnalyzer.h"
#include "Lucy/Analysis/StandardTokenizer.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Index/Indexer.h"
#include "Lucy/Plan/FullTextType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Plan/StringType.h"
#include "Lucy/Search/Hits.h"
#include "Lucy/Search/IndexSearcher.h"
#include "Lucy/Search/TermQuery.h"
#include "Lucy/Store/RAMFolder.h"

static String *analyzed_str;
static String *easy_analyzed_str;
static String *state_str;
static String *states_str;
static String *string_str;
static String *unindexed_but_analyzed_str;
static String *unindexed_unanalyzed_str;
static String *united_states_str;

TestFieldMisc*
TestFieldMisc_new() {
    return (TestFieldMisc*)Class_Make_Obj(TESTFIELDMISC);
}

static void
S_init_strings() {
    analyzed_str               = Str_newf("analyzed");
    easy_analyzed_str          = Str_newf("easy_analyzed");
    state_str                  = Str_newf("state");
    states_str                 = Str_newf("States");
    string_str                 = Str_newf("string");
    unindexed_but_analyzed_str = Str_newf("unindexed_but_analyzed");
    unindexed_unanalyzed_str   = Str_newf("unindexed_unanalyzed");
    united_states_str          = Str_newf("United States");
}

static void
S_destroy_strings() {
    DECREF(analyzed_str);
    DECREF(easy_analyzed_str);
    DECREF(state_str);
    DECREF(states_str);
    DECREF(string_str);
    DECREF(unindexed_but_analyzed_str);
    DECREF(unindexed_unanalyzed_str);
    DECREF(united_states_str);
}

static Schema*
S_create_schema() {
    Schema *schema = Schema_new();

    StandardTokenizer *tokenizer     = StandardTokenizer_new();
    String            *language      = Str_newf("en");
    EasyAnalyzer      *easy_analyzer = EasyAnalyzer_new(language);

    FullTextType *plain         = FullTextType_new((Analyzer*)tokenizer);
    FullTextType *easy_analyzed = FullTextType_new((Analyzer*)easy_analyzer);

    StringType *string_spec = StringType_new();

    FullTextType *unindexed_but_analyzed
        = FullTextType_new((Analyzer*)tokenizer);
    FullTextType_Set_Indexed(unindexed_but_analyzed, false);

    StringType *unindexed_unanalyzed = StringType_new();
    StringType_Set_Indexed(unindexed_unanalyzed, false);

    Schema_Spec_Field(schema, analyzed_str, (FieldType*)plain);
    Schema_Spec_Field(schema, easy_analyzed_str, (FieldType*)easy_analyzed);
    Schema_Spec_Field(schema, string_str, (FieldType*)string_spec);
    Schema_Spec_Field(schema, unindexed_but_analyzed_str,
                      (FieldType*)unindexed_but_analyzed);
    Schema_Spec_Field(schema, unindexed_unanalyzed_str,
                      (FieldType*)unindexed_unanalyzed);

    DECREF(unindexed_unanalyzed);
    DECREF(unindexed_but_analyzed);
    DECREF(string_spec);
    DECREF(easy_analyzed);
    DECREF(plain);
    DECREF(easy_analyzer);
    DECREF(language);
    DECREF(tokenizer);

    return schema;
}

static void
S_add_doc(Indexer *indexer, String *field_name) {
    Doc *doc = Doc_new(NULL, 0);
    Doc_Store(doc, field_name, (Obj*)united_states_str);
    Indexer_Add_Doc(indexer, doc, 1.0f);
    DECREF(doc);
}

static void
S_check(TestBatchRunner *runner, RAMFolder *folder, String *field,
        String *query_text, uint32_t expected_num_hits) {
    TermQuery *query = TermQuery_new(field, (Obj*)query_text);
    IndexSearcher *searcher = IxSearcher_new((Obj*)folder);
    Hits *hits = IxSearcher_Hits(searcher, (Obj*)query, 0, 10, NULL);

    TEST_TRUE(runner, Hits_Total_Hits(hits) == expected_num_hits,
              "%s correct num hits", Str_Get_Ptr8(field));

    // Don't check the contents of the hit if there aren't any.
    if (expected_num_hits) {
        HitDoc *hit = Hits_Next(hits);
        String *value = (String*)HitDoc_Extract(hit, field);
        TEST_TRUE(runner, Str_Equals(united_states_str, (Obj*)value),
                  "%s correct doc returned", Str_Get_Ptr8(field));
        DECREF(value);
        DECREF(hit);
    }

    DECREF(hits);
    DECREF(searcher);
    DECREF(query);
}

static void
test_spec_field(TestBatchRunner *runner) {
    RAMFolder *folder  = RAMFolder_new(NULL);
    Schema    *schema  = S_create_schema();
    Indexer   *indexer = Indexer_new(schema, (Obj*)folder, NULL, 0);

    S_add_doc(indexer, analyzed_str);
    S_add_doc(indexer, easy_analyzed_str);
    S_add_doc(indexer, string_str);
    S_add_doc(indexer, unindexed_but_analyzed_str);
    S_add_doc(indexer, unindexed_unanalyzed_str);

    Indexer_Commit(indexer);

    S_check(runner, folder, analyzed_str,               states_str,        1);
    S_check(runner, folder, easy_analyzed_str,          state_str,         1);
    S_check(runner, folder, string_str,                 united_states_str, 1);
    S_check(runner, folder, unindexed_but_analyzed_str, state_str,         0);
    S_check(runner, folder, unindexed_but_analyzed_str, united_states_str, 0);
    S_check(runner, folder, unindexed_unanalyzed_str,   state_str,         0);
    S_check(runner, folder, unindexed_unanalyzed_str,   united_states_str, 0);

    DECREF(indexer);
    DECREF(schema);
    DECREF(folder);
}

static void
S_add_many_fields_doc(Indexer *indexer, String *content, int num_fields) {
    Doc *doc = Doc_new(NULL, 0);
    for (int32_t i = 1; i <= num_fields; ++i) {
        String *field = Str_newf("field%i32", i);
        Doc_Store(doc, field, (Obj*)content);
        DECREF(field);
    }
    Indexer_Add_Doc(indexer, doc, 1.0f);
    DECREF(doc);
}

static void
test_many_fields(TestBatchRunner *runner) {
    Schema            *schema    = Schema_new();
    StandardTokenizer *tokenizer = StandardTokenizer_new();
    FullTextType      *type      = FullTextType_new((Analyzer*)tokenizer);
    String            *query     = Str_newf("x");

    for (int32_t num_fields = 1; num_fields <= 10; ++num_fields) {
        // Build an index with num_fields fields, and the same content in each.
        String *field = Str_newf("field%i32", num_fields);
        Schema_Spec_Field(schema, field, (FieldType*)type);

        RAMFolder *folder  = RAMFolder_new(NULL);
        Indexer   *indexer = Indexer_new(schema, (Obj*)folder, NULL, 0);

        String *content;

        for (int c = 'a'; c <= 'z'; ++c) {
            content = Str_new_from_char(c);
            S_add_many_fields_doc(indexer, content, num_fields);
            DECREF(content);
        }

        content = Str_newf("x x y");
        S_add_many_fields_doc(indexer, content, num_fields);
        DECREF(content);

        Indexer_Commit(indexer);

        // See if our search results match as expected.
        IndexSearcher *searcher = IxSearcher_new((Obj*)folder);
        Hits *hits = IxSearcher_Hits(searcher, (Obj*)query, 0, 100, NULL);
        TEST_TRUE(runner, Hits_Total_Hits(hits) == 2,
                  "correct number of hits for %d fields", num_fields);
        HitDoc *top_hit = Hits_Next(hits);

        DECREF(top_hit);
        DECREF(hits);
        DECREF(searcher);
        DECREF(indexer);
        DECREF(folder);
        DECREF(field);
    }

    DECREF(query);
    DECREF(type);
    DECREF(tokenizer);
    DECREF(schema);
}

void
TestFieldMisc_Run_IMP(TestFieldMisc *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 20);
    S_init_strings();
    test_spec_field(runner);
    test_many_fields(runner);
    S_destroy_strings();
}


