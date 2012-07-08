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


#include "Lucy/Util/ToolSet.h"
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

static CharBuf *analyzed_cb;
static CharBuf *easy_analyzed_cb;
static CharBuf *state_cb;
static CharBuf *states_cb;
static CharBuf *string_cb;
static CharBuf *unindexed_but_analyzed_cb;
static CharBuf *unindexed_unanalyzed_cb;
static CharBuf *united_states_cb;

static void
S_init_strings() {
    analyzed_cb               = CB_newf("analyzed");
    easy_analyzed_cb          = CB_newf("easy_analyzed");
    state_cb                  = CB_newf("state");
    states_cb                 = CB_newf("States");
    string_cb                 = CB_newf("string");
    unindexed_but_analyzed_cb = CB_newf("unindexed_but_analyzed");
    unindexed_unanalyzed_cb   = CB_newf("unindexed_unanalyzed");
    united_states_cb          = CB_newf("United States");
}

static void
S_destroy_strings() {
    DECREF(analyzed_cb);
    DECREF(easy_analyzed_cb);
    DECREF(state_cb);
    DECREF(states_cb);
    DECREF(string_cb);
    DECREF(unindexed_but_analyzed_cb);
    DECREF(unindexed_unanalyzed_cb);
    DECREF(united_states_cb);
}

static Schema*
S_create_schema() {
    Schema *schema = Schema_new();

    StandardTokenizer *tokenizer     = StandardTokenizer_new();
    CharBuf           *language      = CB_newf("en");
    EasyAnalyzer      *easy_analyzer = EasyAnalyzer_new(language);

    FullTextType *plain         = FullTextType_new((Analyzer*)tokenizer);
    FullTextType *easy_analyzed = FullTextType_new((Analyzer*)easy_analyzer);

    StringType *string_spec = StringType_new();

    FullTextType *unindexed_but_analyzed
        = FullTextType_new((Analyzer*)tokenizer);
    FullTextType_Set_Indexed(unindexed_but_analyzed, false);

    StringType *unindexed_unanalyzed = StringType_new();
    StringType_Set_Indexed(unindexed_unanalyzed, false);

    Schema_Spec_Field(schema, analyzed_cb, (FieldType*)plain);
    Schema_Spec_Field(schema, easy_analyzed_cb, (FieldType*)easy_analyzed);
    Schema_Spec_Field(schema, string_cb, (FieldType*)string_spec);
    Schema_Spec_Field(schema, unindexed_but_analyzed_cb,
                      (FieldType*)unindexed_but_analyzed);
    Schema_Spec_Field(schema, unindexed_unanalyzed_cb,
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
S_add_doc(Indexer *indexer, CharBuf *field_name) {
    Doc *doc = Doc_new(NULL, 0);
    Doc_Store(doc, field_name, (Obj*)united_states_cb);
    Indexer_Add_Doc(indexer, doc, 1.0f);
    DECREF(doc);
}

static void
S_check(TestBatch *batch, RAMFolder *folder, CharBuf *field,
        CharBuf *query_text, int expected_num_hits) {
    TermQuery *query = TermQuery_new(field, (Obj*)query_text);
    IndexSearcher *searcher = IxSearcher_new((Obj*)folder);
    Hits *hits = IxSearcher_Hits(searcher, (Obj*)query, 0, 10, NULL);

    TEST_TRUE(batch, Hits_Total_Hits(hits) == expected_num_hits,
              "%s correct num hits", CB_Get_Ptr8(field));

    // Don't check the contents of the hit if there aren't any.
    if (expected_num_hits) {
        HitDoc *hit = Hits_Next(hits);
        ViewCharBuf *value = (ViewCharBuf*)ZCB_BLANK();
        HitDoc_Extract(hit, field, value);
        TEST_TRUE(batch, CB_Equals(united_states_cb, (Obj*)value),
                  "%s correct doc returned", CB_Get_Ptr8(field));
        DECREF(hit);
    }

    DECREF(hits);
    DECREF(searcher);
    DECREF(query);
}

static void
test_spec_field(TestBatch *batch) {
    RAMFolder *folder  = RAMFolder_new(NULL);
    Schema    *schema  = S_create_schema();
    Indexer   *indexer = Indexer_new(schema, (Obj*)folder, NULL, 0);

    S_add_doc(indexer, analyzed_cb);
    S_add_doc(indexer, easy_analyzed_cb);
    S_add_doc(indexer, string_cb);
    S_add_doc(indexer, unindexed_but_analyzed_cb);
    S_add_doc(indexer, unindexed_unanalyzed_cb);

    Indexer_Commit(indexer);

    S_check(batch, folder, analyzed_cb,               states_cb,        1);
    S_check(batch, folder, easy_analyzed_cb,          state_cb,         1);
    S_check(batch, folder, string_cb,                 united_states_cb, 1);
    S_check(batch, folder, unindexed_but_analyzed_cb, state_cb,         0);
    S_check(batch, folder, unindexed_but_analyzed_cb, united_states_cb, 0);
    S_check(batch, folder, unindexed_unanalyzed_cb,   state_cb,         0);
    S_check(batch, folder, unindexed_unanalyzed_cb,   united_states_cb, 0);

    DECREF(indexer);
    DECREF(schema);
    DECREF(folder);
}

void
TestFieldMisc_run_tests() {
    TestBatch *batch = TestBatch_new(10);
    TestBatch_Plan(batch);
    S_init_strings();
    test_spec_field(batch);
    S_destroy_strings();
    DECREF(batch);
}


