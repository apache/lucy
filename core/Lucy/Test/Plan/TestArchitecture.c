#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Plan/TestArchitecture.h"
#include "Lucy/Plan/Architecture.h"
#include "Lucy/Index/Similarity.h"

static void
test_Equals(TestBatch *batch)
{
    Architecture *arch1 = Arch_new();
    Architecture *arch2 = Arch_new();
    ASSERT_TRUE(batch, Arch_Equals(arch1, (Obj*)arch2), 
        "Equals() true for different objects of the same class");
    ASSERT_FALSE(batch, Arch_Equals(arch1, NULL), 
        "Equals() false for NULL");
    ASSERT_FALSE(batch, Arch_Equals(arch1, (Obj*)&EMPTY), 
        "Equals() false for different type of object");
    DECREF(arch1);
    DECREF(arch2);
}

static void
test_intervals(TestBatch *batch)
{
    Architecture *arch = Arch_new();
    ASSERT_INT_EQ(batch, Arch_Index_Interval(arch), 128, "Index_Interval()");
    ASSERT_INT_EQ(batch, Arch_Skip_Interval(arch), 16, "Skip_Interval()");
}

static void
test_Make_Similarity(TestBatch *batch)
{
    Architecture *arch = Arch_new();
    Similarity *sim = Arch_Make_Similarity(arch);
    ASSERT_TRUE(batch, sim && Sim_Is_A(sim, SIMILARITY), "Make_Similarity()");
    DECREF(sim);
    DECREF(arch);
}

void
TestArch_run_tests()
{
    TestBatch *batch = TestBatch_new(6);
    TestBatch_Plan(batch);
    test_Equals(batch);
    test_intervals(batch);
    test_Make_Similarity(batch);
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

