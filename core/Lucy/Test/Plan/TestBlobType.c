#define C_LUCY_TESTBLOBTYPE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Plan/TestBlobType.h"
#include "Lucy/Test/Index/Similarity/DummySimilarity.h"
#include "Lucy/Plan/BlobType.h"

static void
test_simple_accessors(TestBatch *batch)
{
    BlobType *type = BlobType_new(NULL);
    ASSERT_TRUE(batch, BlobType_Scalar_Type_ID(type) == Obj_BLOB, 
        "Scalar_Type_ID()");
    ASSERT_TRUE(batch, BlobType_Binary(type), "Binary() returns true");
    DECREF(type);
}

static void
test_Dump_Load_and_Equals(TestBatch *batch)
{
    Similarity *sim  = (Similarity*)DummySim_new(1);
    BlobType   *type = BlobType_new(sim);

    // Set to non-defaults to stress Dump/Load.
    BlobType_Set_Sortable(type, true);

    {
        Obj *dump = (Obj*)BlobType_Dump(type);
        Obj *clone = Obj_Load(dump, dump);
        ASSERT_TRUE(batch, BlobType_Equals(type, (Obj*)clone), 
            "Dump => Load round trip");
        DECREF(dump);
        DECREF(clone);
    }

    {
        Obj      *dump  = (Obj*)BlobType_Dump_For_Schema(type);
        // (This step is normally performed by Schema_Load() internally.) 
        Hash_Store_Str((Hash*)dump, "similarity", 10, INCREF(sim));
        BlobType *clone = BlobType_load(NULL, dump);
        ASSERT_TRUE(batch, BlobType_Equals(type, (Obj*)clone), 
            "Dump_For_Schema => Load round trip");
        DECREF(dump);
        DECREF(clone);
    }

    DECREF(type);
}

void
TestBlobType_run_tests()
{
    TestBatch *batch = TestBatch_new(4);
    TestBatch_Plan(batch);
    test_simple_accessors(batch);
    test_Dump_Load_and_Equals(batch);
    DECREF(batch);
}

/* Copyright 2010 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

