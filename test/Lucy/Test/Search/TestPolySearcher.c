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

#define C_TESTLUCY_TESTPOLYSEARCHER
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test/Search/TestPolySearcher.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Index/DocVector.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/Collector.h"
#include "Lucy/Search/Hits.h"
#include "Lucy/Search/IndexSearcher.h"
#include "Lucy/Search/PolySearcher.h"
#include "Lucy/Search/Query.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Test.h"
#include "Lucy/Test/TestSchema.h"
#include "Lucy/Test/TestUtils.h"

TestPolySearcher*
TestPolySearcher_new() {
    return (TestPolySearcher*)Class_Make_Obj(TESTPOLYSEARCHER);
}

static PolySearcher*
S_create_poly_searcher() {
    Schema *schema = (Schema*)TestSchema_new(false);

    Folder *folder_a = TestUtils_create_index_c("x a", "x b", "x c", NULL);
    Folder *folder_b = TestUtils_create_index_c("y b", "y c", "y d", NULL);

    Vector *searchers = Vec_new(2);
    Vec_Push(searchers, (Obj*)IxSearcher_new((Obj*)folder_a));
    Vec_Push(searchers, (Obj*)IxSearcher_new((Obj*)folder_b));

    PolySearcher *poly_searcher = PolySearcher_new(schema, searchers);

    DECREF(searchers);
    DECREF(folder_b);
    DECREF(folder_a);
    DECREF(schema);
    return poly_searcher;
}

static void
test_poly_searcher(TestBatchRunner *runner) {
    PolySearcher *poly_searcher = S_create_poly_searcher();

    Vector *searchers = PolySearcher_Get_Searchers(poly_searcher);
    TEST_UINT_EQ(runner, Vec_Get_Size(searchers), 2, "Get_Searchers");

    TEST_UINT_EQ(runner,
                 PolySearcher_Doc_Freq(poly_searcher, SSTR_WRAP_C("content"),
                                       (Obj*)SSTR_WRAP_C("b")),
                 2, "Doc_Freq");
    TEST_INT_EQ(runner, PolySearcher_Doc_Max(poly_searcher), 6, "Doc_Max");

    HitDoc *doc = PolySearcher_Fetch_Doc(poly_searcher, 1);
    Obj *content = HitDoc_Extract(doc, SSTR_WRAP_C("content"));
    TEST_TRUE(runner, Obj_Equals(content, (Obj*)SSTR_WRAP_C("x a")),
              "Fetch_Doc");
    DECREF(content);
    DECREF(doc);

    DocVector *doc_vec = PolySearcher_Fetch_Doc_Vec(poly_searcher, 1);
    TEST_TRUE(runner, DocVec_is_a(doc_vec, DOCVECTOR), "Fetch_Doc_Vec");
    DECREF(doc_vec);

    {
        Hits *hits = PolySearcher_Hits(poly_searcher, (Obj*)SSTR_WRAP_C("a"),
                                       0, 10, NULL);
        TEST_UINT_EQ(runner, Hits_Total_Hits(hits), 1,
                     "Find hit in first searcher");
        DECREF(hits);
    }

    {
        Hits *hits = PolySearcher_Hits(poly_searcher, (Obj*)SSTR_WRAP_C("d"),
                                       0, 10, NULL);
        TEST_UINT_EQ(runner, Hits_Total_Hits(hits), 1,
                     "Find hit in second searcher");
        DECREF(hits);
    }

    {
        Hits *hits = PolySearcher_Hits(poly_searcher, (Obj*)SSTR_WRAP_C("b"),
                                       0, 10, NULL);
        TEST_UINT_EQ(runner, Hits_Total_Hits(hits), 2,
                     "Find hits in both searchers");
        DECREF(hits);
    }

    BitVector *bit_vec = BitVec_new(PolySearcher_Doc_Max(poly_searcher));
    Collector *collector = (Collector*)BitColl_new(bit_vec);
    Query *query = PolySearcher_Glean_Query(poly_searcher,
                                            (Obj*)SSTR_WRAP_C("b"));
    PolySearcher_Collect(poly_searcher, query, collector);
    TEST_INT_EQ(runner, BitVec_Next_Hit(bit_vec, 0),  2, "Collect doc 1");
    TEST_INT_EQ(runner, BitVec_Next_Hit(bit_vec, 3),  4, "Collect doc 2");
    TEST_INT_EQ(runner, BitVec_Next_Hit(bit_vec, 5), -1, "Collect end");
    DECREF(query);
    DECREF(collector);
    DECREF(bit_vec);

    DECREF(poly_searcher);
}

void
TestPolySearcher_Run_IMP(TestPolySearcher *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 11);
    test_poly_searcher(runner);
}
