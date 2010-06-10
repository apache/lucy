#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Plan/TestTextType.h"
#include "Lucy/Test/Plan/TestFieldType.h"
#include "Lucy/Test/Index/Similarity/DummySimilarity.h"
#include "Lucy/Plan/TextType.h"
#include "Lucy/Test/Analysis/DummyAnalyzer.h"

static void
test_Equals(TestBatch *batch)
{
    Analyzer *analyzer = (Analyzer*)DummyAnalyzer_new(1);
    TextType *type     = TextType_new(analyzer, NULL);

    ASSERT_FALSE(batch, TextType_Equals(type, NULL),
        "Equals() false with NULL");

    FieldType *dummy = (FieldType*)DummyFieldType_new(NULL);
    ASSERT_FALSE(batch, TextType_Equals(type, (Obj*)dummy),
        "Equals() false with non-TextType");
    DECREF(dummy);

    Analyzer *alt_analyzer = (Analyzer*)DummyAnalyzer_new(2);
    TextType *analyzer_differs = TextType_new(alt_analyzer, NULL);
    ASSERT_FALSE(batch, TextType_Equals(type, (Obj*)analyzer_differs),
        "Equals() false with different Analyzer");
    DECREF(analyzer_differs);
    DECREF(alt_analyzer);

    TextType *hl_differs = TextType_new(analyzer, NULL);
    TextType_Set_Highlightable(hl_differs, true);
    ASSERT_FALSE(batch, TextType_Equals(type, (Obj*)hl_differs),
        "Equals() false with highlightable => true");
    DECREF(hl_differs);

    DECREF(type);
    DECREF(analyzer);
}

static void
test_Dump_and_Load(TestBatch *batch)
{
    Analyzer   *analyzer = (Analyzer*)DummyAnalyzer_new(1);
    Similarity *sim      = (Similarity*)DummySim_new(1);
    TextType   *type     = TextType_new(analyzer, sim);

    // Set all settings to their non-defaults so that Dump must catch them.
    TextType_Set_Highlightable(type, true);
    TextType_Set_Indexed(type, false);
    TextType_Set_Stored(type, false);
    TextType_Set_Sortable(type, true);

    {
        Obj *dump  = (Obj*)TextType_Dump(type);
        Obj *other = Obj_Load(dump, dump);
        ASSERT_TRUE(batch, TextType_Equals(type, other), 
            "Dump => Load round trip");
        DECREF(dump);
        DECREF(other);
    }

    {
        Obj *dump = (Obj*)TextType_Dump_For_Schema(type);
        // (These steps are normally performed by Schema_Load() internally.) 
        Hash_Store_Str((Hash*)dump, "analyzer", 8, INCREF(analyzer));
        Hash_Store_Str((Hash*)dump, "similarity", 10, INCREF(sim));
        TextType *other = (TextType*)TextType_load(NULL, dump);
        ASSERT_TRUE(batch, TextType_Equals(type, (Obj*)other), 
            "Dump_For_Schema => Load round trip");
        DECREF(dump);
        DECREF(other);
    }

    DECREF(type);
    DECREF(sim);
    DECREF(analyzer);
}

void
TestTextType_run_tests()
{
    TestBatch *batch = TestBatch_new(6);
    TestBatch_Plan(batch);
    test_Equals(batch);
    test_Dump_and_Load(batch);
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

