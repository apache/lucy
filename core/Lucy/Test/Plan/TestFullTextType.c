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

#define C_TESTLUCY_TESTFULLTEXTTYPE
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Plan/TestFullTextType.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Plan/FullTextType.h"
#include "Lucy/Analysis/Normalizer.h"
#include "Lucy/Analysis/StandardTokenizer.h"
#include "Lucy/Util/Freezer.h"

TestFullTextType*
TestFullTextType_new() {
    return (TestFullTextType*)Class_Make_Obj(TESTFULLTEXTTYPE);
}

static void
test_Dump_Load_and_Equals(TestBatchRunner *runner) {
    StandardTokenizer *tokenizer     = StandardTokenizer_new();
    Normalizer        *normalizer    = Normalizer_new(NULL, true, false);
    FullTextType      *type          = FullTextType_new((Analyzer*)tokenizer);
    FullTextType      *other         = FullTextType_new((Analyzer*)normalizer);
    FullTextType      *boost_differs = FullTextType_new((Analyzer*)tokenizer);
    FullTextType      *not_indexed   = FullTextType_new((Analyzer*)tokenizer);
    FullTextType      *not_stored    = FullTextType_new((Analyzer*)tokenizer);
    FullTextType      *highlightable = FullTextType_new((Analyzer*)tokenizer);
    Obj               *dump          = (Obj*)FullTextType_Dump(type);
    Obj               *clone         = Freezer_load(dump);
    Obj               *another_dump  = (Obj*)FullTextType_Dump_For_Schema(type);

    FullTextType_Set_Boost(boost_differs, 1.5);
    FullTextType_Set_Indexed(not_indexed, false);
    FullTextType_Set_Stored(not_stored, false);
    FullTextType_Set_Highlightable(highlightable, true);

    // (This step is normally performed by Schema_Load() internally.)
    Hash_Store_Utf8((Hash*)another_dump, "analyzer", 8, INCREF(tokenizer));
    FullTextType *another_clone = FullTextType_Load(type, another_dump);

    TEST_FALSE(runner, FullTextType_Equals(type, (Obj*)boost_differs),
               "Equals() false with different boost");
    TEST_FALSE(runner, FullTextType_Equals(type, (Obj*)other),
               "Equals() false with different Analyzer");
    TEST_FALSE(runner, FullTextType_Equals(type, (Obj*)not_indexed),
               "Equals() false with indexed => false");
    TEST_FALSE(runner, FullTextType_Equals(type, (Obj*)not_stored),
               "Equals() false with stored => false");
    TEST_FALSE(runner, FullTextType_Equals(type, (Obj*)highlightable),
               "Equals() false with highlightable => true");
    TEST_TRUE(runner, FullTextType_Equals(type, (Obj*)clone),
              "Dump => Load round trip");
    TEST_TRUE(runner, FullTextType_Equals(type, (Obj*)another_clone),
              "Dump_For_Schema => Load round trip");

    DECREF(another_clone);
    DECREF(dump);
    DECREF(clone);
    DECREF(another_dump);
    DECREF(highlightable);
    DECREF(not_stored);
    DECREF(not_indexed);
    DECREF(boost_differs);
    DECREF(other);
    DECREF(type);
    DECREF(normalizer);
    DECREF(tokenizer);
}

static void
test_Compare_Values(TestBatchRunner *runner) {
    StandardTokenizer *tokenizer = StandardTokenizer_new();
    FullTextType      *type      = FullTextType_new((Analyzer*)tokenizer);
    StackString       *a         = SSTR_WRAP_UTF8("a", 1);
    StackString       *b         = SSTR_WRAP_UTF8("b", 1);

    TEST_TRUE(runner,
              FullTextType_Compare_Values(type, (Obj*)a, (Obj*)b) < 0,
              "a less than b");
    TEST_TRUE(runner,
              FullTextType_Compare_Values(type, (Obj*)b, (Obj*)a) > 0,
              "b greater than a");
    TEST_TRUE(runner,
              FullTextType_Compare_Values(type, (Obj*)b, (Obj*)b) == 0,
              "b equals b");

    DECREF(type);
    DECREF(tokenizer);
}

void
TestFullTextType_Run_IMP(TestFullTextType *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 10);
    test_Dump_Load_and_Equals(runner);
    test_Compare_Values(runner);
}


