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

#include "Lucy/Test/Index/TestBackgroundMerger.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Index/BackgroundMerger.h"
#include "Lucy/Index/Indexer.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Search/Hits.h"
#include "Lucy/Search/IndexSearcher.h"
#include "Lucy/Store/FSFolder.h"
#include "Lucy/Store/RAMFolder.h"
#include "Lucy/Test/Index/NoMergeManager.h"
#include "Lucy/Test/TestSchema.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Util/Json.h"

TestBackgroundMerger*
TestBGMerger_new() {
    return (TestBackgroundMerger*)Class_Make_Obj(TESTBACKGROUNDMERGER);
}

static void
S_add_doc(Schema *schema, Folder *folder, int code_point,
          IndexManager *manager) {
    Indexer      *indexer = Indexer_new(schema, (Obj*)folder, manager, 0);
    Doc          *doc     = Doc_new(NULL, 0);
    String       *field   = SSTR_WRAP_C("content");
    String       *content = Str_new_from_char(code_point);

    Doc_Store(doc, field, (Obj*)content);
    Indexer_Add_Doc(indexer, doc, 1.0f);
    Indexer_Commit(indexer);

    DECREF(content);
    DECREF(doc);
    DECREF(indexer);
}

static int
S_count_segs(Folder *folder) {
    Vector *entries = Folder_List_R(folder, NULL);
    int count = 0;

    for (size_t i = 0, max = Vec_Get_Size(entries); i < max; i++) {
        String *entry = (String*)Vec_Fetch(entries, i);
        if (Str_Ends_With_Utf8(entry, "segmeta.json", 12)) { count++; }
    }

    DECREF(entries);
    return count;
}

static void
S_test_terms(TestBatchRunner *runner, Folder *folder, const char *msg) {
    IndexSearcher *searcher = IxSearcher_new((Obj*)folder);

    for (int i = 'a'; i <= 'e'; i++) {
        const char *status;
        uint32_t expected;

        if (i == 'b') {
            expected = 0;
            status   = "deleted";
        }
        else {
            expected = 1;
            status   = "present";
        }

        String *query = Str_new_from_char(i);
        Hits *hits = IxSearcher_Hits(searcher, (Obj*)query, 0, 10, NULL);
        TEST_UINT_EQ(runner, Hits_Total_Hits(hits), expected,
                     "term %c still %s%s", i, status, msg);
        DECREF(hits);
        DECREF(query);
    }

    DECREF(searcher);
}

static void
test_bg_merger(TestBatchRunner *runner) {
    Schema  *schema        = (Schema*)TestSchema_new(false);
#if 1
    Folder  *folder        = (Folder*)RAMFolder_new(NULL);
#else
    Folder  *folder        = (Folder*)FSFolder_new(SSTR_WRAP_C("_test_index"));
#endif
    IndexManager *no_merge = (IndexManager*)NoMergeManager_new();

    Folder_Initialize(folder);

    for (int i = 'a'; i <= 'c'; i++) {
        S_add_doc(schema, folder, i, no_merge);
    }

    BackgroundMerger *bg_merger = BGMerger_new((Obj*)folder, NULL);
    S_add_doc(schema, folder, 'd', NULL);
    TEST_INT_EQ(runner, S_count_segs(folder), 4,
                "BackgroundMerger prevents Indexer from merging claimed"
                " segments");

    {
        Indexer      *indexer = Indexer_new(NULL, (Obj*)folder, NULL, 0);
        Doc          *doc     = Doc_new(NULL, 0);
        String       *field   = SSTR_WRAP_C("content");
        String       *content = Str_new_from_char('e');

        Doc_Store(doc, field, (Obj*)content);
        Indexer_Add_Doc(indexer, doc, 1.0f);
        Indexer_Delete_By_Term(indexer, field, (Obj*)SSTR_WRAP_C("b"));
        Indexer_Commit(indexer);
        TEST_INT_EQ(runner, S_count_segs(folder), 4,
                    "Indexer may still merge unclaimed segments");

        DECREF(content);
        DECREF(doc);
        DECREF(indexer);
    }

    BGMerger_Commit(bg_merger);
    TEST_INT_EQ(runner, S_count_segs(folder), 3, "Background merge completes");
    String *del_file = SSTR_WRAP_C("seg_7/deletions-seg_4.bv");
    TEST_TRUE(runner, Folder_Exists(folder, del_file),
              "deletions carried forward");

    S_test_terms(runner, folder, "");

    String *merge_json = SSTR_WRAP_C("merge.json");
    String *cutoff_seg = NULL;

    {
        // Simulate failed background merge.
        DECREF(bg_merger);
        IndexManager *manager = IxManager_new(NULL, NULL);
        bg_merger = BGMerger_new((Obj*)folder, manager);
        BGMerger_Prepare_Commit(bg_merger);
        DECREF(bg_merger);

        TEST_TRUE(runner, Folder_Local_Exists(folder, merge_json),
                  "merge.json exists");
        Hash *merge_data = IxManager_Read_Merge_Data(manager);
        Obj  *cutoff     = Hash_Fetch_Utf8(merge_data, "cutoff", 6);

        cutoff_seg = Seg_num_to_name(Json_obj_to_i64(cutoff));
        TEST_TRUE(runner, Folder_Local_Exists(folder, cutoff_seg),
                  "cutoff segment exists");

        Indexer *indexer = Indexer_new(NULL, (Obj*)folder, no_merge, 0);
        Indexer_Commit(indexer);

        DECREF(indexer);
        DECREF(cutoff_seg);
        DECREF(merge_data);
        DECREF(manager);
    }

    TEST_FALSE(runner, Folder_Local_Exists(folder, merge_json),
               "merge.json deleted after failed bg merge");
    // Doesn't work because an indexing session always creates an empty
    // segment directory even if no documents were added and nothing
    // was merged.
#if 0
    TEST_FALSE(runner, Folder_Local_Exists(folder, cutoff_seg),
               "cutoff segment deleted after failed bg merge");
#endif

    {
        Indexer *indexer = Indexer_new(NULL, (Obj*)folder, NULL, 0);
        Indexer_Optimize(indexer);
        Indexer_Commit(indexer);
        DECREF(indexer);
    }

    TEST_INT_EQ(runner, S_count_segs(folder), 1,
                "Only a single segment remains after full optimize");
    S_test_terms(runner, folder, " after full optimize");

    DECREF(no_merge);
    DECREF(folder);
    DECREF(schema);
}

void
TestBGMerger_Run_IMP(TestBackgroundMerger *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 18);
    test_bg_merger(runner);
}


