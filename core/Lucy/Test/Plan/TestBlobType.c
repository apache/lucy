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

#define C_LUCY_TESTBLOBTYPE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Plan/TestBlobType.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Plan/BlobType.h"
#include "Lucy/Analysis/RegexTokenizer.h"

static void
test_Dump_Load_and_Equals(TestBatch *batch) {
    BlobType *type            = BlobType_new(true);
    Obj      *dump            = (Obj*)BlobType_Dump(type);
    Obj      *clone           = Obj_Load(dump, dump);
    Obj      *another_dump    = (Obj*)BlobType_Dump_For_Schema(type);
    BlobType *another_clone   = BlobType_load(NULL, another_dump);

    TEST_TRUE(batch, BlobType_Equals(type, (Obj*)clone),
              "Dump => Load round trip");
    TEST_TRUE(batch, BlobType_Equals(type, (Obj*)another_clone),
              "Dump_For_Schema => Load round trip");

    DECREF(type);
    DECREF(dump);
    DECREF(clone);
    DECREF(another_dump);
    DECREF(another_clone);
}

void
TestBlobType_run_tests() {
    TestBatch *batch = TestBatch_new(2);
    TestBatch_Plan(batch);
    test_Dump_Load_and_Equals(batch);
    DECREF(batch);
}


