#define C_LUCY_TESTNUMERICTYPE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Plan/TestNumericType.h"
#include "Lucy/Test/Index/Similarity/DummySimilarity.h"
#include "Lucy/Plan/NumericType.h"

static void
test_Dump_Load_and_Equals(TestBatch *batch)
{
    Similarity  *sim = (Similarity*)DummySim_new(1);
    Int32Type   *i32 = Int32Type_new(sim);
    Int64Type   *i64 = Int64Type_new(sim);
    Float32Type *f32 = Float32Type_new(sim);
    Float64Type *f64 = Float64Type_new(sim);

    ASSERT_FALSE(batch, Int32Type_Equals(i32, (Obj*)i64), 
        "Int32Type_Equals() false for different type");
    ASSERT_FALSE(batch, Int32Type_Equals(i32, NULL), 
        "Int32Type_Equals() false for NULL");

    ASSERT_FALSE(batch, Int64Type_Equals(i64, (Obj*)i32), 
        "Int64Type_Equals() false for different type");
    ASSERT_FALSE(batch, Int64Type_Equals(i64, NULL), 
        "Int64Type_Equals() false for NULL");

    ASSERT_FALSE(batch, Float32Type_Equals(f32, (Obj*)f64), 
        "Float32Type_Equals() false for different type");
    ASSERT_FALSE(batch, Float32Type_Equals(f32, NULL), 
        "Float32Type_Equals() false for NULL");

    ASSERT_FALSE(batch, Float64Type_Equals(f64, (Obj*)f32), 
        "Float64Type_Equals() false for different type");
    ASSERT_FALSE(batch, Float64Type_Equals(f64, NULL), 
        "Float64Type_Equals() false for NULL");

    {
        Obj *dump = (Obj*)Int32Type_Dump(i32);
        Obj *other = Obj_Load(dump, dump);
        ASSERT_TRUE(batch, Int32Type_Equals(i32, other), 
            "Dump => Load round trip for Int32Type");
        DECREF(dump);
        DECREF(other);
    }

    {
        Obj *dump = (Obj*)Int64Type_Dump(i64);
        Obj *other = Obj_Load(dump, dump);
        ASSERT_TRUE(batch, Int64Type_Equals(i64, other), 
            "Dump => Load round trip for Int64Type");
        DECREF(dump);
        DECREF(other);
    }

    {
        Obj *dump = (Obj*)Float32Type_Dump(f32);
        Obj *other = Obj_Load(dump, dump);
        ASSERT_TRUE(batch, Float32Type_Equals(f32, other), 
            "Dump => Load round trip for Float32Type");
        DECREF(dump);
        DECREF(other);
    }

    {
        Obj *dump = (Obj*)Float64Type_Dump(f64);
        Obj *other = Obj_Load(dump, dump);
        ASSERT_TRUE(batch, Float64Type_Equals(f64, other), 
            "Dump => Load round trip for Float64Type");
        DECREF(dump);
        DECREF(other);
    }

    {
        Obj *dump = (Obj*)Int32Type_Dump_For_Schema(i32);
        // (This step is normally performed by Schema_Load() internally.) 
        Hash_Store_Str((Hash*)dump, "similarity", 10, INCREF(sim));
        Int32Type *other = (Int32Type*)NumType_load(NULL, dump);
        ASSERT_TRUE(batch, Int32Type_Equals(i32, (Obj*)other), 
            "Dump_For_Schema => Load round trip for Int32Type");
        DECREF(dump);
        DECREF(other);
    }

    {
        Obj *dump = (Obj*)Int64Type_Dump_For_Schema(i64);
        Hash_Store_Str((Hash*)dump, "similarity", 10, INCREF(sim));
        Int64Type *other = (Int64Type*)NumType_load(NULL, dump);
        ASSERT_TRUE(batch, Int64Type_Equals(i64, (Obj*)other), 
            "Dump_For_Schema => Load round trip for Int64Type");
        DECREF(dump);
        DECREF(other);
    }

    {
        Obj *dump = (Obj*)Float32Type_Dump_For_Schema(f32);
        Hash_Store_Str((Hash*)dump, "similarity", 10, INCREF(sim));
        Float32Type *other = (Float32Type*)NumType_load(NULL, dump);
        ASSERT_TRUE(batch, Float32Type_Equals(f32, (Obj*)other), 
            "Dump_For_Schema => Load round trip for Float32Type");
        DECREF(dump);
        DECREF(other);
    }

    {
        Obj *dump = (Obj*)Float64Type_Dump_For_Schema(f64);
        Hash_Store_Str((Hash*)dump, "similarity", 10, INCREF(sim));
        Float64Type *other = (Float64Type*)NumType_load(NULL, dump);
        ASSERT_TRUE(batch, Float64Type_Equals(f64, (Obj*)other), 
            "Dump_For_Schema => Load round trip for Float64Type");
        DECREF(dump);
        DECREF(other);
    }

    DECREF(i32);
    DECREF(i64);
    DECREF(f32);
    DECREF(f64);
    DECREF(sim);
}

void
TestNumericType_run_tests()
{
    TestBatch *batch = TestBatch_new(16);
    TestBatch_Plan(batch);
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

