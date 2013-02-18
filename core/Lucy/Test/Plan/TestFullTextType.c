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

#define C_LUCY_TESTFULLTEXTTYPE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Plan/TestFullTextType.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Plan/FullTextType.h"
#include "Lucy/Analysis/Normalizer.h"
#include "Lucy/Analysis/StandardTokenizer.h"

TestFullTextType*
TestFullTextType_new() {
    TestFullTextType *self = (TestFullTextType*)VTable_Make_Obj(TESTFULLTEXTTYPE);
    return TestFullTextType_init(self);
}

TestFullTextType*
TestFullTextType_init(TestFullTextType *self) {
    return (TestFullTextType*)TestBatch_init((TestBatch*)self, 10);
}

static void
test_Dump_Load_and_Equals(TestBatch *batch) {
    StandardTokenizer *tokenizer     = StandardTokenizer_new();
    Normalizer        *normalizer    = Normalizer_new(NULL, true, false);
    FullTextType      *type          = FullTextType_new((Analyzer*)tokenizer);
    FullTextType      *other         = FullTextType_new((Analyzer*)normalizer);
    FullTextType      *boost_differs = FullTextType_new((Analyzer*)tokenizer);
    FullTextType      *not_indexed   = FullTextType_new((Analyzer*)tokenizer);
    FullTextType      *not_stored    = FullTextType_new((Analyzer*)tokenizer);
    FullTextType      *highlightable = FullTextType_new((Analyzer*)tokenizer);
    Obj               *dump          = (Obj*)FullTextType_Dump(type);
    Obj               *clone         = Obj_Load(dump, dump);
    Obj               *another_dump  = (Obj*)FullTextType_Dump_For_Schema(type);

    FullTextType_Set_Boost(boost_differs, 1.5);
    FullTextType_Set_Indexed(not_indexed, false);
    FullTextType_Set_Stored(not_stored, false);
    FullTextType_Set_Highlightable(highlightable, true);

    // (This step is normally performed by Schema_Load() internally.)
    Hash_Store_Str((Hash*)another_dump, "analyzer", 8, INCREF(tokenizer));
    FullTextType *another_clone = FullTextType_load(type, another_dump);

    TEST_FALSE(batch, FullTextType_Equals(type, (Obj*)boost_differs),
               "Equals() false with different boost");
    TEST_FALSE(batch, FullTextType_Equals(type, (Obj*)other),
               "Equals() false with different Analyzer");
    TEST_FALSE(batch, FullTextType_Equals(type, (Obj*)not_indexed),
               "Equals() false with indexed => false");
    TEST_FALSE(batch, FullTextType_Equals(type, (Obj*)not_stored),
               "Equals() false with stored => false");
    TEST_FALSE(batch, FullTextType_Equals(type, (Obj*)highlightable),
               "Equals() false with highlightable => true");
    TEST_TRUE(batch, FullTextType_Equals(type, (Obj*)clone),
              "Dump => Load round trip");
    TEST_TRUE(batch, FullTextType_Equals(type, (Obj*)another_clone),
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
test_Compare_Values(TestBatch *batch) {
    StandardTokenizer *tokenizer = StandardTokenizer_new();
    FullTextType      *type      = FullTextType_new((Analyzer*)tokenizer);
    ZombieCharBuf     *a         = ZCB_WRAP_STR("a", 1);
    ZombieCharBuf     *b         = ZCB_WRAP_STR("b", 1);

    TEST_TRUE(batch,
              FullTextType_Compare_Values(type, (Obj*)a, (Obj*)b) < 0,
              "a less than b");
    TEST_TRUE(batch,
              FullTextType_Compare_Values(type, (Obj*)b, (Obj*)a) > 0,
              "b greater than a");
    TEST_TRUE(batch,
              FullTextType_Compare_Values(type, (Obj*)b, (Obj*)b) == 0,
              "b equals b");

    DECREF(type);
    DECREF(tokenizer);
}

void
TestFullTextType_run_tests(TestFullTextType *self) {
    TestBatch *batch = (TestBatch*)self;
    test_Dump_Load_and_Equals(batch);
    test_Compare_Values(batch);
}


