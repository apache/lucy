#define C_LUCY_CHARBUF
#define C_LUCY_SCHEMA
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Plan/TestSchema.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Plan/BlobType.h"
#include "Lucy/Plan/NumericType.h"
#include "Lucy/Plan/TextType.h"
#include "Lucy/Test/Plan/TestFieldType.h"
#include "Lucy/Test/Index/Similarity/DummySimilarity.h"
#include "Lucy/Test/Analysis/DummyAnalyzer.h"

CharBuf foo = ZCB_LITERAL("foo");
CharBuf bar = ZCB_LITERAL("bar");
CharBuf baz = ZCB_LITERAL("baz");

static void
test_Spec_Field_and_fetch_methods(TestBatch *batch)
{
    Analyzer   *analyzer_a = (Analyzer*)DummyAnalyzer_new(1);
    Analyzer   *analyzer_b = (Analyzer*)DummyAnalyzer_new(2);
    Similarity *sim_a      = (Similarity*)DummySim_new(1);
    Similarity *sim_b      = (Similarity*)DummySim_new(2);
    FieldType  *type_a     = (FieldType*)TextType_new(analyzer_a, sim_a);
    FieldType  *type_b     = (FieldType*)TextType_new(analyzer_b, sim_b);
    Schema     *schema     = Schema_new();

    Schema_Spec_Field(schema, &foo, type_a);
    Schema_Spec_Field(schema, &bar, type_b);
    
    ASSERT_TRUE(batch, Schema_Num_Fields(schema) == 2, "Num_Fields()");

    VArray *all_fields = Schema_All_Fields(schema);
    VA_Sort(all_fields, NULL, NULL);
    ASSERT_TRUE(batch,
           VA_Get_Size(all_fields) == 2
        && CB_Equals(&bar, VA_Fetch(all_fields, 0)) 
        && CB_Equals(&foo, VA_Fetch(all_fields, 1)),
        "All_Fields");
    DECREF(all_fields);

    ASSERT_TRUE(batch, Schema_Fetch_Type(schema, &foo) == type_a,
        "Fetch_Type");
    ASSERT_TRUE(batch, Schema_Fetch_Type(schema, &bar) == type_b,
        "Fetch_Type");
    ASSERT_TRUE(batch, Schema_Fetch_Type(schema, &baz) == NULL,
        "Fetch_Type with unknown field returns NULL");

    ASSERT_TRUE(batch, Schema_Fetch_Analyzer(schema, &foo) == analyzer_a,
        "Fetch_Analyzer");
    ASSERT_TRUE(batch, Schema_Fetch_Analyzer(schema, &bar) == analyzer_b,
        "Fetch_Analyzer");
    ASSERT_TRUE(batch, Schema_Fetch_Analyzer(schema, &baz) == NULL,
        "Fetch_Analyzer with unknown field returns NULL");

    ASSERT_TRUE(batch, Schema_Fetch_Sim(schema, &foo) == sim_a,
        "Fetch_Sim");
    ASSERT_TRUE(batch, Schema_Fetch_Sim(schema, &bar) == sim_b,
        "Fetch_Sim");
    ASSERT_TRUE(batch, Schema_Fetch_Sim(schema, &baz) == NULL,
        "Fetch_Sim with unknown field returns NULL");

    DECREF(schema);
    DECREF(type_b);
    DECREF(type_a);
    DECREF(sim_b);
    DECREF(sim_a);
    DECREF(analyzer_b);
    DECREF(analyzer_a);
}

static void
test_Dump_Load_and_Equals(TestBatch *batch)
{
    FieldType *type_a = (FieldType*)BlobType_new(NULL);
    Schema    *schema = Schema_new();
    Schema_Spec_Field(schema, &foo, type_a);

    ASSERT_FALSE(batch, Schema_Equals(schema, NULL),
       "Equals() is false for NULL");
    ASSERT_FALSE(batch, Schema_Equals(schema, (Obj*)&foo),
       "Equals() is false for non-Schema object");

    {
        FieldType *type_b = (FieldType*)Int32Type_new(NULL);
        Schema *type_differs = Schema_new();
        Schema_Spec_Field(type_differs, &foo, type_b);
        ASSERT_FALSE(batch, Schema_Equals(schema, (Obj*)type_differs),
            "Conflicting types spoil Equals()");
        DECREF(type_differs);
        DECREF(type_b);
    }

    {
        Schema *sim_differs = Schema_new();
        Schema_Spec_Field(sim_differs, &foo, type_a);
        DECREF(sim_differs->sim); // hackity hackity
        sim_differs->sim = (Similarity*)DummySim_new(0); // hackity hack
        ASSERT_FALSE(batch, Schema_Equals(schema, (Obj*)sim_differs),
            "Conflicting sims spoil Equals()");
        DECREF(sim_differs);
    }

    {
        Obj *dump = (Obj*)Schema_Dump(schema);
        Schema *loaded = (Schema*)Obj_Load(dump, dump);
        ASSERT_TRUE(batch, Schema_Equals(schema, (Obj*)loaded),
            "Round trip through Dump/Load");
        DECREF(loaded);
        DECREF(dump);
    }

    DECREF(schema);
    DECREF(type_a);
}

static void
test_Eat(TestBatch *batch)
{
    FieldType *type     = (FieldType*)BlobType_new(NULL);
    Schema    *predator = Schema_new();
    Schema    *prey     = Schema_new();
    Schema_Spec_Field(predator, &foo, type);
    Schema_Spec_Field(prey, &foo, type);
    Schema_Spec_Field(prey, &bar, type);
    Schema_Eat(predator, prey);
    ASSERT_TRUE(batch, Schema_Fetch_Type(predator, &bar) == type,
        "Copy field specs during Eat()");
    DECREF(prey);
    DECREF(predator);
    DECREF(type);
}

void
TestSchema_run_tests()
{
    TestBatch *batch = TestBatch_new(17);
    TestBatch_Plan(batch);
    test_Spec_Field_and_fetch_methods(batch);
    test_Dump_Load_and_Equals(batch);
    test_Eat(batch);
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

