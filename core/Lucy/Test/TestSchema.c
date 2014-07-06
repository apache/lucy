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

#define C_TESTLUCY_TESTSCHEMA
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Plan/TestArchitecture.h"
#include "Lucy/Test/TestSchema.h"
#include "Lucy/Analysis/StandardTokenizer.h"
#include "Lucy/Plan/FullTextType.h"
#include "Lucy/Plan/Architecture.h"
#include "Lucy/Util/Freezer.h"

TestSchema*
TestSchema_new(bool use_alt_arch) {
    TestSchema *self = (TestSchema*)Class_Make_Obj(TESTSCHEMA);
    return TestSchema_init(self, use_alt_arch);
}

TestSchema*
TestSchema_init(TestSchema *self, bool use_alt_arch) {
    StandardTokenizer *tokenizer = StandardTokenizer_new();
    FullTextType *type = FullTextType_new((Analyzer*)tokenizer);

    TestSchema_IVARS(self)->use_alt_arch = use_alt_arch;

    Schema_init((Schema*)self);
    FullTextType_Set_Highlightable(type, true);
    String *content = (String*)SSTR_WRAP_UTF8("content", 7);
    TestSchema_Spec_Field(self, content, (FieldType*)type);
    DECREF(type);
    DECREF(tokenizer);

    return self;
}

Architecture*
TestSchema_Architecture_IMP(TestSchema *self) {
    if (TestSchema_IVARS(self)->use_alt_arch) {
        return Arch_new();
    }
    else {
        return (Architecture*)TestArch_new();
    }
}

TestBatchSchema*
TestBatchSchema_new() {
    return (TestBatchSchema*)Class_Make_Obj(TESTBATCHSCHEMA);
}

static void
test_Equals(TestBatchRunner *runner) {
    TestSchema *schema = TestSchema_new(false);
    TestSchema *arch_differs = TestSchema_new(true);
    TestSchema *spec_differs = TestSchema_new(false);
    String     *content      = (String*)SSTR_WRAP_UTF8("content", 7);
    FullTextType *type = (FullTextType*)TestSchema_Fetch_Type(spec_differs,
                                                              content);

    TEST_TRUE(runner, TestSchema_Equals(schema, (Obj*)schema), "Equals");

    FullTextType_Set_Boost(type, 2.0f);
    TEST_FALSE(runner, TestSchema_Equals(schema, (Obj*)spec_differs),
               "Equals spoiled by differing FieldType");

    TEST_FALSE(runner, TestSchema_Equals(schema, (Obj*)arch_differs),
               "Equals spoiled by differing Architecture");

    DECREF(schema);
    DECREF(arch_differs);
    DECREF(spec_differs);
}

static void
test_Dump_and_Load(TestBatchRunner *runner) {
    TestSchema *schema = TestSchema_new(false);
    Obj        *dump   = (Obj*)TestSchema_Dump(schema);
    TestSchema *loaded = (TestSchema*)Freezer_load(dump);

    TEST_FALSE(runner, TestSchema_Equals(schema, (Obj*)loaded),
               "Dump => Load round trip");

    DECREF(schema);
    DECREF(dump);
    DECREF(loaded);
}

void
TestBatchSchema_Run_IMP(TestBatchSchema *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 4);
    test_Equals(runner);
    test_Dump_and_Load(runner);
}


