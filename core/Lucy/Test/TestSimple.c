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

#define C_TESTLUCY_TESTSIMPLE
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test/TestSimple.h"
#include "Lucy/Simple.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Store/RAMFolder.h"

TestSimple*
TestSimple_new() {
    return (TestSimple*)Class_Make_Obj(TESTSIMPLE);
}

static void
test_simple(TestBatchRunner *runner) {
    RAMFolder *folder   = RAMFolder_new(NULL);
    String    *language = SSTR_WRAP_UTF8("en", 2);
    Simple    *lucy     = Simple_new((Obj*)folder, language);

    String *food_field = SSTR_WRAP_UTF8("food", 4);

    {
        Doc *doc = Doc_new(NULL, 0);
        String *value = SSTR_WRAP_UTF8("creamed corn", 12);
        Doc_Store(doc, food_field, (Obj*)value);
        Simple_Add_Doc(lucy, doc);
        DECREF(doc);

        String *query = SSTR_WRAP_UTF8("creamed", 7);
        uint32_t num_results = Simple_Search(lucy, query, 0, 10);
        TEST_INT_EQ(runner, num_results, 1, "Search works right after add");
    }

    {
        Doc *doc = Doc_new(NULL, 0);
        String *value = SSTR_WRAP_UTF8("creamed spinach", 15);
        Doc_Store(doc, food_field, (Obj*)value);
        Simple_Add_Doc(lucy, doc);
        DECREF(doc);

        String *query = SSTR_WRAP_UTF8("creamed", 7);
        uint32_t num_results = Simple_Search(lucy, query, 0, 10);
        TEST_INT_EQ(runner, num_results, 2, "Search returns total hits");
    }

    {
        Doc *doc = Doc_new(NULL, 0);
        String *value = SSTR_WRAP_UTF8("creamed broccoli", 16);
        Doc_Store(doc, food_field, (Obj*)value);
        Simple_Add_Doc(lucy, doc);
        DECREF(doc);

        DECREF(lucy);
        lucy = Simple_new((Obj*)folder, language);

        String *query = SSTR_WRAP_UTF8("cream", 5);
        uint32_t num_results = Simple_Search(lucy, query, 0, 10);
        TEST_INT_EQ(runner, num_results, 3, "commit upon destroy");

        HitDoc *hit;
        while ((hit = Simple_Next(lucy)) != NULL) {
            String *food = (String*)HitDoc_Extract(hit, food_field);
            TEST_TRUE(runner, Str_Starts_With_Utf8(food, "cream", 5), "Next");
            DECREF(food);
            DECREF(hit);
        }
    }

    {
        Doc *doc = Doc_new(NULL, 0);
        String *band_field = SSTR_WRAP_UTF8("band", 4);
        String *value = SSTR_WRAP_UTF8("Cream", 5);
        Doc_Store(doc, band_field, (Obj*)value);
        Simple_Add_Doc(lucy, doc);
        DECREF(doc);

        String *query = SSTR_WRAP_UTF8("cream", 5);
        uint32_t num_results = Simple_Search(lucy, query, 0, 10);
        TEST_INT_EQ(runner, num_results, 4,
                    "Search uses correct EasyAnalyzer");
    }

    DECREF(lucy);
    DECREF(folder);
}

void
TestSimple_Run_IMP(TestSimple *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 7);
    test_simple(runner);
}

