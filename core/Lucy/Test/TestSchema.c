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

#define C_LUCY_TESTSCHEMA
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Plan/TestArchitecture.h"
#include "Lucy/Test/TestSchema.h"
#include "Lucy/Analysis/CaseFolder.h"
#include "Lucy/Analysis/RegexTokenizer.h"
#include "Lucy/Plan/FullTextType.h"
#include "Lucy/Plan/Architecture.h"

TestSchema*
TestSchema_new() {
    TestSchema *self = (TestSchema*)VTable_Make_Obj(TESTSCHEMA);
    return TestSchema_init(self);
}

TestSchema*
TestSchema_init(TestSchema *self) {
    RegexTokenizer *tokenizer = RegexTokenizer_new(NULL);
    FullTextType *type = FullTextType_new((Analyzer*)tokenizer);

    Schema_init((Schema*)self);
    FullTextType_Set_Highlightable(type, true);
    CharBuf *content = (CharBuf*)ZCB_WRAP_STR("content", 7);
    TestSchema_Spec_Field(self, content, (FieldType*)type);
    DECREF(type);
    DECREF(tokenizer);

    return self;
}

Architecture*
TestSchema_architecture(TestSchema *self) {
    UNUSED_VAR(self);
    return (Architecture*)TestArch_new();
}

static void
test_Equals(TestBatch *batch) {
    TestSchema *schema = TestSchema_new();
    TestSchema *arch_differs = TestSchema_new();
    TestSchema *spec_differs = TestSchema_new();
    CharBuf    *content      = (CharBuf*)ZCB_WRAP_STR("content", 7);
    FullTextType *type = (FullTextType*)TestSchema_Fetch_Type(spec_differs,
                                                              content);
    CaseFolder *case_folder = CaseFolder_new();

    TEST_TRUE(batch, TestSchema_Equals(schema, (Obj*)schema), "Equals");

    FullTextType_Set_Boost(type, 2.0f);
    TEST_FALSE(batch, TestSchema_Equals(schema, (Obj*)spec_differs),
               "Equals spoiled by differing FieldType");

    DECREF(arch_differs->arch);
    arch_differs->arch = Arch_new();
    TEST_FALSE(batch, TestSchema_Equals(schema, (Obj*)arch_differs),
               "Equals spoiled by differing Architecture");

    DECREF(schema);
    DECREF(arch_differs);
    DECREF(spec_differs);
    DECREF(case_folder);
}

static void
test_Dump_and_Load(TestBatch *batch) {
    TestSchema *schema = TestSchema_new();
    Obj        *dump   = (Obj*)TestSchema_Dump(schema);
    TestSchema *loaded = (TestSchema*)Obj_Load(dump, dump);

    TEST_FALSE(batch, TestSchema_Equals(schema, (Obj*)loaded),
               "Dump => Load round trip");

    DECREF(schema);
    DECREF(dump);
    DECREF(loaded);
}

void
TestSchema_run_tests() {
    TestBatch *batch = TestBatch_new(4);
    TestBatch_Plan(batch);
    test_Equals(batch);
    test_Dump_and_Load(batch);
    DECREF(batch);
}


