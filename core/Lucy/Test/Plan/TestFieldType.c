#define C_LUCY_DUMMYFIELDTYPE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Plan/TestFieldType.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test/Index/Similarity/DummySimilarity.h"

DummyFieldType*
DummyFieldType_new(Similarity *similarity)
{
    DummyFieldType *self = (DummyFieldType*)VTable_Make_Obj(DUMMYFIELDTYPE);
    self->binary = false;
    return (DummyFieldType*)FType_init((FieldType*)self, similarity);
}

bool_t
DummyFieldType_binary(DummyFieldType *self) { return self->binary; }

static void
test_primitive_accessors(TestBatch *batch)
{
    FieldType *type = (FieldType*)DummyFieldType_new(NULL);

    ASSERT_FALSE(batch, FType_Indexed(type), "indexed false by default");
    FType_Set_Indexed(type, true);
    ASSERT_TRUE(batch, FType_Indexed(type), "Set_Indexed()");

    ASSERT_FALSE(batch, FType_Stored(type), "stored false by default");
    FType_Set_Stored(type, true);
    ASSERT_TRUE(batch, FType_Stored(type), "Set_Stored()");

    ASSERT_FALSE(batch, FType_Sortable(type), "sortable false by default");
    FType_Set_Sortable(type, true);
    ASSERT_TRUE(batch, FType_Sortable(type), "Set_Sortable()");

    ASSERT_TRUE(batch, FType_Get_Boost(type) == 1.0f, 
        "Boost defaults to 1.0");
    FType_Set_Boost(type, 2.0f);
    ASSERT_TRUE(batch, FType_Get_Boost(type) == 2.0f, "Set_Boost()");

    DECREF(type);
}

static void
test_Compare_Values(TestBatch *batch)
{
    FieldType     *type = (FieldType*)DummyFieldType_new(NULL);
    ZombieCharBuf *a    = ZCB_WRAP_STR("a", 1);
    ZombieCharBuf *b    = ZCB_WRAP_STR("b", 1);

    ASSERT_TRUE(batch, FType_Compare_Values(type, (Obj*)a, (Obj*)b) < 0,
        "a less than b");
    ASSERT_TRUE(batch, FType_Compare_Values(type, (Obj*)b, (Obj*)a) > 0,
        "b greater than a");
    ASSERT_TRUE(batch, FType_Compare_Values(type, (Obj*)b, (Obj*)b) == 0,
        "b equals b");

    DECREF(type);
}

void
test_Equals(TestBatch *batch)
{
    FieldType *type = (FieldType*)DummyFieldType_new(NULL);

    ASSERT_FALSE(batch, FType_Equals(type, NULL),
        "Equals() false for NULL other");
    ASSERT_FALSE(batch, FType_Equals(type, (Obj*)&EMPTY),
        "Equals() false for non-FieldType other");

    FieldType *doppelganger = (FieldType*)DummyFieldType_new(NULL);
    ASSERT_TRUE(batch, FType_Equals(type, (Obj*)doppelganger), "Equals()");
    DECREF(doppelganger);

    FieldType *indexed_differs = (FieldType*)DummyFieldType_new(NULL);
    FType_Set_Indexed(indexed_differs, true);
    ASSERT_FALSE(batch, FType_Equals(type, (Obj*)indexed_differs), 
        "Different value for 'indexed' spoils Equals()");
    DECREF(indexed_differs);

    FieldType *stored_differs = (FieldType*)DummyFieldType_new(NULL);
    FType_Set_Stored(stored_differs, true);
    ASSERT_FALSE(batch, FType_Equals(type, (Obj*)stored_differs), 
        "Different value for 'stored' spoils Equals()");
    DECREF(stored_differs);
    
    FieldType *sortable_differs = (FieldType*)DummyFieldType_new(NULL);
    FType_Set_Sortable(sortable_differs, true);
    ASSERT_FALSE(batch, FType_Equals(type, (Obj*)sortable_differs), 
        "Different value for 'sortable' spoils Equals()");
    DECREF(sortable_differs);

    FieldType *boost_differs = (FieldType*)DummyFieldType_new(NULL);
    FType_Set_Boost(boost_differs, 2.0f);
    ASSERT_FALSE(batch, FType_Equals(type, (Obj*)boost_differs), 
        "Different value for 'boost' spoils Equals()");
    DECREF(boost_differs);

    FieldType *binary_differs = (FieldType*)DummyFieldType_new(NULL);
    ((DummyFieldType*)binary_differs)->binary = true;
    ASSERT_FALSE(batch, FType_Equals(type, (Obj*)binary_differs), 
        "Different value for 'binary' spoils Equals()");
    DECREF(binary_differs);

    Similarity *sim     = (Similarity*)DummySim_new(1);
    Similarity *alt_sim = (Similarity*)DummySim_new(2);
    FieldType  *has_sim = (FieldType*)DummyFieldType_new(sim);
    FieldType  *has_alt_sim = (FieldType*)DummyFieldType_new(alt_sim);
    ASSERT_FALSE(batch, FType_Equals(type, (Obj*)alt_sim), 
        "NULL vs non-NULL sim spoils Equals()");
    ASSERT_FALSE(batch, FType_Equals(has_sim, (Obj*)type), 
        "non-NULL vs NULL sim spoils Equals()");
    ASSERT_FALSE(batch, FType_Equals(has_sim, (Obj*)has_alt_sim), 
        "different sim spoils Equals()");
    DECREF(has_alt_sim);
    DECREF(has_sim);
    DECREF(alt_sim);
    DECREF(sim);

    DECREF(type);
}

void
TestFType_run_tests()
{
    TestBatch *batch = TestBatch_new(22);
    TestBatch_Plan(batch);
    test_primitive_accessors(batch);
    test_Compare_Values(batch);
    test_Equals(batch);
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

